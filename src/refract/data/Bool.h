//
//  refract/data/Bool.h
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_DATA_BOOL_H
#define REFRACT_DATA_BOOL_H

namespace refract
{
    namespace data
    {
        ///
        /// Data structure definition (DSD) of a Refract Bool Element
        ///
        /// @remark Defined by its value
        /// TODO move value to attributes and leave this defined by its type
        ///
        class bool_t final
        {
            bool data_ = false; // literal value

        public:
            static const char* name; //< syntactical name of the DSD

        public:
            ///
            /// Initialize a Bool DSD with literal `false`
            ///
            bool_t() = default;
            //
            ///
            /// Initialize a Bool DSD with custom literal
            ///
            bool_t(bool v) noexcept : data_(v)
            {
            }

        public:
            ///
            /// Get the value of this Bool DSD
            ///
            /// @return the value
            ///
            bool get() const noexcept
            {
                return data_;
            }

        public:
            ///
            /// Implicitly convert this DSD to its value
            ///
            /// @returns this DSD's value
            ///
            operator bool() const noexcept
            {
                return data_;
            }
        };
    }
}

#endif

