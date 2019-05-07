/**
 * @file Uri.cpp
 *
 * This module contains the implementation of the Uri::Uri class.
 *
 * © 2019 by YaMing Wu
 */

#include <algorithm>
#include <functional>
#include <memory>
#include <Uri/Uri.hpp>
#include <inttypes.h>
#include "CharacterSet.hpp"
#include "NormalizeCaseInsensitiveString.hpp"
#include "PercentEncodedCharacterDecoder.hpp"


namespace {

    /**
     * This is the character set containing just the alphabetic characters
     * from the ASCII character set.
     */
    const Uri::CharacterSet ALPHA{
        Uri::CharacterSet('a', 'z'),
        Uri::CharacterSet('A', 'Z')
    };

    /**
     * This is the character set containing just the DIGIT characters
     * from the ASCII character set.
     */
    const Uri::CharacterSet DIGIT('0', '9');

    /**
     * This is the character set containing just the characters allowed
     * in a hexadecimal digit.
     */
    const Uri::CharacterSet HEXDIG{
        Uri::CharacterSet('0', '9'),
        Uri::CharacterSet('A', 'F'),
        Uri::CharacterSet('a', 'f')
    };

    /**
     * This is the character set corresponds to the "unreserved" syntax
     * specified in RFC 3986 (https://tools.ietf.org/html/rfc3986).
     */
    const Uri::CharacterSet UNRESERVED{
        ALPHA,
        DIGIT,
        '-', '.', '_', '~'
    };

    /**
     * This is the character set corresponds to the "sub-delims" syntax
     * specified in RFC 3986 (https://tools.ietf.org/html/rfc3986).
     */
    const Uri::CharacterSet SUB_DELIMS{
        '!', '$', '&', '\'', '(', ')',
        '*', '+', ',', ';', '='
    };

    /**
     * This is the character set corresponds to the second of
     * the scheme syntax specified in RFC 3986 (https://tools.ietf.org/html/rfc3986).
     */
    const Uri::CharacterSet SCHEME_NOT_FIRST{
        ALPHA,
        DIGIT,
        '+', '-', '.'
    };

    /**
     * This is the character set corresponds to the "pchar" syntax
     * specified in RFC 3986 (https://tools.ietf.org/html/rfc3986),
     * leaving out "pct_encoded".
     */
    const Uri::CharacterSet PCHAR_NOT_PCT_ENCODED{
        UNRESERVED,
        SUB_DELIMS,
        // (also allowed in segment or pchar)
        ':', '@',
    };

    /**
     * This is the character set corresponds to the "query" syntax
     * and the "fragment" sytax
     * specified in RFC 3986 (https://tools.ietf.org/html/rfc3986),
     * leaving out "pct_encoded".
     */
    const Uri::CharacterSet QUERY_OR_FRAGMENT_NOT_PCT_ENCODED{
        PCHAR_NOT_PCT_ENCODED,
        // (also allowed in query or fragment)
        '/', '?',
    };

    /**
     * This is the character set corresponds to the "userinfo" syntax
     * specified in RFC 3986 (https://tools.ietf.org/html/rfc3986),
     * leaving out "pct_encoded".
     */
    const Uri::CharacterSet USER_INFO_NOT_PCT_ENCODED{
        UNRESERVED,
        SUB_DELIMS,
        // (also allowed in userInfo)
        ':',
    };

    /**
     * This is the character set corresponds to the "reg-name" syntax
     * specified in RFC 3986 (https://tools.ietf.org/html/rfc3986),
     * leaving out "pct_encoded".
     */
    const Uri::CharacterSet REG_NAME_NOT_PCT_ENCODED{
        UNRESERVED,
        SUB_DELIMS,
    };

    /**
     * This is the character set corresponds to the last part of
     * the "IPvFuture" syntax
     * specified in RFC 3986 (https://tools.ietf.org/html/rfc3986).
     */
    const Uri::CharacterSet IPV_FUTURE_LAST_PART{
        UNRESERVED,
        SUB_DELIMS,
        // (also allowed in IPvFuture)
        ':'
    };

