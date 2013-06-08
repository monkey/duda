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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <string.h>
#include "MKPlugin.h"

#include "duda.h"
#include "duda_gc.h"
#include "duda_data.h"

/*
 * @OBJ_NAME: data
 * @OBJ_MENU: Data
 * @OBJ_DESC: It provides a set of methods to handle the web service data directory.
 * A data directory aims to provide a local storage point for specific files and directories
 * handled by the web services. A data directory can be set manually in the virtual host
 * configuration using the DataDir key under the [WEB_SERVICE] section or through the API
 * exposed in this documentation.
 */


/*
 * @METHOD_NAME: set_path
 * @METHOD_DESC: It specify a new data directory for the web service. It must be a valid path. If it fails it will continue using the configuration directory set on the web server virtual host definition.
 * @METHOD_PROTO: int set_path(const char *dir)
 * @METHOD_PARAM: dir directory path where the configuration files are located.
 * @METHOD_RETURN: Upon successful completion it returns 0, on error returns -1.
 */
int duda_data_set_path(struct web_service *ws, const char *dir)
{
    int ret;

    ret = duda_conf_set_datadir(ws, dir);
    if (ret == -1) {
        return -1;
    }

    return 0;
};

/*
 * @METHOD_NAME: get_path
 * @METHOD_DESC: It returns the data  directory being used by the web service.
 * @METHOD_PROTO: const char *get_path()
 * @METHOD_RETURN: Upon successful completion it returns the directory path, on error or if the path is not set returns NULL.
 */
const char *duda_data_get_path(struct web_service *ws)
{
    return ws->datadir.data;
};

/*
 * @METHOD_NAME: locate
 * @METHOD_DESC: It compose the absolute path for a given file or directory name using as
 * reference the web service data directory. This method can be used only inside a
 * callback context. The resulting memory buffer is freed by the garbage collector once the
 * callback finish it works.
 * @METHOD_PROTO: char *locate(const char *filename)
 * @METHOD_PARAM: filename the name of the file or directory entry
 * @METHOD_RETURN: Upon successful completion it returns the location path, on error or if the path is not set returns NULL.
 */
char *duda_data_locate(duda_request_t *dr, const char *filename)
{
    int len;
    int flen;
    char *path;
    struct web_service *ws = dr->ws_root;

    /* some minor validations */
    if (!ws) {
        return NULL;
    }

    if (!ws->datadir.data) {
        return NULL;
    }

    if (!filename) {
        return NULL;
    }

    /* compose the new buffer */
    flen = strlen(filename);
    len = ws->datadir.len + flen + 1;
    path = mk_api->mem_alloc(len);
    memcpy(path, ws->datadir.data, ws->datadir.len);
    memcpy(path + ws->datadir.len, filename, flen);
    path[len] = '\0';

    /* register in the garbage collector */
    duda_gc_add(dr, path);

    return path;
}

struct duda_api_data *duda_data_object()
{
    struct duda_api_data *d;

    d = mk_api->mem_alloc(sizeof(struct duda_api_data));

    /* path */
    d->_get_path = duda_data_get_path;
    d->_set_path = duda_data_set_path;
    d->_locate   = duda_data_locate;

    return d;
}
