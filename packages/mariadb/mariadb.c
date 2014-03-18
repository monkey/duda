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
 * @METHOD_NAME: escape
 * @METHOD_DESC: Escape special characters in a string for use in an SQL statement.
 * @METHOD_PROTO: unsigned long escape(mariadb_conn_t *conn, char *to, const char *from, unsigned long length)
 * @METHOD_PARAM: conn The MariaDB connection handle, it must be a valid, open connection.
 * @METHOD_PARAM: to The escaped string, the buffer that pointed to by `to' should be at least `length' * 2 + 1 bytes long.
 * @METHOD_PARAM: from The string that is to be escaped.
 * @METHOD_PARAM: length The length of the string pointed to by `from'.
 * @METHOD_RETURN: The length of the string placed into `to', not including the terminating null character.
 */

unsigned long mariadb_real_escape_string(mariadb_conn_t *conn, char *to,
                                         const char *from, unsigned long length)
{
    return mysql_real_escape_string(&conn->mysql, to, from, length);
}

int mariadb_on_read(int fd, void *data)
{
    (void) data;
    msg->info("[FD %i] MariaDB Connection Handler / read", fd);
    mariadb_conn_t *conn = mariadb_get_conn(fd);

    if (!conn) {
        msg->err("[FD %i] Error: MariaDB Connection Not Found", fd);
        return DUDA_EVENT_CLOSE;
    }

    int status;
    switch (conn->state) {
    case CONN_STATE_CONNECTING:
        status = mysql_real_connect_cont(&conn->mysql_ret, &conn->mysql,
                                         MYSQL_WAIT_READ);
        if (!status) {
            if (!conn->mysql_ret) {
                msg->err("[FD %i] MariaDB Connect Error: %s", fd,
                         mysql_error(&conn->mysql));
                if (conn->connect_cb)
                    conn->connect_cb(conn, MARIADB_ERR, conn->dr);
                return DUDA_EVENT_CLOSE;
            }
            conn->state = CONN_STATE_CONNECTED;
            if (conn->connect_cb)
                conn->connect_cb(conn, MARIADB_OK, conn->dr);
            mariadb_async_handle_query(conn);
        }
        break;
    case CONN_STATE_QUERYING:
        status = mysql_real_query_cont(&conn->current_query->error, &conn->mysql,
                                       MYSQL_WAIT_READ);
        if (!status) {
            if (conn->current_query->error) {
                msg->err("[FD %i] MariaDB Query Error: %s", conn->fd,
                         mysql_error(&conn->mysql));
                /* may add a query on error cb to be called here */
                mariadb_query_free(conn->current_query);
                conn->state = CONN_STATE_CONNECTED;
                mariadb_async_handle_query(conn);
            } else {
                conn->state = CONN_STATE_QUERIED;
                mariadb_async_handle_result(conn);
                if (conn->state == CONN_STATE_CONNECTED) {
                    mariadb_async_handle_query(conn);
                }
            }
        }
        break;
    case CONN_STATE_ROW_FETCHING:
        while (1) {
            status = mysql_fetch_row_cont(&conn->current_query->row,
                                          conn->current_query->result,
                                          MYSQL_WAIT_READ);
            if (status) {
                break;
            }
            conn->state = CONN_STATE_ROW_FETCHED;
            if (!conn->current_query->row) {
                if (mysql_errno(&conn->mysql) > 0) {
                    msg->err("[FD %i] MariaDB Fetch Result Row Error: %s", conn->fd,
                             mysql_error(&conn->mysql));
                } else {
                    if (conn->current_query->end_cb) {
                        conn->current_query->end_cb(conn->current_query->privdata,
                                                    conn->current_query,
                                                    conn->dr);
                    }
                }
                mariadb_async_handle_result_free(conn);
                if (conn->state == CONN_STATE_CONNECTED) {
                    mariadb_async_handle_query(conn);
                }
                break;
            }
            if (conn->current_query->row_cb) {
                conn->current_query->row_cb(conn->current_query->privdata,
                                            conn->current_query->n_fields,
                                            conn->current_query->fields,
                                            conn->current_query->row,
                                            conn->dr);
            }
        }
        break;
    case CONN_STATE_RESULT_FREEING:
        status = mysql_free_result_cont(conn->current_query->result, MYSQL_WAIT_READ);
        if (!status) {
            conn->state = CONN_STATE_RESULT_FREED;
            if ((conn->config.client_flag & CLIENT_MULTI_STATEMENTS) == 0) {
                conn->current_query = NULL;
                mariadb_query_free(conn->current_query);
                conn->state = CONN_STATE_CONNECTED;
                mariadb_async_handle_query(conn);
            } else {
                mariadb_async_handle_next_result(conn);
            }
        }
        break;
    case CONN_STATE_NEXT_RESULTING:
        while (1) {
            status = mysql_next_result_cont(&conn->current_query->error,
                                            &conn->mysql, MYSQL_WAIT_READ);
            if (status) {
                break;
            }
            conn->state = CONN_STATE_NEXT_RESULTED;
            if (conn->current_query->error == -1) {
                conn->current_query = NULL;
                mariadb_query_free(conn->current_query);
                conn->state = CONN_STATE_CONNECTED;
                break;
            } else if (conn->current_query->error == 0) {
                mariadb_async_handle_result(conn);
                if (conn->state != CONN_STATE_RESULT_FREED) {
                    break;
                }
            } else {
                msg->err("[FD %i] MariaDB Execute Statement Error: %s", conn->fd,
                         mysql_error(&conn->mysql));
            }
        }
        if (conn->state == CONN_STATE_CONNECTED) {
            mariadb_async_handle_query(conn);
        }
        break;
    default:
        break;
    }
    return DUDA_EVENT_OWNED;
}

