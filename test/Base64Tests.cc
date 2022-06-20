/**
 * @copyright Copyright (c) 2022 ApertureData Inc.
 */

#include "gtest/gtest.h"

#include "util/Base64.h"

TEST(Base64Test, string)
{
    std::string str("The quick brown fox jumps over the lazy dog.");
    auto b64 = Base64::encode(str);
    auto decoded_bytes = Base64::decode(b64);
    std::string round_trip(decoded_bytes.begin(), decoded_bytes.end());

    EXPECT_EQ(str, round_trip);
}

TEST(Base64Test, partial_string)
{
    std::string str("The quick brown fox jumps over the lazy dog.");
    for (std::size_t i = 0; i < 20; ++i ) {
        auto b64 = Base64::encode(str.data(), i);
        auto decoded_bytes = Base64::decode(b64);
        std::string round_trip(decoded_bytes.begin(), decoded_bytes.end());

        EXPECT_EQ(str.substr(0,i), round_trip);
    }
}

TEST(Base64Test, serialize_pod)
{
    struct my_pod {
        int x;
        float y;
        bool flag;
        std::array<char, 16> buf;
    };

    std::vector< my_pod > my_pods = {
        my_pod{ 1, 2, true, "first thing" },
        my_pod{ 3, 4, false, "second thing" },
        my_pod{ 5, 6, true, "third thing" },
        my_pod{ 7, 8, false, "fourth thing" },
    };

    auto b64 = Base64::encode(my_pods);
    auto decoded_bytes = Base64::decode(b64);
    ASSERT_EQ(decoded_bytes.size(), sizeof(my_pod) * my_pods.size());
    const auto* rt_pods = reinterpret_cast< const my_pod* >(decoded_bytes.data());
    for (std::size_t i = 0; i < my_pods.size(); ++i) {
        EXPECT_EQ(my_pods[i].x, rt_pods[i].x);
        EXPECT_EQ(my_pods[i].y, rt_pods[i].y);
        EXPECT_EQ(my_pods[i].flag, rt_pods[i].flag);
        EXPECT_EQ(my_pods[i].buf, rt_pods[i].buf);
    }
}
