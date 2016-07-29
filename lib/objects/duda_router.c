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

#include <duda/duda.h>
#include <duda/objects/duda_router.h>

#define ROUTER_REDIR_SIZE 64

/* Router Internals */

/*
 * The STATIC 'way', aims to replace the old map->static_add() method used
 * to route fixed and static URL patterns to specific callbacks, it do not
 * support dynamic parameters.
 */
struct duda_router_path *router_new_path(char *pattern,
                                         void (*callback)(duda_request_t *),
                                         char *callback_name,
                                         struct mk_list *list)
{
    struct duda_router_path *path;

    path                = mk_mem_alloc(sizeof(struct duda_router_path));
    path->pattern       = mk_string_dup(pattern);
    path->pattern_len   = strlen(pattern);
    path->callback      = callback;
    path->callback_name = callback_name;
    mk_list_init(&path->fields);

    /* Redirect flags, for details please read comments on duda_router.h */
    if (path->pattern[path->pattern_len - 1] == '/') {
        path->redirect = MK_TRUE;
    }
    else {
        path->redirect = MK_FALSE;
    }

    mk_list_add(&path->_head, list);
    return path;
}

static int router_add_static(char *pattern,
                             void (*callback)(duda_request_t *),
                             struct mk_list *list)
{
    struct duda_router_path *path;

    path = router_new_path(pattern, callback, "", list);
    if (!path) {
        return -1;
    }
    path->type = DUDA_ROUTER_STATIC;

    return 0;
}


/* For a string pattern, split each field and generate a linked list */
static struct mk_list *router_split_pattern(char *pattern)
{
    int end;
    unsigned int len;
    unsigned int val_len;
    unsigned int i = 0;
    char *val;
    struct mk_list *list;
    struct mk_string_line *new;

    list = mk_mem_alloc(sizeof(struct mk_list));
    mk_list_init(list);

    len = strlen(pattern);

    while (i < len) {
        end = mk_string_char_search(pattern + i, '/', len - i);

        if (end >= 0 && end + i < len) {
            end += i;

            if (i == (unsigned int) end) {
                i++;
                continue;
            }

            val = mk_string_copy_substr(pattern, i, end);
            val_len = end - i;
        }
        else {
            val = mk_string_copy_substr(pattern, i, len);
            val_len = len - i;
            end = len;

        }

        /* Alloc node */
        new = mk_mem_alloc(sizeof(struct mk_string_line));
        new->val = val;
        new->len = val_len;

        mk_list_add(&new->_head, list);
        i = end + 1;
    }

    return list;
}

static int router_add_dynamic(char *pattern,
                              void (*callback)(duda_request_t *),
                              struct mk_list *list)
{
    struct mk_list *head;
    struct mk_list *plist;
    struct mk_string_line *entry;
    struct duda_router_path *path;
    struct duda_router_field *field;

    if (!pattern || !callback) {
        return -1;
    }

    if (pattern[0] != '/') {
        return -1;
    }

    plist = router_split_pattern(pattern);
    if (!plist) {
        return -1;
    }

    path = router_new_path(pattern, callback, "", list);
    if (!path) {
        return -1;
    }
    path->type = DUDA_ROUTER_DYNAMIC;

    mk_list_foreach(head, plist) {
        entry = mk_list_entry(head, struct mk_string_line, _head);

        /* allocate memory for the field, lookup the type and register */
        field = mk_mem_alloc(sizeof(struct duda_router_field));
        if (entry->val[0] == ':') {
            field->type = DUDA_ROUTER_FVAR;
        }
        else {
            field->type = DUDA_ROUTER_FKEY;
        }
        field->name = mk_string_dup(entry->val);
        field->name_len = entry->len;

        mk_list_add(&field->_head, &path->fields);
    }
    mk_string_split_free(plist);
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
    struct mk_http_request *sr = dr->request;

    duda_response_http_status(dr, 301);

    if (dr->request->host.data && sr->port > 0) {
        if (dr->request->port != mk_api->config->standard_port) {
            port_redirect = dr->request->port;
        }
    }

    redirect_size = (ROUTER_REDIR_SIZE + dr->request->host.len +
                     dr->request->uri_processed.len + 2);

    buf = duda_gc_alloc(dr, redirect_size);
    host = mk_api->pointer_to_buf(dr->request->host);
    duda_gc_add(dr, host);

    /*
     * Add ending slash to the location string
     */
    location = (char *) duda_gc_alloc(dr, dr->request->uri_processed.len + 2);
    if (!location) {
        /* FIXME: Need to raise memory problem message somewhere */
        exit(EXIT_FAILURE);
    }

    memcpy(location, dr->request->uri_processed.data, dr->request->uri_processed.len);
    location[dr->request->uri_processed.len]     = '/';
    location[dr->request->uri_processed.len + 1] = '\0';

