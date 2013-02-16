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

#ifndef DUDA_QUEUE_H
#define DUDA_QUEUE_H

#include "duda.h"

#define DUDA_QTYPE_ERROR        -1
#define DUDA_QTYPE_BODY_BUFFER   1
#define DUDA_QTYPE_SENDFILE      2

/* Queue item status */
#define DUDA_QSTATUS_ACTIVE      1
#define DUDA_QSTATUS_INACTIVE    0

struct duda_queue_item {
    short int type;        /* item type */
    short int status;      /* item status */
    void *data;            /* the data it self */

    struct mk_list _head;  /* link to the queue list */
};

struct duda_queue_item *duda_queue_item_new(short int type);
int duda_queue_add(struct duda_queue_item *item, struct mk_list *queue);
struct duda_queue_item *duda_queue_last(struct mk_list *queue);
unsigned long duda_queue_length(struct mk_list *queue);
int duda_queue_flush(duda_request_t *dr);
int duda_queue_free(struct mk_list *queue);

int duda_queue_event_write_callback(int sockfd);
int duda_queue_event_is_registered_write(duda_request_t *dr);
int duda_queue_event_register_write(duda_request_t *dr);
int duda_queue_event_unregister_write(duda_request_t *dr);

#endif
