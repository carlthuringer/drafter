//
//
//  refract/JSONSchemaVisitor.cc
//  librefract
//
//  Created by Vilibald Wanča on 09/11/15.
//  Copyright (c) 2015, 2016 Apiary Inc. All rights reserved.
//

#include "JSONSchemaVisitor.h"

#include "Exception.h"
#include "RenderJSONVisitor.h"
#include "RenderJSONVisitor.h"
#include "SerializeCompactVisitor.h"
#include "VisitorUtils.h"
#include "sosJSON.h"

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

using namespace refract;

namespace
{

    template <typename T, typename Collection>
    void CloneMembers(T& a, const Collection& val)
    {
        for (const auto& value : val) {

            if ((value)->empty()) {
                continue;
            }

            RenderJSONVisitor v;
            Visit(v, *value);
            a.push_back(v.getOwnership());
        }
    }

    template <typename T>
    void IncludeMembers(const T& element, typename T::ValueType& members)
    {
        auto val = GetValue<T>{}(element);

        if (!val || val->empty())
            return;

        for (auto const& value : val->get()) {

            if (!value || value->empty()) {
                continue;
            }

            if (value->element() == "ref") {
                HandleRefWhenFetchingMembers<T>(*value, members, IncludeMembers<T>);
                continue;
            }

            members.push_back(clone(*value));
        }
    }

    bool AllItemsEmpty(const ArrayElement::ValueType& val)
    {
        return std::all_of(val.begin(), val.end(), [](const auto& el) {
            assert(el);
            return el->empty();
        });
    }
}

JSONSchemaVisitor::JSONSchemaVisitor(ObjectElement& pDefinitions, bool _fixed /*= false*/, bool _fixedType /*= false*/)
    : pObj(make_element<ObjectElement>()), pDefs(pDefinitions), fixed(_fixed), fixedType(_fixedType)
{
}

void JSONSchemaVisitor::setFixed(bool _fixed)
{
    fixed = _fixed;
}

void JSONSchemaVisitor::setFixedType(bool _fixedType)
{
    fixedType = _fixedType;
}

template <>
void JSONSchemaVisitor::setPrimitiveType(const BooleanElement& e)
{
    setSchemaType("boolean");
}

template <>
void JSONSchemaVisitor::setPrimitiveType(const StringElement& e)
{
    setSchemaType("string");
}

template <>
void JSONSchemaVisitor::setPrimitiveType(const NumberElement& e)
{
    setSchemaType("number");
}

void JSONSchemaVisitor::setSchemaType(const std::string& type)
{
    addMember("type", make_primitive(type));
}

void JSONSchemaVisitor::addSchemaType(const std::string& type)
{
    // FIXME: this will not work corretly if "type" attribute will already
    // have more members. Need to check if type is it is Array and for
    // already pushed types
    MemberElement* m = FindMemberByKey(*pObj, "type");

    if (m) {
        assert(!m->empty());
        if (auto t = m->get().claim()) {
            auto a = make_element<ArrayElement>();
            a->get().push_back(std::move(t));
            a->get().push_back(make_primitive(type));
            m->get().value(std::move(a));
            return;
        }
    }

    setSchemaType(type);
}

void JSONSchemaVisitor::addNullToEnum()
{
    MemberElement* m = FindMemberByKey(*pObj, "enum");

    if (m && m->get().value()) {
        ArrayElement* a = TypeQueryVisitor::as<ArrayElement>(m->get().value());
        assert(a);
        a->get().push_back(make_empty<NullElement>());
    }
}

void JSONSchemaVisitor::addMember(const std::string& key, std::unique_ptr<IElement> val)
{
    const auto s = pObj->get().size();
    pObj->get().add_member(key, std::move(val));
    assert(pObj->get().size() == s + 1);
}

template <typename T>
void JSONSchemaVisitor::primitiveType(const T& e)
{
    if (auto value = GetValue<T>{}(e)) {
        setPrimitiveType(e);

        if (fixed) {
            auto a = make_element<ArrayElement>();
            a->get().push_back(value->empty() ? //
                    make_element<T>() :
                    make_element<T>(value->get()));
            addMember("enum", std::move(a));
        }
    }
}

void JSONSchemaVisitor::operator()(const IElement& e)
{
    VisitBy(e, *this);
}

