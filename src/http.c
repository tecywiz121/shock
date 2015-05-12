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

#include "http_pvt.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
// Private Method Prototypes
///////////////////////////////////////////////////////////////////////////////

static void output(shock_http_pvt* me, const char* data, size_t length);

static void to_version(shock_http_pvt* me);
static void to_body(shock_http_pvt* self);
static void to_header_value(shock_http_pvt* self);
static void to_header_name(shock_http_pvt* self);
static void to_status_msg(shock_http_pvt* self);
static void to_status_code(shock_http_pvt* self);

static shock_send_state send_part_to_state(shock_response_part part);

static size_t receive_body_length(shock_http_pvt* me,
                                  const char* data,
                                  size_t length);

static size_t receive_body(shock_http_pvt* me,
                           const char* data,
                           size_t length);

static size_t receive_after_header(shock_http_pvt* me,
                                   const char* data,
                                   size_t length);

static size_t receive_header_value(shock_http_pvt* me,
                                   const char* data,
                                   size_t length);

static size_t receive_header_name(shock_http_pvt* me,
                                  const char* data,
                                  size_t length);

static size_t receive_version(shock_http_pvt* me,
                              const char* data,
                              size_t length);

static size_t receive_method(shock_http_pvt* me,
                             const char* data,
                             size_t length);

static size_t receive_path(shock_http_pvt* me,
                           const char* data,
                           size_t length);

static size_t receive_lws(shock_http_pvt* me,
                          const char* data,
                          size_t length,
                          shock_receive_state next);

static void parse_transfer_encoding(shock_http_pvt* me,
                                    const char* data,
                                    size_t length);

static void parse_content_length(shock_http_pvt* me,
                                 const char* data,
                                 size_t length);

static shock_bool find_or_discard_eol(shock_http_pvt* me,
                                      const char* data,
                                      size_t length,
                                      size_t* data_len,
                                      size_t* consumed,
                                      shock_receive_state next);

static shock_bool find_or_discard_ws(shock_http_pvt* me,
                                     const char* data,
                                     size_t* length,
                                     const char** whitespace,
                                     shock_receive_state next);

static const char* find_ws(const char* data, size_t length);

///////////////////////////////////////////////////////////////////////////////
// Implementations
///////////////////////////////////////////////////////////////////////////////
void shock_http_init(shock_http* self,
                     shock_dispatch_fn* dispatch,
                     shock_output_fn* output,
                     void* closure)
{
    shock_http_pvt* me = (shock_http_pvt*)self;
    me->dispatch = dispatch;
    me->output = output;
    me->closure = closure;
    me->discarding = 0;
    me->header = HEADER_UNKNOWN;
    me->content_length = 0;
    me->chunked = 0;
    me->rstate = RECEIVE_METHOD;
    me->sstate = SEND_VERSION;
}

void shock_http_fini(shock_http* self)
{
    shock_http_pvt* me = (shock_http_pvt*)self;
    me->dispatch = NULL;
    me->output = NULL;
    me->closure = NULL;
}

void shock_http_drop(shock_http* self)
{
    ((shock_http_pvt*)self)->discarding = 1;
}

