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
#include <vector>
#include <memory>

namespace comm
{

class Socket
{
   public:
    virtual ~Socket() {}

    virtual bool listen()                                                      = 0;
    virtual bool set_boolean_option(int level, int option_name, bool value)    = 0;
    virtual bool set_timeval_option(int level, int option_name, timeval value) = 0;
    virtual void shutdown()                                                    = 0;

    virtual std::unique_ptr< Socket > accept() = 0;

    static std::pair< int, std::unique_ptr< Socket > > accept(
        std::vector< std::unique_ptr< Socket > >& listening_sockets);

    virtual std::string print_source() = 0;
    virtual short source_family()      = 0;
    virtual int fd() const = 0;
};
}  // namespace comm
