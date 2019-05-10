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
#include <sstream>
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
     * This function checks to make sure the given string
     * is a valid rendering of an octet as a decimal number.
     *
     * @param[in] octetString
     *     This is the octet string to validate.
     *
     * @return
     *     An indication of whether or not the given astring
     *     is a valid rendering of an octet as a
     *     decimal number is returned.
     */
    bool ValidateOctet(const std::string& octetString) {
        int octet = 0;
        for (auto c : octetString) {
            if (DIGIT.Contains(c)) {
                octet *= 10;
                octet += (int)(c - '0');
            }
            else {
                return false;
            }
        }
        return (octet <= 255);
    }

    /**
    * This function checks to make sure the given address
    * is a valid IPv6 address according to the rules in
    * RFC 3986 (https://tools.ietf.org/html/rfc3986).
    *
    * @param[in] address
    *     This is the IPv6 address to validate.
    *
    * @return
    *     An indication of whether or not the given address
    *     is a valid IPv6 address is returned.
    */
    bool ValidateIpv4Adress(const std::string& address) {
        size_t numGroups = 0;
        size_t state = 0;
        std::string octetBuffer;
        for (auto c : address) {
            switch (state) {
            case 0: { // not in an octet yet
                if (DIGIT.Contains(c)) {
                    octetBuffer.push_back(c);
                    state = 1;
                }
                else {
                    return false;
                }
                break;
            }
            case 1: { // expect a digit or dot
                if (c == '.') {
                    if (numGroups++ >= 4) {
                        return false;
                    }
                    if (!ValidateOctet(octetBuffer)) {
                        return false;
                    }
                    octetBuffer.clear();
                    state = 0;
                }
                else if (DIGIT.Contains(c)) {
                    octetBuffer.push_back(c);
                }
                else {
                    return false;
                }
                break;
            }
            }
        }
        if (!octetBuffer.empty()) {
            ++numGroups;
            if (!ValidateOctet(octetBuffer)) {
                return false;
            }
        }
        return (numGroups == 4);
    }

    /**
     * This function checks to make sure the given address
     * is a valid IPv6 address according to the rules in
     * RFC 3986 (https://tools.ietf.org/html/rfc3986).
     *
     * @param[in] address
     *     This is the IPv6 address to validate.
     *
     * @return
     *     An indication of whether or not the given address
     *     is a valid IPv6 address is returned.
     */
    bool ValidateIpv6Address(const std::string&address) {
        enum class ValidationState {
            NO_GROUPS_YET,
            COLON_BUT_NO_GROUPS_YET,
            AFTER_COLON_EXPECT_GROUP_OR_IPV4,
            IN_GROUP_NOT_IPV4,
            IN_GROUP_COULD_BE_IPV4,
            COLON_AFTER_GROUP,
        } state = ValidationState::NO_GROUPS_YET;

        size_t numGroups = 0;
        size_t numDigits = 0;
        bool doubleColonEncountered = false;
        size_t potentialIpv4AddressStart = 0;
        size_t position = 0;
        bool ipv4AddressEncountered = false;

        for (auto c : address) {
            switch (state) {
            case ValidationState::NO_GROUPS_YET: {
                // not in a group yet
                if (c == ':') {
                    state = ValidationState::COLON_BUT_NO_GROUPS_YET;
                }
                else if (DIGIT.Contains(c)) {
                    potentialIpv4AddressStart = position;
                    numDigits = 1;
                    state = ValidationState::IN_GROUP_COULD_BE_IPV4;
                }
                else if (HEXDIG.Contains(c)) {
                    numDigits = 1;
                    state = ValidationState::IN_GROUP_NOT_IPV4;
                }
                else {
                    return false;
                }
                break;
            }
            case ValidationState::COLON_BUT_NO_GROUPS_YET: { // not in a group yet, encountered one colon
                if (c == ':') {
                    if (doubleColonEncountered) {
                        return false;
                    }
                    else {
                        doubleColonEncountered = true;
                        state = ValidationState::AFTER_COLON_EXPECT_GROUP_OR_IPV4;
                    }
                }
                break;
            }
            case ValidationState::AFTER_COLON_EXPECT_GROUP_OR_IPV4: { // expect a hex digit
                if (DIGIT.Contains(c)) {
                    potentialIpv4AddressStart = position;
                    if (++numDigits > 4) {
                        return false;
                    }
                    state = ValidationState::IN_GROUP_COULD_BE_IPV4;
                }
                else if (HEXDIG.Contains(c)) {
                    if (++numDigits > 4) {
                        return false;
                    }
                    state = ValidationState::IN_GROUP_NOT_IPV4;
                }
                else {
                    return false;
                }
                break;
            }
            case ValidationState::IN_GROUP_NOT_IPV4: { // expect either a hex digit or colon
                if (c == ':') {
                    numDigits = 0;
                    ++numGroups;
                    state = ValidationState::COLON_AFTER_GROUP;
                }
                else if (HEXDIG.Contains(c)) {
                    if (++numDigits > 4) {
                        return false;
                    }
                }
                else {
                    return false;
                }
                break;
            }
            case ValidationState::IN_GROUP_COULD_BE_IPV4: { // expect either a hex digit, dot, or colon
                if (c == ':') {
                    numDigits = 0;
                    ++numGroups;
                    state = ValidationState::AFTER_COLON_EXPECT_GROUP_OR_IPV4;
                }
                else if (c == '.') {
                    ipv4AddressEncountered = true;
                }
                else if (DIGIT.Contains(c)) {
                    if (++numDigits > 4) {
                        return false;
                    }
                }
                else if (HEXDIG.Contains(c)) {
                    if (++numDigits > 4) {
                        return false;
                    }
                    state = ValidationState::IN_GROUP_NOT_IPV4;
                }
                else {
                    return false;
                }
                break;
            }
            case ValidationState::COLON_AFTER_GROUP: { // we are in a group, encountered one colon
                if (c == ':') {
                    if (doubleColonEncountered) {
                        return false;
                    }
                    else {
                        doubleColonEncountered = true;
                        state = ValidationState::AFTER_COLON_EXPECT_GROUP_OR_IPV4;
                    }
                }
                else if (DIGIT.Contains(c)) {
                    potentialIpv4AddressStart = position;
                    if (++numDigits > 4) {
                        return false;
                    }
                    state = ValidationState::IN_GROUP_COULD_BE_IPV4;
                }
                else if (HEXDIG.Contains(c)) {
                    if (++numDigits > 4) {
                        return false;
                    }
                    state = ValidationState::IN_GROUP_NOT_IPV4;
                }
                else {
                    return false;
                }
                break;
            }

            }
            if (ipv4AddressEncountered) {
                break;
            }
            ++position;
        }

        if (
            (state == ValidationState::IN_GROUP_NOT_IPV4)
            || (state == ValidationState::IN_GROUP_COULD_BE_IPV4)
        ){
            ++numGroups;
        }

        if (
            (position == address.length())
            && (
                (state == ValidationState::COLON_BUT_NO_GROUPS_YET)
                || (state == ValidationState::AFTER_COLON_EXPECT_GROUP_OR_IPV4)
                || (state == ValidationState::COLON_AFTER_GROUP)
                )
            ) {
            // trailing single colon
            return false;
        }

        if (ipv4AddressEncountered) {
            if (!ValidateIpv4Adress(address.substr(potentialIpv4AddressStart))) {
                return false;
            }
            numGroups += 2;
        }

        if (doubleColonEncountered) {
            return (numGroups <= 7);
        }
        else {
            return (numGroups == 8);
        }
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
        bool hasPort = false;
        uint16_t port = 0;
        std::vector<std::string> path;
        bool hasQuery = false;
        std::string query;
        bool hasFragment = false;
        std::string fragment;

        // Methods

        /**
         * This method returns an indication of whether or not
         * the URI includes any element that is part of the authority of the URI.
         *
         * return
         *      An indication of whether or not the URI includes
         *      any element that is part of the authority of the URI is returned.
         */
        bool HasAuthority() const {
            return (
                !host.empty()
                || !userInfo.empty()
                || hasPort
                );
        }

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
            /**
             * These are the various states for the state machine implemented
             * below to correctly split up and validate the URI substring
             * containing the host and potentially a port number as well.
             */
            enum class HostParsingState {
                FIRST_CHARACTER,
                NOT_IP_LITERAL,
                PERCENT_ENCODED_CHARACTER,
                IP_LITERAL,
                IPV6_ADDRESS,
                IPV_FUTURE_NUMBER,
                IPV_FUTURE_BODY,
                GARBAGE_CHECK,
                PORT,
            };

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

            HostParsingState hostParsingState = HostParsingState::FIRST_CHARACTER;
            int decodedCharacter = 0;
            host.clear();
            PercentEncodedCharacterDecoder pecDecoder;
            bool hostIsRegName = false;
            for (const auto c : hostPortString) {
                switch (hostParsingState) {
                case HostParsingState::FIRST_CHARACTER: { // first character
                    if (c == '[') {
                        hostParsingState = HostParsingState::IP_LITERAL;
                        break;
                    }
                    else {
                        hostParsingState = HostParsingState::NOT_IP_LITERAL;
                        hostIsRegName = true;
                    }
                }
                case HostParsingState::NOT_IP_LITERAL: { // reg-name or IPv4Address
                    if (c == '%') {
                        pecDecoder = PercentEncodedCharacterDecoder();
                        hostParsingState = HostParsingState::PERCENT_ENCODED_CHARACTER;
                    }
                    else if (c == ':') {
                        hostParsingState = HostParsingState::PORT;
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
                case HostParsingState::PERCENT_ENCODED_CHARACTER: {
                    if (!pecDecoder.NextEncodedCharacter(c)) {
                        return false;
                    }
                    if (pecDecoder.Done()) {
                        hostParsingState = HostParsingState::FIRST_CHARACTER;
                        host.push_back((char)pecDecoder.GetDecodedCharacter());
                    }
                    break;
                }
                case HostParsingState::IP_LITERAL: { // IP-literal
                    if (c == 'v') {
                        host.push_back(c);
                        hostParsingState = HostParsingState::IPV_FUTURE_NUMBER;
                        break;
                    }
                    else {
                        hostParsingState = HostParsingState::IPV6_ADDRESS;
                    }
                }
                case HostParsingState::IPV6_ADDRESS: { // IPv6address
                    if (c == ']') {
                        if (!ValidateIpv6Address(host)) {
                            return false;
                        }
                        hostParsingState = HostParsingState::GARBAGE_CHECK;
                    }
                    else {
                        host.push_back(c);
                    }
                    break;
                }
                case HostParsingState::IPV_FUTURE_NUMBER: { // IPvFuture: v ...
                    if (c == '.') {
                        hostParsingState = HostParsingState::IPV_FUTURE_BODY;
                    }
                    else if (!HEXDIG.Contains(c)) {
                        return false;
                    }
                    host.push_back(c);
                    break;
                }
                case HostParsingState::IPV_FUTURE_BODY: { // IPvFuture
                    if (c == ']') {
                        hostParsingState = HostParsingState::GARBAGE_CHECK;
                    }
                    else if (!IPV_FUTURE_LAST_PART.Contains(c)) {
                        return false;
                    }
                    else {
                        host.push_back(c);
                    }
                    break;
                }
                case HostParsingState::GARBAGE_CHECK: { // illegal to have anything else, unless it's a colon,
                          // in which case it's a port delimiter
                    if (c == ':') {
                        hostParsingState = HostParsingState::PORT;
                    }
                    else {
                        return false;
                    }
                    break;
                }
                case HostParsingState::PORT: { // port
                    portString.push_back(c);
                    break;
                }
                }
            }

            if (
                (hostParsingState != HostParsingState::FIRST_CHARACTER)
                && (hostParsingState != HostParsingState::NOT_IP_LITERAL)
                && (hostParsingState != HostParsingState::GARBAGE_CHECK)
                && (hostParsingState != HostParsingState::PORT)
                ) {
                // truncated or ended early
                return false;
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
         * This method takes an unparsed URI string and separates out
         * the scheme (if any) and parses it, returning the remainder
         * of the unparsed URI string.
         *
         * @param[in] authorityAndPathString
         *     This is the the part of an unparsed URI consisting
         *     of the authority (if any) followed by the path.
         *
         * @param[out] pathString
         *     This is where to store the the path
         *     part of the input string.
         *
         * @return
         *     An indication of whether or not the given input string
         *     was successfully parsed is returned.
         */
        bool ParseScheme(const std::string& uriString, std::string& rest) {
            // First parse the scheme.	
            // Limit our search so we don't scan into the authority	
            // or path elements, because these may have the colon	
            // character as well, which we might misinterpret	
            // as the scheme delimiter.	
            auto authorityOrPathDelimiterStart = uriString.find('/');
            if (authorityOrPathDelimiterStart == std::string::npos) {
                authorityOrPathDelimiterStart = uriString.length();
            }
            const auto schemeEnd = uriString.substr(0, authorityOrPathDelimiterStart).find(':');

            if (schemeEnd == std::string::npos) {
                    scheme.clear();
                    rest = uriString;
            }
            else {
                scheme = uriString.substr(0, schemeEnd);
                bool isFirstCharacter = true;
                if (
                    FailsMatch(
                        scheme,
                        LegalSchemeCheckStrategy()
                    )
                    ) {
                    return false;
                }
                scheme = NormalizeCaseInsensitiveString(scheme);
                rest = uriString.substr(schemeEnd + 1);
            }
            return true;
        }

        /**
         * This method takes the part of an unparsed URI consisting
         * of the authority (if any) followed by the path, and divides
         * it into the authority and path parts, storing any authority
         * information in the internal state, and returning the path
         * part of the input string.
         *
         * @param[in] authorityAndPathString
         *     This is the the part of an unparsed URI consisting
         *     of the authority (if any) followed by the path.
         *
         * @param[out] pathString
         *     This is where to store the the path
         *     part of the input string.
         *
         * @return
         *     An indication of whether or not the given input string
         *     was successfully parsed is returned.
         */
        bool SplitAuthorityFromPathAndParseIt(
            std::string authorityAndPathString,
            std::string& pathString
        ) {
            port = 0;
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
                if (!ParseAuthority(authorityString)) {
                    return false;
                }
            }
            else {
                hasPort = false;
                userInfo.clear();
                host.clear();

                pathString = authorityAndPathString;
            }
            return true;
        }

        /**
         * This method handles the special case of the URI having an
         * authority but having an empty path.  In this case it sets
         * the path as "/".
         */
        void SetDefaultPathIfAuthorityPresentAndPathEmpty() {
            if (
                !host.empty()
                && path.empty()
                ) {
                path.push_back("");
            }
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
                hasFragment = false;
                fragment.clear();
                rest = queryAndOrFragment;
            }
            else {
                hasFragment = true;
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
            hasQuery = !queryWithDelimiter.empty();
            if (hasQuery) {
                query = queryWithDelimiter.substr(1);
            }
            else {
                query.clear();
            }

            return DecodeQueryOrFragment(query);
        }

        /**
         * This method replaces the URI's scheme with that of
         * another URI.
         *
         * @param[in] other
         *     This is the other URI from which to copy the scheme.
         */
        void CopyScheme(const Uri& other) {
            scheme = other.impl_->scheme;
        }

        /**
         * This method replaces the URI's authority with that of
         * another URI.
         *
         * @param[in] other
         *     This is the other URI from which to copy the authority.
         */
        void CopyAuthority(const Uri& other) {
            host = other.impl_->host;
            userInfo = other.impl_->userInfo;
            hasPort = other.impl_->hasPort;
            port = other.impl_->port;
        }

        /**
         * This method replaces the URI's path with that of
         * another URI.
         *
         * @param[in] other
         *     This is the other URI from which to copy the path.
         */
        void CopyPath(const Uri& other) {
            path = other.impl_->path;
        }

        /**
         * This method replaces the URI's path with that of
         * the normalized form of another URI.
         *
         * @param[in] other
         *     This is the other URI from which to copy
         *     the normalized path.
         */
        void CopyAndNormalizePath(const Uri& other) {
            CopyPath(other);
            NormalizePath();
        }

        /**
         * This method replaces the URI's query with that of
         * another URI.
         *
         * @param[in] other
         *     This is the other URI from which to copy the query.
         */
        void CopyQuery(const Uri& other) {
            query = other.impl_->query;
        }

        /**
         * This method replaces the URI's fragment with that of
         * another URI.
         *
         * @param[in] other
         *     This is the other URI from which to copy the query.
         */
        void CopyFragment(const Uri& other) {
            fragment = other.impl_->fragment;
        }

        /**
         * This method returns an indication of whether or not the
         * path of the URI is an absolute path, meaning it begins
         * with a forward slash ('/') character.
         *
         * @return
         *     An indication of whether or not the path of the URI
         *     is an absolute path, meaning it begins
         *     with a forward slash ('/') character is returned.
         */
        bool IsPathAbsolute() const {
            return (
                !path.empty()
                && (path[0] == "")
                );
        }

        /**
         * This method determines whether or not it makes sense to
         * navigate one level up from the current path
         * (in other words, does appending ".." to the path
         * actually change the path?)
         *
         * @return
         *     An indication of whether or not it makes sense to
         *     navigate one level up from the current path is returned.
         */
        bool CanNavigatePathUpOneLevel() const {
            return (
                !IsPathAbsolute()
                || (path.size() > 1)
                );
        }

        /**
         * This method applies the "remove_dot_segments" routine talked about
         * in RFC 3986 (https://tools.ietf.org/html/rfc3986) to the path segments
         * of the URI, in order to normalize the path
         * (apply and remove "." and ".." segments).
         */
        void NormalizePath() {
            // Rebuild the path one segment
            // at a time, removing and applying special
            // navigation segments ("." and "..") as we go.
            auto oldPath = std::move(path);
            path.clear();

            bool atDirectoryLevel = false;
            for (const auto segment : oldPath) {
                if (segment == ".") {
                    atDirectoryLevel = true;
                }
                else if (segment == "..") {
                    // Remove last path element
                    // if we can navigate up a level.
                    if (!path.empty()) {
                        if (CanNavigatePathUpOneLevel()) {
                            path.pop_back();
                        }
                    }
                    atDirectoryLevel = true;
                }
                else {
                    // Non-relative elements can just
                    // transfer over fine.  An empty
                    // segment marks a transition to
                    // a directory level context.  If we're
                    // already in that context, we
                    // want to ignore the transition.
                    if (
                        !atDirectoryLevel
                        || !segment.empty()
                        ) {
                        path.push_back(segment);
                    }
                    atDirectoryLevel = segment.empty();
                }

            }
            // If at the end of rebuilding the path,
            // we're in a directory level context,
            // add an empty segment to mark the fact.
            if (atDirectoryLevel && (
                !path.empty()
                && !path.back().empty()
                )) {
                path.push_back("");
            }
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
        std::string rest;
        if (!impl_->ParseScheme(uriString, rest)) {
            return false;
        }

        // next parse the authority
        const auto pathEnd = rest.find_first_of("?#");
        const auto authorityAndPathString = rest.substr(0, pathEnd);
        const auto queryAndOrFragment = rest.substr(authorityAndPathString.length());

        std::string pathString;
        if (!impl_->SplitAuthorityFromPathAndParseIt(authorityAndPathString, pathString)) {
            return false;
        }

        // next, parse the path
        if (!impl_->ParsePath(pathString)) {
            return false;
        }

        // Handle special case of absolute URI with  authority empty
        // path -- treat the same as "/" path.
        impl_->SetDefaultPathIfAuthorityPresentAndPathEmpty();

        // parse the fragment if there is one
        if (!impl_->ParseFragment(queryAndOrFragment, rest)) {
            return false;
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
        return !impl_->IsPathAbsolute();
    }

    bool Uri::HasFragment() const {
        return impl_->hasFragment;
    }

    std::string Uri::GetFragment() const
    {
        return impl_->fragment;
    }

    bool Uri::HasQuery() const {
        return impl_->hasQuery;
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
        impl_->NormalizePath();
    }


    /*
     * Resolve the reference by following the
     * algorithm from section 5.2.2 of
     * RFC 3986 (https://tools.ietf.org/html/rfc3986).
     */
    Uri Uri::Resolve(const Uri& relativeReference) const {
        // Resolve the reference by following the algorithm
        // from section 5.2.2 in
        // RFC 3986 (https://tools.ietf.org/html/rfc3986).
        Uri target;
        if (!relativeReference.impl_->scheme.empty()) {
            target.impl_->CopyScheme(relativeReference);
            target.impl_->CopyAuthority(relativeReference);
            target.impl_->CopyAndNormalizePath(relativeReference);
            target.impl_->CopyQuery(relativeReference);
        }
        else {
            if (!relativeReference.impl_->host.empty()) {
                target.impl_->CopyAuthority(relativeReference);
                target.impl_->CopyAndNormalizePath(relativeReference);
                target.impl_->CopyQuery(relativeReference);
            }
            else {
                if (relativeReference.impl_->path.empty()) {
                    target.impl_->path = impl_->path;
                    if (!relativeReference.impl_->query.empty()) {
                        target.impl_->CopyQuery(relativeReference);
                    }
                    else {
                        target.impl_->CopyQuery(*this);
                    }
                }
                else {
                    // RFC describes this as:
                    // "if (R.path starts-with "/") then"
                    if (relativeReference.impl_->IsPathAbsolute()) {
                        target.impl_->CopyAndNormalizePath(relativeReference);
                    }
                    else {
                        // RFC describes this as:
                        // "T.path = merge(Base.path, R.path);"
                        target.impl_->CopyPath(*this);
                        if (target.impl_->path.size() > 1) {
                            target.impl_->path.pop_back();
                        }
                        std::copy(
                            relativeReference.impl_->path.begin(),
                            relativeReference.impl_->path.end(),
                            std::back_inserter(target.impl_->path)
                        );
                        target.NormalizePath();
                    }
                    target.impl_->CopyQuery(relativeReference);
                }
                target.impl_->CopyAuthority(*this);
            }
            target.impl_->CopyScheme(*this);
        }
        target.impl_->CopyFragment(relativeReference);
        return target;
    }

    void Uri::SetScheme(const std::string& scheme) {
        impl_->scheme = scheme;
    }

    void Uri::SetUserInfo(const std::string& userInfo) {
        impl_->userInfo = userInfo;
    }

    void Uri::SetHost(const std::string& host) {
        impl_->host = host;
    }

    void Uri::SetPort(uint16_t port) {
        impl_->port = port;
        impl_->hasPort = true;
    }

    void Uri::ClearPort() {
        impl_->hasPort = false;
    }

    void Uri::SetPath(const std::vector<std::string>& path) {
        impl_->path = path;
    }

    void Uri::SetQuery(const std::string& query) {
        impl_->query = query;
        impl_->hasQuery = true;
    }

    void Uri::ClearQuery() {
        impl_->hasQuery = false;
    }

    void Uri::SetFragment(const std::string& fragment) {
        impl_->fragment = fragment;
        impl_->hasFragment = true;
    }

    void Uri::ClearFragment() {
        impl_->hasFragment = false;
    }

    std::string Uri::GenerateString() const {
        std::ostringstream buffer;

        if (!impl_->scheme.empty()) {
            buffer << impl_->scheme << ':';
        }

        if (impl_->HasAuthority()) {
            buffer << "//";

            if (!impl_->userInfo.empty()) {
                buffer << impl_->userInfo << '@';
            }

            if (!impl_->host.empty()) {
                if (ValidateIpv6Address(impl_->host)) {
                    buffer << '[' << impl_->host << ']';
                }
                else {
                    buffer << impl_->host;
                }
            }

            if (impl_->hasPort) {
                buffer << ':' << impl_->port;
            }
        }

        // Special case: absolute but otherwise empty path.
        if (
            impl_->IsPathAbsolute()
            && (impl_->path.size() == 1)
            ) {
            buffer << '/';
        }

        size_t i = 0;
        for (const auto& segment : impl_->path) {
            buffer << segment;
            if (i + 1 < impl_->path.size()) {
                buffer << '/';
            }
            ++i;
        }

        if (impl_->hasQuery) {
            buffer << '?' << impl_->query;
        }

        if (impl_->hasFragment) {
            buffer << '#' << impl_->fragment;
        }

        return buffer.str();
    }

}
