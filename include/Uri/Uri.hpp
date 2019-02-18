#ifndef URI_HPP
#define URI_HPP

/**
 * @file Uri.hpp
 *
 * This module declares the Uri::Uri class.
 *
 * © 2019 by YaMing Wu
 */

#include <memory>
#include <string>
#include <vector>

namespace Uri {

    /**
     * This class represents a Uniform Resource Identifier (URI),
     * as defined in RFC 3986 (https://tools.ietf.org/html/rfc3986).
     */
    class Uri {
        // Lifecycle management
    public:
        ~Uri();
        Uri(const Uri&) = delete;
        Uri(Uri&&) = delete;
        Uri& operator=(const Uri&) = delete;
        Uri& operator=(Uri&&) = delete;

        // Public methods
    public:
        /**
         * This is the default constructor.
         */
        Uri();


        /**
         * This method builds the URI from the elements parsed from
         * the given string rendering of a URI
         *
         * @param[in] uriString
         *
         * @return
         *      An indication of whether or not the URI was parsed successfully
         */
        bool ParseFromString(const std::string& uriString);


        /**
         * This method returns the "scheme" element of the URI
         *
         * @return
         *      The "scheme" element of the URI is returned
         *
         * @retval ""
         *      Return "" if no "scheme" element in the URI
         */
        std::string GetScheme() const;

        /**
         * This method returns the "userinfo" element of the URI
         *
         * @return
         *      The "userinfo" element of the URI is returned
         *
         * @retval ""
         *      Return "" if no "userinfo" element in the URI
         */
        std::string GetUserInfo() const;

        /**
         * This method returns the "host" element of the URI
         *
         * @return
         *      The "host" element of the URI is returned
         *
         * @retval ""
         *      Return "" if no "host" element in the URI
         */
        std::string GetHost() const;

        /**
         * This method returns the "path" element of the URI
         * as a sequence of segements
         *
         * @note
         *      If the first step of the path is an empty string
         *      then the path is absolute
         *
         * @return
         *      The "path" element of the URI is returned
         *
         */
        std::vector<std::string> GetPath() const;

        /**
         * This method returns whether or not the URI includes a port number
         *
         * @return
         *      An indication of whether or not the URI includes a port number is returned
         */
        bool HasPort() const;

        /**
         * This method returns port number of the URI if the URI has port number
         *
         * @return
         *      port number if the URI has port number
         */
        uint16_t GetPort() const;

        /**
         * This method returns whether or not the URI is a relative reference
         *
         * @return
         *      An indication of whether or not the URI is a relative reference is returned
         */
        bool IsRelativeReference() const;

        /**
         * This method returns whether or not the URI constains relative path
         *
         * @return
         *      An indication of whether or not the URI constains relative path is returned
         */
        bool ContainsRelativePath() const;

        /**
         * This method returns the "fragment" element of the URI
         *
         * @return
         *      The "fragment" element of the URI is returned
         *
         * @retval ""
         *      Return "" if no "fragment" element in the URI
         */
        std::string GetFragment() const;

        /**
         * This method returns the "query" element of the URI
         *
         * @return
         *      The "query" element of the URI is returned
         *
         * @retval ""
         *      Return "" if no "query" element in the URI
         */
        std::string GetQuery() const;

        // Private properties
    private:
        /**
         * This is the type of structure that contains the private
         * properties of the instance.  It is defined in the implementation
         * and declared here to ensure that it is scoped inside the class.
         */
        struct Impl;

        /**
         * This contains the private properties of the instance.
         */
        std::unique_ptr< struct Impl > impl_;
    };

}

#endif /* URI_HPP */