    /**
     *  Parse the given string to uint16_t number
     *
     *  @Param[in] numberString
     *      This is the string containing the number to parse.
     *
     *  @Param[out] number
     *      This is where to store the number parsed.
     *
     *  @return
     *      An indication of whether or not the number was parsed successfully is returned.
     */
    bool ParseUint16(const std::string& numberString, uint16_t& number) {
        uint32_t numberIn32bits = 0;
        for (auto c : numberString) {
            if ((c < '0') || (c > '9')) {
                return false;
            }

            numberIn32bits *= 10;
            numberIn32bits += (uint16_t)(c - '0');
            // 1 << 16 == 65536         == 000000000001000000000000
            // (1 << 16) - 1 == 65535   == 000000000000111111111111
            // ~((1 << 16) - 1)         == 111111111111000000000000
            if ((numberIn32bits & ~((1 << 16) - 1)) != 0) {
                return false;
            }
        }
        number = numberIn32bits;
        return true;
    }

    /**
     *  This function takes a given "stillPassing" strategy
     *  and invokes it on the sequence of characters in the given
     *  string, to check if the string passes or not.
     *
     *  @Param[in] numberString
     *      This is the string containing the number to parse.
     *
     *  @Param[out] number
     *      This is where to store the number parsed.
     *
     *  @return
     *      An indication of whether or not the number was parsed successfully is returned.
     */
    bool FailsMatch(
        const std::string candidate,
        std::function< bool(char, bool)> stillPassing
    ) {
        for (const auto c : candidate) {
            if (!stillPassing(c, false)) {
                return true;
            }
        }
        return !stillPassing(' ', true);
    }

    /**
    * This function returns a strategy function that
    * may be used with the FailsMatch function to test a scheme
    * to make sure it is legalaccording to the standard.
    *
    * @return
    *       The strategy function.
    */
    std::function< bool(char, bool) > LegalSchemeCheckStrategy(
//        char c,
//        bool end
    ) {
        auto isFirstCharacter = std::make_shared< bool >(true);
        *isFirstCharacter = true;
        return [isFirstCharacter](char c, bool end) {
            if (end) {
                return !*isFirstCharacter;
            }
            else {
                bool check;
                if (*isFirstCharacter) {
                    check = ALPHA.Contains(c);
                }
                else {
                    check = SCHEME_NOT_FIRST.Contains(c);
                }
                *isFirstCharacter = false;
                return check;
            }
        };
    }

    /**
     * This method checks and decodes the given URI element
     * What we are calling a "URI element" is any part of the URI
     * which is a sequence of characters that:
     * - may be percent-encoded
     * - or a restricted set of characters
     *
     * @param[in, out] element
     *      On input, this is the element to check and decode.
     *      On output, this is the decoded element.
     *
     * @param[in] allowedCharacters
     *      This is the set of characters that do not need to
     *      be percent-encoded
     *
     * @return
     *      An indication of whether or not the given element
     *      passes all the checks and was decoded successfully is returned.
     */
    bool DecodeElement(
        std::string& element,
        const Uri::CharacterSet& allowedCharacters
        ) {
        const auto originalSegment = std::move(element);
        element.clear();

        bool decodingPec = false;
        Uri::PercentEncodedCharacterDecoder pecDecoder;
        for (const auto c : originalSegment) {
            if (decodingPec) {
                if (!pecDecoder.NextEncodedCharacter(c)) {
                    return false;
                }
                if (pecDecoder.Done()) {
                    element.push_back((char)pecDecoder.GetDecodedCharacter());
                    decodingPec = false;
                }
            }
            else if (c == '%') {
                decodingPec = true;
                pecDecoder = Uri::PercentEncodedCharacterDecoder();
            }
            else {
                if (allowedCharacters.Contains(c)) {
                    element.push_back(c);
                }
                else {
                    return false;
                }

            }
        }
        return true;
    }
}

/**
 * This method checks and decodes the given path queryOrFragment
 *
 * @param[in, out] queryOrFragment
 *      On input, this is the queryOrFragment to check and decode.
 *      On output, this is the decoded queryOrFragment.
 *
 * @return
 *      An indication of whether or not the given queryOrFragment
 *      passes all the checks and was decoded successfully is returned.
 */
