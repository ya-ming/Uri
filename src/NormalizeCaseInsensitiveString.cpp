/**
 * @file NormalizeCaseInsensitiveString.cpp
 *
 * This module declares the Uri::NormalizeCaseInsensitiveString class.
 *
 * 2019 by YaMing Wu
 */

#include "NormalizeCaseInsensitiveString.hpp"

namespace Uri {
    std::string NormalizeCaseInsensitiveString(const std::string& inString) {
        std::string outString;
        for (char c : inString) {
            outString.push_back(tolower(c));
        }
        return outString;
    }
}
