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

#include <shock/storage.h>

// Stops my YouCompleteMe from freaking out
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

CHEAT_DECLARE(
    const int8_t g_i8 SHOCK_PROGMEM = 5;
    const uint8_t g_u8 SHOCK_PROGMEM = 7;
    const int16_t g_i16 SHOCK_PROGMEM = 0x0CDE;
    const uint16_t g_u16 SHOCK_PROGMEM = 0xABAB;
    const int32_t g_i32 SHOCK_PROGMEM = 0x0ABCACDE;
    const uint32_t g_u32 SHOCK_PROGMEM = 0xFEFEFEFE;
    const void* g_ptr SHOCK_PROGMEM = (void*)0xDEADBEEF;
    const uint8_t g_data[] SHOCK_PROGMEM = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
)

CHEAT_TEST(shock_storage_gets,
    cheat_assert(5 == shock_storage_i8(&g_i8));
    cheat_assert(7 == shock_storage_u8(&g_u8));
    cheat_assert(0x0CDE == shock_storage_i16(&g_i16));
    cheat_assert(0xABAB == shock_storage_u16(&g_u16));
    cheat_assert(0x0ABCACDE == shock_storage_i32(&g_i32));
    cheat_assert(0xFEFEFEFE == shock_storage_u32(&g_u32));
    cheat_assert((void*)0xDEADBEEF == shock_storage_ptr(&g_ptr));
)

CHEAT_TEST(stock_storage_cpy,
    uint8_t result[11];
    cheat_assert(result == shock_storage_cpy(result, g_data, 11));
    for (uint8_t ii = 1; ii < 12; ii++) {
        cheat_assert(result[ii-1] == ii);
    }
)
