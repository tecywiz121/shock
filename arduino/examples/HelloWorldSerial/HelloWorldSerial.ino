/* vim: et sw=4 sts=4 ts=4 : */

/*****************************************************************************
 * Shock HTTP-over-Serial Sample
 * -----------------------------
 *
 * This sample demonstrates how to serve some data over HTTP using an Arduino's
 * default serial connection. When an HTTP request is received, the words
 * "hello world" will be returned to the client/browser.
 *
 * Requirements:
 *
 * This sketch requires the Shock HTTP library, available from:
 *   https://github.com/tecywiz121/shock
 *
 * Usage:
 *
 * Run the sketch, and try making an HTTP request. If you are using the
 * Arduino IDE:
 *   1. Upload the sketch.
 *   2. Open the Serial Monitor (Tools->Serial Monitor).
 *   3. Change the "No Line Ending" to "Both NL & CR".
 *   4. Type the following, pressing Send on each <Send>:
 *        GET / HTTP/1.1        <Send>
 *        Host: www.example.com <Send>
 *        <Send>
 *   5. Voila! If everything worked, you should see an HTTP response.
 *
 * It is also possible to see the sample working with a browser:
 *   1. After uploading the sketch, open a browser.
 *   2. In a terminal, input the following command, substituting the correct
 *      serial port:
 *        nc -l 8080 < /dev/ttyACM0 > /dev/ttyACM0
 *   3. Navigate to http://localhost:8080/
 *   4. Voila! If everything worked, you should see "hello world".
 *
 * License:
 *
 * Copyright (c) 2015 Sam Wilson
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
 *****************************************************************************/

#include "http_pvt.h"
#include <ShockHTTP.h>


void dispatch(ShockHTTP& server,
              void* closure,
              shock_request_part part,
              const char* data,
              size_t length) {
  if (SHOCK_REQUEST_COMPLETE == part) {
    server.send(SHOCK_RESPONSE_VERSION, "HTTP/1.1", 8);
    server.send(SHOCK_RESPONSE_STATUS_CODE, "200", 3);
    server.send(SHOCK_RESPONSE_STATUS_MSG, "OK", 2);
    server.send(SHOCK_RESPONSE_BODY, "hello world", 11);
    server.send(SHOCK_RESPONSE_BODY, "", 0);
  }
}

void output(ShockHTTP& server,
            void* closure,
            const char* data,
            size_t length) {
  Serial.write((const uint8_t*)data, length);
}

ShockHTTP server(dispatch, output, NULL);

void setup() {
  Serial.begin(9600);
}

const int bufferLength = 20;
char buffer[bufferLength];
int length = 0;

void loop()
{
  int c = Serial.read();
  if (0 > c) {
    return;
  }

  if (length >= bufferLength) {
    length = 0;
    server.drop();
    return;
  }

  buffer[length++] = (byte)(c & 0xFF);

  int bytesRead = server.receive(buffer, length);

  if (!bytesRead) {
    return;
  }

  memmove(buffer, &buffer[bytesRead], length - bytesRead);
  length -= bytesRead;
}
