//
//  test/refract/data/test-Array.cc
//  test-librefract
//
//  Created by Thomas Jandecka on 27/08/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "catch.hpp"

#include "refract/data/Array.h"
#include "ElementMock.h"

using namespace refract;
using namespace experimental;
using namespace data;

TEST_CASE("`array_t`'s default element name is `array`", "[Element][array_t]")
{
    REQUIRE(std::string(array_t::name) == "array");
}

SCENARIO("`array_t` is default constructed and both copy- and move constructed from", "[ElementData][array_t]")
{
    GIVEN("A default initialized array_t")
    {
        array_t array;

        THEN("it is empty")
        {
            REQUIRE(array.empty());
        }

        THEN("its size is zero")
        {
            REQUIRE(array.size() == 0);
        }

        THEN("its begin is equal to its end")
        {
            REQUIRE(array.begin() == array.end());
        }

        WHEN("from it another array_t is copy constructed")
        {
            array_t array2(array);

            THEN("the latter array_t is also empty")
            {
                REQUIRE(array2.empty());
            }
        }

        WHEN("from it another array_t is copy constructed")
        {
            array_t array2(std::move(array));

            THEN("the original array_t is empty")
            {
                REQUIRE(array2.empty());
            }

            THEN("the latter array_t is also empty")
            {
                REQUIRE(array2.empty());
            }
        }
    }
}

SCENARIO("`array_t` is inserted to and erased from", "[ElementData][array_t]")
{
    GIVEN("A default initialized `array_t`")
    {
        array_t array;

        WHEN("an ElementMock is pushed back")
        {
            auto mock = std::make_unique<test::ElementMock>();
            auto mock1ptr = mock.get();

            REQUIRE(test::ElementMock::instances().size() == 1);

            array.push_back(std::move(mock));

            THEN("its size is 1")
            {
                REQUIRE(array.size() == 1);
            }

            THEN("it is not empty")
            {
                REQUIRE(!array.empty());
            }

            THEN("its begin points to the mock")
            {
                REQUIRE((*array.begin()).get() == mock1ptr);
            }

            THEN("there were no method calls to the mock")
            {
                REQUIRE(mock1ptr->_total_ctx == 0);
            }

            THEN("there still is just one mock instance")
            {
                REQUIRE(test::ElementMock::instances().size() == 1);
            }

            WHEN("another three ElementMocks are inserted at begin two at end")
            {
                auto mock2 = std::make_unique<test::ElementMock>();
                auto mock2ptr = mock.get();

                auto mock3 = std::make_unique<test::ElementMock>();
                auto mock3ptr = mock.get();

                auto mock4 = std::make_unique<test::ElementMock>();
                auto mock4ptr = mock.get();

                array.insert(array.begin(), std::move(mock2));
                array.insert(array.begin(), std::move(mock3));
                array.insert(array.begin(), std::move(mock4));

                auto mock5 = std::make_unique<test::ElementMock>();
                auto mock5ptr = mock.get();

                auto mock6 = std::make_unique<test::ElementMock>();
                auto mock6ptr = mock.get();

                array.insert(array.begin(), std::move(mock5));
                array.insert(array.begin(), std::move(mock6));

                REQUIRE(test::ElementMock::instances().size() == 6);

                THEN("its size is 6")
                {
                    REQUIRE(array.size() == 6);
                }

                WHEN("the third Element is erased")
                {
                    auto it_ = array.begin();
                    std::advance(it_, 2);
                    auto it = array.erase(it_);

                    THEN("its size is 5")
                    {
                        REQUIRE(array.size() == 5);
                    }

                    THEN("the resulting iterator points at the newly third element")
                    {
                        REQUIRE(it == std::next(array.begin(), 2));
                    }

                    THEN("only 5 instances of ElementMock exist")
                    {
                        REQUIRE(test::ElementMock::instances().size() == 5);
                    }

                    THEN("none of the existing instances of ElementMock is the erased Element")
                    {
                        const auto& instances = test::ElementMock::instances();
                        REQUIRE(std::find_if(instances.begin(), instances.end(), [mock5ptr](const test::ElementMock* el) {
                            assert(el);
                            return el == mock5ptr;
                        }) == instances.end());
                    }
                }

                WHEN("all but the first Element are erased")
                {
                    auto it = array.erase(std::next(array.begin()), array.end());

                    THEN("its size is 1")
                    {
                        REQUIRE(array.size() == 1);
                    }

                    THEN("the resulting iterator is end")
                    {
                        REQUIRE(it == array.end());
                    }
                }
            }

            WHEN("another ElementMock is pushed back")
            {
                auto mock2 = std::make_unique<test::ElementMock>();
                auto mock2ptr = mock2.get();

                REQUIRE(test::ElementMock::instances().size() == 2);

                array.push_back(std::move(mock2));

                THEN("its size is 2")
                {
                    REQUIRE(array.size() == 2);
                }

                THEN("the begin is 2 away from the end")
                {
                    REQUIRE(array.end() - array.begin() == 2);
                }

                THEN("it is not empty")
                {
                    REQUIRE(!array.empty());
                }

                THEN("its first two elements are the given mocks")
                {
                    REQUIRE(array.begin()[0].get() == mock1ptr);
                    REQUIRE(array.begin()[1].get() == mock2ptr);
                }

                THEN("there were no method calls to the mocks")
                {
                    REQUIRE(mock1ptr->_total_ctx == 0);
                    REQUIRE(mock2ptr->_total_ctx == 0);
                }

                THEN("there still are just two mock instances")
                {
                    REQUIRE(test::ElementMock::instances().size() == 2);
                }

                THEN("it is iterable")
                {
                    auto mocks = std::array<IElement*, 2>{ mock1ptr, mock2ptr };
                    auto mocks_it = mocks.begin();
                    int ctx = 0;
                    for (const auto& el : array) {
                        REQUIRE(mocks_it < mocks.end()); // memory overflow
                        REQUIRE(*mocks_it == el.get());
                        ++mocks_it;
                        ++ctx;
                        REQUIRE(ctx != 0); // integer overflow
                    }
                    REQUIRE(ctx == 2);
                }
            }
        }
    }
}

