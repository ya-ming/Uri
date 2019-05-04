#ifndef URI_NORMALIZE_CASE_INSENSITIVE_STRING
#define URI_NORMALIZE_CASE_INSENSITIVE_STRING

/**
 * @file NormalizeCaseInsensitiveString.hpp
 *
 * This module declares the Uri::NormalizeCaseInsensitiveString class.
 *
 * © 2019 by YaMing Wu
 */

#include <string>

namespace Uri {

    /**
     * This function takes a string and swaps all upper-case characters
     * with their lower-case equivalents, returning the result.
     *
     * @param[in] inString
     *      This is the string to be normalized.
     *
     * @return
     *      The normalized string is returned.
     */
    std::string NormalizeCaseInsensitiveString(const std::string& inString);
}

#endif /* URI_NORMALIZE_CASE_INSENSITIVE_STRING */
