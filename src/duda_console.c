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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

#include "MKPlugin.h"
#include "duda.h"
#include "duda_map.h"
#include "duda_api.h"
#include "duda_conf.h"

/*
 * @OBJ_NAME: Console
 * @OBJ_DESC: The console object provides a set of methods to write debug information
 * to the console URL interface of the running web service.
 */

struct duda_api_console *duda_console_object()
{
    struct duda_api_console *c;

    c = mk_api->mem_alloc(sizeof(struct duda_api_console));
    c->_debug = duda_console_write;

    return c;
}

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

/* callback for /app/console/map */
void duda_console_cb_map(duda_request_t *dr)
{
    struct web_service *ws = dr->ws_root;
    struct mk_list *head;

    char *header = "<html><head>"
                          "<title>Duda Map: %s</title>"
                          " %s"
                          "</head>"
                    "<body>\n";
    char *css    =  "<link href=\"/ddr/bootstrap/css/bootstrap.css\" rel=\"stylesheet\">";
    char *footer = "</body></html>\n";

    duda_response_http_status(dr, 200);
    duda_response_http_header(dr, "Content-Type: text/html");

    /* Header */
    duda_response_printf(dr, header, dr->ws_root->name.data, css);
    duda_response_printf(dr, "<div class=\"container\">"
                             "  <div class=\"row\">"
                                   "<h1>%s/</h1>\n"
                                       "<address>\n"
                                       "  Routing and URL map for <strong>%s</strong> web service<br>\n"
                                       "</address>\n",
                         dr->ws_root->name.data, dr->ws_root->name.data);


    /* <ul> */

    duda_response_printf(dr, "<hr>\n");
    duda_response_printf(dr, "<h2>Routing</h2>");
    duda_response_printf(dr, "<p class=\"muted\">"
                             "   the following section list the static URL maps and "
                             "   the interfaces/methods defined through the API."
                             "</p>");

    /*
     * List the service defined based on static map routes
     */
    struct duda_map_static_cb *st;

    duda_response_printf(dr, "<h3>Static maps</h3>");
    duda_response_printf(dr, "<table class=\"table table-bordered table-hover table-condensed\">"
                         "<thead>"
                         "<tr>"
                         "  <th>Route</th>"
                         "  <th>Callback</th>"
                         "</tr>"
                         "</thead>"
                         "<tbody>");

    mk_list_foreach(head, dr->ws_root->map_urls) {
        st = mk_list_entry(head, struct duda_map_static_cb, _head);
        duda_response_printf(dr,
                             "<tr>"
                             "<td>%s</td>"
                             "<td>%s</td>"
                             "</tr>",
                             st->path, st->cb_name
                             );
    }
    duda_response_printf(dr, "</tbody></table>");

    /*
     * List service defined Map-Interfaces
     */
    struct duda_interface *iface;
    struct duda_method *method;
    struct mk_list *head_method;
    duda_response_printf(dr, "<h3>Interfaces and methods</h3>");
    duda_response_printf(dr, "<table class=\"table table-bordered table-hover table-condensed\">"
                         "<thead>"
                         "<tr>"
                         "  <th>Interface</th>"
                         "  <th>Method</th>"
                         "  <th>Callback</th>"
                         "</tr>"
                         "</thead>"
                         "<tbody>");
    mk_list_foreach(head, ws->map_interfaces) {
        iface = mk_list_entry(head, struct duda_interface, _head);

        /* Print the interface */
        duda_response_printf(dr,
                             "<tr class=\"success\">"
                             "<td>%s/</td>"
                             "<td></td>"
                             "<td></td>"
                             "</tr>",
                             iface->uid);

        /* Print the methods associated */
        mk_list_foreach(head_method, &iface->methods) {
            method = mk_list_entry(head_method, struct duda_method, _head);

            if (method->callback) {
                duda_response_printf(dr,
                                     "<tr>"
                                     "<td></td>"
                                     "<td>%s</td>"
                                     "<td>%s</td>"
                                     "</tr>",
                                     method->uid, method->callback);
            }
            else {
                duda_response_printf(dr,
                                     "<tr>"
                                     "<td></td>"
                                     "<td>%s</td>"
                                     "<td></td>"
                                     "</tr>",
                                     method->uid, method->callback);
            }
        }
    }

    /* Footer */
    duda_response_print(dr, footer, strlen(footer));
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
    mk_pointer *now;

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
        if ((np = realloc (p, size)) == NULL) {
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
