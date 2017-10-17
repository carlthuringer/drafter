//
//  refract/VisitorUtils.cc
//  librefract
//
//  Created by Vilibald Wanča on 09/11/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//
#include "Element.h"
#include "VisitorUtils.h"

using namespace refract;

const StringElement* refract::GetDescription(const IElement& e)
{
    auto i = e.meta().find("description");

    if (i == e.meta().end()) {
        return nullptr;
    }

    return TypeQueryVisitor::as<const StringElement>(i->second.get());
}

std::string refract::GetKeyAsString(const MemberElement& e)
{

    auto element = e.get().key();

    if (auto str = TypeQueryVisitor::as<const StringElement>(element)) {
        return str->get();
    }

    if (auto ext = TypeQueryVisitor::as<const ExtendElement>(element)) {
        auto merged = ext->get().merge();

        if (auto str = TypeQueryVisitor::as<const StringElement>(merged.get())) {

            std::string key = str->get();
            if (key.empty()) {
                auto k = GetValue<const StringElement>()(*str);
                if (k) {
                    key = k->get();
                }
            }

            return key;
        }
    }

    return std::string();
}

MemberElement* refract::FindMemberByKey(const ObjectElement& e, const std::string& name)
{
    auto it = std::find_if(e.get().begin(), e.get().end(), [&name](const auto& el) {
        ComparableVisitor cmp(name, ComparableVisitor::key);
        VisitBy(*el, cmp);
        return cmp.get();
    });

    if (it == e.get().end()) {
        return nullptr;
    }

    return static_cast<MemberElement*>(it->get());
}
