//
//  refract/data/Array.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "Array.h"

#include <cassert>

using namespace refract;
using namespace data;

const char* array_t::name = "array";

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

array_t::array_t(array_t::container_type data) : elements_(std::move(data))
{
    for (const auto& e : elements_)
        assert(e);
}

array_t& array_t::operator=(const array_t& rhs)
{
    array_t a(rhs);
    std::swap(a.elements_, this->elements_);
    return *this;
}

array_t::array_t(const array_t& other) : elements_()
{
    elements_.reserve(other.elements_.size());
    std::transform(other.elements_.begin(),
        other.elements_.end(),
        std::back_inserter(elements_),
        [](const auto& el) -> std::unique_ptr<IElement> {
            if (!el)
                return nullptr;
            return std::unique_ptr<IElement>(el->clone());
        });
}

array_t::iterator array_t::insert(array_t::const_iterator it, std::unique_ptr<IElement> el)
{
    assert(it >= begin());
    assert(it <= end());
    assert(el);

    return elements_.insert(it, std::move(el));
}

array_t::iterator array_t::erase(array_t::const_iterator b, array_t::const_iterator e)
{
    return elements_.erase(b, e);
}
