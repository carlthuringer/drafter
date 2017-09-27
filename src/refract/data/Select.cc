//
//  refract/data/Select.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "Select.h"

#include <algorithm>

#include "Traits.h"
#include "Option.h"
#include "../Element.h"

using namespace refract;
using namespace data;

const char* select_t::name = "select";

static_assert(supports_erase<array_t>::value);
static_assert(supports_empty<array_t>::value);
static_assert(supports_insert<array_t>::value);
static_assert(supports_push_back<array_t>::value);
static_assert(supports_begin<array_t>::value);
static_assert(supports_end<array_t>::value);
static_assert(supports_size<array_t>::value);
static_assert(supports_erase<array_t>::value);
static_assert(is_iterable<array_t>::value);

static_assert(!supports_key<array_t>::value);
static_assert(!supports_value<array_t>::value);
static_assert(!supports_merge<array_t>::value);
static_assert(!is_pair<array_t>::value);

select_t::select_t(const select_t& other) : elements_()
{
    elements_.reserve(other.elements_.size());
    std::transform(other.elements_.begin(),
        other.elements_.end(),
        std::back_inserter(elements_),
        [](const auto& el) -> std::unique_ptr<OptionElement> {
            if (!el)
                return nullptr;
            return clone(*el);
        });
}

select_t::select_t(select_t::container_type elements) noexcept : elements_(std::move(elements))
{
    for (const auto& e : elements_)
        assert(e);
}

select_t& select_t::operator=(const select_t& rhs)
{
    select_t a(rhs);
    std::swap(a, *this);
    return *this;
}

select_t::iterator select_t::insert(select_t::const_iterator it, std::unique_ptr<OptionElement> el)
{
    assert(it >= begin());
    assert(it <= end());
    assert(el);

    return elements_.insert(it, std::move(el));
}

select_t::iterator select_t::erase(select_t::const_iterator b, select_t::const_iterator e)
{
    return elements_.erase(b, e);
}

select_t::~select_t()
{
}
