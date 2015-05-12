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

#ifndef SHOCK_HTTP_H
#define SHOCK_HTTP_H

#include <shock/common.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>

typedef enum shock_request_part
{
    SHOCK_REQUEST_METHOD,
    SHOCK_REQUEST_PATH,
    SHOCK_REQUEST_VERSION,
    SHOCK_REQUEST_HEADER_NAME,
    SHOCK_REQUEST_HEADER_VALUE,
    SHOCK_REQUEST_BODY,

    SHOCK_REQUEST_COMPLETE,
}
shock_request_part;

typedef enum shock_response_part
{
    SHOCK_RESPONSE_VERSION,
    SHOCK_RESPONSE_STATUS_CODE,
    SHOCK_RESPONSE_STATUS_MSG,
    SHOCK_RESPONSE_HEADER_NAME,
    SHOCK_RESPONSE_HEADER_VALUE,
    SHOCK_RESPONSE_BODY,
}
shock_response_part;

typedef struct shock_http shock_http;

typedef void (shock_output_fn)(shock_http* self,
                               void* closure,
                               const char* data,
                               size_t length);
typedef void (shock_dispatch_fn)(shock_http* self,
                                 void* closure,
                                 shock_request_part state,
                                 const char* data,
                                 size_t length);

struct shock_http
{
    SHOCK_PVT(shock_dispatch_fn*);
    SHOCK_PVT(shock_output_fn*);
    SHOCK_PVT(void*);
    SHOCK_PVT(shock_bool);

    SHOCK_PVT(int);
    SHOCK_PVT(int);
    SHOCK_PVT(shock_bool);
    SHOCK_PVT(unsigned long);

    SHOCK_PVT(int);
};

SHOCK_API
void shock_http_init(shock_http* self,
                     shock_dispatch_fn* dispatch,
                     shock_output_fn* output,
                     void* closure);

SHOCK_API
void shock_http_fini(shock_http* self);

SHOCK_API
size_t shock_http_recv(shock_http* self, const char* data, size_t length);

SHOCK_API
void shock_http_send(shock_http* self,
                     shock_response_part part,
                     const char* data,
                     size_t length);

SHOCK_API
void shock_http_drop(shock_http* self);

#ifdef __cplusplus
}
#endif

#endif /* SHOCK_HTTP_H */
