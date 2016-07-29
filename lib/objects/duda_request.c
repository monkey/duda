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
#include <duda/objects/duda_request.h>

/*
 * @OBJ_NAME: request
 * @OBJ_MENU: Request
 * @OBJ_DESC: The request object provides a set of methods to manipulate the
 * incoming data set in the HTTP request.
 */

struct duda_api_request *duda_request_object()
{
    struct duda_api_request *r;

    r = mk_mem_alloc(sizeof(struct duda_api_request));
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
    r->validate_socket  = duda_request_validate_socket;
    r->validate_request = duda_request_validate_request;

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
    if (dr->request->method != MK_METHOD_POST &&
        dr->request->method != MK_METHOD_PUT) {
        return MK_FALSE;
    }

    if (dr->request->content_length <= 0) {
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
    if (dr->request->method == MK_METHOD_GET) {
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
    if (dr->request->method == MK_METHOD_POST) {
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
    if (dr->request->method == MK_METHOD_HEAD) {
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
    if (dr->request->method == MK_METHOD_PUT) {
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
    if (dr->request->method == MK_METHOD_DELETE) {
        return MK_TRUE;
    }

    return MK_FALSE;
}

/*
 * @METHOD_NAME: is_content_type
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

    if (dr->request->content_type.len <= 0) {
        return MK_FALSE;
    }

    len = strlen(content_type);
    if (len != dr->request->content_type.len) {
        return MK_FALSE;
    }

    if (strncmp(dr->request->content_type.data, content_type, len) != 0) {
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
    if (!dr->session || !dr->request || !dr->request->data.data) {
        *len = 0;
        return NULL;
    }

    n = (size_t) dr->request->data.len;
    data = mk_mem_alloc_z(n + 1);
    if (!data) {
        return NULL;
    }

    memcpy(data, dr->request->data.data, n);
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
    if (!dr->session || !dr->request || !dr->request->data.data) {
        return -1;
    }

    return dr->request->content_length;
}

/*
 * @METHOD_NAME: header_get
 * @METHOD_DESC: It returns a new buffer string with with the value of the given
 *               header key. The new buffer must be freed by the user once it
 *               finish their usage.
 * @METHOD_PROTO: char *header_get(duda_request_t *dr, int name, const char *key, unsigned int len)
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_PARAM: name the Header name reference, it can be one of these: MK_HEADER_ACCEPT, MK_HEADER_ACCEPT_CHARSET, MK_HEADER_ACCEPT_ENCODING, MK_HEADER_ACCEPT_LANGUAGE, MK_HEADER_AUTHORIZATION, MK_HEADER_CACHE_CONTROL, MK_HEADER_COOKIE, MK_HEADER_CONNECTION, MK_HEADER_CONTENT_LENGTH, MK_HEADER_CONTENT_RANGE, MK_HEADER_CONTENT_TYPE, MK_HEADER_HOST, MK_HEADER_IF_MODIFIED_SINCE, MK_HEADER_LAST_MODIFIED, MK_HEADER_LAST_MODIFIED_SINCE, MK_HEADER_RANGE, MK_HEADER_REFERER, MK_HEADER_UPGRADE or MK_HEADER_USER_AGENT. If the desired header is not listed in the given options, use MK_HEADER_OTHER and set the desired header name on the 'key' parameter in lowercase string.
 * @METHOD_PARAM: key Custom/other header key.
 * @METHOD_PARAM: len the key string length.
 * @METHOD_RETURN: Upon successful completion, it returns a new allocated buffer
 * containing the header value. On error it returns NULL.
 */
char *duda_request_header_get(duda_request_t *dr, int name,
                              const char *key, unsigned int len)
{
    char *value;
    int  vsize;
    struct mk_http_header *header;

    /* FIXME */
    return NULL;

    /* Some silly but required validations */
    if (!dr->session || !dr->request || !key) {
        return NULL;
    }

    len = strlen(key);

    /* Loop around every request header
    for (i = 0; i < toc->length; i++) {
        if (strncasecmp(row[i].init, key, len) == 0) {
            vsize = (row[i].end - (len + 1)  - row[i].init);
            value = mk_mem_alloc(vsize + 1);
            strncpy(value, row[i].init + len + 1, vsize);
            value[vsize] = '\0';
            return value;
        }
    }*/
    header = mk_api->header_get(name, dr->request, key, len);
    if (header) {
        value = mk_mem_alloc(header->val.len + 1);
        memcpy(value, header->val.data, header->val.len);
        value[header->val.len - 1] = '\0';
        return value;
    }
    return NULL;
}

/*
 * @METHOD_NAME: header_cmp
 * @METHOD_DESC: It compares the value of a given header key. Use just the Header name
 * without colon.
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_PARAM: name the Header name reference, it can be one of these: MK_HEADER_ACCEPT, MK_HEADER_ACCEPT_CHARSET, MK_HEADER_ACCEPT_ENCODING, MK_HEADER_ACCEPT_LANGUAGE, MK_HEADER_AUTHORIZATION, MK_HEADER_CACHE_CONTROL, MK_HEADER_COOKIE, MK_HEADER_CONNECTION, MK_HEADER_CONTENT_LENGTH, MK_HEADER_CONTENT_RANGE, MK_HEADER_CONTENT_TYPE, MK_HEADER_HOST, MK_HEADER_IF_MODIFIED_SINCE, MK_HEADER_LAST_MODIFIED, MK_HEADER_LAST_MODIFIED_SINCE, MK_HEADER_RANGE, MK_HEADER_REFERER, MK_HEADER_UPGRADE or MK_HEADER_USER_AGENT. If the desired header is not listed in the given options, use MK_HEADER_OTHER and set the desired header name on the 'key' parameter in lowercase string.
 * @METHOD_PARAM: key Custom/other header key (just if MK_HEADER_OTHER is used).
 * @METHOD_PARAM: len the key string length (just if MK_HEADER_OTHER is used).
 * @METHOD_PARAM: val the value of the HTTP header key.
 * @METHOD_RETURN: if the header values matches it returns 0, if they mismatch or the
 *                 header is not found, it returns -1.
 */
int duda_request_header_cmp(duda_request_t *dr,
                            int name, const char *key, unsigned int len,
                            const char *val)
{
    int i;
    int key_len;
    int val_len;

    /* FIXME */
    return 0;

    struct mk_http_header *header;

    /* Some silly but required validations
    if (!dr->session || !dr->request || !key) {
        return -1;
    }
    */

    key_len = strlen(key);
    val_len = strlen(val);

    /* Loop around every request header
    for (i = 0; i < toc->length; i++) {
        if (strncasecmp(row[i].init, key, key_len) == 0 &&
            row[i].init[key_len] == ':') {
            if (strncmp(row[i].init + key_len + 2, val, val_len) == 0) {
                return 0;
            }
    */
    len = strlen(val);

    header = mk_api->header_get(name, dr->request, key, len);
    if (header) {
        if (len != header->val.len) {
            return -1;
        }

        if (strncmp(header->val.data, val, len) == 0) {
            return 0;
        }
    }
    return -1;
}

/*
 * @METHOD_NAME: header_contains
 * @METHOD_DESC: It checks if a given header key contains on its value a
 * specific string.
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_PARAM: name the Header name reference, it can be one of these: MK_HEADER_ACCEPT, MK_HEADER_ACCEPT_CHARSET, MK_HEADER_ACCEPT_ENCODING, MK_HEADER_ACCEPT_LANGUAGE, MK_HEADER_AUTHORIZATION, MK_HEADER_CACHE_CONTROL, MK_HEADER_COOKIE, MK_HEADER_CONNECTION, MK_HEADER_CONTENT_LENGTH, MK_HEADER_CONTENT_RANGE, MK_HEADER_CONTENT_TYPE, MK_HEADER_HOST, MK_HEADER_IF_MODIFIED_SINCE, MK_HEADER_LAST_MODIFIED, MK_HEADER_LAST_MODIFIED_SINCE, MK_HEADER_RANGE, MK_HEADER_REFERER, MK_HEADER_UPGRADE or MK_HEADER_USER_AGENT. If the desired header is not listed in the given options, use MK_HEADER_OTHER and set the desired header name on the 'key' parameter in lowercase string.
 * @METHOD_PARAM: key Custom/other header key (just if MK_HEADER_OTHER is used).
 * @METHOD_PARAM: len the key string length (just if MK_HEADER_OTHER is used).
 * @METHOD_PARAM: val the string to check in the value of the HTTP header key.
 * @METHOD_RETURN: if the header value contains the string it returns a numver >= 0, otherwise it returns -1.
 */
int duda_request_header_contains(duda_request_t *dr, int name,
                                 const char *key, unsigned int len,
                                 const char *val)
{
    int ret;
    struct mk_http_header *header;

    /* Some silly but required validations */
    if (!dr->session || !dr->request || !key) {
        return -1;
    }

    header = mk_api->header_get(name, dr->request, key, len);
    if (header) {
        if (len != header->val.len) {
            return -1;
        }

        ret = mk_api->str_search_n(header->val.data,
                                   val,
                                   MK_STR_INSENSITIVE,
                                   len);
        return ret;
    }
    return -1;
}

/*
 * @METHOD_NAME: validate_socket
 * @METHOD_DESC: Validate if a given socket number belongs to a Duda request.
 * @METHOD_PARAM: socket the socket file descriptor number.
 * @METHOD_RETURN: If there is a Duda request holding the given socket number, it
 * return MK_TRUE, otherwise MK_FALSE.
 */
int duda_request_validate_socket(int socket)
{
    duda_request_t *dr;

    dr = duda_dr_list_get(socket);
    if (dr) {
        return MK_TRUE;
    }

    return MK_FALSE;
}

/*
 * @METHOD_NAME: validate_request
 * @METHOD_DESC: Validate if a given Duda request context is an active connection.
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type.
 * @METHOD_RETURN: If the Duda request context is valid it return MK_TRUE, otherwise
 * MK_FALSE.
 */
int duda_request_validate_request(duda_request_t *dr)
{
    duda_request_t *tmp;

    tmp = duda_dr_list_get(dr->socket);
    if (!tmp) {
        return MK_FALSE;
    }

    if (tmp != dr || tmp->socket != dr->socket) {
        return MK_FALSE;
    }

    return MK_TRUE;
}