SCENARIO("`array_t` is constructed from elements, copy- and move constructed from and all its copies are destroyed",
    "[ElementData][array_t]")
{
    GIVEN("Three ElementMock instances")
    {
        REQUIRE(test::ElementMock::instances().size() == 0);

        auto mock1 = std::make_unique<test::ElementMock>();
        auto mock2 = std::make_unique<test::ElementMock>();
        auto mock3 = std::make_unique<test::ElementMock>();

        const auto mock1ptr = mock1.get();
        const auto mock2ptr = mock2.get();
        const auto mock3ptr = mock3.get();

        REQUIRE(test::ElementMock::instances().size() == 3);

        WHEN("An array_t is constructed from iterators to an STL container composed of the mocks")
        {
            mock1->clone_out = std::make_unique<test::ElementMock>();
            mock2->clone_out = std::make_unique<test::ElementMock>();
            mock3->clone_out = std::make_unique<test::ElementMock>();

            const auto mock1clone = mock1->clone_out.get();
            const auto mock2clone = mock2->clone_out.get();
            const auto mock3clone = mock3->clone_out.get();

            REQUIRE(test::ElementMock::instances().size() == 6);

            const auto mocks
                = std::array<std::unique_ptr<IElement>, 3>{ std::move(mock1), std::move(mock2), std::move(mock3) };

            array_t array(mocks.begin(), mocks.end());

            THEN("its size is three")
            {
                REQUIRE(array.size() == 3);
            }
            THEN("its begin is 3 away from its end")
            {
                REQUIRE(array.end() - array.begin() == 3);
            }
            THEN("it is not empty")
            {
                REQUIRE(!array.empty());
            }
            THEN("it is iterable")
            {
                auto mock_clones = std::array<const IElement*, 3>{ mock1clone, mock2clone, mock3clone };
                auto mock_clones_it = mock_clones.begin();
                int ctx = 0;
                for (const auto& el : array) {
                    REQUIRE(mock_clones_it < mock_clones.end()); // memory overflow
                    REQUIRE(*mock_clones_it == el.get());
                    ++mock_clones_it;
                    ++ctx;
                    REQUIRE(ctx != 0); // integer overflow
                }
                REQUIRE(ctx == 3);
            }
            THEN("non-temporary ElementMocks were not constructed")
            {
                REQUIRE(test::ElementMock::instances().size() == 6);
            }
            THEN("its members were obtained calling `IElement::clone(cAll)` on original mocks")
            {
                REQUIRE(mock1ptr->_total_ctx == 1);
                REQUIRE(mock2ptr->_total_ctx == 1);
                REQUIRE(mock3ptr->_total_ctx == 1);

                REQUIRE(mock1ptr->clone_ctx == 1);
                REQUIRE(mock2ptr->clone_ctx == 1);
                REQUIRE(mock3ptr->clone_ctx == 1);

                REQUIRE(mock1ptr->clone_in == IElement::cAll);
                REQUIRE(mock2ptr->clone_in == IElement::cAll);
                REQUIRE(mock3ptr->clone_in == IElement::cAll);

                REQUIRE(array.begin()[0].get() == mock1clone);
                REQUIRE(array.begin()[1].get() == mock2clone);
                REQUIRE(array.begin()[2].get() == mock3clone);
            }
        }

        WHEN("an array_t is created from them")
        {
            {
                array_t array;
                array.push_back(std::move(mock1));
                array.push_back(std::move(mock2));
                array.push_back(std::move(mock3));

                THEN("its size is three")
                {
                    REQUIRE(array.size() == 3);
                }
                THEN("its begin is 3 away from its end")
                {
                    REQUIRE(array.end() - array.begin() == 3);
                }
                THEN("it is not empty")
                {
                    REQUIRE(!array.empty());
                }
                THEN("its first three elements are the given mocks")
                {
                    REQUIRE(array.begin()[0].get() == mock1ptr);
                    REQUIRE(array.begin()[1].get() == mock2ptr);
                    REQUIRE(array.begin()[2].get() == mock3ptr);
                }
                THEN("it is iterable")
                {
                    auto mocks = std::array<IElement*, 3>{ mock1ptr, mock2ptr, mock3ptr };
                    auto mocks_it = mocks.begin();
                    int ctx = 0;
                    for (const auto& el : array) {
                        REQUIRE(mocks_it < mocks.end()); // memory overflow
                        REQUIRE(*mocks_it == el.get());
                        ++mocks_it;
                        ++ctx;
                        REQUIRE(ctx != 0); // integer overflow
                    }
                    REQUIRE(ctx == 3);
                }
                THEN("non-temporary ElementMocks were not constructed")
                {
                    REQUIRE(test::ElementMock::instances().size() == 3);
                }
                THEN("nothing was called on the mocks")
                {
                    REQUIRE(mock1ptr->_total_ctx == 0);
                    REQUIRE(mock2ptr->_total_ctx == 0);
                    REQUIRE(mock3ptr->_total_ctx == 0);
                }

                WHEN("another array_t is copy constructed from it")
                {
                    auto mock1clone = new test::ElementMock{};
                    auto mock2clone = new test::ElementMock{};
                    auto mock3clone = new test::ElementMock{};
                    REQUIRE(test::ElementMock::instances().size() == 6);

                    mock1ptr->clone_out.reset(mock1clone);
                    mock2ptr->clone_out.reset(mock2clone);
                    mock3ptr->clone_out.reset(mock3clone);
                    REQUIRE(test::ElementMock::instances().size() == 6);

                    array_t array2(array);

                    THEN("its size is three")
                    {
                        REQUIRE(array2.size() == 3);
                    }
                    THEN("its begin is 3 away from its end")
                    {
                        REQUIRE(array2.end() - array2.begin() == 3);
                    }
                    THEN("the original's size is three")
                    {
                        REQUIRE(array.size() == 3);
                    }
                    THEN("it is not empty")
                    {
                        REQUIRE(!array2.empty());
                    }
                    THEN("the original is not empty")
                    {
                        REQUIRE(!array.empty());
                    }
                    THEN("its size equals the original's size")
                    {
                        REQUIRE(array.size() == array2.size());
                    }
                    THEN("it is iterable")
                    {
                        auto mocks = std::array<IElement*, 3>{ mock1clone, mock2clone, mock3clone };
                        auto mocks_it = mocks.begin();
                        int ctx = 0;
                        for (const auto& el : array2) {
                            REQUIRE(mocks_it < mocks.end()); // memory overflow
                            REQUIRE(*mocks_it == el.get());
                            ++mocks_it;
                            ++ctx;
                            REQUIRE(ctx != 0); // integer overflow
                        }
                        REQUIRE(ctx == 3);
                    }
                    THEN("non-temporary ElementMocks were not constructed")
                    {
                        REQUIRE(test::ElementMock::instances().size() == 6);
                    }
                    THEN("its members were obtained calling `IElement::clone(cAll)` on original mocks")
                    {
                        REQUIRE(mock1ptr->_total_ctx == 1);
                        REQUIRE(mock2ptr->_total_ctx == 1);
                        REQUIRE(mock3ptr->_total_ctx == 1);

                        REQUIRE(mock1ptr->clone_ctx == 1);
                        REQUIRE(mock2ptr->clone_ctx == 1);
                        REQUIRE(mock3ptr->clone_ctx == 1);

                        REQUIRE(mock1ptr->clone_in == IElement::cAll);
                        REQUIRE(mock2ptr->clone_in == IElement::cAll);
                        REQUIRE(mock3ptr->clone_in == IElement::cAll);

                        REQUIRE(array2.begin()[0].get() == mock1clone);
                        REQUIRE(array2.begin()[1].get() == mock2clone);
                        REQUIRE(array2.begin()[2].get() == mock3clone);
                    }
                }

                WHEN("another array_t is move constructed from it")
                {
                    {
                        array_t array2(std::move(array));

                        THEN("its size is three")
                        {
                            REQUIRE(array2.size() == 3);
                        }
                        THEN("its begin is 3 away from its end")
                        {
                            REQUIRE(array2.end() - array2.begin() == 3);
                        }
                        THEN("the original's begin is at its end")
                        {
                            REQUIRE(array.end() == array.begin());
                        }
                        THEN("the original's size is zero")
                        {
                            REQUIRE(array.size() == 0);
                        }
                        THEN("the original is empty")
                        {
                            REQUIRE(array.empty());
                        }
                        THEN("it is not empty")
                        {
                            REQUIRE(!array2.empty());
                        }
                        THEN("it is iterable")
                        {
                            auto mocks = std::array<IElement*, 3>{ mock1ptr, mock2ptr, mock3ptr };
                            auto mocks_it = mocks.begin();
                            int ctx = 0;
                            for (const auto& el : array2) {
                                REQUIRE(mocks_it < mocks.end()); // memory overflow
                                REQUIRE(*mocks_it == el.get());
                                ++mocks_it;
                                ++ctx;
                                REQUIRE(ctx != 0); // integer overflow
                            }
                            REQUIRE(ctx == 3);
                        }
                        THEN("non-temporary ElementMocks were not constructed")
                        {
                            REQUIRE(test::ElementMock::instances().size() == 3);
                        }
                        THEN("its members are the original mocks")
                        {
                            REQUIRE(array2.begin()[0].get() == mock1ptr);
                            REQUIRE(array2.begin()[1].get() == mock2ptr);
                            REQUIRE(array2.begin()[2].get() == mock3ptr);
                        }
                    }
                    WHEN("it goes out of scope")
                    {
                        THEN("no mocks are left")
                        {
                            REQUIRE(test::ElementMock::instances().empty());
                        }
                    }
                }
            }
            WHEN("it goes out of scope")
            {
                THEN("no mocks are left")
                {
                    REQUIRE(test::ElementMock::instances().empty());
                }
            }
        }
    }
}
