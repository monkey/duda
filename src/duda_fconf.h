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

#include "mk_string.h"
#include "duda_api.h"

#ifndef MK_DUDA_FCONF_H
#define MK_DUDA_FCONF_H

/* config constants */
#define DUDA_CONFIG_STR    MK_CONFIG_VAL_STR
#define DUDA_CONFIG_NUM    MK_CONFIG_VAL_NUM
#define DUDA_CONFIG_BOOL   MK_CONFIG_VAL_BOOL
#define DUDA_CONFIG_LIST   MK_CONFIG_VAL_LIST

#define duda_string_line   mk_string_line

/* Remap configuration structures from Monkey core */
struct duda_config
{
    int created;
    char *file;

    /* list of sections */
    struct mk_list sections;
};

struct duda_config_section
{
    char *name;

    struct mk_list entries;
    struct mk_list _head;
};

struct duda_config_entry
{
    char *key;
    char *val;

    struct mk_list _head;
};


/* Object API */

struct duda_api_fconf {
    #define get_path() _get_path(self)
    const char *(*_get_path) (struct web_service *);

    #define set_path(dir)  _set_path(self, dir)
    int (*_set_path) (struct web_service *, const char *);

    #define read_file(f) _read_file(self, f)
    char *(*read_file) (const char *);

    /* --- specific handlers for struct duda_config --- */
    #define read_conf(f)   _read_conf(self, f)
    struct duda_config *(*_read_conf) (struct web_service *, const char *);

    void (*free_conf) (struct duda_config *);
    struct duda_config_section *(*section_get) (struct duda_config *,
                                                const char *);
    void *(*section_key) (struct duda_config_section *, char *, int);
};

struct duda_api_fconf *duda_fconf_object();

#endif
