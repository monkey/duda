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

#ifndef DUDA_REQUEST_H
#define DUDA_REQUEST_H

#include "duda.h"

struct duda_api_request {
    int (*is_data)    (duda_request_t *);
    int (*is_get)     (duda_request_t *);
    int (*is_post)    (duda_request_t *);
    int (*is_head)    (duda_request_t *);
    int (*is_put)     (duda_request_t *);
    int (*is_delete)  (duda_request_t *);
    int (*is_content_type) (duda_request_t *, const char *);
    void *(*get_data) (duda_request_t *, unsigned long *);
    long (*content_length) (duda_request_t *dr);
    char *(*header_get) (duda_request_t *dr, const char *key);
    int (*header_cmp) (duda_request_t *dr, const char *key, const char *val);
    int (*header_contains) (duda_request_t *dr,
                            const char *key,
                            const char *val);
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
char *duda_request_header_get(duda_request_t *dr, const char *key);
int duda_request_header_cmp(duda_request_t *dr, const char *key,
                            const char *val);
int duda_request_header_contains(duda_request_t *dr, const char *key,
                                 const char *val);

#endif
