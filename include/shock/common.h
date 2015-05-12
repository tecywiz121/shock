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

#ifndef SHOCK_COMMON_H
#define SHOCK_COMMON_H

/* ************************************************************************* *
 * Public API Visibility                                                     *
 * ************************************************************************* */
#if defined(_WIN32) || defined(__CYGWIN__)
#   define SHOCK_SHARED_IMPORT __declspec(dllimport)
#   define SHOCK_SHARED_EXPORT __declspec(dllexport)
#else /* WIN32 */
#   if __GNUC__ >= 4
#       define SHOCK_SHARED_IMPORT __attribute__ ((visibility ("default")))
#       define SHOCK_SHARED_EXPORT __attribute__ ((visibility ("default")))
#   else
#       define SHOCK_SHARED_IMPORT
#       define SHOCK_SHARED_EXPORT
#   endif /* __GNUC__ */
#endif

#ifdef SHOCK_SHARED                             // Defined if using shared lib.
#   ifdef SHOCK_SHARED_EXPORTS                  // Defined when building clowder.
#       define SHOCK_API SHOCK_SHARED_EXPORT
#   else
#       define SHOCK_API SHOCK_SHARED_IMPORT
#   endif /* SHOCK_SHARED_EXPORTS */
#else                                           // Must be a static library.
#   define SHOCK_API
#endif /* SHOCK_SHARED */

#define SHOCK_UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#define SHOCK_TOKENPASTE(x, y) x ## y
#define SHOCK_TOKENPASTE2(x, y) SHOCK_TOKENPASTE(x, y)
#define SHOCK_PVT(T) T SHOCK_TOKENPASTE2(_pvt, __COUNTER__) __attribute__((deprecated))
typedef char shock_bool;

#if __cplusplus >= 201103L
#   define shock_static_assert(cond, msg) static_assert(cond, msg);
#elif __STDC_VERSION__ >= 201112L
#   define shock_static_assert(cond, msg) _Static_assert(cond, msg);
#else
#   define shock_static_assert(cond, msg)
#endif

#if __STDC_VERSION__ >= 201112L || __cplusplus >= 201103L
#   define SHOCK_PR_SIZET "zX"
#elif defined(ARDUINO) && (defined(__avr__) || defined(__AVR__))
#   define SHOCK_PR_SIZET "hX"
#else
#   error Unsure of how to format size_t for printf
#endif

#endif /* SHOCK_COMMON_H */
