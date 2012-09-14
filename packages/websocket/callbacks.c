/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda Framework
 *  ------------------
 *  Copyright (C) 2012, Eduardo Silva P. <edsiper@gmail.com>
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

#include "duda_api.h"
#include "callbacks.h"

/*
 * @METHOD_NAME: set_callback
 * @METHOD_DESC: It set a specific callback function against on a received event.
 * @METHOD_PROTO: int set_callback(int type, void (*callback) (duda_request_t *, struct ws_request *))
 * @METHOD_PARAM: type it specifies one of the following event types: WS_ON_MESSAGE, WS_ON_ERROR, WS_ON_CLOSE or WS_ON_TIMEOUT.
 * @METHOD_PARAM: callback the callback function
 * @METHOD_RETURN: It returns zero when the callback is set, otherwise -1 on errir
 */

int ws_set_callback(int type, void (*callback) (duda_request_t *, struct ws_request *))
{
    switch (type) {
        case WS_ON_OPEN:
            ws_callbacks->on_open = callback;
            break;
        case WS_ON_MESSAGE:
            ws_callbacks->on_message = callback;
            break;
        case WS_ON_ERROR:
            ws_callbacks->on_error = callback;
            break;
        case WS_ON_CLOSE:
            ws_callbacks->on_close = callback;
            break;
        case WS_ON_TIMEOUT:
            ws_callbacks->on_timeout = callback;
            break;
        default:
            printf("Error: invalid callback type %i\n", type);
            return -1;
        };

    return 0;
}
