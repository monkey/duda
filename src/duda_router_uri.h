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

#ifndef MK_DUDA_ROUTER_URI_H
#define MK_DUDA_ROUTER_URI_H

#define DUDA_ROUTER_URI_MAX   32

struct duda_router_uri_field {
    int type;
    int name_len;
    char *name;
};

/*
 * A context of a parsed URI to perform further lookups
 * with Router rules
 */
struct duda_router_uri {
    int len;
    struct duda_router_uri_field fields[DUDA_ROUTER_URI_MAX];
};

#endif
