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

#include "Connection.h"

#include "ExceptionComm.h"

using namespace comm;

void Connection::send_message(const uint8_t* data, uint32_t size)
{
    if (size > _max_buffer_size) {
        std::string error_msg = "Cannot send messages larger than " +
                                msg_size_to_str_KB(_max_buffer_size) + "KB." +
                                " Message size is " +
                                msg_size_to_str_KB(size) + "KB.";
        throw ExceptionComm(InvalidMessageSize, error_msg);
    }

    auto ret0 = write(reinterpret_cast<const uint8_t*>(&size), sizeof(size));

    if (ret0 != sizeof(size)) {
        throw ExceptionComm(WriteFail);
    }

    size_t bytes_sent = 0;

    while (bytes_sent < size) {
        bytes_sent += write(data + bytes_sent, size - bytes_sent);
    }
}

const std::basic_string<uint8_t>& Connection::recv_message()
{
    uint32_t recv_message_size;

    auto recv_and_check = [this](uint8_t* buffer, uint32_t size)
    {
        size_t bytes_recv = 0;

        while (bytes_recv < size) {
            bytes_recv += read(buffer + bytes_recv, size - bytes_recv);
        }

        return bytes_recv;
    };

    size_t bytes_recv = recv_and_check(reinterpret_cast<uint8_t*>(&recv_message_size),
                                       sizeof(uint32_t));

    if (bytes_recv != sizeof(recv_message_size)) {
        throw ExceptionComm(ReadFail);
    }

    if (recv_message_size > _max_buffer_size) {
        std::string error_msg = "Cannot recieve messages larger than " +
                                msg_size_to_str_KB(_max_buffer_size) + "KB." +
                                "Received size: " + std::to_string(recv_message_size);
        throw ExceptionComm(InvalidMessageSize, error_msg);
    }

    _buffer_str.resize(recv_message_size);

    bytes_recv = recv_and_check(const_cast<uint8_t*>(_buffer_str.data()), recv_message_size);

    if (recv_message_size != bytes_recv) {
        throw ExceptionComm(ReadFail);
    }

    if (recv_message_size != _buffer_str.size()) {
        throw ExceptionComm(ReadFail);
    }

    return _buffer_str;
}