/* vim: et sw=4 sts=4 ts=4 : */

/* Copyright (c) 2015 Sam Wilson
 *
 * This file is part of Shock.
 *
 * Shock is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Shock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Shock.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SHOCK_HTTP_PVT_H
#define SHOCK_HTTP_PVT_H

#include <limits.h>
#include <shock/http.h>

typedef enum shock_receive_state
{
    RECEIVE_METHOD,
    RECEIVE_AFTER_METHOD,
    RECEIVE_PATH,
    RECEIVE_AFTER_PATH,
    RECEIVE_VERSION,
    RECEIVE_HEADER_NAME,
    RECEIVE_AFTER_HEADER_NAME,
    RECEIVE_HEADER_VALUE,
    RECEIVE_AFTER_HEADER_VALUE,
    RECEIVE_BODY,
    RECEIVE_BODY_CHUNKED,
    RECEIVE_BODY_LENGTH,
}
shock_receive_state;

typedef enum shock_header
{
    HEADER_UNKNOWN,
    HEADER_CONTENT_LENGTH,
    HEADER_TRANSFER_ENCODING,
}
shock_header;

typedef enum shock_send_state
{
    SEND_VERSION,
    SEND_STATUS_CODE,
    SEND_STATUS_MSG,
    SEND_HEADER_NAME,
    SEND_HEADER_VALUE,
    SEND_BODY,
}
shock_send_state;

typedef struct shock_http_pvt
{
    shock_dispatch_fn*  dispatch;
    shock_output_fn*    output;
    void*               closure;
    shock_bool          discarding;

    shock_receive_state rstate;
    shock_header header;

    shock_bool chunked;
    uintmax_t content_length;

    shock_send_state sstate;
}
shock_http_pvt;

_Static_assert(sizeof(shock_http_pvt) == sizeof(shock_http),
               "shock_http_pvt and shock_http don't match");

#endif /* SHOCK_HTTP_PVT_H */
