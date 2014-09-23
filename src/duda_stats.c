/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2012-2014, Eduardo Silva P. <eduardo@monkey.io>.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#if defined(MALLOC_JEMALLOC) && defined(JEMALLOC_STATS)

#include <string.h>

#include "duda.h"
#include "duda_stats.h"

#define drp(dr, ...)   duda_response_printf(dr, __VA_ARGS__)

/*
 * This function is invoked by each worker of the stack on initialization,
 * mostly to set their own data and pointer access to memory statistics.
 */
int duda_stats_worker_init()
{
    struct duda_stats_worker *st;

    st = mk_api->mem_alloc(sizeof(struct duda_stats_worker));
    st->task_id     = syscall(__NR_gettid);
    st->worker_name = NULL;

    /* Protect this section as it needs to be atomic */
    pthread_mutex_lock(&duda_mutex_stats);

    /* Get pointers to memory counters */
    size_t sz;
    sz = sizeof(st->mem_allocated);
    mk_api->je_mallctl("thread.allocatedp", &st->mem_allocated, &sz, NULL, 0);
    mk_api->je_mallctl("thread.deallocatedp", &st->mem_deallocated, &sz, NULL, 0);

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

#endif /* defined JEMALLOC_STATS */
