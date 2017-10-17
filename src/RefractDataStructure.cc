//
//  RefractDataStructure.cc
//  drafter
//
//  Created by Jiri Kratochvil on 18/05/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "SourceAnnotation.h"
#include "RefractDataStructure.h"

#include "RefractSourceMap.h"
#include "refract/VisitorUtils.h"
#include "refract/ExpandVisitor.h"
#include "refract/SerializeVisitor.h"

#include "NamedTypesRegistry.h"
#include "RefractElementFactory.h"
#include "ConversionContext.h"

#include "ElementData.h"

using namespace refract;
using namespace drafter;

namespace
{

    auto const PrimitiveType = std::true_type::value;
    auto const ComplexType = std::false_type::value;

    template <typename U>
    struct FetchSourceMap {

        snowcrash::SourceMap<U> operator()(const NodeInfo<mson::ValueMember>& valueMember)
        {
            snowcrash::SourceMap<U> sourceMap = *NodeInfo<U>::NullSourceMap();
            sourceMap.sourceMap = valueMember.sourceMap->valueDefinition.sourceMap;
            return sourceMap;
        }

        snowcrash::SourceMap<U> operator()(const NodeInfo<mson::TypeSection>& typeSection)
        {
            snowcrash::SourceMap<U> sourceMap = *NodeInfo<U>::NullSourceMap();
            sourceMap.sourceMap = typeSection.sourceMap->value.sourceMap;
            return sourceMap;
        }
    };

    template <typename V, bool dummy = true>
    struct CheckValueValidity {

        using Info = ElementInfo<V>;

        void operator()(const Info&, ConversionContext&) const
        {
            // do nothing
        }
    };

    template <bool dummy>
    struct CheckValueValidity<NumberElement, dummy> {

        using Info = ElementInfo<NumberElement>;

        void operator()(const Info& info, ConversionContext& context) const
        {

            auto result = LiteralTo<data::number_t>(info.value);

            if (!std::get<0>(result)) {
                context.warn(snowcrash::Warning(
                    "invalid value format for 'number' type. please check mson specification for valid format",
                    snowcrash::MSONError,
                    info.sourceMap.sourceMap));
            }
        }
    };

    template <bool dummy>
    struct CheckValueValidity<BooleanElement, dummy> {

        using Info = ElementInfo<BooleanElement>;

        void operator()(const Info& info, ConversionContext& context) const
        {
            auto result = LiteralTo<data::bool_t>(info.value);

            if (!std::get<0>(result)) {
                context.warn(
                    snowcrash::Warning("invalid value for 'boolean' type. allowed values are 'true' or 'false'",
                        snowcrash::MSONError,
                        info.sourceMap.sourceMap));
            }
        }
    };

    void SetElementType(IElement& element, const mson::TypeDefinition& td)
    {
        if (!td.typeSpecification.name.symbol.literal.empty()) {
            element.element(td.typeSpecification.name.symbol.literal);
        }
    }

    mson::BaseTypeName NamedTypeFromElement(const IElement& element)
    {
        TypeQueryVisitor type;
        Visit(type, element);

        switch (type.get()) {
            case TypeQueryVisitor::Boolean:
                return mson::BooleanTypeName;

            case TypeQueryVisitor::Number:
                return mson::NumberTypeName;

            case TypeQueryVisitor::String:
                return mson::StringTypeName;

            case TypeQueryVisitor::Array:
                return mson::ArrayTypeName;

            case TypeQueryVisitor::Enum:
                return mson::EnumTypeName;

            case TypeQueryVisitor::Object:
                return mson::ObjectTypeName;

            default:
                return mson::UndefinedTypeName;
        }

        return mson::UndefinedTypeName;
    }

    template <typename T>
    mson::BaseTypeName GetType(const T& type, ConversionContext& context)
    {
        mson::BaseTypeName nameType = type.typeDefinition.typeSpecification.name.base;
        const std::string& parent = type.typeDefinition.typeSpecification.name.symbol.literal;

        if (nameType == mson::UndefinedTypeName && !parent.empty()) {
            const IElement* base = FindRootAncestor(parent, context.GetNamedTypesRegistry());
            if (base) {
                nameType = NamedTypeFromElement(*base);
            }
        }

        return nameType;
    }

    std::unique_ptr<ArrayElement> MsonTypeAttributesToRefract(const mson::TypeAttributes& ta)
    {
        if (ta == 0) {
            return nullptr;
        }

        auto attr = make_element<ArrayElement>();
        auto& content = attr->get();

        if (ta & mson::RequiredTypeAttribute) {
            content.push_back(make_primitive(SerializeKey::Required));
        }
        if (ta & mson::OptionalTypeAttribute) {
            content.push_back(make_primitive(SerializeKey::Optional));
        }
        if (ta & mson::FixedTypeAttribute) {
            content.push_back(make_primitive(SerializeKey::Fixed));
        }
        if (ta & mson::FixedTypeTypeAttribute) {
            content.push_back(make_primitive(SerializeKey::FixedType));
        }
        if (ta & mson::NullableTypeAttribute) {
            content.push_back(make_primitive(SerializeKey::Nullable));
        }

        if (content.empty()) {
            return nullptr;
        }

        return std::move(attr);
    }

    std::unique_ptr<IElement> MsonElementToRefract(const NodeInfo<mson::Element>& mse,
        ConversionContext& context,
        mson::BaseTypeName defaultNestedType = mson::StringTypeName);

