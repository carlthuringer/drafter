//
//  refract/data/Null.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "Null.h"

#include "Traits.h"

using namespace refract;
using namespace data;

const char* null_t::name = "null";

static_assert(!supports_erase<null_t>::value);
static_assert(!supports_empty<null_t>::value);
static_assert(!supports_insert<null_t>::value);
static_assert(!supports_push_back<null_t>::value);
static_assert(!supports_begin<null_t>::value);
static_assert(!supports_end<null_t>::value);
static_assert(!supports_size<null_t>::value);
static_assert(!supports_erase<null_t>::value);
static_assert(!supports_key<null_t>::value);
static_assert(!supports_value<null_t>::value);
static_assert(!supports_merge<null_t>::value);
static_assert(!is_iterable<null_t>::value);
static_assert(!is_pair<null_t>::value);