void JSONSchemaVisitor::operator()(const MemberElement& e)
{
    JSONSchemaVisitor renderer(pDefs);

    const auto& content = e.get();

    if (auto val = content.value()) {
        if (IsTypeAttribute(e, "fixed") || fixed) {
            renderer.setFixed(true);
        }

        renderer.setFixedType(IsTypeAttribute(e, "fixedType"));
        Visit(renderer, *val);
    }

    const StringElement* str = TypeQueryVisitor::as<const StringElement>(content.key());
    const ExtendElement* ext = TypeQueryVisitor::as<const ExtendElement>(content.key());

    std::unique_ptr<IElement> merged = ext ? ext->get().merge() : nullptr;

    if (merged)
        str = TypeQueryVisitor::as<StringElement>(merged.get());

    if (str) {
        const IElement* desc = GetDescription(e);

        if (desc) {
            renderer.addMember("description", desc->clone());
        }

        if (IsTypeAttribute(e, "nullable")) {
            renderer.addSchemaType("null");
            renderer.addNullToEnum();
        }

        const auto val = content.value();

        // Check for primitive types
        auto s = TypeQueryVisitor::as<const StringElement>(val);
        auto n = TypeQueryVisitor::as<const NumberElement>(val);
        auto b = TypeQueryVisitor::as<const BooleanElement>(val);

        if (val && (s || n || b)) {
            auto defaultIt = val->attributes().find("default");

            if (defaultIt != val->attributes().end()) {
                assert(defaultIt->second);
                renderer.addMember("default", defaultIt->second->clone());
            }
        }

        addMember(str->empty() ? std::string{} : std::string{str->get()}, renderer.getOwnership());
    } else {
        throw LogicError("A property's key in the object is not of type string");
    }
}

std::unique_ptr<ObjectElement> JSONSchemaVisitor::definitionFromVariableProperty(JSONSchemaVisitor& renderer)
{
    auto definition = make_element<ObjectElement>();
    auto& content = definition->get();

    content.push_back(make_element<MemberElement>("type", make_element<StringElement>("object")));

    {
        auto obj = make_element<ObjectElement>();
        obj->get().push_back(make_element<MemberElement>("", renderer.getOwnership()));

        content.push_back(make_element<MemberElement>("patternProperties", std::move(obj)));
    }

    return std::move(definition);
}

std::unique_ptr<ArrayElement> JSONSchemaVisitor::arrayFromProps(std::vector<const MemberElement*>& props)
{
    if (props.empty()) {
        return make_empty<ArrayElement>();

    } else {
        auto result = make_element<ArrayElement>();
        auto& content = result->get();

        for (auto const& prop : props) {

            const StringElement* str = TypeQueryVisitor::as<const StringElement>(prop->get().key());

            if (str) {
                bool fixedType = IsTypeAttribute(*prop, "fixedType");
                JSONSchemaVisitor renderer(pDefs, fixed, fixedType);
                Visit(renderer, *prop->get().value());

                pDefs.get().push_back(
                    make_element<MemberElement>(str->get(), definitionFromVariableProperty(renderer)));

                {
                    using namespace std::string_literals;

                    auto obj = make_element<ObjectElement>();
                    obj->get().push_back(make_element<MemberElement>(
                        "$ref", make_element<StringElement>("#/definitions/" + str->get().get())));
                    content.push_back(std::move(obj));
                }
            }
        }

        return std::move(result);
    }
}

// TODO XXX @tjanc@ issue with next failing test somewhere here
void JSONSchemaVisitor::addVariableProps(std::vector<const MemberElement*>& props, std::unique_ptr<ObjectElement> o)
{
    if (o->empty() && props.size() == 1) {
        const StringElement* str = TypeQueryVisitor::as<const StringElement>(props[0]->get().key());

        if (str) {
            bool fixedType = IsTypeAttribute(*props.front(), "fixedType");
            JSONSchemaVisitor renderer(pDefs, fixed, fixedType);
            Visit(renderer, *props.front()->get().value());

            pDefs.get().push_back(make_element<MemberElement>(str->get(), definitionFromVariableProperty(renderer)));

            addMember("$ref", make_element<StringElement>("#/definitions/" + str->get().get()));
        }
    } else {
        auto a = arrayFromProps(props);

        if (!o->empty()) {
            auto obj = make_element<ObjectElement>();
            obj->get().push_back(make_element<MemberElement>("properties", std::move(o)));
            a->get().push_back(std::move(obj));
        }

        addMember("allOf", std::move(a));
    }
}

