#ifndef PATTERN_H
#define PATTERN_H

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "Regex.h"
#include "Match.h"
#include "AnchorPattern.h"
#include "Token.h"
#include "Property.h"
#include "BaseProperty.h"
#include "ConditionalAssignment.h"

class Pattern {
public:
    Pattern(std::string name, std::string id, AnchorPattern& anchor, size_t precedence);
    Pattern(std::string name, std::string id, AnchorPattern& anchor, std::vector<std::shared_ptr<Token>> tokens, std::vector<std::string> asserts, std::vector<std::shared_ptr<BaseProperty>> properties, size_t precedence, Match::Type type);


    void add_token(std::string name, std::string ptn, ValueType type, size_t idx);
    void add_conditional_assignment(std::shared_ptr<ConditionalAssignment> assignment) {
        mDynamics.push_back(assignment);
    }

    bool match(Match& m, bool anchor = true);
    std::vector<std::shared_ptr<Token>>& get_tokens() {return mTokens;}

    std::vector<std::shared_ptr<BaseProperty>> get_properties() {
        std::vector<std::shared_ptr<BaseProperty>> props{};

        for( auto& p : mProperties) {
            props.push_back(p);
        }

        for(auto dn : mDynamics) {
            if(dn->evaluate()) {
                auto vec = dn->get_results();
                for(auto d : vec) {
                    props.push_back(d);
                }
            }
        }

        return props;
    }


    size_t num_tokens() {return mTokens.size();}
    size_t precedence() {return mPrecedence;}
    void reset();
    bool matched();

    void validate();
    void print_tokens();

    std::string name() {return mName;}
    std::string id() {return mId;}
    std::string event() {return mName;}

    std::string get_anchor_regex_pattern() {return mAnchor.get_regex_pattern();}

private:
    std::string mName;
    std::string mId;
    size_t mPrecedence;
    AnchorPattern mAnchor;
    std::vector<std::shared_ptr<Token>> mTokens;
    std::vector<std::string> mTkAsserts;
    bool mMatched;
    Match::Type mMatchType;
    std::vector<std::shared_ptr<BaseProperty>> mProperties;
    std::vector<std::shared_ptr<ConditionalAssignment>> mDynamics;

};

#endif//PATTERN_H