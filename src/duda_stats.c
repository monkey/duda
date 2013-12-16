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

#include <string.h>

#include "duda.h"
#include "duda_stats.h"

#define drp(dr, ...)   duda_response_printf(dr, __VA_ARGS__)

static char *human_readable_size(long size)
{
    long u = 1024, i, len = 128;
    char *buf = api->mem_alloc(len);
    static const char *__units[] = { "b", "K", "M", "G",
        "T", "P", "E", "Z", "Y", NULL
    };

    for (i = 0; __units[i] != NULL; i++) {
        if ((size / u) == 0) {
            break;
        }
        u *= 1024;
    }
    if (!i) {
        snprintf(buf, len, "%ld %s", size, __units[0]);
    }
    else {
        float fsize = (float) ((double) size / (u / 1024));
        snprintf(buf, len, "%.2f%s", fsize, __units[i]);
    }

    return buf;
}

void duda_stats_cb(duda_request_t *dr)
{
    char *hr;
    uint64_t total;
    struct mk_list *head;
    struct duda_stats_worker *st;

    duda_response_http_status(dr, 200);
    duda_response_http_header(dr, "Content-Type: text/plain");

    drp(dr, "Duda I/O Statistics\n===================\n\n");
    drp(dr,
        "+--------------------------------------------+\n"
        "|          Workers Memory Usage              |\n"
        "+-----------+-----------------+--------------+\n"
        "| task ID   | mem bytes       | mem unit     |\n"
        "+-----------+-----------------+--------------+\n");

    mk_list_foreach(head, &duda_stats.mem) {
        st = mk_list_entry(head, struct duda_stats_worker, _head);

        total = (*st->mem_allocated - *st->mem_deallocated);
        hr    = human_readable_size(total);
        drp(dr, "| %-9lu | %-15lu | %-12s |\n", st->task_id, total, hr);

        /* current memory in use */
        //drp(dr, "   memory usage: %lu (%s)\n", total, hr);
        api->mem_free(hr);
    }

    drp(dr, "+-----------+-----------------+--------------+\n");
    duda_response_end(dr, NULL);
}

/*
 * This function is invoked by each worker of the stack on initialization,
 * mostly to set their own data and pointer access to memory statistics.
 */
int duda_stats_worker_init()
{
    uint64_t sz;
    struct duda_stats_worker *st;

    st = api->mem_alloc(sizeof(struct duda_stats_worker));
    st->task_id     = syscall(__NR_gettid);
    st->worker_name = NULL;

    /* Protect this section as it needs to be atomic */
    pthread_mutex_lock(&duda_mutex_stats);

    sz = sizeof(st->mem_allocated);

    /* Get pointers to memory counters */
    je_mallctl("thread.allocatedp", &st->mem_allocated, &sz, NULL, 0);
    je_mallctl("thread.deallocatedp", &st->mem_deallocated, &sz, NULL, 0);

    /* Link the worker info into the global list */
    mk_list_add(&st->_head, &duda_stats.mem);

    pthread_mutex_unlock(&duda_mutex_stats);

    return 0;
}

int duda_stats_init()
{
    /* Initialize global statistics */
    memset(&duda_stats, '\0', sizeof(struct duda_statistics));
    mk_list_init(&duda_stats.mem);

    /* Set mutex for stats initialization through workers */
    pthread_mutex_init(&duda_mutex_stats, (pthread_mutexattr_t *) NULL);
    return 0;
}