    template <typename It>
    It MsonElementsToRefract(const NodeInfo<mson::Elements>& elements,
        It whereTo,
        ConversionContext& context,
        mson::BaseTypeName defaultNestedType = mson::StringTypeName)
    {
        NodeInfoCollection<mson::Elements> elementsNodeInfo(elements);

        return std::transform(elementsNodeInfo.begin(),
            elementsNodeInfo.end(),
            whereTo,
            [&context, &defaultNestedType](
                const auto& nodeInfo) { return MsonElementToRefract(nodeInfo, context, defaultNestedType); });
    }

    mson::BaseTypeName SelectNestedTypeSpecification(
        const mson::TypeNames& nestedTypes, const mson::BaseTypeName defaultNestedType = mson::StringTypeName)
    {
        mson::BaseTypeName type = defaultNestedType;
        // Found if type of element is specified.
        // if more types is used - fallback to "StringType"
        if (nestedTypes.size() == 1) {
            type = nestedTypes.begin()->base;
        }
        return type;
    }

    /**
     * Extract mson definitions like
     * - value
     *       - ts1
     *       - ts2
     */

    template <typename T>
    class ExtractTypeSection
    {
        using ElementType = T;
        using ValueType = typename T::ValueType;

        ElementData<T>& data;
        ConversionContext& context;

        mson::BaseTypeName elementTypeName;
        mson::BaseTypeName defaultNestedType;

        template <typename U, bool IsPrimitive = is_primitive<U> >
        struct Fetch;

        template <typename U>
        struct Fetch<U, true> {
            ElementInfo<T> operator()(const NodeInfo<mson::TypeSection>& typeSection,
                ConversionContext& context,
                const mson::BaseTypeName& defaultNestedType)
            {

                snowcrash::SourceMap<ValueType> sourceMap = FetchSourceMap<ValueType>()(typeSection);
                return ElementInfo<T>{ typeSection.node->content.value, sourceMap };
            }
        };

        template <typename U>
        struct Fetch<U, false> {
            ElementInfo<T> operator()(const NodeInfo<mson::TypeSection>& typeSection,
                ConversionContext& context,
                const mson::BaseTypeName& defaultNestedType)
            {
                std::deque<std::unique_ptr<IElement> > values;
                MsonElementsToRefract(
                    MakeNodeInfo(typeSection.node->content.elements(), typeSection.sourceMap->elements()),
                    std::back_inserter(values),
                    context,
                    defaultNestedType);
                return ElementInfo<T>{ std::move(values), FetchSourceMap<ValueType>()(typeSection) };
            }
        };

        template <typename U, bool IsPrimitive = is_primitive<U>, bool dummy = false>
        struct Store;

        template <typename U, bool dummy>
        struct Store<U, true, dummy> {
            void operator()(ElementData<U>& data,
                const NodeInfo<mson::TypeSection>& typeSection,
                ConversionContext& context,
                const mson::BaseTypeName& defaultNestedType)
            {
                // do nothing

                // Primitives should not contain members
                // this is to avoid push "empty" elements to primitives
                // it is related to test/fixtures/mson/primitive-with-members.apib
            }
        };

        template <typename U, bool dummy>
        struct Store<U, false, dummy> {
            void operator()(ElementData<U>& data,
                const NodeInfo<mson::TypeSection>& typeSection,
                ConversionContext& context,
                const mson::BaseTypeName& defaultNestedType)
            {

                data.values.push_back(Fetch<U>()(typeSection, context, defaultNestedType));
            }
        };

        template <bool dummy>
        struct Store<EnumElement, false, dummy> {
            void operator()(ElementData<EnumElement>& data,
                const NodeInfo<mson::TypeSection>& typeSection,
                ConversionContext& context,
                const mson::BaseTypeName& defaultNestedType)
            {

                data.enumerations.push_back(Fetch<EnumElement>()(typeSection, context, defaultNestedType));
            }
        };

        template <typename U, bool dummy = true>
        struct TypeDefinition;

        template <bool dummy>
        struct TypeDefinition<snowcrash::DataStructure, dummy> {
            const mson::TypeDefinition& operator()(const snowcrash::DataStructure& dataStructure)
            {
                return dataStructure.typeDefinition;
            }
        };

        template <bool dummy>
        struct TypeDefinition<mson::ValueMember, dummy> {
            const mson::TypeDefinition& operator()(const mson::ValueMember& valueMember)
            {
                return valueMember.valueDefinition.typeDefinition;
            }
        };

    public:
        template <typename U>
        ExtractTypeSection(ElementData<T>& data, ConversionContext& context, const NodeInfo<U>& sectionHolder)
            : data(data)
            , context(context)
            , elementTypeName(TypeDefinition<U>()(*sectionHolder.node).typeSpecification.name.base)
            , defaultNestedType(
                  SelectNestedTypeSpecification(TypeDefinition<U>()(*sectionHolder.node).typeSpecification.nestedTypes))
        {
        }

        void operator()(const NodeInfo<mson::TypeSection>& typeSection)
        {
            Fetch<ElementType> fetch;

            switch (typeSection.node->klass) {
                case mson::TypeSection::MemberTypeClass:

                    Store<ElementType>()(data, typeSection, context, defaultNestedType);
                    break;

                case mson::TypeSection::SampleClass:
                    data.samples.push_back(fetch(typeSection, context, defaultNestedType));
                    break;

                case mson::TypeSection::DefaultClass:
                    data.defaults.push_back(fetch(typeSection, context, defaultNestedType));
                    break;

                case mson::TypeSection::BlockDescriptionClass:
                    data.descriptions.push_back(
                        DescriptionInfo{ typeSection.node->content.description, typeSection.sourceMap->description });
                    break;

                default:
                    // we are not able to get sourcemap info there
                    // It is strongly dependent on type of section.
                    // For unknown type of Element we are not able to locate SourceMap
                    // with adequate effort we do not provide it to upper level
                    throw snowcrash::Error("unknown section type", snowcrash::ApplicationError);
            }
        }
    };

