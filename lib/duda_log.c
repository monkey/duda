/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2012-2016, Eduardo Silva P. <eduardo@monkey.io>
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

#define _GNU_SOURCE         /* See feature_test_macros(7) */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdarg.h>

#include <monkey/mk_api.h>
#include <duda/duda.h>
#include <duda/duda_api.h>
#include <duda/duda_conf.h>
#include <duda/duda_stats.h>

/*
 * Logger writer: this function runs in a separate thread and its main job
 * is to read incoming messages from Logger pipes, wait until the pipe buffer
 * reach some % of it capacity and flush the content to the proper file on disk.
 */
void duda_logger_writer(void *arg)
{
    (void) arg;
    int i;
    int fd;
    int sec = 3;
    int nfds;
    struct mk_list *head_vs;
    struct mk_list *head_ws;
    struct mk_list *head;
    struct vhost_services *entry_vs;
    struct web_service *entry_ws;
    struct mk_event_loop *evl;
    duda_logger_context_t *log_ctx;

    int bytes;
    int err;
    int flog;
    int clk;
    int max_events = 256;
    long slen;
    int timeout;

    mk_api->worker_rename("duda:logwriter");

#if defined(MALLOC_JEMALLOC) && defined(JEMALLOC_STATS)
    duda_stats_worker_init();
#endif

    /* pipe_size:
     * ----------
     * Linux set a pipe size usingto the PAGE_SIZE,
     * check linux/include/pipe_fs_i.h for details:
     *
     *       #define PIPE_SIZE               PAGE_SIZE
     *
     * In the same header file we can found that every
     * pipe has 16 pages, so our real memory allocation
     * is: (PAGE_SIZE*PIPE_BUFFERS)
     */
    long pipe_size;

    /* buffer_limit:
     * -------------
     * it means the maximum data that a monkey log pipe can contain.
     */
    long buffer_limit;

    /* Lets set 75% of a pipe capacity */
    pipe_size = sysconf(_SC_PAGESIZE) * 16;
    buffer_limit = (pipe_size * 0.70);

    /* Creating poll */
    evl = mk_api->ev_loop_create(max_events);

    /* Lookup all loggers and grab the pipe's FDs */
    mk_list_foreach(head_vs, &services_list) {
        entry_vs = mk_list_entry(head_vs, struct vhost_services, _head);

        mk_list_foreach(head_ws, &entry_vs->services) {
            entry_ws = mk_list_entry(head_ws, struct web_service, _head);

            /* go around each Logger */
            mk_list_foreach(head, entry_ws->loggers) {
                log_ctx = mk_list_entry(head, duda_logger_context_t, _head);
                /* FIXME mk_api->ev_add(evl, log_ctx->pipe_fd[0], MK_EVENT_READ, log_ctx); */
            }
        }
    }

    /* Set initial timeout to 3 seconds*/
    timeout = time(NULL) + sec;

    /* Reading pipe buffer */
    while (1) {
        usleep(50000);

        mk_api->ev_wait(evl);
        clk = mk_api->time_unix();

        /* FIXME nfds = mk_api->ev_translate(evl); */
        for (i = 0; i < nfds; i++) {
            fd       = evl->events[i].fd;
            log_ctx  = evl->events[i].data;

            err = ioctl(fd, FIONREAD, &bytes);
            if (mk_unlikely(err == -1)){
                perror("ioctl");
                continue;
            }

            if (bytes < buffer_limit && clk <= timeout) {
                continue;
            }

            timeout = clk + sec;

            flog = open(log_ctx->log_path, O_WRONLY | O_CREAT | O_CLOEXEC, 0644);
            if (mk_unlikely(flog == -1)) {
                mk_warn("Could not open logfile '%s'", log_ctx->log_path);

                int consumed = 0;
                char buf[255];
                do {
                    slen = read(fd, buf, 255);
                    if (slen > 0) {
                        consumed += slen;
                    }
                    else {
                        break;
                    }
                } while (consumed < bytes);
                continue;
            }

            lseek(flog, 0, SEEK_END);

#if defined (__linux__)
            slen = splice(fd, NULL, flog,
                          NULL, bytes, SPLICE_F_MOVE);
#else
            long len;
            char *tmp;

            slen = -1;
            tmp = mk_api->mem_alloc(bytes);
            if (tmp) {
                len = read(fd, tmp, bytes);
                if (len > 0) {
                    slen = write(flog, tmp, len);
                }
                mk_api->mem_free(tmp);
            }
#endif
            if (mk_unlikely(slen == -1)) {
                mk_warn("Could not write to log file: splice() = %ld", slen);
            }

            PLUGIN_TRACE("written %i bytes", bytes);
            close(flog);
        }
    }
}

