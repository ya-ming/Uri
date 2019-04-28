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

TEST(UriTests, ParseFromStringSchemeIllegalCharacters) {
    const std::vector<std::string> testVectors{
            { "://www.example.com/"},
            { "0://www.example.com/"},
            { "+://www.example.com/"},
            { "@://www.example.com/"},
            { ".://www.example.com/"},
            { "h@://www.example.com/"},
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(testVector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringSchemeBarelyLegal) {
    struct TestVector {
        std::string uriString;
        std::string scheme;
    };

    const std::vector<TestVector> testVectors{
            { "h://www.example.com/", "h"},
            { "x+://www.example.com/", "x+"},
            { "y-://www.example.com/", "y-"},
            { "z.://www.example.com/", "z."},
            { "aa://www.example.com/", "aa"},
            { "a0://www.example.com/", "a0"},
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.scheme, uri.GetScheme()) << index;
        ++index;
    }
}


TEST(UriTests, ParseFromStringUserInfoIllegalCharacters) {
    const std::vector<std::string> testVectors{
            { "//%X@www.example.com/"},
            { "//^@www.example.com/"},
            { "//{@www.example.com/"},
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(testVector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringUserInfoBarelyLegal) {
    struct TestVector {
        std::string uriString;
        std::string userInfo;
    };

    const std::vector<TestVector> testVectors{
            { "//%41@www.example.com/", "A"},
            { "//@www.example.com/", ""},
            { "//!@www.example.com/", "!"},
            { "//'@www.example.com/", "'"},
            { "//(@www.example.com/", "("},
            { "//;@www.example.com/", ";"},
            { "http://:@www.example.com/", ":"},
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.userInfo, uri.GetUserInfo()) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringHostIllegalCharacters) {
    const std::vector<std::string> testVectors{
            { "//%X@www.example.com/"},
            { "//@www:example.com/"},
            { "//[vX.:]/"},
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(testVector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringHostBarelyLegal) {
    struct TestVector {
        std::string uriString;
        std::string host;
    };

    const std::vector<TestVector> testVectors{
            { "//%41/", "A"},
            { "///", ""},
            { "//!/", "!"},
            { "//'/", "'"},
            { "//(/", "("},
            { "//;/", ";"},
            { "//1.2.3.4/", "1.2.3.4"},
            { "//[v7.:]/", "[v7.:]"},
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.host, uri.GetHost()) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringDontMisinterpretColonInOtherPlacesAsSchemeDelimiter) {
    const std::vector<std::string> testVectors{
            { "//foo:bar@www.example.com/"},
            { "//www.example.com/a:b"},
            { "//www.example.com/foo?a:b"},
            { "//www.example.com/foo#a:b"},
            { "//[v7.:]/"},
            { "/:/foo"},
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector)) << index;
        ASSERT_TRUE(uri.GetScheme().empty());
        ++index;
    }
}

TEST(UriTests, ParseFromStringPathIllegalCharacters) {
    const std::vector<std::string> testVectors{
            { "http://www.example.com/foo[bar"},
            { "http://www.example.com/]bar"},
            { "http://www.example.com/foo]"},
            { "http://www.example.com/["},
            { "http://www.example.com/abc/foo]"},
            { "http://www.example.com/abc/["},
            { "http://www.example.com/foo]/abc"},
            { "http://www.example.com/[/abc"},
            { "http://www.example.com/[/"},
            { "/foo[bar"},
            { "/]bar"},
            { "/foo]"},
            { "/["},
            { "/abc/foo]"},
            { "/abc/["},
            { "/foo]/abc"},
            { "/[/abc"},
            { "/[/"},
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(testVector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringPathBarelyLegal) {
    struct TestVector {
        std::string uriString;
        std::vector< std::string> path;
    };

    const std::vector<TestVector> testVectors{
            { "/:/foo", {"", ":", "foo"}},
            { "bob@/foo",{"bob@", "foo"}},
            { "hello!",{"hello!"}},
            { "urn:hello,%20w%6Frld!",{"hello, world!"}},
            { "//example.com/foo/(bar)/",{ "", "foo", "(bar)", ""}},
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.path, uri.GetPath()) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringQueryIllegalCharacters) {
    const std::vector<std::string> testVectors{
            { "http://www.example.com/?foo[bar"},
            { "http://www.example.com/?]bar"},
            { "http://www.example.com/?foo]"},
            { "http://www.example.com/?["},
            { "http://www.example.com/?abc/foo]"},
            { "http://www.example.com/?abc/["},
            { "http://www.example.com/?foo]/abc"},
            { "http://www.example.com/?[/abc"},
            { "http://www.example.com/?[/"},
            { "?foo[bar"},
            { "?]bar"},
            { "?foo]"},
            { "?["},
            { "?abc/foo]"},
            { "?abc/["},
            { "?foo]/abc"},
            { "?[/abc"},
            { "?[/"},
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(testVector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringQueryBarelyLegal) {
    struct TestVector {
        std::string uriString;
        std::string query;
    };

    const std::vector<TestVector> testVectors{
            { "/?:/foo", ":/foo"},
            { "?bob@/foo","bob@/foo"},
            { "?hello!","hello!"},
            { "urn:?hello,%20w%6Frld!","hello, world!"},
            { "//example.com/foo?(bar)/", "(bar)/"},
            {"http://www.example.com/?foo?bar", "foo?bar"},
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.query, uri.GetQuery()) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringFragmentIllegalCharacters) {
    const std::vector<std::string> testVectors{
            { "http://www.example.com/?foo[bar"},
            { "http://www.example.com/?]bar"},
            { "http://www.example.com/?foo]"},
            { "http://www.example.com/?["},
            { "http://www.example.com/?abc/foo]"},
            { "http://www.example.com/?abc/["},
            { "http://www.example.com/?foo]/abc"},
            { "http://www.example.com/?[/abc"},
            { "http://www.example.com/?[/"},
            { "#foo[bar"},
            { "#]bar"},
            { "#foo]"},
            { "#["},
            { "#abc/foo]"},
            { "#abc/["},
            { "#foo]/abc"},
            { "#[/abc"},
            { "#[/"},
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(testVector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringFragmentBarelyLegal) {
    struct TestVector {
        std::string uriString;
        std::string fragment;
    };

    const std::vector<TestVector> testVectors{
            { "/#:/foo", ":/foo"},
            { "#bob@/foo","bob@/foo"},
            { "#hello!","hello!"},
            { "urn:#hello,%20w%6Frld!","hello, world!"},
            { "//example.com/foo#(bar)/", "(bar)/"},
            {"http://www.example.com/#foo?bar", "foo?bar"},
    };

    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.fragment, uri.GetFragment()) << index;
        ++index;
    }
}
