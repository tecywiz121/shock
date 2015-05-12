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

#include <stdio.h>
#include "cheat.h"

#include <shock/http.h>

// Stops my YouCompleteMe from freaking out
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

CHEAT_DECLARE(
    shock_http*         g_dispatch_srv;
    unsigned int        g_dispatch_count;
    void*               g_dispatch_closure;
    shock_request_part  g_dispatch_part;
    const char*         g_dispatch_data;
    size_t              g_dispatch_length;

    shock_http*         g_output_srv;
    const char*         g_output_data;
    size_t              g_output_length;
    size_t              g_output_count;

    void dispatch(shock_http* srv,
                  void* closure,
                  shock_request_part state,
                  const char* data,
                  size_t length)
    {
        g_dispatch_srv = srv;
        g_dispatch_closure = closure;
        g_dispatch_part = state;
        g_dispatch_data = data;
        g_dispatch_length = length;
        g_dispatch_count += 1;
    }

    void output(shock_http* srv,
                void* closure,
                const char* data,
                size_t len)
    {
        g_output_srv = srv;
        g_output_data = data;
        g_output_length = len;
        g_output_count += 1;
    }
)

CHEAT_TEST(shock_http_init,
    shock_http srv;
    shock_http_init(&srv, dispatch, output, NULL);
    shock_http_fini(&srv);
)


CHEAT_DECLARE(
    shock_http g_srv;
)

CHEAT_SET_UP(
    shock_http_init(&g_srv, dispatch, output, NULL);
    g_dispatch_srv = NULL;
    g_dispatch_closure = NULL;
    g_dispatch_part = 0;
    g_dispatch_data = NULL;
    g_dispatch_length = 0;
    g_dispatch_count = 0;

    g_output_srv = NULL;
    g_output_data = NULL;
    g_output_length = 0;
    g_output_count = 0;
)

CHEAT_TEAR_DOWN(
    shock_http_fini(&g_srv);
)

CHEAT_TEST(shock_http_recv_method,
    char buf[] = "hello ";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 6);
    cheat_assert(g_dispatch_count == 1);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_METHOD);
    cheat_assert(g_dispatch_data == &buf[0]);
    cheat_assert(g_dispatch_length == 5);
    cheat_assert(memcmp(g_dispatch_data, "hello", 5) == 0);
)

CHEAT_TEST(shock_http_recv_method_partial,
    char buf[] = "hel";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 0);
    cheat_assert(g_dispatch_count == 0);
)

CHEAT_TEST(shock_http_drop_method,
    char buf[] = "hello ";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte

    shock_http_drop(&g_srv);
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 6);
    cheat_assert(g_dispatch_count == 0);

    char buf2[] = "world ";
    length = sizeof(buf2)/sizeof(char) - 1;         // Still don't count null.

    length = shock_http_recv(&g_srv, buf2, length);
    cheat_assert(length == 6);
    cheat_assert(g_dispatch_count == 1);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_PATH);
    cheat_assert(g_dispatch_data == buf2);
    cheat_assert(g_dispatch_length == 5);
)

CHEAT_TEST(shock_http_drop_method_partial,
    char buf[] = "hel";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte

    shock_http_drop(&g_srv);
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 3);
    cheat_assert(g_dispatch_count == 0);

    char buf2[] = " world ";
    length = sizeof(buf2)/sizeof(char) - 1;         // Still don't count null.

    length = shock_http_recv(&g_srv, buf2, length);
    cheat_assert(length == 7);
    cheat_assert(g_dispatch_count == 1);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_PATH);
    cheat_assert(g_dispatch_data == buf2+1);
    cheat_assert(g_dispatch_length == 5);
)

CHEAT_TEST(shock_http_recv_after_method,
    char buf[] = "hello /";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 6);                      // 5 from hello, 1 from ' '
    cheat_assert(g_dispatch_count == 1);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_METHOD);
    cheat_assert(g_dispatch_data == &buf[0]);
    cheat_assert(g_dispatch_length == 5);
    cheat_assert(memcmp(g_dispatch_data, "hello", 5) == 0);
)

CHEAT_TEST(shock_http_recv_after_method_partial,
    char buf[] = "hello  ";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 7);                      // 5 from hello, 2 from ' '
    cheat_assert(g_dispatch_count == 1);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_METHOD);
    cheat_assert(g_dispatch_data == &buf[0]);
    cheat_assert(g_dispatch_length == 5);
    cheat_assert(memcmp(g_dispatch_data, "hello", 5) == 0);
)

