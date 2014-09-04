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

#include <monkey/mk_api.h>

#include "duda.h"
#include "duda_router.h"

#define ROUTER_REDIR_SIZE 64

/* Router Internals */
/*
 * The STATIC 'way', aims to replace the old map->static_add() method used
 * to route fixed and static URL patterns to specific callbacks, it do not
 * support dynamic parameters.
 */
static int router_add_static(char *pattern,
                             void (*callback)(duda_request_t *),
                             char *callback_name,
                             struct mk_list *list)
{
    struct duda_router_path *path;

    path                = mk_api->mem_alloc(sizeof(struct duda_router_path));
    path->type          = DUDA_ROUTER_STATIC;
    path->pattern       = pattern;
    path->pattern_len   = strlen(pattern);
    path->callback      = callback;
    path->callback_name = callback_name;

    /* Redirect flags, for details please read comments on duda_router.h */
    if (path->pattern[path->pattern_len - 1] == '/') {
        path->redirect = MK_TRUE;
    }
    else {
        path->redirect = MK_FALSE;
    }

    mk_list_add(&path->_head, list);
    return 0;
}

static int router_add_dynamic(char *pattern,
                              void (*callback)(duda_request_t *),
                              char *callback_name,
                              struct mk_list *list)
{
    (void) pattern;
    (void) callback;
    (void) callback_name;
    (void) list;

    return 0;
}

/*
 * Performs a HTTP redirection, the new Location will be the same
 * that arrived but plus and ending slash.
 */
int duda_router_redirect(duda_request_t *dr)
{
    int len;
    int port_redirect;
    int redirect_size;
    char *buf;
    char *host;
    char *location = NULL;
    struct session_request *sr = dr->sr;

    duda_response_http_status(dr, 301);

    if (dr->sr->host.data && sr->port > 0) {
        if (dr->sr->port != mk_api->config->standard_port) {
            port_redirect = dr->sr->port;
        }
    }

    redirect_size = (ROUTER_REDIR_SIZE + dr->sr->host.len +
                     dr->sr->uri_processed.len + 2);

    buf = duda_gc_alloc(dr, redirect_size);
    host = mk_api->pointer_to_buf(dr->sr->host);
    duda_gc_add(dr, host);

    /*
     * Add ending slash to the location string
     */
    location = (char *) duda_gc_alloc(dr, dr->sr->uri_processed.len + 2);
    if (!location) {
        /* FIXME: Need to raise memory problem message somewhere */
        exit(EXIT_FAILURE);
    }

    memcpy(location, dr->sr->uri_processed.data, dr->sr->uri_processed.len);
    location[dr->sr->uri_processed.len]     = '/';
    location[dr->sr->uri_processed.len + 1] = '\0';

    if (port_redirect > 0) {
        len = snprintf(buf, redirect_size,
                       "Location: %s://%s:%i%s",
                       mk_api->config->transport, host, port_redirect, location);
    }
    else {
        len = snprintf(buf, redirect_size,
                       "Location: %s://%s%s",
                       mk_api->config->transport, host, location);
    }

    duda_response_http_header_n(dr, buf, len);
    duda_response_end(dr, NULL);

    return 0;
}


/*
 * Given a specific duda request context, it lookup a router path,
 * if it find a match, the 'path' variable will be set and the
 * function will return zero. If no path is found it will return -1.
 *
 * There is a specific exception where 'path' can be NULL and the return
 * value zero, just when the path_lookup instructed a HTTP redirection.
 */
int duda_router_path_lookup(struct web_service *ws,
                            duda_request_t *dr,
                            struct duda_router_path **path)
{
    int offset = 0;
    struct mk_list *head;
    struct duda_router_path *p;

    *path = NULL;

    /*
     * Check URI offset, it will depends on if the running web service
     * 'owns' the complete HTTP server or not. The offset finally represents
     * the position when the rules starts
     */
    if (ws->is_root == MK_FALSE) {
        offset = ws->name.len + 1;
    }

