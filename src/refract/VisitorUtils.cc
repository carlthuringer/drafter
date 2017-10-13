//
//  refract/VisitorUtils.cc
//  librefract
//
//  Created by Vilibald Wanča on 09/11/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//
#include "Element.h"
#include "VisitorUtils.h"

namespace refract
{

    const StringElement* GetDescription(const IElement& e)
    {
        auto i = e.meta().find("description");

        if (i == e.meta().end()) {
            return nullptr;
        }

        return TypeQueryVisitor::as<const StringElement>(i->second.get());
    }

    std::string GetKeyAsString(const MemberElement& e)
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
}
