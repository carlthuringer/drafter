//
//  refract/data/Enum.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "Enum.h"

#include <algorithm>
#include <cassert>

using namespace refract;
using namespace data;

const char* enum_t::name = "enum";

static_assert(!supports_erase<enum_t>::value);
static_assert(!supports_empty<enum_t>::value);
static_assert(!supports_insert<enum_t>::value);
static_assert(!supports_push_back<enum_t>::value);
static_assert(!supports_begin<enum_t>::value);
static_assert(!supports_end<enum_t>::value);
static_assert(!supports_size<enum_t>::value);
static_assert(!supports_erase<enum_t>::value);
static_assert(!is_iterable<enum_t>::value);

static_assert(!supports_key<enum_t>::value);
static_assert(supports_value<enum_t>::value);
static_assert(!supports_merge<enum_t>::value);
static_assert(!is_pair<enum_t>::value);

enum_t::enum_t(enum_t::Data value) : value_(std::move(value))
{
        assert(value_);
}

enum_t::enum_t(const enum_t& other) : value_(other.value_ ? other.value_->clone() : nullptr)
{
}

enum_t& enum_t::operator=(const enum_t& rhs)
{
    enum_t a(rhs);
    std::swap(a, *this);
    return *this;
}
