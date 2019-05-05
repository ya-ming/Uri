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
         * This is the current state of the decoder's state machine
         * - 0: we haven't yet received the first hex digit.
         * - 1: we received the first hex digit but not the second.
         * - 2: we received both hex digits.
         */
        size_t decoderState = 0;
    };

    PercentEncodedCharacterDecoder::~PercentEncodedCharacterDecoder() = default;
    PercentEncodedCharacterDecoder::PercentEncodedCharacterDecoder(PercentEncodedCharacterDecoder&&) = default;
    PercentEncodedCharacterDecoder& PercentEncodedCharacterDecoder::operator=(PercentEncodedCharacterDecoder&&) = default;

    PercentEncodedCharacterDecoder::PercentEncodedCharacterDecoder()
        : impl_(new Impl())
    {

    }

    bool PercentEncodedCharacterDecoder::NextEncodedCharacter(char c) {
        switch (impl_->decoderState) {
        case 0: { // % ...
            impl_->decoderState = 1;

            if (DIGIT.Has(c)) {
                impl_->decodedCharacter = (int)(c - '0');
            }
            else if (HEX_UPPER.Has(c)) {
                impl_->decodedCharacter = (int)(c - 'A') + 10;
            }
            else if (HEX_LOWER.Has(c)) {
                impl_->decodedCharacter = (int)(c - 'a') + 10;
            }
            else {
                return false;
            }
            break;
        }
        case 1: { // %[0-9A-F]
            impl_->decoderState = 2;
            impl_->decodedCharacter <<= 4;
            if (DIGIT.Has(c)) {
                impl_->decodedCharacter += (int)(c - '0');
            }
            else if (HEX_UPPER.Has(c)) {
                impl_->decodedCharacter += (int)(c - 'A') + 10;
            }
            else if (HEX_LOWER.Has(c)) {
                impl_->decodedCharacter += (int)(c - 'a') + 10;
            }
            else {
                return false;
            }
            break;
        }
        default:
            break;
        }
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
        return (impl_->decoderState == 2);
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
