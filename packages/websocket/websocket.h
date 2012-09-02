/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Daemon
 *  ------------------
 *  Copyright (C) 2012, Eduardo Silva P.
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

#ifndef DUDA_PACKAGE_WEBSOCKETS_H
#define DUDA_PACKAGE_WEBSOCKETS_H

#include "duda_api.h"
#include "request.h"

struct duda_api_websockets {
    int (*handshake) (duda_request_t *,
                      void (*cb_read)   (duda_request_t *, ws_request_t *),
                      void (*cb_write)  (duda_request_t *, ws_request_t *),
                      void (*cb_error)  (duda_request_t *, ws_request_t *),
                      void (*cb_close)  (duda_request_t *, ws_request_t *),
                      void (*cb_timeout)(duda_request_t *, ws_request_t *));
    int (*write) (struct ws_request *, unsigned int, unsigned char *, uint64_t);
};

int ws_handshake(duda_request_t *dr,
                 void (*cb_read)   (duda_request_t *, struct ws_request *),
                 void (*cb_write)  (duda_request_t *, struct ws_request *),
                 void (*cb_error)  (duda_request_t *, struct ws_request *),
                 void (*cb_close)  (duda_request_t *, struct ws_request *),
                 void (*cb_timeout)(duda_request_t *, struct ws_request *));

int ws_send_data(int sockfd,
                unsigned int fin,
                unsigned int rsv1,
                unsigned int rsv2,
                unsigned int rsv3,
                unsigned int opcode,
                uint64_t payload_len,
                unsigned char *payload_data);

int ws_write(struct ws_request *wr, unsigned int code, unsigned char *data, uint64_t len);

/* API Object */
struct duda_api_websockets *websocket;

#endif

