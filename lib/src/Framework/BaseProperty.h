#ifndef BASE_PROPERTY_H
#define BASE_PROPERTY_H

class BaseProperty {
public:
    virtual std::string name() = 0;
    virtual std::string value() = 0;

    virtual ~BaseProperty() {}
};

#endif//BASE_PROPERTY_H