//
//  test/refract/data/test-Number.cc
//  test-librefract
//
//  Created by Thomas Jandecka on 27/08/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "catch.hpp"

#include "refract/data/Number.h"

using namespace refract;
using namespace data;

SCENARIO("`number_t` is default constructed and both copy- and move constructed from", "[ElementData][number_t]")
{
    GIVEN("A default initialized number_t")
    {
        number_t number;

        THEN("its data is zero")
        {
            REQUIRE(number.get() == 0.0);
        }
        THEN("its value is zero")
        {
            double n = number;
            REQUIRE(n == 0.0);
        }

        WHEN("from it another number_t is copy constructed")
        {
            number_t number2(number);

            THEN("the latter number_t's data is also zero")
            {
                REQUIRE(number2.get() == 0);
            }
        }

        WHEN("from it another number_t is move constructed")
        {
            number_t number2(std::move(number));

            THEN("the latter number_t's data is also zero")
            {
                REQUIRE(number2.get() == 0);
            }
        }
    }
}

SCENARIO("number_t is constructed from values, both copy- and move constructed from and all its copies are destroyed",
    "[ElementData][number_t]")
{
    GIVEN("A double N = 39025.341")
    {
        const double N = 39025.341;

        WHEN("a number_t is constructed using it")
        {
            {
                number_t number(N);

                THEN("its data is N")
                {
                    REQUIRE(number.get() == N);
                }
                THEN("its value is N")
                {
                    const double P = number;
                    REQUIRE(N == P);
                }
            }
        }
    }

    GIVEN("A number_t with value N = 34893.2539")
    {
        const double N = 34893.2539;
        number_t number(N);

        WHEN("another number_t is copy constructed from it")
        {
            number_t number2(number);

            THEN("its data is N")
            {
                REQUIRE(number2.get() == N);
            }
            THEN("its value is N")
            {
                const double P = number2;
                REQUIRE(N == P);
            }
        }

        WHEN("another number_t is move constructed from it")
        {
            number_t number2(std::move(number));

            THEN("its data is N")
            {
                REQUIRE(number2.get() == N);
            }
            THEN("its value is N")
            {
                const double P = number2;
                REQUIRE(N == P);
            }
        }
    }
}