int mariadb_on_write(int fd, void *data)
{
    (void) data;
    msg->info("[FD %i] MariaDB Connection Hander / write", fd);

    mariadb_conn_t *conn = mariadb_get_conn(fd);
    if (!conn) {
        msg->err("[fd %i] Error: MariaDB Connection Not Found", fd);
        return DUDA_EVENT_CLOSE;
    }
    return DUDA_EVENT_OWNED;
}

int mariadb_on_error(int fd, void *data)
{
    (void) data;
    msg->info("[FD %i] MariaDB Connection Handler / error", fd);

    mariadb_conn_t *conn = mariadb_get_conn(fd);
    if (!conn) {
        msg->err("[fd %i] Error: MariaDB Connection Not Found", fd);
        return DUDA_EVENT_CLOSE;
    }

    if (conn->current_query) {
        if (conn->current_query->result) {
            mysql_free_result(conn->current_query->result);
        }
    }
    /* error occurs, close the connection no matter it is pooled or not */
    conn->is_pooled = 0;
    mariadb_async_handle_release(conn, MARIADB_ERR);

    return DUDA_EVENT_OWNED;
}

int mariadb_on_close(int fd, void *data)
{
    (void) data;
    msg->info("[FD %i] MariaDB Connection Handler / close", fd);

    mariadb_conn_t *conn = mariadb_get_conn(fd);
    if (!conn) {
        msg->err("[fd %i] Error: MariaDB Connection Not Found", fd);
    } else {
        event->delete(conn->fd);
        if (conn->disconnect_cb) {
            conn->disconnect_cb(conn, MARIADB_ERR, conn->dr);
        }
        mk_list_del(&conn->_head);
        mariadb_conn_free(conn);
    }
    return DUDA_EVENT_CLOSE;
}

int mariadb_on_timeout(int fd, void *data)
{
    (void) data;
    msg->info("[FD %i] MariaDB Connection Handler / timeout", fd);
    return DUDA_EVENT_OWNED;
}
