/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
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
