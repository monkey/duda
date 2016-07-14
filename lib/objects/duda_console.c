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

#include <monkey/mk_api.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

#include <duda/duda.h>
#include <duda/duda_api.h>
#include <duda/duda_conf.h>
#include <duda/objects/duda_router.h>
#include <duda/duda_webservice.h>
#include <duda/duda_stats.h>

/*
 * @OBJ_NAME: console
 * @OBJ_MENU: Console
 * @OBJ_DESC: The console object provides a set of methods to write debug information
 * to the console URL interface of the running web service.
 */


/* callback for /app/console/messages */
void duda_console_cb_messages(duda_request_t *dr)
{
    (void) dr;

    duda_response_http_status(dr, 500);
    duda_response_end(dr, NULL);
}

static char *human_readable_size(long size)
{
    long u = 1024, i, len = 128;
    char *buf = mk_api->mem_alloc(len);
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

static inline void service_info_row(duda_request_t *dr, char *title, char *value)
{
    duda_response_printf(dr, "<tr><td><strong>%s</strong></td><td>%s</td></tr>\n",
                         title, !value ? "": value);
}

static void dashboard_panel_memory(duda_request_t *dr)
{
    char *hr;
    uint64_t total;
    uint64_t total_all = 0;
    struct mk_list *head;
    struct duda_stats_worker *st;

    duda_response_printf(dr, DD_HTML_PANEL_HEADER, "info", "Memory Usage per Worker");

#if !defined(JEMALLOC_STATS)
    duda_response_printf(dr,
                         "The server have <strong>not</strong> been "
                         "built with Memory Stats support\n");
    duda_response_printf(dr, DD_HTML_PANEL_FOOTER, "");
    return;
#endif

    duda_response_printf(dr,
                         "<table class='table table-striped'>\n"
                         "  <thead>\n"
                         "    <tr>\n"
                         "      <th>WID</th>\n"
                         "      <th>Name</th>\n"
                         "      <th>Bytes</th>\n"
                         "      <th>Total</th>\n"
                         "    </tr>\n"
                         "  </thead>\n"
                         "  <tbody>\n");

    mk_list_foreach(head, &duda_stats.mem) {
        st = mk_list_entry(head, struct duda_stats_worker, _head);

        total = (*st->mem_allocated - *st->mem_deallocated);
        hr    = human_readable_size(total);

        duda_response_printf(dr,
                             "    <tr>\n"
                             "        <td>%lu</td>\n"
                             "        <td>%s</td>\n"
                             "        <td>%lu</td>\n"
                             "        <td>%s</td>\n"
                             "    </tr>\n",
                             st->task_id, st->worker_name, total, hr
                             );
        total_all += total;
        mk_api->mem_free(hr);
    }
    duda_response_printf(dr,
                         "</tbody>\n"
                         "</table>\n");

    duda_response_printf(dr, DD_HTML_PANEL_FOOTER,
                         "the information above represents a global view of the server");
}

/*
 * @METHOD_NAME: debug
 * @METHOD_DESC: It format and and prints a customized message to the web service
 * console interface
 * @METHOD_PROTO: void debug(duda_request_t *dr, char *format, ...)
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_PARAM: format Specifies the subsequent arguments to be formatted
 * @METHOD_RETURN: Do not return anything
 */
void duda_console_write(duda_request_t *dr,
                        char *file, int line,
                        char *format, ...)
{
}

/*
 * @METHOD_NAME: dashboard
 * @METHOD_DESC: It enable the Console Web Interface and creates an URI association
 * to access it following the pattern of the Router mechanism.
 * @METHOD_PROTO: int dashboard(char *uri)
 * @METHOD_PARAM: uri The URL pattern, e.g: '/dashboard'
 * @METHOD_RETURN: Upon successful completion it returns 0, on error returns -1.
 */
int duda_console_dashboard(char *uri, struct web_service *ws)
{
    if (!uri) {
        return -1;
    }

    if (uri[0] != '/') {
        return -1;
    }

    if (strstr(uri, ":")) {
        return -1;
    }

    ws->dashboard = uri;
    return 0;
}

int duda_console_enable(char *map, struct mk_list *list)
{
    int len;
    char buf[1024];

    len = strlen(map);
    memset(buf, '\0', sizeof(buf));
    strncpy(buf, map, len);

    if (buf[len - 1] != '/') {
        buf[len] = '/';
    }

    //FIXME return duda_router_map(buf, duda_console_dash_home, "dashboard-home", list);
    return -1;
}

struct duda_api_console *duda_console_object()
{
    struct duda_api_console *c;

    c = mk_api->mem_alloc(sizeof(struct duda_api_console));
    c->_dashboard = duda_console_dashboard;
    c->_debug = duda_console_write;

    return c;
}