CHEAT_TEST(shock_http_recv_path,
    char buf[] = "hello /world ";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 13);
    cheat_assert(g_dispatch_count == 2);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_PATH);
    cheat_assert(g_dispatch_data == &buf[6]);
    cheat_assert(g_dispatch_length == 6);
    cheat_assert(memcmp(g_dispatch_data, "/world", 6) == 0);
)

CHEAT_TEST(shock_http_recv_path_partial,
    char buf[] = "hello /worl";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 6);                      // 5 from hello, 1 from ' '
    cheat_assert(g_dispatch_count == 1);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_METHOD);
    cheat_assert(g_dispatch_data == &buf[0]);
    cheat_assert(g_dispatch_length == 5);
    cheat_assert(memcmp(g_dispatch_data, "hello", 5) == 0);
)

CHEAT_TEST(shock_http_drop_path,
    char buf[] = "hello ";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 6);
    cheat_assert(g_dispatch_count == 1);

    char buf2[] = "/world ";
    length = sizeof(buf2)/sizeof(char) - 1;

    shock_http_drop(&g_srv);
    length = shock_http_recv(&g_srv, buf2, length);

    cheat_assert(length == 7);
    cheat_assert(g_dispatch_count == 1);
)

CHEAT_TEST(shock_http_drop_path_partial,
    char buf[] = "hello ";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 6);
    cheat_assert(g_dispatch_count == 1);

    char buf2[] = "/wor";
    length = sizeof(buf2)/sizeof(char) - 1;

    shock_http_drop(&g_srv);
    length = shock_http_recv(&g_srv, buf2, length);

    cheat_assert(length == 4);
    cheat_assert(g_dispatch_count == 1);
)


CHEAT_TEST(shock_http_recv_after_path,
    char buf[] = "hello /world H";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 13);
    cheat_assert(g_dispatch_count == 2);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_PATH);
    cheat_assert(g_dispatch_data == &buf[6]);
    cheat_assert(g_dispatch_length == 6);
    cheat_assert(memcmp(g_dispatch_data, "/world", 6) == 0);
)

CHEAT_TEST(shock_http_recv_after_path_partial,
    char buf[] = "hello /world  ";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 14);
    cheat_assert(g_dispatch_count == 2);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_PATH);
    cheat_assert(g_dispatch_data == &buf[6]);
    cheat_assert(g_dispatch_length == 6);
    cheat_assert(memcmp(g_dispatch_data, "/world", 6) == 0);
)

CHEAT_TEST(shock_http_recv_after_path_partial2,
    char buf[] = "hello /world   ";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 15);
    cheat_assert(g_dispatch_count == 2);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_PATH);
    cheat_assert(g_dispatch_data == &buf[6]);
    cheat_assert(g_dispatch_length == 6);
    cheat_assert(memcmp(g_dispatch_data, "/world", 6) == 0);
)

CHEAT_TEST(shock_http_recv_version,
    char buf[] = "hello /world HTTP/1.1\r\n";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 23);
    cheat_assert(g_dispatch_count == 3);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_VERSION);
    cheat_assert(g_dispatch_data == &buf[13]);
    cheat_assert(g_dispatch_length == 8);
    cheat_assert(memcmp(g_dispatch_data, "HTTP/1.1", 8) == 0);
)

CHEAT_TEST(shock_http_recv_version_no_cr,
    char buf[] = "hello /world HTTP/1.1\n";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 22);
    cheat_assert(g_dispatch_count == 3);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_VERSION);
    cheat_assert(g_dispatch_data == &buf[13]);
    cheat_assert(g_dispatch_length == 8);
    cheat_assert(memcmp(g_dispatch_data, "HTTP/1.1", 8) == 0);
)

CHEAT_TEST(shock_http_drop_version,
    char buf[] = "hello /world ";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 13);
    cheat_assert(g_dispatch_count == 2);

    char buf2[] = "HTTP/1.1\r\n";
    length = sizeof(buf2)/sizeof(char) - 1;         // Still don't count it.

    shock_http_drop(&g_srv);
    length = shock_http_recv(&g_srv, buf2, length);

    cheat_assert(length == 10);
    cheat_assert(g_dispatch_count == 2);

    char buf3[] = "Host: ";
    length = sizeof(buf3)/sizeof(char) - 1;         // Damn NULL byte.
    length = shock_http_recv(&g_srv, buf3, length);

    cheat_assert(length == 6);
    cheat_assert(g_dispatch_count == 3);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_HEADER_NAME);
    cheat_assert(g_dispatch_data == buf3);
    cheat_assert(g_dispatch_length == 4);
)

