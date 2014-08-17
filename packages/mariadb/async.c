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
#include "mariadb.h"
#include "query_priv.h"
#include "connection_priv.h"
#include "pool.h"
#include "async.h"

/*
 * @METHOD_NAME: connect_async
 * @METHOD_DESC: Attempts to establish a connection using a MariaDB connection handle created by `mairadb->create_conn()'.
 * @METHOD_PROTO: int connect_async(mariadb_conn_t *conn, mariadb_connect_cb *cb)
 * @METHOD_PARAM: conn The MariaDB connection handle, it must be a newly allocated one.
 * @METHOD_PARAM: cb The callback function that will take actions when a connection success or fail to establish.
 * @METHOD_RETURN: MAIRADB_OK on success, or MARIADB_ERR on failure.
 */

int mariadb_async_handle_connect(mariadb_conn_t *conn, mariadb_connect_cb *cb)
{
    int status;
    if(!conn->connect_cb) {
        conn->connect_cb = cb;
    }

    /* whether the connection has already been established */
    if (conn->state == CONN_STATE_CLOSED) { 
        status = mysql_real_connect_start(&conn->mysql_ret, &conn->mysql,
                                          conn->config.host, conn->config.user,
                                          conn->config.password, conn->config.db,
                                          conn->config.port,
                                          conn->config.unix_socket,
                                          conn->config.client_flag);
        if (!conn->mysql_ret && mysql_errno(&conn->mysql) > 0) {
            msg->err("MariaDB Connect Error: %s", mysql_error(&conn->mysql));
            if (conn->connect_cb) {
                conn->connect_cb(conn, MARIADB_ERR, conn->dr);
            }
            mariadb_conn_free(conn);
            return MARIADB_ERR;
        }
        conn->fd = mysql_get_socket(&conn->mysql);

        if (status) {
            conn->state = CONN_STATE_CONNECTING;
        } else {
            if (!conn->mysql_ret) {
                msg->err("[FD %i] MariaDB Connect Error: %s", conn->fd,
                         mysql_error(&conn->mysql));
                if (conn->connect_cb) {
                    conn->connect_cb(conn, MARIADB_ERR, conn->dr);
                }
                mariadb_conn_free(conn);
                return MARIADB_ERR;
            }
            conn->state = CONN_STATE_CONNECTED;
            if (conn->connect_cb) {
                conn->connect_cb(conn, MARIADB_OK, conn->dr);
            }
        }

        event->add(conn->fd, DUDA_EVENT_READ, DUDA_EVENT_LEVEL_TRIGGERED,
                   mariadb_on_read, mariadb_on_write, mariadb_on_error,
                   mariadb_on_close, mariadb_on_timeout, NULL);

        struct mk_list *conn_list = global->get(mariadb_conn_list);
        if (!conn_list) {
            conn_list = monkey->mem_alloc(sizeof(struct mk_list));
            if (!conn_list) {
                mariadb_conn_free(conn);
                return MARIADB_ERR;
            }
            mk_list_init(conn_list);
            global->set(mariadb_conn_list, (void *) conn_list);
        }
        mk_list_add(&conn->_head, conn_list);
        /* handle pending queries on connected */
        if (conn->state == CONN_STATE_CONNECTED) {
            mariadb_async_handle_query(conn);
        }
    }

    return MARIADB_OK;
}

