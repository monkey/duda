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

#ifndef DUDA_API_H
#define DUDA_API_H

#include <pthread.h>

#include "MKPlugin.h"
#include "mk_list.h"
#include "duda.h"
#include "duda_map.h"
#include "duda_event.h"
#include "duda_global.h"
#include "duda_cookie.h"
#include "duda_console.h"
#include "duda_request.h"
#include "duda_response.h"
#include "duda_worker.h"

/* data types */
typedef void * duda_callback_t;

/* The basic web service information */
struct duda_webservice {
    char *app_name;
    char *app_path;
};

/*
 * API objects
 * ===========
 * We provide an useful and easy to understand API for the developer,
 * this is not so easy due to the language and server side nature, if
 * you are not ready to take this red pill, go and run to the NodeJS arms :P
 *
 * Monkey
 * ------
 * Object pointing to the original parent API which expose the Monkey
 * internal, here we have many useful functions to manage strings, memory,
 * configuration files, etc.
 */

/* MONKEY object: monkey->x() */
struct plugin_api *monkey;

/* MSG object: msg->x() */
struct duda_api_msg {
    void (*info) (const char *, ...);
    void (*warn) (const char *, ...);
    void (*err)  (const char *, ...);
    void (*bug)  (const char *, ...);
};

/* DEBUG object: debug->x() */
struct duda_api_debug {
    /* FIXME: pending interfaces... */
    void (*trace) ();
    void (*stacktrace) (void);
};

/*
 * Group all objects in one struct so we can pass this memory space
 * to the web service when it's loaded, then the webservice.h macros
 * do the dirty job...
 */
struct duda_api_objects {
    struct duda_api_main *duda;
    struct plugin_api *monkey;
    struct duda_api_map *map;
    struct duda_api_msg *msg;
    struct duda_api_request *request;
    struct duda_api_response *response;
    struct duda_api_debug *debug;
    struct duda_api_event *event;
    struct duda_api_console *console;
    struct duda_api_global *global;
    struct duda_api_param *param;
    struct duda_api_session *session;
    struct duda_api_cookie *cookie;
    struct duda_api_qs *qs;
    struct duda_api_data *data;
    struct duda_api_conf *conf;
    struct duda_api_fconf *fconf;
    struct duda_api_worker *worker;
    struct duda_api_xtime *xtime;
};

struct duda_api_objects *duda_api_master();

/* MAP specific Duda calls */
struct duda_api_main {
    struct duda_package *(*package_load) (const char *, struct duda_api_objects *);
};


void duda_api_exception(duda_request_t *dr, const char *message);

#endif
