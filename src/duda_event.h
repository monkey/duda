/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Daemon
 *  ------------------
 *  Copyright (C) 2001-2012, Eduardo Silva P.
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

#ifndef DUDA_EVENT_H
#define DUDA_EVENT_H

#include "MKPlugin.h"
#include "duda.h"

/* Thread key to map the event lists per worker */
pthread_key_t duda_events_list;

struct duda_event_handler {
    int sockfd;
    struct duda_request *dr;

    int (*cb_on_read) (int, struct duda_request *);
    int (*cb_on_write) (int, struct duda_request *);
    int (*cb_on_error) (int, struct duda_request *);
    int (*cb_on_close) (int, struct duda_request *);
    int (*cb_on_timeout) (int, struct duda_request *);

    struct mk_list _head;
};

struct duda_api_event {
    int (*add) (int, struct duda_request *,
                int (*cb_on_read) (int, struct duda_request *),
                int (*cb_on_write) (int, struct duda_request *),
                int (*cb_on_error) (int, struct duda_request *),
                int (*cb_on_close) (int, struct duda_request *),
                int (*cb_on_timeout) (int, struct duda_request *));
    struct duda_event_handler *(*lookup) (int);
    int (*delete) (int);
};


/* Export an API object */
struct duda_api_event *duda_event_object();

/* Register a new event into Duda events handler */
int duda_event_add(int sockfd, struct duda_request *dr,
                   int (*cb_on_read) (int sockfd, struct duda_request *dr),
                   int (*cb_on_write) (int sockfd, struct duda_request *dr),
                   int (*cb_on_error) (int sockfd, struct duda_request *dr),
                   int (*cb_on_close) (int sockfd, struct duda_request *dr),
                   int (*cb_on_timeout) (int sockfd, struct duda_request *dr));

/* Lookup a specific event_handler through it socket descriptor */
struct duda_event_handler *duda_event_lookup(int sockfd);

/* Delete an event_handler from the thread list */
int duda_event_delete(int sockfd);

#endif