size_t shock_http_recv(shock_http* self, const char* data, size_t len)
{
    shock_http_pvt* me = (shock_http_pvt*)self; // Access our privates.

    size_t total = 0;                           // Used to get total consumed.
    size_t consumed = 0 ;                       // Bytes consumed in one state.
    shock_receive_state state;
    do {
        state = me->rstate;                     // Save the rstate to tell if
                                                // it changed.

        len -= consumed;                        // Update the length from the
                                                // last iteration.

        switch (me->rstate) {                   // What are we doing?
            case RECEIVE_METHOD:                // Ah. Getting the method.
                consumed = receive_method(me,   // See if we have the whole
                                          data, // method in the buffer, and if
                                          len); // so, tell the application.
                break;
            case RECEIVE_AFTER_METHOD:          // We're skipping whitespace.
                consumed = receive_lws(me,      // See if we have the first
                                       data,    // non-space character, and if
                                       len,     // so, go to the next state.
                                       RECEIVE_PATH);
                break;

            case RECEIVE_PATH:
                consumed = receive_path(me,     // Cool, on to the path. See if
                                        data,   // we have the whole path, and
                                        len);   // if so, tell the application.
                break;

            case RECEIVE_AFTER_PATH:
                consumed = receive_lws(me,      // Oh boy! Skipping more space!
                                       data,
                                       len,
                                       RECEIVE_VERSION);
                break;

            case RECEIVE_VERSION:
                consumed = receive_version(me,  // At the HTTP version!
                                           data,
                                           len);
                break;

            case RECEIVE_HEADER_NAME:
                consumed = receive_header_name(me, data, len);
                break;

            case RECEIVE_AFTER_HEADER_NAME:
                consumed = receive_lws(me,
                                       data,
                                       len,
                                       RECEIVE_HEADER_VALUE);
                break;

            case RECEIVE_HEADER_VALUE:
                consumed = receive_header_value(me, data, len);
                break;

            case RECEIVE_AFTER_HEADER_VALUE:
                consumed =
                    receive_after_header(me,    // Determine if there is
                                         data,  // another header, or if we're
                                         len);  // going into the body.
                break;

            case RECEIVE_BODY:
                consumed = receive_body(me,     // Jump to the correct body
                                        data,   // mode based on the chunked
                                        len);   // flag.
                break;

            case RECEIVE_BODY_LENGTH:
                consumed =
                    receive_body_length(me,
                                        data,
                                        len);
                break;

            case RECEIVE_BODY_CHUNKED:
                consumed=0;
                break;

        }

        data += consumed;                       // Advance the pointer.
        total += consumed;                      // More bytes for the byte god!
    } while (len > consumed                     // Stop if: buffer is empty, or
             && (0 < consumed                   //  nothing was read, and
                 || me->rstate != state));      //  the state wasn't changed.
                                                //
                                                // A receive_* function will
                                                // return zero when it doesn't
                                                // have enough data to process.

    return total;
}

static size_t receive_method(shock_http_pvt* me, const char* data, size_t length)
{
    // Scan for the space after the method:
    //      GET /path HTTP/1.1
    //      ---^
    const char* whitespace;
    if (find_or_discard_ws(me, data, &length, &whitespace, RECEIVE_AFTER_METHOD)) {
        return length;
    }

    if (!whitespace) {              // Did we find the whitespace?
        return 0;                   // Nope, so don't read any characters.
    }

    length = whitespace - data;     // How many bytes did we consume?

    me->dispatch((shock_http*)me,   // Invoke the callback to say we found the
                 me->closure,       // HTTP method of the request.
                 SHOCK_REQUEST_METHOD,
                 data,
                 length);

    me->rstate = RECEIVE_AFTER_METHOD;  // Advance to the next state

    return length;                  // Report that we've consumed some bytes.
}

static size_t receive_lws(shock_http_pvt* me,
                          const char* data,
                          size_t length,
                          shock_receive_state next)
{
    if (0 == length) {                  // Check for empty data.
        return 0;                       // It's empty? Odd, so return.
    }

    size_t ii;
    for (ii = 0; ii < length; ii++) {   // Try and find the first non-space.
        char c = data[ii];
        if (c != ' ' && c != '\t') {    // Did we find one?
            me->rstate = next;          // Yes! Advance to the next state, and
            return ii;                  // return the number of consumed bytes.
        }
    }

    return ii;                          // Didn't find a non-whitespace byte,
}

static size_t receive_path(shock_http_pvt* me, const char* data, size_t length)
{
    // Scan for the space after the path:
    //      GET /path HTTP/1.1
    //      ---------^
    const char* whitespace;
    if (find_or_discard_ws(me, data, &length, &whitespace, RECEIVE_AFTER_PATH)) {
        return length;
    }

    if (!whitespace) {              // Did we find the whitespace?
        return 0;                   // Nope, so don't read any characters.
    }

    length = whitespace - data;     // How many bytes did we consume?

    me->dispatch((shock_http*)me,   // Invoke the callback to say we found the
                 me->closure,       // path the request is looking for.
                 SHOCK_REQUEST_PATH,
                 data,
                 length);

    me->rstate = RECEIVE_AFTER_PATH;    // Advance to the next state.

    return length;                  // Return the number of consumed bytes.
}

