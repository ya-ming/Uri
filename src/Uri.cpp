/**
 * @file Uri.cpp
 *
 * This module contains the implementation of the Uri::Uri class.
 *
 * © 2019 by YaMing Wu
 */

#include <Uri/Uri.hpp>

namespace Uri {
    /**
     * This contains the private properties of a Uri instance.
     */
    struct Uri::Impl {
        std::string scheme;
        std::string host;
        std::vector<std::string> path;
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
        impl_->scheme = uriString.substr(0, schemeEnd);
        auto rest = uriString.substr(schemeEnd + 1);

        // next parse the host
        if (rest.substr(0, 2) == "//") {
            const auto authorityEnd = rest.find("/", 2);
            impl_->host = rest.substr(2, authorityEnd - 2);
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

}
