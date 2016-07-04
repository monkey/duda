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

#include <monkey/mk_api.h>

#ifndef DUDA_DWEBSERVICE_H
#define DUDA_DWEBSERVICE_H

struct duda_setup {
    void (*event_signal_cb) (int fd, uint64_t val);
};

/* Web service information */
struct web_service {
    mk_ptr_t name;       /* web service name                 */
    mk_ptr_t fixed_name; /* service name given by vhost/file */
    mk_ptr_t docroot;    /* document root for static content */
    mk_ptr_t confdir;    /* configuration directory          */
    mk_ptr_t datadir;    /* data store                       */
    mk_ptr_t logdir;     /* directory to store logs          */

    int  enabled;
    int  is_root;
    int  bind_messages;

    void *handler;

    char *router_root_name;
    void (*router_root_cb) (void *);

    struct mk_list *router_list;

    char *dashboard;

    /* global data */
    struct mk_list *global;

    /* pre-worker-loop functions */
    struct mk_list *pre_loop;

    /* workers list */
    struct mk_list *workers;

    /* loggers list */
    struct mk_list *loggers;

    /* packages loaded by the web service */
    struct mk_list *packages;

    /* generic setup things related to the web service and Duda stack */
    struct duda_setup *setup;

    /* node entry associated with services_list */
    struct mk_list _head;

    /* reference to the parent vhost_services entry */
    struct vhost_services *vh_parent;

    /* node entry associated with services_loaded */
    struct mk_list _head_loaded;

    /* Callbacks */
    void (*exit_cb) ();
};

#endif