    mson::BaseTypeName RefractElementTypeToMsonType(TypeQueryVisitor::ElementType type)
    {
        switch (type) {
            case TypeQueryVisitor::String:
                return mson::StringTypeName;

            case TypeQueryVisitor::Number:
                return mson::NumberTypeName;

            case TypeQueryVisitor::Boolean:
                return mson::BooleanTypeName;

            case TypeQueryVisitor::Array:
                return mson::ArrayTypeName;

            case TypeQueryVisitor::Object:
                return mson::ObjectTypeName;

            case TypeQueryVisitor::Enum:
                return mson::EnumTypeName;

            case TypeQueryVisitor::Null:
            case TypeQueryVisitor::Holder:
            case TypeQueryVisitor::Member:
            case TypeQueryVisitor::Ref:
            case TypeQueryVisitor::Extend:
            case TypeQueryVisitor::Option:
            case TypeQueryVisitor::Select:;
        };
        return mson::UndefinedTypeName;
    }

    mson::BaseTypeName GetMsonTypeFromName(const std::string& name, const ConversionContext& context)
    {
        const IElement* e = FindRootAncestor(name, context.GetNamedTypesRegistry());
        if (!e) {
            return mson::UndefinedTypeName;
        }

        TypeQueryVisitor query;
        VisitBy(*e, query);
        return RefractElementTypeToMsonType(query.get());
    }

    /**
     * Extract mson definitions like
     * - value enum[td1, td2]
     */

    template <typename T>
    struct ExtractTypeDefinition {

        ElementData<T>& data;
        const ConversionContext& context;

        template <typename E, bool IsPrimitive = is_primitive<E> >
        struct Fetch;

        template <typename E>
        struct Fetch<E, ComplexType> {
            ElementInfo<T> operator()(const mson::TypeNames& typeNames, const ConversionContext& context)
            {
                std::deque<std::unique_ptr<IElement> > types;

                for (const auto& type : typeNames) {
                    mson::BaseTypeName typeName = type.base;
                    FactoryCreateMethod method = eValue;

                    if (typeName == mson::UndefinedTypeName && !type.symbol.literal.empty()) {
                        typeName = GetMsonTypeFromName(type.symbol.literal, context);
                        method = type.symbol.variable ? eSample : eElement;
                    }

                    const RefractElementFactory& f = FactoryFromType(typeName);
                    types.push_back(f.Create(type.symbol.literal, method));
                }

                return ElementInfo<T>{ std::move(types), *NodeInfo<typename T::ValueType>::NullSourceMap() };
            }
        };

        template <typename E, bool IsPrimitive = is_primitive<E>, bool dummy = true>
        struct Store;

        template <typename E, bool dummy>
        struct Store<E, true, dummy> {
            void operator()(ElementData<E>& data, const mson::TypeNames& typeNames, const ConversionContext& context)
            {
                // do nothing - Primitive types does not contain TypeDefinitions
            }
        };

        template <typename E, bool dummy>
        struct Store<E, false, dummy> {
            void operator()(ElementData<E>& data, const mson::TypeNames& typeNames, const ConversionContext& context)
            {
                data.values.push_back(Fetch<E>()(typeNames, context));
            }
        };

        template <bool dummy>
        struct Store<EnumElement, false, dummy> {
            using E = EnumElement;
            void operator()(ElementData<E>& data, const mson::TypeNames& typeNames, const ConversionContext& context)
            {
                data.enumerations.push_back(Fetch<E>()(typeNames, context));
            }
        };

        ExtractTypeDefinition(ElementData<T>& data, const ConversionContext& context) : data(data), context(context) {}

        void operator()(const NodeInfo<mson::TypeDefinition>& typeDefinition)
        {
            Store<T>()(data, typeDefinition.node->typeSpecification.nestedTypes, context);
        }
    };

    /**
     * Extract mson definitions like
     * - value: vm1, vm2
     */
    template <typename T, typename V = typename T::ValueType>
    struct ExtractValueMember {

        ElementData<T>& data;
        ConversionContext& context;

        template <typename U, bool IsPrimitive = is_primitive<U>, bool dummy = true>
        struct Fetch;

        template <typename U, bool dummy>
        struct Fetch<U, true, dummy> { // primitive values

            ElementInfo<T> operator()(const NodeInfo<mson::ValueMember>& valueMember, ConversionContext& context)
            {
                if (valueMember.node->valueDefinition.values.size() > 1) {
                    throw snowcrash::Error("only one value is supported for primitive types",
                        snowcrash::MSONError,
                        valueMember.sourceMap->sourceMap);
                }

                const mson::Value& value = *valueMember.node->valueDefinition.values.begin();
                snowcrash::SourceMap<V> sourceMap = FetchSourceMap<V>()(valueMember);

                return ElementInfo<T>{ value.literal, sourceMap };
            }
        };

        template <typename U, bool dummy>
        struct Fetch<U, false, dummy> { // Array|Object

