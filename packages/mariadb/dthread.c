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
                mk_list_del(&conn->_head);
                mariadb_conn_free(conn);
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
                    mk_list_del(&conn->_head);
                    mariadb_conn_free(conn);
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
            mk_list_del(&conn->_head);
            mariadb_conn_free(conn);
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
            mk_list_del(&conn->_head);
            mariadb_conn_free(conn);
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

void mariadb_dthread_disconnect(mariadb_conn_t *conn)
{
    assert(conn);
    mysql_close(&conn->mysql);
    mk_list_del(&conn->_head);
    mariadb_conn_free(conn);
}

char **mariadb_dthread_get_fields(mariadb_result_t *result)
{
    assert(result);
    return result->fields;
}

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