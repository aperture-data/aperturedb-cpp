/**
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#pragma once

#define ENUM(T1, T2) \
enum class T1 : T2; \
inline T1  operator~  (T1  a      ) { return static_cast<T1>(~static_cast<T2>(a)); } \
inline T1  operator|  (T1  a, T1 b) { return static_cast<T1>((static_cast<T2>(a) | static_cast<T2>(b))); } \
inline T1  operator&  (T1  a, T1 b) { return static_cast<T1>((static_cast<T2>(a) & static_cast<T2>(b))); } \
inline T1  operator^  (T1  a, T1 b) { return static_cast<T1>((static_cast<T2>(a) ^ static_cast<T2>(b))); } \
inline T1& operator|= (T1& a, T1 b) { return reinterpret_cast<T1&>((reinterpret_cast<T2&>(a) |= static_cast<T2>(b))); } \
inline T1& operator&= (T1& a, T1 b) { return reinterpret_cast<T1&>((reinterpret_cast<T2&>(a) &= static_cast<T2>(b))); } \
inline T1& operator^= (T1& a, T1 b) { return reinterpret_cast<T1&>((reinterpret_cast<T2&>(a) ^= static_cast<T2>(b))); } \
enum class T1 : T2
