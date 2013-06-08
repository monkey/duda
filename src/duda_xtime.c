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

#include "MKPlugin.h"
#include "duda_xtime.h"

/*
 * @OBJ_NAME: xtime
 * @OBJ_MENU: Time Handling
 * @OBJ_DESC: The xtime object provides a set of methods to retrieve and manipulate
 * the timing information based on unix timestamp
 */

struct duda_api_xtime *duda_xtime_object() {
    struct duda_api_xtime *t;

    t = mk_api->mem_alloc(sizeof(struct duda_api_xtime));
    t->now = duda_xtime_now;
    t->tomorrow = duda_xtime_tomorrow;
    t->next_hours = duda_xtime_next_hours;

    return t;
}

/*
 * @METHOD_NAME: now
 * @METHOD_DESC: Returns the current time in unix timestamp format
 * @METHOD_RETURN: Upon successful completion it returns the unix timestamp
 */
time_t duda_xtime_now()
{
    return mk_api->time_unix();
}

/*
 * @METHOD_NAME: tomorrow
 * @METHOD_DESC: Returns the unix timestamp for the next 24 hours from now
 * @METHOD_RETURN: Upon successful completion it returns the unix timestamp
 */
time_t duda_xtime_tomorrow()
{
    return (mk_api->time_unix() + TIME_DAY);
}

/*
 * @METHOD_NAME: next_hours
 * @METHOD_DESC: Returns the unix timestamp for the given next hours
 * @METHOD_PARAM: h Represent the number of hours to perform the offset
 * @METHOD_RETURN: Upon successful completion it returns the unix timestamp
 */
time_t duda_xtime_next_hours(int h)
{
    return (mk_api->time_unix() + (h * TIME_HOUR));
}
