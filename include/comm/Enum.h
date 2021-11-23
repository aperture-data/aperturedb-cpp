/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#define ENUM_FLAGS(T1, T2) \
enum class T1 : T2; \
inline T1  operator~  (T1  a      ) { return static_cast<T1>(~static_cast<T2>(a)); } \
inline T1  operator|  (T1  a, T1 b) { return static_cast<T1>((static_cast<T2>(a) | static_cast<T2>(b))); } \
inline T1  operator&  (T1  a, T1 b) { return static_cast<T1>((static_cast<T2>(a) & static_cast<T2>(b))); } \
inline T1  operator^  (T1  a, T1 b) { return static_cast<T1>((static_cast<T2>(a) ^ static_cast<T2>(b))); } \
inline T1& operator|= (T1& a, T1 b) { return reinterpret_cast<T1&>((reinterpret_cast<T2&>(a) |= static_cast<T2>(b))); } \
inline T1& operator&= (T1& a, T1 b) { return reinterpret_cast<T1&>((reinterpret_cast<T2&>(a) &= static_cast<T2>(b))); } \
inline T1& operator^= (T1& a, T1 b) { return reinterpret_cast<T1&>((reinterpret_cast<T2&>(a) ^= static_cast<T2>(b))); } \
enum class T1 : T2
