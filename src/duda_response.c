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

#include <stdio.h>
#include <stdarg.h>

#include "duda.h"
#include "duda_api.h"
#include "duda_queue.h"
#include "duda_event.h"
#include "duda_sendfile.h"
#include "duda_body_buffer.h"
#include "duda_response.h"

/*
 * @OBJ_NAME: response
 * @OBJ_DESC: The response object provides a set of methods to manipulate the
 * response to the HTTP client. It helps to compose response headers as well
 * the body content.
 */


/*
 * @METHOD_NAME: send_headers
 * @METHOD_DESC: Send the HTTP response headers
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 */
int duda_response_send_headers(duda_request_t *dr)
{
    int r;

    if (dr->_st_http_headers_sent == MK_TRUE) {
        return -1;
    }

    if (dr->_st_body_writes > 0) {
        /* FIXME: Console error */
        return -1;
    }

    /* Calculate body length */
    dr->sr->headers.content_length = duda_queue_length(&dr->queue_out);

    r = mk_api->header_send(dr->cs->socket, dr->cs, dr->sr);
    if (r != 0) {
        /* FIXME: Console error */
        return -1;
    }

    /* Change flag status */
    dr->_st_http_headers_sent = MK_TRUE;
    return 0;
}

/* set HTTP response status */
int duda_response_http_status(duda_request_t *dr, int status)
{
    mk_api->header_set_http_status(dr->sr, status);
    return 0;
}

/* add a new HTTP response header */
int duda_response_http_header(duda_request_t *dr, char *row)
{
    mk_api->header_add(dr->sr, row, strlen(row));
    return 0;
}

/* add a new HTTP response header */
int duda_response_http_header_n(duda_request_t *dr, char *row, int len)
{
    mk_api->header_add(dr->sr, row, len);
    return 0;
}

/* Compose the body_buffer */
static int _print(duda_request_t *dr, char *raw, int len, int free)
{
    int ret;
    struct duda_body_buffer *body_buffer;
    struct duda_queue_item *item;

    item = duda_queue_last(&dr->queue_out);
    if (!item || item->type != DUDA_QTYPE_BODY_BUFFER) {
        body_buffer = duda_body_buffer_new();
        item = duda_queue_item_new(DUDA_QTYPE_BODY_BUFFER);
        item->data = body_buffer;
        duda_queue_add(item, &dr->queue_out);
    }
    else {
        body_buffer = item->data;
    }

    /* perform realloc if body_write() is called more than body_buffer_size */
    if (body_buffer->buf->iov_idx >= body_buffer->size)  {
        ret = duda_body_buffer_expand(body_buffer);
        if (ret == -1) {
            return -1;
        }
    }

    /* Link data */
    if (free == MK_TRUE) {
        mk_api->iov_add_entry(body_buffer->buf, raw, len, mk_iov_none, MK_IOV_FREE_BUF);
    }
    else {
        mk_api->iov_add_entry(body_buffer->buf, raw, len, mk_iov_none, MK_IOV_NOT_FREE_BUF);
    }

    return 0;
}

/* Enqueue a constant raw buffer */
int duda_response_print(duda_request_t *dr, char *raw, int len)
{
    return _print(dr, raw, len, MK_FALSE);
}

/*
 * Format a new buffer and enqueue it contents, when the queue is flushed all reference
 * to the buffers created here are freed
 */
int duda_response_printf(duda_request_t *dr, const char *format, ...)
{
    int ret;
    int n, size = 128;
    char *p, *np;
    va_list ap;

    if ((p = mk_api->mem_alloc(size)) == NULL) {
        return -1;
    }

    while (1) {
        /* Try to print in the allocated space. */
        va_start(ap, format);
        n = vsnprintf(p, size, format, ap);
        va_end(ap);

        /* If that worked, return the string. */
        if (n > -1 && n < size)
            break;

        size *= 2;  /* twice the old size */
        if ((np = mk_api->mem_realloc(p, size)) == NULL) {
            mk_api->mem_free(p);
            return - 1;
        } else {
            p = np;
        }
    }

    ret = _print(dr, p, n, MK_TRUE);
    if (ret == -1) {
        mk_api->mem_free(p);
    }

    return ret;
}

int duda_response_sendfile(duda_request_t *dr, char *path)
{
    struct duda_sendfile *sf;
    struct duda_queue_item *item;

    sf = duda_sendfile_new(path);

    if (!sf) {
        return -1;
    }

    item = duda_queue_item_new(DUDA_QTYPE_SENDFILE);
    item->data = sf;
    duda_queue_add(item, &dr->queue_out);

    return 0;
}

int duda_response_continue(duda_request_t *dr)
{
    return mk_api->event_socket_change_mode(dr->cs->socket, DUDA_EVENT_WAKEUP, -1);
}

int duda_response_wait(duda_request_t *dr)
{
    /*
     * send the socket to sleep, the behavior is not required as the Monkey 'event
     * states' already have the previous mode and behavior
     */
    return mk_api->event_socket_change_mode(dr->cs->socket, DUDA_EVENT_SLEEP, -1);
}

/* Finalize the response process */
int duda_response_end(duda_request_t *dr, void (*end_cb) (duda_request_t *))
{
    int ret;

    /* Make sure the caller set a valid HTTP response code */
    if (dr->sr->headers.status == 0) {
        duda_api_exception(dr, "Callback did not set the HTTP response status");
        exit(EXIT_FAILURE);
    }

    dr->end_callback = end_cb;
    duda_response_send_headers(dr);
    ret = duda_queue_flush(dr);

    if (ret == 0) {
        duda_service_end(dr);
    }

    return 0;
}

struct duda_api_response *duda_response_object()
{
    struct duda_api_response *obj;

    obj = mk_api->mem_alloc(sizeof(struct duda_api_response));
    obj->send_headers  = duda_response_send_headers;
    obj->http_status   = duda_response_http_status;
    obj->http_header   = duda_response_http_header;
    obj->http_header_n = duda_response_http_header_n;
    obj->print         = duda_response_print;
    obj->printf        = duda_response_printf;
    obj->sendfile      = duda_response_sendfile;
    obj->wait          = duda_response_wait;
    obj->cont          = duda_response_continue;
    obj->_end          = duda_response_end;

    return obj;
}