    /* Go around each router rule */
    mk_list_foreach(head, ws->router_list) {
        p = mk_list_entry(head, struct duda_router_path, _head);

        /* Static processing */
        if (p->type == DUDA_ROUTER_STATIC) {
            /*
               request       rule      redirect
               ================================
               /aaa          /aaa      False
               /aaa/         /aaa      False
               /aaa          /aaa/     True
               /aaa/         /aaa/     True
             */
            int uri_len    = dr->sr->uri_processed.len - offset;
            char *uri_data = dr->sr->uri_processed.data + offset;

            if (strncmp(uri_data, p->pattern, p->pattern_len) == 0) {
                *path = p;
                return DUDA_ROUTER_MATCH;
            }
            else if (p->redirect == MK_TRUE) {
                /* Check if we need to send back a redirection */
                if (strncmp(uri_data, p->pattern, p->pattern_len - 1) == 0 &&
                    uri_len <= p->pattern_len) {
                    duda_router_redirect(dr);
                    return DUDA_ROUTER_REDIRECT;
                }
            }
        }
    }

    return DUDA_ROUTER_NOTFOUND;
}

/* Given a Duda request, determinate if it belongs to a 'root request' */
int duda_router_is_request_root(struct web_service *ws, duda_request_t *dr)
{
    int uri_len;
    int ws_len;
    char *ws_data;
    char *uri_data;
    struct session_request *sr;

    sr = dr->sr;
    ws_len   = ws->fixed_name.len;
    ws_data  = ws->fixed_name.data;
    uri_len  = sr->uri_processed.len;
    uri_data = sr->uri_processed.data;

    if (ws->is_root == MK_TRUE && uri_len == 1) {
        return MK_TRUE;
    }

    if (ws->is_root == MK_FALSE && strncmp(uri_data + 1, ws_data, ws_len) == 0) {
        if (uri_len - 1 == ws_len ||
            ((uri_data[ws_len + 1] == '/') && uri_len <= ws_len + 2)) {
            return MK_TRUE;
        }
    }

    return MK_FALSE;
}

/*
 * @OBJ_NAME: router
 * @OBJ_MENU: Router
 * @OBJ_DESC: The Router object exposes methods to handle routing for requests
 * that arrive. It register certain patterns as rules and associate specific
 * callback functions to be invoked once the core finds a match.
 */


/*
 * @METHOD_NAME: map
 * @METHOD_DESC: It register a Router interface and pattern associating it
 * to a service callback.
 * @METHOD_PROTO: int map(char *pattern, void (*callback)(duda_request_t *))
 * @METHOD_PARAM: pattern the string pattern representing the URL format.
 * @METHOD_PARAM: callback the callback function invoked once the pattern matches.
 * @METHOD_RETURN: Upon successful completion it returns 0, on error returns -1.
 */
int duda_router_map(char *pattern,
                    void (*callback)(duda_request_t *),
                    char *callback_name,
                    struct mk_list *list)
{
    int ret;
    char *tmp;

    if (!pattern || !callback || !list) {
        mk_err("Duda: invalid usage of map method.");
        exit(EXIT_FAILURE);
    }

    tmp = strstr(pattern, ":");
    if (!tmp) {
        ret = router_add_static(pattern, callback, callback_name, list);
    }
    else {
        ret = router_add_dynamic(pattern, callback, callback_name, list);
    }

    return ret;
}

/*
 * @METHOD_NAME: root
 * @METHOD_DESC: It maps the root URL to a specific callback function.
 * @METHOD_PROTO: int root(void (*callback)(duda_request_t *))
 * @METHOD_PARAM: callback the callback function invoked once the pattern matches.
 * @METHOD_RETURN: Upon successful completion it returns 0, on error returns -1.
 */
int duda_router_root(struct web_service *ws,
                     void (*cb) (void *),
                     char *name)
{
    if (!cb) {
        return -1;
    }

    ws->router_root_name = name;
    ws->router_root_cb   = cb;

    return 0;
}

/* WIP: register a path to access Duda console for this service */
int duda_router_console(char *pattern)
{
    (void) pattern;
    return 0;
}

struct duda_api_router *duda_router_object()
{
    struct duda_api_router *r;

    r = mk_api->mem_alloc(sizeof(struct duda_api_router));
    r->_map    = duda_router_map;
    r->_root   = duda_router_root;

    //r->console = duda_router_console;

    return r;
}
