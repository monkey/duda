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

#ifndef MARIADB_ASYNC_H
#define MARIADB_ASYNC_H

int mariadb_async_handle_connect(mariadb_conn_t *conn, mariadb_connect_cb *cb);
void mariadb_async_handle_query(mariadb_conn_t *conn);
void mariadb_async_handle_result(mariadb_conn_t *conn);
void mariadb_async_handle_row(mariadb_conn_t *conn);
void mariadb_async_handle_result_free(mariadb_conn_t *conn);
void mariadb_async_handle_next_result(mariadb_conn_t *conn);
void mariadb_async_handle_disconnect(mariadb_conn_t *conn, mariadb_disconnect_cb *cb);
void mariadb_async_handle_release(mariadb_conn_t* conn, int status);

int mariadb_async_handle_add_query(mariadb_conn_t *conn, const char *query_str,
                                   mariadb_query_result_cb *result_cb,
                                   mariadb_query_row_cb *row_cb,
                                   mariadb_query_end_cb *end_cb, void *privdata);

#endif
