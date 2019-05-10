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
        Uri(Uri&&);
        Uri& operator=(const Uri&) = delete;
        Uri& operator=(Uri&&);

        // Public methods
    public:
        /**
         * This is the default constructor.
         */
        Uri();

        /**
         * This is the equality comparison operator for the class.
         *
         * @param[in] other
         *     This is the other URI to which to compare this URI.
         *
         * @return
         *     An indication of whether or not the two URIs are
         *     equal is returned.
         */
        bool operator==(const Uri& other) const;

        /**
         * This is the inequality comparison operator for the class.
         *
         * @param[in] other
         *     This is the other URI to which to compare this URI.
         *
         * @return
         *     An indication of whether or not the two URIs are
         *     not equal is returned.
         */
        bool operator!=(const Uri& other) const;

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

        // Overwrite the << operator to print out the detail of the Uri class
        friend std::ostream& operator<<(std::ostream &strm, const Uri &uri);


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
         * as a sequence of segments
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
         * This method returns an indication of whether or not the
         * URI includes a fragment.
         *
         * @return
         *     An indication of whether or not the
         *     URI includes a fragment is returned.
         */
        bool HasFragment() const;

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
         * This method returns an indication of whether or not the
         * URI includes a query.
         *
         * @return
         *     An indication of whether or not the
         *     URI includes a query is returned.
         */
        bool HasQuery() const;

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

        /**
         * This method applies the "remove_dot_segments" routine talked about
         * in RFC 3986 (https://tools.ietf.org/html/rfc3986) to the path segments
         * of the URI, in order to normalize the path
         * (apply and remove "." and ".." segments).
         */
        void NormalizePath();

        /**
         * This method resolves the given relative reference, based on the given
         * base URI, returning the resolved target URI.
         *
         * @param[in] relativeReference
         *      this describes how to get to the target starting at the base.
         *
         * @return
         *      The resolved target URI is returned.
         */
        Uri Resolve(const Uri& relativeReference) const;

        /**
         * This method sets the scheme element of the URI.
         *
         * @param[in] scheme
         *      This is the scheme to set for the URI.
         */
        void SetScheme(const std::string& scheme);

        /**
         * This method sets the userinfo element of the URI.
         *
         * @param[in] userinfo
         *      This is the userinfo to set for the URI.
         */
        void SetUserInfo(const std::string& userinfo);

        /**
         * This method sets the host element of the URI.
         *
         * @param[in] host
         *      This is the host to set for the URI.
         */
        void SetHost(const std::string& host);

        /**
         * This method sets the port element of the URI.
         *
         * @param[in] port
         *      This is the port to set for the URI.
         */
        void SetPort(uint16_t port);

        /**
         * This method clears the port element of the URI.
         */
        void ClearPort();

        /**
         * This method sets the path element of the URI.
         *
         * @param[in] path
         *      This is the path to set for the URI.
         */
        void SetPath(const std::vector<std::string>& path);

        /**
         * This method sets the query element of the URI.
         *
         * @param[in] query
         *      This is the query to set for the URI.
         */
        void SetQuery(const std::string& query);

        /**
         * This method removes the query element from the URI.
         */
        void ClearQuery();

        /**
         * This method sets the fragment element of the URI.
         *
         * @param[in] fragment
         *      This is the fragment to set for the URI.
         */
        void SetFragment(const std::string& fragment);

        /**
         * This method removes the fragment element from the URI.
         */
        void ClearFragment();

        /**
         * This method constructs and returns the string
         * rendering of the URI, according to the rules in
         * RFC 3986 (https://tools.ietf.org/html/rfc3986).
         *
         * @return
         *      The string rendering of the URI is returned.
         */
        std::string GenerateString() const;

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
