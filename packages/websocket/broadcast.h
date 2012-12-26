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

#include "duda_package.h"

#ifndef WEBSOCKET_BROADCAST_H
#define WEBSOCKET_BROADCAST_H

#define BROADCAST_BUFFER    1024  /* 1Kb */

int ws_broadcast_count;

pthread_mutex_t ws_spawn_mutex;

struct ws_broadcast_worker {
    pid_t pid;
    int channel;
    struct mk_list *conn_list;
};

struct ws_broadcast_t {
    int wid;
    int pipe[2];
    struct mk_list *request_list;
    struct mk_list _head;
};

struct ws_broadcast_frame {
    int source;                            /* origin                 */
    uint64_t len;                          /* data length            */
    int type;                              /* opcode TEXT or BINARY  */
    int channel;                           /* channel                */
    unsigned char data[BROADCAST_BUFFER];  /* data to be broadcasted */
};

struct mk_list ws_broadcast_channels;

void ws_broadcast_worker(void *args);
int ws_broadcast(ws_request_t *wr, unsigned char *data,
                 uint64_t len, int msg_type, int channel);
int ws_broadcast_all(unsigned char *data, uint64_t len,
                     int msg_type, int channel);

int ws_broadcaster();

#endif