CHEAT_TEST(shock_http_drop_version_partial,
    char buf[] = "hello /world ";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 13);
    cheat_assert(g_dispatch_count == 2);

    char buf2[] = "HTTP/1.1\r";
    length = sizeof(buf2)/sizeof(char) - 1;         // Still don't count it.

    shock_http_drop(&g_srv);
    length = shock_http_recv(&g_srv, buf2, length);

    cheat_assert(length == 9);
    cheat_assert(g_dispatch_count == 2);

    char buf3[] = "\nHost: ";
    length = sizeof(buf3)/sizeof(char) - 1;         // Damn NULL byte.
    length = shock_http_recv(&g_srv, buf3, length);

    cheat_assert(length == 7);
    cheat_assert(g_dispatch_count == 3);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_HEADER_NAME);
    cheat_assert(g_dispatch_data == &buf3[1]);
    cheat_assert(g_dispatch_length == 4);
)

CHEAT_TEST(shock_http_recv_version_partial,
    char buf[] = "hello /world HTTP/1.";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 13);
    cheat_assert(g_dispatch_count == 2);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_PATH);
    cheat_assert(g_dispatch_data == &buf[6]);
    cheat_assert(g_dispatch_length == 6);
    cheat_assert(memcmp(g_dispatch_data, "/world", 6) == 0);
)

CHEAT_TEST(shock_http_recv_header_name,
    char buf[] = "hello /world HTTP/1.1\r\n"
                 "Host:";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 28);
    cheat_assert(g_dispatch_count == 4);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_HEADER_NAME);
    cheat_assert(g_dispatch_data == &buf[23]);
    cheat_assert(g_dispatch_length == 4);
    cheat_assert(memcmp(g_dispatch_data, "Host", 4) == 0);
)

CHEAT_TEST(shock_http_recv_header_name_partial,
    char buf[] = "hello /world HTTP/1.1\r\n"
                 "Host";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 23);
    cheat_assert(g_dispatch_count == 3);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_VERSION);
    cheat_assert(g_dispatch_data == &buf[13]);
    cheat_assert(g_dispatch_length == 8);
    cheat_assert(memcmp(g_dispatch_data, "HTTP/1.1", 8) == 0);
)

CHEAT_TEST(shock_http_recv_header_value,
    char buf[] = "hello /world HTTP/1.1\r\n"
                 "Host: www.example.com\r\n";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 46);
    cheat_assert(g_dispatch_count == 5);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_HEADER_VALUE);
    cheat_assert(g_dispatch_data == &buf[29]);
    cheat_assert(g_dispatch_length == 15);
    cheat_assert(memcmp(g_dispatch_data, "www.example.com", 15) == 0);
)

CHEAT_TEST(shock_http_drop_header_name,
    char buf[] = "g / 1\n";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 6);
    cheat_assert(g_dispatch_count == 3);

    char buf2[] = "k: ";
    length = sizeof(buf2)/sizeof(char) - 1;

    shock_http_drop(&g_srv);
    length = shock_http_recv(&g_srv, buf2, length);

    cheat_assert(length == 3);
    cheat_assert(g_dispatch_count == 3);

    char buf3[] = "v\n";
    length = 2;

    length = shock_http_recv(&g_srv, buf3, length);

    cheat_assert(length == 2);
    cheat_assert(g_dispatch_count == 4);
    cheat_assert(g_dispatch_data == buf3);
    cheat_assert(g_dispatch_length == 1);
)

CHEAT_TEST(shock_http_drop_header_name_partial,
    char buf[] = "g / 1\n";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 6);
    cheat_assert(g_dispatch_count == 3);

    char buf2[] = "k";
    length = sizeof(buf2)/sizeof(char) - 1;

    shock_http_drop(&g_srv);
    length = shock_http_recv(&g_srv, buf2, length);

    cheat_assert(length == 1);
    cheat_assert(g_dispatch_count == 3);

    char buf3[] = ": v\n";
    length = 4;

    length = shock_http_recv(&g_srv, buf3, length);

    cheat_assert(length == 4);
    cheat_assert(g_dispatch_count == 4);
    cheat_assert(g_dispatch_data == &buf3[2]);
    cheat_assert(g_dispatch_length == 1);
)

CHEAT_TEST(shock_http_recv_header_value_partial,
    char buf[] = "hello /world HTTP/1.1\r\n"
                 "Host: www.example.com\r";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 29);
    cheat_assert(g_dispatch_count == 4);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_HEADER_NAME);
    cheat_assert(g_dispatch_data == &buf[23]);
    cheat_assert(g_dispatch_length == 4);
    cheat_assert(memcmp(g_dispatch_data, "Host", 4) == 0);
)

