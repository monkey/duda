/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2013, Zeying Xie <swpdtz at gmail dot com>
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

#ifndef MARIADB_QUERY_H
#define MARIADB_QUERY_H

typedef struct mariadb_query mariadb_query_t;

typedef void (mariadb_query_result_cb)(void *privdata, mariadb_query_t *query,
                                       unsigned long n_fields, char **fields,
                                       duda_request_t *dr);

typedef void (mariadb_query_row_cb)(void *privdata, unsigned long n_fields,
                                    char **fields, char **values, duda_request_t *dr);

typedef void (mariadb_query_end_cb)(void *privdata, mariadb_query_t *query,
                                    duda_request_t *dr);

#endif