/* Initialize Logger internals, no API exposed */
int duda_logger_init()
{
    pthread_t t;
    mk_api->worker_spawn(duda_logger_writer, NULL, &t);
    return 0;
}

/*
 * @OBJ_NAME: logger
 * @OBJ_MENU: Log Writer
 * @OBJ_DESC: The Log Writer object provides a set of methods to register
 * text messages into the file system, most well known as log files. You
 * can define many log writer contexts that can be used on callbacks or
 * in other sections of the web service.
 *
 * In order to handle multiple contexts and a high concurrency, the Log Writer
 * core implements a thread worker that contains a buffer for each Log Writer
 * context, so every time a log entry is registered, it's buffered into the
 * queue and flushed to disk every three seconds or when the buffer reachs its
 * 75% of capacity. A common buffer size is 65Kb.
 *
 * It's mandatory that every Log Writer context must be initialized inside duda_main(), so
 * it become available for the whole service when entering in the server loop.
 *
 * The storage path for the file logs it is given by the LogDir entry in the [WEB_SERVICE]
 * section from the virtual host definition. If you are using Duda Client Manager (DudaC)
 * this is automatically configured when the 'logs' directory exists in the web service
 * source code tree.
 *
 * As an additional feature, each log context can be enabled/disabled on run time, to
 * accomplish this just change the field 'enabled' to MK_TRUE or MK_FALSE,
 * e.g: context.enabled = MK_FALSE.
 *
 */

/*
 * @METHOD_NAME: duda_logger_create
 * @METHOD_DESC: This function initialize a Log Writter context for the web service,
 * it can be used many times as required to initialize multiple contexts. It must be
 * used ONLY inside duda_main().
 * @METHOD_PROTO: int duda_logger_create(duda_logger_t *context, char *name)
 * @METHOD_PARAM: context the Log Writer context, this is the unique identifier for logs
 * associated to 'name'.
 * @METHOD_PARAM: name represents the target file name.
 * @METHOD_RETURN: This function always returns zero.
 */

/*
 * @METHOD_NAME: print
 * @METHOD_DESC: It format and prints a customized message to the given Log Writer
 * context. At the moment the only restriction of this method is that it cannot be used
 * from duda_main().
 * @METHOD_PROTO: int print(duda_logger_t *context, char *fmt, ...)
 * @METHOD_PARAM: context the Log Writer context initialized previously from duda_main().
 * @METHOD_PARAM: fmt it specifies the string format, much like printf works.
 * @METHOD_RETURN: Uppon successfull completion it returns zero, under any error it returns
 * a negative number.
 */
int duda_logger_print(duda_logger_t *key, char *fmt, ...)
{
    int ret;
    int n, size = 128;
    char *p, *np;
    char *time_fmt;
    mk_ptr_t *time_human;
    va_list ap;
    duda_global_t *gl = &key->global_key;
    duda_logger_context_t *ctx;

    if (key->enabled == MK_FALSE) {
        return 0;
    }

    ctx = duda_global_get(*gl);
    if (!ctx) {
        printf("UNSUPPORTED: %s():%i\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if ((p = mk_api->mem_alloc(size)) == NULL) {
        return -1;
    }

    time_fmt = pthread_getspecific(duda_logger_fmt_cache);
    memset(time_fmt, 0, 512);
    time_human = mk_api->time_human();
    strncpy(time_fmt, time_human->data, time_human->len);
    time_fmt[time_human->len] = ' ';
    strcpy(time_fmt + time_human->len + 1, fmt);
    fmt = time_fmt;

    while (1) {
        /* Try to print in the allocated space. */
        va_start(ap, fmt);
        n = vsnprintf(p, size, fmt, ap);
        va_end(ap);
        /* If that worked, return the string. */
        if (n > -1 && n < size)
            break;

        size *= 2;  /* twice the old size */
        if ((np = mk_api->mem_realloc(p, size)) == NULL) {
            monkey->mem_free(p);
            return -1;
        } else {
            p = np;
        }
    }

    ret = write(ctx->pipe_fd[1], p, n);
    mk_api->mem_free(p);

    return ret;
}

struct duda_api_logger *duda_logger_object()
{
    struct duda_api_logger *c;

    c = mk_api->mem_alloc(sizeof(struct duda_api_logger));
    c->print = duda_logger_print;
    return c;
}
