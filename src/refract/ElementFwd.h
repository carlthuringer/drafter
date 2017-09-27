//
//  refract/ElementFwd.h
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_ELEMENTFWD_H
#define REFRACT_ELEMENTFWD_H

namespace refract
{
    namespace data
    {
        struct null_t;
        struct string_t;
        struct bool_t;
        struct number_t;
        struct ref_t;
        struct holder_t;
        struct member_t;
        struct array_t;
        struct enum_t;
        struct object_t;
        struct extend_t;
        struct option_t;
        struct select_t;
    }

    template <typename>
    class Element;

    using NullElement = Element<data::null_t>;
    using StringElement = Element<data::string_t>;
    using BooleanElement = Element<data::bool_t>;
    using NumberElement = Element<data::number_t>;
    using RefElement = Element<data::ref_t>;

    using HolderElement = Element<data::holder_t>;
    using MemberElement = Element<data::member_t>;

    using ArrayElement = Element<data::array_t>;
    using EnumElement = Element<data::enum_t>;
    using ObjectElement = Element<data::object_t>;

    using ExtendElement = Element<data::extend_t>;

    using OptionElement = Element<data::option_t>;
    using SelectElement = Element<data::select_t>;
}

#endif
