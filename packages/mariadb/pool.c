/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2013, Zeying Xie <swpdtz at gmail dot com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <mysql.h>
#include "common.h"
#include "query.h"
#include "connection_priv.h"
#include "async.h"
#include "dthread.h"
#include "pool.h"

static inline int  __mariadb_pool_spawn_conn(mariadb_pool_t *pool, int size)
{
    int i;
    mariadb_conn_t *conn;
    mariadb_conn_config_t config = pool->config->conn_config;

    for (i = 0; i < size; ++i) {
        conn = mariadb_conn_create(NULL, config.user, config.password, config.host,
                                   config.db, config.port, config.unix_socket,
                                   config.client_flag);
        if (!conn) {
            break;
        }

        if (pool->config->use_ssl) {
            mysql_ssl_set(&conn->mysql, config.ssl_key, config.ssl_cert,
                          config.ssl_ca, config.ssl_capath, config.ssl_cipher);
        }

        conn->is_pooled = 1;
        conn->pool = pool;
        mk_list_add(&conn->_pool_head, &pool->free_conns);
        pool->size++;
        pool->free_size++;
    }

    if (pool->free_size == 0) {
        return MARIADB_ERR;
    }

    return MARIADB_OK;
}

static inline void __mariadb_pool_release_conn(mariadb_pool_t *pool, int size, int is_async)
{
    int i;
    mariadb_conn_t *conn;

    for (i = 0; i < size; ++i) {
        conn = mk_list_entry_first(&pool->free_conns, mariadb_conn_t, _pool_head);
        mk_list_del(&conn->_pool_head);
        conn->is_pooled = 0;
        if (is_async) {
            mariadb_async_handle_release(conn, MARIADB_OK);
        } else {
            mariadb_dthread_disconnect(conn);
        }
        pool->size--;
        pool->free_size--;
    }
}

static inline mariadb_pool_config_t *__mariadb_pool_get_config(duda_global_t *pool_key)
{
    struct mk_list *head;
    mariadb_pool_config_t *config = NULL;

    mk_list_foreach(head, &mariadb_pool_config_list) {
        config = mk_list_entry(head, mariadb_pool_config_t, _head);
        if (config->pool_key == pool_key) {
            break;
        }
    }

    return config;
}

/*
 * @METHOD_NAME: create_pool
 * @METHOD_DESC: Create a connection pool per thread for connection sharing. It must be called within the function `duda_main()' of a Duda web service.
 * @METHOD_PROTO: int create_pool(duda_global_t *pool_key, int min_size, int max_size, const char *user, const char *password, const char *host, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag)
 * @METHOD_PARAM: pool_key The pointer that refers to the global key definition of a pool.
 * @METHOD_PARAM: min_size The minimum number of connections in the pool.
 * @METHOD_PARAM: max_size The maximum number of connections in the pool.
 * @METHOD_PARAM: user The user's MYSQL login ID. If `user' is NULL or the empty string, the current user is assumed.
 * @METHOD_PARAM: password The password of `user'.
 * @METHOD_PARAM: host The host name or IP address of the MariaDB server. If `host' is NULL or the string is "localhost", a connection to the local host is assumed.
 * @METHOD_PARAM: db The database to use for this connection.
 * @METHOD_PARAM: port The port number to connect to. If `port' is 0, the default port number 3306 is assumed.
 * @METHOD_PARAM: unix_socket The path name to a unix domain socket to connect to. If `unix_socket' is NULL, the default path is assumed.(Depending on your Linux distribution)
 * @METHOD_PARAM: client_flag The combination of flags to enable some features, such as mutli-statements query. For full information and available flags please refer to <a href="http://dev.mysql.com/doc/refman/5.7/en/mysql-real-connect.html">this</a>.
 * @METHOD_RETURN: MAIRADB_OK on success, or MARIADB_ERR on failure.
 */

