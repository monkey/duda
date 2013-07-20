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

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <netdb.h>

#include "duda_api.h"
#include "duda_package.h"


#include "ssls.h"

/* Modify the events flags for a registered file descriptor */
static int ssls_event_handler(int efd, int fd, int ctrl, int mode)
{
	int ret;
	struct epoll_event event = {0, {0}};

	event.data.fd = fd;
	event.events = EPOLLERR | EPOLLHUP | EPOLLRDHUP | mode;
	event.events |= EPOLLET;

	/* Add to epoll queue */
	ret = epoll_ctl(efd, ctrl, fd, &event);
	if (ret < 0) {
		perror("epoll_ctl");
		return -1;
	}

	return 0;
}

/* Change a file descriptor mode */
int ssls_event_mod(int efd, int fd, int mode)
{
    return ssls_event_handler(efd, fd, EPOLL_CTL_MOD, mode);
}

/* Register a given file descriptor into the epoll queue */
int ssls_event_add(int efd, int fd)
{
	return ssls_event_handler(efd, fd, EPOLL_CTL_ADD, EPOLLIN);
}

/* Remove an epoll file descriptor from the queue */
int ssls_event_del(int efd, int fd)
{
    return epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
}

static int ssls_create_socket(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

static int ssls_socket_bind(int socket_fd, const struct sockaddr *addr,
                            socklen_t addrlen, int backlog)
{
    int ret;

    ret = bind(socket_fd, addr, addrlen);
    if( ret == -1 ) {
        mk_warn("Error binding socket");
        return ret;
    }

    ret = listen(socket_fd, backlog);
    if(ret == -1 ) {
        mk_warn("Error setting up the listener");
        return -1;
    }

    return ret;
}

int ssls_load_cert(ssls_ctx_t *ctx, char *cert_file)
{
    char err_buf[72];
    int ret;

    ret = x509parse_crtfile(&ctx->srvcert, cert_file);
    if (ret) {
        error_strerror(ret, err_buf, sizeof(err_buf));
        msg->warn("[ssls] Load cert chain '%s' failed: %s",
                  cert_file,
                  err_buf);
        return -1;
    }

    return 0;
}

int ssls_load_key(ssls_ctx_t *ctx, char *key_file)
{
    char err_buf[72];
    int ret;

    ret = x509parse_keyfile(&ctx->rsa, key_file, NULL);
    if (ret < 0) {
        error_strerror(ret, err_buf, sizeof(err_buf));
        msg->warn("[ssls] Load key '%s' failed: %s",
                  key_file,
                  err_buf);
        return -1;
    }

    return 0;
}


static void ssls_ssl_debug(void *ctx, int level, const char *str)
{
    (void) ctx;

    if (level < POLAR_DEBUG_LEVEL) {
        msg->warn("%s", str);
    }
}

/* Register and initialize new connection into the server context */
static ssls_conn_t *ssls_register_connection(ssls_ctx_t *ctx, int fd)
{
    ssl_context *ssl;
    ssls_conn_t *conn;

    conn = monkey->mem_alloc(sizeof(ssls_conn_t));
    if (!conn) {
        msg->err("[SSLS] could not allocate memory for new connection");
        return NULL;
    }

    conn->fd  = fd;
    ssl = &conn->ssl_ctx;

    /* SSL library initialization */
    ssl_init(ssl);
    ssl_set_endpoint(ssl, SSL_IS_SERVER);
    ssl_set_authmode(ssl, SSL_VERIFY_NONE);
    ssl_set_rng(ssl, ctr_drbg_random, &ctx->ctr_drbg);
    ssl_set_dbg(ssl, ssls_ssl_debug, 0);
    ssl_set_ca_chain(ssl, ctx->srvcert.next, NULL, NULL);
    ssl_set_own_cert(ssl, &ctx->srvcert, &ctx->rsa);
    ssl_set_bio(ssl, net_recv, &ctx->fd, net_send, &ctx->fd);

    mk_list_add(&conn->_head, &ctx->conns);
    return conn;
}

static int ssls_remove_connection(ssls_ctx_t *ctx, int fd)
{
    ssls_conn_t *conn;
    struct mk_list *head, *tmp;

    mk_list_foreach_safe(head, tmp, &ctx->conns) {
        conn = mk_list_entry(head, ssls_conn_t, _head);
        if (conn->fd == fd) {
            mk_list_del(&conn->_head);
            ssl_free(&conn->ssl_ctx);
            monkey->mem_free(conn);
            return 0;
        }
    }

    return -1;
}

/*
 * It creates a TCP socket server. We do not use the server core APIs
 * as we don't know which plugin is using as a layer
 */
int ssls_socket_server(int port, char *listen_addr)
{
    int socket_fd = -1;
    int ret;
    char *port_str = 0;
    unsigned long len;
    struct addrinfo hints;
    struct addrinfo *res, *rp;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    monkey->str_build(&port_str, &len, "%d", port);

    ret = getaddrinfo(listen_addr, port_str, &hints, &res);
    monkey->mem_free(port_str);
    if(ret != 0) {
        mk_err("Can't get addr info: %s", gai_strerror(ret));
        return -1;
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        socket_fd = ssls_create_socket(rp->ai_family,
                                       rp->ai_socktype,
                                       rp->ai_protocol);

        if( socket_fd == -1) {
            mk_warn("Error creating server socket, retrying");
            continue;
        }

        monkey->socket_set_tcp_nodelay(socket_fd);
        monkey->socket_reset(socket_fd);
        ret = ssls_socket_bind(socket_fd, rp->ai_addr, rp->ai_addrlen,
                               MK_SOMAXCONN);
        if(ret == -1) {
            mk_err("Cannot listen on %s:%i\n", listen_addr, port);
            continue;
        }
        break;
    }
    freeaddrinfo(res);

    if (rp == NULL)
        return -1;

    return socket_fd;
}

void ssls_set_callbacks(ssls_ctx_t *ctx,
                        void (*cb_read)    (int, char *, int),
                        void (*cb_write)   (int),
                        void (*cb_close)   (int),
                        void (*cb_timeout) (int))
{
    ctx->cb_read    = cb_read;
    ctx->cb_write   = cb_write;
    ctx->cb_close   = cb_close;
    ctx->cb_timeout = cb_timeout;
}


/*
 * starts the server loop waiting for incoming connections, this function
 * should never return.
 */
void ssls_server_loop(ssls_ctx_t *ctx)
{
    int i;
    int fd;
    int ret;
    int remote_fd;
    int size;
    int num_fds;
    int max_events = 128;
    int buf_size = 4096;
    ssls_conn_t *conn;
    struct sockaddr_un address;
    socklen_t socket_size = sizeof(struct sockaddr_in);

    /* per context we have a read-buffer that reads up to 4KB per round */
    void *buf = monkey->mem_alloc(buf_size);
	struct epoll_event *events;

    /* validate the context */
    if (!ctx || ctx->fd <= 0 || ctx->efd <=0) {
        msg->err("[SSLS] Context not initialized properly. Aborting");
        exit(EXIT_FAILURE);
    }

    /* events queue handler */
	size = (max_events * sizeof(struct epoll_event));
	events = (struct epoll_event *) malloc(size);

    while (1) {
        /* wait for events */
        num_fds = epoll_wait(ctx->fd, events, max_events, -1);

        for (i = 0; i < num_fds; i++) {
            fd = events[i].data.fd;

            if (events[i].events & EPOLLIN) {
                /*
                 * Event on socket server: this means a new connection,
                 * the proper way to handle it is to accept the connection
                 * and perform a handshake before to get back the control to
                 * the events callbacks
                 */
                if (fd == ctx->fd) {
                    remote_fd = accept(ctx->fd, (struct sockaddr *) &address,
                                       &socket_size);
                    if (remote_fd < 0) {
                        continue;
                    }

                    /* set the socket to non-blocking mode */
                    monkey->socket_set_nonblocking(remote_fd);

                    /*
                     * When a new connection arrives, we need to register this
                     * connection in our context, initialize some SSL stuff
                     * and as we are in a "ready for read" mode (EPOLLIN), means
                     * that we should start making the SSL handshake.
                     */
                    conn = ssls_register_connection(ctx, remote_fd);
                    ret = ssl_read(&conn->ssl_ctx, buf, buf_size);
                    if (ret < 0) {
                        switch (ret) {
                        case POLARSSL_ERR_NET_WANT_READ:
                        case POLARSSL_ERR_NET_WANT_WRITE:
                            errno = EAGAIN;
                        case POLARSSL_ERR_SSL_CONN_EOF:
                            ret = 0;
                        }
                    }
                    else {
                        /*
                         * This is an unacceptable condition as every new connection
                         * must initiate a handshake and the PolarSSL API must write
                         * back with the ServerHello.
                         */
                        ssls_remove_connection(ctx, remote_fd);
                        close(remote_fd);
                    }
                }
                else {
                    /* FIXME: handle existent connnection, read data */

                }
            }
            else if (events[i].events & EPOLLOUT) {

            }
        }
    }
}


/* Initialize a context of SSL Server */
ssls_ctx_t *ssls_init(int port, char *listen_addr)
{
    int fd;
    int ret;
    ssls_ctx_t *ctx;

    /* create the context */
    ctx = monkey->mem_alloc(sizeof(ssls_ctx_t));
    if (!ctx) {
        msg->err("[SSLS] Memory allocation failed. Aborting");
        exit(EXIT_FAILURE);
    }
    memset(&ctx->srvcert, 0, sizeof(x509_cert));

    /* create a listener socket and bind the given address */
    fd = ssls_socket_server(port, listen_addr);
    if (fd == -1) {
        monkey->mem_free(ctx);
        return NULL;
    }
    ctx->fd = fd;
    monkey->socket_set_nonblocking(ctx->fd);

    /* create an epoll(7) queue */
    ctx->efd = epoll_create(100);
    if (ctx->efd == -1) {
        msg->err("[SSLS] Cannot create epoll queue");
        monkey->mem_free(ctx);
        return NULL;
    }

    /* initialize head list for active connections */
    mk_list_init(&ctx->conns);

    /* Initialize PolarSSL internals */
#ifdef POLARSSL_SSL_CACHE_C
    ssl_cache_init(&ctx->cache);
#endif
    rsa_init(&ctx->rsa, RSA_PKCS_V15, 0);
    entropy_init(&ctx->entropy);
    ret = ctr_drbg_init(&ctx->ctr_drbg,
                        entropy_func, &ctx->entropy,
                        NULL, 0);
    if (ret) {
        msg->err("crt_drbg_init failed: %d", ret);
        monkey->mem_free(ctx);
        return NULL;
    }


    /* register the socket server into the events queue (default EPOLLIN) */
    ssls_event_add(ctx->efd, ctx->fd);
    return ctx;
}
