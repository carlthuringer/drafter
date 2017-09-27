//
//  refract/data/Object.h
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_DATA_OBJECT_H
#define REFRACT_DATA_OBJECT_H

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

#include "../ElementIfc.h"
#include "Traits.h"

namespace refract
{
    namespace data
    {
        ///
        /// Data structure definition (DSD) of a Refract Object Element
        ///
        /// @remark Defined by a sequence of Refract Elements
        ///
        class object_t final : public container_traits<object_t, std::vector<std::unique_ptr<IElement> > >
        {
            container_type elements_; //< sequence of Elements

        public:
            static const char* name; //< syntactical name of the DSD

        public:
            ///
            /// Initialize an empty Object DSD
            ///
            object_t() = default;

            ///
            /// Initialize an Object DSD from its memory representation
            /// @deprecated
            ///
            /// @param data     memory represenation
            ///
            [[deprecated]] object_t(container_type elements) noexcept;

            ///
            /// Initialize an Object DSD from a range of Elements
            ///
            /// @tparam It  range iterator type
            ///
            /// @param b    iterator to the first Element to be cloned
            /// @param e    iterator following the last Element to be cloned
            ///
            template <typename It>
            object_t(It b, It e) : elements_()
            {
                elements_.reserve(e - b);
                std::transform(b,
                    e,
                    std::back_inserter(elements_),
                    [](const IElement* el) -> IElement* {
                        assert(el);
                        return el->clone();
                    });
            }

            ///
            /// Initialize an Object DSD from another by consuming its memory representation
            ///
            /// @param other   Object DSD to be consumed
            ///
            object_t(object_t&& other) = default;

            ///
            /// Initialize an Object DSD from another by cloning its children
            ///
            /// @param other   Object DSD to be cloned from
            ///
            object_t(const object_t& other);

            ///
            /// Clear children and consume another Object DSD's memory representation
            ///
            /// @param rhs   Object DSD to be consumed
            ///
            object_t& operator=(object_t&& rhs) = default;

            ///
            /// Clear children and clone them from another Object DSD
            ///
            /// @param rhs   Object DSD to be cloned from
            ///
            object_t& operator=(const object_t& rhs);

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
