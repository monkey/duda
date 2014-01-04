/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2012-2013, Eduardo Silva P. <edsiper@gmail.com>.
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

#ifndef DUDA_STATS_H
#define DUDA_STATS_H

#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>

#include "mk_list.h"

struct duda_stats_worker {
    pid_t     task_id;
    uint64_t *mem_allocated;
    uint64_t *mem_deallocated;
    char     *worker_name;

    struct mk_list _head;
};

struct duda_statistics {
    struct mk_list mem;
};

pthread_mutex_t duda_mutex_stats;
struct duda_statistics duda_stats;

void duda_stats_cb(duda_request_t *dr);
void duda_stats_txt_cb(duda_request_t *dr);
int  duda_stats_worker_init();
int  duda_stats_init();

#endif
