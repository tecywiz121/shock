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

#ifndef SHOCK_STORAGE_PVT_H
#define SHOCK_STORAGE_PVT_H

#include <shock/storage.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct shock_storage_pvt shock_storage_pvt;

struct shock_storage_pvt
{
    const char* data;
    uint8_t     mask;
};

shock_static_assert(sizeof(shock_storage_pvt) == sizeof(shock_storage),
               "shock_storage_pvt and shock_storage don't match")

#ifdef __cplusplus
}
#endif

#endif /* SHOCK_STORAGE_PVT_H */