            ElementInfo<T> operator()(const NodeInfo<mson::ValueMember>& valueMember, ConversionContext& context)
            {

                const mson::BaseTypeName type = SelectNestedTypeSpecification(
                    valueMember.node->valueDefinition.typeDefinition.typeSpecification.nestedTypes);

                const RefractElementFactory& factory = FactoryFromType(type);
                const mson::Values& values = valueMember.node->valueDefinition.values;

                std::deque<std::unique_ptr<IElement> > elements;

                std::transform(
                    values.begin(), values.end(), std::back_inserter(elements), [&factory](const mson::Value& value) {
                        return factory.Create(value.literal, value.variable ? eSample : eValue);
                    });

                return ElementInfo<T>{ std::move(elements), FetchSourceMap<V>()(valueMember) };
            }
        };

        template <bool dummy>
        struct Fetch<EnumElement, false, dummy> { // Enum

            ElementInfo<T> operator()(const NodeInfo<mson::ValueMember>& valueMember, ConversionContext& context)
            {

                const mson::BaseTypeName type = SelectNestedTypeSpecification(
                    valueMember.node->valueDefinition.typeDefinition.typeSpecification.nestedTypes);

                const RefractElementFactory& f = FactoryFromType(type);
                const mson::Values& values = valueMember.node->valueDefinition.values;

                std::deque<std::unique_ptr<IElement> > elements;

                for (const auto& value : values) {
                    elements.push_back(f.Create(value.literal, eValue));
                }

                return ElementInfo<T>{ std::move(elements), FetchSourceMap<V>()(valueMember) };
            }
        };

        template <typename Y, bool IsIterable = data::is_iterable<Y>::value>
        struct IsValueVariable {
            bool operator()(const mson::Value& value)
            {
                return value.variable;
            }
        };

        template <typename Y>
        struct IsValueVariable<Y, true> {
            bool operator()(const mson::Value&)
            {
                return false;
            }
        };

        template <typename E, bool dummy = true>
        struct Store {
            void operator()(ElementData<E>& data, ElementInfo<E> info)
            {
                data.values.push_back(std::move(info));
            }
        };

        template <bool dummy>
        struct Store<EnumElement, dummy> {
            void operator()(ElementData<EnumElement>& data, ElementInfo<EnumElement> info)
            {
                if (info.value.size() == 1) {
                    data.values.push_back(std::move(info));
                } else {
                    data.enumerations.push_back(std::move(info));
                }
            }
        };

        ExtractValueMember(ElementData<T>& data, ConversionContext& context, const mson::BaseTypeName)
            : data(data), context(context)
        {
        }

        void operator()(const NodeInfo<mson::ValueMember>& valueMember)
        {
            // silently ignore "value" for ObjectElement e.g.
            // # A (array)
            // - key (object)
            // warning is attached while creating ValueMember in snowcrash
            if (valueMember.node->valueDefinition.typeDefinition.baseType == mson::ImplicitObjectBaseType
                || valueMember.node->valueDefinition.typeDefinition.baseType == mson::ObjectBaseType) {
                return;
            }

            Fetch<T> fetch;
            mson::TypeAttributes attrs = valueMember.node->valueDefinition.typeDefinition.attributes;

            if (!valueMember.node->valueDefinition.values.empty()) {
                const mson::Value& value = *valueMember.node->valueDefinition.values.begin();

                if (attrs & mson::DefaultTypeAttribute) {
                    data.defaults.push_back(fetch(valueMember, context));
                } else if ((attrs & mson::SampleTypeAttribute) || IsValueVariable<typename T::ValueType>()(value)) {
                    data.samples.push_back(fetch(valueMember, context));
                } else {
                    Store<T>()(data, fetch(valueMember, context));
                }
            } else {
                if (attrs & mson::DefaultTypeAttribute) {
                    context.warn(snowcrash::Warning("no value present when 'default' is specified",
                        snowcrash::MSONError,
                        valueMember.sourceMap->sourceMap));
                }

                if (attrs & mson::SampleTypeAttribute) {
                    context.warn(snowcrash::Warning("no value present when 'sample' is specified",
                        snowcrash::MSONError,
                        valueMember.sourceMap->sourceMap));
                }
            }

            if (!valueMember.node->description.empty()) {
                data.descriptions.push_back(
                    DescriptionInfo{ valueMember.node->description, valueMember.sourceMap->description });
            }

            if ((valueMember.node->valueDefinition.values.empty()
                    || (valueMember.node->valueDefinition.typeDefinition.typeSpecification.nestedTypes.size() > 1))) {

                ExtractTypeDefinition<T> extd(data, context);
                extd(MakeNodeInfoWithoutSourceMap(valueMember.node->valueDefinition.typeDefinition));
            }
        }
    };

    struct Join {
        std::string& base;
        Join(std::string& str) : base(str) {}

        void operator()(const std::string& append, const std::string separator = "\n")
        {
            if (append.empty()) {
                return;
            }

            if (!base.empty()) {
                base.append(separator);
            }

            base.append(append);
        }
    };

    template <typename T, bool IsPrimitive = is_primitive<T> >
    struct Merge;

    template <typename T>
    struct Merge<T, true> {
        using Info = ElementInfo<T>;
        using Container = std::deque<Info>;

        ElementInfo<T> operator()(Container container) const
        {
            if (container.empty()) {
                return ElementInfo<T>();
            } else {
                auto result = std::move(container.front());
                container.pop_front();
                return std::move(result);
            }
        }
    };

    template <typename T>
    struct Merge<T, false> {
        using Info = ElementInfo<T>;
        using Container = std::deque<Info>;
        using StoredType = typename stored_type<T>::type;
        using SourceMap = typename content_source_map_type<T>::type;

