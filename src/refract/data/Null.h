//
//  refract/data/Null.h
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_DATA_NULL_H
#define REFRACT_DATA_NULL_H

namespace refract
{
    namespace data
    {
        ///
        /// Data structure definition (DSD) of a Refract Null Element
        ///
        /// @remark Defined by its type
        ///
        struct null_t final {
            static const char* name; //< syntactical name of the DSD
        };
    }
}

#endif

