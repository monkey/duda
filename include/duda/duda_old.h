/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2012-2016, Eduardo Silva P. <eduardo@monkey.io>
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

extern struct mk_plugin mk_plugin_duda;

#include "duda_webservice.h"
#include "duda_conf.h"
#include "duda_gc_map.h"
#include "duda_qs_map.h"
#include "duda_router_uri.h"

#ifndef DUDA_MAIN_H
#define DUDA_MAIN_H

#define MAP_WS_APP_NAME   0X00
#define MAP_WS_INTERFACE  0X10
#define MAP_WS_METHOD     0X20
#define MAP_WS_PARAM      0X30
#define MAP_WS_END        0X40

/* Max number of parameters allowed in Duda URI */
#define MAP_WS_MAX_PARAMS 8

/* self identifier for the plugin context inside Monkey internals */
struct mk_plugin *duda_plugin;

pthread_key_t duda_global_events_write;
pthread_key_t duda_global_dr_list;
pthread_mutex_t duda_mutex_thctx;

mk_ptr_t dd_iov_none;

void *duda_load_library(const char *path);
void *duda_load_symbol(void *handle, const char *symbol);
int duda_service_end(duda_request_t *dr);

duda_request_t *duda_dr_list_get(struct mk_http_request *sr);
void duda_worker_init();

#endif
