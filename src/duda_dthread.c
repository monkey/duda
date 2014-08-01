/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2014, Zeying Xie <swpdtz at gmail dot com>
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

#include <assert.h>
#include <string.h>
#include <ucontext.h>
#include <limits.h>
#include "duda.h"
#include "duda_api.h"
#include "duda_dthread.h"

#ifdef USE_VALGRIND
#include <valgrind/valgrind.h>
#endif

#define DTHREAD_STACK_SIZE (3 * (PTHREAD_STACK_MIN) / 2)
#define DEFAULT_DTHREAD_NUM    16

typedef struct duda_dthread_t {
    duda_dthread_func func;
    void *data;
    ucontext_t context;
    duda_dthread_scheduler_t *sch;
    int status;
    int parent_id;
#ifdef USE_VALGRIND
    unsigned int valgrind_stack_id;
#endif
    char stack[DTHREAD_STACK_SIZE];
} duda_dthread_t;

struct duda_dthread_scheduler_t {
    ucontext_t main;
    int n_dthread;
    int cap;
    int running_id;
    duda_dthread_t **dt;
};

static void _duda_dthread_release(duda_dthread_t *dt);

static void _duda_dthread_entry_point(duda_dthread_scheduler_t *sch)
{
    assert(sch);
    int id = sch->running_id;
    duda_dthread_t *dt = sch->dt[id];
    dt->func(dt->data);
    dt->status = DTHREAD_DEAD;
    sch->n_dthread--;
    sch->running_id = dt->parent_id;
}

duda_dthread_scheduler_t *duda_dthread_open()
{
    duda_dthread_scheduler_t *sch = mk_api->mem_alloc(sizeof(*sch));
    assert(sch);
    sch->n_dthread = 0;
    sch->cap = DEFAULT_DTHREAD_NUM;
    sch->running_id = -1;
    sch->dt = mk_api->mem_alloc(sizeof(duda_dthread_t *) * sch->cap);
    memset(sch->dt, 0, sizeof(duda_dthread_t *) * sch->cap);
    return sch;
}

void duda_dthread_close(duda_dthread_scheduler_t *sch)
{
    assert(sch);
    int i;
    for (i = 0; i < sch->cap; ++i) {
        duda_dthread_t *dt = sch->dt[i];
        if (dt) {
            _duda_dthread_release(dt);
        }
    }
    mk_api->mem_free(sch->dt);
    sch->dt = NULL;
    mk_api->mem_free(sch);
}

int duda_dthread_create(duda_dthread_func func, void *data)
{
    duda_dthread_scheduler_t *sch = pthread_getspecific(duda_dthread_scheduler);
    if (!sch) {
        sch = duda_dthread_open();
        assert(sch);
        pthread_setspecific(duda_dthread_scheduler, (void *) sch);
    }
    int id;
    if (sch->n_dthread >= sch->cap) {
        id = sch->cap;
        sch->dt = mk_api->mem_realloc(sch->dt, sch->cap * 2 * sizeof(duda_dthread_t *));
        assert(sch->dt);
        memset(sch->dt + sch->cap, 0, sizeof(duda_dthread_t *) * sch->cap);
        sch->cap *= 2;
    } else {
        int i;
        for (i = 0; i < sch->cap; ++i) {
            id = (i + sch->cap) % sch->cap;
            if (sch->dt[id] == NULL || sch->dt[id]->status == DTHREAD_DEAD) {
                break;
            }
        }
    }
    /* may use dthread pooling instead of release and realloc */
    if (sch->dt[id] && sch->dt[id]->status == DTHREAD_DEAD) {
        _duda_dthread_release(sch->dt[id]);
        sch->dt[id] = NULL;
    }
    duda_dthread_t *dt = mk_api->mem_alloc(sizeof(*dt));
    assert(dt);
    dt->func = func;
    dt->data = data;
    dt->sch = sch;
    dt->status = DTHREAD_READY;
    dt->parent_id = -1;
#ifdef USE_VALGRIND
    dt->valgrind_stack_id = VALGRIND_STACK_REGISTER(dt->stack, dt->stack + DTHREAD_STACK_SIZE);
#endif
    sch->dt[id] = dt;
    sch->n_dthread++;
    return id;
}

