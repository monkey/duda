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

#ifndef DUDA_SERVICE_INTERNAL_H
#define DUDA_SERVICE_INTERNAL_H

#include "duda.h"

/* Web Service context */
struct duda_service {
    /* Configuration paths */
    char *path_root;            /* Prefix path for web service requirements */
    char *path_log;             /* Directory for logs                 */
    char *path_data;            /* Private data directory             */
    char *path_html;            /* Public HTML files                  */
    char *path_service;         /* Path for web service file (.duda)  */
    void *dl_handle;            /* Service/Shared library handle      */
    struct mk_list _head;       /* link to parent list duda->services */

    /* Specific requirements by API Objects used in duda_main() context */
    struct mk_list router_list; /* list head for routing paths        */
};

#endif