int mariadb_pool_create(duda_global_t *pool_key, int min_size, int max_size,
                        const char *user, const char *password, const char *host,
                        const char *db, unsigned int port, const char *unix_socket,
                        unsigned long client_flag)
{
    mariadb_pool_config_t *config = monkey->mem_alloc(sizeof(mariadb_pool_config_t));
    if (!config) {
        return MARIADB_ERR;
    }

    config->pool_key                = pool_key;
    config->conn_config.user        = monkey->str_dup(user);
    config->conn_config.password    = monkey->str_dup(password);
    config->conn_config.host        = monkey->str_dup(host);
    config->conn_config.db          = monkey->str_dup(db);
    config->conn_config.port        = port;
    config->conn_config.unix_socket = monkey->str_dup(unix_socket);
    config->conn_config.client_flag = client_flag;
    config->use_ssl                 = 0;

    if (min_size == 0) {
        config->min_size = MARIADB_POOL_DEFAULT_MIN_SIZE;
    } else {
        config->min_size = min_size;
    }
    if (max_size == 0) {
        config->max_size = MARIADB_POOL_DEFAULT_MAX_SIZE;
    } else {
        config->max_size = max_size;
    }

    mk_list_add(&config->_head, &mariadb_pool_config_list);
    return MARIADB_OK;
}

/*
 * @METHOD_NAME: pool_set_ssl
 * @METHOD_DESC: Create a pool of MariaDB secure connections per thread using SSL. It must be called within the function `duda_main()' of a Duda web service and shall be called after `mariadb->create_pool()'. 
 * @METHOD_PROTO: int pool_set_ssl(duda_global_t *pool_key, const char *key, const char *cert, const char *ca, const char *capath, const char *cipher)
 * @METHOD_PARAM: pool_key The pointer that refers to the global key definition of a pool.
 * @METHOD_PARAM: key The path name to the key file.
 * @METHOD_PARAM: cert The path name to the certificate file.
 * @METHOD_PARAM: ca The path name to the certificate authority file.
 * @METHOD_PARAM: capath The path name to a directory that contains trusted SSL CA certificates in PEM format.
 * @METHOD_PARAM: cipher A list of permissible ciphers to use for SSL encryption. If `cipher' is NULL, the default cipher list is assumed.
 * @METHOD_RETURN: MAIRADB_OK on success, or MARIADB_ERR on failure.
 */

int mariadb_pool_set_ssl(duda_global_t *pool_key, const char *key, const char *cert,
                         const char *ca, const char *capath, const char *cipher)
{
    mariadb_pool_config_t *config = __mariadb_pool_get_config(pool_key);
    if (!config) {
        return MARIADB_ERR;
    }

    config->conn_config.ssl_key    = monkey->str_dup(key);
    config->conn_config.ssl_cert   = monkey->str_dup(cert);
    config->conn_config.ssl_ca     = monkey->str_dup(ca);
    config->conn_config.ssl_capath = monkey->str_dup(capath);
    config->conn_config.ssl_cipher = monkey->str_dup(cipher);
    config->use_ssl                = 1;

    return MARIADB_OK;
}

mariadb_conn_t *mariadb_pool_get_conn(duda_global_t *pool_key, duda_request_t *dr,
        mariadb_connect_cb *cb, int is_async)
{
    mariadb_pool_t *pool;
    mariadb_pool_config_t *config;
    mariadb_conn_t *conn;
    mariadb_conn_config_t conn_config;
    int ret;

    pool = global->get(*pool_key);
    if (!pool) {
        pool = monkey->mem_alloc(sizeof(mariadb_pool_t));
        if (!pool) {
            return NULL;
        }

        config = __mariadb_pool_get_config(pool_key);
        if (!config) {
            FREE(pool);
            return NULL;
        }

        pool->size      = 0;
        pool->free_size = 0;
        pool->config = config;
        mk_list_init(&pool->free_conns);
        global->set(*pool_key, (void *) pool);
    }

    /* configuration of a pool */
    config = pool->config;

    if (mk_list_is_empty(&pool->free_conns) == 0) {
        if (pool->size < config->max_size) {
            ret = __mariadb_pool_spawn_conn(pool, MARIADB_POOL_DEFAULT_SIZE);

            if (ret != MARIADB_OK) {
                return NULL;
            }
        } else {
            conn_config = config->conn_config;
            conn = mariadb_conn_create(dr, conn_config.user, conn_config.password,
                                       conn_config.host, conn_config.db, conn_config.port,
                                       conn_config.unix_socket, conn_config.client_flag);
            if (!conn) {
                return NULL;
            }

            if (is_async) {
                ret = mariadb_async_handle_connect(conn, cb);
            } else {
                ret = mariadb_dthread_connect(conn);
            }
            if (ret != MARIADB_OK) {
                return NULL;
            }
            return conn;
        }
    }

    conn = mk_list_entry_first(&pool->free_conns, mariadb_conn_t, _pool_head);
    mk_list_del(&conn->_pool_head);
    pool->free_size--;

    if (is_async) {
        ret = mariadb_async_handle_connect(conn, NULL);
        conn->dr = dr;
        conn->connect_cb = cb;
        if (conn->connect_cb) {
            conn->connect_cb(conn, ret, conn->dr);
        }
    } else {
        ret = mariadb_dthread_connect(conn);
    }
    if (ret != MARIADB_OK) {
        return NULL;
    }

    return conn;
}

