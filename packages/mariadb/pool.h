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

#ifndef MARIADB_POOL_H
#define MARIADB_POOL_H

#define MARIADB_POOL_DEFAULT_SIZE 1
#define MARIADB_POOL_DEFAULT_MIN_SIZE 2
#define MARIADB_POOL_DEFAULT_MAX_SIZE 8

typedef struct mariadb_pool_config {
    mariadb_conn_config_t conn_config;
    duda_global_t *pool_key;

    int use_ssl;
    int min_size;
    int max_size;

    struct mk_list _head;
} mariadb_pool_config_t;

struct mk_list mariadb_pool_config_list;

typedef struct mariadb_pool {
    int size;
    int free_size;
    mariadb_pool_config_t *config;

    struct mk_list free_conns;
} mariadb_pool_t;

int mariadb_pool_create(duda_global_t *pool_key, int min_size, int max_size,
                        const char *user, const char *password, const char *host,
                        const char *db, unsigned int port, const char *unix_socket,
                        unsigned long client_flag);

int mariadb_pool_set_ssl(duda_global_t *pool_key, const char *key, const char *cert,
                         const char *ca, const char *capath, const char *cipher);

mariadb_conn_t *mariadb_async_pool_get_conn(duda_global_t *pool_key, duda_request_t *dr,
        mariadb_connect_cb *cb);

void mariadb_pool_reclaim_conn(mariadb_conn_t *conn);

mariadb_conn_t *mariadb_dthread_pool_get_conn(duda_global_t *pool_key);
void mariadb_dthread_pool_reclaim_conn(mariadb_conn_t *conn);

#endif
