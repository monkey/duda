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
    obj->set   = duda_global_set;
    obj->get   = duda_global_get;

    return obj;
}
