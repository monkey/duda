/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2014, Zeying Xie <swpdtz at gmail dot com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <assert.h>
#include <mysql.h>
#include "common.h"
#include "query.h"
#include "connection_priv.h"
#include "pool.h"
#include "dthread.h"

extern duda_global_t mariadb_conn_list;

struct mariadb_result {
    unsigned int n_fields;
    char **fields;
    MYSQL_RES *result;
};

static inline mariadb_conn_t *mariadb_get_conn(int fd)
{
    struct mk_list *conn_list, *head;
    mariadb_conn_t *conn = NULL;

    conn_list = global->get(mariadb_conn_list);
    mk_list_foreach(head, conn_list) {
        conn = mk_list_entry(head, mariadb_conn_t, _head);
        if (conn->fd == fd) {
            break;
        }
    }
    return conn;
}

/*
 * @METHOD_NAME: connect
 * @METHOD_DESC: Similar to `maraidb->connect_async` except it won't return until the connection is established or some errors occur on the connection.
 * @METHOD_PROTO: int connect(mariadb_conn_t *conn)
 * @METHOD_PARAM: conn The MariaDB connection handle, it must be a newly allocated one.
 * @METHOD_RETURN: MAIRADB_OK on success, or MARIADB_ERR on failure.
 */
int mariadb_dthread_connect(mariadb_conn_t *conn)
{
    assert(conn);
    conn->dthread_id = dthread->running();
    int status;

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
                mariadb_conn_free(conn);
                return MARIADB_ERR;
            }
            conn->state = CONN_STATE_CONNECTED;
        }
        // add to connection list
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

        event->add(conn->fd, DUDA_EVENT_READ, DUDA_EVENT_LEVEL_TRIGGERED,
                   mariadb_dthread_on_read, mariadb_dthread_on_write,
                   mariadb_dthread_on_error, mariadb_dthread_on_close,
                   mariadb_dthread_on_timeout, NULL);

        if (conn->state == CONN_STATE_CONNECTING) {
            dthread->yield();
        }
        if (conn->state == CONN_STATE_CONNECTED) {
            return MARIADB_OK;
        }

        while (1) {
            if (conn->state == CONN_STATE_ERROR) {
                msg->err("[FD %i] MariaDB Connect Error: %s", conn->fd,
                         mysql_error(&conn->mysql));
                conn->is_pooled = 0;
                mariadb_dthread_disconnect(conn);
                return MARIADB_ERR;
            }

            status = mysql_real_connect_cont(&conn->mysql_ret, &conn->mysql,
                                             MYSQL_WAIT_READ);
            if (status) {
                dthread->yield();
            } else {
                if (!conn->mysql_ret) {
                    msg->err("[FD %i] MariaDB Connect Error: %s", conn->fd,
                             mysql_error(&conn->mysql));
                    conn->is_pooled = 0;
                    mariadb_dthread_disconnect(conn);
                    return MARIADB_ERR;
                }
                conn->state = CONN_STATE_CONNECTED;
                return MARIADB_OK;
            }
        }
    }
    return MARIADB_OK;
}

mariadb_result_t *mariadb_dthread_result_create()
{
    mariadb_result_t *result = monkey->mem_alloc(sizeof(*result));
    assert(result);
    result->n_fields = 0;
    result->fields = NULL;
    result->result = NULL;
    return result;
}

static void mariadb_dthread_result_free(mariadb_conn_t *conn, mariadb_result_t *result)
{
    assert(conn);
    assert(result);
    int status = mysql_free_result_start(result->result);
    if (status) {
        conn->state = CONN_STATE_RESULT_FREEING;
        dthread->yield();
    } else {
        conn->state = CONN_STATE_RESULT_FREED;
        return;
    }

    while (1) {
        if (conn->state == CONN_STATE_ERROR) {
            msg->err("[FD %i] MariaDB Result Free Error: %s", conn->fd,
                     mysql_error(&conn->mysql));
            conn->is_pooled = 0;
            mariadb_dthread_disconnect(conn);
            return;
        }

        status = mysql_free_result_cont(result->result, MYSQL_WAIT_READ);
        if (status) {
            conn->state = CONN_STATE_RESULT_FREEING;
            dthread->yield();
        } else {
            conn->state = CONN_STATE_RESULT_FREED;
            return;
        }
    }
}

static void mariadb_dthread_handle_result(mariadb_conn_t *conn, mariadb_result_t *result)
{
    assert(conn);
    assert(result);
    result->result = mysql_use_result(&conn->mysql);
    if (!result->result) return;
    result->n_fields = mysql_num_fields(result->result);
    result->fields = monkey->mem_alloc(sizeof(char *) * result->n_fields);
    MYSQL_FIELD *fields = mysql_fetch_fields(result->result);
    unsigned int i;
    for (i = 0; i < result->n_fields; ++i) {
        (result->fields)[i] = monkey->str_dup(fields[i].name);
    }
}