/*
 * @METHOD_NAME: pool_get_conn_async
 * @METHOD_DESC: Get a MariaDB connection from a connection pool. If all the connections in a pool are currently used, the pool will spawn more connections as long as the pool size don't exceed the maximum.
 * @METHOD_PROTO: mariadb_conn_t *pool_get_conn_async(duda_global_t *pool_key, duda_request_t *dr, mariadb_connect_cb *cb)
 * @METHOD_PARAM: pool_key The pointer that refers to the global key definition of a pool.
 * @METHOD_PARAM: dr The request context information hold by a duda_request_t type.
 * @METHOD_PARAM: cb The callback function that will take actions when a connection success or fail to establish.
 * @METHOD_RETURN: A MariaDB connection on success, or NULL on failure.
 */
mariadb_conn_t *mariadb_async_pool_get_conn(duda_global_t *pool_key, duda_request_t *dr,
        mariadb_connect_cb *cb)
{
    return mariadb_pool_get_conn(pool_key, dr, cb, 1);
}

/*
 * @METHOD_NAME: pool_get_conn
 * @METHOD_DESC: Similar to `pool_get_conn_async`, except that it will block util an established connection is returned.
 * @METHOD_PROTO: mariadb_conn_t *pool_get_conn(duda_global_t *pool_key)
 * @METHOD_PARAM: pool_key The pointer that refers to the global key definition of a pool.
 * @METHOD_RETURN: A MariaDB connection on success, or NULL on failure.
 */
mariadb_conn_t *mariadb_dthread_pool_get_conn(duda_global_t *pool_key)
{
    return mariadb_pool_get_conn(pool_key, NULL, NULL, 0);
}

void mariadb_pool_reclaim_conn(mariadb_conn_t *conn)
{
    mariadb_pool_t *pool = conn->pool;

    conn->dr                   = NULL;
    conn->connect_cb           = NULL;
    conn->disconnect_cb        = NULL;
    conn->disconnect_on_finish = 0;

    mk_list_add(&conn->_pool_head, &pool->free_conns);
    pool->free_size++;

    /* shrink the pool */
    while (pool->free_size * 2 > pool->size &&
           pool->size > MARIADB_POOL_DEFAULT_MIN_SIZE) {
        __mariadb_pool_release_conn(pool, MARIADB_POOL_DEFAULT_SIZE, 1);
    }
}

void mariadb_dthread_pool_reclaim_conn(mariadb_conn_t *conn)
{
    mariadb_pool_t *pool = conn->pool;
    conn->state = CONN_STATE_CONNECTED;
    mk_list_add(&conn->_pool_head, &pool->free_conns);
    pool->free_size++;

    /* shrink the pool */
    while (pool->free_size * 2 > pool->size &&
           pool->size > MARIADB_POOL_DEFAULT_MIN_SIZE) {
        __mariadb_pool_release_conn(pool, MARIADB_POOL_DEFAULT_SIZE, 0);
    }
}
