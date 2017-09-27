//
//  test/refract/data/test-Member.cc
//  test-librefract
//
//  Created by Thomas Jandecka on 27/08/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "catch.hpp"

#include "refract/data/Member.h"

#include "refract/data/Element.h"
#include "ElementMock.h"

using namespace refract;
using namespace experimental;
using namespace data;

SCENARIO("`member_t` is default constructed and both copy- and move constructed from", "[ElementData][member_t]")
{
    GIVEN("A default initialized member_t")
    {
        member_t member;

        THEN("its key and value are nullptr")
        {
            REQUIRE(member.key() == nullptr);
            REQUIRE(member.value() == nullptr);
        }

        WHEN("from it another member_t is copy constructed")
        {
            member_t member2(member);

            THEN("the latter member_t's key and value are also nullptr")
            {
                REQUIRE(member2.key() == nullptr);
                REQUIRE(member2.value() == nullptr);
            }
        }

        WHEN("from it another member_t is move constructed")
        {
            member_t member2(std::move(member));

            THEN("the latter member_t's key and value are also nullptr")
            {
                REQUIRE(member2.key() == nullptr);
                REQUIRE(member2.value() == nullptr);
            }
        }
    }
}

SCENARIO("member_t is constructed from values, both copy- and move constructed from and all its copies are destroyed",
    "[ElementData][member_t]")
{
    GIVEN("A std::string key and an ElementMock* value")
    {
        std::string key = "eigenvalue";
        auto mock = new test::ElementMock{};

        REQUIRE(test::ElementMock::instances().size() == 1);
        REQUIRE(&test::ElementMock::last_instance() == mock);

        WHEN("a member_t is constructed using them")
        {
            {
                member_t member(key, std::unique_ptr<IElement>(mock));

                THEN("its key is not nullptr")
                {
                    REQUIRE(member.key());
                }
                THEN("its element obtained by key() holds a StringElement with the key")
                {
                    auto strValue = dynamic_cast<const StringElement*>(member.key());
                    REQUIRE(strValue);
                    REQUIRE(key == strValue->get());
                }
                THEN("its element obtained by value() is the mock")
                {
                    auto value = dynamic_cast<const test::ElementMock*>(member.value());
                    REQUIRE(value == mock);
                }
                THEN("its mock still is the only instance of its type")
                {
                    REQUIRE(test::ElementMock::instances().size() == 1);
                    REQUIRE(&test::ElementMock::last_instance() == mock);
                }
                THEN("nothing was called on the mock")
                {
                    REQUIRE(test::ElementMock::last_instance()._total_ctx == 0);
                }
            }

            WHEN("it goes out of scope")
            {
                THEN("the value is destroyed")
                {
                    REQUIRE(test::ElementMock::instances().empty());
                }
            }
        }
    }

    GIVEN("A member_t with a StringElement key and an ElementMock value")
    {
        std::string key = "eigenvalue";
        auto mock = new test::ElementMock{};
        mock->clone_out = std::make_unique<test::ElementMock>();
        auto clone_out_ptr = mock->clone_out.get();
        mock->_value = 349802;

        REQUIRE(test::ElementMock::instances().size() == 2);
        member_t member(key, std::unique_ptr<IElement>(mock));

        WHEN("another member_t is copy constructed from it")
        {
            member_t member2(member);

            THEN("its key and value differ from the original")
            {
                REQUIRE(member.key() != member2.key());
                REQUIRE(member.value() != member2.value());
            }

            THEN("its key is a StringElement containing a copy of the original key string")
            {
                auto strValue = dynamic_cast<const StringElement*>(member2.key());
                REQUIRE(strValue);
                REQUIRE(key == strValue->get());
            }

            THEN("its value is the result of invoking clone() on original value")
            {
                REQUIRE(mock->_total_ctx == 1);
                REQUIRE(mock->clone_ctx == 1);
                REQUIRE(mock->clone_in == IElement::cAll);

                REQUIRE(member2.value() == clone_out_ptr);
            }

            THEN("no further ElementMocks are constructed")
            {
                REQUIRE(test::ElementMock::instances().size() == 2);
            }
        }

        WHEN("another member_t is move constructed from it")
        {
            auto memberKey = member.key();
            auto memberValue = member.value();

            member_t member2(std::move(member));

            THEN("the original's key and value are now nullptr")
            {
                REQUIRE(member.key() == nullptr);
                REQUIRE(member.value() == nullptr);
            }

            THEN("its key and value are equal to the original key and value")
            {
                REQUIRE(member2.key() == memberKey);
                REQUIRE(member2.value() == memberValue);
            }

            THEN("nothing gets called on the original mock")
            {
                REQUIRE(mock->_total_ctx == 0);
            }

            THEN("no further ElementMocks are constructed")
            {
                REQUIRE(test::ElementMock::instances().size() == 2);
            }
        }
    }
}

SCENARIO("`member_t` is modified", "[ElementData][member_t]")
{
    GIVEN("A member_t with a StringElement key and an ElementMock value")
    {
        std::string key = "eigenvalue";
        auto mock = new test::ElementMock{};

        REQUIRE(test::ElementMock::instances().size() == 1);
        member_t member(key, std::unique_ptr<IElement>(mock));
        REQUIRE(test::ElementMock::instances().size() == 1);

        WHEN("its value is modified through its setter")
        {
            auto keyBefore = member.key();

            auto mock2 = new test::ElementMock{};
            member.value(std::unique_ptr<IElement>(mock2));

            THEN("its value changes to the new one")
            {
                REQUIRE(member.value() == mock2);
            }
            THEN("its key does not change")
            {
                REQUIRE(member.key() == keyBefore);
            }
            THEN("its old value was destructed")
            {
                REQUIRE(test::ElementMock::instances().size() == 1);
                REQUIRE(&test::ElementMock::last_instance() == mock2);
            }
        }
    }
}
