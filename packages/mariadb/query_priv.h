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

#ifndef MARIADB_QUERY_PRIV_H
#define MARIADB_QUERY_PRIV_H

#include "query.h"

typedef enum {
    QUERY_ABORT_NO, QUERY_ABORT_YES
} mariadb_query_abort_t;

struct mariadb_query {
    char *query_str;
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int n_fields;
    char **fields;
    int error;
    mariadb_query_abort_t abort;

    mariadb_query_result_cb *result_cb;
    mariadb_query_row_cb *row_cb;
    mariadb_query_end_cb *end_cb;
    void *privdata;

    struct mk_list _head;
};

/*
 * @METHOD_NAME: abort_async
 * @METHOD_DESC: Abort a query.
 * @METHOD_PROTO: void abort_async(mariadb_query_t *query)
 * @METHOD_PARAM: query The query to be aborted.
 * @METHOD_RETURN: None.
 */

static inline void mariadb_query_abort(mariadb_query_t *query)
{
    query->abort = QUERY_ABORT_YES;
}

mariadb_query_t *mariadb_query_init(const char *query_str,
                                    mariadb_query_result_cb *result_cb,
                                    mariadb_query_row_cb *row_cb,
                                    mariadb_query_end_cb *end_cb, void *privdata);

void mariadb_query_free(mariadb_query_t *query);

#endif
