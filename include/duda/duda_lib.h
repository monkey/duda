/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2012-2016, Eduardo Silva P. <eduardo@monkey.io>.
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

#ifndef DUDA_LIB_H
#define DUDA_LIB_H

/* Duda runtime context */
struct duda {
    /* Monkey runtime context */
    mk_ctx_t *monkey;

    /* Setup */
    char *tcp_port;

    /* List head for active web services */
    struct mk_list services;
};

#define DUDA_DEFAULT_PORT   "8080"

struct duda *duda_create();
int duda_destroy(struct duda *duda_ctx);

struct duda_service *duda_service_create(struct duda *d, char *root, char *log,
                                         char *data, char *html, char *service);
int duda_service_destroy(struct duda_service *ds);

int duda_start(struct duda *duda_ctx);
int duda_stop(struct duda *duda_ctx);

#endif
