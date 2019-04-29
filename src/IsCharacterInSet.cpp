/**
 * @file PercentEncodedCharacterDecoder.cpp
 *
 * This module declares the Uri::PercentEncodedCharacterDecoder class.
 *
 * © 2019 by YaMing Wu
 */

#include "IsCharacterInSet.hpp"

namespace Uri {
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
}
