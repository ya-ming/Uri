/**
 * @file Uri.cpp
 *
 * This module contains the implementation of the Uri::Uri class.
 *
 * © 2019 by YaMing Wu
 */

#include <Uri/Uri.hpp>
#include <inttypes.h>

namespace Uri {
    /**
     * This contains the private properties of a Uri instance.
     */
    struct Uri::Impl {
        std::string scheme;
        std::string userInfo;
        std::string host;
        std::vector<std::string> path;
        bool hasPort = false;
        uint16_t port = 0;
        std::string query;
        std::string fragment;
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
            rest = uriString.substr(schemeEnd + 1);
        }

        // next parse the authority
        impl_->hasPort = false;
        const auto pathEnd = rest.find_first_of("?#");
        auto authorityAndPathString = rest.substr(0, pathEnd);
        const auto queryAndOrFragment = rest.substr(authorityAndPathString.length());
        std::string hostPortAndPathString;


        impl_->port = 0;
        if (authorityAndPathString.substr(0, 2) == "//") {
            // strip off authority marker
            authorityAndPathString = authorityAndPathString.substr(2);

            // first separate the authority from the path
            auto authorityEnd = authorityAndPathString.find("/");
            if (authorityEnd == std::string::npos) {
                authorityEnd = authorityAndPathString.length();
            }

            // check if there is a UserInfo, and if so, extract it
            const auto userInfoDelimiter = authorityAndPathString.find('@');
            if (userInfoDelimiter == std::string::npos) {
                impl_->userInfo.clear();
                hostPortAndPathString = authorityAndPathString;
            }
            else {

                impl_->userInfo = authorityAndPathString.substr(0, userInfoDelimiter);
                hostPortAndPathString = authorityAndPathString.substr(userInfoDelimiter + 1);
            }

            // paring host and port from authority
            const auto portDelimiter = hostPortAndPathString.find(":");
            if (portDelimiter == std::string::npos) {
                impl_->host = hostPortAndPathString.substr(0, authorityEnd);
            }
            else {
                impl_->host = hostPortAndPathString.substr(0, portDelimiter);

                uint32_t newPort = 0;
                for (auto c : hostPortAndPathString.substr(portDelimiter + 1, authorityEnd - portDelimiter - 1)) {
                    if ((c < '0') || (c > '9')) {
                        return false;
                    }

                    newPort *= 10;
                    newPort += (uint16_t)(c - '0');
                    // 1 << 16 == 65536         == 000000000001000000000000
                    // (1 << 16) - 1 == 65535   == 000000000000111111111111
                    // ~((1 << 16) - 1)         == 111111111111000000000000
                    if ((newPort & ~((1 << 16) - 1)) != 0) {
                        return false;
                    }
                }

                impl_->port = newPort;
                impl_->hasPort = true;
            }

            hostPortAndPathString = authorityAndPathString.substr(authorityEnd);
        }
        else {
            impl_->host.clear();
            hostPortAndPathString = authorityAndPathString;
        }

        auto pathString = hostPortAndPathString;

        // next, parse the path
        impl_->path.clear();

        if (pathString == "/") {
            // special case of a path that is empty but needs a single
            // empty-string element to indicate that it is absolute
            impl_->path.push_back("");
            pathString.clear();
        }
        else if (!pathString.empty()) {
            for (;;) {
                auto pathDelimiter = pathString.find("/");

                if (pathDelimiter == std::string::npos) {
                    impl_->path.push_back(pathString);
                    pathString.clear();
                    break;
                }
                else {
                    impl_->path.emplace_back(
                        pathString.begin(),
                        pathString.begin() + pathDelimiter
                    );
                    pathString = pathString.substr(pathDelimiter + 1);

                }
            }
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
