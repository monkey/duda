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

#include <duda.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct duda *duda_create()
{
    struct duda *d;

    /* Create and initialize a Duda context */
    d = mk_mem_alloc_z(sizeof(struct duda));
    if (!d) {
        return NULL;
    }
    mk_list_init(&d->services);

    /* Create Monkey server context */
    d->monkey = mk_create();
    if (!d->monkey) {
        mk_mem_free(d);
        return NULL;
    }
    return d;
}

int duda_destroy(struct duda *duda_ctx)
{
    if (duda_ctx->monkey) {
        /* FIXME: Add mk_destroy() API function */
    }

    mk_mem_free(duda_ctx->tcp_port);
    mk_mem_free(duda_ctx);

    return 0;
}

static void duda_switcher(mk_request_t *request, void *data)
{
    int ret;
    struct mk_list *head;
    struct mk_list *r_head;
    struct duda *duda_ctx;
    struct duda_service *service;
    struct duda_router_path *path;
    struct duda_request *dr;

    duda_ctx = data;

    /* Iterate registered services and find a route */
    mk_list_foreach(head, &duda_ctx->services) {
        service = mk_list_entry(head, struct duda_service, _head);

        /* Check service route paths */
        ret = duda_router_path_lookup(service, request, &path);
        if (ret == DUDA_ROUTER_MATCH) {
            dr = duda_request_create(request, service, path);
            if (!dr) {
                goto error;
            }
            path->callback(dr);
            return;
        }
    }

 error:
    /* Handle a custom error */
    mk_http_status(request, 500);
    mk_http_send(request, "Hello from Duda!\n", 17, NULL);
}

int duda_start(struct duda *duda_ctx)
{
    int port;
    mk_vhost_t *vh;

    /* Dummy check */
    if (!duda_ctx) {
        return -1;
    }

    /* validate TCP port */
    if (!duda_ctx->tcp_port) {
        duda_ctx->tcp_port = mk_string_dup(DUDA_DEFAULT_PORT);
    }

    port = atoi(duda_ctx->tcp_port);
    if (port <= 1) {
        fprintf(stderr, "Invalid TCP Port %s\n", duda_ctx->tcp_port);
        return -1;
    }

    /* Create HTTP server instance */
    mk_config_set(duda_ctx->monkey,
                  "Listen", duda_ctx->tcp_port,
                  NULL);

    /* Setup Virtual Host */
    vh = mk_vhost_create(duda_ctx->monkey, NULL);
    mk_vhost_set(vh,
                 "Name", "default",
                 NULL);
    mk_vhost_handler(vh, "/", duda_switcher, duda_ctx);
    mk_start(duda_ctx->monkey);
}

int duda_stop(struct duda *duda_ctx)
{
}
