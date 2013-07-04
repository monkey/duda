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

#include "MKPlugin.h"
#include "duda_global.h"

#include "webservice.h"


/*
 * @OBJ_NAME: global
 * @OBJ_MENU: Global Worker
 * @OBJ_DESC: Duda stack works in threaded mode depending of the number of workers
 * spawn when started. The global object aims to provide interfaces to save references
 * for data which must be in the scope of the worker. The most common example
 * would be a database connection context.
 */

/*
 * @METHOD_NAME: init
 * @METHOD_DESC: Initialize a specific global key that will be used later in the
 * callbacks. This call MUST be used ONLY inside duda_main().
 * @METHOD_PROTO: void init(duda_global_t *key, void *callback(void) )
 * @METHOD_PARAM: key the global key variable that must be declared globally.
 * @METHOD_PARAM: callback this optional parameter points to a callback function
 * that will be invoked once duda_main() returns and before to enter the main
 * server loop. This callback is useful if you want to initialize some data in the
 * global key before the events start arriving.
 * @METHOD_RETURN: Do not return anything.
 */

/* REF: duda_global_init() is defined inside duda_object.h */

/*
 * @METHOD_NAME: set
 * @METHOD_DESC: Add a new value to the global key.
 * @METHOD_PROTO: int set(duda_global_t key, const void *data)
 * @METHOD_PARAM: key the global key previously initialized by the init() method.
 * @METHOD_PARAM: data the data you want to associate
 * @METHOD_RETURN: Upon successful completion, it returns zero. On error it returns
 * a negative number.
 */

int duda_global_set(duda_global_t global, const void *data)
{
    return pthread_setspecific(global.key, data);
}


/*
 * @METHOD_NAME: get
 * @METHOD_DESC: Retrieve the memory reference associated to the global key
 * @METHOD_PROTO: void *get(duda_global_t key)
 * @METHOD_PARAM: key the global key previously initialized by the init() method.
 * @METHOD_RETURN: Upon successful completion, it returns the memory address associated
 * with the global key. On error it returns NULL.
 */

void *duda_global_get(duda_global_t global)
{
    return pthread_getspecific(global.key);
}

struct duda_api_global *duda_global_object()
{
    struct duda_api_global *obj;

    obj = mk_api->mem_alloc(sizeof(struct duda_api_global));
    obj->init  = duda_global_init;
    obj->set   = duda_global_set;
    obj->get   = duda_global_get;

    return obj;
}
