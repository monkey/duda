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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "MKPlugin.h"
#include "duda.h"
#include "duda_qs.h"
#include "duda_qs_map.h"

/*
 * @OBJ_NAME: qs
 * @OBJ_DESC: The query string object provides a set of methods to manipulate the
 * incoming information from the query string URL section.
 */

struct duda_api_qs *duda_qs_object()
{
    struct duda_api_qs *qs;

    qs = mk_api->mem_alloc(sizeof(struct duda_api_qs));
    qs->count = duda_qs_count;
    return qs;
};


/*
 * @METHOD_NAME: count
 * @METHOD_DESC: It returns the number of valid parameters given in the query
 * string. A valid parameter is composed by a key and a value.
 * @METHOD_PARAM: dr the request context information hold by a duda_request_t type
 * @METHOD_RETURN: The number of valid parameters.
 */
int duda_qs_count(duda_request_t *dr)
{
    return dr->qs.count;
}


/*
 * Query string parser, if the query string section exists, it will parse the
 * content and fill the entries mapping the keys and values.
 */
int duda_qs_parse(duda_request_t *dr)
{
    int i;
    int len   = 0;
    int qs_len;
    int count = 0;
    char *key = NULL;
    char *val = NULL;
    struct duda_qs_map *qs = &dr->qs;
    struct session_request *sr = dr->sr;

    /* If we do not have a query string just return */
    if (!sr->query_string.data) {
        return -1;
    }

    qs_len = sr->query_string.len;

    /* Start parsing the query string */
    for (i=0 ; i < qs_len; i++ ) {
        switch (sr->query_string.data[i]) {
        case '&':
            if (key && val) {
                len = (i - (val - sr->query_string.data));
                qs->entries[count].value.data = val + 1;
                qs->entries[count].value.len  = len - 1;
                count++;
            }

            len = 0;
            key = NULL;
            val = NULL;
            break;
        case '=':
            /* check if the key has finished */
            if (key && !val) {
                /* set the key */
                len = (i - (key - sr->query_string.data));
                qs->entries[count].key.data = key;
                qs->entries[count].key.len  = len;

                /* set the value start */
                val = sr->query_string.data + i;
            }
            break;
        default:
            /* A new key is starting */
            if (!key) {
                key = sr->query_string.data + i;
            }
        };
    }

    if (key) {
        len = (i - (val - sr->query_string.data));
        qs->entries[count].value.data = val + 1;
        qs->entries[count].value.len  = len - 1;
        count++;
    }

    qs->count = count;

    /* DEBUG
    key = malloc(sr->query_string.len + 1);
    strncpy(key, sr->query_string.data, sr->query_string.len);
    key[sr->query_string.len] = '\0';

    printf("query string = '%s'\n", key);
    free(key);

    for (i=0; i < qs->count; i++) {
        key = malloc(qs->entries[i].key.len + 1);
        strncpy(key, qs->entries[i].key.data, qs->entries[i].key.len);
        key[qs->entries[i].key.len] = '\0';

        val = malloc(qs->entries[i].value.len + 1);
        strncpy(val, qs->entries[i].value.data, qs->entries[i].value.len);
        val[qs->entries[i].value.len] = '\0';

        printf("key='%s' ; val='%s'\n", key, val);
        free(key);
        free(val);
    }
    */

    return count;
}
