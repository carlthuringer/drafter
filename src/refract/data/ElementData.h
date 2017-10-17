//
//  refract/data/ElementData.h
//  librefract
//
//  Created by Thomas Jandecka on 05/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_DATA_ELEMENTDATA_H
#define REFRACT_DATA_ELEMENTDATA_H

#include "Array.h"
#include "Bool.h"
#include "Enum.h"
#include "Extend.h"
#include "Holder.h"
#include "Member.h"
#include "Null.h"
#include "Number.h"
#include "Object.h"
#include "Option.h"
#include "Ref.h"
#include "Select.h"
#include "String.h"

namespace refract
{
    namespace data
    {
        ///
        /// Wrap type with associated data type
        ///
        /// Obtains the data representation of a primitive type
        ///
        /// @tparam T   primitive type to wrap
        ///
        template <typename T>
        struct data_of {
        };

        template <>
        struct data_of<bool> {
            using type = data::bool_t;
        };

        template <>
        struct data_of<nullptr_t> {
            using type = data::null_t;
        };

        template <>
        struct data_of<double> {
            using type = data::number_t;
        };

        template <>
        struct data_of<int> {
            using type = data::number_t;
        };

        template <>
        struct data_of<std::string> {
            using type = data::string_t;
        };

        template <size_t N>
        struct data_of<char[N]> {
            using type = data::string_t;
        };
    }
}

#endif
