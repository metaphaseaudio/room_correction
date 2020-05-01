/*
 * AllTests.cpp
 *      Author: Matt Zapp
 */
#include <gtest/gtest.h>
#include "tests/test_IRCalculator.h"


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
