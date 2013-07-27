/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2012-2013, Eduardo Silva P. <edsiper@gmail.com>
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

#include <stdio.h>
#include <stdarg.h>

#include "MKPlugin.h"

#include "duda_debug.h"
#include "duda_console.h"
#include "duda.h"
#include "duda_api.h"
#include "duda_map.h"
#include "duda_param.h"
#include "duda_session.h"
#include "duda_xtime.h"
#include "duda_cookie.h"
#include "duda_package.h"
#include "duda_event.h"
#include "duda_queue.h"
#include "duda_global.h"
#include "duda_sendfile.h"
#include "duda_body_buffer.h"
#include "duda_data.h"
#include "duda_fconf.h"
#include "duda_qs.h"
#include "duda_worker.h"
#include "webservice.h"

struct duda_api_objects *duda_api_master()
{
    struct duda_api_objects *objs;

    /* Alloc memory */
    objs = mk_api->mem_alloc(sizeof(struct duda_api_objects));
    objs->duda     = mk_api->mem_alloc(sizeof(struct duda_api_main));
    objs->monkey   = mk_api;
    objs->msg      = mk_api->mem_alloc(sizeof(struct duda_api_msg));
    objs->debug    = mk_api->mem_alloc(sizeof(struct duda_api_debug));

    /* MAP Duda calls */
    objs->duda->package_load = duda_package_load;

    /* MSG object */
    objs->msg->info  = duda_debug_info;
    objs->msg->warn  = duda_debug_warn;
    objs->msg->err   = duda_debug_err;
    objs->msg->bug   = duda_debug_bug;

    /* Assign Objects */
    objs->global   = duda_global_object();
    objs->map      = duda_map_object();
    objs->event    = duda_event_object();
    objs->gc       = duda_gc_object();
    objs->request  = duda_request_object();
    objs->response = duda_response_object();
    objs->console  = duda_console_object();
    objs->logger   = duda_logger_object();
    objs->param    = duda_param_object();
    objs->session  = duda_session_object();
    objs->xtime    = duda_xtime_object();
    objs->cookie   = duda_cookie_object();
    objs->qs       = duda_qs_object();
    objs->data     = duda_data_object();
    objs->conf     = duda_conf_object();
    objs->fconf    = duda_fconf_object();
    objs->worker   = duda_worker_object();

    /* FIXME - DEBUG object */
#ifdef DEBUG
    objs->debug->stacktrace = mk_api->stacktrace;
#endif

    return objs;
}

void duda_api_exception(duda_request_t *dr, const char *message)
{
    /* Convert monkey pointers to fixed size buffer strings */
    char *appname   = mk_api->pointer_to_buf(dr->appname);
    char *interface = mk_api->pointer_to_buf(dr->interface);
    char *method    = mk_api->pointer_to_buf(dr->method);
    char *body      = mk_api->pointer_to_buf(dr->sr->body);

    /* Print out the exception */
    printf("%sDuda API Exception%s\nURI    : /%s/%s/%s\nError  : %s\n",
           ANSI_BOLD,
           ANSI_RESET,
           appname,
           interface,
           method,
           message);
    printf("%s<---- request ---->%s\n%s%s\n%s<------ end ------>%s\n",
           ANSI_YELLOW, ANSI_RESET,
           ANSI_CYAN, body, ANSI_YELLOW, ANSI_RESET);
    fflush(stdout);

    /* Free resources */
    mk_api->mem_free(appname);
    mk_api->mem_free(interface);
    mk_api->mem_free(method);
    mk_api->mem_free(body);
}
