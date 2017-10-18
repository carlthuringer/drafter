//
//  test/refract/data/test-Element.cc
//  test-librefract
//
//  Created by Thomas Jandecka on 27/08/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "catch.hpp"

#include "refract/Element.h"
#include "ElementMock.h"

using namespace refract;
using namespace data;

namespace
{
    struct DataMock : tracked<DataMock> {
        using DataType = int;

        static const char* name;

        DataType foo = 42;

        DataMock() = default;
        DataMock(DataType val) : foo(std::move(val))
        {
        }

        DataMock(const DataMock&) = default;
        DataMock(DataMock&&) = default;

        DataMock& operator=(const DataMock&) = default;
        DataMock& operator=(DataMock&&) = default;

        ~DataMock() = default;
    };
    const char* DataMock::name = "{data-mock}";

    struct CollectionMock : tracked<CollectionMock> {
        using DataType = std::vector<int>;

        static const char* name;

        DataType foo = { 349, 8935, 33 };

        CollectionMock() = default;
        CollectionMock(DataType val) : foo(std::move(val))
        {
        }

        CollectionMock(std::initializer_list<int> val) : foo(std::move(val))
        {
        }

        CollectionMock(const CollectionMock&) = default;
        CollectionMock(CollectionMock&&) = default;

        CollectionMock& operator=(const CollectionMock&) = default;
        CollectionMock& operator=(CollectionMock&&) = default;

        ~CollectionMock() = default;

        mutable int _total_ctx = 0;

        mutable int begin_const_ctx = 0;
        const auto begin() const noexcept
        {
            ++_total_ctx;
            ++begin_const_ctx;
            return foo.begin();
        }

        int begin_ctx = 0;
        const auto begin() noexcept
        {
            ++_total_ctx;
            ++begin_ctx;
            return foo.begin();
        }

        mutable int end_const_ctx = 0;
        const auto end() const noexcept
        {
            ++_total_ctx;
            ++end_const_ctx;
            return foo.end();
        }

        int end_ctx = 0;
        const auto end() noexcept
        {
            ++_total_ctx;
            ++end_ctx;
            return foo.end();
        }

        mutable int empty_ctx = 0;
        mutable bool empty_out = false;
        const auto empty() const noexcept
        {
            ++_total_ctx;
            ++empty_ctx;
            return empty_out;
        }

        mutable int size_ctx = 0;
        bool size_out = foo.size();
        const auto size() const noexcept
        {
            ++_total_ctx;
            ++size_ctx;
            return size_out;
        }
    };
    const char* CollectionMock::name = "{collection-mock}";
}

SCENARIO("`isReserved` identifies reserved type names", "[Element]")
{
    WHEN("it is invoked with an `array` literal") {
        bool result = isReserved("array");
        THEN("the result is `true`") {
            REQUIRE(result);
        }
    }
    WHEN("it is invoked with an `boolean` literal") {
        bool result = isReserved("boolean");
        THEN("the result is `true`") {
            REQUIRE(result);
        }
    }
    WHEN("it is invoked with an `enum` literal") {
        bool result = isReserved("enum");
        THEN("the result is `true`") {
            REQUIRE(result);
        }
    }
    WHEN("it is invoked with an `extend` literal") {
        bool result = isReserved("extend");
        THEN("the result is `true`") {
            REQUIRE(result);
        }
    }
    WHEN("it is invoked with an `generic` literal") {
        bool result = isReserved("generic");
        THEN("the result is `true`") {
            REQUIRE(result);
        }
    }
    WHEN("it is invoked with an `member` literal") {
        bool result = isReserved("member");
        THEN("the result is `true`") {
            REQUIRE(result);
        }
    }
    WHEN("it is invoked with an `null` literal") {
        bool result = isReserved("null");
        THEN("the result is `true`") {
            REQUIRE(result);
        }
    }
    WHEN("it is invoked with an `number` literal") {
        bool result = isReserved("number");
        THEN("the result is `true`") {
            REQUIRE(result);
        }
    }
    WHEN("it is invoked with an `object` literal") {
        bool result = isReserved("object");
        THEN("the result is `true`") {
            REQUIRE(result);
        }
    }
    WHEN("it is invoked with an `option` literal") {
        bool result = isReserved("option");
        THEN("the result is `true`") {
            REQUIRE(result);
        }
    }
    WHEN("it is invoked with an `ref` literal") {
        bool result = isReserved("ref");
        THEN("the result is `true`") {
            REQUIRE(result);
        }
    }
    WHEN("it is invoked with an `select` literal") {
        bool result = isReserved("select");
        THEN("the result is `true`") {
            REQUIRE(result);
        }
    }
    WHEN("it is invoked with an `string` literal") {
        bool result = isReserved("string");
        THEN("the result is `true`") {
            REQUIRE(result);
        }
    }
}

