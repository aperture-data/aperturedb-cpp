/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include "gtest/gtest.h"

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    // To make GoogleTest silent:
    // if (true) {
    //     auto& listeners = ::testing::UnitTest::GetInstance()->listeners();
    //     delete listeners.Release(listeners.default_result_printer());
    // }
    return RUN_ALL_TESTS();
}
