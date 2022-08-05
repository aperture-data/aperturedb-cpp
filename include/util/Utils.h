/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#pragma once

#include <util/TypeName.h>
#include <iostream>

template<typename T>
void print_binary_data(const T& data)
{
    // print size of hellomessage
    std::cout << "sizeof(" << type_name(data) << "): " << sizeof(T) << std::endl;
    for (int i = 0; i < sizeof(T); ++i)
    {
        printf("%02X ", reinterpret_cast<uint8_t *>(&data)[i] & 0xff);
    }
    std::cout << std::endl;
}
