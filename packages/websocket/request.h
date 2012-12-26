/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Daemon
 *  ------------------
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

#ifndef WEBSOCKET_REQUEST_H
#define WEBSOCKET_REQUEST_H

#include <stdint.h>
#include "duda_api.h"
#include "mk_macros.h"
#include "mk_list.h"
#include "protocol.h"

struct ws_request
{
    int socket;
    int channel;
    struct duda_request *dr;

    /* callbacks */
    void (*cb_on_open)    (duda_request_t *, struct ws_request *);
    void (*cb_on_message) (duda_request_t *, struct ws_request *);
    void (*cb_on_error)   (duda_request_t *, struct ws_request *);
    void (*cb_on_close)   (duda_request_t *, struct ws_request *);
    void (*cb_on_timeout) (duda_request_t *, struct ws_request *);

    /* Protocol specifics */
    unsigned int  opcode;
    unsigned int  mask;
    unsigned char masking_key[WS_FRAME_MASK_LEN];
    unsigned char *payload;
    uint64_t payload_len;

    /* Client request data */
    struct client_session *cs;
    struct session_request *sr;

    struct mk_list _head;
};

typedef struct ws_request  ws_request_t;

/*
 * We use the Duda internals global object to handle the request list
 * at worker level
 */
duda_global_t ws_request_list;

/* Functions */
void *cb_request_list_init();
void ws_request_init();
struct ws_request *ws_request_create(int socket_fd,
                                     int channel,
                                     struct duda_request *dr,
                                     void (*on_open)   (duda_request_t *, ws_request_t *),
                                     void (*on_message)(duda_request_t *, ws_request_t *),
                                     void (*on_error)  (duda_request_t *, ws_request_t *),
                                     void (*on_close)  (duda_request_t *, ws_request_t *),
                                     void (*on_timeout)(duda_request_t *, ws_request_t *));
void ws_request_add(ws_request_t *pr);
ws_request_t *ws_request_get(int socket);
void ws_request_update(int socket, ws_request_t *wr);
int ws_request_delete(int socket);
void ws_free_request(int sockfd);

#endif
