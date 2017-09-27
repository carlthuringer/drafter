//
//  refract/data/Bool.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "Bool.h"

#include "Traits.h"

using namespace refract;
using namespace data;

const char* bool_t::name = "bool";

static_assert(!supports_erase<bool_t>::value);
static_assert(!supports_empty<bool_t>::value);
static_assert(!supports_push_back<bool_t>::value);
static_assert(!supports_begin<bool_t>::value);
static_assert(!supports_end<bool_t>::value);
static_assert(!supports_size<bool_t>::value);
static_assert(!supports_erase<bool_t>::value);
static_assert(!supports_key<bool_t>::value);
static_assert(!supports_value<bool_t>::value);
static_assert(!supports_merge<bool_t>::value);
static_assert(!is_iterable<bool_t>::value);
static_assert(!is_pair<bool_t>::value);
