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

#ifndef DUDA_API_WORKER_H
#define DUDA_API_WORKER_H

/*
 * A simple struct/node to store the information of a working
 * thread that must me run at once duda_main() have been finished
 */
struct duda_worker {
    int id;
    void *(*func) (void *);
    void *arg;

    struct mk_list _head;
};

/* Worker object: worker->x() */
struct duda_api_worker {
    int (*_spawn) (void *(start_routine) (void *), void *, struct mk_list *);
};

int duda_worker_spawn_all(struct mk_list *list);
int duda_worker_spawn(void *(start_routine) (void *), void *arg, struct mk_list *list);
struct duda_api_worker *duda_worker_object();

#define spawn(routine, arg) _spawn(routine, arg, &duda_worker_list)

#endif
