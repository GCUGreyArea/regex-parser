#ifndef PROPERTY_VALUE_H
#define PROPERTY_VALUE_H

#include "BaseProperty.h"

class PropertyValue : public BaseProperty {
public:
    PropertyValue(std::shared_ptr<Variable> v)
        : mVariable(v) {}

    virtual std::string name()  {return mVariable->name();}
    virtual std::string value() {return mVariable->as_string();}

private:
    std::shared_ptr<Variable> mVariable;
};

#endif//PROPERTY_VALUE_H