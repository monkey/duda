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

#include "duda_api.h"
#include "duda_package.h"
#include "redis.h"
#include "hiredis.h"
#include "async.h"


struct duda_api_redis *get_redis_api()
{
    struct duda_api_redis *redis;

    /* Alloc object */
    redis = malloc(sizeof(struct duda_api_redis));

    /* Map API calls */
    redis->connect               = redis_connect;
    redis->disconnect            = redis_disconnect;  
    redis->attach                = redis_attach;
    redis->setConnectCallback    = redisAsyncSetConnectCallback;
    redis->setDisconnectCallback = redisAsyncSetDisconnectCallback;
    redis->command               = redisAsyncCommand;
    redis->getDudarequest        = redis_request_map;
    redis->free                  = redis_free;
    
    return redis;
}

duda_package_t *duda_package_main(struct duda_api_objects *api)
{
    duda_package_t *dpkg;

    /* Initialize package internals */
    duda_package_init();

    /* Init redis*/
    redis_init();

    /* Package object */    
    dpkg = monkey->mem_alloc(sizeof(duda_package_t));
    dpkg->name    = "redis";
    dpkg->version = "0.1";
    dpkg->api     = get_redis_api();

    return dpkg;
}