bool DecodeQueryOrFragment(std::string& queryOrFragment) {
    return DecodeElement(
        queryOrFragment,
        QUERY_OR_FRAGMENT_NOT_PCT_ENCODED
    );
}

namespace Uri {
    /**
     * This contains the private properties of a Uri instance.
     */
    struct Uri::Impl {
        // Parameters
        std::string scheme;
        std::string userInfo;
        std::string host;
        std::vector<std::string> path;
        bool hasPort = false;
        uint16_t port = 0;
        std::string query;
        std::string fragment;

        // Methods

        /**
         * This method builds the internal path element sequence
         * by parsing it from the given sring.
         *
         * @param[in] candidate
         *      This is the string to test.
         *
         * @param[in] stillPassing
         *      This is the strategy to invoke in order to test the string.
         *
         * @return
         *      An indication of whether or not the given candidate string
         *      passes the test is returned.
         */
        bool ParsePath(std::string pathString) {
            path.clear();

            if (pathString == "/") {
                // special case of a path that is empty but needs a single
                // empty-string element to indicate that it is absolute
                path.push_back("");
                pathString.clear();
            }
            else if (!pathString.empty()) {
                for (;;) {
                    auto pathDelimiter = pathString.find("/");

                    if (pathDelimiter == std::string::npos) {
                        path.push_back(pathString);
                        pathString.clear();
                        break;
                    }
                    else {
                        path.emplace_back(
                            pathString.begin(),
                            pathString.begin() + pathDelimiter
                        );
                        pathString = pathString.substr(pathDelimiter + 1);

                    }
                }
            }
            for (auto& segment : path) {
                if (!DecodeElement(segment, PCHAR_NOT_PCT_ENCODED)) {
                    return false;
                }
            }
            return true;
        }

        /**
         * This method parses the elements that make up the authority
         * composite part of the URI, by parsing it from the given string.
         *
         * @param[in] authorityString
         *      This is the string containing the whole authority part of the URI.
         *
         * @return
         *      An indication if the authority was parsed correctly or not is returned.
         */
        bool ParseAuthority(const std::string& authorityString) {
            // Next, check if there is a UserInfo, and if so, extract it
            const auto userInfoDelimiter = authorityString.find('@');
            std::string hostPortString;

            if (userInfoDelimiter == std::string::npos) {
                userInfo.clear();
                hostPortString = authorityString;
            }
            else {
                userInfo = authorityString.substr(0, userInfoDelimiter);

                if (!DecodeElement(userInfo, USER_INFO_NOT_PCT_ENCODED)) {
                    return false;
                }
                hostPortString = authorityString.substr(userInfoDelimiter + 1);
            }

            // paring host and port from authority
            std::string portString;

            size_t decoderState = 0;
            int decodedCharacter = 0;
            host.clear();
            PercentEncodedCharacterDecoder pecDecoder;
            bool hostIsRegName = false;
            for (const auto c : hostPortString) {
                switch (decoderState) {
                case 0: { // first character
                    if (c == '[') {
                        host.push_back(c);
                        decoderState = 3;
                        break;
                    }
                    else {
                        decoderState = 1;
                        hostIsRegName = true;
                    }
                }
                case 1: { // reg-name or IPv4Address
                    if (c == '%') {
                        pecDecoder = PercentEncodedCharacterDecoder();
                        decoderState = 2;
                    }
                    else if (c == ':') {
                        decoderState = 8;
                    }
                    else {
                        if (REG_NAME_NOT_PCT_ENCODED.Contains(c)) {
                            host.push_back(c);
                        }
                        else {
                            return false;
                        }
                    }
                    break;
                }
                case 2: {
                    if (!pecDecoder.NextEncodedCharacter(c)) {
                        return false;
                    }
                    if (pecDecoder.Done()) {
                        decoderState = 0;
                        host.push_back((char)pecDecoder.GetDecodedCharacter());
                    }
                    break;
                }
                case 3: { // IP-literal
                    if (c == 'v') {
                        host.push_back(c);
                        decoderState = 5;
                        break;
                    }
                    else {
                        decoderState = 4;
                    }
                }
                case 4: { // IPv6address
                    // TODO

                    host.push_back(c);
                    if (c == ']') {
                        decoderState = 7;
                    }
                    break;
                }
                case 5: { // IPvFuture: v ...
                    if (c == '.') {
                        decoderState = 6;
                    }
                    else if (!HEXDIG.Contains(c)) {
                        return false;
                    }
                    host.push_back(c);
                    break;
                }
                case 6: { // IPvFuture
                    host.push_back(c);

                    if (c == ']') {
                        decoderState = 7;
                    }
                    else if (!IPV_FUTURE_LAST_PART.Contains(c)) {
                        return false;
                    }
                    break;
                }
                case 7: { // illegal to have anything else, unless it's a colon,
                          // in which case it's a port delimiter
                    if (c == ':') {
                        decoderState = 8;
                    }
                    else {
                        return false;
                    }
                    break;
                }
                case 8: { // port
                    portString.push_back(c);
                    break;
                }
                }
            }
            if (hostIsRegName) {
                host = NormalizeCaseInsensitiveString(host);
            }

            if (portString.empty())
            {
                hasPort = false;
            }
            else {
                if (!ParseUint16(portString, port)) {
                    return false;
                }
                hasPort = true;
            }
            return true;
        }

