//
//  refract/data/Number.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "Number.h"

#include "Traits.h"

using namespace refract;
using namespace data;

const char* number_t::name = "number";

static_assert(!supports_erase<number_t>::value);
static_assert(!supports_empty<number_t>::value);
static_assert(!supports_insert<number_t>::value);
static_assert(!supports_push_back<number_t>::value);
static_assert(!supports_begin<number_t>::value);
static_assert(!supports_end<number_t>::value);
static_assert(!supports_size<number_t>::value);
static_assert(!supports_erase<number_t>::value);
static_assert(!supports_key<number_t>::value);
static_assert(!supports_value<number_t>::value);
static_assert(!supports_merge<number_t>::value);
static_assert(!is_iterable<number_t>::value);
static_assert(!is_pair<number_t>::value);