        Info operator()(Container container) const
        {
            Info result{};

            for (auto& info : container) {
                std::move(info.value.begin(), info.value.end(), std::back_inserter(result.value));

                // FIXME: merge source map?
                // it is not used later,
                // every element in vector has it own map
            }

            return std::move(result);
        }
    };

    template <typename E, bool IsPrimitive = is_primitive<E>, bool dummy = true>
    struct ElementInfoToElement;

    template <typename E, bool dummy>
    struct ElementInfoToElement<E, true, dummy> {
        std::unique_ptr<E> operator()(ElementInfo<E>&& info)
        {
            auto result = LiteralTo<typename E::ValueType>(info.value);
            return result.first ? make_element<E>(result.second) : make_empty<E>();
        }
    };

    template <typename E, bool dummy>
    struct ElementInfoToElement<E, false, dummy> {
        std::unique_ptr<E> operator()(ElementInfo<E>&& info)
        {
            auto result = make_element<E>();
            std::move(info.value.begin(), info.value.end(), std::back_inserter(result->get()));
            return std::move(result);
        }
    };

    template <bool dummy>
    struct ElementInfoToElement<EnumElement, false, dummy> {
        std::unique_ptr<EnumElement> operator()(ElementInfo<EnumElement>&& info)
        {

            auto& v = info.value;

            if (v.size() == 1) {
                auto result = make_element<EnumElement>(std::move(v.front()));
                v.pop_front(); // front has been moved from - remove garbage
                return std::move(result);
            } else if (v.size() > 1) {
                auto result = make_empty<EnumElement>();
                auto enumerations = make_element<ArrayElement>();
                std::move(v.begin(), v.end(), std::back_inserter(enumerations->get()));
                result->attributes().set(SerializeKey::Enumerations, std::move(enumerations));
                return std::move(result);
            }

            return make_empty<EnumElement>();
        }
    };

    template <typename T, bool IsPrimitive = is_primitive<T> >
    struct SaveValue;

    template <typename T>
    struct SaveValue<T, true> {
        void operator()(std::deque<ElementInfo<T> > values, std::deque<ElementInfo<T> > /* enumerations*/, T& element)
        {
            if (values.empty()) {
                return;
            }

            auto info = Merge<T>()(std::move(values));

            auto result = LiteralTo<typename T::ValueType>(info.value);
            assert(result.first); // by now we should have validated

            element.set(result.second);

            // FIXME: refactoring adept - AttachSourceMap require NodeInfo, let it pass for now
            AttachSourceMap(element, MakeNodeInfo(result.second, info.sourceMap));
        }
    };

    template <typename T>
    struct SaveValue<T, false> {
        void operator()(std::deque<ElementInfo<T> > values, std::deque<ElementInfo<T> > /* enumerations*/, T& element)
        {
            auto info = Merge<T>()(std::move(values));

            if (!info.value.empty()) {
                element.set();
                std::move(info.value.begin(), info.value.end(), std::back_inserter(element.get()));
            }
        }
    };

    template <>
    struct SaveValue<EnumElement, false> {

        void operator()(std::deque<ElementInfo<EnumElement> > values,
            std::deque<ElementInfo<EnumElement> > enumerations,
            EnumElement& element)
        {
            auto valuesInfo = Merge<EnumElement>()(std::move(values));
            auto enumInfo = Merge<EnumElement>()(std::move(enumerations));

            if (!valuesInfo.value.empty()) {
                auto content = std::move(valuesInfo.value.front());
                valuesInfo.value.pop_front();
                element.set(data::enum_t{ std::move(content) });
            }

            if (!enumInfo.value.empty()) {
                auto enumsElement = make_element<ArrayElement>();
                std::move(enumInfo.value.begin(), enumInfo.value.end(), std::back_inserter(enumsElement->get()));
                element.attributes().set(SerializeKey::Enumerations, std::move(enumsElement));
            }
        }
    };

    template <typename T, typename U>
    void AllElementsToAtribute(U values, const std::string& key, IElement& element)
    {
        if (values.empty()) {
            return;
        }

        auto a = make_element<ArrayElement>();

        ElementInfoToElement<T> fetch;
        std::transform(std::make_move_iterator(values.begin()),
            std::make_move_iterator(values.end()),
            std::back_inserter(a->get()),
            fetch);

        element.attributes().set(key, std::move(a));
    }

    template <typename T, typename U>
    void LastElementToAttribute(U values, const std::string& key, IElement& element)
    {

        if (values.empty()) {
            return;
        }

        ElementInfoToElement<T> fetch;
        element.attributes().set(key, fetch(std::move(values.back())));
    }

    template <typename T>
    void ElementDataToElement(T& element, ElementData<T> data, ConversionContext& context)
    {
        auto validate = [&context](const auto& val) {
            CheckValueValidity<T> validate;
            validate(val, context);
        };

        std::for_each(data.values.begin(), data.values.end(), validate);
        std::for_each(data.samples.begin(), data.samples.end(), validate);
        std::for_each(data.defaults.begin(), data.defaults.end(), validate);
        std::for_each(data.enumerations.begin(), data.enumerations.end(), validate);

        SaveValue<T>()(std::move(data.values), std::move(data.enumerations), element);
        AllElementsToAtribute<T>(std::move(data.samples), SerializeKey::Samples, element);
        LastElementToAttribute<T>(std::move(data.defaults), SerializeKey::Default, element);
    }

