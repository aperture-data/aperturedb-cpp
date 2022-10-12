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

#pragma once

#include <string>
#include "util/Macros.h"

namespace comm
{

class ConnMetrics
{
   public:
    virtual ~ConnMetrics() = 0;
    virtual void observe_bytes_sent(std::size_t bytes_sent);
    virtual void observe_bytes_recv(std::size_t bytes_recv);
};

class Connection
{
   public:
    explicit Connection(ConnMetrics* metrics = nullptr);
    virtual ~Connection();

    MOVEABLE_BY_DEFAULT(Connection);
    NOT_COPYABLE(Connection);

    void send_message(const uint8_t* data, uint32_t size);
    const std::basic_string< uint8_t >& recv_message();

    std::string msg_size_to_str_KB(uint32_t size);
    void set_max_buffer_size(uint32_t max_buffer_size);
    bool check_message_size(uint32_t size);

    std::string source_family_name( short source_family );
    const std::string& get_source();
    short get_source_family();
    const std::string& get_encryption();

   protected:
    virtual size_t read(uint8_t* buffer, size_t length)        = 0;
    virtual size_t write(const uint8_t* buffer, size_t length) = 0;

    std::basic_string< uint8_t > _buffer_str{};
    uint32_t _max_buffer_size{};

    ConnMetrics* _metrics{nullptr};

    std::string _source{};
    std::string _encryption{};
    short _source_family; // from socket(2)

};

};  // namespace comm
