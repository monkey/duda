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
#include <duda/objects/duda_param.h>
#include <duda/duda_utils.h>


/*
 * @OBJ_NAME: param
 * @OBJ_MENU: Parameters
 * @OBJ_DESC: The param object provides a set of methods to manipulate parameters
 * that comes in the URI per the webservice spec
 */

struct duda_api_param *duda_param_object()
{
    struct duda_api_param *p;

    p = mk_api->mem_alloc(sizeof(struct duda_api_param));
    p->get        = duda_param_get;
    p->get_number = duda_param_get_number;

    return p;
};

static inline int get_param_position(duda_request_t *dr, const char *key)
{
    int i = 0;
    int klen;
    struct mk_list *head;
    struct duda_router_field *path_field;

    if (!key) {
        return -1;
    }

    klen = strlen(key);

    /*
     * Determinate the expected position of the given key in the
     * original Router.
     */
    mk_list_foreach(head, &dr->router_path->fields) {
        path_field = mk_list_entry(head, struct duda_router_field, _head);
        if (path_field->type != DUDA_ROUTER_DYNAMIC) {
            goto next;
        }

        if (klen != (path_field->name_len - 1)) {
            goto next;
        }

        if (strncmp(path_field->name + 1, key, klen) == 0) {
            return i;
        }
    next:
        i++;
    }

    return -1;
}

/*
 * @METHOD_NAME: get
 * @METHOD_DESC: For a given key associated to a dynamic Router path, locate the
 * value in the URL and return a new allocated buffer with it value. The buffer
 * is freed by Duda once the callback finish.
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_PARAM: key the identifier key set on the Router, e.g: 'name'.
 * @METHOD_RETURN: Upon successful completion it returns the new memory buffer with
 * the given value in the URL, the buffer is freed once the callback ends. On error it returns NULL.
 */
char *duda_param_get(duda_request_t *dr, const char *key)
{
    int match;
    char *value = NULL;
    struct duda_router_uri_field *field;

    if (!key) {
        return NULL;
    }

    if (!dr->router_path) {
        return NULL;
    }

    /*
     * If the key exists, perform a copy of the incoming data from the URL
     * and register the new buffer with the garbage collector, do not trust
     * the end user will do that.
     */
    match = get_param_position(dr, key);
    if (match >= 0) {
        field = &dr->router_uri.fields[match];
        value = mk_api->str_copy_substr(field->name, 0, field->name_len);
        if (value) {
            duda_gc_add(dr, value);
        }
        return value;
    }

    /* Bad luck Brian */
    return NULL;
}

/*
 * @METHOD_NAME: get_number
 * @METHOD_DESC: Get the numeric value of the given key. Use only when expecting a numeric value.
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_PARAM: key the identifier key set on the Router, e.g: 'id'.
 * @METHOD_PARAM: res stores the parameter value.
 * @METHOD_RETURN: Upon successful completion it returns 0, on error it returns -1.
 */
int duda_param_get_number(duda_request_t *dr, const char *key, long *res)
{
    int ret;
    int match;
    long number;
    struct duda_router_uri_field *field;

    match = get_param_position(dr, key);
    if (match == -1) {
        return -1;
    }

    field = &dr->router_uri.fields[match];
    ret = duda_utils_strtol(field->name, field->name_len, &number);
    if (ret == -1) {
        return -1;
    }

    *res = number;
    return 0;
}
