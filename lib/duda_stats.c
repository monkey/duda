/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2012-2016, Eduardo Silva P. <eduardo@monkey.io>.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <duda/duda.h>
#include <duda/duda_stats.h>
#include <duda/duda_stats_proc.h>

#define drp(dr, ...)   duda_response_printf(dr, __VA_ARGS__)

struct duda_proc_task *duda_stats_proc_stat(pid_t pid)
{
    int fd;
    int ret;
    char *p, *q;
    char *buf;
    char pid_path[PROC_PID_SIZE];
    struct duda_proc_task *t;

    t = mk_api->mem_alloc_z(sizeof(struct duda_proc_task));

    /* Compose path for /proc/PID/stat */
    ret = snprintf(pid_path, PROC_PID_SIZE, "/proc/%i/stat", pid);
    if (ret < 0) {
        printf("Error: could not compose PID path\n");
        exit(EXIT_FAILURE);
    }

    fd = open(pid_path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    buf = mk_api->mem_alloc_z(8192);
    ret = read(fd, buf, 8192 - 1);
    if (ret < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    close(fd);

    sscanf(buf, "%d", &t->pid);
    mk_api->mem_free(buf);

    /*
     * workaround for process with spaces in the name, so we dont screw up
     * sscanf(3).
     */
    p = buf;
    while (*p != '(') p++; p++;
    q = p;
    while (*q != ')') q++;
    strncpy(t->comm, p, q - p);
    q += 2;

    /* Read pending values */
    sscanf(q, PROC_STAT_FORMAT,
           &t->state,
           &t->ppid,
           &t->pgrp,
           &t->session,
           &t->tty_nr,
           &t->tpgid,
           &t->flags,
           &t->minflt,
           &t->cminflt,
           &t->majflt,
           &t->cmajflt,
           &t->utime,
           &t->stime,
           &t->cutime,
           &t->cstime,
           &t->priority,
           &t->nice,
           &t->num_threads,
           &t->itrealvalue,
           &t->starttime,
           &t->vsize,
           &t->rss,
           &t->rlim,
           &t->startcode,
           &t->endcode,
           &t->startstack,
           &t->kstkesp,
           &t->kstkeip,
           &t->signal,
           &t->blocked,
           &t->sigignore,
           &t->sigcatch,
           &t->wchan,
           &t->nswap,
           &t->cnswap,
           &t->exit_signal,
           &t->processor,
           &t->rt_priority,
           &t->policy,
           &t->delayacct_blkio_ticks);

    /* Internal conversion */
    t->dd_rss = (t->rss * duda_stats_pagesize);
    t->dd_utime_s = (t->utime / duda_stats_cpu_hz);
    t->dd_utime_ms = ((t->utime * 1000) / duda_stats_cpu_hz);
    t->dd_stime_s = (t->stime / duda_stats_cpu_hz);
    t->dd_stime_ms = ((t->stime * 1000) / duda_stats_cpu_hz);

    return t;
}

void duda_stats_proc_free(struct duda_proc_task *t)
{
    mk_api->mem_free(t->dd_rss_hr);
    mk_api->mem_free(t);
}

/*
 * This function is invoked by each worker of the stack on initialization,
 * mostly to set their own data and pointer access to memory statistics.
 */
int duda_stats_worker_init()
{
    struct duda_stats_worker *st;
    struct duda_proc_task *task;

    st = mk_api->mem_alloc(sizeof(struct duda_stats_worker));
    st->task_id     = syscall(__NR_gettid);

    task = duda_stats_proc_stat(st->task_id);
    st->worker_name = mk_api->str_dup(task->comm);
    duda_stats_proc_free(task);

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

    /* Kernel information: PAGESIZE and CPU_HZ */
    duda_stats_pagesize = sysconf(_SC_PAGESIZE);
    duda_stats_cpu_hz = sysconf(_SC_CLK_TCK);

    return 0;
}

#endif /* defined JEMALLOC_STATS */
