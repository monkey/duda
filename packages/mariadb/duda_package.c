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
    mariadb->create_conn   = mariadb_conn_create;
    mariadb->create_pool   = mariadb_pool_create;
    mariadb->pool_get_conn = mariadb_pool_get_conn;
    mariadb->set_ssl       = mariadb_conn_set_ssl;
    mariadb->pool_set_ssl  = mariadb_pool_set_ssl;
    mariadb->connect       = mariadb_async_handle_connect;
    mariadb->disconnect    = mariadb_async_handle_disconnect;
    mariadb->escape        = mariadb_real_escape_string;
    mariadb->query         = mariadb_async_handle_add_query;
    mariadb->abort         = mariadb_query_abort;

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
