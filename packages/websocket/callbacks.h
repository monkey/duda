/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda Framework
 *  --------------
 *  Copyright (C) 2012, Eduardo Silva P. <edsiper@gmail.com>
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
#include "request.h"

#ifndef WEBSOCKET_CALLBACKS_H
#define WEBSOCKET_CALLBACKS_H

/* Event callback codes */
#define WS_ON_OPEN           0x0
#define WS_ON_MESSAGE        0x1
#define WS_ON_ERROR          0x2
#define WS_ON_CLOSE          0x3
#define WS_ON_TIMEOUT        0x4

/* Struct to set the callbacks from duda_main() */
struct ws_callbacks_t {
    void (*on_open)   (duda_request_t *, ws_request_t *);
    void (*on_message)(duda_request_t *, ws_request_t *);
    void (*on_error)  (duda_request_t *, ws_request_t *);
    void (*on_close)  (duda_request_t *, ws_request_t *);
    void (*on_timeout)(duda_request_t *, ws_request_t *);
};

int ws_set_callback(int type, void (*callback) (duda_request_t *, struct ws_request *));
struct ws_callbacks_t *ws_callbacks;

#endif
