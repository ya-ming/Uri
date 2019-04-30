/**
 * @file PercentEncodedCharacterDecoder.cpp
 *
 * This module declares the Uri::PercentEncodedCharacterDecoder class.
 *
 * © 2019 by YaMing Wu
 */

#include "IsCharacterInSet.hpp"

#include <set>

namespace Uri {
    /** 
     * This contains the private properties of the CharacterSet class.
     */
    struct CharacterSet::Impl {
        /** 
         * This holds the characters in the set.
         */
        std::set< char > charactersInSet;
    };

    CharacterSet::~CharacterSet() = default;

    CharacterSet::CharacterSet(const CharacterSet& other)
        :impl_(new Impl(*other.impl_))
    {
    }

    CharacterSet::CharacterSet(CharacterSet&& other) = default;

    CharacterSet& CharacterSet::operator=(const CharacterSet& other) {
        if (this != &other) {
            *impl_ = *other.impl_;
        }
        return *this;
    }

    CharacterSet& CharacterSet::operator= (CharacterSet&& other) = default;

    CharacterSet::CharacterSet()
        : impl_(new Impl) {
    }

    CharacterSet::CharacterSet(char c) 
        : impl_(new Impl) {
        (void)impl_->charactersInSet.insert(c);
    }

    CharacterSet::CharacterSet (char first, char last) 
        : impl_(new Impl) {
        for (char c = first; c <= last; ++c) {
            (void)impl_->charactersInSet.insert(c);
        }
    }

    CharacterSet::CharacterSet (
        std::initializer_list< CharacterSet > characterSets
    ) : impl_(new Impl) {
        for (auto characterSet = characterSets.begin();
            characterSet != characterSets.end();
            ++characterSet) {
            impl_->charactersInSet.insert(
                characterSet->impl_->charactersInSet.begin(),
                characterSet->impl_->charactersInSet.end()
            );
        }
    }

    bool CharacterSet::Contains(char c) const {
        return impl_->charactersInSet.find(c) != impl_->charactersInSet.end();
    }

    bool IsCharacterInSet(
        char c,
        const CharacterSet& characterSet
    ) {
        return characterSet.Contains(c);
    }
}
