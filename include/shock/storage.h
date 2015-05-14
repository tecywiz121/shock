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
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__avr__) || defined(__AVR__)
#   define SHOCK_PSTR(s) ((const PROGMEM char*)(s))
#   define SHOCK_PROGMEM PROGMEM
#   define SHOCK_USE_PROGMEM 1
#else
#   define SHOCK_PSTR(s) (s)
#   define SHOCK_PROGMEM
#endif

SHOCK_API
int8_t shock_storage_i8(const int8_t* data);

SHOCK_API
int16_t shock_storage_i16(const int16_t* data);

SHOCK_API
int32_t shock_storage_i32(const int32_t* data);

SHOCK_API
uint8_t shock_storage_u8(const uint8_t* data);

SHOCK_API
uint16_t shock_storage_u16(const uint16_t* data);

SHOCK_API
uint32_t shock_storage_u32(const uint32_t* data);

SHOCK_API
const void* shock_storage_ptr(const void** data);

SHOCK_API
void* shock_storage_cpy(void* dest, const void* src, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SHOCK_STORAGE_H */
