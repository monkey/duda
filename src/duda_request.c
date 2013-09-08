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

#include "MKPlugin.h"
#include "duda.h"
#include "duda_request.h"

/*
 * @OBJ_NAME: request
 * @OBJ_MENU: Request
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
    r->get_data   = duda_request_get_data;
    r->content_length = duda_request_content_length;
    r->header_get = duda_request_header_get;
    r->header_cmp = duda_request_header_cmp;
    r->header_contains = duda_request_header_contains;

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
    unsigned long len;

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
    *len = n;

    return data;
}

/*
 * @METHOD_NAME: content_length
 * @METHOD_DESC: Get the body length for the request in question.
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_RETURN: Upon successful completion, it returns the body content length.
 */
long duda_request_content_length(duda_request_t *dr)
{
    /* Some silly but required validations */
    if (!dr->cs || !dr->sr || !dr->sr->data.data) {
        return -1;
    }

    return dr->sr->content_length;
}

/*
 * @METHOD_NAME: header_get
 * @METHOD_DESC: It returns a new buffer string with with the value of the given
 *               header key. The new buffer must be freed by the user once it finish
 *               their usage.
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_PARAM: key HTTP header key
 * @METHOD_RETURN: Upon successful completion, it returns a new allocated buffer
 * containing the header value. On error it returns NULL.
 */
char *duda_request_header_get(duda_request_t *dr, const char *key)
{
    int i;
    int len;
    char *value;
    int  vsize;
    struct headers_toc *toc;
    struct header_toc_row *row;

    /* Some silly but required validations */
    if (!dr->cs || !dr->sr || !key) {
        return NULL;
    }

    len = strlen(key);
    toc = &dr->sr->headers_toc;
    row = toc->rows;

    /* Loop around every request header */
    for (i = 0; i < toc->length; i++) {
        /* Compare header key */
        if (strncasecmp(row[i].init, key, len) == 0) {
            /* Create new buffer */
            vsize = (row[i].end - (len + 1)  - row[i].init);
            value = mk_api->mem_alloc(vsize + 1);
            strncpy(value, row[i].init + len + 1, vsize);
            value[vsize] = '\0';
            return value;
        }
    }

    return NULL;
}

/*
 * @METHOD_NAME: header_cmp
 * @METHOD_DESC: It compares the value of a given header key.
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_PARAM: key HTTP header key
 * @METHOD_PARAM: val the value of the HTTP header key.
 * @METHOD_RETURN: if the header values matches it returns 0, if they mismatch or the
 *                 header is not found, it returns -1.
 */
int duda_request_header_cmp(duda_request_t *dr, const char *key, const char *val)
{
    int i;
    int key_len;
    int val_len;
    struct headers_toc *toc;
    struct header_toc_row *row;

    /* Some silly but required validations */
    if (!dr->cs || !dr->sr || !key) {
        return -1;
    }

    key_len = strlen(key);
    val_len = strlen(val);

    toc = &dr->sr->headers_toc;
    row = toc->rows;

    /* Loop around every request header */
    for (i = 0; i < toc->length; i++) {
        /* Compare header key */
        if (strncasecmp(row[i].init, key, key_len) == 0) {
            /* Match the value */
            if (strncmp(row[i].init + key_len + 1, val, val_len) == 0) {
                return 0;
            }
        }
    }

    return -1;
}

/*
 * @METHOD_NAME: header_contains
 * @METHOD_DESC: It checks if a given header key contains on its value a
 * specific string.
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_PARAM: key HTTP header key
 * @METHOD_PARAM: val the string to check in the value of the HTTP header key.
 * @METHOD_RETURN: if the header value contains the string it returns 0, if it's not
 * found it returns -1.
 */
int duda_request_header_contains(duda_request_t *dr,
                                 const char *key, const char *val)
{
    int i;
    int ret;
    int len;
    int key_len;
    struct headers_toc *toc;
    struct header_toc_row *row;

    /* Some silly but required validations */
    if (!dr->cs || !dr->sr || !key) {
        return -1;
    }

    key_len = strlen(key);

    toc = &dr->sr->headers_toc;
    row = toc->rows;

    /* Loop around every request header */
    for (i = 0; i < toc->length; i++) {
        /* Compare header key */
        if (strncasecmp(row[i].init, key, key_len) == 0) {
            /* Match the value */
            len = (row[i].end - row[i].init) - key_len - 1;
            ret = mk_api->str_search_n(row[i].init + key_len + 1,
                                       val,
                                       MK_STR_INSENSITIVE,
                                       len);
            return ret;
        }
    }

    return -1;
}

