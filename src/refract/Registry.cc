//
//  refract/Registry.cc
//  librefract
//
//  Created by Jiri Kratochvil on 21/05/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "Registry.h"
#include "Element.h"
#include "SerializeCompactVisitor.h"
#include "TypeQueryVisitor.h"

#include <algorithm>

namespace refract
{

    const IElement* FindRootAncestor(const std::string& name, const Registry& registry)
    {
        const IElement* parent = registry.find(name);

        while (parent && !isReserved(parent->element())) {
            const IElement* next = registry.find(parent->element());

            if (!next || (next == parent)) {
                return parent;
            }

            parent = next;
        }

        return parent;
    }

    std::string Registry::getElementId(IElement& element)
    {
        auto it = element->meta().find("id");

        if (it == element->meta().end()) {
            throw LogicError("Element has no ID");
        }

        // FIXME: remove dependecy on SosSerializeCompactVisitor
        SosSerializeCompactVisitor v;
        VisitBy(it->second, v);

        if (const StringElement* s = TypeQueryVisitor::as<const StringElement>(it->second)) {
            return s->value;
        }

        throw LogicError("Value of element meta 'id' is not StringElement");
    }

    const IElement* Registry::find(const std::string& name) const
    {
        auto i = registrated.find(name);

        if (i == registrated.end()) {
            return nullptr;
        }

        return i->second.get();
    }

    bool Registry::add(std::unique_ptr<IElement> element)
    {
        auto it = element->meta().find("id");

        if (it == element->meta().end()) {
            throw LogicError("Element has no ID");
        }

        std::string id = getElementId(element);

        if (isReserved(id)) {
            throw LogicError("You can not register a basic element");
        }

        if (find(id)) {
            // there is already already element with given name
            return false;
        }

        registrated[id] = std::move(element);
        return true;
    }

    bool Registry::remove(const std::string& name)
    {
        auto i = registrated.find(name);

        if (i == registrated.end()) {
            return false;
        }

        registrated.erase(i);
        return true;
    }

    void Registry::clearAll(bool releaseElements)
    {
        registrated.clear();
    }
}; // namespace refract
