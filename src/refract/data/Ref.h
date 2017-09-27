//
//  refract/data/Ref.h
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_DATA_REF_H
#define REFRACT_DATA_REF_H

#include <string>

namespace refract
{
    namespace data
    {
        ///
        /// Data structure definition (DSD) of a Refract Ref Element
        ///
        /// @remark Defined by a string symbol
        ///
        class ref_t final
        {
        public:
            static const char* name; //< syntactical name of the DSD

        private:
            std::string data_ = {}; //< string symbol

        public:
            ///
            /// Initialize a Ref DSD with an empty symbol
            ///
            ref_t() = default;

            ///
            /// Initialize a Ref DSD from a string symbol
            ///
            explicit ref_t(std::string symbol);

            ///
            /// Query string symbol
            ///
            /// @return the Ref's string symbol
            ///
            const std::string& symbol() const noexcept
            {
                return data_;
            }

            ///
            /// Check whether two Ref Elements are equivalent
            ///
            /// @return true iff the underlying string symbols are equivalent
            ///
            friend bool operator==(const ref_t& lhs, const ref_t& rhs)
            {
                return lhs.data_ == rhs.data_;
            }

            ///
            /// Check whether a Ref Element is equivalent to a string symbol
            ///
            /// @return true iff the underlying string symbols are equivalent
            ///
            friend bool operator==(const ref_t& lhs, const std::string& rhs)
            {
                return lhs.data_ == rhs;
            }
        };
    }
}

#endif
