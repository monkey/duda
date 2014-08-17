/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2013, Zeying Xie <swpdtz at gmail dot com>
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

/*
 * @OBJ_NAME: mariadb
 * @OBJ_MENU: MariaDB
 * @OBJ_DESC: The MariaDB package exposes a set of methods to communicate with
 * the MariaDB relational database.
 * @PKG_HEADER: #include "packages/mariadb/mariadb.h"
 * @PKG_INIT: duda_load_package(mariadb, "mariadb");
 */

#include <mysql.h>
#include "duda_package.h"
#include "mariadb.h"
#include "query_priv.h"
#include "connection_priv.h"
#include "pool.h"
#include "async.h"

mariadb_object_t *get_mariadb_api()
{
    mariadb_object_t *mariadb;

    /* Alloc MariaDB object */
    mariadb = monkey->mem_alloc(sizeof(mariadb_object_t));

    /* Map API calls */
    mariadb->create_conn         = mariadb_conn_create;
    mariadb->create_pool         = mariadb_pool_create;
    mariadb->pool_get_conn_async = mariadb_async_pool_get_conn;
    mariadb->set_ssl             = mariadb_conn_set_ssl;
    mariadb->pool_set_ssl        = mariadb_pool_set_ssl;
    mariadb->connect_async       = mariadb_async_handle_connect;
    mariadb->disconnect_async    = mariadb_async_handle_disconnect;
    mariadb->escape              = mariadb_real_escape_string;
    mariadb->query_async         = mariadb_async_handle_add_query;
    mariadb->abort_async         = mariadb_query_abort;
    mariadb->connect             = mariadb_dthread_connect;
    mariadb->query               = mariadb_dthread_query;
    mariadb->get_row             = mariadb_dthread_get_row;
    mariadb->disconnect          = mariadb_dthread_disconnect;
    mariadb->get_fields          = mariadb_dthread_get_fields;
    mariadb->get_field_num       = mariadb_dthread_get_field_num;
    mariadb->pool_get_conn       = mariadb_dthread_pool_get_conn;

    return mariadb;
}

duda_package_t *duda_package_main()
{
    duda_package_t *dpkg;

    duda_global_init(&mariadb_conn_list, NULL, NULL);
    mk_list_init(&mariadb_pool_config_list);

    dpkg          = monkey->mem_alloc(sizeof(duda_package_t));
    dpkg->name    = "MariaDB";
    dpkg->version = "0.1";
    dpkg->api     = get_mariadb_api();

    return dpkg;
}
