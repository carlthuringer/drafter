//
//  refract/data/Ref.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "Ref.h"

#include "Traits.h"

using namespace refract;
using namespace data;

const char* ref_t::name = "ref";

static_assert(!supports_erase<ref_t>::value);
static_assert(!supports_empty<ref_t>::value);
static_assert(!supports_insert<ref_t>::value);
static_assert(!supports_push_back<ref_t>::value);
static_assert(!supports_begin<ref_t>::value);
static_assert(!supports_end<ref_t>::value);
static_assert(!supports_size<ref_t>::value);
static_assert(!supports_erase<ref_t>::value);
static_assert(!is_iterable<ref_t>::value);
static_assert(!supports_key<ref_t>::value);
static_assert(!supports_value<ref_t>::value);
static_assert(!supports_merge<ref_t>::value);
static_assert(!is_pair<ref_t>::value);

ref_t::ref_t(std::string symbol) : data_(std::move(symbol))
{
}