static size_t receive_version(shock_http_pvt* me, const char* data, size_t length)
{
    // Scan for the new line after the version:
    //      GET /path HTTP/1.1<CR><LF>
    //      ------------------^^^^^^^^
    size_t data_len;                // Length of data before the end of line.
    size_t consumed;                // Length including data & EOL characters.
    shock_bool no_data = find_or_discard_eol(me,
                                             data,
                                             length,
                                             &data_len,
                                             &consumed,
                                             RECEIVE_HEADER_NAME);

    if (no_data) {                  // Did we find any data?
        return consumed;            // Nope, returned the consumed count.
    }

    me->dispatch((shock_http*)me,   // Invoke the callback to say we found the
                 me->closure,       // HTTP version of the request.
                 SHOCK_REQUEST_VERSION,
                 data,
                 data_len);

    me->rstate = RECEIVE_HEADER_NAME;

    return consumed;                // Return the number of consumed bytes.
}

static size_t receive_header_name(shock_http_pvt* me,
                                  const char* data,
                                  size_t length)
{
    // Scan for the colon after the header name:
    //      Host: www.example.com<CR><LF>
    //      ----^
    char* colon = (char*) memchr(data, ':', length);

    if (!colon) {                           // Did we find a colon?
        if (me->discarding) {               // No, so are we discarding?
            return length;                  // Yep, consume everything.
        } else {
            return 0;                       // Nope, so don't consume anything.
        }
    }

    size_t consumed = colon - data + 1;     // Read bytes including the colon.

    me->rstate = RECEIVE_AFTER_HEADER_NAME;

    if (me->discarding) {                   // Are we discarding?
        me->discarding = 0;
        return consumed;                    // Yes, consume everything.
    }

    length = consumed - 1;                  // Exclude the colon from the data.

    me->dispatch((shock_http*)me,           // Inform the application we have a
                 me->closure,               // header name to give them.
                 SHOCK_REQUEST_HEADER_NAME,
                 data,
                 length);

    // Important Headers
    me->header = HEADER_UNKNOWN;            // Clear the expected header.

    const char* te = "Transfer-Encoding";   // We care about this header.
    const size_t n_te = strlen(te);

    const char* cl = "Content-Length";      // This one too!
    const size_t n_cl = strlen(cl);

    if (n_cl == length) {                   // Does the size match cl's?
        size_t eq = memcmp(cl,              // Yes! Compare what we have with
                           data,            // "Content-Length".
                           length);
        if (0 == eq) {                      // Is this header Content-Length?
            me->header = HEADER_CONTENT_LENGTH;
        }
    } else if (n_te == length) {            // Does the size match te's?
        size_t eq = memcmp(te,              // Yes! Compare what we have with
                           data,            // "Transfer-Encoding".
                           length);

        if (0 == eq) {                      // Is this Transfer-Encoding?
            me->header = HEADER_TRANSFER_ENCODING;
        }
    }

    return consumed;
}

static size_t receive_header_value(shock_http_pvt* me,
                                   const char* data,
                                   size_t length)
{
    // Scan for the end of line after the header value:
    //      Host: www.example.com<CR><LF>
    //      ---------------------^^^^^^^^
    size_t data_len;                // Length of data before the end of line.
    size_t consumed;                // Length including data & EOL characters.
    shock_bool no_data = find_or_discard_eol(me,
                                             data,
                                             length,
                                             &data_len,
                                             &consumed,
                                             RECEIVE_AFTER_HEADER_VALUE);

    if (no_data) {                  // Did we find any data?
        return consumed;            // Nope, return number of consumed bytes.
    }

    me->dispatch((shock_http*)me,
                 me->closure,
                 SHOCK_REQUEST_HEADER_VALUE,
                 data,
                 data_len);

    me->rstate = RECEIVE_AFTER_HEADER_VALUE;

    // Important Headers
    switch (me->header) {
        case HEADER_CONTENT_LENGTH:
            parse_content_length(me, data, data_len);
            break;

        case HEADER_TRANSFER_ENCODING:
            parse_transfer_encoding(me, data, data_len);
            break;

        case HEADER_UNKNOWN:
            // No-op
            break;
    }

    return consumed;
}

static void advance_to_body(shock_http_pvt* me)
{
    if (me->chunked || me->content_length) {    // Are we expecting a body?
        me->rstate = RECEIVE_BODY;              // Yes, so advance to it.
    } else {
        char buf[1];                            // No body, so dispatch a
        me->dispatch((shock_http*)me,           // request complete message.
                     me->closure,
                     SHOCK_REQUEST_COMPLETE,
                     buf,
                     0);
        me->rstate = RECEIVE_METHOD;            // Ready for next request.
    }
}

