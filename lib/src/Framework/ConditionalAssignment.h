#ifndef CONDITIONAL_ASSIGNMENT_H
#define CONDITIONAL_ASSIGNMENT_H
/**
 * @file ConditionalAssignment.h
 * @author Barry Robinson (barry.w.robinson64@gmail.com)
 * @brief Represents a simple if then else structure for dynamic assignment of properties
 * If the BoolExpression evaluates to true, mThen will be evaluated. If there is an mElse, that
 * expression will be evaluated if the "if" BoolExpression evalutes to false.
 * @version 1.0
 * @date 30-04-2022
 *
 * @copyright Copyright (C) Barry Robinson 2022
 *
 */

#include <vector>
#include "Assignment.h"
#include "Variable.h"
#include "BoolExpression.h"
#include "ResolvedProperty.h"

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
                mResults.push_back({v->name(), v->as_string()});
            }
        }
        else {
            for(auto& a : mElse) {
                std::shared_ptr<Variable> v = a->evaluate();
                mResults.push_back({v->name(), v->as_string()});
            }
        }

        return mRet;
    }

    /**
     * @brief Get the values object as a reference so that when it is updated
     * the list will be remade
     *
     * @return std::vector<ResolvedProperty>&
     */

    std::vector<ResolvedProperty>& get_results() {
        return mResults;
    }


    std::vector<ResolvedProperty> get_expression_properies() {
        std::vector<ResolvedProperty> props {};

        for(auto as : mThen) {
            std::shared_ptr<Variable> var = as->get_assignmet_variable();
            props.push_back({var->name(), var->as_string()});
        }

        for(auto as : mElse) {
            std::shared_ptr<Variable> var = as->get_assignmet_variable();
            props.push_back({var->name(), var->as_string()});
        }

        return props;
    }

protected:

private:
    std::shared_ptr<BoolExpression> mExp;
    std::vector<std::shared_ptr<Assignment>> mThen;     // Could be multiple assignments
    std::vector<std::shared_ptr<Assignment>> mElse;
    std::vector<ResolvedProperty> mResults;
    bool mRet;
};

#endif//CONDITIONAL_ASSIGNMENT_H
