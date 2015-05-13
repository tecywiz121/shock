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
    shock_storage g_sto;
)

CHEAT_SET_UP(
    shock_storage_init(&g_sto, SHOCK_PSTR("hello world"));
)

CHEAT_TEST(shock_storage_get_one,
    uint8_t expected[] = {0, 0, 0, 1, 0, 1, 1, 0,
                          1, 0, 1, 0, 0, 1, 1, 0,
                          0, 0, 1, 1, 0, 1, 1, 0,
                          0, 0, 1, 1, 0, 1, 1, 0,
                          1, 1, 1, 1, 0, 1, 1, 0,
                          0, 0, 0, 0, 0, 1, 0, 0,
                          1, 1, 1, 0, 1, 1, 1, 0,
                          1, 1, 1, 1, 0, 1, 1, 0,
                          0, 1, 0, 0, 1, 1, 1, 0,
                          0, 0, 1, 1, 0, 1, 1, 0,
                          0, 0, 1, 0, 0, 1, 1, 0,
                          0, 0, 0, 0, 0, 0, 0, 0};

    for (unsigned int ii = 0; ii < sizeof(expected)/sizeof(uint8_t); ii++) {
        uint8_t v = shock_storage_get_one(&g_sto);
        cheat_assert(v == expected[ii]);
    }
)

CHEAT_TEST(shock_storage_get_n_1,
    uint16_t expected[] = {0, 0, 0, 1, 0, 1, 1, 0,
                           1, 0, 1, 0, 0, 1, 1, 0,
                           0, 0, 1, 1, 0, 1, 1, 0,
                           0, 0, 1, 1, 0, 1, 1, 0,
                           1, 1, 1, 1, 0, 1, 1, 0,
                           0, 0, 0, 0, 0, 1, 0, 0,
                           1, 1, 1, 0, 1, 1, 1, 0,
                           1, 1, 1, 1, 0, 1, 1, 0,
                           0, 1, 0, 0, 1, 1, 1, 0,
                           0, 0, 1, 1, 0, 1, 1, 0,
                           0, 0, 1, 0, 0, 1, 1, 0,
                           0, 0, 0, 0, 0, 0, 0, 0};

    for (unsigned int ii = 0; ii < sizeof(expected)/sizeof(expected[0]); ii++) {
        uint16_t v = shock_storage_get_n(&g_sto, 1);
        cheat_assert(v == expected[ii]);
    }
)

CHEAT_TEST(shock_storage_get_n_8,
    uint16_t expected[] = {0x16, 0xa6, 0x36, 0x36, 0xf6, 0x04,
                           0xee, 0xf6, 0x4e, 0x36, 0x26};

    for (unsigned int ii = 0; ii < sizeof(expected)/sizeof(expected[0]); ii++) {
        uint16_t v = shock_storage_get_n(&g_sto, 8);
        cheat_assert(v == expected[ii]);
    }
)
