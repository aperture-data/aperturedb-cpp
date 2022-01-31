/**
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 * @copyright Copyright (c) 2021 ApertureData Inc
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

// old style compiler guards because multiple copies of this file may be accessible
#ifndef __MACROS_H__
#define __MACROS_H__

#if !defined(NOT_COPYABLE)
    #define NOT_COPYABLE(Type)                  \
        Type(const Type&) = delete;             \
        Type& operator=(const Type&) = delete;
#endif // !defined(NOT_COPYABLE)

#if !defined(COPYABLE_BY_DEFAULT)
    #define COPYABLE_BY_DEFAULT(Type)           \
        Type(const Type&) = default;            \
        Type& operator=(const Type&) = default;
#endif // !defined(COPYABLE_BY_DEFAULT)

#if !defined(NOT_MOVEABLE)
    #define NOT_MOVEABLE(Type)                  \
        Type(Type&&) = delete;                  \
        Type& operator=(Type&&) = delete;
#endif // !defined(NOT_MOVEABLE)

#if !defined(MOVEABLE_BY_DEFAULT)
    #define MOVEABLE_BY_DEFAULT(Type)           \
        Type(Type&&) = default;                 \
        Type& operator=(Type&&) = default;
#endif // !defined(MOVEABLE_BY_DEFAULT)

#endif // __MACROS_H__
