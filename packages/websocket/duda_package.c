/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "duda_api.h"
#include "duda_objects.h"
#include "duda_package.h"

#include "websocket.h"
#include "broadcast.h"
#include "request.h"
#include "callbacks.h"

/* API object */
struct duda_api_websockets *get_websockets_api()
{
    struct duda_api_websockets *ws;

    /* Alloc object */
    ws = malloc(sizeof(struct duda_api_websockets));
    ws->handshake     = ws_handshake;
    ws->write         = ws_write;
    ws->broadcast     = ws_broadcast;
    ws->broadcast_all = ws_broadcast_all;
    ws->broadcaster   = ws_broadcaster;
    ws->set_callback  = ws_set_callback;

    return ws;
}

duda_package_t *duda_package_main(struct duda_api_objects *api)
{
    duda_package_t *dpkg;

    /* Initialize package internals */
    duda_package_init();

    /* Package default configuration */
    ws_config = monkey->mem_alloc(sizeof(struct ws_config_t));
    ws_config->is_broadcast = MK_FALSE;

    /* Initialize callbacks */
    ws_callbacks = monkey->mem_alloc(sizeof(struct ws_callbacks_t));
    memset(ws_callbacks, '\0', sizeof(struct ws_callbacks_t));

    /* Package internals */
    global->init(&ws_request_list, cb_request_list_init);

    /* Package object */
    dpkg = monkey->mem_alloc(sizeof(duda_package_t));
    dpkg->name = "websocket";
    dpkg->version = "0.1";
    dpkg->api = get_websockets_api();

    return dpkg;
}
