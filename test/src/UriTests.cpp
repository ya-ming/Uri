/**
 * @file UriTests.cpp
 *
 * This module contains the unit tests of the Uri::Uri class.
 *
 * © 2019 YaMing Wu
 */

#include <gtest/gtest.h>
#include <Uri/Uri.hpp>
#include "UriTests.h"

TEST(UriTests, ParseFromStringNoScheme) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("foo/bar"));
    ASSERT_EQ("", uri.GetScheme());
    ASSERT_EQ("", uri.GetHost());
    ASSERT_EQ(
        (std::vector<std::string> {
            "foo",
            "bar"
        }),
        uri.GetPath()
    );
}

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
}

TEST(UriTests, ParseFromStringEndsAfterAuthority) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com"));
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

TEST(UriTests, ParseFromStringHasAPortNumber) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com:8080/foo/bar"));
    ASSERT_EQ("www.example.com", uri.GetHost());
    ASSERT_TRUE(uri.HasPort());
    ASSERT_EQ(8080, uri.GetPort());
}

TEST(UriTests, ParseFromStringDoseNotHasAPortNumber) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com/foo/bar"));
    ASSERT_EQ("www.example.com", uri.GetHost());
    ASSERT_FALSE(uri.HasPort());
    ASSERT_EQ(0, uri.GetPort());
}

TEST(UriTests, ParseFromStringTwiceFirstWithPortNumberThenWithout) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com:8080/foo/bar"));
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com/foo/bar"));
    ASSERT_FALSE(uri.HasPort());
    ASSERT_EQ(0, uri.GetPort());
}

TEST(UriTests, ParseFromStringTwiceFirstUserInfothenWithoutPort) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://joe@www.example.com/foo/bar"));
    ASSERT_FALSE(uri.GetUserInfo().empty());
    ASSERT_TRUE(uri.ParseFromString("/foo/bar"));
    ASSERT_TRUE(uri.GetUserInfo().empty());
}

TEST(UriTests, ParseFromStringBadPortNumber) {
    Uri::Uri uri;
    ASSERT_FALSE(uri.ParseFromString("http://www.example.com:spam/foo/bar"));
    ASSERT_FALSE(uri.ParseFromString("http://www.example.com:8080spam/foo/bar"));
    ASSERT_FALSE(uri.ParseFromString("http://www.example.com:65536/foo/bar"));
    ASSERT_FALSE(uri.ParseFromString("http://www.example.com:-1234/foo/bar"));
}

TEST(UriTests, ParseFromStringRelativeVsNonRelativeReference) {
    struct TestVector {
        std::string uriString;
        bool isRelative;
    };

    const std::vector<TestVector> testVectors{
            { "http://www.example.com/", false },
            { "http://www.example.com", false },
            { "/", true },
            { "/foo", true },
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.isRelative, uri.IsRelativeReference()) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringRelativePathVsNonRelativePath) {
    struct TestVector {
        std::string uriString;
        bool isRelativePath;
    };

    const std::vector<TestVector> testVectors{
            { "http://www.example.com/", false },
            { "http://www.example.com", true },
            { "/", false },
            { "foo", true },
            { "", true },   // Note: ??? correct ??? Is an empty string a valid relative reference URI with an empty path ???
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.isRelativePath, uri.ContainsRelativePath()) << index;
        ++index;
    }
}


TEST(UriTests, ParseFromStringQueryAndFragmentElements) {
    struct TestVector {
        std::string uriString;
        std::string host;
        std::string query;
        std::string fragment;
    };

    const std::vector<TestVector> testVectors {
            { "http://www.example.com/", "www.example.com", "", "" },
            { "http://example.com?foo", "example.com", "foo", "" },
            { "http://www.example.com#foo", "www.example.com", "", "foo" },
            { "http://www.example.com/?earth?day#bar", "www.example.com", "earth?day", "bar" },
            { "http://www.example.com/spam?foo#bar", "www.example.com", "foo", "bar" },

            // maybe this is correct, that having atrailing question mark is
            // equivalent to not having any question mark.
            { "http://www.example.com/?", "www.example.com", "", "" },

    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.host, uri.GetHost()) << index;
        ASSERT_EQ(testVector.query, uri.GetQuery()) << index;
        ASSERT_EQ(testVector.fragment, uri.GetFragment()) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringUserInfo) {
    struct TestVector {
        std::string uriString;
        std::string userInfo;
    };

    const std::vector<TestVector> testVectors{
            { "http://www.example.com/", "" },
            { "http://joe@www.example.com", "joe" },
            { "http://bob:password@www.example.com", "bob:password" },
            { "//www.example.com", "" },
            { "//bob@www.example.com", "bob" },
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.userInfo, uri.GetUserInfo()) << index;
        ++index;
    }
}