void mariadb_async_handle_query(mariadb_conn_t *conn)
{
    int status;

    while (mk_list_is_empty(&conn->queries) != 0) {
        mariadb_query_t *query = mk_list_entry_first(&conn->queries,
                                                     mariadb_query_t, _head);
        conn->current_query = query;
        if (query->abort) {
            mariadb_query_free(query);
            conn->state = CONN_STATE_CONNECTED;
            continue;
        }

        if (query->query_str) {
            status = mysql_real_query_start(&query->error, &conn->mysql,
                                            query->query_str,
                                            strlen(query->query_str));
            if (status) {
                conn->state = CONN_STATE_QUERYING;
                return;
            }
            if (query->error) {
                msg->err("[FD %i] MariaDB Query Error: %s", conn->fd,
                         mysql_error(&conn->mysql));
                /* may add a query on error callback to be called here */
                mariadb_query_free(query);
            } else {
                conn->state = CONN_STATE_QUERIED;
                mariadb_async_handle_result(conn);
                if (conn->state != CONN_STATE_CONNECTED) {
                    return;
                }
            }
        } else {
            msg->err("[FD %i] MariaDB Query Statement Missing", conn->fd);
            mariadb_query_free(query);
        }
    }
    conn->current_query = NULL;
    conn->state         = CONN_STATE_CONNECTED;
    /* all queries have been handled */
    event->mode(conn->fd, DUDA_EVENT_SLEEP, DUDA_EVENT_LEVEL_TRIGGERED);
    if (conn->disconnect_on_finish) {
        mariadb_async_handle_release(conn, MARIADB_OK);
    }
}

void mariadb_async_handle_result(mariadb_conn_t *conn)
{
    mariadb_query_t *query = conn->current_query;

    query->result = mysql_use_result(&conn->mysql);
    if (!query->result) {
        mariadb_query_free(query);
        conn->state = CONN_STATE_CONNECTED;
        return;
    }
    if (query->abort) {
        if (query->result) {
            mariadb_async_handle_result_free(conn);
        } else {
            mariadb_query_free(query);
            conn->state = CONN_STATE_CONNECTED;
        }
        return;
    }

    query->n_fields = mysql_num_fields(query->result);
    query->fields = monkey->mem_alloc(sizeof(char *) * query->n_fields);
    MYSQL_FIELD *fields = mysql_fetch_fields(query->result);
    unsigned int i;
    for (i = 0; i < query->n_fields; ++i) {
        (query->fields)[i] = monkey->str_dup(fields[i].name);
    }

    if (query->result_cb) {
        query->result_cb(query->privdata, query, query->n_fields, query->fields, conn->dr);
    }
    mariadb_async_handle_row(conn);
}

void mariadb_async_handle_row(mariadb_conn_t *conn)
{
    int status;
    mariadb_query_t *query = conn->current_query;

    while (1) {
        status = mysql_fetch_row_start(&query->row, query->result);
        if (status) {
            conn->state = CONN_STATE_ROW_FETCHING;
            return;
        }
        conn->state = CONN_STATE_ROW_FETCHED;
        /* all rows have been fetched */
        if (!query->row) {
            if (mysql_errno(&conn->mysql) > 0) {
                msg->err("[FD %i] MariaDB Fetch Result Row Error: %s", conn->fd,
                         mysql_error(&conn->mysql));
            } else {
                if (query->end_cb) {
                    query->end_cb(query->privdata, query, conn->dr);
                }
            }
            mariadb_async_handle_result_free(conn);
            break;
        }
        if (query->row_cb) {
            query->row_cb(query->privdata, query->n_fields, query->fields,
                          query->row, conn->dr);
        }
    }
}

void mariadb_async_handle_result_free(mariadb_conn_t *conn)
{
    int status;
    mariadb_query_t *query = conn->current_query;

    status = mysql_free_result_start(query->result);
    if (status) {
        conn->state = CONN_STATE_RESULT_FREEING;
        return;
    }
    conn->state = CONN_STATE_RESULT_FREED;
    if ((conn->config.client_flag & CLIENT_MULTI_STATEMENTS) == 0) {
        conn->current_query = NULL;
        mariadb_query_free(query);
        conn->state = CONN_STATE_CONNECTED;
        return;
    }
    mariadb_async_handle_next_result(conn);
}

void mariadb_async_handle_next_result(mariadb_conn_t *conn)
{
    int status;
    mariadb_query_t *query = conn->current_query;

    while (1) {
        status = mysql_next_result_start(&query->error, &conn->mysql);
        if (status) {
            conn->state = CONN_STATE_NEXT_RESULTING;
            return;
        }
        conn->state = CONN_STATE_NEXT_RESULTED;
        if (query->error == -1) {
            break;
        } else if (query->error == 0){
            mariadb_async_handle_result(conn);
            if (conn->state != CONN_STATE_RESULT_FREED) {
                return;
            }
        } else {
            msg->err("[FD %i] MariaDB Execute Statement Error: %s", conn->fd,
                     mysql_error(&conn->mysql));
        }
    }
    conn->current_query = NULL;
    mariadb_query_free(query);
    conn->state = CONN_STATE_CONNECTED;
}

