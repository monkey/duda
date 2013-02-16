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

#ifndef DUDA_QS_H
#define DUDA_QS_H

struct duda_api_qs {
    int (*count) (duda_request_t *);
    char *(*get) (duda_request_t *, const char *);
    int (*cmp) (duda_request_t *, const char *, const char *);
};


int duda_qs_parse(duda_request_t *dr);
int duda_qs_count(duda_request_t *dr);
char *duda_qs_get(duda_request_t *dr, const char *key);
int duda_qs_cmp(duda_request_t *dr, const char *key, const char *value);

struct duda_api_qs *duda_qs_object();

#endif
