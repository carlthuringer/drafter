//
//  refract/data/Object.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "Object.h"

#include <algorithm>
#include "Traits.h"

using namespace refract;
using namespace data;

const char* object_t::name = "object";

static_assert(supports_erase<object_t>::value);
static_assert(supports_empty<object_t>::value);
static_assert(supports_insert<object_t>::value);
static_assert(supports_push_back<object_t>::value);
static_assert(supports_begin<object_t>::value);
static_assert(supports_end<object_t>::value);
static_assert(supports_size<object_t>::value);
static_assert(supports_erase<object_t>::value);
static_assert(is_iterable<object_t>::value);

static_assert(!supports_key<object_t>::value);
static_assert(!supports_value<object_t>::value);
static_assert(!supports_merge<object_t>::value);
static_assert(!is_pair<object_t>::value);

object_t::object_t(const object_t& other) : elements_()
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

object_t::object_t(object_t::container_type elements) noexcept : elements_(std::move(elements))
{
    for (const auto& e : elements_)
        assert(e);
}

object_t& object_t::operator=(const object_t& rhs)
{
    object_t a(rhs);
    std::swap(a, *this);
    return *this;
}

object_t::iterator object_t::insert(object_t::const_iterator it, std::unique_ptr<IElement> el)
{
    assert(it >= begin());
    assert(it <= end());
    assert(el);

    return elements_.insert(it, std::move(el));
}

object_t::iterator object_t::erase(object_t::const_iterator b, object_t::const_iterator e)
{
    return elements_.erase(b, e);
}
