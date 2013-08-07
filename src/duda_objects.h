/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2012-2013, Eduardo Silva P. <edsiper@gmail.com>
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
 * Duda objects are struct passed by references to third party components of
 * the framework, like web services or packages.
 */

#include "mk_macros.h"
#include "duda_global.h"

#ifndef DUDA_OBJECTS_H
#define DUDA_OBJECTS_H

struct mk_list MK_EXPORT duda_map_interfaces;
struct mk_list MK_EXPORT duda_map_urls;
struct mk_list MK_EXPORT duda_global_dist;
struct mk_list MK_EXPORT duda_ws_packages;
struct mk_list MK_EXPORT duda_worker_list;
struct mk_list MK_EXPORT duda_logger_main_list;
struct mk_list MK_EXPORT duda_logger_worker_list;

/*
 * the _setup structure holds function references and flags
 * that modify the behavior of the web service
 */
struct duda_setup _setup;

/* Objects exported to the web service */
struct plugin_api *monkey;
struct duda_api_map *map;
struct duda_api_msg *msg;
struct duda_api_request *request;
struct duda_api_response *response;
struct duda_api_debug *debug;
struct duda_api_event *event;
struct duda_api_console *console;
struct duda_api_logger *logger;
struct duda_api_gc *gc;
struct duda_api_param *param;
struct duda_api_session *session;
struct duda_api_cookie *cookie;
struct duda_api_qs *qs;
struct duda_api_data *data;
struct duda_api_conf *conf;
struct duda_api_fconf *fconf;
struct duda_api_global *global;
struct duda_api_worker *worker;
struct duda_api_xtime *xtime;
struct web_service *self;

/* system headers */
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

/* function that depends on webservice or package specific data */
static inline void duda_global_init(duda_global_t *global,
                                    void *(*callback)(void *),
                                    void *data)
{
    pthread_key_create(&global->key, NULL);
    global->callback = callback;
    global->data     = data;
    mk_list_add(&global->_head, &duda_global_dist);
}

#endif
