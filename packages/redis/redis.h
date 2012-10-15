/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Daemon
 *  ------------------
 *  Copyright (C) 2001-2012, Sourabh Chandak<sourabh3934@gmail.com>
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

#ifndef DUDA_PACKAGE_REDIS_H
#define DUDA_PACKAGE_REDIS_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "hiredis.h"
#include "async.h"

#include "duda_api.h"
#include "webservice.h"

pthread_key_t redis_key;

typedef struct duda_redis {
    
    redisAsyncContext *rc;
    duda_request_t *dr;
    struct mk_list _head_redis_fd;
    
} duda_redis_t;

struct duda_api_redis {

    /* redis functions */
    redisAsyncContext *(*connect) (const char *, int, 
                                   duda_request_t *);
    void (*disconnect) (redisAsyncContext *);
    int (*attach) (redisAsyncContext *, duda_request_t *);
    int (*setConnectCallback) (redisAsyncContext *,
                               void (*)(const redisAsyncContext *, int));
    int (*setDisconnectCallback) (redisAsyncContext *,
                                  void (*)(const redisAsyncContext *, int));
    int (*command) (redisAsyncContext *, 
                    void (*) (redisAsyncContext*, void*, void*), void *, 
                    const char *,...);
    void (*free) (redisAsyncContext *);
    duda_request_t * (*getDudarequest) (const redisAsyncContext *);
};

typedef struct duda_api_redis redis_object_t;

redis_object_t *redis;

redisAsyncContext * redis_connect(const char *ip, int port, duda_request_t *dr);
void redis_disconnect(redisAsyncContext *rc);
int redis_attach(redisAsyncContext *rc, duda_request_t *dr);
int redis_init();
duda_request_t * redis_request_map(const redisAsyncContext *rc);
void redis_free(redisAsyncContext *rc);

void redisAddRead(void *privdata);
void redisDel(void *privdata);
void redisAddWrite(void *privdata);

int redis_read(int fd, struct duda_request *dr);
int redis_write(int fd, struct duda_request *dr);
int redis_error(int fd, struct duda_request *dr);
int redis_close(int fd, struct duda_request *dr);
int redis_timeout(int fd, struct duda_request *dr);

#endif
