//
//  refract/data/Holder.h
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_DATA_HOLDER_H
#define REFRACT_DATA_HOLDER_H

#include <memory>

#include "../ElementIfc.h"

namespace refract
{
    namespace data
    {
        ///
        /// Data structure definition (DSD) of a Holder Element
        ///
        /// @remark Defined by another Refract Element
        /// FIXME: This is just a temporary element which is not in the refract spec
        /// until the Element implementation is moved away from abstraction.
        class holder_t final
        {
        public:
            using Data = std::unique_ptr<IElement>;

            static const char* name; //< syntactical name of the DSD

        private:
            Data data_ = nullptr;

        public:
            ///
            /// Initialize a Holder DSD with its Element set to nullptr
            ///
            holder_t() = default;

            ///
            /// Initialize a Holder DSD from another by consuming its Element
            ///
            /// @param other   Holder DSD to be consumed
            ///
            holder_t(holder_t&& other) = default;

            ///
            /// Initialize a Holder DSD from another by cloning its Element
            ///
            /// @param other   Holder DSD to be cloned from
            ///
            holder_t(const holder_t& other);

            ///
            /// Initialize a Member DSD from an Element
            ///
            /// @param el   Element to be consumed
            ///
            explicit holder_t(std::unique_ptr<IElement> el);

            ///
            /// Consume another Holder DSD's Element
            ///
            /// @param other   Holder DSD to be consumed
            ///
            holder_t& operator=(holder_t&&) = default;

            /// Copy another Holder DSD's Element
            ///
            /// @param other   Holder DSD to be copied from
            ///
            holder_t& operator=(const holder_t& rhs);

            ~holder_t() = default;

        public:
            const IElement* data() const noexcept
            {
                return data_.get();
            }

            ///
            /// Take ownership of the Element of this DSD
            /// @remark sets Element to nullptr
            ///
            /// @return Element of this DSD
            ///
            Data claim() noexcept {
                return std::move(data_);
            }
        };
    }
}

#endif