static size_t receive_after_header(shock_http_pvt* me,
                                   const char* data,
                                   size_t length)
{
    if (0 == length) {                  // Check if we have any data at all.
        return 0;                       // No? Well we can't consume anything.
    }


    // Scan for the blank line after a header value:
    //      Host: www.example.com<LF>
    //      <LF>
    //      ^^^^
    if ('\n' == data[0]) {              // Is the first character a line feed?
        advance_to_body(me);            // Yes, so advance to the body.
        return 1;                       // Consume the <LF> character.
    }

    // Scan for the blank line after a header value:
    //      Host: www.example.com<CR><LF>
    //      <CR><LF>
    //      ^^^^^^^^
    if ('\r' == data[0]) {              // Is the 1st byte a carriage return?
        if (1 == length) {              // Yes, so is there room for a <LF>?
            return 0;                   // No, so we need more data.
        } else if (data[1] == '\n') {   // Are we seeing a '<CR><LF>'?
            advance_to_body(me);        // Yes, so advance to the body.
            return 2;                   // Consume the <CR><LF> pair.
        }
    }

    // Neither a <CR><LF> or a <LF> was found, and we had enough data to tell.
    me->rstate = RECEIVE_HEADER_NAME;
    return 0;
}

static size_t receive_body(shock_http_pvt* me,
                           const char* SHOCK_UNUSED(data),
                           size_t SHOCK_UNUSED(length))
{
    if (me->chunked) {
        me->rstate = RECEIVE_BODY_CHUNKED;
    } else {
        me->rstate = RECEIVE_BODY_LENGTH;
    }
    return 0;
}

static void parse_transfer_encoding(shock_http_pvt* me,
                                    const char* data,
                                    size_t length)
{
    const char* const chunked = "chunked";
    const size_t n_chunked = strlen(chunked) - 1;

    if (n_chunked != length) {                              // Length matches?
        return;                                             // Nope! So bail.
    }

    me->chunked = (0 == memcmp(chunked, data, n_chunked));  // data=='chunked'?
}

static void parse_content_length(shock_http_pvt* me,
                                 const char* data,
                                 size_t length)
{
    char text[length+1];                            // Space to put null byte.
    memcpy(text, data, length);                     // Copy data into text.
    text[length] = '\0';                            // Add null terminator.

    errno = 0;                                      // Reset to detect errors.
    unsigned long value = strtoul(text, NULL, 10);  // Convert text to integer.

    if (0 != errno) {                               // Did we have an error?
        return;                                     // Yep, bail out.
    }

    me->content_length = value;                     // Set content length.
}

static size_t receive_body_length(shock_http_pvt* me,
                                  const char* data,
                                  size_t length)
{
    if (length >= me->content_length) {
        length = me->content_length;
    }

    if (length) {
        me->dispatch((shock_http*)me,
                     me->closure,
                     SHOCK_REQUEST_BODY,
                     data,
                     length);
    }

    me->content_length -= length;

    if (0 == me->content_length) {
        me->rstate = RECEIVE_METHOD;
    }

    return length;
}

static const char* find_ws(const char* data, size_t length)
{
    // TODO: Don't scan data twice
    char* space = (char*) memchr(data, ' ', length);
    char* tab = (char*) memchr(data, '\t', length);

    if (!space) {
        return tab;
    } else if (!tab) {
        return space;
    } else {
        return tab < space ? tab : space;
    }
}

static shock_bool find_or_discard_ws(shock_http_pvt* me,
                                     const char* data,
                                     size_t* length,
                                     const char** whitespace,
                                     shock_receive_state next)
{
    *whitespace = find_ws(data, *length);

    if (!me->discarding) {
        return 0;
    }

    if (*whitespace) {
        *length = *whitespace - data;
        me->rstate = next;
        me->discarding = 0;
    }

    return 1;
}


