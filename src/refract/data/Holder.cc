//
//  refract/data/Holder.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "Holder.h"

#include <cassert>

#include "Traits.h"

using namespace refract;
using namespace data;

const char* holder_t::name = "";

static_assert(!supports_erase<holder_t>::value);
static_assert(!supports_empty<holder_t>::value);
static_assert(!supports_insert<holder_t>::value);
static_assert(!supports_push_back<holder_t>::value);
static_assert(!supports_begin<holder_t>::value);
static_assert(!supports_end<holder_t>::value);
static_assert(!supports_size<holder_t>::value);
static_assert(!supports_erase<holder_t>::value);
static_assert(!supports_merge<holder_t>::value);
static_assert(!is_iterable<holder_t>::value);
static_assert(!supports_key<holder_t>::value);
static_assert(!supports_value<holder_t>::value);
static_assert(!is_pair<holder_t>::value);

holder_t::holder_t(const holder_t& other) : data_(other.data_ ? other.data_->clone() : nullptr)
{
}

holder_t::holder_t(std::unique_ptr<IElement> el) : data_(std::move(el))
{
    assert(data_);
}

holder_t& holder_t::operator=(const holder_t& rhs)
{
    holder_t h(rhs);
    std::swap(h, *this);
    return *this;
}
