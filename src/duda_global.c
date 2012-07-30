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

#include "MKPlugin.h"
#include "duda_global.h"

#include "webservice.h"

void duda_global_init(duda_global_t global, void *(*callback)())
{
   /* Make sure the developer has initialized variables from duda_main() */
    if (getpid() != syscall(__NR_gettid)) {
        /* FIXME: error handler */
        monkey->_error(MK_ERR,
                       "Duda: You can only define global vars inside duda_main()");
        exit(EXIT_FAILURE);
    }
    pthread_key_create(&global.key, NULL);
    global.callback = callback;
    mk_list_add(&global._head, &_duda_global_dist);
}

int duda_global_set(duda_global_t global, const void *data)
{
    return pthread_setspecific(global.key, data);
}

void *duda_global_get(duda_global_t global)
{
    return pthread_getspecific(global.key);
}

struct duda_api_global *duda_global_object()
{
    struct duda_api_global *obj;

    obj = mk_api->mem_alloc(sizeof(struct duda_api_global));
    obj->init = duda_global_init;
    obj->set   = duda_global_set;
    obj->get   = duda_global_get;

    return obj;
}