        /**
         * This method takes the part of a URI string that has just
         * the query and/or fragment elements, and breaks off
         * and decodes the fragment part, returning the rest,
         * which will be either empty or have the query with the
         * query delimiter still attached.
         *
         * @param[in] queryAndOrFragment
         *      This is the part of a URI string that has just
         *      the query and/or fragment elements.
         *
         * @param[in] rest
         *      This is where to store the rest of the input string
         *      after removing any fragment and fragment delimiter.
         *
         * @return
         *      An indication of whether or not the method succeeded
         *      is returned.
         */
        bool ParseFragment(
            const std::string& queryAndOrFragment,
            std::string& rest
        ) {
            const auto fragmentDelimiter = queryAndOrFragment.find('#');
            if (fragmentDelimiter == std::string::npos) {
                fragment.clear();
                rest = queryAndOrFragment;
            }
            else {
                fragment = queryAndOrFragment.substr(fragmentDelimiter + 1);
                rest = queryAndOrFragment.substr(0, fragmentDelimiter);
            }
            return DecodeQueryOrFragment(fragment);
        }

        /**
         * This method takes the part of a URI string that has just
         * the query element with its delimiter, and breaks off
         * and decodes the query.
         *
         * @param[in] queryWithDelimiter
         *      This is the part of a URI string that has just
         *      the query and its delimiter.
         *
         * @return
         *      An indication of whether or not the method succeeded
         *      is returned.
         */
        bool ParseQuery(const std::string& queryWithDelimiter) {
            if (!queryWithDelimiter.empty()) {
                query = queryWithDelimiter.substr(1);
            }
            else {
                query.clear();
            }

            return DecodeQueryOrFragment(query);
        }
    };

    Uri::~Uri() = default;
    Uri::Uri(Uri&&) = default;
    Uri& Uri::operator=(Uri&&) = default;

    Uri::Uri()
        : impl_(new Impl)
    {
    }

    bool Uri::operator==(const Uri& other) const {
        return (
            (impl_->scheme == other.impl_->scheme)
            && (impl_->userInfo == other.impl_->userInfo)
            && (impl_->host == other.impl_->host)
            && (
                (!impl_->hasPort && !other.impl_->hasPort)
                || (
                    (impl_->hasPort && other.impl_->hasPort)
                    && (impl_->port == other.impl_->port)
                    )
                )
            && (impl_->path == other.impl_->path)
            && (impl_->query == other.impl_->query)
            && (impl_->fragment == other.impl_->fragment)
            );
    }


    bool Uri::operator!=(const Uri& other) const {
        return !(*this == other);
            
    }

