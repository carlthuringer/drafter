//
//  refract/IsExpandableVisitor.h
//  librefract
//
//  Created by Jiri Kratochvil on 17/06/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#ifndef REFRACT_ISEXPANDABLEVISITOR_H
#define REFRACT_ISEXPANDABLEVISITOR_H

namespace refract
{

    class IsExpandableVisitor
    {

        bool result;

    public:
        IsExpandableVisitor();

        template <typename T>
        void operator()(const T& e);

        bool get() const;
    };

}; // namespace refract

#endif // #ifndef REFRACT_EXPANDVISITOR_H
