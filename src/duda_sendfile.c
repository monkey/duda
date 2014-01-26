/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2012-2014, Eduardo Silva P. <edsiper@gmail.com>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "MKPlugin.h"
#include "duda_sendfile.h"

struct duda_sendfile *duda_sendfile_new(char *path, off_t offset,
                                        size_t count)
{
    int ret;
    uint64_t fsize;
    struct duda_sendfile *file;

    if (offset < 0) {
        return NULL;
    }

    file = mk_api->mem_alloc(sizeof(struct duda_sendfile));
    file->fd = -1;

    ret = mk_api->file_get_info(path, &file->info);
    if (ret == -1) {
        mk_api->mem_free(file);
        return NULL;
    }

    fsize = file->info.size;
    if ((unsigned) offset > fsize ||
        count > fsize ||
        ((unsigned) offset + count) > fsize) {
        mk_api->mem_free(file);
        return NULL;
    }

    if (file->info.read_access == MK_FALSE) {
        mk_api->mem_free(file);
        return NULL;
    }

    file->fd = open(path, O_RDONLY | O_NONBLOCK);
    if (file->fd < 0) {
        mk_warn("%s:%i %s", __FILE__, __LINE__, strerror(errno));
        mk_api->mem_free(file);
        return NULL;
    }

    file->offset = offset;
    if (count == 0) {
        file->pending_bytes = file->info.size - file->offset;
    }
    else {
        file->pending_bytes = count;
    }

    return file;
}

int duda_sendfile_flush(int socket, struct duda_sendfile *sf)
{
    int bytes;

    bytes = mk_api->socket_send_file(socket, sf->fd,
                                     &sf->offset, sf->pending_bytes);

    if (bytes > 0) {
        sf->pending_bytes -= bytes;
    }
    else if (bytes == -1) {
        sf->pending_bytes = 0;
        return 0;
    }

    return sf->pending_bytes;
}
