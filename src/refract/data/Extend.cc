//
//  refract/data/Extend.cc
//  librefract
//
//  Created by Thomas Jandecka on 04/09/2017
//  Copyright (c) 2017 Apiary Inc. All rights reserved.
//

#include "Extend.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <set>

#include "../Exception.h"
#include "../Element.h"
#include "../InfoElements.h"
#include "../TypeQueryVisitor.h"

using namespace refract;
using namespace data;

const char* extend_t::name = "extend";

static_assert(supports_erase<extend_t>::value);
static_assert(supports_empty<extend_t>::value);
static_assert(supports_insert<extend_t>::value);
static_assert(supports_push_back<extend_t>::value);
static_assert(supports_begin<extend_t>::value);
static_assert(supports_end<extend_t>::value);
static_assert(supports_size<extend_t>::value);
static_assert(supports_erase<extend_t>::value);
static_assert(is_iterable<extend_t>::value);
static_assert(supports_merge<extend_t>::value);

static_assert(!supports_key<extend_t>::value);
static_assert(!supports_value<extend_t>::value);
static_assert(!is_pair<extend_t>::value);

namespace
{
    class ElementMerger
    {
        std::unique_ptr<IElement> result;
        TypeQueryVisitor::ElementType base;

        template <typename T, bool IsContainer = is_iterable<typename T::ValueType>::value>
        struct ValueMerge;

        /**
         * Merge strategy for Primitive types - just replace by latest value */
        template <typename T>
        struct ValueMerge<T, false> {
            void operator()(T& value, const T& merge) const
            {
                value = merge;
            }
        };

        /**
         * Merge stategy for objects/array
         * - if member
         *   - without existing key -> append
         *   - with existing key - replace old value
         * - if not member
         *   - if empty value -> ignore (type holder for array)
         *   - else append
         */
        template <typename T>
        struct ValueMerge<T, true> {

            void operator()(T& value, const T& merge) const
            {
                typedef std::map<std::string, MemberElement*> MapKeyToMember;
                typedef std::map<std::string, RefElement*> MapNameToRef;

                MapKeyToMember keysBase;
                MapNameToRef refBase;

                for (auto& el : value.get()) {
                    if (auto member = TypeQueryVisitor::as<MemberElement>(el.get())) {

                        if (auto key = TypeQueryVisitor::as<const StringElement>(member->get().key())) {
                            keysBase[key->get()] = member;
                        }
                    } else if (auto ref = TypeQueryVisitor::as<RefElement>(el.get())) {
                        refBase[ref->get().name] = ref;
                    }
                }

                for (auto& el : merge.get()) {
                    if (auto member = TypeQueryVisitor::as<const MemberElement>(el.get())) {
                        if (auto key = TypeQueryVisitor::as<const StringElement>(member->get().key())) {

                            auto iKey = keysBase.find(key->get());
                            if (iKey != keysBase.end()) { // key is already presented, replace value
                                iKey->second->get().value(member->get().value()->clone());

                                std::set<std::string> emptySet;
                                InfoMerge<T>(iKey->second->meta(), emptySet)(member->meta());
                                InfoMerge<T>(iKey->second->attributes(), emptySet)(member->attributes());
                            } else { // unknown key, append value
                                auto c = clone(*member);
                                keysBase[key->get()] = c.get();
                                value.get().push_back(std::move(c));
                            }
                        }
                    } else if (auto ref = TypeQueryVisitor::as<const RefElement>(el.get())) {
                        if (refBase.find(ref->get().name) == refBase.end()) {
                            auto c = clone(*ref);
                            refBase[ref->get().symbol()] = c.get();
                            value.get().push_back(std::move(c));
                        }
                    } else if (!el->empty()) { // merge member is not MemberElement, append value
                        value.get().push_back(clone(*el));
                    }
                }
            }
        };

        template <typename T>
        class InfoMerge
        {
            InfoElements& info;
            const std::set<std::string>& noMergeKeys;

