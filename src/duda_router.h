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

#define DUDA_ROUTER_STATIC     0
#define DUDA_ROUTER_DYNAMIC    1

#define DUDA_ROUTER_FKEY       0
#define DUDA_ROUTER_FVAR       1

#define DUDA_ROUTER_NOTFOUND  -1
#define DUDA_ROUTER_MATCH      0
#define DUDA_ROUTER_REDIRECT   1

/* each router path have a list of router fields */
struct duda_router_field {
    /* Field type: DUDA_ROUTER_FKEY or DUDA_ROUTER_FVAR */
    int type;

    /* The key or variable name */
    int name_len;
    char *name;

    /* Link to the list head on duda_router_path->fields */
    struct mk_list _head;
};

/* A static or dynamic path set by router->map() */
struct duda_router_path {
    /* The type defines if is it a static or dynamic router rule */
    int type;

    /* The pattern to match in the URI */
    char *pattern;
    int pattern_len;

    /*
     * If the pattern was defined with an ending slash (e.g: '/something/')
     * and the incoming request do not contain the last slash, the server
     * may perform a HTTP redirect request based on the value of this flag.
     *
     * This is auto set when creating a new static map. It can be MK_FALSE or
     * MK_TRUE.
     */
    int redirect;

    /* List of fields found on pattern, only used on DYNAMIC routes */
    struct mk_list fields;

    /* The target callback function and it's name */
    char *callback_name;
    void (*callback) (duda_request_t *);

    /*
     * Link to the list head located on struct web_service at
     * duda_webservice.h.
     */
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

    #define root(callback) _root(self, (void *) callback, #callback)
    int (*_root) (struct web_service *,
                  void (*callback)(void *),
                  char *);

    int (*console) (char *);
};

struct duda_api_router *duda_router_object();

int duda_router_redirect(duda_request_t *dr);
int duda_router_is_request_root(struct web_service *ws, duda_request_t *dr);
int duda_router_uri_parse(duda_request_t *dr);
int duda_router_path_lookup(struct web_service *ws,
                            duda_request_t *dr,
                            struct duda_router_path **path);
int duda_router_map(char *pattern,
                    void (*callback)(duda_request_t *),
                    char *callback_name,
                    struct mk_list *list);

#endif
