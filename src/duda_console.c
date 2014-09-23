/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2012-2014, Eduardo Silva P. <eduardo@monkey.io>
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

#include <monkey/mk_api.h>
#include "duda.h"
#include "duda_api.h"
#include "duda_conf.h"
#include "duda_router.h"
#include "duda_webservice.h"
#include "duda_stats.h"

/*
 * @OBJ_NAME: console
 * @OBJ_MENU: Console
 * @OBJ_DESC: The console object provides a set of methods to write debug information
 * to the console URL interface of the running web service.
 */


/* callback for /app/console/messages */
void duda_console_cb_messages(duda_request_t *dr)
{
    int size = 1024;
    char buf[size];

    /* Create new path */
    snprintf(buf, size, "/tmp/%s.duda.messages", dr->ws_root->name.data);

    /* response */
    duda_response_http_status(dr, 200);
    duda_response_http_header(dr, "Content-Type: text/plain");
    duda_response_sendfile(dr, buf);
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

static void dashboard_panel_service_info(duda_request_t *dr)
{
    duda_response_printf(dr, DD_HTML_PANEL_HEADER, "primary", "Web Service Information");

#if !defined(JEMALLOC_STATS)
    duda_response_printf(dr,
                         "The server have <strong>not</strong> been "
                         "built with Memory Stats support\n");
    duda_response_printf(dr, DD_HTML_PANEL_FOOTER, "");
    return;
#endif

    duda_response_printf(dr,
                         "<table class='table table-striped'>\n"
                         "  <tbody>\n");

    service_info_row(dr, "Name",  dr->ws_root->name.data);
    service_info_row(dr, "Internal Name", dr->ws_root->fixed_name.data);
    service_info_row(dr, "Document", dr->ws_root->docroot.data);
    service_info_row(dr, "Config", dr->ws_root->confdir.data);
    service_info_row(dr, "Data", dr->ws_root->datadir.data);
    service_info_row(dr, "Logs", dr->ws_root->logdir.data);

    duda_response_printf(dr,
                         "</tbody>\n"
                         "</table>\n");

    duda_response_printf(dr, DD_HTML_PANEL_FOOTER,
                         "");
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

/* callback for Dashboard home */
void duda_console_dash_home(duda_request_t *dr)
{
    duda_response_http_status(dr, 200);
    duda_response_http_header(dr, "Content-Type: text/html");

    /* Header */
    duda_response_printf(dr, DD_HTML_HEADER, "Console Map", DD_HTML_CSS);
    duda_response_printf(dr, DD_HTML_NAVBAR_BASIC, "", "Dashboard");

    duda_response_printf(dr,
                         "<div class=\"container\">"
                         "  <div class=\"duda-template\">"
                         "    <h1>Dashboard: %s/</h1>\n"
                         "      <address>\n"
                         "        General information of <strong>%s</strong> web service\n"
                         "      </address>\n",
                         dr->ws_root->name.data,
                         dr->ws_root->name.data);

    /* First row: service info and memory usage */
    duda_response_printf(dr, "<div class='row'>");
    duda_response_printf(dr, "<div class='col-md-7'>");
    dashboard_panel_service_info(dr);
    duda_response_printf(dr, "</div>");

    duda_response_printf(dr, "<div class='col-md-5'>");
    dashboard_panel_memory(dr);
    duda_response_printf(dr, "</div>");
    duda_response_printf(dr, "</div>");
    /* --- end first row */


    duda_response_printf(dr, "<hr>\n");
    duda_response_printf(dr, "<h2>Routing</h2>\n");
    duda_response_printf(dr, "<p class=\"muted\">\n"
                             "   the following section list the static URL maps and "
                             "   the interfaces/methods defined through the API.\n"
                             "</p>\n");

    /* Footer */
    duda_response_print(dr, DD_HTML_FOOTER, sizeof(DD_HTML_FOOTER) - 1);
    duda_response_end(dr, NULL);
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
    int fd;
    int buf_size = 1024;
    char buf[buf_size];
    char path[buf_size];
    mk_ptr_t *now;

    /* Guess we need no more than 128 bytes. */
    int n, size = 128;
    char *p, *np;
    va_list ap;

    if ((p = mk_api->mem_alloc(size)) == NULL) {
        return;
    }

    while (1) {
        /* Try to print in the allocated space. */
        va_start(ap, format);
        n = vsnprintf(p, size, format, ap);
        va_end(ap);
        /* If that worked, return the string. */
        if (n > -1 && n < size)
            break;

        size *= 2;  /* twice the old size */
        if ((np = mk_api->mem_realloc (p, size)) == NULL) {
            free(p);
            return;
        } else {
            p = np;
        }
    }

    snprintf(path, buf_size, "/tmp/%s.duda.messages",
             dr->ws_root->name.data);
    fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        perror("open");
    }

    now = mk_api->time_human();
    n = snprintf(buf, buf_size, "%s [fd=%i] [%s:%i] %s\n",
                 now->data, dr->cs->socket, file, line, p);
    n = write(fd, buf, n);
    close(fd);

    mk_api->mem_free(p);
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

    return duda_router_map(buf, duda_console_dash_home, "dashboard-home", list);
}

struct duda_api_console *duda_console_object()
{
    struct duda_api_console *c;

    c = mk_api->mem_alloc(sizeof(struct duda_api_console));
    c->_dashboard = duda_console_dashboard;
    c->_debug = duda_console_write;

    return c;
}
