//
//  refract/data/Member.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "Member.h"

#include "Traits.h"
#include "../ElementFwd.h"
#include "../Element.h"
#include "String.h"

using namespace refract;
using namespace data;

const char* member_t::name = "member";

static_assert(!supports_erase<member_t>::value);
static_assert(!supports_empty<member_t>::value);
static_assert(!supports_insert<member_t>::value);
static_assert(!supports_push_back<member_t>::value);
static_assert(!supports_begin<member_t>::value);
static_assert(!supports_end<member_t>::value);
static_assert(!supports_size<member_t>::value);
static_assert(!supports_erase<member_t>::value);
static_assert(!supports_merge<member_t>::value);
static_assert(!is_iterable<member_t>::value);
static_assert(supports_key<member_t>::value);
static_assert(supports_value<member_t>::value);
static_assert(is_pair<member_t>::value);

member_t::member_t(const member_t& other)
    : key_(other.key_ ? clone(*other.key_) : nullptr), value_(other.value_ ? clone(*other.value_) : nullptr)
{
}
member_t::member_t(std::unique_ptr<IElement> key, std::unique_ptr<IElement> value)
    : key_(std::move(key)), value_(std::move(value))
{
}

member_t::member_t(const char* key, std::unique_ptr<IElement> value)
    : key_(make_element<StringElement>(key)), value_(std::move(value))
{
}

member_t::member_t(const std::string& key, std::unique_ptr<IElement> value)
    : member_t(key.c_str(), std::move(value))
{
}

member_t& member_t::operator=(const member_t& rhs)
{
    member_t _(rhs);
    std::swap(_, *this);
    return *this;
}
