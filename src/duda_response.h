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

#ifndef DUDA_API_RESPONSE_H
#define DUDA_API_RESPONSE_H

/* RESPONSE object: response->x() */
struct duda_api_response {
    int (*send_headers)  (duda_request_t *);
    int (*headers_off)   (duda_request_t *);
    int (*http_status)   (duda_request_t *, int);
    int (*http_header)   (duda_request_t *, char *);
    int (*http_header_n) (duda_request_t *, char *, int);
    int (*http_content_length) (duda_request_t *, long);
    int (*print)  (duda_request_t *, char *, int);
    int (*printf) (duda_request_t *, const char *, ...);
    int (*sendfile)    (duda_request_t *, char *);
    int (*wait) (duda_request_t *);
    int (*cont) (duda_request_t *);

    #define end(dr, cb) _end(dr, cb); return;
    int (*_end) (duda_request_t *, void (*end_callback) ());

    #define finalize(dr, cb)  _end(dr, cb);
    int (*_finalize) (duda_request_t *, void (*end_callback) ());

    int (*flush)(duda_request_t *dr);

};

int duda_response_send_headers(duda_request_t *dr);
int duda_response_http_status(duda_request_t *dr, int status);
int duda_response_http_header(duda_request_t *dr, char *row);
int duda_response_http_header_n(duda_request_t *dr, char *row, int len);
int duda_response_http_content_length(duda_request_t *dr, long length);
int duda_response_print(duda_request_t *dr, char *raw, int len);
int duda_response_printf(duda_request_t *dr, const char *format, ...);
int duda_response_sendfile(duda_request_t *dr, char *path);
int duda_response_continue(duda_request_t *dr);
int duda_response_wait(duda_request_t *dr);
int duda_response_end(duda_request_t *dr, void (*end_cb) (duda_request_t *));
int duda_response_flush(duda_request_t *dr);

struct duda_api_response *duda_response_object();

#endif
