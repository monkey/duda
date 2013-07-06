/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2001-2013, Eduardo Silva P. <edsiper@gmail.com>
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

#include "unqlite.h"

#ifndef DUDA_PACKAGE_KV_H
#define DUDA_PACKAGE_KV_H

#include "duda_api.h"
#include "webservice.h"

struct duda_api_kv {
    int (*init) (unqlite **);
    int (*store) (unqlite *, const void *, int, const void *, unqlite_int64);
    int (*store_fmt) (unqlite *, const void *, int, const char *, ...);
    int (*append) (unqlite *, const void *, int, const void *, unqlite_int64);
    int (*append_fmt) (unqlite *, const void *, int, const char *, ...);
    int (*fetch) (unqlite *, const void *, int, void *, unqlite_int64 *);
    int (*fetch_callback) (unqlite *, const void *, int,
                           int (*xConsumer)(const void *pData, unsigned int iDatalen,\
                                            void *pUserData),
                           void *);
    int (*delete) (unqlite *, const void *, int);
};

typedef struct duda_api_kv kv_object_t;
typedef unqlite kv_conn_t;

kv_object_t *kv;

/* functions */
int kv_init();

#endif
