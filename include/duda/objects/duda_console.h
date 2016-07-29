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

#ifndef DUDA_CONSOLE_H
#define DUDA_CONSOLE_H

#include <duda/duda.h>

#define DD_HTML_HEADER                            \
    "<html>\n"                                    \
    "<head>\n"                                    \
    "    <title>Duda I/O - %s</title>\n"          \
    "%s\n"                                        \
    "</head>\n"

#define DD_HTML_CSS                                                     \
    "    <link href=\"/ddr/bootstrap/css/bootstrap.min.css\" rel=\"stylesheet\">\n" \
    "    <link href=\"/ddr/css/duda.css\" rel=\"stylesheet\">\n"

#define DD_HTML_NAVBAR_BASIC                                            \
    "<div class=\"navbar navbar-inverse navbar-fixed-top\" role=\"navigation\">\n" \
    "   <div class=\"container\">\n"                                    \
    "        <div class=\"navbar-header\">\n"                           \
    "            <button type=\"button\" class=\"navbar-toggle\" data-toggle=\"collapse\" data-target=\".navbar-collapse\">\n" \
    "                <span class=\"sr-only\">Toggle navigation</span>\n" \
    "                <span class=\"icon-bar\"></span>\n"                \
    "                <span class=\"icon-bar\"></span>\n"                \
    "                <span class=\"icon-bar\"></span>\n"                \
    "            </button>\n"                                           \
    "            <img src=\"/ddr/imgs/duda-logo-header.png\">\n"        \
    "        </div>\n"                                                  \
    "        <ul class=\"nav navbar-nav navbar-right\">"                \
    "            <li><a href=\"%s\">%s</a></li>"                        \
    "        </ul>"                                                     \
    "    </div>\n"                                                      \
    "</div>\n"

#define DD_HTML_FOOTER "</body></html>\n"

#define DD_HTML_PANEL_HEADER                                        \
    "<div class='panel panel-%s'>\n"                                \
    "  <div class='panel-heading'>\n"                               \
    "    <h3 class='panel-title'>\n"                                \
    "       %s\n"                                                   \
    "    </h3>\n"                                                   \
    "  </div>\n"                                                    \
    "  <div class='panel-body'>\n"

#define DD_HTML_PANEL_FOOTER                                            \
    "</div>\n"                                                          \
    "<div class='panel-footer'>\n"                                      \
    "    %s\n"                                                          \
    "</div>\n"                                                          \
    "</div>"


#define console_debug(dr, fmt, ...) duda_console_write(dr, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

struct duda_api_console {
    #define debug(dr, fmt, ...) _debug(dr, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
    void (*_debug) (duda_request_t *, char *, int, char *, ...);

    #define dashboard(uri) _dashboard(uri, self);
    int (*_dashboard) (char *uri, struct web_service *ws);
};

struct duda_api_console *duda_console_object();

int duda_console_enable(char *map, struct mk_list *list);
void duda_console_cb_messages(duda_request_t *dr);
void duda_console_cb_map(duda_request_t *dr);
void duda_console_write(duda_request_t *dr,
                        char *file, int line,
                        char *format, ...);

#endif
