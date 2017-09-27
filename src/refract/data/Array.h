//
//  refract/data/Array.h
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_DATA_ARRAY_H
#define REFRACT_DATA_ARRAY_H

#include <vector>
#include <memory>
#include <algorithm>

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
        class array_t final : public container_traits<array_t, std::vector<std::unique_ptr<IElement> > >
        {
            container_type elements_; //< sequence of Elements

        public:
            static const char* name; //< syntactical name of the DSD

        public:
            ///
            /// Initialize an empty Array DSD
            ///
            array_t() = default;

            ///
            /// Initialize an Array DSD from its memory representation
            /// @deprecated
            ///
            /// @param data     memory represenation
            ///
            [[deprecated]] array_t(container_type data);

            ///
            /// Initialize an Array DSD from a range of Elements
            ///
            /// @tparam It  range iterator type
            ///
            /// @param b    iterator to the first Element to be cloned
            /// @param e    iterator following the last Element to be cloned
            ///
            template <typename It>
            array_t(It b, It e) : elements_()
            {
                elements_.reserve(e - b);
                std::transform(b, e, std::back_inserter(elements_), [](const auto& el) {
                    assert(el);
                    return el->clone();
                });
            }

            ///
            /// Initialize an Array DSD from another by consuming its memory representation
            ///
            /// @param other   Array DSD to be consumed
            ///
            array_t(array_t&& other) = default;

            ///
            /// Initialize an Array DSD from another by cloning its children
            ///
            /// @param other   Array DSD to be cloned from
            ///
            array_t(const array_t& other);

            ///
            /// Clear children and consume another Array DSD's memory representation
            ///
            /// @param rhs   Array DSD to be consumed
            ///
            array_t& operator=(array_t&& rhs) = default;

            ///
            /// Clear children and clone them from another Array DSD
            ///
            /// @param rhs   Array DSD to be cloned from
            ///
            array_t& operator=(const array_t& rhs);

            ~array_t() = default;

        public:
            ///
            /// Get reference to memory representation
            /// @deprecated
            ///
            [[deprecated]] const container_type& data() const noexcept
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

            using container_traits<array_t, container_type>::erase;
        };
    }
}

#endif
