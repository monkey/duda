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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "duda_api.h"

#ifndef MK_DUDA_DATA_H
#define MK_DUDA_DATA_H

/* Object API */

struct duda_api_data {
    #define set_path(dir) _set_path(self, dir)
    int (*_set_path) (struct web_service *, const char *);

    #define get_path() _get_path(self)
    const char *(*_get_path) (struct web_service *);

    #define locate(f)      _locate(dr, f)
    char *(*_locate) (duda_request_t *, const char *);
};

struct duda_api_data *duda_data_object();

#endif
