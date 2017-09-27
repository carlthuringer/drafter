//
//  refract/data/Member.h
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_DATA_MEMBER_H
#define REFRACT_DATA_MEMBER_H

#include <memory>

#include "../ElementIfc.h"

namespace refract
{
    namespace data
    {
        ///
        /// Data structure definition (DSD) of a Refract Member Element
        ///
        /// @remark Defined by a key and a value Refract Element
        ///
        class member_t final
        {
            std::unique_ptr<IElement> key_ = nullptr;   //< key Element
            std::unique_ptr<IElement> value_ = nullptr; //< value Element

        public:
            static const char* name; //< value

        public:
            ///
            /// Initialize a Member DSD with both key and value set to nullptr
            ///
            member_t() = default;

            ///
            /// Initialize a Member DSD from another by consuming its key and value
            ///
            /// @param other   Member DSD to be consumed
            ///
            member_t(member_t&& other) noexcept = default;

            ///
            /// Initialize a Member DSD from another by cloning its key and value
            ///
            /// @param other   Member DSD to be cloned from
            ///
            member_t(const member_t& other);

            ///
            /// Initialize a Member DSD from a key and value
            ///
            /// @param key   key Element to be consumed
            /// @param value value Element to be consumed
            ///
            member_t(std::unique_ptr<IElement> key, std::unique_ptr<IElement> value);

            ///
            /// Initialize a Member DSD from a key string and value
            ///
            /// @param key   key string to be copied
            /// @param value value Element to be consumed
            ///
            member_t(const char* key, std::unique_ptr<IElement> value);

            ///
            /// Initialize a Member DSD from a key string and value
            ///
            /// @param key   key string to be copied
            /// @param value value Element to be consumed
            ///
            member_t(const std::string& key, std::unique_ptr<IElement> value);

            ///
            /// Consume another Member DSD's key and value
            ///
            /// @param other   Member DSD to be consumed
            ///
            member_t& operator=(member_t&&) noexcept = default;

            ///
            /// Copy another Member DSD's key and value
            ///
            /// @param rhs   Member DSD to be copied from
            ///
            member_t& operator=(const member_t& rhs);

        public:
            ///
            /// Query the key Element of this DSD
            ///
            /// @return key Element; nullptr iff  not set
            ///
            const IElement* key() const noexcept
            {
                return key_.get();
            }

            ///
            /// Query the key Element of this DSD
            ///
            /// @return key Element; nullptr iff  not set
            ///
            IElement* key() noexcept
            {
                return key_.get();
            }

            ///
            /// Set the key Element of this DSD
            ///
            /// @param v  Element to be consumed as key
            ///
            void key(std::unique_ptr<IElement> k) noexcept
            {
                key_ = std::move(k);
            }

            ///
            /// Query the value Element of this DSD
            ///
            /// @return value Element; nullptr iff  not set
            ///
            const IElement* value() const noexcept
            {
                return value_.get();
            }

            ///
            /// Query the value Element of this DSD
            ///
            /// @return value Element; nullptr iff  not set
            ///
            IElement* value() noexcept
            {
                return value_.get();
            }

            ///
            /// Set the value Element of this DSD
            ///
            /// @param v  Element to be consumed as value
            ///
            void value(std::unique_ptr<IElement> v) noexcept
            {
                value_ = std::move(v);
            }

            ///
            /// Take ownership of the value Element of this DSD
            /// @remark sets value to nullptr
            ///
            /// @return value of this DSD
            ///
            std::unique_ptr<IElement> claim() noexcept
            {
                return std::move(value_);
            }
        };
    }
}

#endif
