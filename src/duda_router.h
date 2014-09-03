/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2012-2014, Eduardo Silva P. <eduardo@monkey.io>
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

#include "duda_api.h"

#ifndef MK_DUDA_ROUTER_H
#define MK_DUDA_ROUTER_H

#define DUDA_ROUTER_STATIC    0
#define DUDA_ROUTER_DYNAMIC   1

struct duda_router_rule {
    int type;            /* ROUTER_STATIC or ROUTER_DYNAMIC */
    int pattern_len;
    char *pattern;
    void (*callback) (duda_request_t *);

    struct mk_list _head;
};

/* Object API */
struct duda_api_router {

    /*
     * The map() method allows to set a static or dynamic route,
     * we implement this through a macro as we want to have registered
     * the callback reference as well it name in string format. Also we
     * need to pass the list head as it lives inside the service (library)
     * context.
     *
     * The list head is read as a symbol later inside duda.c .
     */
    #define map(pattern, callback) \
        _map(pattern, callback, #callback, &duda_router_list)
    int (*_map) (char *,
                 void (*callback)(duda_request_t *),
                 char *callback_name, struct mk_list *);

    int (*console) (char *);
};

struct duda_api_router *duda_router_object();

#endif
