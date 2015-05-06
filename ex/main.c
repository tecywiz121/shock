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

#include <shock/http.h>
#include <memory.h>
#include <stdio.h>

static void dispatch(shock_http*        self,
                     void*              closure,
                     shock_request_part state,
                     char*              data,
                     size_t             length)
{
    char text[length+1];
    memcpy(text, data, length);
    text[length] = '\0';

    fprintf(stderr, "Part (%d): %s\n", state, text);

    if (SHOCK_REQUEST_COMPLETE == state) {
        shock_http_send(self, SHOCK_RESPONSE_VERSION, "HTTP/1.1", 8);
        shock_http_send(self, SHOCK_RESPONSE_STATUS_CODE, "200", 3);
        shock_http_send(self, SHOCK_RESPONSE_STATUS_MSG, "OK", 2);
        shock_http_send(self, SHOCK_RESPONSE_BODY, "hello world", 11);
        shock_http_send(self, SHOCK_RESPONSE_BODY, "", 0);
    }
}

static void output(shock_http* self,
                   void* closure,
                   char* data,
                   size_t length)
{
    fwrite(data, 1, length, stdout);
}

int main(int argc, char** argv)
{
    shock_http srv;

    shock_http_init(&srv, &dispatch, &output, NULL);

    char input[512];
    while (fgets(input, sizeof(input), stdin) != NULL) {
        // XXX: Doesn't properly handle split reads or NULL bytes
        fprintf(stderr, "Read %zu bytes.\n", strlen(input));
        shock_http_recv(&srv, input, strlen(input));
    }

    shock_http_fini(&srv);
}
