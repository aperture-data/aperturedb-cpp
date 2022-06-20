/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#pragma once

#include <string>
#include <vector>
#include <cassert>
#include <openssl/evp.h>

class Base64 {
public:
    // static methods only
    Base64() = delete;

    static constexpr std::size_t encoded_bytes( std::size_t bytes )
    {
        return ( ( ( bytes + 2 ) / 3 ) * 4 );
    }

    static constexpr std::size_t decoded_bytes( std::size_t chars )
    {
        return ( ( ( chars + 3 ) / 4 ) * 3 );
    }

    template< typename IN >
    static std::string encode( const IN* in, std::size_t in_size ) {
        auto in_size_bytes = in_size * sizeof(IN);
        auto enc_bytes = encoded_bytes(in_size_bytes);
        std::string out(enc_bytes, '\0');
        auto written = EVP_EncodeBlock(reinterpret_cast< unsigned char* >(out.data()),
            reinterpret_cast< const unsigned char* >(in), in_size_bytes);
        assert(written == enc_bytes);
        return out;
    }

    template< typename IN >
    static std::vector< unsigned char > decode( const IN* in, std::size_t in_size ) {
        auto in_size_bytes = in_size * sizeof(IN);
        assert(in_size_bytes % 4 == 0);
        auto dec_bytes = decoded_bytes(in_size_bytes);
        std::vector< unsigned char > out(dec_bytes);
        auto written = EVP_DecodeBlock( out.data(),
            reinterpret_cast< const unsigned char* >(in), in_size_bytes);
        assert(written == dec_bytes);
        if (in_size > 0) {
            for (auto* tail = in + (in_size - 1); *tail == IN('='); --tail) {
                --written;
            }
            out.resize(written);
        }
        return out;
    }

    template< typename IN_STR >
    static std::string encode( const IN_STR& in ) {
        return encode(in.data(), in.size());
    }

    template< typename IN_STR >
    static std::vector< unsigned char > decode( const IN_STR& in ) {
        return decode(in.data(), in.size());
    }
};