    template <typename T>
    std::unique_ptr<IElement> DescriptionToRefract(const T& descriptions)
    {
        if (descriptions.empty()) {
            return nullptr;
        }

        std::string description;
        Join join(description);
        snowcrash::SourceMap<std::string> sourceMap;

        for (const auto& item : descriptions) {
            join(item.description);
            sourceMap.sourceMap.append(item.sourceMap.sourceMap);
        }

        if (description.empty()) {
            return nullptr;
        }

        return PrimitiveToRefract(NodeInfo<std::string>(&description, &sourceMap));
    }

    // FIXME: refactoring - description is not used while calling from
    // `RefractElementFromProperty()`
    template <typename T>
    std::unique_ptr<IElement> RefractElementFromValue(const NodeInfo<mson::ValueMember>& value,
        ConversionContext& context,
        mson::BaseTypeName defaultNestedType,
        DescriptionInfoContainer& descriptions)
    {
        using ElementType = T;

        ElementData<ElementType> data;
        auto element = make_empty<ElementType>();

        ExtractValueMember<ElementType>(data, context, defaultNestedType)(value);

        SetElementType(*element, value.node->valueDefinition.typeDefinition);
        AttachSourceMap(*element, value);

        NodeInfoCollection<mson::TypeSections> typeSections(MAKE_NODE_INFO(value, sections));

        std::for_each(typeSections.begin(), typeSections.end(), ExtractTypeSection<T>(data, context, value));

        descriptions = std::move(data.descriptions);
        ElementDataToElement(*element, std::move(data), context);

        return std::move(element);
    }

    bool VariablePropertyIsString(const mson::ValueDefinition& variable, ConversionContext& context)
    {
        if (variable.typeDefinition.typeSpecification.name.base == mson::StringTypeName) {
            return true;
        }

        if (TypeQueryVisitor::as<StringElement>(FindRootAncestor(
                variable.typeDefinition.typeSpecification.name.symbol.literal, context.GetNamedTypesRegistry()))) {
            return true;
        }

        return false;
    }

    std::unique_ptr<IElement> GetPropertyKey(const NodeInfo<mson::PropertyMember>& property, ConversionContext& context)
    {

        auto key = make_element<StringElement>();
        snowcrash::SourceMap<mson::Literal> sourceMap;
        sourceMap.sourceMap.append(property.sourceMap->name.sourceMap);

        if (!property.node->name.variable.empty()) {

            if (property.node->name.variable.values.size() > 1) {
                // FIXME: is there example for multiple variables?
                context.warn(snowcrash::Warning("multiple variables in property definition is not implemented",
                    snowcrash::MSONError,
                    sourceMap.sourceMap));
            }

            // variable containt type definition
            if (!property.node->name.variable.typeDefinition.empty()) {
                if (!VariablePropertyIsString(property.node->name.variable, context)) {
                    throw snowcrash::Error("'variable named property' must be string or its sub-type",
                        snowcrash::MSONError,
                        sourceMap.sourceMap);
                }

                SetElementType(*key, property.node->name.variable.typeDefinition);
            }

            key->attributes().set(SerializeKey::Variable, make_primitive(true));

            if (!property.node->name.variable.values.empty()) {
                key->set(property.node->name.variable.values.begin()->literal);
            }
        }

        if (!property.node->name.literal.empty()) {
            key->set(property.node->name.literal);
        }

        AttachSourceMap(*key, MakeNodeInfo(property.node->name.literal, sourceMap));

        return std::move(key);
    }

    template <typename T>
    std::unique_ptr<MemberElement> RefractElementFromProperty(const NodeInfo<mson::PropertyMember>& property,
        ConversionContext& context,
        const mson::BaseTypeName defaultNestedType)
    {

        DescriptionInfoContainer dummy; // we need no this
        DescriptionInfoContainer descriptions;

        auto element = make_element<MemberElement>(GetPropertyKey(property, context),
            RefractElementFromValue<T>(
                NodeInfo<mson::ValueMember>(property.node, property.sourceMap), context, defaultNestedType, dummy));

        mson::TypeAttributes attrs = property.node->valueDefinition.typeDefinition.attributes;

        // there is no source map for attributes
        if (auto attributes = MsonTypeAttributesToRefract(attrs)) {
            element->attributes().set(SerializeKey::TypeAttributes, std::move(attributes));
        }

        if (!property.node->description.empty()) {
            descriptions.push_back(DescriptionInfo{ property.node->description, property.sourceMap->description });
        }

        NodeInfoCollection<mson::TypeSections> typeSections(MAKE_NODE_INFO(property, sections));

        for (const auto& typeSection : typeSections) {
            if (typeSection.node->klass == mson::TypeSection::BlockDescriptionClass) {
                descriptions.push_back(
                    DescriptionInfo{ typeSection.node->content.description, typeSection.sourceMap->description });
            }
        }

        if (!property.node->description.empty() && (descriptions.size() > 1)) {
            descriptions[0].description.append("\n");
        }

        if (auto description = DescriptionToRefract(descriptions)) {
            element->meta().set(SerializeKey::Description, std::move(description));
        }

        return std::move(element);
    }

    bool ValueHasMembers(const mson::ValueMember* value)
    {
        for (mson::TypeSections::const_iterator it = value->sections.begin(); it != value->sections.end(); ++it) {
            if (it->klass == mson::TypeSection::MemberTypeClass) {
                return true;
            }
        }
        return false;
    }

    bool ValueHasChildren(const mson::ValueMember* value)
    {
        return value->valueDefinition.values.size() > 1;
    }