SCENARIO("Elements can be cloned with refract::clone(const IElement&)", "[Element]")
{
    GIVEN("An ElementMock")
    {
        auto mock = std::make_unique<test::ElementMock>();
        auto cloneMockOwner = std::make_unique<test::ElementMock>();
        auto cloneMock = cloneMockOwner.get();
        mock->clone_out = std::move(cloneMockOwner);

        WHEN("it is cloned with cValue | cElement")
        {
            auto c = refract::clone(*mock, IElement::cValue | IElement::cElement);

            THEN("there was one call to its methods")
            {
                REQUIRE(mock->_total_ctx == 1);
            }

            THEN("its clone method is invoked once")
            {
                REQUIRE(mock->clone_ctx == 1);
            }

            THEN("its clone method is invoked with cValue | cElement")
            {
                REQUIRE(mock->clone_in == (IElement::cValue | IElement::cElement));
            }

            THEN("its clone method's mocked return value is the one returned from clone")
            {
                REQUIRE(cloneMock == c.get());
            }

            THEN("there were no calls to methods of the clone")
            {
                REQUIRE(c->_total_ctx == 0);
            }
        }
    }
}

SCENARIO("Elements can be cloned with Element<T>::clone(int)", "[Element]")
{
    GIVEN("An Element<DataMock> constructed from DataMock&& with a custom name, one attribute and two meta entries")
    {
        REQUIRE(DataMock::instances().size() == 0);
        Element<DataMock> element(DataMock(12453));
        element.element("how do you turn this on?");

        auto meta1key = "id";
        auto meta1value = new test::ElementMock();
        auto meta1clone = new test::ElementMock();
        meta1clone->_value = 5490;
        meta1value->clone_out.reset(meta1clone);
        element.meta().set(meta1key, std::unique_ptr<IElement>(meta1value));

        auto meta2key = "od";
        auto meta2value = new test::ElementMock();
        auto meta2clone = new test::ElementMock();
        meta2clone->_value = 8903;
        meta2value->clone_out.reset(meta2clone);
        element.meta().set(meta2key, std::unique_ptr<IElement>(meta2value));

        auto attr1key = "id";
        auto attr1value = new test::ElementMock();
        auto attr1clone = new test::ElementMock();
        attr1clone->_value = 5490;
        attr1value->clone_out.reset(attr1clone);
        element.attributes().set(attr1key, std::unique_ptr<IElement>(attr1value));

        REQUIRE(element.meta().size() == 2);
        REQUIRE(element.attributes().size() == 1);
        REQUIRE(!element.empty());

        REQUIRE(DataMock::instances().size() == 1);
        REQUIRE(test::ElementMock::instances().size() == 6);

        WHEN("it is cloned without parameter")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(element));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are of same size")
            {
                REQUIRE(element.meta().size() == clone->meta().size());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1);
                REQUIRE(meta1value->clone_ctx == 1);
                REQUIRE(meta1value->clone_in == IElement::cAll);
                REQUIRE(meta1clone == clone->meta().find(meta1key)->second.get());

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cAll")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(element, IElement::cAll));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are of same size")
            {
                REQUIRE(element.meta().size() == clone->meta().size());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1);
                REQUIRE(meta1value->clone_ctx == 1);
                REQUIRE(meta1value->clone_in == IElement::cAll);
                REQUIRE(meta1clone == clone->meta().find(meta1key)->second.get());
                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cMeta")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(element, IElement::cMeta));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are of same size")
            {
                REQUIRE(element.meta().size() == clone->meta().size());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1);
                REQUIRE(meta1value->clone_ctx == 1);
                REQUIRE(meta1value->clone_in == IElement::cAll);
                REQUIRE(meta1clone == clone->meta().find(meta1key)->second.get());

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cAttributes")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(element, IElement::cAttributes));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cValue")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(element, IElement::cValue));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cElement")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(element, IElement::cElement));
            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cNoMetaId")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(element, IElement::cNoMetaId));
            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cMeta | cAttributes")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cMeta | IElement::cAttributes));
            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are of same size")
            {
                REQUIRE(element.meta().size() == clone->meta().size());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1);
                REQUIRE(meta1value->clone_ctx == 1);
                REQUIRE(meta1value->clone_in == IElement::cAll);
                REQUIRE(meta1clone == clone->meta().find(meta1key)->second.get());

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cMeta | cValue")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cMeta | IElement::cValue));
            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are of same size")
            {
                REQUIRE(element.meta().size() == clone->meta().size());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1);
                REQUIRE(meta1value->clone_ctx == 1);
                REQUIRE(meta1value->clone_in == IElement::cAll);
                REQUIRE(meta1clone == clone->meta().find(meta1key)->second.get());

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cMeta | cElement")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cMeta | IElement::cElement));
            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are of same size")
            {
                REQUIRE(element.meta().size() == clone->meta().size());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1);
                REQUIRE(meta1value->clone_ctx == 1);
                REQUIRE(meta1value->clone_in == IElement::cAll);
                REQUIRE(meta1clone == clone->meta().find(meta1key)->second.get());

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cMeta | cNoMetaId")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cMeta | IElement::cNoMetaId));

            REQUIRE(element.meta().find("id") != element.meta().end());
            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta is one element smaller")
            {
                REQUIRE(element.meta().size() - 1 == clone->meta().size());
            }
            THEN("the clone's meta does not contain an `id` key")
            {
                REQUIRE(clone->meta().find("id") == clone->meta().end());
            }
            THEN("the clone's meta are a transformation of clone from the original values minus the `id` entry")
            {
                REQUIRE(meta1value->_total_ctx == 1); // TODO change this to `0` once no intermediate copy is created

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cAttributes | cValue")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cAttributes | IElement::cValue));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cAttributes | cElement")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cAttributes | IElement::cElement));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        ///
        WHEN("it is cloned with cAttributes | cNoMetaId")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cAttributes | IElement::cNoMetaId));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cValue | cElement")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cValue | IElement::cElement));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cValue | cNoMetaId")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cValue | IElement::cNoMetaId));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cElement | cNoMetaId")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cElement | IElement::cNoMetaId));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cMeta | cAttributes | cValue")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cMeta | IElement::cAttributes | IElement::cValue));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are of same size")
            {
                REQUIRE(element.meta().size() == clone->meta().size());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1);
                REQUIRE(meta1value->clone_ctx == 1);
                REQUIRE(meta1value->clone_in == IElement::cAll);
                REQUIRE(meta1clone == clone->meta().find(meta1key)->second.get());

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cMeta | cAttributes | cElement")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cMeta | IElement::cAttributes | IElement::cElement));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are of same size")
            {
                REQUIRE(element.meta().size() == clone->meta().size());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1);
                REQUIRE(meta1value->clone_ctx == 1);
                REQUIRE(meta1value->clone_in == IElement::cAll);
                REQUIRE(meta1clone == clone->meta().find(meta1key)->second.get());

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cMeta | cAttributes | cNoMetaId")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cMeta | IElement::cAttributes | IElement::cNoMetaId));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta is one element smaller")
            {
                REQUIRE(element.meta().size() - 1 == clone->meta().size());
            }
            THEN("the clone's meta does not contain an `id` key")
            {
                REQUIRE(clone->meta().find("id") == clone->meta().end());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1); // TODO change this to `0` once no intermediate copy is created

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cMeta | cValue | cElement")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cMeta | IElement::cValue | IElement::cElement));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are of same size")
            {
                REQUIRE(element.meta().size() == clone->meta().size());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1);
                REQUIRE(meta1value->clone_ctx == 1);
                REQUIRE(meta1value->clone_in == IElement::cAll);
                REQUIRE(meta1clone == clone->meta().find(meta1key)->second.get());

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cMeta | cValue | cNoMetaId")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cMeta | IElement::cValue | IElement::cNoMetaId));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta is one element smaller")
            {
                REQUIRE(element.meta().size() - 1 == clone->meta().size());
            }
            THEN("the clone's meta does not contain an `id` key")
            {
                REQUIRE(clone->meta().find("id") == clone->meta().end());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1); // TODO change this to `0` once no intermediate copy is created

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cMeta | cElement | cNoMetaId")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cMeta | IElement::cElement | IElement::cNoMetaId));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta is one element smaller")
            {
                REQUIRE(element.meta().size() - 1 == clone->meta().size());
            }
            THEN("the clone's meta does not contain an `id` key")
            {
                REQUIRE(clone->meta().find("id") == clone->meta().end());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1); // TODO change this to `0` once no intermediate copy is created

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cAttributes | cValue | cElement")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cAttributes | IElement::cValue | IElement::cElement));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cAttributes | cValue | cNoMetaId")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cAttributes | IElement::cValue | IElement::cNoMetaId));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cAttributes | cElement | cNoMetaId")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(
                element, IElement::cAttributes | IElement::cElement | IElement::cNoMetaId));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cValue | cElement | cNoMetaId")
        {
            std::unique_ptr<Element<DataMock> > clone(
                refract::clone(element, IElement::cValue | IElement::cElement | IElement::cNoMetaId));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cMeta | cAttributes | cValue | cElement")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(
                element, IElement::cMeta | IElement::cAttributes | IElement::cValue | IElement::cElement));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are of same size")
            {
                REQUIRE(element.meta().size() == clone->meta().size());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1);
                REQUIRE(meta1value->clone_ctx == 1);
                REQUIRE(meta1value->clone_in == IElement::cAll);
                REQUIRE(meta1clone == clone->meta().find(meta1key)->second.get());

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cMeta | cAttributes | cValue | cNoMetaId")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(
                element, IElement::cMeta | IElement::cAttributes | IElement::cValue | IElement::cNoMetaId));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta is one element smaller")
            {
                REQUIRE(element.meta().size() - 1 == clone->meta().size());
            }
            THEN("the clone's meta does not contain an `id` key")
            {
                REQUIRE(clone->meta().find("id") == clone->meta().end());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1); // TODO change this to `0` once no intermediate copy is created

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the default")
            {
                REQUIRE(clone->element() == DataMock::name);
            }
        }

        WHEN("it is cloned with cAttributes | cValue | cElement | cNoMetaId")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(
                element, IElement::cAttributes | IElement::cValue | IElement::cElement | IElement::cNoMetaId));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta are empty")
            {
                REQUIRE(clone->meta().size() == 0);
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        //
        WHEN("it is cloned with cValue | cElement | cNoMetaId | cMeta")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(
                element, IElement::cValue | IElement::cElement | IElement::cNoMetaId | IElement::cMeta));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta is one element smaller")
            {
                REQUIRE(element.meta().size() - 1 == clone->meta().size());
            }
            THEN("the clone's meta does not contain an `id` key")
            {
                REQUIRE(clone->meta().find("id") == clone->meta().end());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1); // TODO change this to `0` once no intermediate copy is created

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are empty")
            {
                REQUIRE(clone->attributes().size() == 0);
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cElement | cNoMetaId | cMeta | cAttributes")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(
                element, IElement::cElement | IElement::cNoMetaId | IElement::cMeta | IElement::cAttributes));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta is one element smaller")
            {
                REQUIRE(element.meta().size() - 1 == clone->meta().size());
            }
            THEN("the clone's meta does not contain an `id` key")
            {
                REQUIRE(clone->meta().find("id") == clone->meta().end());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1); // TODO change this to `0` once no intermediate copy is created

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone is empty")
            {
                REQUIRE(clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }

        WHEN("it is cloned with cElement | cNoMetaId | cMeta | cAttributes | cValue")
        {
            std::unique_ptr<Element<DataMock> > clone(refract::clone(element,
                IElement::cElement | IElement::cNoMetaId | IElement::cMeta | IElement::cAttributes | IElement::cValue));

            THEN("the clone is not nullptr")
            {
                REQUIRE(clone);
            }
            THEN("the clone's meta is one element smaller")
            {
                REQUIRE(element.meta().size() - 1 == clone->meta().size());
            }
            THEN("the clone's meta does not contain an `id` key")
            {
                REQUIRE(clone->meta().find("id") == clone->meta().end());
            }
            THEN("the clone's meta are a transformation of clone from the original values")
            {
                REQUIRE(meta1value->_total_ctx == 1); // FIXME change this to `0` once no intermediate copy is created

                REQUIRE(meta2value->_total_ctx == 1);
                REQUIRE(meta2value->clone_ctx == 1);
                REQUIRE(meta2value->clone_in == IElement::cAll);
                REQUIRE(meta2clone == clone->meta().find(meta2key)->second.get());
            }
            THEN("the clone's attributes are of same size")
            {
                REQUIRE(element.attributes().size() == clone->attributes().size());
            }
            THEN("the clone's attributes are a transformation of clone from the original values")
            {
                REQUIRE(attr1value->_total_ctx == 1);
                REQUIRE(attr1value->clone_ctx == 1);
                REQUIRE(attr1value->clone_in == IElement::cAll);
                REQUIRE(attr1clone == clone->attributes().find(attr1key)->second.get());
            }
            THEN("the clone's content is copied")
            {
                REQUIRE(clone->get().foo == 12453);
                REQUIRE(DataMock::instances().size() == 2);
                REQUIRE(&DataMock::last_instance() == &clone->get());
            }
            THEN("the clone is not empty")
            {
                REQUIRE(!clone->empty());
            }
            THEN("the clone's name is the same as the original's")
            {
                REQUIRE(clone->element() == "how do you turn this on?");
            }
        }
    }
}