    std::ostream& operator<<(std::ostream &strm, const Uri &uri) {

        std::string pathString = "";
        for (auto p : uri.GetPath()) {
            pathString = pathString + "\"" + p + "\",";
        }

        return strm << "Uri(\n" << 
            "\n\t  Scheme:" << uri.GetScheme() <<
            "\n\t    Host:" << uri.GetHost() <<
            "\n\t    Port:" << uri.GetPort() <<
            "\n\tUserInfo:" << uri.GetUserInfo() <<
            "\n\t    Path:" << pathString <<
            "\n\t   Query:" << uri.GetQuery() <<
            "\n\tFragment:" << uri.GetFragment() << 
            "\n)";
    }

    bool Uri::ParseFromString(const std::string & uriString)
    {
        // First parse the scheme
        // Limit our search so we don't scan into the authority
        // or path elements, because these may have the colon
        // character as well, which we might misinterpret
        // as the scheme delimiter.
        auto authorityOrPathDelimiterStart = uriString.find('/');
        if (authorityOrPathDelimiterStart == std::string::npos) {
            authorityOrPathDelimiterStart = uriString.length();
        }
        const auto schemeEnd = uriString.substr(0, authorityOrPathDelimiterStart).find(':');
        std::string rest;
        if (schemeEnd == std::string::npos) {
            impl_->scheme.clear();
            rest = uriString;
        }
        else {
            impl_->scheme = uriString.substr(0, schemeEnd);

            bool isFirstCharacter = true;
            if (
                FailsMatch(
                    impl_->scheme,
                    LegalSchemeCheckStrategy()
                )
                ) {
                return false;
            }

            impl_->scheme = NormalizeCaseInsensitiveString(impl_->scheme);
            rest = uriString.substr(schemeEnd + 1);
        }

        // next parse the authority
        const auto pathEnd = rest.find_first_of("?#");
        auto authorityAndPathString = rest.substr(0, pathEnd);
        const auto queryAndOrFragment = rest.substr(authorityAndPathString.length());

        std::string pathString;
        impl_->port = 0;
        if (authorityAndPathString.substr(0, 2) == "//") {
            // strip off authority marker
            authorityAndPathString = authorityAndPathString.substr(2);

            // first separate the authority from the path
            auto authorityEnd = authorityAndPathString.find("/");
            if (authorityEnd == std::string::npos) {
                authorityEnd = authorityAndPathString.length();
            }
            pathString = authorityAndPathString.substr(authorityEnd);
            auto authorityString = authorityAndPathString.substr(0, authorityEnd);

            // Parse the elemnts inside the authority string.
            if (!impl_->ParseAuthority(authorityString)) {
                return false;
            }
        }
        else {
            impl_->hasPort = false;
            impl_->userInfo.clear();
            impl_->host.clear();

            pathString = authorityAndPathString;
        }

        // next, parse the path
        if (!impl_->ParsePath(pathString)) {
            return false;
        }

        // parse the fragment if there is one
        if (!impl_->ParseFragment(queryAndOrFragment, rest)) {
            return false;
        }

        // Handle special case of absolute URI with empty
        // path -- treat the same as "/" path.
        if (
            !impl_->scheme.empty()
            && impl_->path.empty()
            ) {
            impl_->path.push_back("");
        }

        // Finally, if anyting is letf, it's the query
        return impl_->ParseQuery(rest);

        return true;
    }

    std::string Uri::GetScheme() const
    {
        return impl_->scheme;
    }

    std::string Uri::GetUserInfo() const
    {
        return impl_->userInfo;
    }

    std::string Uri::GetHost() const
    {
        return impl_->host;
    }

    std::vector<std::string> Uri::GetPath() const
    {
        return impl_->path;
    }

    bool Uri::HasPort() const
    {
        return impl_->hasPort;
    }

    uint16_t Uri::GetPort() const
    {
        return impl_->port;
    }

    bool Uri::IsRelativeReference() const
    {
        return impl_->scheme.empty();
    }

    bool Uri::ContainsRelativePath() const
    {
        if (impl_->path.empty()) {
            return true;
        }
        else {
            return !impl_->path[0].empty();
        }
    }

    std::string Uri::GetFragment() const
    {
        return impl_->fragment;
    }

    std::string Uri::GetQuery() const
    {
        return impl_->query;
    }

