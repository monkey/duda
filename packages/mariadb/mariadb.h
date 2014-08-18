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

#ifndef DUDA_PACKAGE_MARIADB_H
#define DUDA_PACKAGE_MARIADB_H

#ifndef _mysql_com_h
#define CLIENT_FOUND_ROWS	    2	        /* Found instead of affected rows */
#define CLIENT_NO_SCHEMA	    16	        /* Don't allow database.table.column */
#define CLIENT_COMPRESS		    32	        /* Can use compression protocol */
#define CLIENT_ODBC		        64	        /* Odbc client */
#define CLIENT_LOCAL_FILES	    128	        /* Can use LOAD DATA LOCAL */
#define CLIENT_IGNORE_SPACE	    256	        /* Ignore spaces before '(' */
#define CLIENT_INTERACTIVE	    1024	    /* This is an interactive client */
#define CLIENT_SSL              2048	    /* Switch to SSL after handshake */
#define CLIENT_IGNORE_SIGPIPE   4096        /* IGNORE sigpipes */
#define CLIENT_MULTI_STATEMENTS (1UL << 16) /* Enable/disable multi-stmt support */
#define CLIENT_MULTI_RESULTS    (1UL << 17) /* Enable/disable multi-results */
#define CLIENT_REMEMBER_OPTIONS (1UL << 31)
#endif

#include "common.h"
#include "query.h"
#include "connection.h"
#include "dthread.h"

duda_global_t mariadb_conn_list;

typedef struct duda_api_mariadb {
    mariadb_conn_t *(*create_conn)(duda_request_t *, const char *, const char *,
                                 const char *, const char *, unsigned int,
                                 const char *, unsigned long);
    int (*create_pool)(duda_global_t *, int , int, const char *, const char *,
                        const char *, const char *, unsigned int, const char *,
                        unsigned long);
    void (*set_ssl)(mariadb_conn_t *, const char *, const char *, const char*,
                    const char *, const char *);
    int (*pool_set_ssl)(duda_global_t *, const char *, const char *, const char*,
                        const char *, const char *);
    mariadb_conn_t *(*pool_get_conn_async)(duda_global_t *, duda_request_t *, mariadb_connect_cb *);
    int (*connect_async)(mariadb_conn_t *, mariadb_connect_cb *);
    void (*disconnect_async)(mariadb_conn_t *, mariadb_disconnect_cb *);
    unsigned long (*escape)(mariadb_conn_t *, char *, const char *, unsigned long);
    int (*query_async)(mariadb_conn_t *, const char *, mariadb_query_result_cb *,
                 mariadb_query_row_cb *, mariadb_query_end_cb *, void *);
    void (*abort_async)(mariadb_query_t *);
    int (*connect)(mariadb_conn_t *);
    mariadb_result_t *(*query)(mariadb_conn_t *conn, const char *);
    char **(*get_row)(mariadb_conn_t *, mariadb_result_t *, int *);
    void (*disconnect)(mariadb_conn_t *conn);
    char **(*get_fields)(mariadb_result_t *);
    int (*get_field_num)(mariadb_result_t *);
    mariadb_conn_t *(*pool_get_conn)(duda_global_t *);
} mariadb_object_t;

mariadb_object_t *mariadb;

unsigned long mariadb_real_escape_string(mariadb_conn_t *conn, char *to,
                                         const char *from, unsigned long length);

int mariadb_on_read(int fd, void *data);
int mariadb_on_write(int fd, void *data);
int mariadb_on_error(int fd, void *data);
int mariadb_on_close(int fd, void *data);
int mariadb_on_timeout(int fd, void *data);

#endif
