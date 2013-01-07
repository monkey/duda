 /* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Daemon
 *  ------------------
 *  Copyright (C) 2001-2012, Eduardo Silva P. <edsiper@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <string.h>
#include "MKPlugin.h"

#include "duda.h"
#include "duda_conf.h"
#include "duda_fconf.h"

/*
 * @OBJ_NAME: fconf
 * @OBJ_DESC: It provides a set of methods to handle specific configuration files
 * for the web service in question. Its mandatory that the configuration key
 * 'ConfDir' exists under the [WEB_SERVICE] section for the virtual host where
 * this service is running. You can also define configuration directory using the
 * method fconf->set_path().
 */


/*
 * @METHOD_NAME: set_path
 * @METHOD_DESC: It specify a new configuration directory for the web service. It must be a valid path. If it fails it will continue using the configuration directory set on the web server virtual host definition.
 * @METHOD_PROTO: int set_path(const char *dir)
 * @METHOD_PARAM: dir directory path where the configuration files are located.
 * @METHOD_RETURN: Upon successful completion it returns 0, on error returns -1.
 */
int duda_fconf_set_path(struct web_service *ws, const char *dir)
{
    int ret;

    ret = duda_conf_set_confdir(ws, dir);
    if (ret == -1) {
        return -1;
    }

    return 0;
};

/*
 * @METHOD_NAME: get_path
 * @METHOD_DESC: It returns the configuration directory being used by the web service.
 * @METHOD_PROTO: const char *get_path()
 * @METHOD_RETURN: Upon successful completion it returns the directory path, on error or if the path is not set returns NULL.
 */
const char *duda_fconf_get_path(struct web_service *ws)
{
    return ws->confdir.data;
};

/*
 * @METHOD_NAME: read_file
 * @METHOD_DESC: Locate a named file under the web service configuration directory , read it and return a buffer with it contents.
 * @METHOD_PROTO: char *read_file()
 * @METHOD_RETURN: Upon successful completion it returns the buffered file content,
 * on error it returns NULL.
 */
char *duda_fconf_read_file(struct web_service *ws, const char *path)
{
    unsigned long len;
    char *buf;
    char *tmp;
    struct file_info finfo;

    /* Compose full path */
    mk_api->str_build(&tmp, &len, "%s/%s", ws->confdir.data, path);
    mk_api->file_get_info(tmp, &finfo);

    if (finfo.is_file == MK_FALSE) {
        mk_api->mem_free(tmp);
        return NULL;
    }

    buf = mk_api->file_to_buffer(tmp);
    mk_api->mem_free(tmp);

    /* FIXME: we need to register this buf pointer into the Garbage Collector,
     * as well add a GC trigger after duda_main() is ran, so the buffer is
     * not longer available. Wondering if developers would like to keep
     * this content in memory.
     */
    return buf;
}

/*
 * @METHOD_NAME: read_file_path
 * @METHOD_DESC: Locate a named file under the file system, read it and return a buffer with it contents.
 * @METHOD_PROTO: char *read_file_path(const char *path)
 * @METHOD_RETURN: Upon successful completion it returns the buffered file content, on error it returns NULL.
 */
char *duda_fconf_read_file_path(const char *path)
{
    char *buf;

    struct file_info finfo;

    mk_api->file_get_info(path, &finfo);
    if (finfo.is_file == MK_FALSE) {
        return NULL;
    }

    buf = mk_api->file_to_buffer(path);

    /* FIXME: we need to register this buf pointer into the Garbage Collector,
     * as well add a GC trigger after duda_main() is ran, so the buffer is
     * not longer available. Wondering if developers would like to keep
     * this content in memory.
     */

    return buf;
}

struct duda_api_fconf *duda_fconf_object()
{
    struct duda_api_fconf *c;

    c = mk_api->mem_alloc(sizeof(struct duda_api_fconf));

    /* path */
    c->_get_path = duda_fconf_get_path;
    c->_set_path = duda_fconf_set_path;

    /* read files */
    c->_read_file = duda_fconf_read_file;
    c->read_file_path = duda_fconf_read_file_path;

    return c;
}
