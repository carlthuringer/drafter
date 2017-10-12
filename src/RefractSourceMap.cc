#include "RefractSourceMap.h"

using namespace refract;

namespace
{
    std::unique_ptr<IElement> CharacterRangeToRefract(const mdp::CharactersRange& sourceMap)
    {
        auto range = make_element<ArrayElement>();
        auto& content = range->get();

        content.push_back(make_primitive(static_cast<double>(sourceMap.location)));
        content.push_back(make_primitive(static_cast<double>(sourceMap.length)));

        return range;
    }
}

std::unique_ptr<IElement> drafter::SourceMapToRefract(const mdp::CharactersRangeSet& sourceMap)
{
    auto sourceMapElement = make_element<ArrayElement>();
    sourceMapElement->element(SerializeKey::SourceMap);

    std::transform(
        sourceMap.begin(), sourceMap.end(), std::back_inserter(sourceMapElement->get()), CharacterRangeToRefract);

    return CreateArrayElement(std::move(sourceMapElement));
}
