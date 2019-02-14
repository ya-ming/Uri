/**
 * @file UriTests.cpp
 *
 * This module contains the unit tests of the Uri::Uri class.
 *
 * © 2019 YaMing Wu
 */

#include <gtest/gtest.h>
#include <Uri/Uri.hpp>

TEST(UriTests, ParseFromString) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com/foo/bar"));
    ASSERT_EQ("http", uri.GetScheme());
    ASSERT_EQ("www.example.com", uri.GetHost());
    ASSERT_EQ(
        (std::vector<std::string> {
            "",
            "foo",
            "bar"
        }),
        uri.GetPath()
    );
}

TEST(UriTests, ParseFromString2) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("urn:book:fantasy:Hobbit"));
    ASSERT_EQ("urn", uri.GetScheme());
    ASSERT_EQ("", uri.GetHost());
    ASSERT_EQ(
        (std::vector<std::string> {
        "book:fantasy:Hobbit"
        }),
        uri.GetPath()
    );

    uri.SetPathDelimiter(":");
    ASSERT_TRUE(uri.ParseFromString("urn:book:fantasy:Hobbit"));
    ASSERT_EQ("urn", uri.GetScheme());
    ASSERT_EQ("", uri.GetHost());
    ASSERT_EQ(
        (std::vector<std::string> {
            "book",
            "fantasy",
            "Hobbit"
        }),
        uri.GetPath()
    );
}

TEST(UriTests, ParseFromStringMultiCharDelimieter) {
    Uri::Uri uri;

    uri.SetPathDelimiter("/-");
    ASSERT_TRUE(uri.ParseFromString("urn:book/-fan/tasy/-Hob-bit"));
    ASSERT_EQ("urn", uri.GetScheme());
    ASSERT_EQ("", uri.GetHost());
    ASSERT_EQ(
        (std::vector<std::string> {
            "book",
            "fan/tasy",
            "Hob-bit"
        }),
        uri.GetPath()
    );
}

TEST(UriTests, ParseFromStringPathCornerCases) {
    struct TestVector {
        std::string pathIn;
        std::vector<std::string> pathOut;
    };

    const std::vector<TestVector> testVectors{
            { "", {} },
            { "/", {""} },
            { "foo/", {"foo", ""} },
            { "/foo", {"", "foo"} },
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.pathIn)) << index;
        ASSERT_EQ(testVector.pathOut, uri.GetPath()) << index;
        ++index;
    }


}
