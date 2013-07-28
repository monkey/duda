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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "duda_api.h"


#ifndef MK_DUDA_CONF_H
#define MK_DUDA_CONF_H

#define WS_ROOT_URI_LEN   64

char *services_root;  /* Location of web services   */
char *packages_root;  /* Duda packages path         */
char *document_root;  /* Duda Document Root (/ddr)  */

/*
 * List that contains memory references to each service
 * associated to a virtual host.
 */
struct mk_list services_list;

/*
 * List that contains memory references to all web services
 * loaded.
 */
struct mk_list services_loaded;

struct vhost_services {
    struct host *host;
    struct mk_list services;
    struct mk_list _head;
};

int duda_conf_set_confdir(struct web_service *ws, const char *dir);
int duda_conf_set_datadir(struct web_service *ws, const char *dir);

int duda_conf_main_init(const char *confdir);
int duda_conf_vhost_init();
void duda_conf_messages_to(struct web_service *ws);

/* Object API */

struct duda_api_conf {
    #define force_redirect()  _force_redirect(self)
    void (*_force_redirect) (struct web_service *);

    #define bind_messages() _bind_messages(self)
    void (*_bind_messages) (struct web_service *);
};

struct duda_api_conf *duda_conf_object();

#endif
