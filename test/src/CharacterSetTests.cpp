/**
 * @file NormalizeCaseInsensitiveStringTests.cpp
 *
 * This module contains the unit tests of the Uri::NormalizeCaseInsensitiveStringTests class.
 *
 * 2019 YaMing Wu
 */

#include <gtest/gtest.h>
#include <src/NormalizeCaseInsensitiveString.hpp>

 TEST(NormalizeCaseInsensitiveStringTests, NormalizeCaseInsensitiveString) {
    ASSERT_EQ(
        "example",
        Uri::NormalizeCaseInsensitiveString("eXAmplE")
    );
    ASSERT_EQ(
        "example",
        Uri::NormalizeCaseInsensitiveString("example")
    );
    ASSERT_EQ(
        "example",
        Uri::NormalizeCaseInsensitiveString("EXAMPLE")
    );
    ASSERT_EQ(
        "foo1bar",
        Uri::NormalizeCaseInsensitiveString("foo1BAR")
    );
    ASSERT_EQ(
        "foo1bar",
        Uri::NormalizeCaseInsensitiveString("fOo1bAr")
    );
    ASSERT_EQ(
        "foo1bar",
        Uri::NormalizeCaseInsensitiveString("foo1bar")
    );
    ASSERT_EQ(
        "foo1bar",
        Uri::NormalizeCaseInsensitiveString("FOO1BAR")
    );
}