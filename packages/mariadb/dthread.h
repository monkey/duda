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

#ifndef MARIADB_DTHREAD_H
#define MARIADB_DTHREAD_H

#include "connection.h"

typedef struct mariadb_result mariadb_result_t;

int mariadb_dthread_connect(mariadb_conn_t *conn);
mariadb_result_t *mariadb_dthread_query(mariadb_conn_t *conn, const char *query_str);
char **mariadb_dthread_get_row(mariadb_conn_t *conn, mariadb_result_t *result, int *error);
void mariadb_dthread_disconnect(mariadb_conn_t *conn);
char **mariadb_dthread_get_fields(mariadb_result_t *result);
int mariadb_dthread_get_field_num(mariadb_result_t *result);

int mariadb_dthread_on_read(int fd, void *data);
int mariadb_dthread_on_write(int fd, void *data);
int mariadb_dthread_on_error(int fd, void *data);
int mariadb_dthread_on_close(int fd, void *data);
int mariadb_dthread_on_timeout(int fd, void *data);

#endif
