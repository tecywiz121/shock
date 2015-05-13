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

/*
 * Portions of this file are derived from software under the following license:
 *
 * Copyright (c)  2011
 *     James Bowman  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 3. Neither the name of James Bowman nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * http://excamera.com/files/gameduino/synth/
 */

#include "storage_pvt.h"

///////////////////////////////////////////////////////////////////////////////
// Private Method Prototypes
///////////////////////////////////////////////////////////////////////////////

#ifdef SHOCK_USE_PROGMEM
#   define shock_read_byte_near(x) pgm_read_byte_near(x)
#else
#   define shock_read_byte_near(x) (*(uint8_t*)(x))
#endif

///////////////////////////////////////////////////////////////////////////////
// Implementations
///////////////////////////////////////////////////////////////////////////////
void shock_storage_init(shock_storage* self, const char* data)
{
    shock_storage_pvt* me = (shock_storage_pvt*) self;
    me->data = data;
    me->mask = 0x01;
}

uint8_t shock_storage_get_one(shock_storage* self)
{
    shock_storage_pvt* me = (shock_storage_pvt*) self;
    uint8_t r = (shock_read_byte_near(me->data) & me->mask) != 0;
    me->mask <<= 1;
    if (!me->mask) {
        me->mask = 1;
        me->data += 1;
    }
    return r;
}

uint16_t shock_storage_get_n(shock_storage* self, unsigned char n)
{
    // TODO - SW: Reading in entire bytes could be optimized
    uint16_t r = 0;
    while (n--) {
        r <<= 1;
        r |= shock_storage_get_one(self);
    }
    return r;
}