    bool ValueHasName(const mson::ValueMember* value)
    {
        return !value->valueDefinition.typeDefinition.typeSpecification.name.symbol.literal.empty();
    }

    struct PropertyTrait {
        typedef MemberElement ElementType;
        typedef NodeInfo<mson::PropertyMember> InputType;

        template <typename T>
        static std::unique_ptr<ElementType> Invoke(
            const InputType& prop, ConversionContext& context, const mson::BaseTypeName defaultNestedType)
        {
            return RefractElementFromProperty<T>(prop, context, defaultNestedType);
        }
    };

    struct ValueTrait {
        typedef IElement ElementType;
        typedef NodeInfo<mson::ValueMember> InputType;

        template <typename T>
        static std::unique_ptr<ElementType> Invoke(
            const InputType& val, ConversionContext& context, const mson::BaseTypeName defaultNestedType)
        {
            DescriptionInfoContainer descriptions;

            auto element = RefractElementFromValue<T>(val, context, defaultNestedType, descriptions);

            mson::TypeAttributes attrs = val.node->valueDefinition.typeDefinition.attributes;

            // there is no source map for attributes
            if (auto attributes = MsonTypeAttributesToRefract(attrs)) {
                element->attributes().set(SerializeKey::TypeAttributes, std::move(attributes));
            }

            if (auto description = DescriptionToRefract(descriptions)) {
                element->meta().set(SerializeKey::Description, std::move(description));
            }

            return std::move(element);
        }
    };

    void CheckTypeAttributesClash(const mson::TypeAttributes& attributes,
        const snowcrash::SourceMap<mson::ValueDefinition>& sourceMap,
        ConversionContext& context)
    {

        if ((attributes & mson::FixedTypeAttribute) != 0 && (attributes & mson::OptionalTypeAttribute) != 0) {

            context.warn(snowcrash::Warning(
                "cannot use 'fixed' and 'optional' together", snowcrash::MSONError, sourceMap.sourceMap));
        }

        if ((attributes & mson::RequiredTypeAttribute) != 0 && (attributes & mson::OptionalTypeAttribute) != 0) {

            context.warn(snowcrash::Warning(
                "cannot use 'required' and 'optional' together", snowcrash::MSONError, sourceMap.sourceMap));
        }

        if ((attributes & mson::DefaultTypeAttribute) != 0 && (attributes & mson::SampleTypeAttribute) != 0) {

            context.warn(snowcrash::Warning(
                "cannot use 'default' and 'sample' together", snowcrash::MSONError, sourceMap.sourceMap));
        }

        if ((attributes & mson::FixedTypeAttribute) != 0 && (attributes & mson::FixedTypeTypeAttribute) != 0) {

            context.warn(snowcrash::Warning(
                "cannot use 'fixed' and 'fixed-type' together", snowcrash::MSONError, sourceMap.sourceMap));
        }
    }

    template <typename Trait>
    std::unique_ptr<IElement> MsonMemberToRefract(const typename Trait::InputType& input,
        ConversionContext& context,
        const mson::BaseTypeName nameType,
        const mson::BaseTypeName defaultNestedType,
        bool checkTypeAttributes = true)
    {

        if (checkTypeAttributes) {
            CheckTypeAttributesClash(
                input.node->valueDefinition.typeDefinition.attributes, input.sourceMap->valueDefinition, context);
        }

        switch (nameType) {
            case mson::BooleanTypeName:
                return Trait::template Invoke<BooleanElement>(input, context, defaultNestedType);

            case mson::NumberTypeName:
                return Trait::template Invoke<NumberElement>(input, context, defaultNestedType);

            case mson::StringTypeName:
                return Trait::template Invoke<StringElement>(input, context, defaultNestedType);

            case mson::EnumTypeName:
                return Trait::template Invoke<EnumElement>(input, context, defaultNestedType);

            case mson::ArrayTypeName:
                return Trait::template Invoke<ArrayElement>(input, context, defaultNestedType);

            case mson::ObjectTypeName:
                return Trait::template Invoke<ObjectElement>(input, context, defaultNestedType);

            case mson::UndefinedTypeName: {
                if (ValueHasChildren(input.node)) {
                    // FIXME: what about EnumElement
                    return Trait::template Invoke<ArrayElement>(input, context, defaultNestedType);
                } else if (ValueHasName(input.node) || ValueHasMembers(input.node)) {
                    return Trait::template Invoke<ObjectElement>(input, context, defaultNestedType);
                } else if (nameType != defaultNestedType) {
                    return MsonMemberToRefract<Trait>(input, context, defaultNestedType, defaultNestedType, false);
                }

                return MsonMemberToRefract<Trait>(input, context, mson::StringTypeName, defaultNestedType, false);
            }
        }

        throw snowcrash::Error("unknown type of mson member", snowcrash::MSONError, input.sourceMap->sourceMap);
    }

    std::unique_ptr<IElement> MsonOneofToRefract(const NodeInfo<mson::OneOf>& oneOf, ConversionContext& context)
    {

        NodeInfoCollection<mson::OneOf> oneOfNodeInfo(oneOf);
        auto select = oneOfNodeInfo.empty() ? make_empty<SelectElement>() : make_element<SelectElement>();

        for (const auto& oneOfInfo : oneOfNodeInfo) {

            auto option = make_element<OptionElement>();

            // we can not use MsonElementToRefract() for groups,
            // "option" element handles directly all elements in group
            if (oneOfInfo.node->klass == mson::Element::GroupClass) {
                MsonElementsToRefract(MakeNodeInfo(oneOfInfo.node->content.elements(), oneOfInfo.sourceMap->elements()),
                    std::back_inserter(option->get()),
                    context);
            } else {
                option->get().push_back(MsonElementToRefract(oneOfInfo, context, mson::StringTypeName));
            }

            select->get().push_back(std::move(option));
        }

        return std::move(select);
    }

