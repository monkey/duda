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

#ifndef DUDA_STATS_PROC_H
#define DUDA_STATS_PROC_H

#define PROC_PID_SIZE      1024
#define PROC_STAT_BUF_SIZE 1024

/*
 * This 'stat' format omits the first two fields, due to the nature
 * of  sscanf(3) and whitespaces, programs with spaces in the name can
 * screw up when scanning the information.
 */
#define PROC_STAT_FORMAT "%c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %lu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %lu %lu %llu"

/* Our tast struct to read the /proc/PID/stat values */
struct duda_proc_task {
    int  pid;                  /* %d  */
    char comm[256];            /* %s  */
    char state;			       /* %c  */
    int ppid;			       /* %d  */
    int pgrp;			       /* %d  */
    int session;		       /* %d  */
    int tty_nr;			       /* %d  */
    int tpgid;			       /* %d  */
    unsigned long flags;	   /* %lu */
    unsigned long minflt;	   /* %lu */
    unsigned long cminflt;	   /* %lu */
    unsigned long majflt;	   /* %lu */
    unsigned long cmajflt;	   /* %lu */
    unsigned long utime;	   /* %lu */
    unsigned long stime; 	   /* %lu */
    long cutime;		       /* %ld */
    long cstime;		       /* %ld */
    long priority;		       /* %ld */
    long nice;			       /* %ld */
    long num_threads;		   /* %ld */
    long itrealvalue;		   /* %ld */
    unsigned long starttime;   /* %lu */
    unsigned long vsize;	   /* %lu */
    long rss;			       /* %ld */
    unsigned long rlim;		   /* %lu */
    unsigned long startcode;   /* %lu */
    unsigned long endcode;	   /* %lu */
    unsigned long startstack;  /* %lu */
    unsigned long kstkesp;	   /* %lu */
    unsigned long kstkeip;	   /* %lu */
    unsigned long signal;	   /* %lu */
    unsigned long blocked;	   /* %lu */
    unsigned long sigignore;   /* %lu */
    unsigned long sigcatch;	   /* %lu */
    unsigned long wchan;	   /* %lu */
    unsigned long nswap;	   /* %lu */
    unsigned long cnswap;	   /* %lu */
    int exit_signal;		   /* %d  */
    int processor;		       /* %d  */
    unsigned long rt_priority; /* %lu */
    unsigned long policy;	   /* %lu */
    unsigned long long delayacct_blkio_ticks; /* %llu */

    /* Internal conversion */
    long           dd_rss;            /* bytes = (rss * PAGESIZE)                 */
    char          *dd_rss_hr;         /* RSS in human readable format             */
    unsigned long  dd_utime_s;        /* seconds = (utime / _SC_CLK_TCK)          */
    unsigned long  dd_utime_ms;       /* milliseconds = ((utime * 1000) / CPU_HZ) */
    unsigned long  dd_stime_s;        /* seconds = (utime / _SC_CLK_TCK)          */
    unsigned long  dd_stime_ms;       /* milliseconds = ((utime * 1000) / CPU_HZ) */
};

struct duda_proc_task *duda_stats_proc_stat(pid_t pid);
void duda_stats_proc_free(struct duda_proc_task *t);

#endif