CHEAT_TEST(shock_http_recv_header_name_two,
    char buf[] = "hello /world HTTP/1.1\r\n"
                 "Host: www.example.com\r\n"
                 "Accept-Language:";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 62);
    cheat_assert(g_dispatch_count == 6);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_HEADER_NAME);
    cheat_assert(g_dispatch_data == &buf[46]);
    cheat_assert(g_dispatch_length == 15);
    cheat_assert(memcmp(g_dispatch_data, "Accept-Language", 15) == 0);
)

CHEAT_TEST(shock_http_drop_header_value,
    char buf[] = "h / 1\nt: ";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 9);
    cheat_assert(g_dispatch_count == 4);

    char buf2[] = "value\n";
    length = sizeof(buf2)/sizeof(char) - 1;         // Still no NULL!

    shock_http_drop(&g_srv);
    length = shock_http_recv(&g_srv, buf2, length);

    cheat_assert(length == 6);
    cheat_assert(g_dispatch_count == 4);

    char buf3[] = "k: ";
    length = sizeof(buf3)/sizeof(char) - 1;

    length = shock_http_recv(&g_srv, buf3, length);

    cheat_assert(length == 3);
    cheat_assert(g_dispatch_count == 5);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_HEADER_NAME);
    cheat_assert(g_dispatch_length == 1);
)

CHEAT_TEST(shock_http_drop_header_value_partial,
    char buf[] = "h / 1\nt: ";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 9);
    cheat_assert(g_dispatch_count == 4);

    char buf2[] = "value";
    length = sizeof(buf2)/sizeof(char) - 1;         // Still no NULL!

    shock_http_drop(&g_srv);
    length = shock_http_recv(&g_srv, buf2, length);

    cheat_assert(length == 5);
    cheat_assert(g_dispatch_count == 4);

    char buf3[] = "\nk: ";
    length = sizeof(buf3)/sizeof(char) - 1;

    length = shock_http_recv(&g_srv, buf3, length);

    cheat_assert(length == 4);
    cheat_assert(g_dispatch_count == 5);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_HEADER_NAME);
    cheat_assert(g_dispatch_length == 1);
)

CHEAT_TEST(shock_http_recv_body_length,
    char buf[] = "hello /world HTTP/1.1\r\n"
                 "Host: www.example.com\r\n"
                 "Content-Length: 6\r\n"
                 "\r\n"
                 "banana";
    size_t length = sizeof(buf)/sizeof(char) - 1;   // Don't count null byte
    length = shock_http_recv(&g_srv, buf, length);

    cheat_assert(length == 73);
    cheat_assert(g_dispatch_count == 8);
    cheat_assert(g_dispatch_srv == &g_srv);
    cheat_assert(g_dispatch_closure == NULL);
    cheat_assert(g_dispatch_part == SHOCK_REQUEST_BODY);
    cheat_assert(g_dispatch_data == &buf[67]);
    cheat_assert(g_dispatch_length == 6);
    cheat_assert(memcmp(g_dispatch_data, "banana", 6) == 0);
)

CHEAT_TEST(shock_http_send_version,
    char buf[] = "HTTP/1.1";
    shock_http_send(&g_srv, SHOCK_RESPONSE_VERSION, buf, sizeof(buf)-1);

    cheat_assert(g_output_count == 1);
    cheat_assert(g_output_srv == &g_srv);
    cheat_assert(g_output_data == buf);
    cheat_assert(g_output_length == sizeof(buf)-1);
    cheat_assert(memcmp(g_output_data, "HTTP/1.1", 8) == 0);
)

CHEAT_TEST(shock_http_send_status_code,
    char buf[] = "HTTP/1.1";

    // Send Version
    shock_http_send(&g_srv, SHOCK_RESPONSE_VERSION, buf, sizeof(buf)-1);
    cheat_assert(g_output_count == 1);

    // Send Status
    char buf2[] = "200";
    shock_http_send(&g_srv, SHOCK_RESPONSE_STATUS_CODE, buf2, sizeof(buf2)-1);

    cheat_assert(g_output_count == 3);
    cheat_assert(g_output_srv == &g_srv);
    cheat_assert(g_output_data == buf2); // Doesn't test for ' ' in between.
    cheat_assert(g_output_length == sizeof(buf2)-1);
    cheat_assert(memcmp(g_output_data, "200", 3) == 0);
)

