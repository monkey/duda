/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Daemon
 *  ------------------
 *  Copyright (C) 2001-2012, Eduardo Silva P.
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

/*
 * Duda objects are struct passed by references to third party components of the framework,
 * like web services or packages.
 */

#ifndef DUDA_OBJECTS_H
#define DUDA_OBJECTS_H

/* Objects exported to the web service */
struct plugin_api *monkey;
struct duda_api_map *map;
struct duda_api_msg *msg;
struct duda_api_response *response;
struct duda_api_debug *debug;
struct duda_api_event *event;
struct duda_api_console *console;
struct duda_api_param *param;
struct duda_api_session *session;
struct duda_api_cookie *cookie;
struct duda_api_global *global;
struct duda_api_xtime *xtime;

#endif
