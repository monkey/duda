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

#include "duda_event.h"

/*
 * @OBJ_NAME: event
 * @OBJ_DESC: The event object provides a set of methods to handle event-driven sockets
 * over the main events loop of the server
 */

/* Event object / API */
struct duda_api_event *duda_event_object()
{
    struct duda_api_event *e;

    e = mk_api->mem_alloc(sizeof(struct duda_api_event));
    e->add    = duda_event_add;
    e->lookup = duda_event_lookup;
    e->mode   = duda_event_mode;
    e->delete = duda_event_delete;

    return e;
};

/*
 * @METHOD_NAME: add
 * @METHOD_DESC: Register a new event into Duda events handler
 * @METHOD_PARAM: sockfd socket file descriptor
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_PARAM: init_mode defines the initial event mode for the file descriptor in question,
 * allowed values are: DUDA_EVENT_READ, DUDA_EVENT_WRITE, DUDA_EVENT_RW, DUDA_EVENT_SLEEP or
 * DUDA_EVENT_WAKEUP.
 * @METHOD_PARAM: behavior defines the events triggered mode to work on. Allowed values are
 * DUDA_EVENT_LEVEL_TRIGGERED OR DUDA_EVENT_EDGE_TRIGGERED. For more details about the behavior
 * refer to the manpage epoll(7).
 * @METHOD_PARAM: cb_on_read callback function for read events or NULL
 * @METHOD_PARAM: cb_on_write callback function for write events or NULL
 * @METHOD_PARAM: cb_on_error callback function for error events or NULL
 * @METHOD_PARAM: cb_on_close callback function for close events or NULL
 * @METHOD_PARAM: cb_on_timeout callback function for timeout events or NULL
 * @METHOD_RETURN: Upon successful completion it returns 0, on error it returns -1
 */
int duda_event_add(int sockfd, struct duda_request *dr,
                   int init_mode, int behavior,
                   int (*cb_on_read) (int, struct duda_request *),
                   int (*cb_on_write) (int, struct duda_request *),
                   int (*cb_on_error) (int, struct duda_request *),
                   int (*cb_on_close) (int, struct duda_request *),
                   int (*cb_on_timeout) (int, struct duda_request *))
{
    struct mk_list *event_list;
    struct duda_event_handler *eh;

    eh = mk_api->mem_alloc_z(sizeof(struct duda_event_handler));
    if (!eh) {
        return -1;
    }

    /* set node */
    eh->sockfd = sockfd;
    eh->dr = dr;
    eh->mode = init_mode;
    eh->behavior = behavior;
    eh->cb_on_read = cb_on_read;
    eh->cb_on_write = cb_on_write;
    eh->cb_on_error = cb_on_error;
    eh->cb_on_close = cb_on_close;
    eh->cb_on_timeout = cb_on_timeout;

    /* Link to thread list */
    event_list = pthread_getspecific(duda_events_list);
    mk_list_add(&eh->_head, event_list);

    if (init_mode < DUDA_EVENT_READ || init_mode > DUDA_EVENT_SLEEP) {
        mk_err("Duda: Invalid usage of duda_event_add()");
        exit(EXIT_FAILURE);
    }

    if (sockfd != dr->socket) {
        mk_api->event_add(sockfd, init_mode, dr->plugin, behavior);
    }
    else {
        mk_api->event_socket_change_mode(sockfd, init_mode, behavior);
    }

    return 0;
}

/*
 * @METHOD_NAME: lookup
 * @METHOD_DESC: Find a specific event_handler through the given file descriptor
 * @METHOD_PARAM: sockfd socket file descriptor
 * @METHOD_RETURN: Upon successful completion it returns the event handler node, if the
 * lookup fails it returns NULL
 */
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

/*
 * @METHOD_NAME: mode
 * @METHOD_DESC: Change the mode and behavior for a given file descriptor registered into
 * the events handler
 * @METHOD_PARAM: sockfd socket file descriptor
 * @METHOD_PARAM: mode defines the new event mode for the file descriptor in question,
 * allowed values are: DUDA_EVENT_READ, DUDA_EVENT_WRITE, DUDA_EVENT_RW, DUDA_EVENT_SLEEP or
 * DUDA_EVENT_WAKEUP.
 * @METHOD_PARAM: behavior defines the events triggered mode to work on. Allowed values are
 * DUDA_EVENT_LEVEL_TRIGGERED OR DUDA_EVENT_EDGE_TRIGGERED. For more details about the behavior
 * refer to the manpage epoll(7).
 * @METHOD_RETURN: Upon successful completion it returns 0, on error it returns -1
 */
int duda_event_mode(int sockfd, int mode, int behavior)
{
    struct duda_event_handler *eh;

    /* We just put to sleep epoll events created through this event object */
    eh = duda_event_lookup(sockfd);
    if (!eh) {
        return -1;
    }

    return mk_api->event_socket_change_mode(sockfd, mode, behavior);
}

/*
 * @METHOD_NAME: delete
 * @METHOD_DESC: Delete a registered event from the events handler
 * @METHOD_PARAM: sockfd socket file descriptor
 * @METHOD_RETURN: Upon successful completion it returns 0, on error it returns -1
 */
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
