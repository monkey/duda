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

#ifndef DUDA_API_DTHREAD_H
#define DUDA_API_DTHREAD_H

#include "duda_dthread_channel.h"

#define DTHREAD_DEAD       0
#define DTHREAD_READY      1
#define DTHREAD_RUNNING    2
#define DTHREAD_SUSPEND    3

pthread_key_t duda_dthread_scheduler;

typedef struct duda_dthread_scheduler_t duda_dthread_scheduler_t;

typedef void (*duda_dthread_func)(void *data);

/* DTHREAD object: dthread->x() */
struct duda_api_dthread {
    int (*create)(duda_dthread_func func, void *data);
    int (*status)(int id);
    void (*yield)();
    void (*resume)(int id);
    duda_dthread_channel_t *(*chan_create)(int size);
    void (*chan_free)(duda_dthread_channel_t *chan);
    int (*chan_get_sender)(duda_dthread_channel_t *chan);
    void (*chan_set_sender)(duda_dthread_channel_t *chan, int sender);
    int (*chan_get_receiver)(duda_dthread_channel_t *chan);
    void (*chan_set_receiver)(duda_dthread_channel_t *chan, int receiver);
    int (*chan_done)(duda_dthread_channel_t *chan);
    void (*chan_end)(duda_dthread_channel_t *chan);
    int (*chan_send)(duda_dthread_channel_t *chan, void *data);
    void *(*chan_recv)(duda_dthread_channel_t *chan);
    int (*running)();
};

duda_dthread_scheduler_t *duda_dthread_open();
void duda_dthread_close(duda_dthread_scheduler_t *sch);

int duda_dthread_create(duda_dthread_func func, void *data);
int duda_dthread_status(int id);
void duda_dthread_yield();
void duda_dthread_resume(int id);
int duda_dthread_running();

void duda_dthread_add_channel(int id, struct duda_dthread_channel_t *chan);

struct duda_api_dthread *duda_dthread_object();

#endif
