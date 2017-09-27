//
//  test/refract/data/test-Bool.cc
//  test-librefract
//
//  Created by Thomas Jandecka on 27/08/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "catch.hpp"

#include "refract/data/Bool.h"

using namespace refract;
using namespace data;

SCENARIO("`bool_t` is default constructed and both copy- and move constructed from", "[ElementData][bool_t]")
{
    GIVEN("A default initialized bool_t")
    {
        bool_t boolean;

        THEN("its data is false")
        {
            REQUIRE(boolean.get() == false);
        }
        THEN("its value is false")
        {
            bool value = boolean;
            REQUIRE(value == false);
        }

        WHEN("from it another bool_t is copy constructed")
        {
            bool_t boolean2(boolean);

            THEN("the latter bool_t's data is also false")
            {
                REQUIRE(boolean2.get() == false);
            }
        }

        WHEN("from it another bool_t is move constructed")
        {
            bool_t boolean2(std::move(boolean));

            THEN("the latter bool_t's data is also false")
            {
                REQUIRE(boolean2.get() == false);
            }
        }
    }
}

SCENARIO("bool_t is constructed from a value, copy- and move constructed from and all its copies are destroyed",
    "[ElementData][bool_t]")
{
    GIVEN("A bool B = true")
    {
        const bool B = true;

        WHEN("a bool_t is constructed using it")
        {
            {
                bool_t boolean(B);

                THEN("its data is B")
                {
                    REQUIRE(boolean.get() == B);
                }
                THEN("its value is B")
                {
                    const bool P = boolean;
                    REQUIRE(B == P);
                }
            }
        }
    }

    GIVEN("A bool_t with value B = true")
    {
        const bool B = true;
        bool_t boolean(B);

        WHEN("another bool_t is copy constructed from it")
        {
            bool_t boolean2(boolean);

            THEN("its data is B")
            {
                REQUIRE(boolean2.get() == B);
            }
            THEN("its value is B")
            {
                const bool P = boolean2;
                REQUIRE(B == P);
            }
        }

        WHEN("another bool_t is move constructed from it")
        {
            bool_t boolean2(std::move(boolean));

            THEN("its data is B")
            {
                REQUIRE(boolean2.get() == B);
            }
            THEN("its value is B")
            {
                const bool P = boolean2;
                REQUIRE(B == P);
            }
        }
    }
}