SCENARIO("Elements can be constructed", "[Element]")
{
    GIVEN("A default constructed Element<DataMock>")
    {
        Element<DataMock> element;

        THEN("its meta is empty")
        {
            REQUIRE(element.meta().empty());
        }
        THEN("its attributes are empty")
        {
            REQUIRE(element.attributes().empty());
        }
        THEN("a DataMock is default constructed")
        {
            REQUIRE(DataMock::instances().size() == 1);
            REQUIRE(DataMock::last_instance().foo == 42);
        }
        THEN("it is empty")
        {
            REQUIRE(element.empty());
        }

        WHEN("another Element<DataMock> is copy constructed from it")
        {
            Element<DataMock> element2(element);

            THEN("an additional DataMock is constructed")
            {
                REQUIRE(DataMock::instances().size() == 2);
            }
            THEN("its DataMock is default constructed")
            {
                REQUIRE(element2.get().foo == 42);
            }
            THEN("its meta is empty")
            {
                REQUIRE(element2.meta().empty());
            }
            THEN("its attributes are empty")
            {
                REQUIRE(element2.attributes().empty());
            }
            THEN("it is empty")
            {
                REQUIRE(element2.empty());
            }
        }
    }

    GIVEN("An Element<DataMock> constructed from DataMock&&")
    {
        REQUIRE(DataMock::instances().size() == 0);

        Element<DataMock> element(DataMock(12453));

        THEN("its meta is empty")
        {
            REQUIRE(element.meta().empty());
        }
        THEN("its attributes are empty")
        {
            REQUIRE(element.attributes().empty());
        }
        THEN("only one DataMock is constructed")
        {
            REQUIRE(DataMock::instances().size() == 1);
        }
        THEN("it is not empty")
        {
            REQUIRE(!element.empty());
        }
        THEN("its content is the same as the reference")
        {
            REQUIRE(element.get().foo == 12453);
        }
    }

    GIVEN("An Element<CollectionMock> constructed from CollectionMock&&")
    {
        REQUIRE(CollectionMock::instances().size() == 0);
        Element<CollectionMock> element(CollectionMock{ 3, 5, 8 });

        THEN("its meta is empty")
        {
            REQUIRE(element.meta().empty());
        }
        THEN("its attributes are empty")
        {
            REQUIRE(element.attributes().empty());
        }
        THEN("only one CollectionMock is constructed")
        {
            REQUIRE(CollectionMock::instances().size() == 1);
        }
        THEN("it is not empty")
        {
            REQUIRE(!element.empty());
        }
        THEN("its content is the same as the reference")
        {
            REQUIRE(element.get().foo[0] == 3);
            REQUIRE(element.get().foo[1] == 5);
            REQUIRE(element.get().foo[2] == 8);
        }
        THEN("it is not empty if the collection is")
        {
            CollectionMock::last_instance().empty_out = true;
            REQUIRE(!element.empty());
        }
        THEN("it is not empty if the collection is not")
        {
            CollectionMock::last_instance().empty_out = false;
            REQUIRE(!element.empty());
        }
    }
}
