//
//  refract/data/Enum.h
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_DATA_ENUM_H
#define REFRACT_DATA_ENUM_H

#include <vector>
#include <memory>

#include "../ElementIfc.h"
#include "Traits.h"

namespace refract
{
    namespace data
    {
        ///
        /// Data structure definition (DSD) of a Refract Array Element
        ///
        /// @remark Defined by a sequence of Refract Elements
        ///
        class enum_t final : public container_traits<enum_t, std::vector<std::unique_ptr<IElement> > >
        {
            container_type elements_; //< sequence of Elements

        public:
            static const char* name; //< syntactical name of the DSD

        public:
            ///
            /// Initialize an empty Enum DSD
            ///
            enum_t() = default;

            ///
            /// Initialize an Enum DSD from its memory representation
            /// @deprecated
            ///
            /// @param data     memory represenation
            ///
            [[deprecated]] enum_t(container_type elements);

            ///
            /// Initialize an Enum DSD from another by consuming its memory representation
            ///
            /// @param other   Enum DSD to be consumed
            ///
            enum_t(enum_t&&) = default;

            ///
            /// Initialize an Enum DSD from another by cloning its children
            ///
            /// @param other   Enum DSD to be cloned from
            ///
            enum_t(const enum_t& other);

            ///
            /// Clear children and consume another Enum DSD's memory representation
            ///
            /// @param other   Enum DSD to be consumed
            ///
            enum_t& operator=(enum_t&&) = default;

            ///
            /// Clear children and clone them from another Enum DSD
            ///
            /// @param other   Enum DSD to be cloned from
            ///
            enum_t& operator=(const enum_t& rhs);

            ~enum_t() = default;

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
