#include "RefractSourceMap.h"

static refract::IElement* CharacterRangeToRefract(const mdp::CharactersRange& sourceMap)
{
    refract::ArrayElement* range = new refract::ArrayElement;

    range->push_back(refract::Create(sourceMap.location));
    range->push_back(refract::Create(sourceMap.length));

    return range;
}

refract::IElement* drafter::SourceMapToRefract(const mdp::CharactersRangeSet& sourceMap)
{
    refract::ArrayElement* sourceMapElement = new refract::ArrayElement;
    sourceMapElement->element(SerializeKey::SourceMap);

    refract::ArrayElement::ValueType ranges;
    std::transform(sourceMap.begin(), sourceMap.end(), std::back_inserter(ranges), CharacterRangeToRefract);

    sourceMapElement->set(ranges);

    refract::ArrayElement* element = new refract::ArrayElement;
    element->push_back(sourceMapElement);

    return element;
}
