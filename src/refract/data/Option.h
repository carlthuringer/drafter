//
//  refract/data/Option.h
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_DATA_OPTION_H
#define REFRACT_DATA_OPTION_H

#include <vector>
#include <memory>

#include "../ElementIfc.h"
#include "Traits.h"

namespace refract
{
    namespace data
    {
        ///
        /// Data structure definition (DSD) of a Refract Option Element
        ///
        /// @remark Defined by a set of Refract Elements
        ///
        class option_t final : public container_traits<option_t, std::vector<std::unique_ptr<IElement> > >
        {
            container_type elements_; //< set of Elements

        public:
            static const char* name; //< syntactical name of the DSD

        public:
            ///
            /// Initialize an empty Option DSD
            ///
            option_t() = default;

            ///
            /// Initialize an Option DSD from its memory representation
            /// @deprecated
            ///
            /// @param elements     memory represenation
            ///
            [[deprecated]] option_t(container_type elements);

            ///
            /// Initialize an Option DSD from another by consuming its memory representation
            ///
            /// @param other   Option DSD to be consumed
            ///
            option_t(option_t&& other) = default;

            ///
            /// Initialize an Option DSD from another by cloning its children
            ///
            /// @param other   Option DSD to be cloned from
            ///
            option_t(const option_t& other);

            ///
            /// Clear children and consume another Option DSD's memory representation
            ///
            /// @param other   Option DSD to be consumed
            ///
            option_t& operator=(option_t&& other) = default;

            ///
            /// Clear children and clone them from another Option DSD
            ///
            /// @param other   Option DSD to be cloned from
            ///
            option_t& operator=(const option_t& rhs);

            ~option_t() = default;

        public:
            ///
            /// Get reference to memory representation
            /// @deprecated
            ///
            [[deprecated]] const container_type& get() const noexcept
            {
                return elements_;
            }

        public: // iterators
            auto begin() noexcept
            {
                return elements_.begin();
            }
            auto end() noexcept
            {
                return elements_.end();
            }
            auto begin() const noexcept
            {
                return elements_.begin();
            }
            auto end() const noexcept
            {
                return elements_.end();
            }

        public:
            ///
            /// Add a child Element
            ///
            /// @param it   where the element is to be added
            /// @param el   Element to be consumed
            ///
            /// @return iterator to Element added
            ///
            iterator insert(const_iterator it, std::unique_ptr<IElement> el);

            ///
            /// Remove a subsequence of children
            ///
            /// @param b    iterator to the first Element to be removed
            /// @param e    iterator following the last Element to be removed
            ///
            /// @return iterator following the last child removed
            ///
            iterator erase(const_iterator b, const_iterator e);
        };
    }
}

#endif
