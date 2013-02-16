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

#ifndef DUDA_QS_MAP_H
#define DUDA_QS_MAP_H

/*
 * We initially define that we will get no more than QS_ENTRIES_SIZE variables
 * in the query string. This is a fixed value to reduce the risk of a memory
 * consumption attack.
 */
#define QS_ENTRIES_SIZE 32

struct duda_qs_entry {
    mk_pointer key;
    mk_pointer value;
};

struct duda_qs_map {
    int count;         /* number of key/values in the query string */
    struct duda_qs_entry entries[QS_ENTRIES_SIZE];
};


#endif