    std::unique_ptr<RefElement> MsonMixinToRefract(const NodeInfo<mson::Mixin>& mixin)
    {
        auto ref = make_element<RefElement>(mixin.node->typeSpecification.name.symbol.literal);

        ref->attributes().set(SerializeKey::Path, make_primitive(SerializeKey::Content));

        return std::move(ref);
    }

    std::unique_ptr<IElement> MsonElementToRefract(const NodeInfo<mson::Element>& mse,
        ConversionContext& context,
        const mson::BaseTypeName defaultNestedType /* = mson::StringTypeName */)
    {
        switch (mse.node->klass) {
            case mson::Element::PropertyClass:
                return MsonMemberToRefract<PropertyTrait>(
                    MakeNodeInfo(mse.node->content.property, mse.sourceMap->property),
                    context,
                    GetType(mse.node->content.property.valueDefinition, context),
                    defaultNestedType);

            case mson::Element::ValueClass:
                return MsonMemberToRefract<ValueTrait>(MakeNodeInfo(mse.node->content.value, mse.sourceMap->value),
                    context,
                    GetType(mse.node->content.value.valueDefinition, context),
                    defaultNestedType);

            case mson::Element::MixinClass:
                return MsonMixinToRefract(MakeNodeInfo(mse.node->content.mixin, mse.sourceMap->mixin));

            case mson::Element::OneOfClass:
                return MsonOneofToRefract(MakeNodeInfo(mse.node->content.oneOf(), mse.sourceMap->oneOf()), context);

            case mson::Element::GroupClass:
                throw snowcrash::Error("unable to handle element group", snowcrash::ApplicationError);

            default:
                throw snowcrash::Error("unknown type of mson element", snowcrash::ApplicationError);
        }
    }

    template <typename T>
    std::unique_ptr<IElement> RefractElementFromMSON(
        const NodeInfo<snowcrash::DataStructure>& ds, ConversionContext& context)
    {
        using namespace refract;
        typedef T ElementType;

        auto element = make_empty<ElementType>();
        SetElementType(*element, ds.node->typeDefinition);

        if (!ds.node->name.symbol.literal.empty()) {
            snowcrash::SourceMap<mson::Literal> sourceMap = *NodeInfo<mson::Literal>::NullSourceMap();
            sourceMap.sourceMap.append(ds.sourceMap->name.sourceMap);
            element->meta().set(
                SerializeKey::Id, PrimitiveToRefract(MakeNodeInfo(ds.node->name.symbol.literal, sourceMap)));
        }

        AttachSourceMap(*element, MakeNodeInfo(ds.node, ds.sourceMap));

        // there is no source map for attributes
        if (auto attributes = MsonTypeAttributesToRefract(ds.node->typeDefinition.attributes)) {
            element->attributes().set(SerializeKey::TypeAttributes, std::move(attributes));
        }

        ElementData<T> data;

        ExtractTypeDefinition<ElementType> extd(data, context);
        extd(MAKE_NODE_INFO(ds, typeDefinition));

        NodeInfoCollection<mson::TypeSections> typeSections(MAKE_NODE_INFO(ds, sections));

        std::for_each(typeSections.begin(), typeSections.end(), ExtractTypeSection<T>(data, context, ds));

        if (auto description = DescriptionToRefract(std::move(data.descriptions))) {
            element->meta().set(SerializeKey::Description, std::move(description));
        }

        ElementDataToElement(*element, std::move(data), context);

        return std::move(element);
    }
}

std::unique_ptr<IElement> drafter::MSONToRefract(
    const NodeInfo<snowcrash::DataStructure>& dataStructure, ConversionContext& context)
{
    if (dataStructure.node->empty()) {
        return nullptr;
    }

    mson::BaseTypeName nameType = GetType(*dataStructure.node, context);

    switch (nameType) {
        case mson::BooleanTypeName:
            return RefractElementFromMSON<BooleanElement>(dataStructure, context);
        case mson::NumberTypeName:
            return RefractElementFromMSON<NumberElement>(dataStructure, context);
        case mson::StringTypeName:
            return RefractElementFromMSON<StringElement>(dataStructure, context);
        case mson::EnumTypeName:
            return RefractElementFromMSON<EnumElement>(dataStructure, context);
        case mson::ArrayTypeName:
            return RefractElementFromMSON<ArrayElement>(dataStructure, context);
        case mson::ObjectTypeName:
        case mson::UndefinedTypeName:
            return RefractElementFromMSON<ObjectElement>(dataStructure, context);
        default:
            throw snowcrash::Error("unknown type of data structure", snowcrash::ApplicationError);
    }
}

std::unique_ptr<IElement> drafter::ExpandRefract(std::unique_ptr<IElement> element, ConversionContext& context)
{
    if (!element) {
        return nullptr;
    }

    ExpandVisitor expander(context.GetNamedTypesRegistry());
    Visit(expander, *element);

    if (auto expanded = expander.get()) {
        return std::move(expanded);
    }

    return std::move(element);
}

sos::Object drafter::SerializeRefract(const IElement* element, ConversionContext& context)
{
    if (!element) {
        return sos::Object();
    }

    SosSerializeVisitor serializer(context.options.generateSourceMap);
    Visit(serializer, *element);

    return serializer.get();
}
