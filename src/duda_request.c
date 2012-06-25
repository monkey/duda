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

    return r;
}

/*
 * Validate if the request contains a body with content length
 * greater than zero. As well it validate the HTTP methods
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

/* Check if the request uses GET method */
int duda_request_is_get(duda_request_t *dr)
{
    if (dr->sr->method == HTTP_METHOD_GET) {
        return MK_TRUE;
    }

    return MK_FALSE;
}

/* Check if the request uses POST method */
int duda_request_is_post(duda_request_t *dr)
{
    if (dr->sr->method == HTTP_METHOD_POST) {
        return MK_TRUE;
    }

    return MK_FALSE;
}

/* Check if the request uses HEAD method */
int duda_request_is_head(duda_request_t *dr)
{
    if (dr->sr->method == HTTP_METHOD_HEAD) {
        return MK_TRUE;
    }

    return MK_FALSE;
}

/* Check if the request uses PUT method */
int duda_request_is_put(duda_request_t *dr)
{
    if (dr->sr->method == HTTP_METHOD_PUT) {
        return MK_TRUE;
    }

    return MK_FALSE;
}

/* Check if the request uses DELETE method */
int duda_request_is_delete(duda_request_t *dr)
{
    if (dr->sr->method == HTTP_METHOD_DELETE) {
        return MK_TRUE;
    }

    return MK_FALSE;
}

/* Compare the content type of the request */
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
