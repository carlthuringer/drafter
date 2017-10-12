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
        public:
            using Data = std::unique_ptr<IElement>;

            static const char* name; //< syntactical name of the DSD

        private:
            Data value_ = nullptr;

        public:
            ///
            /// Initialize an empty Enum DSD
            ///
            enum_t() = default;

            ///
            /// Initialize an Enum DSD from its value
            ///
            /// @param value     an Element
            ///
            explicit enum_t(Data value);

            ///
            /// Initialize an Enum DSD from another by consuming its memory representation
            ///
            /// @param other   Enum DSD to be consumed
            ///
            enum_t(enum_t&& other) = default;

            ///
            /// Initialize an Enum DSD from another by cloning its children
            ///
            /// @param other   Enum DSD to be cloned from
            ///
            enum_t(const enum_t& other);

            ///
            /// Clear children and consume another Enum DSD's memory representation
            ///
            /// @param rhs   Enum DSD to be consumed
            ///
            enum_t& operator=(enum_t&& rhs) = default;

            ///
            /// Clear children and clone them from another Enum DSD
            ///
            /// @param rhs   Enum DSD to be cloned from
            ///
            enum_t& operator=(const enum_t& rhs);

            ~enum_t() = default;

        public:
            const IElement* value() const noexcept
            {
                return value_.get();
            }

            ///
            /// Take ownership of the Element of this DSD
            /// @remark sets Element to nullptr
            ///
            /// @return Element of this DSD
            ///
            Data claim() noexcept {
                return std::move(value_);
            }
        };
    }
}

#endif
