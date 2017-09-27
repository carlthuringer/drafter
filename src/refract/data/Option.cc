//
//  refract/data/Option.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "Option.h"

#include <algorithm>
#include <cassert>

#include "Traits.h"

using namespace refract;
using namespace data;

const char* option_t::name = "option";

static_assert(supports_erase<option_t>::value);
static_assert(supports_empty<option_t>::value);
static_assert(supports_insert<option_t>::value);
static_assert(supports_push_back<option_t>::value);
static_assert(supports_begin<option_t>::value);
static_assert(supports_end<option_t>::value);
static_assert(supports_size<option_t>::value);
static_assert(supports_erase<option_t>::value);
static_assert(is_iterable<option_t>::value);

static_assert(!supports_key<option_t>::value);
static_assert(!supports_value<option_t>::value);
static_assert(!supports_merge<option_t>::value);
static_assert(!is_pair<option_t>::value);

option_t::option_t(container_type elements) : elements_(std::move(elements))
{
    for (const auto& e : elements_)
        assert(e);
}

option_t::option_t(const option_t& other) : elements_()
{
    elements_.reserve(other.elements_.size());
    std::transform(other.elements_.begin(),
        other.elements_.end(),
        std::back_inserter(elements_),
        [](const auto& el) -> std::unique_ptr<IElement> {
            if (!el)
                return nullptr;
            return el->clone();
        });
}

option_t& option_t::operator=(const option_t& rhs)
{
    option_t a(rhs);
    std::swap(a, *this);
    return *this;
}

option_t::iterator option_t::insert(option_t::const_iterator it, std::unique_ptr<IElement> el)
{
    assert(it >= begin());
    assert(it <= end());
    assert(el);

    return elements_.insert(it, std::move(el));
}

option_t::iterator option_t::erase(option_t::const_iterator b, option_t::const_iterator e)
{
    return elements_.erase(b, e);
}