        public:
            InfoMerge(InfoElements& info, const std::set<std::string>& noMergeKeys)
                : info(info), noMergeKeys(noMergeKeys)
            {
            }

            void operator()(const InfoElements& append)
            {
                InfoElements toAppend;

                for (const auto& el : append) {
                    assert(el.second);

                    if (noMergeKeys.find(el.first) != noMergeKeys.end()) {
                        continue; // this key should not be merged
                    }

                    info.set(el.first, *el.second);
                }
            }
        };

        /**
         * precondition - target && append element MUST BE of same type
         * we use static_cast<> without checking type this is responsibility of caller
         */
        template <typename T>
        static void doMerge(IElement* target, const IElement* append)
        {
            typedef T ElementType;

            // FIXME: static initialization on C++11
            static std::set<std::string> noMeta;
            static std::set<std::string> noAttributes;

            if (noMeta.empty()) {
                noMeta.insert("id");
                noMeta.insert("prefix");
                noMeta.insert("namespace");
            }

            InfoMerge<T>(target->meta(), noMeta)(append->meta());
            InfoMerge<T>(target->attributes(), noAttributes)(append->attributes());

            ValueMerge<T>{}(static_cast<ElementType&>(*target), static_cast<const ElementType&>(*append));
        }

    public:
        ElementMerger() : result(nullptr), base(TypeQueryVisitor::Unknown) {}

        template <typename E>
        void operator()(const E& e)
        {
            if (!e) {
                return;
            }

            if (!result) {
                result = e->clone();

                TypeQueryVisitor type;
                Visit(type, *result);
                base = type.get();
                return;
            }

            TypeQueryVisitor type;
            VisitBy(*e, type);

            if (type.get() != base) {
                throw refract::LogicError("Can not merge different types of elements");
            }

            switch (base) {
                case TypeQueryVisitor::String:
                    doMerge<StringElement>(result.get(), e.get());
                    return;

                case TypeQueryVisitor::Number:
                    doMerge<NumberElement>(result.get(), e.get());
                    return;

                case TypeQueryVisitor::Boolean:
                    doMerge<BooleanElement>(result.get(), e.get());
                    return;

                case TypeQueryVisitor::Array:
                    doMerge<ArrayElement>(result.get(), e.get());
                    return;

                case TypeQueryVisitor::Object:
                    doMerge<ObjectElement>(result.get(), e.get());
                    return;

                case TypeQueryVisitor::Enum:
                    doMerge<EnumElement>(result.get(), e.get());
                    return;

                case TypeQueryVisitor::Ref:
                    doMerge<RefElement>(result.get(), e.get());
                    return;

                case TypeQueryVisitor::Member:
                case TypeQueryVisitor::Extend:
                case TypeQueryVisitor::Null:
                    throw LogicError("Unappropriate kind of element to merging");
                default:
                    throw LogicError("Element has no implemented merging");
            }
        }

        operator std::unique_ptr<IElement>()
        {
            return std::move(result);
        }
    };
}

extend_t::extend_t(extend_t::container_type elements) : elements_(std::move(elements))
{
    for (const auto& e : elements_)
        assert(e);
}

extend_t::extend_t(const extend_t& other) : elements_()
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

extend_t::iterator extend_t::insert(extend_t::const_iterator it, std::unique_ptr<IElement> el)
{
    assert(it >= begin());
    assert(it <= end());
    assert(el);

    if (end() - begin() > 0) {
        if (typeid(*el) != typeid(elements_.front())) {
            throw LogicError("ExtendElement must be composed from Elements of same type");
        }
    }

    return elements_.insert(it, std::move(el));
}

extend_t::iterator extend_t::erase(extend_t::const_iterator b, extend_t::const_iterator e)
{
    return elements_.erase(b, e);
}

std::unique_ptr<IElement> extend_t::merge() const
{
    return std::for_each(begin(), end(), ElementMerger());
}