void JSONSchemaVisitor::operator()(const ObjectElement& e)
{
    ObjectElement::ValueType val;
    IncludeMembers(e, val);

    ArrayElement::ValueType reqVals;
    std::vector<const MemberElement*> varProps;
    ArrayElement::ValueType oneOfMembers;

    if (IsTypeAttribute(e, "fixed")) {
        fixed = true;
    }

    if (IsTypeAttribute(e, "fixedType")) {
        fixedType = true;
    }

    {
        auto o = make_empty<ObjectElement>();

        processMembers(val, reqVals, varProps, oneOfMembers, *o);

        if (!varProps.empty()) {
            addVariableProps(varProps, std::move(o));
        } else {
            setSchemaType("object");
            addMember("properties", std::move(o)); // TODO XXX @tjanc@ HIT 2
        }
    }

    if (!reqVals.empty()) {
        addMember("required", make_element<ArrayElement>(std::move(reqVals)));
    }

    if (!oneOfMembers.empty()) {
        addMember("oneOf", make_element<ArrayElement>(std::move(oneOfMembers)));
    }

    if (fixed || fixedType) {
        addMember("additionalProperties", make_primitive(false));
    }
}

void JSONSchemaVisitor::anyOf(
    std::map<std::string, std::vector<const IElement*> >& types, std::vector<std::string>& typesOrder)
{
    if (typesOrder.empty()) {
        addMember("anyOf", make_empty<ArrayElement>());

    } else {
        auto a = make_element<ArrayElement>();

        for (auto const& item : typesOrder) {

            const auto& items = types[item];

            const IElement* elm = items.front();
            JSONSchemaVisitor v(pDefs);
            Visit(v, *elm);

            if (TypeQueryVisitor::as<EnumElement>(elm)) {
                v.addMember("enum", elm->clone());
            } else if (!TypeQueryVisitor::as<ObjectElement>(elm)) {
                data::array_t enmVals;

                CloneMembers(enmVals, items);

                if (!enmVals.empty()) {
                    v.addMember("enum", make_element<ArrayElement>(std::move(enmVals)));
                }
            }

            a->get().push_back(v.getOwnership());
        }
        addMember("anyOf", std::move(a));
    }
}

void JSONSchemaVisitor::operator()(const ArrayElement& e)
{
    const ArrayElement* v = GetValue<ArrayElement>{}(e);

    if (!v)
        return;
    if (v->empty())
        return;

    const auto& val = v->get();
    if (val.empty())
        return;

    JSONSchemaVisitor renderer(pDefs);
    setSchemaType("array");

    if (IsTypeAttribute(e, "fixed")) {
        fixed = true;
    }

    if (IsTypeAttribute(e, "fixedType")) {
        fixedType = true;
    }

    if (fixed || fixedType) {
        data::array_t av;
        bool allEmpty = AllItemsEmpty(val);

        for (auto const& value : val) {

            assert(value);
            if (!value) {
                continue;
            }

            // if all items are just type items then we
            // want them in the schema, otherwise skip
            // empty ones
            if (allEmpty || !value->empty()) {
                JSONSchemaVisitor v(pDefs, fixed);
                Visit(v, *value);
                av.push_back(v.getOwnership());
            }
        }

        if (!av.empty()) {
            if (av.size() == 1) {
                addMember("items", std::move(av.begin()[0]));

                assert(av.size() == 1);
                assert(!av.begin()[0]);

                av.clear();
            } else {
                addMember("items", make_element<ArrayElement>(std::move(av)));
            }
        }
    }

    const ArrayElement* def = GetDefault(e);

    if (def && !def->empty()) {
        addMember("default", clone(*def));
    }
}

void JSONSchemaVisitor::operator()(const EnumElement& e)
{

    std::vector<const IElement*> elms;

    const auto& it = e.attributes().find("enumerations");
    if (it != e.attributes().end()) {
        if (const ArrayElement* enums = TypeQueryVisitor::as<const ArrayElement>(it->second.get())) {
            std::transform(enums->get().begin(),
                enums->get().end(),
                std::back_inserter(elms),
                [](const std::unique_ptr<IElement>& el) { return el.get(); });
        }
    }

    if (!e.empty())
        if (auto v = e.get().value()) {
            elms.push_back(v);
        }

    if (elms.empty()) {
        return;
    }

    std::map<std::string, std::vector<const IElement*> > types;
    std::vector<std::string> typesOrder;

    for (const auto& enumeration : elms) {

        if (enumeration) {
            auto& items = types[enumeration->element()];

            if (items.empty()) {
                typesOrder.push_back(enumeration->element());
            }

            items.push_back(enumeration);
        }
    }

    if (types.size() > 1) {
        anyOf(types, typesOrder);
    } else {
        const EnumElement* def = GetDefault(e);
        if (!elms.empty() || (def && !def->empty())) {
            auto a = make_element<ArrayElement>();
            CloneMembers(a->get(), elms);
            setSchemaType(types.begin()->first);
            addMember("enum", std::move(a));
        }
    }

    const EnumElement* def = GetDefault(e);

    // this works because "default" is everytime set by value
    // if value will be moved into "enumerations" it need aditional check
    if (def && !def->empty() && !def->get().value()->empty()) {
        addMember("default", def->get().value()->clone());
    }
}

