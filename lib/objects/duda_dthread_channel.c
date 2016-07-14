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

#include <stdlib.h>
#include <assert.h>
#include <duda/duda_api.h>
#include <duda/objects/duda_dthread.h>
#include <duda/objects/duda_dthread_channel.h>

struct duda_dthread_channel_elem_t {
    void *data;
    struct mk_list _head;
};

duda_dthread_channel_elem_t *duda_dthread_channel_elem_create(void *data)
{
    duda_dthread_channel_elem_t *elem = mk_api->mem_alloc(sizeof(*elem));
    assert(elem);
    elem->data = data;
    return elem;
}

static void duda_dthread_channel_elem_free(duda_dthread_channel_elem_t *elem)
{
    assert(elem);
    mk_list_del(&elem->_head);
    mk_api->mem_free(elem);
}

/*
 * @METHOD_NAME: chan_create
 * @METHOD_DESC: create a channel(pipe) for dthread communication.
 * @METHOD_PROTO: duda_dthread_channel_t *chan_create(int size)
 * @METHOD_PARAM: size the buffered size of the channel.
 * @METHOD_RETURN: returns a new channel.
 */
duda_dthread_channel_t *duda_dthread_channel_create(int size)
{
    duda_dthread_channel_t *chan = mk_api->mem_alloc(sizeof(*chan));
    assert(chan);
	chan->size = size + 1;
    chan->used = 0;
    mk_list_init(&chan->bufs);
    chan->sender = -1;
    chan->receiver = -1;
    chan->ended = 0;
    chan->done = 0;
	return chan;
}

/*
 * @METHOD_NAME: chan_free
 * @METHOD_DESC: release a given channel.
 * @METHOD_PROTO: void chan_free(duda_dthread_channel_t *chan)
 * @METHOD_PARAM: chan the target channel to be released.
 * @METHOD_RETURN: this method do not return any value.
 */
void duda_dthread_channel_free(duda_dthread_channel_t *chan)
{
    assert(chan);
    if (chan->receiver != -1) {
        mk_list_del(&chan->_head);
    }
    mk_api->mem_free(chan);
}

/*
 * @METHOD_NAME: chan_send
 * @METHOD_DESC: add a new element to the given channel.
 * @METHOD_PROTO: int chan_send(duda_dthread_channel_t *chan, void *data)
 * @METHOD_PARAM: chan the target channel to send.
 * @METHOD_PARAM: data the new element to be sent to channel.
 * @METHOD_RETURN: return DTHREAD_CHANNEL_BROKEN if the other side of the pipe
 * is closed, otherwise return DTHREAD_CHANNEL_OK.
 */
int duda_dthread_channel_send(duda_dthread_channel_t *chan, void *data)
{
    assert(chan);
    if (chan->receiver == -1) {
        return DTHREAD_CHANNEL_BROKEN;
    }
    if (chan->used == chan->size) {
        // channel is full
        duda_dthread_resume(chan->receiver);
    }
    duda_dthread_channel_elem_t *elem = duda_dthread_channel_elem_create(data);
    mk_list_add(&elem->_head, &chan->bufs);
    chan->used++;
    return DTHREAD_CHANNEL_OK;
}

/*
 * @METHOD_NAME: chan_recv
 * @METHOD_DESC: remove an element from a given channel.
 * @METHOD_PROTO: void *chan_recv(duda_dthread_channel_t *chan)
 * @METHOD_PARAM: chan the target channel to receive.
 * @METHOD_RETURN: the front element of the channel.
 */
void *duda_dthread_channel_recv(duda_dthread_channel_t *chan)
{
    assert(chan);
    assert(!chan->done);
    if (chan->used == 0) {
        // channel is empty
        duda_dthread_resume(chan->sender);
    }
    duda_dthread_channel_elem_t *elem = mk_list_entry_first(&chan->bufs,
            duda_dthread_channel_elem_t, _head);
    void *data = elem->data;
    duda_dthread_channel_elem_free(elem);
    chan->used--;
    if (chan->used == 0 && chan->ended) {
        chan->done = 1;
    }
    return data;
}
