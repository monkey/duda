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

#include "duda_event.h"

/* Event object / API */
struct duda_api_event *duda_event_object()
{
    struct duda_api_event *e;

    e = mk_api->mem_alloc(sizeof(struct duda_api_event));
    e->add    = duda_event_add;
    e->lookup = duda_event_lookup;
    e->delete = duda_event_delete;

    return e;
};

/* Register a new event into Duda events handler */
int duda_event_add(int sockfd, struct duda_request *dr,
                   int (*cb_on_read) (int, struct duda_request *),
                   int (*cb_on_write) (int, struct duda_request *),
                   int (*cb_on_error) (int, struct duda_request *),
                   int (*cb_on_close) (int, struct duda_request *),
                   int (*cb_on_timeout) (int, struct duda_request *))
{
    int mode;
    struct mk_list *event_list;
    struct duda_event_handler *eh;

    eh = mk_api->mem_alloc_z(sizeof(struct duda_event_handler));
    if (!eh) {
        return -1;
    }

    /* set node */
    eh->sockfd = sockfd;
    eh->dr = dr;
    eh->cb_on_read = cb_on_read;
    eh->cb_on_write = cb_on_write;
    eh->cb_on_error = cb_on_error;
    eh->cb_on_close = cb_on_close;
    eh->cb_on_timeout = cb_on_timeout;

    /* Link to thread list */
    event_list = pthread_getspecific(duda_events_list);
    mk_list_add(&eh->_head, event_list);

    /* Register the event with Monkey API */
    if (cb_on_read && cb_on_write) {
        mode = MK_EPOLL_RW;
    }
    else if (cb_on_read) {
        mode = MK_EPOLL_READ;
    }
    else if (cb_on_write) {
        mode = MK_EPOLL_WRITE;
    }
    else {
        mk_err("Duda: Invalid usage of duda_event_add()");
        exit(EXIT_FAILURE);
    }

    mk_api->event_add(sockfd, mode, dr->plugin, dr->cs, dr->sr,
                      MK_EPOLL_LEVEL_TRIGGERED);

    return 0;
}

/* Lookup a specific event_handler through it socket descriptor */
struct duda_event_handler *duda_event_lookup(int sockfd)
{
    struct mk_list *head, *event_list;
    struct duda_event_handler *eh;

    event_list = pthread_getspecific(duda_events_list);
    if (!event_list) {
        return NULL;
    }

    mk_list_foreach(head, event_list) {
        eh = mk_list_entry(head, struct duda_event_handler, _head);
        if (eh->sockfd == sockfd) {
            return eh;
        }
    }

    return NULL;
}

/* Delete an event_handler from the thread list */
int duda_event_delete(int sockfd)
{
    struct mk_list *head, *tmp, *event_list;
    struct duda_event_handler *eh;

    event_list = pthread_getspecific(duda_events_list);
    if (!event_list) {
        return -1;
    }

    mk_list_foreach_safe(head, tmp, event_list) {
        eh = mk_list_entry(head, struct duda_event_handler, _head);
        if (eh->sockfd == sockfd) {
            mk_list_del(&eh->_head);
            mk_api->mem_free(eh);
            return 0;
        }
    }

    return -1;
}