CHEAT_TEST(shock_http_send_status_msg,
    char buf[] = "HTTP/1.1";

    // Send Version
    shock_http_send(&g_srv, SHOCK_RESPONSE_VERSION, buf, sizeof(buf)-1);
    cheat_assert(g_output_count == 1);

    // Send Status Code
    char buf2[] = "200";
    shock_http_send(&g_srv, SHOCK_RESPONSE_STATUS_CODE, buf2, sizeof(buf2)-1);

    // Send Status Message
    char buf3[] = "OK";
    shock_http_send(&g_srv, SHOCK_RESPONSE_STATUS_MSG, buf3, 2);

    cheat_assert(g_output_count == 5);
    cheat_assert(g_output_srv == &g_srv);
    cheat_assert(g_output_data == buf3); // Doesn't test for ' ' in between.
    cheat_assert(g_output_length == 2);
    cheat_assert(memcmp(g_output_data, "OK", 2) == 0);
)

CHEAT_TEST(shock_http_send_header_name,
    char buf[] = "HTTP/1.1";

    // Send Version
    shock_http_send(&g_srv, SHOCK_RESPONSE_VERSION, buf, sizeof(buf)-1);
    cheat_assert(g_output_count == 1);

    // Send Status Code
    char buf2[] = "200";
    shock_http_send(&g_srv, SHOCK_RESPONSE_STATUS_CODE, buf2, sizeof(buf2)-1);

    // Send Status Message
    char buf3[] = "OK";
    shock_http_send(&g_srv, SHOCK_RESPONSE_STATUS_MSG, buf3, 2);

    // Send Header Name
    char buf4[] = "Content-Length";
    shock_http_send(&g_srv, SHOCK_RESPONSE_HEADER_NAME, buf4, sizeof(buf4)-1);

    cheat_assert(g_output_count == 7);
    cheat_assert(g_output_srv == &g_srv);
    cheat_assert(g_output_data == buf4); // Doesn't test for ' ' in between.
    cheat_assert(g_output_length == sizeof(buf4)-1);
    cheat_assert(memcmp(g_output_data, "Content-Length", 14) == 0);
)

CHEAT_TEST(shock_http_send_header_value,
    char buf[] = "HTTP/1.1";

    // Send Version
    shock_http_send(&g_srv, SHOCK_RESPONSE_VERSION, buf, sizeof(buf)-1);
    cheat_assert(g_output_count == 1);

    // Send Status Code
    char buf2[] = "200";
    shock_http_send(&g_srv, SHOCK_RESPONSE_STATUS_CODE, buf2, sizeof(buf2)-1);

    // Send Status Message
    char buf3[] = "OK";
    shock_http_send(&g_srv, SHOCK_RESPONSE_STATUS_MSG, buf3, 2);

    // Send Header Name
    char buf4[] = "Content-Length";
    shock_http_send(&g_srv, SHOCK_RESPONSE_HEADER_NAME, buf4, sizeof(buf4)-1);

    // Send Header Value
    char buf5[] = "11";
    shock_http_send(&g_srv, SHOCK_RESPONSE_HEADER_VALUE, buf5, 2);

    cheat_assert(g_output_count == 9);
    cheat_assert(g_output_srv == &g_srv);
    cheat_assert(g_output_data == buf5); // Doesn't test for ' ' in between.
    cheat_assert(g_output_length == sizeof(buf5)-1);
    cheat_assert(memcmp(g_output_data, "11", 2) == 0);
)

CHEAT_TEST(shock_http_send_body,
    char buf[] = "HTTP/1.1";

    // Send Version
    shock_http_send(&g_srv, SHOCK_RESPONSE_VERSION, buf, sizeof(buf)-1);
    cheat_assert(g_output_count == 1);

    // Send Status Code
    char buf2[] = "200";
    shock_http_send(&g_srv, SHOCK_RESPONSE_STATUS_CODE, buf2, sizeof(buf2)-1);

    // Send Status Message
    char buf3[] = "OK";
    shock_http_send(&g_srv, SHOCK_RESPONSE_STATUS_MSG, buf3, 2);

    // Send Header Name
    char buf4[] = "hello world";
    shock_http_send(&g_srv, SHOCK_RESPONSE_BODY, buf4, sizeof(buf4)-1);

    // TODO - SW: Test the penultimate output as well
    cheat_assert(g_output_count == 9);
    cheat_assert(g_output_srv == &g_srv);
    cheat_assert(g_output_length == 2);
    cheat_assert(memcmp(g_output_data, "\r\n", 2) == 0);
)