void JSONSchemaVisitor::operator()(const NullElement& e)
{
    addMember("type", make_empty<NullElement>());
}

void JSONSchemaVisitor::operator()(const StringElement& e)
{
    primitiveType(e);
}

void JSONSchemaVisitor::operator()(const NumberElement& e)
{
    primitiveType(e);
}

void JSONSchemaVisitor::operator()(const BooleanElement& e)
{
    primitiveType(e);
}

void JSONSchemaVisitor::operator()(const ExtendElement& e)
{
    auto merged = e.get().merge();
    if (!merged) {
        return;
    }

    Visit(*this, *merged);
}

void JSONSchemaVisitor::operator()(const OptionElement& e)
{
    data::option_t members;
    data::array_t reqVals;
    std::vector<const MemberElement*> varProps; // TODO: Add variable properties processing
    data::array_t oneOfMembers;
    IncludeMembers(e, members);

    auto props = std::move(pObj);
    processMembers(members, reqVals, varProps, oneOfMembers, *props);

    pObj = make_element<ObjectElement>();

    addMember("properties", std::move(props)); // TODO XXX @tjanc@ HIT 2

    if (!reqVals.empty()) {
        addMember("required", make_element<ArrayElement>(std::move(reqVals)));
    }

    if (!oneOfMembers.empty()) {
        addMember("oneOf", make_element<ArrayElement>(std::move(oneOfMembers)));
    }
}

ObjectElement* JSONSchemaVisitor::get()
{
    return pObj.get();
}

std::unique_ptr<ObjectElement> JSONSchemaVisitor::getOwnership()
{
    return std::move(pObj);
}

void JSONSchemaVisitor::processMember(const IElement& member,
    std::vector<const MemberElement*>& varProps,
    data::array_t& oneOfMembers,
    ObjectElement& o,
    std::set<std::string>& required)
{
    TypeQueryVisitor type;
    Visit(type, member);

    switch (type.get()) {
        case TypeQueryVisitor::Member: {
            const MemberElement* mr = static_cast<const MemberElement*>(&member);

            if (IsTypeAttribute(member, "required") || IsTypeAttribute(member, "fixed")
                || ((fixed || fixedType) && !IsTypeAttribute(member, "optional"))) {

                const StringElement* str = TypeQueryVisitor::as<const StringElement>(mr->get().key());

                if (str) {
                    required.insert(str->get());
                }
            }

            if (IsVariableProperty(*mr->get().key())) {
                varProps.push_back(mr);
            } else {
                JSONSchemaVisitor renderer(pDefs, fixed);
                Visit(renderer, member);
                ObjectElement* o1 = TypeQueryVisitor::as<ObjectElement>(renderer.get());

                if (!o1->get().empty()) {
                    auto front = o1->get().begin()[0]->clone();
                    if (TypeQueryVisitor::as<MemberElement>(front.get())) {
                        o.set();
                        o.get().push_back(std::move(front));
                    }
                }
            }
        } break;

        case TypeQueryVisitor::Select: {
            const SelectElement* sel = static_cast<const SelectElement*>(&member);

            // FIXME: there is no valid solution for multiple "SelectElement" in one object.

            for (auto const& select : sel->get()) {
                JSONSchemaVisitor v(pDefs);
                VisitBy(*select, v);
                oneOfMembers.push_back(v.getOwnership());
            }
        } break;

        default:
            throw LogicError("Invalid member type of object in MSON definition");
    }
}

std::string refract::renderJsonSchema(const IElement& e)
{
    auto pDefs = make_element<ObjectElement>();
    JSONSchemaVisitor gen_schema(*pDefs);

    gen_schema.get()->get().add_member(
        "$schema", make_element<StringElement>("http://json-schema.org/draft-04/schema#"));
    gen_schema.get()->get().add_member("type", make_element<StringElement>("object"));

    Visit(gen_schema, e);

    if (!pDefs->get().empty()) {
        gen_schema.get()->get().add_member("definitions", std::move(pDefs));
    }

    sos::SerializeJSON s;
    std::stringstream ss;
    // FIXME: remove SosSerializeCompactVisitor dependency
    SosSerializeCompactVisitor sv(false);
    VisitBy(*gen_schema.get(), sv);

    s.process(sv.value(), ss);

    return ss.str();
}
