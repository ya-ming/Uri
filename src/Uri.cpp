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
        std::string host;
        std::vector<std::string> path;
        bool hasPort = false;
        uint16_t port = 0;
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

        // next parse the host
        impl_->hasPort = false;
        impl_->port = 0;
        if (rest.substr(0, 2) == "//") {
            auto authorityEnd = rest.find("/", 2);
            if (authorityEnd == std::string::npos) {
                authorityEnd = rest.length();
            }
            const auto portDelimiter = rest.find(":");
            if (portDelimiter == std::string::npos) {
                impl_->host = rest.substr(2, authorityEnd - 2);
            }
            else {
                impl_->host = rest.substr(2, portDelimiter - 2);

                uint32_t newPort = 0;
                for (auto c : rest.substr(portDelimiter + 1, authorityEnd - portDelimiter - 1)) {
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

            rest = rest.substr(authorityEnd);
        }
        else {
            impl_->host = "";
        }

        // finally parse the path
        impl_->path.clear();
        if (rest == "/") {
            // special case of a path that is empty but needs a single
            // empty-string element to indicate that it is absolute
            impl_->path.push_back("");
        }
        else if (!rest.empty()) {
            for (;;) {
                auto pathDelimiter = rest.find("/");

                if (pathDelimiter == std::string::npos) {
                    impl_->path.push_back(rest);
                    break;
                }
                else {
                    impl_->path.emplace_back(
                        rest.begin(),
                        rest.begin() + pathDelimiter
                    );
                    rest = rest.substr(pathDelimiter + 1);

                }
            }
        }

        return true;
    }

    std::string Uri::GetScheme() const
    {
        return impl_->scheme;
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

}
