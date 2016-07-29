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

#ifndef DUDA_OBJECT_REQUEST_H
#define DUDA_OBJECT_REQUEST_H

#include <duda/duda.h>

struct duda_api_request {
    int (*is_data)    (duda_request_t *);
    int (*is_get)     (duda_request_t *);
    int (*is_post)    (duda_request_t *);
    int (*is_head)    (duda_request_t *);
    int (*is_put)     (duda_request_t *);
    int (*is_delete)  (duda_request_t *);
    int (*is_content_type) (duda_request_t *, const char *);
    void *(*get_data) (duda_request_t *, unsigned long *);
    long (*content_length) (duda_request_t *);
    char *(*header_get) (duda_request_t *, int, const char *, unsigned int);
    int (*header_cmp) (duda_request_t *, int, const char *, unsigned int,
                       const char *);
    int (*header_contains) (duda_request_t *, int, const char *,
                            unsigned int, const char *);
    int (*validate_socket) (int);
    int (*validate_request) (duda_request_t *);
};

/* functions */
struct duda_api_request *duda_request_object();
int duda_request_is_data(duda_request_t *dr);
int duda_request_is_get(duda_request_t *dr);
int duda_request_is_post(duda_request_t *dr);
int duda_request_is_head(duda_request_t *dr);
int duda_request_is_put(duda_request_t *dr);
int duda_request_is_delete(duda_request_t *dr);
int duda_request_is_content_type(duda_request_t *dr, const char *content_type);
void *duda_request_get_data(duda_request_t *dr, unsigned long *len);
long duda_request_content_length(duda_request_t *dr);
char *duda_request_header_get(duda_request_t *dr, int name, const char *key, unsigned int len);
int duda_request_header_cmp(duda_request_t *dr,
                            int name,
                            const char *key, unsigned int len,
                            const char *val);
int duda_request_header_contains(duda_request_t *dr,
                                 int name,
                                 const char *key, unsigned int len,
                                 const char *val);
int duda_request_validate_socket(int socket);
int duda_request_validate_request(duda_request_t *dr);

#endif
