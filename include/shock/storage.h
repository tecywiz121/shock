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

#ifndef SHOCK_STORAGE_H
#define SHOCK_STORAGE_H

#include <shock/common.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct shock_storage shock_storage;

struct shock_storage
{
    SHOCK_PVT(const char*);
    SHOCK_PVT(uint8_t);
};

#if defined(__avr__) || defined(__AVR__)
#   define SHOCK_PSTR(s) ((const PROGMEM char*)(s))
#   define SHOCK_USE_PROGMEM 1
#else
#   define SHOCK_PSTR(s) (s)
#endif

SHOCK_API
void shock_storage_init(shock_storage* self, const char* data);

SHOCK_API
uint8_t shock_storage_get_one(shock_storage* self);

SHOCK_API
uint16_t shock_storage_get_n(shock_storage* self, unsigned char n);

#ifdef __cplusplus
}
#endif

#endif /* SHOCK_STORAGE_H */
