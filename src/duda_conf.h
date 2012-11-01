/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Daemon
 *  ------------------
 *  Copyright (C) 2001-2012, Eduardo Silva P. <edsiper@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef MK_DUDA_CONF_H
#define MK_DUDA_CONF_H

#include "duda_api.h"

#define WS_ROOT_URI_LEN   64

char *services_root;
char *packages_root;

struct mk_list services_list;

struct vhost_services {
    struct host *host;
    struct mk_list services;
    struct mk_list _head;
};

/* Web service information */
struct web_service {
    mk_pointer name;
    mk_pointer docroot;

    int  enabled;
    int  url_force_redirect;
    void *handler;

    /* Specifics data when registering the service */
    struct mk_list *map_interfaces;
    struct mk_list *map_urls;

    /* global data */
    struct mk_list *global;

    /* packages loaded by the web service */
    struct mk_list *packages;

    /* node hook */
    struct mk_list _head;
};

int duda_conf_main_init(const char *confdir);
int duda_conf_vhost_init();

/* Object API */

struct duda_api_conf {
    void (*_force_redirect) (struct web_service *);
};

#define force_redirect()  _force_redirect(self)

#endif
