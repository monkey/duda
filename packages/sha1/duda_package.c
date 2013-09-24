/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
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
 * @OBJ_NAME: sha1
 * @OBJ_MENU: SHA1
 * @OBJ_DESC: The SHA1 package provides the cryptographic hash function to encode
 * any data with the SHA1 algorithm.
 * @PKG_HEADER: #include "packages/sha1/sha1.h"
 * @PKG_INIT: duda_load_package(sha1, "sha1");
 */

#include <stdlib.h>

#include "duda_package.h"
#include "sha1.h"

/*
 * @METHOD_NAME: encode
 * @METHOD_DESC: It encodes an input data stream with SHA1 algorithm and writes
 * the output to data_out buffer. Finally it stores the encoded data length in
 * the length variable.
 * @METHOD_PROTO: void encode(const void *data_in, unsigned char *data_out, unsigned long length)
 * @METHOD_PARAM: data_in the source data to be encoded
 * @METHOD_PARAM: data_out the buffer where the encoded data is written
 * @METHOD_PARAM: length the length of data_out
 * @METHOD_RETURN: None
 */
static void sha1_encode (const void *data_in, unsigned char *data_out,
                         unsigned long length)
{
    SHA_CTX sha;
    SHA1_Init(&sha);
    SHA1_Update(&sha, data_in, length);
    SHA1_Final(data_out, &sha);
}

struct duda_api_sha1 *get_sha1_api()
{
    struct duda_api_sha1 *sha1;

    /* Alloc object */
    sha1 = malloc(sizeof(struct duda_api_sha1));

    /* Map API calls */
    sha1->encode = sha1_encode;

    return sha1;
}

duda_package_t *duda_package_main()
{
    duda_package_t *dpkg;

    /* Package object */
    dpkg = monkey->mem_alloc(sizeof(duda_package_t));
    dpkg->name = "sha1";
    dpkg->version = "0.1";
    dpkg->api = get_sha1_api();

    return dpkg;
}
