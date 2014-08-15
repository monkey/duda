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
#include "query_priv.h"
#include "connection_priv.h"

/*
 * @METHOD_NAME: create_conn
 * @METHOD_DESC: Create a new MariaDB connection handle.
 * @METHOD_PROTO: mariadb_conn_t *create_conn(duda_request_t *dr, const char *user, const char *password, const char *host, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag)
 * @METHOD_PARAM: dr The request context information hold by a duda_request_t type.
 * @METHOD_PARAM: user The user's MYSQL login ID. If `user' is NULL or the empty string, the current user is assumed.
 * @METHOD_PARAM: password The password of `user'.
 * @METHOD_PARAM: host The host name or IP address of the MariaDB server. If `host' is NULL or the string is "localhost", a connection to the local host is assumed.
 * @METHOD_PARAM: db The database to use for this connection.
 * @METHOD_PARAM: port The port number to connect to. If `port' is 0, the default port number 3306 is assumed.
 * @METHOD_PARAM: unix_socket The path name to a unix domain socket to connect to. If `unix_socket' is NULL, the default path is assumed.(Depending on your Linux distribution)
 * @METHOD_PARAM: client_flag The combination of flags to enable some features, such as mutli-statements query. For full information and available flags please refer to <a href="http://dev.mysql.com/doc/refman/5.7/en/mysql-real-connect.html">this</a>.
 * @METHOD_RETURN: A newly initialized MariaDB connection handle on success, or NULL on failure.
 */

mariadb_conn_t *mariadb_conn_create(duda_request_t *dr, const char *user,
                                    const char *password, const char *host,
                                    const char *db, unsigned int port,
                                    const char *unix_socket, unsigned long client_flag)
{
    mariadb_conn_t *conn = monkey->mem_alloc(sizeof(mariadb_conn_t));
    if (!conn) {
        return NULL;
    }

    conn->dr                   = dr;
    conn->config.user          = monkey->str_dup(user);
    conn->config.password      = monkey->str_dup(password);
    conn->config.host          = monkey->str_dup(host);
    conn->config.db            = monkey->str_dup(db);
    conn->config.port          = port;
    conn->config.unix_socket   = monkey->str_dup(unix_socket);
    conn->config.client_flag   = client_flag;
    conn->config.ssl_key       = NULL;
    conn->config.ssl_cert      = NULL;
    conn->config.ssl_ca        = NULL;
    conn->config.ssl_capath    = NULL;
    conn->config.ssl_cipher    = NULL;
    conn->mysql_ret            = NULL;
    conn->fd                   = 0;
    conn->state                = CONN_STATE_CLOSED;
    conn->connect_cb           = NULL;
    conn->disconnect_cb        = NULL;
    conn->current_query        = NULL;
    conn->disconnect_on_finish = 0;
    conn->is_pooled            = 0;
    conn->dthread_id           = -1;
    conn->pool                 = NULL;
    mysql_init(&conn->mysql);

    mysql_options(&conn->mysql, MYSQL_OPT_NONBLOCK, 0);
    mk_list_init(&conn->queries);

    return conn;
}

/*
 * @METHOD_NAME: set_ssl
 * @METHOD_DESC: Establish a MariaDB secure connection using SSL. It shall be called after `mariadb->create_conn()' and before `mariadb->connect()'.
 * @METHOD_PROTO: void set_ssl(mariadb_conn_t *conn, const char *key, const char *cert, const char *ca, const char *capath, const char *cipher)
 * @METHOD_PARAM: conn The MariaDB connection handle.
 * @METHOD_PARAM: key The path name to the key file.
 * @METHOD_PARAM: cert The path name to the certificate file.
 * @METHOD_PARAM: ca The path name to the certificate authority file.
 * @METHOD_PARAM: capath The path name to a directory that contains trusted SSL CA certificates in PEM format.
 * @METHOD_PARAM: cipher A list of permissible ciphers to use for SSL encryption. If `cipher' is NULL, the default cipher list is assumed.
 * @METHOD_RETURN: None.
 */

void mariadb_conn_set_ssl(mariadb_conn_t *conn, const char *key, const char *cert,
                          const char *ca, const char *capath, const char *cipher)
{
    conn->config.ssl_key    = monkey->str_dup(key);
    conn->config.ssl_cert   = monkey->str_dup(cert);
    conn->config.ssl_ca     = monkey->str_dup(ca);
    conn->config.ssl_capath = monkey->str_dup(capath);
    if (cipher) {
        conn->config.ssl_cipher = monkey->str_dup(cipher);
    } else {
        conn->config.ssl_cipher = DEFAULT_CIPHER;
    }
    mysql_ssl_set(&conn->mysql, conn->config.ssl_key, conn->config.ssl_cert,
                  conn->config.ssl_ca, conn->config.ssl_capath,
                  conn->config.ssl_cipher);
}

void mariadb_conn_free(mariadb_conn_t *conn)
{
    FREE(conn->config.user);
    FREE(conn->config.password);
    FREE(conn->config.host);
    FREE(conn->config.db);
    FREE(conn->config.unix_socket);
    FREE(conn->config.ssl_key);
    FREE(conn->config.ssl_cert);
    FREE(conn->config.ssl_ca);
    FREE(conn->config.ssl_capath);
    FREE(conn->config.ssl_cipher);
    while (mk_list_is_empty(&conn->queries) != 0) {
        mariadb_query_t *query = mk_list_entry_first(&conn->queries,
                                                     mariadb_query_t, _head);
        mariadb_query_free(query);
    }
    FREE(conn);
}
