//
//  refract/PrintVisitor.cc
//  librefract
//
//  Created by Vilibald Wanča on 09/11/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include <cassert>
#include <iostream>
#include "VisitorUtils.h"

#include "PrintVisitor.h"
#include "Visitor.h"

namespace refract
{

    namespace
    {
        std::ostream& operator<<(std::ostream& out, const data::string_t& obj)
        {
            out << '"';
            out << obj.get();
            out << '"';
            return out;
        }

        std::ostream& operator<<(std::ostream& out, const data::number_t& obj)
        {
            out << obj.get();
            return out;
        }

        std::ostream& operator<<(std::ostream& out, const data::bool_t& obj)
        {
            out << obj.get();
            return out;
        }

        std::ostream& operator<<(std::ostream& out, const data::ref_t& obj)
        {
            out << "&[";
            out << obj.symbol();
            out << ']';
            return out;
        }
    }

    PrintVisitor::PrintVisitor() : indent(0), os(std::cerr), ommitSourceMap(false) {}

    PrintVisitor::PrintVisitor(int indent_, std::ostream& os_, bool ommitSourceMap_)
        : indent(indent_), os(os_), ommitSourceMap(ommitSourceMap_)
    {
    }

    std::ostream& PrintVisitor::indented()
    {
        for (int i = 0; i < indent; ++i) {
            os << "  ";
        }
        return os;
    }

    void PrintVisitor::printMeta(const IElement& e)
    {
        indented() << "- <meta>\n";

        PrintVisitor renderer{ indent + 1, os, ommitSourceMap };
        for (const auto& m : e.meta()) {
            renderer.indented() << "- `" << m.first << "`\n";

            assert(m.second);
            renderer(*m.second);
        }
    }

    void PrintVisitor::printAttr(const IElement& e)
    {
        indented() << "- <attr>\n";

        PrintVisitor renderer{ indent + 1, os, ommitSourceMap };
        for (const auto& a : e.attributes()) {
            if (a.first == "sourceMap")
                continue;

            renderer.indented() << "- `" << a.first << "`\n";

            assert(a.second);
            renderer(*a.second);
        }
    }

    void PrintVisitor::operator()(const IElement& e)
    {
        indented() << "+ " << e.element() << '\n';

        PrintVisitor pv{ indent + 1, os, ommitSourceMap };

        pv.printMeta(e);
        pv.printAttr(e);

        refract::VisitBy(e, pv);
    }

    void PrintVisitor::operator()(const NullElement& e)
    {
        indented() << "- Null\n";
    }

    void PrintVisitor::operator()(const HolderElement& e)
    {
        indented() << "- Holder[" << e.element() << "]\n";

        assert(!e.empty());
        assert(e.get().data());
        PrintVisitor{ indent + 1, os, ommitSourceMap }(*e.get().data());
    }

    void PrintVisitor::operator()(const StringElement& e)
    {
        assert(!e.empty());
        indented() << "- String " << e.get() << '\n';
    }

    void PrintVisitor::operator()(const NumberElement& e)
    {
        assert(!e.empty());
        indented() << "- Number " << e.get() << '\n';
    }

    void PrintVisitor::operator()(const BooleanElement& e)
    {
        assert(!e.empty());
        indented() << "- Boolean " << e.get() << '\n';
    }

    void PrintVisitor::operator()(const RefElement& e)
    {
        assert(!e.empty());
        indented() << "- RefElement " << e.get() << '\n';
    }

    void PrintVisitor::operator()(const MemberElement& e)
    {
        assert(!e.empty());
        const auto& content = e.get();

        indented() << "- MemberElement\n";

        {
            PrintVisitor renderer{ indent + 1, os, ommitSourceMap };

            const auto keyPtr = content.key();
            assert(keyPtr);
            renderer(*keyPtr);

            const auto valuePtr = content.value();
            assert(valuePtr);
            renderer(*valuePtr);
        }
    }

    void PrintVisitor::operator()(const ArrayElement& e)
    {
        printValues(e, "Array");
    }

    void PrintVisitor::operator()(const EnumElement& e)
    {
        indented() << "- EnumElement "
                   << "\n";

        if (!e.empty() && e.get().value()) {
            PrintVisitor{ indent + 1, os, ommitSourceMap }(*e.get().value());
        }
    }

    void PrintVisitor::operator()(const ObjectElement& e)
    {
        printValues(e, "Object");
    }

    void PrintVisitor::operator()(const ExtendElement& e)
    {
        printValues(e, "Extend");
    }

    void PrintVisitor::operator()(const OptionElement& e)
    {
        printValues(e, "Option");
    }

    void PrintVisitor::operator()(const SelectElement& e)
    {
        printValues(e, "Select");
    }

    void PrintVisitor::Visit(const IElement& e)
    {
        PrintVisitor ps;
        refract::Visit(ps, e);
    }

}; // namespace refract

#undef VISIT_IMPL
