/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Daemon
 *  ------------------
 *  Copyright (C) 2001-2012, Eduardo Silva P.
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

#include "MKPlugin.h"
#include "duda.h"
#include "duda_request.h"

/*
 * @OBJ_NAME: request
 * @OBJ_DESC: The request object provides a set of methods to manipulate the
 * incoming data set in the HTTP request.
 */

struct duda_api_request *duda_request_object()
{
    struct duda_api_request *r;

    r = mk_api->mem_alloc(sizeof(struct duda_api_request));
    r->is_data   = duda_request_is_data;
    r->is_get    = duda_request_is_get;
    r->is_post   = duda_request_is_post;
    r->is_head   = duda_request_is_head;
    r->is_put    = duda_request_is_put;
    r->is_delete = duda_request_is_delete;
    r->is_content_type = duda_request_is_content_type;
    r->get_data  = duda_request_get_data;

    return r;
}


/*
 * @METHOD_NAME: is_data
 * @METHOD_DESC: Validate if the request contains a body with content length
 * greater than zero. As well it validate the proper POST or PUT HTTP methods.
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_RETURN: If the request contains data it returns MK_TRUE, otherwise MK_FALSE.
 */
int duda_request_is_data(duda_request_t *dr)
{
    if (dr->sr->method != HTTP_METHOD_POST && dr->sr->method != HTTP_METHOD_PUT) {
        return MK_FALSE;
    }

    if (dr->sr->content_length <= 0) {
        return MK_FALSE;
    }

    return MK_TRUE;
}

/*
 * @METHOD_NAME: is_get
 * @METHOD_DESC: Check if the incoming request is a GET HTTP method
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_RETURN: If the method is GET it returns MK_TRUE, otherwise MK_FALSE.
 */
int duda_request_is_get(duda_request_t *dr)
{
    if (dr->sr->method == HTTP_METHOD_GET) {
        return MK_TRUE;
    }

    return MK_FALSE;
}

/*
 * @METHOD_NAME: is_post
 * @METHOD_DESC: Check if the incoming request is a POST HTTP method
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_RETURN: If the method is POST it returns MK_TRUE, otherwise MK_FALSE.
 */
int duda_request_is_post(duda_request_t *dr)
{
    if (dr->sr->method == HTTP_METHOD_POST) {
        return MK_TRUE;
    }

    return MK_FALSE;
}

/*
 * @METHOD_NAME: is_head
 * @METHOD_DESC: Check if the incoming request is a HEAD HTTP method
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_RETURN: If the method is HEAD it returns MK_TRUE, otherwise MK_FALSE.
 */
int duda_request_is_head(duda_request_t *dr)
{
    if (dr->sr->method == HTTP_METHOD_HEAD) {
        return MK_TRUE;
    }

    return MK_FALSE;
}

/*
 * @METHOD_NAME: is_put
 * @METHOD_DESC: Check if the incoming request is a PUT HTTP method
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_RETURN: If the method is PUT it returns MK_TRUE, otherwise MK_FALSE.
 */
int duda_request_is_put(duda_request_t *dr)
{
    if (dr->sr->method == HTTP_METHOD_PUT) {
        return MK_TRUE;
    }

    return MK_FALSE;
}

/*
 * @METHOD_NAME: is_delete
 * @METHOD_DESC: Check if the incoming request is a DELETE HTTP method
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_RETURN: If the method is DELETE it returns MK_TRUE, otherwise MK_FALSE.
 */
int duda_request_is_delete(duda_request_t *dr)
{
    if (dr->sr->method == HTTP_METHOD_DELETE) {
        return MK_TRUE;
    }

    return MK_FALSE;
}

/*
 * @METHOD_NAME: content_type
 * @METHOD_DESC: Compare the content-type of the request with the given string
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_PARAM: content_type the comparisson string.
 * @METHOD_RETURN: If the content-type is equal, it returns MK_TRUE, otherwise MK_FALSE
 */
int duda_request_is_content_type(duda_request_t *dr, const char *content_type)
{
    int len;

    if (!content_type) {
        return MK_FALSE;
    }

    if (dr->sr->content_type.len <= 0) {
        return MK_FALSE;
    }

    len = strlen(content_type);
    if (len != dr->sr->content_type.len) {
        return MK_FALSE;
    }

    if (strncmp(dr->sr->content_type.data, content_type, len) != 0) {
        return MK_FALSE;
    }

    return MK_TRUE;
}

/*
 * @METHOD_NAME: get_data
 * @METHOD_DESC: It generate a buffer with the data sent in a POST or PUT HTTP method.
 * The new buffer must be freed by the user once it finish their usage.
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_PARAM: len Upon successful completion, the length of the data is
 * stored on this variable.
 * @METHOD_RETURN: Upon successful completion, it returns a new allocated buffer
 * containing the data received. On error it returns NULL.
 */
void *duda_request_get_data(duda_request_t *dr, unsigned long *len)
{
    size_t n;
    void *data;

    /* Some silly but required validations */
    if (!dr->cs || !dr->sr || !dr->sr->data.data) {
        *len = 0;
        return NULL;
    }

    n = (size_t) dr->sr->data.len;
    data = mk_api->mem_alloc_z(n);
    if (!data) {
        return NULL;
    }

    memcpy(data, dr->sr->data.data, n);
    return data;
}