    if (port_redirect > 0) {
        len = snprintf(buf, redirect_size,
                       "Location: %s://%s:%i%s",
                       "FIXME" /*mk_api->config->transport*/, host, port_redirect, location);
    }
    else {
        len = snprintf(buf, redirect_size,
                       "Location: %s://%s%s",
                       "FIXME" /*mk_api->config->transport */, host, location);
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
int duda_router_path_lookup(struct duda_service *ds,
                            mk_request_t *sr,
                            struct duda_router_path **path)
{
    int offset = 0;
    int uri_len;
    char *uri_data;
    struct mk_list *head;
    struct duda_router_path *p;
    duda_request_t *dr = NULL; /* FIXME */
    *path = NULL;

    uri_data = sr->uri_processed.data;
    uri_len  = sr->uri_processed.len;

    /* Go around each router rule */
    mk_list_foreach(head, &ds->router_list) {
        p = mk_list_entry(head, struct duda_router_path, _head);

        /* Static processing */
        if (p->type == DUDA_ROUTER_STATIC) {
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
        else if (p->type == DUDA_ROUTER_DYNAMIC) {
            /* This is a very lazy lookup */
            int i = 0;
            int match = MK_FALSE;
            struct mk_list *dhead;
            struct duda_router_field *field;

            mk_list_foreach(dhead, &p->fields) {
                if (i == dr->router_uri.len) {
                    match = MK_FALSE;
                    break;
                }

                field = mk_list_entry(dhead, struct duda_router_field, _head);
                if (field->type == DUDA_ROUTER_FKEY) {
                    if (strncmp(field->name,
                                dr->router_uri.fields[i].name,
                                field->name_len) == 0 &&
                        field->name_len == dr->router_uri.fields[i].name_len) {
                        match = MK_TRUE;
                    }
                    else {
                        match = MK_FALSE;
                        break;
                    }
                }
                else if (field->type == DUDA_ROUTER_FVAR) {
                    i++;
                    continue;
                }
                if (match == MK_FALSE) {
                    break;
                }
                i++;
            }

            if (match == MK_TRUE) {
                *path = p;
                return DUDA_ROUTER_MATCH;
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
    struct mk_http_request *sr;

    sr = dr->request;
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

static inline int uri_add(struct duda_router_uri *ruri, char *buf, int size)
{
    int idx;

    if (ruri->len >= DUDA_ROUTER_URI_MAX) {
        return -1;
    }

    idx = ruri->len;
    ruri->len++;

    ruri->fields[idx].name = buf;
    ruri->fields[idx].name_len = size;

    return 0;
}


/* For an incoming URI, parse it and store the results on dr->router_uri */
int duda_router_uri_parse(duda_request_t *dr)
{
    int i = 0;
    int len;
    int start = -1;
    int end   = -1;
    int size;
    char *uri;

    len = dr->request->uri_processed.len;
    uri = dr->request->uri_processed.data;
    dr->router_uri.len = 0;

    /* FIXME
    if (dr->ws_root->is_root == MK_FALSE) {
        i = dr->ws_root->name.len + 1;
    }
    else {
        i = 0;
    }
    */

    for (; i < len; i++) {
        if (uri[i] != '/') {
            continue;
        }

        if (start == -1) {
            start = i;
            end = -1;
            continue;
        }
        else if (end == -1) {
            end = i;
            i--;
        }

        size = end - start - 1;
        if (size <= 0) {
            start = -1;
            end   = -1;
            continue;
        }

        uri_add(&dr->router_uri, uri + start + 1, size);
        start = -1;
        end = -1;
    }

    if (start >= 0 && end == -1) {
        end = len;
        size = end - start - 1;
        if (size > 0) {
            uri_add(&dr->router_uri, uri + start + 1, size);
        }
    }


    /*
     * Debug:
     *
    for (i = 0; i < dr->router_uri.len; i++) {
        char buf[20];
        memset(buf, '\0', sizeof(buf));

        strncpy(buf, dr->router_uri.fields[i].name, dr->router_uri.fields[i].name_len);
        printf("RURI=%i buf=%s len=%i\n",
               i,
               buf,
               dr->router_uri.fields[i].name_len);
    }
    */

    return 0;
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
int duda_router_map(struct duda_service *ds,
                    char *pattern,
                    void (*callback)(duda_request_t *))
{
    int ret;
    char *tmp;

    if (!pattern || !callback) {
        mk_err("Duda: invalid usage of map method.");
        return -1;
    }

    tmp = strstr(pattern, ":");
    if (!tmp) {
        ret = router_add_static(pattern, callback, &ds->router_list);
    }
    else {
        ret = router_add_dynamic(pattern, callback, &ds->router_list);
    }

    return ret;
}

struct duda_api_router *duda_router_object()
{
    struct duda_api_router *r;

    r      = mk_mem_alloc(sizeof(struct duda_api_router));
    r->map = duda_router_map;

    return r;
}
