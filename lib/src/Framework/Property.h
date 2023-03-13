#ifndef PROPERTY_H
#define PROPERTY_H


#include "BaseProperty.h"

class Property : public BaseProperty {
public:
    Property(std::vector<std::shared_ptr<Token>>& tokens, std::string name, std::string delim)
        : mTokens(tokens)
        , mName(name)
        , mDelim(delim)
        , mValue("") {}


    Property(const Property& p)
        : mTokens(p.mTokens)
        , mName(p.mName)
        , mDelim(p.mDelim)
        , mValue(p.mValue) {}


    virtual ~Property() {}

    void reset() {mValue = "";}
    virtual std::string name() {return mName;}
    virtual std::string value() {
        if(mValue == "") {
            size_t size = mTokens.size();
            for(size_t i=0;i<size;i++) {
                auto& t = mTokens.at(i);
                mValue.append(t->as_string());
                if(i+1<size)
                    mValue.append(mDelim);
            }
        }

        return mValue;
    }

private:
    std::vector<std::shared_ptr<Token>> mTokens;
    std::string mName;
    std::string mDelim;
    std::string mValue;
};


#endif // PROPERTY_H
