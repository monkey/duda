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

#ifndef DUDA_PACKAGE_SSLS_H
#define DUDA_PACKAGE_SSLS_H

#include "duda_api.h"
#include "webservice.h"

#include <polarssl/version.h>
#include <polarssl/error.h>
#include <polarssl/net.h>
#include <polarssl/ssl.h>
#include <polarssl/bignum.h>
#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>
#include <polarssl/certs.h>
#include <polarssl/x509.h>

#ifdef POLARSSL_SSL_CACHE_C
#include <polarssl/ssl_cache.h>
#endif

#ifndef POLAR_DEBUG_LEVEL
#define POLAR_DEBUG_LEVEL 0
#endif

/* Represents a SSL connection into the server */
typedef struct {
    int fd;
    ssl_context ssl_ctx;

    struct mk_list _head;
} ssls_conn_t;

/* A SSL-Server context */
typedef struct {
    int fd;                   /* socket server listening from incoming connections */
    int efd;                  /* epoll file descriptor */
    struct mk_list conns;     /* active connections */

    /* SSL specific data */
    x509_cert srvcert;
    rsa_context rsa;

#ifdef POLARSSL_SSL_CACHE_C
    ssl_cache_context cache;
#endif
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;

    /* callback hooks */
    void (*cb_read)    (int, char *, int);
    void (*cb_write)   (int);
    void (*cb_close)   (int);
    void (*cb_timeout) (int);

} ssls_ctx_t;

/* API structure */
struct duda_api_ssls {
    int (*event_mod) (int, int, int);
    int (*event_add) (int, int);
    int (*event_del) (int, int);
    int (*load_cert) (ssls_ctx_t *, char *);
    int (*load_key)  (ssls_ctx_t *, char *);
    int (*socket_server)  (int, char *);
    void (*set_callbacks) (ssls_ctx_t *,
                           void (*cb_read)    (int, char *, int),
                           void (*cb_write)   (int),
                           void (*cb_close)   (int),
                           void (*cb_timeout) (int));
    void (*server_loop) (ssls_ctx_t *);
    ssls_ctx_t *(*init) (int, char *);
};

/* functions */
int ssls_event_mod(int efd, int fd, int mode);
int ssls_event_add(int efd, int fd);
int ssls_event_del(int efd, int fd);
int ssls_load_cert(ssls_ctx_t *ctx, char *cert_file);
int ssls_load_key(ssls_ctx_t *ctx, char *key_file);
int ssls_socket_server(int port, char *listen_addr);
void ssls_set_callbacks(ssls_ctx_t *ctx,
                        void (*cb_read)    (int, char *, int),
                        void (*cb_write)   (int),
                        void (*cb_close)   (int),
                        void (*cb_timeout) (int));
void ssls_server_loop(ssls_ctx_t *ctx);
ssls_ctx_t *ssls_init(int port, char *listen_addr);

typedef struct duda_api_ssls ssls_object_t;
ssls_object_t *ssls;

#endif
