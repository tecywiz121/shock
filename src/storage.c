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

#include <shock/storage.h>
#include <string.h>

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
int8_t shock_storage_i8(const int8_t* data)
{
    return *data;
}

int16_t shock_storage_i16(const int16_t* data)
{
    return *data;
}

int32_t shock_storage_i32(const int32_t* data)
{
    return *data;
}

uint8_t shock_storage_u8(const uint8_t* data)
{
    return *data;
}

uint16_t shock_storage_u16(const uint16_t* data)
{
    return *data;
}

uint32_t shock_storage_u32(const uint32_t* data)
{
    return *data;
}

const void* shock_storage_ptr(const void** data)
{
    return *data;
}

void* shock_storage_cpy(void* dst, const void* data, size_t len)
{
    return memcpy(dst, data, len);
}
