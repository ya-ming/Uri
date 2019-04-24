/**
 * @file Uri.cpp
 *
 * This module contains the implementation of the Uri::Uri class.
 *
 * © 2019 by YaMing Wu
 */

#include <functional>
#include <memory>
#include <Uri/Uri.hpp>
#include <inttypes.h>

namespace {
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

    bool IsCharacterInSet(
        char c,
        std::initializer_list< char > characterSet
        ) {
        for (auto charInSet = characterSet.begin();
            charInSet != characterSet.end();
            ++charInSet) {
            const auto first = *charInSet++;
            const auto last = *charInSet;
            if ((c >= first) && (c <= last))
                return true;
        }
        return false;
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
                    check = IsCharacterInSet(c, { 'a', 'z', 'A', 'Z' });
                }
                else {
                    check = IsCharacterInSet(c, { 'a', 'z', 'A', 'Z', '0', '9', '+', '+', '-', '-', '.', '.' });
                }
                *isFirstCharacter = false;
                return check;
            }
        };
    }
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
                const auto userInfoEncoded = authorityString.substr(0, userInfoDelimiter);
                size_t decoderState = 0;
                int decodedCharacter = 0;
                for (const auto c : userInfoEncoded) {
                    switch (decoderState) {
                        case 0: {
                            if (c == '%') {
                                decoderState = 1;
                            }
                            else {
                                if (IsCharacterInSet(c, { 'a', 'z', 'A', 'Z', '0', '9',
                                    '-', '-', '.', '.', '_', '_', '~', '~', ':', ':',
                                    '!', '!', '$', '$', '&', '&', '\'', '\'',
                                    '(', '(', ')', ')', '*', '*', '+', '+', ',', ',',
                                    ';', ';', '=', '=',
                                    })) {
                                    userInfo.push_back(c);
                                }
                                else {
                                    return false;
                                }
                                    
                            }
                            break;
                        }
                        case 1: {
                            if (IsCharacterInSet(c, { '0', '9' })) {
                                decodedCharacter = (int)(c - '0');
                                decoderState = 2;
                            }
                            else if (IsCharacterInSet(c, { 'A', 'F' })) {
                                decodedCharacter = (int)(c - 'A') + 10;
                                decoderState = 2;
                            }
                            else {
                                return false;
                            }
                            break;
                        }
                        case 2: {
                            decoderState = 0;
                            decodedCharacter <<= 4;
                            if (IsCharacterInSet(c, { '0', '9' })) {
                                decodedCharacter += (int)(c - '0');
                            }
                            else if (IsCharacterInSet(c, { 'A', 'F' })) {
                                decodedCharacter += (int)(c - 'A') + 10;
                            }
                            else {
                                return false;
                            }
                            userInfo.push_back((char)decodedCharacter);
                            break;
                        }
                    }
                }
                hostPortString = authorityString.substr(userInfoDelimiter + 1);
            }

            // paring host and port from authority
            const auto portDelimiter = hostPortString.find(":");
            if (portDelimiter == std::string::npos) {
                host = hostPortString;
                hasPort = false;
            }
            else {
                host = hostPortString.substr(0, portDelimiter);

                const auto portString = hostPortString.substr(portDelimiter + 1);

                if (!ParseUint16(portString, port)) {
                    return false;
                }
                hasPort = true;
            }
            return true;
        }
    };

    Uri::~Uri() = default;

    Uri::Uri()
        : impl_(new Impl)
    {
    }

    bool Uri::ParseFromString(const std::string & uriString)
    {
        // first parse the scheme
        const auto schemeEnd = uriString.find(':');
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
        const auto fragmentDelimiter = queryAndOrFragment.find('#');
        if (fragmentDelimiter == std::string::npos) {
            impl_->fragment.clear();
            rest = queryAndOrFragment;
        }
        else {
            impl_->fragment = queryAndOrFragment.substr(fragmentDelimiter + 1);
            rest = queryAndOrFragment.substr(0, fragmentDelimiter);
        }


        // parse the query if there is one
        if (!rest.empty()) {
            impl_->query = rest.substr(1);
        }
        else {
            impl_->query.clear();
        }

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
}
