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

#ifndef DUDA_PACKAGE_H
#define DUDA_PACKAGE_H

#include <stdlib.h>
#include <sys/syscall.h>

#include "MKPlugin.h"
#include "duda_api.h"
#include "duda_map.h"
#include "duda_global.h"
#include "duda_package.h"
#include "duda_param.h"
#include "duda_session.h"
#include "duda_cookie.h"
#include "duda_data.h"
#include "duda_conf.h"
#include "duda_xtime.h"
#include "duda_console.h"
#include "duda_log.h"
#include "duda_gc.h"
#include "duda_objects.h"
#include "duda_fconf.h"
#include "duda_qs.h"

struct duda_package {
    char *name;
    char *version;
    void *api;
    void *handler;

    struct mk_list _head;
};

/* Data type */
typedef struct duda_package duda_package_t;

/* Hook defines for packages */
duda_package_t MK_EXPORT *_duda_package_main(struct duda_api_objects *dapi);

/* Reference and set Duda API object */
#define duda_package_main()                                             \
    _duda_package_bootstrap(struct duda_api_objects *dapi,              \
                            struct web_service *ws) {                   \
        monkey   = dapi->monkey;                                        \
        map      = dapi->map;                                           \
        msg      = dapi->msg;                                           \
        request  = dapi->request;                                       \
        response = dapi->response;                                      \
        debug    = dapi->debug;                                         \
        event    = dapi->event;                                         \
        gc       = dapi->gc;                                            \
        console  = dapi->console;                                       \
        logger   = dapi->logger;                                        \
        param    = dapi->param;                                         \
        session  = dapi->session;                                       \
        cookie   = dapi->cookie;                                        \
        global   = dapi->global;                                        \
        qs       = dapi->qs;                                            \
        fconf    = dapi->fconf;                                         \
        conf     = dapi->conf;                                          \
        data     = dapi->data;                                          \
        worker   = dapi->worker;                                        \
        xtime    = dapi->xtime;                                         \
        mk_list_init(&duda_map_interfaces);                             \
        mk_list_init(&duda_map_urls);                                   \
        mk_list_init(&duda_global_dist);                                \
        mk_list_init(&duda_ws_packages);                                \
        mk_list_init(&duda_worker_list);                                \
        mk_list_init(&duda_logger_main_list);                           \
        mk_list_init(&duda_logger_worker_list);                         \
                                                                        \
        self = ws;                                                      \
                                                                        \
        return _duda_package_main(dapi);                                \
    }                                                                   \
    duda_package_t *_duda_package_main(struct duda_api_objects *dapi)


/* Define package loader */
duda_package_t *duda_package_load(const char *pkgname,
                                  struct duda_api_objects *api,
                                  struct web_service *ws);

/*
#define duda_load_package(obj, pkg)                             \
    duda_package_t *p = duda_package_load(pkg, dapi, self); \
    obj = p->api;
*/
#endif