/*
 * @METHOD_NAME: disconnect_async
 * @METHOD_DESC: Disconnect a previous opened connection and release all the resource with it. It will ensure that all previous enqueued queries of that connection are processed before it is disconnected.
 * @METHOD_PROTO: void disconnect_async(mariadb_conn_t *conn, mariadb_disconnect_cb *cb)
 * @METHOD_PARAM: conn The MariaDB connection handle, it must be a valid, open connection.
 * @METHOD_PARAM: cb The callback function that will take actions when a connection is disconnected.
 * @METHOD_RETURN: None.
 */

void mariadb_async_handle_disconnect(mariadb_conn_t *conn, mariadb_disconnect_cb *cb)
{
    if (!conn->disconnect_cb) {
        conn->disconnect_cb = cb;
    }
    if (conn->state != CONN_STATE_CONNECTED) {
        conn->disconnect_on_finish = 1;
        return;
    }
    mariadb_async_handle_release(conn, MARIADB_OK);
}


void mariadb_async_handle_release(mariadb_conn_t* conn, int status)
{
    if (conn->is_pooled) {
        event->mode(conn->fd, DUDA_EVENT_SLEEP, DUDA_EVENT_LEVEL_TRIGGERED);
    } else {
        event->delete(conn->fd);
    }
    if (conn->state >= CONN_STATE_CONNECTED && conn->disconnect_cb) {
        conn->disconnect_cb(conn, status, conn->dr);
    }
    if (conn->is_pooled) {
        mariadb_pool_reclaim_conn(conn);
    } else {
        if (conn->state != CONN_STATE_CLOSED) {
            mysql_close(&conn->mysql);
        }
        conn->state = CONN_STATE_CLOSED;
        mk_list_del(&conn->_head);
        mariadb_conn_free(conn);
    }
}

/*
 * @METHOD_NAME: query_async
 * @METHOD_DESC: Enqueue a new query to a MariaDB connection.
 * @METHOD_PROTO: int query_async(mariadb_conn_t *conn, const char *query_str, mariadb_query_result_cb *result_cb, mariadb_query_row_cb *row_cb, void *privdata, mariadb_query_end_cb *end_cb)
 * @METHOD_PARAM: conn The MariaDB connection handle.
 * @METHOD_PARAM: query_str The SQL statement string of this query.
 * @METHOD_PARAM: result_cb The callback function that will take actions when the result set of this query is available.
 * @METHOD_PARAM: row_cb The callback function that will take actions when every row of the result set is fetched.
 * @METHOD_PARAM: end_cb The callback function that will take actions after all the row in the result set are fetched.
 * @METHOD_PARAM: privdata The user defined private data that will be passed to callback.
 * @METHOD_RETURN: MAIRADB_OK on success, or MARIADB_ERR on failure.
 */

int mariadb_async_handle_add_query(mariadb_conn_t *conn, const char *query_str,
                                   mariadb_query_result_cb *result_cb,
                                   mariadb_query_row_cb *row_cb,
                                   mariadb_query_end_cb *end_cb, void *privdata)
{
    mariadb_query_t *query = mariadb_query_init(query_str, result_cb, row_cb,
                                                end_cb, privdata);
    if (!query) {
        msg->err("[FD %i] MariaDB Add Query Error", conn->fd);
        return MARIADB_ERR;
    }
    mk_list_add(&query->_head, &conn->queries);

    if (conn->state == CONN_STATE_CONNECTED) {
        event->mode(conn->fd, DUDA_EVENT_WAKEUP, DUDA_EVENT_LEVEL_TRIGGERED);
        mariadb_async_handle_query(conn);
    }
    return MARIADB_OK;
}