static shock_bool find_or_discard_eol(shock_http_pvt* me,
                                      const char* data,
                                      size_t length,
                                      size_t* data_len,
                                      size_t* consumed,
                                      shock_receive_state next)
{
    char* lf = (char*) memchr(data, '\n', length);  // Find a line feed.

    *data_len = 0;                                  // Clear data length.

    if (!lf) {                                      // Did we find a line feed?
        *consumed = me->discarding ? length : 0;    // No. Consume data if
                                                    // discarding, else wait.
        return 1;                                   // No further processing.
    }

    *consumed = lf - data + 1;                      // Get <LF>, consume it.

    if (me->discarding) {                           // Are we discarding?
        me->rstate = next;                          // Yes, advance the state.
        me->discarding = 0;                         // Clear discard flag.
        return 1;                                   // No further processing.
    }

    *data_len = *consumed - 1;                      // Exclude LF from data.

    // Not discarding, so check for carriage return
    if (lf > data) {                                // Is there room for a CR?
        if (*(lf-1) == '\r') {                      // Yep, so is there one?
            *data_len -= 1;                         // Yes, so exclude it.
        }
    }

    return 0;                                       // Got some data!
}

void shock_http_send(shock_http* self,
                     shock_response_part part,
                     const char* data,
                     size_t length)
{
    shock_http_pvt* me = (shock_http_pvt*)self;
    shock_send_state tgt = send_part_to_state(part);

    // Advance the state from where we are to where the application wants to be
    while (me->sstate != tgt) {
        switch (me->sstate) {                   // What state are we in?
            case SEND_VERSION:                  // Sending the version:
                to_status_code(me);             // advance to status code.
                break;
            case SEND_STATUS_CODE:              // Sending the status code:
                to_status_msg(me);              // advance to the status msg.
                break;
            case SEND_STATUS_MSG:               // Sending the status msg, or
            case SEND_HEADER_VALUE:             // sending a header value:
                if (tgt != SEND_HEADER_NAME) {  // Are we going to the body?
                    to_body(me);                // Yes, advance there.
                } else {                        // No?
                    to_header_name(me);         // Advance to header name.
                }
                break;
            case SEND_HEADER_NAME:
                to_header_value(me);
                break;
            case SEND_BODY:
                to_version(me);
                break;
        }
    }

    // Write the Transfer-Encoding: chunked header
    if (SEND_BODY == me->sstate) {              // Are we sending the body?
#define n_digits (sizeof(length)*2 + 3)         // Space for enough hex digits,
        char digits[n_digits];                  // including \r\n\0 at end.

        size_t written = snprintf(digits,
                                  n_digits,
                                  "%" SHOCK_PR_SIZET "\r\n",
                                  length);

        output(me, digits, written);
    }

    output(me, data, length);  // Write out the data.

    if (SEND_BODY == me->sstate) {
        output(me, "\r\n", 2);
    }
}

static void to_status_code(shock_http_pvt* me)
{
    char sp = ' ';
    output(me, &sp, 1);
    me->sstate = SEND_STATUS_CODE;
}

static void to_status_msg(shock_http_pvt* me)
{
    char sp = ' ';
    output(me, &sp, 1);
    me->sstate = SEND_STATUS_MSG;
}

static void to_version(shock_http_pvt* me)
{
    me->sstate = SEND_VERSION;
}

static void to_body(shock_http_pvt* me)
{
    // Write out the transfer encoding header
    char te[] = "\r\nTransfer-Encoding: chunked\r\n\r\n";
    output(me, te, sizeof(te)/sizeof(char)-1);

    // Advance state to body
    me->sstate = SEND_BODY;
}

static void to_header_value(shock_http_pvt* me)
{
    char buf[] = ": ";
    output(me, buf, 2);
    me->sstate = SEND_HEADER_VALUE;
}

static void to_header_name(shock_http_pvt* me)
{
    char buf[] = "\r\n";
    output(me, buf, 2);
    me->sstate = SEND_HEADER_NAME;
}

static shock_send_state send_part_to_state(shock_response_part part)
{
    switch (part) {
        case SHOCK_RESPONSE_VERSION:
            return SEND_VERSION;
        case SHOCK_RESPONSE_STATUS_CODE:
            return SEND_STATUS_CODE;
        case SHOCK_RESPONSE_STATUS_MSG:
            return SEND_STATUS_MSG;
        case SHOCK_RESPONSE_HEADER_NAME:
            return SEND_HEADER_NAME;
        case SHOCK_RESPONSE_HEADER_VALUE:
            return SEND_HEADER_VALUE;
        case SHOCK_RESPONSE_BODY:
            return SEND_BODY;
        default:
            abort();
    }
}

static void output(shock_http_pvt* me, const char* data, size_t length)
{
    me->output((shock_http*)me, me->closure, data, length);
}
