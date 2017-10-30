//
//  test/refract/data/test-Extend.cc
//  test-librefract
//
//  Created by Thomas Jandecka on 27/08/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "catch.hpp"

#include "refract/data/Extend.h"

#include "refract/Element.h"

using namespace refract;
using namespace data;

SCENARIO("extend_t::merge behaves as expected", "[Element][extend_t]")
{
    GIVEN("A default initialized extend_t")
    {
        extend_t extend;

        THEN("merging it yields nullptr")
        {
            REQUIRE(extend.merge() == nullptr);
            REQUIRE(!extend.merge());
        }
    }

    GIVEN("A extend_t with two empty string elements")
    {
        extend_t extend;
        extend.push_back(make_empty<StringElement>());
        extend.push_back(make_empty<StringElement>());

        WHEN("it is merged")
        {
            auto result = extend.merge();
            THEN("the result is an empty element")
            {
                REQUIRE(result->empty());
            }
            THEN("the result is a string element")
            {
                REQUIRE(dynamic_cast<StringElement*>(result.get()));
            }
        }
    }

    GIVEN("A extend_t with two string elements with test values")
    {
        extend_t extend;

        auto first = make_element<StringElement>("foo");
        extend.insert(extend.end(), std::move(first));

        auto last = make_element<StringElement>("bar");
        const auto& lastRef = *last;
        extend.insert(extend.end(), std::move(last));

        WHEN("it is merged")
        {
            auto result = extend.merge();
            THEN("the result is not an empty element")
            {
                REQUIRE(!result->empty());
            }
            THEN("the result is a string element")
            {
                const auto resultEl = dynamic_cast<StringElement*>(result.get());
                REQUIRE(resultEl);

                THEN("the result element is a clone of the last in the original")
                {
                    REQUIRE(resultEl->meta().size() == 0);
                    REQUIRE(resultEl->attributes().size() == 0);
                    REQUIRE(resultEl->get() == lastRef.get());
                }
            }
        }
    }

    GIVEN("A extend_t with two array elements with three and two test values")
    {
        extend_t extend;

        auto first = make_element<ArrayElement>();
        first->get().push_back(make_element<StringElement>("foo"));
        first->get().push_back(make_element<StringElement>("bar"));
        first->get().push_back(make_element<StringElement>("baz"));
        extend.insert(extend.end(), std::move(first));

        auto last = make_element<ArrayElement>();
        last->get().push_back(make_element<StringElement>("zur"));
        last->get().push_back(make_element<NumberElement>(42.0));
        extend.insert(extend.end(), std::move(last));

        WHEN("it is merged")
        {
            auto result = extend.merge();
            THEN("the result is not an empty element")
            {
                REQUIRE(!result->empty());
            }
            THEN("the result is an array element")
            {
                const auto resultEl = dynamic_cast<ArrayElement*>(result.get());
                REQUIRE(resultEl);

                THEN("the result element is a concatenation of both arrays")
                {
                    REQUIRE(resultEl->meta().size() == 0);
                    REQUIRE(resultEl->attributes().size() == 0);

                    REQUIRE(resultEl->get().size() == 5);
                }
            }
        }
    }
}