/*
 * @METHOD_NAME: query
 * @METHOD_DESC: Send a new query to a MariaDB connection and wait for result.
 * @METHOD_PROTO: mariadb_result_t *query(mariadb_conn_t *conn, const char *query_str)
 * @METHOD_PARAM: conn The MariaDB connection handle.
 * @METHOD_PARAM: query_str The SQL statement string of this query.
 * @METHOD_RETURN: result of the given query on success(result may be NULL), or NULL on failure.
 */
mariadb_result_t *mariadb_dthread_query(mariadb_conn_t *conn, const char *query_str)
{
    assert(conn);
    assert(query_str);
    mariadb_result_t *result = NULL;
    int error;
    int status = mysql_real_query_start(&error, &conn->mysql, query_str,
            strlen(query_str));
    if (status) {
        conn->state = CONN_STATE_QUERYING;
        dthread->yield();
    } else {
        if (error) {
            msg->err("[FD %i] MariaDB Query Error: %s", conn->fd,
                     mysql_error(&conn->mysql));
            return NULL;
        } else {
            conn->state = CONN_STATE_QUERIED;
            // handle result
            result = mariadb_dthread_result_create();
            mariadb_dthread_handle_result(conn, result);
            if (!result->result) {
                monkey->mem_free(result);
                return NULL;
            } else {
                return result;
            }
        }
    }

    while (1) {
        if (conn->state == CONN_STATE_ERROR) {
            msg->err("[FD %i] MariaDB Query Error: %s", conn->fd,
                     mysql_error(&conn->mysql));
            conn->is_pooled = 0;
            mariadb_dthread_disconnect(conn);
            return NULL;
        }

        status = mysql_real_query_cont(&error, &conn->mysql, MYSQL_WAIT_READ);
        if (status) {
            conn->state = CONN_STATE_QUERYING;
            dthread->yield();
        } else {
            if (error) {
                msg->err("[FD %i] MariaDB Query Error: %s", conn->fd,
                         mysql_error(&conn->mysql));
                return NULL;
            } else {
                conn->state = CONN_STATE_QUERIED;
                // handle result
                result = mariadb_dthread_result_create();
                mariadb_dthread_handle_result(conn, result);
                if (!result->result) {
                    monkey->mem_free(result);
                    return NULL;
                } else {
                    return result;
                }
            }
        }
    }
}

/*
 * @METHOD_NAME: get_row
 * @METHOD_DESC: Fetch result from a previous issued query.
 * @METHOD_PROTO: char **get_row(mariadb_conn_t *conn, mariadb_result_t *result, int *error)
 * @METHOD_PARAM: conn The MariaDB connection handle.
 * @METHOD_PARAM: result The result of a previous SQL query.
 * @METHOD_PARAM: error Used to determine whether errors occur when `mariadb->get_row` return NULL.
 * @METHOD_RETURN: next row of the given query result on success, there're two
 * situation will return NULL, use `error` to determine: if `*error` is 0 it means
 * all the rows are consumed, otherwise some errors occur.
 */
char **mariadb_dthread_get_row(mariadb_conn_t *conn, mariadb_result_t *result, int *error)
{
    assert(conn);
    assert(result);
    MYSQL_ROW row;
    int status = mysql_fetch_row_start(&row, result->result);
    if (status) {
        conn->state = CONN_STATE_ROW_FETCHING;
        dthread->yield();
    } else {
        conn->state = CONN_STATE_ROW_FETCHED;
        if (!row) {
            if (mysql_errno(&conn->mysql) > 0) {
                msg->err("[FD %i] MariaDB Fetch Result Row Error: %s", conn->fd,
                         mysql_error(&conn->mysql));
                *error = 1;
            } else {
                *error = 0;
            }
            mariadb_dthread_result_free(conn, result);
            return NULL;
        }
        return row;
    }

    while (1) {
        if (conn->state == CONN_STATE_ERROR) {
            msg->err("[FD %i] MariaDB Query Error: %s", conn->fd,
                     mysql_error(&conn->mysql));
            mariadb_dthread_result_free(conn, result);
            conn->is_pooled = 0;
            mariadb_dthread_disconnect(conn);
            *error = 1;
            return NULL;
        }

        status = mysql_fetch_row_cont(&row, result->result, MYSQL_WAIT_READ);
        if (status) {
            conn->state = CONN_STATE_ROW_FETCHING;
            dthread->yield();
        } else {
            conn->state = CONN_STATE_ROW_FETCHED;
            if (!row) {
                if (mysql_errno(&conn->mysql) > 0) {
                    msg->err("[FD %i] MariaDB Fetch Result Row Error: %s", conn->fd,
                             mysql_error(&conn->mysql));
                    *error = 1;
                } else {
                    *error = 0;
                }
                mariadb_dthread_result_free(conn, result);
                return NULL;
            }
            return row;
        }
    }
}