    /*
     * This is a straight-up implementation of the
     * algorithm from section 5.2.4 of
     * RFC 3986 (https://tools.ietf.org/html/rfc3986).
     */
    void Uri::NormalizePath() {
         // Step 1
        auto oldPath = std::move(impl_->path);
        impl_->path.clear();
        // Step 2
        while (!oldPath.empty()) {
            // Step 2A
            if (
                (oldPath[0] == ".")
                || (oldPath[0] == "..")
                ) {
                oldPath.erase(oldPath.begin());
            }
            else

            // Step 2B
            if (
                (oldPath.size() >= 2)
                && (oldPath[0] == "")
                && (oldPath[1] == ".")
                ) {
                oldPath.erase(oldPath.begin() + 1);
            }
            else

            // Step 2C
            if (
                (oldPath.size() >= 2)
                && (oldPath[0] == "")
                && (oldPath[1] == "..")
                ) {
                oldPath.erase(oldPath.begin() + 1);
                if (!impl_->path.empty()) {
                    impl_->path.pop_back();
                }
            }
            else

            // Step 2D
            if (
                (oldPath.size() == 1)
                && (
                (oldPath[0] == ".")
                    || (oldPath[0] == "..")
                    )
                ) {
                oldPath.erase(oldPath.begin());
            }
            else

             // Step 2E
            {
                if (oldPath[0] == "") {
                    if (impl_->path.empty()) {
                        impl_->path.push_back("");
                    }
                    oldPath.erase(oldPath.begin());
                }
                if (!oldPath.empty()) {
                    impl_->path.push_back(oldPath[0]);
                    if (oldPath.size() > 1) {
                        oldPath[0] = "";
                    }
                    else {
                        oldPath.erase(oldPath.begin());
                    }
                }
            }
        }
    }


    /*
     * Resolve the reference by following the
     * algorithm from section 5.2.2 of
     * RFC 3986 (https://tools.ietf.org/html/rfc3986).
     */
    Uri Uri::Resolve(const Uri& relativereference) const {
        Uri target;
        if (!relativereference.impl_->scheme.empty()) {
            target.impl_->scheme = relativereference.impl_->scheme;
            target.impl_->host = relativereference.impl_->host;
            target.impl_->userInfo = relativereference.impl_->userInfo;
            target.impl_->hasPort = relativereference.impl_->hasPort;
            target.impl_->port = relativereference.impl_->port;
            target.impl_->path = relativereference.impl_->path;
            target.NormalizePath();
            target.impl_->query = relativereference.impl_->query;
        }
        else
        {
            if (!relativereference.impl_->host.empty()) {
                target.impl_->host = relativereference.impl_->host;
                target.impl_->userInfo = relativereference.impl_->userInfo;
                target.impl_->hasPort = relativereference.impl_->hasPort;
                target.impl_->port = relativereference.impl_->port;
                target.impl_->path = relativereference.impl_->path;
                target.NormalizePath();
                target.impl_->query = relativereference.impl_->query;
            }
            else {
                if (relativereference.impl_->path.empty()) {
                    target.impl_->path = impl_->path;
                    if (!relativereference.impl_->query.empty()) {
                        target.impl_->query = relativereference.impl_->query;
                    }
                    else {
                        target.impl_->query = impl_->query;
                    }
                }
                else {
                    if (!relativereference.impl_->path.empty() && relativereference.impl_->path[0] == "") {
                        target.impl_->path = relativereference.impl_->path;
                        target.NormalizePath();
                    }
                    else {
                        target.impl_->path = impl_->path;
                        if (!target.impl_->path.empty()) {
                            target.impl_->path.pop_back();
                        }
                        std::copy(
                            relativereference.impl_->path.begin(),
                            relativereference.impl_->path.end(),
                            std::back_inserter(target.impl_->path)
                        );
                        target.NormalizePath();
                    }
                    target.impl_->query = relativereference.impl_->query;
                }
                target.impl_->host = impl_->host;
                target.impl_->userInfo = impl_->userInfo;
                target.impl_->hasPort = impl_->hasPort;
                target.impl_->port = impl_->port;
            }
            target.impl_->scheme = impl_->scheme;
        }

        target.impl_->fragment = relativereference.impl_->fragment;
        return target;
    }
}
