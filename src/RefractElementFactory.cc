#include "RefractElementFactory.h"
#include "refract/Element.h"
#include "ElementData.h"

#include "Serialize.h"        // LiteralTo<>
#include "SourceAnnotation.h" // mson::Error

namespace drafter
{

    template <typename E, typename IsPrimitive = typename IsPrimitive<E>::type>
    struct RefractElementFactoryImpl;

    template <typename E>
    struct RefractElementFactoryImpl<E, std::true_type> : RefractElementFactory {

        typedef typename E::ValueType ValueType;

        RefractElementFactoryImpl() {}

        virtual refract::IElement* Create(const std::string& literal, FactoryCreateMethod method) const
        {
            E* element = new E;

            if (literal.empty()) {
                return element;
            }

            switch (method) {
                case eSample: {
                    refract::ArrayElement* samples = new refract::ArrayElement;
                    std::pair<bool, ValueType> value = LiteralTo<ValueType>(literal);
                    if (value.first) {
                        samples->push_back(refract::IElement::Create(value.second));
                    }
                    element->attributes[SerializeKey::Samples] = samples;
                } break;

                case eValue: {
                    std::pair<bool, ValueType> value = LiteralTo<ValueType>(literal);
                    if (value.first) {
                        element->set(value.second);
                    }
                } break;

                case eElement:
                    element->element(literal);
                    break;
            }

            return element;
        }
    };

    template <typename E>
    struct RefractElementFactoryImpl<E, std::false_type> : RefractElementFactory {

        RefractElementFactoryImpl() {}

        virtual refract::IElement* Create(const std::string& literal, FactoryCreateMethod method) const
        {
            if (method == eSample) {
                refract::StringElement* element = new refract::StringElement;
                element->element(SerializeKey::Generic);
                element->set(literal);
                return element;
            }

            E* element = new E;

            if (!literal.empty()) {
                element->element(literal);
            }

            return element;
        }
    };

    const RefractElementFactory& FactoryFromType(const mson::BaseTypeName typeName)
    {

        static const RefractElementFactoryImpl<refract::BooleanElement> boolFactory;
        static const RefractElementFactoryImpl<refract::NumberElement> numberFactory;
        static const RefractElementFactoryImpl<refract::StringElement> stringFactory;
        static const RefractElementFactoryImpl<refract::EnumElement> enumFactory;
        static const RefractElementFactoryImpl<refract::ArrayElement> arrayFactory;
        static const RefractElementFactoryImpl<refract::ObjectElement> objectFactory;

        switch (typeName) {
            case mson::BooleanTypeName:
                return boolFactory;
            case mson::NumberTypeName:
                return numberFactory;
            case mson::StringTypeName:
                return stringFactory;
            case mson::ArrayTypeName:
                return arrayFactory;
            case mson::EnumTypeName:
                return enumFactory;
            case mson::ObjectTypeName:
            case mson::UndefinedTypeName:
                return objectFactory;
            default:; // do nothing
        }

        throw snowcrash::Error("unknown mson type", snowcrash::ApplicationError);
    }
}
