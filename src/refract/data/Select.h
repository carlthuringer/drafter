//
//  refract/data/Select.h
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_DATA_SELECT_H
#define REFRACT_DATA_SELECT_H

#include <vector>
#include <memory>

#include "../ElementFwd.h"
#include "Option.h"
#include "Traits.h"

namespace refract
{
    namespace data
    {
        ///
        /// Data structure definition (DSD) of a Refract Select Element
        ///
        /// @remark Defined by a set of Refract Option Elements
        ///
        class select_t final
            : public container_traits<select_t, std::vector<std::unique_ptr<OptionElement> > >
        {
            container_type elements_; //< set of Option Elements

        public:
            static const char* name; //< syntactical name of the DSD

        public:
            ///
            /// Initialize an empty Select DSD
            ///
            select_t() = default;

            ///
            /// Initialize a Select DSD from its memory representation
            /// @deprecated
            ///
            /// @param data     memory represenation
            ///
            [[deprecated]] select_t(container_type elements) noexcept;

            ///
            /// Initialize a Select DSD from another by consuming its memory representation
            ///
            /// @param other   Select DSD to be consumed
            ///
            select_t(select_t&&) = default;

            ///
            /// Initialize a Select DSD from another by cloning its children
            ///
            /// @param other   Select DSD to be cloned from
            ///
            select_t(const select_t& other);

            ///
            /// Clear children and consume another Select DSD's memory representation
            ///
            /// @param other   Select DSD to be consumed
            ///
            select_t& operator=(select_t&&) = default;

            ///
            /// Clear children and clone them from another Select DSD
            ///
            /// @param other   Select DSD to be cloned from
            ///
            select_t& operator=(const select_t& rhs);

            ~select_t();

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
            iterator insert(const_iterator it, std::unique_ptr<OptionElement> el);

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
