//
//  test/refract/data/test-String.cc
//  test-librefract
//
//  Created by Thomas Jandecka on 27/08/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "catch.hpp"

#include "refract/data/String.h"

using namespace refract;
using namespace data;

SCENARIO("`string_t` is default constructed and both copy- and move constructed from", "[ElementData][string_t]")
{
    GIVEN("A default initialized string_t")
    {
        string_t string;

        THEN("its data is an empty string")
        {
            REQUIRE(string.get() == "");
        }
        THEN("its value is an empty string")
        {
            std::string value = string;
            REQUIRE(value == "");
        }
        THEN("it is empty")
        {
            REQUIRE(string.empty());
        }

        WHEN("from it another string_t is copy constructed")
        {
            string_t string2(string);

            THEN("the latter string_t's data is also an empty string")
            {
                REQUIRE(string2.get() == "");
            }
            THEN("it is empty")
            {
                REQUIRE(string2.empty());
            }
        }

        WHEN("from it another string_t is move constructed")
        {
            string_t string2(std::move(string));

            THEN("the latter string_t's data is also an empty string")
            {
                REQUIRE(string2.get() == "");
            }
            THEN("it is empty")
            {
                REQUIRE(string2.empty());
            }
        }
    }
}

SCENARIO("string_t is constructed from values, both copy- and move constructed from and all its copies are destroyed",
    "[ElementData][string_t]")
{
    GIVEN("A std::string B = `foobar32dsfjklsgh\\000fdfjks`")
    {
        const std::string B = "foobar32dsfjklsgh\000fdfjks";

        WHEN("a string_t is constructed using it")
        {
            {
                string_t string(B);

                THEN("its data is B")
                {
                    REQUIRE(string.get() == B);
                }
                THEN("its value is B")
                {
                    const std::string P = string;
                    REQUIRE(B == P);
                }
                THEN("it is not empty")
                {
                    REQUIRE(!string.empty());
                }
            }
        }
    }

    GIVEN("A string_t with value B = \"foobar32dsfjklsgh\\000fdfjks\"")
    {
        const std::string B = "foobar32dsfjklsgh\000fdfjks";
        string_t string(B);

        WHEN("another string_t is copy constructed from it")
        {
            string_t string2(string);

            THEN("its data is B")
            {
                REQUIRE(string2.get() == B);
            }
            THEN("its value is B")
            {
                const std::string P = string2;
                REQUIRE(B == P);
            }
        }

        WHEN("another string_t is move constructed from it")
        {
            string_t string2(std::move(string));

            THEN("its data is B")
            {
                REQUIRE(string2.get() == B);
            }
            THEN("its value is B")
            {
                const std::string P = string2;
                REQUIRE(B == P);
            }
        }
    }
}