/*
 * @METHOD_NAME: disconnect
 * @METHOD_DESC: Disconnect a previous opened connection and release all the resource with it.
 * @METHOD_PROTO: void disconnect(mariadb_conn_t *conn)
 * @METHOD_PARAM: conn The MariaDB connection handle, it must be a valid, open connection.
 * @METHOD_RETURN: None.
 */
void mariadb_dthread_disconnect(mariadb_conn_t *conn)
{
    assert(conn);
    if (conn->is_pooled) {
        mariadb_dthread_pool_reclaim_conn(conn);
    } else {
        if (conn->state != CONN_STATE_CLOSED) {
            mysql_close(&conn->mysql);
        }
        mk_list_del(&conn->_head);
        mariadb_conn_free(conn);
    }
}

/*
 * @METHOD_NAME: get_fields
 * @METHOD_DESC: Get fields of a given SQL query result.
 * @METHOD_PROTO: char **get_fields(mariadb_result_t *result)
 * @METHOD_PARAM: result The result of a previous SQL query.
 * @METHOD_RETURN: A string array contains fields of a result.
 */
char **mariadb_dthread_get_fields(mariadb_result_t *result)
{
    assert(result);
    return result->fields;
}

/*
 * @METHOD_NAME: get_field_num
 * @METHOD_DESC: Get field numbers of a given SQL query result.
 * @METHOD_PROTO: int get_field_num(mariadb_result_t *result)
 * @METHOD_PARAM: result The result of a previous SQL query.
 * @METHOD_RETURN: Size of the fields array of a result.
 */
int mariadb_dthread_get_field_num(mariadb_result_t *result)
{
    assert(result);
    return result->n_fields;
}

int mariadb_dthread_on_read(int fd, void *data)
{
    (void) data;
    msg->info("[FD %i] MariaDB Connection Handler / read", fd);

    mariadb_conn_t *conn = mariadb_get_conn(fd);
    if (!conn) {
        msg->err("[fd %i] Error: MariaDB Connection Not Found", fd);
        return DUDA_EVENT_CLOSE;
    }
    dthread->resume(conn->dthread_id);
    return DUDA_EVENT_OWNED;
}

int mariadb_dthread_on_write(int fd, void *data)
{
    (void) data;
    msg->info("[FD %i] MariaDB Connection Hander / write", fd);

    mariadb_conn_t *conn = mariadb_get_conn(fd);
    if (!conn) {
        msg->err("[fd %i] Error: MariaDB Connection Not Found", fd);
        return DUDA_EVENT_CLOSE;
    }
    dthread->resume(conn->dthread_id);
    return DUDA_EVENT_OWNED;
}

int mariadb_dthread_on_error(int fd, void *data)
{
    (void) data;
    msg->info("[FD %i] MariaDB Connection Handler / error", fd);

    mariadb_conn_t *conn = mariadb_get_conn(fd);
    if (!conn) {
        msg->err("[fd %i] Error: MariaDB Connection Not Found", fd);
        return DUDA_EVENT_CLOSE;
    }
    conn->state = CONN_STATE_ERROR;
    dthread->resume(conn->dthread_id);
    return DUDA_EVENT_OWNED;
}

int mariadb_dthread_on_close(int fd, void *data)
{
    (void) data;
    msg->info("[FD %i] MariaDB Connection Handler / close", fd);

    mariadb_conn_t *conn = mariadb_get_conn(fd);
    if (!conn) {
        msg->err("[fd %i] Error: MariaDB Connection Not Found", fd);
        return DUDA_EVENT_CLOSE;
    }
    conn->state = CONN_STATE_ERROR;
    dthread->resume(conn->dthread_id);
    return DUDA_EVENT_CLOSE;
}

int mariadb_dthread_on_timeout(int fd, void *data)
{
    (void) data;
    msg->info("[FD %i] MariaDB Connection Handler / timeout", fd);

    mariadb_conn_t *conn = mariadb_get_conn(fd);
    if (!conn) {
        msg->err("[fd %i] Error: MariaDB Connection Not Found", fd);
        return DUDA_EVENT_CLOSE;
    }
    conn->state = CONN_STATE_ERROR;
    dthread->resume(conn->dthread_id);
    return DUDA_EVENT_OWNED;
}