static void _duda_dthread_release(duda_dthread_t *dt)
{
    assert(dt);
#ifdef USE_VALGRIND
    VALGRIND_STACK_DEREGISTER(dt->valgrind_stack_id);
#endif
    mk_api->mem_free(dt);
}

int duda_dthread_status(int id)
{
    duda_dthread_scheduler_t *sch = pthread_getspecific(duda_dthread_scheduler);
    assert(sch);
    assert(id >= 0 && id < sch->cap);
    if (!sch->dt[id]) return DTHREAD_DEAD;
    return sch->dt[id]->status;
}

void duda_dthread_yield()
{
    duda_dthread_scheduler_t *sch = pthread_getspecific(duda_dthread_scheduler);
    assert(sch);
    int id = sch->running_id;
    assert(id >= 0);
    duda_dthread_t *dt = sch->dt[id];
    dt->status = DTHREAD_SUSPEND;
    sch->running_id = -1;
    swapcontext(&dt->context, &sch->main);
}

void duda_dthread_resume(int id)
{
    duda_dthread_scheduler_t *sch = pthread_getspecific(duda_dthread_scheduler);
    assert(sch);
    assert(id >= 0 && id < sch->cap);
    duda_dthread_t *running_dt = NULL;
    if (sch->running_id != -1) {
        running_dt = sch->dt[sch->running_id];
    }
    duda_dthread_t *dt = sch->dt[id];
    if (!dt) return;
    switch (dt->status) {
    case DTHREAD_READY:
        getcontext(&dt->context);
        dt->context.uc_stack.ss_sp = dt->stack;
        dt->context.uc_stack.ss_size = DTHREAD_STACK_SIZE;
        if (running_dt) {
            dt->context.uc_link = &running_dt->context;
            dt->parent_id = sch->running_id;
            running_dt->status = DTHREAD_SUSPEND;
        } else {
            dt->context.uc_link = &sch->main;
        }
        sch->running_id = id;
        dt->status = DTHREAD_RUNNING;
        makecontext(&dt->context, (void (*)(void))_duda_dthread_entry_point, 1, sch);
        if (running_dt) {
            swapcontext(&running_dt->context, &dt->context);
        } else {
            swapcontext(&sch->main, &dt->context);
        }
        break;
    case DTHREAD_SUSPEND:
        sch->running_id = id;
        dt->status = DTHREAD_RUNNING;
        if (running_dt) {
            running_dt->status = DTHREAD_SUSPEND;
            swapcontext(&running_dt->context, &dt->context);
        } else {
            swapcontext(&sch->main, &dt->context);
        }
        break;
    default:
        assert(0);
    }
}

int duda_dthread_running()
{
    duda_dthread_scheduler_t *sch = pthread_getspecific(duda_dthread_scheduler);
    assert(sch);
    return sch->running_id;
}

struct duda_api_dthread *duda_dthread_object()
{
    struct duda_api_dthread *obj;

    /* Alloc dthreadutine object */
    obj = mk_api->mem_alloc(sizeof(struct duda_api_dthread));

    /* Map API calls */
    obj->create = duda_dthread_create;
    obj->status = duda_dthread_status;
    obj->yield  = duda_dthread_yield;
    obj->resume = duda_dthread_resume;
    obj->chan_create = duda_dthread_channel_create;
    obj->chan_free = duda_dthread_channel_free;
    obj->chan_get_sender = duda_dthread_channel_get_sender;
    obj->chan_set_sender = duda_dthread_channel_set_sender;
    obj->chan_get_receiver = duda_dthread_channel_get_receiver;
    obj->chan_set_receiver = duda_dthread_channel_set_receiver;
    obj->chan_done = duda_dthread_channel_done;
    obj->chan_end = duda_dthread_channel_end;
    obj->chan_send = duda_dthread_channel_send;
    obj->chan_recv = duda_dthread_channel_recv;

    return obj;
}
