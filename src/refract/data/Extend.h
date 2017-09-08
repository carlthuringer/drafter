//
//  refract/data/Extend.h
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_DATA_EXTEND_H
#define REFRACT_DATA_EXTEND_H

#include <memory>
#include <vector>

#include "ElementIfc.h"
#include "Traits.h"

namespace refract
{
    namespace data
    {
        ///
        /// Data structure definition (DSD) of a Refract Extend Element
        ///
        /// @remark Defined by a sequence of Refract Elements
        ///
        class extend_t final : public algorithm_traits<std::vector<std::unique_ptr<experimental::IElement> > >
        {
            container_type elements_;

        public:
            static const char* name; //< sequence of Elements

        public:
            ///
            /// Initialize an empty Extend DSD
            ///
            extend_t() = default;

            ///
            /// Initialize an Extend DSD from its memory representation
            /// @deprecated
            ///
            /// @param data     memory represenation
            ///
            [[deprecated]] extend_t(container_type elements);

            ///
            /// Initialize an Extend DSD from another by consuming its memory representation
            ///
            /// @param other   Extend DSD to be consumed
            ///
            extend_t(extend_t&&) = default;

            ///
            /// Initialize an Extend DSD from another by cloning its children
            ///
            /// @param other   Extend DSD to be cloned from
            ///
            extend_t(const extend_t& other);

            ///
            /// Clear children and consume another Extend DSD's memory representation
            ///
            /// @param other   Extend DSD to be consumed
            ///
            extend_t& operator=(extend_t&&) = default;

            ///
            /// Clear children and clone them from another Extend DSD
            ///
            /// @param other   Extend DSD to be cloned from
            ///
            extend_t& operator=(const extend_t& rhs)
            {
                extend_t a(rhs);
                std::swap(a, *this);
                return *this;
            }

            ~extend_t() = default;

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
            iterator insert(const_iterator it, std::unique_ptr<experimental::IElement> el);

            ///
            /// Remove a subsequence of children
            ///
            /// @param b    iterator to the first Element to be removed
            /// @param e    iterator following the last Element to be removed
            ///
            /// @return iterator following the last child removed
            ///
            iterator erase(const_iterator b, const_iterator e);

        public:
            std::unique_ptr<experimental::IElement> merge() const;
        };
    }
}

#endif
