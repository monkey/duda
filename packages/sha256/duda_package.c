/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2001-2013, Eduardo Silva P. <edsiper@gmail.com>
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
 * @OBJ_NAME: sha256
 * @OBJ_MENU: SHA256
 * @OBJ_DESC: The SHA256 package provides the cryptographic hash function to encode
 * any data with the SHA256 algorithm.
 * @PKG_HEADER: #include "packages/sha256/sha256.h"
 * @PKG_INIT: duda_load_package(sha256, "sha256");
 */

#include <stdlib.h>

#include "duda_package.h"
#include "sha256.h"

/*
 * @METHOD_NAME: encode
 * @METHOD_DESC: It encodes an input data stream with SHA256 algorithm and writes
 * the output to data_out buffer. Finally it stores the encoded data length in
 * the length variable.
 * @METHOD_PROTO: void encode(void *data_in, unsigned char *data_out, unsigned long length)
 * @METHOD_PARAM: data_in the source data to be encoded
 * @METHOD_PARAM: data_out the buffer where the encoded data is written
 * @METHOD_PARAM: length the length of data_out
 * @METHOD_RETURN: None
 */
static void sha256_encode(unsigned char *data_in, unsigned char *data_out,
                          int length)
{
    sha256_context ctx;

    sha256_starts(&ctx);
    sha256_update(&ctx, data_in, length);
    sha256_finish(&ctx, data_out);
}

struct duda_api_sha256 *get_sha256_api()
{
    struct duda_api_sha256 *sha256;

    /* Alloc object */
    sha256 = malloc(sizeof(struct duda_api_sha256));

    /* Map API calls */
    sha256->encode = sha256_encode;

    return sha256;
}

duda_package_t *duda_package_main(struct duda_api_objects *api)
{
    duda_package_t *dpkg;

    /* Initialize package internals */
    duda_package_init();

    /* Package object */
    dpkg = monkey->mem_alloc(sizeof(duda_package_t));
    dpkg->name = "sha256";
    dpkg->version = "0.1";
    dpkg->api = get_sha256_api();

    return dpkg;
}
