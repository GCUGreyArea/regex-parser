#ifndef CONDITIONAL_ASSIGNMENT_H
#define CONDITIONAL_ASSIGNMENT_H
/**
 * @file ConditionalAssignment.h
 * @author Barry Robinson (barry.robinson@alertlogic.com)
 * @brief Represents a simple if then else structure for dynamic assignment of properties
 * If the BoolExpression evaluates to true, mThen will be evaluated. If there is an mElse, that
 * expression will be evaluated if the "if" BoolExpression evalutes to false.
 * @version 1.0
 * @date 30-04-2022
 *
 * @copyright Copyright (c) Alert Logic 2022
 *
 */

#include <vector>
#include "Assignment.h"
#include "Variable.h"
#include "BaseProperty.h"
#include "PropertyValue.h"
#include "BoolExpression.h"

class ConditionalAssignment {
public:
    ConditionalAssignment(std::shared_ptr<BoolExpression> ifEx, std::vector<std::shared_ptr<Assignment>> thenEx, std::vector<std::shared_ptr<Assignment>> elseEx = {})
        : mExp(ifEx)
        , mThen(thenEx)
        , mElse(elseEx)
        , mRet(false) {}


    /**
     * @brief Evaluate a list of expressions which all constitute an OR clause
     * if any of them match then execut the THEN clause. If they don't, and there is
     * an ELSE clause, execute that.
     *
     */
    bool evaluate() {
        mResults.clear();

        mRet = mExp->evaluate();

        if(mRet) {
            for(auto& a : mThen) {
                std::shared_ptr<Variable> v = a->evaluate();
                std::shared_ptr<PropertyValue> val = std::make_shared<PropertyValue>(v);

                mResults.push_back(val);
            }
        }
        else {
            for(auto& a : mElse) {
                std::shared_ptr<Variable> v = a->evaluate();
                std::shared_ptr<PropertyValue> val = std::make_shared<PropertyValue>(v);

                mResults.push_back(val);
            }
        }

        return mRet;
    }

    /**
     * @brief Get the values object as a reference so that when it is updated
     * the list will be remade
     *
     * @return std::vector<std::shared_ptr<BaseProperty>>&
     */

    std::vector<std::shared_ptr<BaseProperty>>& get_results() {
        return mResults;
    }


    std::vector<std::shared_ptr<BaseProperty>> get_expression_properies() {
        std::vector<std::shared_ptr<BaseProperty>> props {};

        for(auto as : mThen) {
            std::shared_ptr<Variable> var = as->get_assignmet_variable();
            std::shared_ptr<BaseProperty> val = std::make_shared<PropertyValue>(var);

            props.push_back(val);
        }

        for(auto as : mElse) {
            std::shared_ptr<Variable> var = as->get_assignmet_variable();
            std::shared_ptr<BaseProperty> val = std::make_shared<PropertyValue>(var);

            props.push_back(val);
        }

        return props;
    }

protected:

private:
    std::shared_ptr<BoolExpression> mExp;
    std::vector<std::shared_ptr<Assignment>> mThen;     // Could be multiple assignments
    std::vector<std::shared_ptr<Assignment>> mElse;
    std::vector<std::shared_ptr<BaseProperty>> mResults;
    bool mRet;
};

#endif//CONDITIONAL_ASSIGNMENT_H