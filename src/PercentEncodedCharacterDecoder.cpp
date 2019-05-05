/**
 * @file PercentEncodedCharacterDecoder.cpp
 *
 * This module declares the Uri::PercentEncodedCharacterDecoder class.
 *
 * © 2019 by YaMing Wu
 */

#include "CharacterSet.hpp"
#include "PercentEncodedCharacterDecoder.hpp"

namespace {
    /**
     * This is the character set containing just the DIGIT characters
     * from the ASCII character set.
     */
    const Uri::CharacterSet DIGIT('0', '9');
    /**
     * This is the character set containing just the upper case letters
     * 'A' through 'F', used in upper-case hexadecimal.
     */
    const Uri::CharacterSet HEX_UPPER('A', 'F');

    /**
     * This is the character set containing just the lower case letters
     * 'a' through 'f', used in lower-case hexadecimal.
     */
    const Uri::CharacterSet HEX_LOWER('a', 'f');
}

namespace Uri {
    struct PercentEncodedCharacterDecoder::Impl {
        /**
         * This is the decoded character
         */
        int decodedCharacter = 0;

        /**
         * This is the number of digits that we still need to shift in
         * to decode the character.
         * - 2: we haven't yet received the first hex digit.
         * - 1: we received the first hex digit but not the second.
         * - 0: we received both hex digits.
         */
        size_t digitsLeft = 2;

        /**
         * This method shifts in th egiven hex digit as part of
         * building the decoded character.
         *
         * @param[in] c
         *      This is the hex digit to shift into the decoded character.
         *
         * @return
         *      An indication of whether or not the given hex digit
         *      was valid is returned.
         */
        bool ShiftInHexDigit(char c) {
            decodedCharacter <<= 4;
            if (DIGIT.Has(c)) {
                decodedCharacter += (int)(c - '0');
            }
            else if (HEX_UPPER.Has(c)) {
                decodedCharacter += (int)(c - 'A') + 10;
            }
            else if (HEX_LOWER.Has(c)) {
                decodedCharacter += (int)(c - 'a') + 10;
            }
            else {
                return false;
            }
            return true;
        }
    };

    PercentEncodedCharacterDecoder::~PercentEncodedCharacterDecoder() = default;
    PercentEncodedCharacterDecoder::PercentEncodedCharacterDecoder(PercentEncodedCharacterDecoder&&) = default;
    PercentEncodedCharacterDecoder& PercentEncodedCharacterDecoder::operator=(PercentEncodedCharacterDecoder&&) = default;

    PercentEncodedCharacterDecoder::PercentEncodedCharacterDecoder()
        : impl_(new Impl())
    {

    }

    bool PercentEncodedCharacterDecoder::NextEncodedCharacter(char c) {
        if (!impl_->ShiftInHexDigit(c)) {
            return false;
        }
        --impl_->digitsLeft;
        return true;
    }

    /**
     * This method checks to see if the decoder is done
     * and has decoded the encoded character.
     *
     * @return
     *      An indication of wheter or not the decoder is done
     *      and has decoded the encoded character is returned.
     */
    bool PercentEncodedCharacterDecoder::Done() const {
        return (impl_->digitsLeft == 0);
    }

    /**
     * This method returns the decoded character, once the
     * decoder is done.
     *
     * @return
     *      The decoded character is returned.
     */
    char PercentEncodedCharacterDecoder::GetDecodedCharacter() const {
        return (char)impl_->decodedCharacter;
    }
}
