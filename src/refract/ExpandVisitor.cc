//
//  refract/ExpandVisitor.cc
//  librefract
//
//  Created by Jiri Kratochvil on 21/05/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "Element.h"
#include "Registry.h"
#include <stack>

#include <functional>

#include <sstream>

#include "SourceAnnotation.h"

#include "IsExpandableVisitor.h"
#include "ExpandVisitor.h"
#include "TypeQueryVisitor.h"
#include "VisitorUtils.h"

#define VISIT_IMPL(ELEMENT)                                                                                            \
    void ExpandVisitor::operator()(const ELEMENT##Element& e)                                                          \
    {                                                                                                                  \
        result = Expand(e, context);                                                                                   \
    }

namespace refract
{

    namespace
    {

        bool Expandable(const IElement& e)
        {
            IsExpandableVisitor v;
            VisitBy(e, v);
            return v.get();
        }

        void CopyMetaId(IElement& dst, const IElement& src)
        {
            auto name = src.meta().find("id");
            if (name != src.meta().end() && name->second && !name->second->empty()) {
                dst.meta().set("id", name->second->clone());
            }
        }

        void MetaIdToRef(IElement& e)
        {
            auto name = e.meta().find("id");
            if (name != e.meta().end() && name->second && !name->second->empty()) {
                e.meta().set("ref", name->second->clone());
                e.meta().erase("id");
            }
        }

        template <typename T, bool IsIterable = data::is_iterable<T>::value>
        struct ExpandValueImpl {

            template <typename Functor>
            T operator()(const T& value, Functor&)
            {
                return value;
            }
        };

        template <>
        struct ExpandValueImpl<data::enum_t, false> {

            template <typename Functor>
            data::enum_t operator()(const data::enum_t& v, Functor& expand)
            {
                return data::enum_t{ expand(v.value()) };
            }
        };

        template <>
        struct ExpandValueImpl<data::holder_t, false> {

            template <typename Functor>
            data::holder_t operator()(const data::holder_t& v, Functor& expand)
            {
                return data::holder_t{ expand(v.data()) };
            }
        };

        template <typename T>
        struct ExpandValueImpl<T, true> {

            template <typename Functor>
            T operator()(const T& value, Functor& expand)
            {
                T members;
                std::transform(value.begin(), value.end(), std::back_inserter(members), [&expand](const auto& el) {
                    return expand(el.get());
                });
                return std::move(members);
            }
        };

        std::unique_ptr<ExtendElement> GetInheritanceTree(const std::string& name, const Registry& registry)
        {
            std::stack<std::unique_ptr<IElement> > inheritance;
            std::string en = name;

            // FIXME: add check against recursive inheritance
            // walk recursive in registry and expand inheritance tree
            for (const IElement *parent = registry.find(en); parent && !isReserved(en);
                 en = parent->element(), parent = registry.find(en)) {

                inheritance.push(clone(*parent, ((IElement::cAll ^ IElement::cElement) | IElement::cNoMetaId)));
                inheritance.top()->meta().set("ref", make_primitive(en));
            }

            if (inheritance.empty())
                return make_empty<ExtendElement>();

            auto e = make_element<ExtendElement>();
            auto& content = e->get();

            do {
                content.push_back(std::move(inheritance.top()));
                inheritance.pop();
            } while (!inheritance.empty());

            // FIXME: posible solution while referenced type is not found in regisry
            // \see test/fixtures/mson-resource-unresolved-reference.apib
            //
            // if (e->value.empty()) {
            //   e->meta["ref"] = IElement::Create(name);
            //}

            return std::move(e);
        }
    } // anonymous namespace

    struct ExpandVisitor::Context {

        const Registry& registry;
        ExpandVisitor* expand;
        std::deque<std::string> members;

        Context(const Registry& registry, ExpandVisitor* expand) : registry(registry), expand(expand) {}

        std::unique_ptr<IElement> ExpandOrClone(const IElement* e) const
        {
            if (!e) {
                return nullptr;
            }

            VisitBy(*e, *expand);
            auto result = expand->get();

            if (!result) {
                result = e->clone();
            }

            return std::move(result);
        }

        template <typename V>
        V ExpandValue(const V& v)
        {
            auto expandOrClone = [this](const auto& el) { return ExpandOrClone(el); };

            return ExpandValueImpl<V>()(v, expandOrClone);
        }

        template <typename T>
        std::unique_ptr<T> ExpandMembers(const T& e)
        {
            auto o = make_empty<T>();

            o->attributes() = e.attributes();
            o->meta() = e.meta();

            if (!e.empty()) {
                o->set(ExpandValue(e.get()));
            }

            return std::move(o);
        }

        template <typename T>
        std::unique_ptr<IElement> ExpandNamedType(const T& e)
        {

            // Look for Circular Reference thro members
            if (std::find(members.begin(), members.end(), e.element()) != members.end()) {
                // To avoid unfinised recursion just clone
                const IElement* root = FindRootAncestor(e.element(), registry);

                // FIXME: if not found root
                assert(root);

                auto result = clone(*root, IElement::cMeta | IElement::cAttributes | IElement::cNoMetaId);

                result->meta().set("ref", make_primitive(e.element()));

                return std::move(result);
            }

            members.push_back(e.element());

            auto extend = ExpandMembers(*GetInheritanceTree(e.element(), registry));

            CopyMetaId(*extend, e);

            members.pop_back();

            auto origin = ExpandMembers(e);
            origin->meta().erase("id");
            extend->get().push_back(std::move(origin));

            return std::move(extend);
        }

        std::unique_ptr<RefElement> ExpandReference(const RefElement& e)
        {
            auto ref = clone(e);
            const auto& symbol = ref->get().symbol();

            if (symbol.empty()) {
                return ref;
            }

            if (std::find(members.begin(), members.end(), symbol) != members.end()) {

                std::stringstream msg;
                msg << "named type '";
                msg << symbol;
                msg << "' is circularly referencing itself by mixin";

                throw snowcrash::Error(msg.str(), snowcrash::MSONError);
            }

            members.push_back(symbol);

            if (auto referenced = registry.find(symbol)) {
                auto expanded = ExpandOrClone(std::move(referenced));
                MetaIdToRef(*expanded);
                ref->attributes().set("resolved", std::move(expanded));
            }

            members.pop_back();

            return std::move(ref);
        }
    };

    template <typename T, typename V = typename T::ValueType, bool IsIterable = data::is_iterable<V>::value>
    struct ExpandElement {
        std::unique_ptr<IElement> operator()(const T& e, ExpandVisitor::Context* context)
        {
            if (!isReserved(e.element().c_str())) { // expand named type
                return context->ExpandNamedType(e);
            }
            return nullptr;
        }
    };

    template <>
    struct ExpandElement<RefElement, RefElement::ValueType, false> {
        std::unique_ptr<IElement> operator()(const RefElement& e, ExpandVisitor::Context* context)
        {
            return context->ExpandReference(e); // expand reference
        }
    };

    template <typename T>
    struct ExpandElement<T, data::select_t, true> {
        std::unique_ptr<IElement> operator()(const T& e, ExpandVisitor::Context* context)
        {
            if (!Expandable(e)) { // do we have some expandable members?
                return nullptr;
            }

            auto o = make_element<T>();
            auto& content = o->get();

            o->meta() = e.meta(); // clone

            for (const auto& opt : e.get()) {
                content.push_back(std::unique_ptr<OptionElement>(
                    static_cast<OptionElement*>(context->ExpandOrClone(opt.get()).release())));
            }

            return std::move(o);
        }
    };

    template <typename T, typename V>
    struct ExpandElement<T, V, true> {
        std::unique_ptr<IElement> operator()(const T& e, ExpandVisitor::Context* context)
        {
            if (!Expandable(e)) { // do we have some expandable members?
                return nullptr;
            }

            std::string en = e.element();

            if (!isReserved(en.c_str())) { // expand named type
                return context->ExpandNamedType(e);
            } else { // walk throught members and expand them
                return context->ExpandMembers(e);
            }
        }
    };

    template <typename T>
    struct ExpandElement<T, data::member_t, false> {
        std::unique_ptr<IElement> operator()(const T& e, ExpandVisitor::Context* context)
        {
            if (!Expandable(e)) {
                return nullptr;
            }

            auto expanded = clone(e, IElement::cAll ^ IElement::cValue);

            expanded->set(
                data::member_t{ context->ExpandOrClone(e.get().key()), context->ExpandOrClone(e.get().value()) });

            return std::move(expanded);
        }
    };

    template <typename T>
    inline std::unique_ptr<IElement> Expand(const T& e, ExpandVisitor::Context* context)
    {
        return ExpandElement<T>()(e, context);
    }

    ExpandVisitor::ExpandVisitor(const Registry& registry) : result(nullptr), context(new Context(registry, this)){};

    ExpandVisitor::~ExpandVisitor()
    {
        delete context;
    }

    void ExpandVisitor::operator()(const IElement& e)
    {
        VisitBy(e, *this);
    }

    // do nothing, DirectElements are not expandable
    void ExpandVisitor::operator()(const HolderElement& e) {}

    // do nothing, NullElements are not expandable
    void ExpandVisitor::operator()(const NullElement& e) {}

    VISIT_IMPL(String)
    VISIT_IMPL(Number)
    VISIT_IMPL(Boolean)
    VISIT_IMPL(Member)
    VISIT_IMPL(Array)
    VISIT_IMPL(Enum)
    VISIT_IMPL(Object)
    VISIT_IMPL(Ref)
    VISIT_IMPL(Extend)
    VISIT_IMPL(Option)
    VISIT_IMPL(Select)

    std::unique_ptr<IElement> ExpandVisitor::get()
    {
        return std::move(result);
    }

}; // namespace refract

#undef VISIT_IMPL
