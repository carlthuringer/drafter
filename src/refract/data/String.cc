//
//  refract/data/String.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "String.h"

#include "Traits.h"

using namespace refract;
using namespace data;

const char* string_t::name = "string";

static_assert(!supports_erase<string_t>::value);
static_assert(!supports_empty<string_t>::value);
static_assert(!supports_insert<string_t>::value);
static_assert(!supports_push_back<string_t>::value);
static_assert(!supports_begin<string_t>::value);
static_assert(!supports_end<string_t>::value);
static_assert(!supports_size<string_t>::value);
static_assert(!supports_erase<string_t>::value);
static_assert(!supports_key<string_t>::value);
static_assert(!supports_value<string_t>::value);
static_assert(!supports_merge<string_t>::value);
static_assert(!is_iterable<string_t>::value);
static_assert(!is_pair<string_t>::value);

string_t::string_t(std::string s) noexcept : value_(std::move(s))
{
}
