//
//  refract/data/Enum.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "Enum.h"

#include <algorithm>
#include <cassert>

using namespace refract;
using namespace data;

const char* enum_t::name = "enum";

static_assert(supports_erase<enum_t>::value);
static_assert(supports_empty<enum_t>::value);
static_assert(supports_insert<enum_t>::value);
static_assert(supports_push_back<enum_t>::value);
static_assert(supports_begin<enum_t>::value);
static_assert(supports_end<enum_t>::value);
static_assert(supports_size<enum_t>::value);
static_assert(supports_erase<enum_t>::value);
static_assert(is_iterable<enum_t>::value);

static_assert(!supports_key<enum_t>::value);
static_assert(!supports_value<enum_t>::value);
static_assert(!supports_merge<enum_t>::value);
static_assert(!is_pair<enum_t>::value);

enum_t::enum_t(enum_t::container_type elements) : elements_(std::move(elements))
{
    for (const auto& e : elements_)
        assert(e);
}

enum_t::enum_t(const enum_t& other) : elements_()
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

enum_t& enum_t::operator=(const enum_t& rhs)
{
    enum_t a(rhs);
    std::swap(a, *this);
    return *this;
}

enum_t::iterator enum_t::insert(enum_t::const_iterator it, std::unique_ptr<IElement> el)
{
    assert(it >= begin());
    assert(it <= end());
    assert(el);

    return elements_.insert(it, std::move(el));
}

enum_t::iterator enum_t::erase(enum_t::const_iterator b, enum_t::const_iterator e)
{
    return elements_.erase(b, e);
}
