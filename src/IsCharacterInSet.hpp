#ifndef URI_IS_CHARACTER_IN_SET_HPP
#define URI_IS_CHARACTER_IN_SET_HPP

/**
 * @file PercentEncodedCharacterDecoder.hpp
 *
 * This module declares the Uri::PercentEncodedCharacterDecoder class.
 *
 * © 2019 by YaMing Wu
 */

#include <initializer_list>

namespace Uri {
    /**
     *  This function determines whether or not the given character
     *  is in the given character set.
     *
     *  @Param[in] c
     *      This is the character to check.
     *
     *  @Param[in] characterSet
     *      This is the set of characters that are allowed.
     *
     *  @return
     *      An indication of whether or not the given character
     *      is in the given character set is returned.
     */
    bool IsCharacterInSet(
        char c,
        std::initializer_list< char > characterSet
    );
}

#endif /* URI_IS_CHARACTER_IN_SET_HPP */
