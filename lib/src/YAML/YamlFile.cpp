#include "YamlFile.h"

#include "Framework/Pattern.h"
#include "Framework/Token.h"
#include "Framework/AnchorPattern.h"
#include "Framework/Regex.h"
#include "Framework/Pattern.h"
#include "Exception/Exceptions.h"
#include "Utils/Utils.h"

void YamlFile::readFile()
{
    mFileNode = YAML::LoadFile(mFileName);

    RuleDesc rd = {};
    rd.id = mFileNode["id"].as<std::string>();
    rd.name = mFileNode["name"].as<std::string>();

    // Iterate through the tokens first so that they are available
    const YAML::Node &tokens = mFileNode["tokens"];
    for (YAML::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        const YAML::Node &token = *it;
        RuleDesc::TokenDesc td;

        td.name = token["name"].as<std::string>();
        td.regex = token["regex"].as<std::string>();
        td.type = token["type"].as<std::string>();

        rd.tokens.emplace(td.name, td);
    }

    const YAML::Node &p = mFileNode["patterns"];

    // Iterate through all the patterns
    for (YAML::const_iterator it = p.begin(); it != p.end(); ++it)
    {
        RuleDesc::PatternDesc ptd = {};
        const YAML::Node &pattern = *it;

        ptd.id = pattern["id"].as<std::string>();
        ptd.name = pattern["name"].as<std::string>();
        ptd.precedence = pattern["precedence"].as<size_t>();
        ptd.type = pattern["type"].as<std::string>();
        ptd.example = pattern["example"].as<std::string>();
        ptd.match_type = pattern["match"].as<std::string>();

        if (ptd.type == "root")
        {
            ptd.anchor = pattern["anchor"].as<std::string>();
        }
        else if (ptd.type == "extends")
        {
            ptd.extends = pattern["extends"].as<std::string>();
        }
        else
        {
            throw Exception::UnknownTypeValue(ptd.type);
        }

        std::vector<std::string> tokens_list{};
        std::vector<RuleDesc::PropDesc> prop_list {};

        if (ptd.type == "extends")
        {
            RuleDesc::PatternDesc ptd_next = ptd;

            // Get all the derived tokens first
            while (ptd_next.type != "root")
            {
                auto pit = rd.patterns.find(ptd_next.extends);
                if (pit != rd.patterns.end())
                {
                    ptd_next = pit->second;
                    for (auto t : ptd_next.tokens)
                    {
                        tokens_list.push_back(t);
                    }

                    for(auto p : ptd_next.properties) {
                        prop_list.push_back(p);
                    }
                }
                else
                {
                    std::cout << "Couldn't find pattern for " << ptd_next.id << std::endl;
                    throw Exception::PatternNotFound(ptd_next.extends);
                }
            }

            // Set the anchor for this pattern to the root anchor pattern
            ptd.anchor = ptd_next.anchor;
        }

        // Now get all the declared tokens
        const YAML::Node &tokens = pattern["tokens"];

        // This is a list of tokens that the pattern declares
        for (YAML::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
        {
            std::string token = it->as<std::string>();
            tokens_list.push_back(token);
        }

        ptd.tokens.assign(tokens_list.begin(), tokens_list.end());
        ptd.properties.assign(prop_list.begin(), prop_list.end());

        YAML::Node asserts = pattern["assert"];
        for (YAML::iterator it = asserts.begin(); it != asserts.end(); ++it)
        {
            const YAML::Node as = *it;

            std::string token = as["token"].as<std::string>();
            std::string assert = as["value"].as<std::string>();

            ptd.asserts.emplace(token, assert);
        }

        YAML::Node properties = pattern["properties"];
        for (YAML::iterator it = properties.begin(); it != properties.end(); ++it)
        {
            const YAML::Node &pr = *it;
            RuleDesc::PropDesc pd;

            try
            {
                pd.name = pr["name"].as<std::string>();
                pd.delim = pr["deliminator"].as<std::string>();

                const YAML::Node &tokens = pr["tokens"];
                for (YAML::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
                {
                    std::string token = it->as<std::string>();
                    pd.tokens.push_back(token);
                }
            }
            catch (std::exception e)
            {
                // This must be a dynamic property
                pd.name = "dynamic";
                auto expression = pr["dynamic"].as<std::string>();
                pd.expressions.push_back(expression);
            }

            ptd.properties.push_back(pd);
        }

        rd.patterns.emplace(ptd.id, ptd);
    }

    // Iterate through the anchors
    YAML::Node anchors = mFileNode["anchors"];
    for (YAML::const_iterator it = anchors.begin(); it != anchors.end(); ++it)
    {
        const YAML::Node &anchor = *it;
        RuleDesc::AnchorDesc ad = {};
        ad.name = anchor["name"].as<std::string>();
        ad.id = anchor["id"].as<std::string>();

        const YAML::Node &patterns = anchor["patterns"];
        for (YAML::const_iterator it = patterns.begin(); it != patterns.end(); ++it)
        {
            std::string astr = it->as<std::string>();
            ad.regex.push_back(astr);
        }

        rd.anchors.emplace(ad.id, ad);
    }

    // Create the patterns for the rule
    std::vector<Pattern> patterns{};
    for (auto &p : rd.patterns)
    {
        std::shared_ptr<Pattern> ptn = build_pattern(p.second.id, rd);
        patterns.push_back(*ptn);
    }

    // Sort the vector by pattern size then precedence
    sort_patterns(patterns);

    mRule = std::make_unique<Rule>(rd.id, rd.name, patterns, rd.tk_obj_list);
}

std::unique_ptr<AnchorPattern> YamlFile::build_anchor(std::string id, RuleDesc &rd)
{
    auto ait = rd.anchors.find(id);
    if (ait != rd.anchors.end())
    {
        std::vector<Regex> regex;
        auto an = ait->second;
        for (auto r : an.regex)
        {
            Regex rx(r);
            regex.push_back(rx);
        }
        if (regex.size() == 0)
            throw Exception::NoAnchorPatterns(an.id);

        return std::make_unique<AnchorPattern>(an.name, an.id, regex);
    }

    throw Exception::AnchorNotFound(id);
}


void YamlFile::build_pattern_properties(RuleDesc::PatternDesc& p, RuleDesc &rd, std::vector<std::shared_ptr<BaseProperty>> &properties, std::vector<std::shared_ptr<ConditionalAssignment>>& assignments)
{
    for (auto& prop : p.properties)
    {
        if (prop.tokens.size() > 0)
        {
            std::vector<std::shared_ptr<Token>> tokens;
            for (auto t : prop.tokens) {

                auto toit = rd.tk_obj_map.find(t);
                if (toit == rd.tk_obj_map.end())
                    throw Exception::TokenNotFound(t);

                tokens.push_back(toit->second);
            }

            std::shared_ptr<BaseProperty> base = std::make_shared<Property>(tokens, prop.name, prop.delim);
            rd.prop_obj_map.emplace(base->name(),base);
            properties.push_back(base);

            continue;
        }
        else if(prop.expressions.size() > 0) {
            for(auto expression : prop.expressions) {
                auto dyn = parse_expression(expression,rd);
                if(dyn)
                    p.dynamic.push_back(dyn);
            }
        }
    }
}

void YamlFile::build_pattern_tokens(RuleDesc::PatternDesc& p, RuleDesc &rd, std::vector<std::shared_ptr<Token>> &tokens)
{
    for (auto t : p.tokens)
    {
        auto tit = rd.tokens.find(t);
        if (tit == rd.tokens.end())
            throw Exception::TokenNotFound(t);

        std::shared_ptr<Token> token = nullptr;

        auto toit = rd.tk_obj_map.find(t);
        if (toit == rd.tk_obj_map.end())
        {
            auto tk = tit->second;
            ValueType type(tk.type);

            if (type == "none")
                throw Exception::UnknownTypeValue(tk.type);

            token = std::make_shared<Token>(tk.name, tk.regex, type, tokens.size());
            rd.tk_obj_map.emplace(t, token);
            rd.tk_obj_list.push_back(token);
        }
        else
        {
            token = toit->second;
        }

        tokens.push_back(token);
    }
    if (tokens.size() == 0)
        throw Exception::PatternHasNoTokens(p.id);
}

std::shared_ptr<Pattern> YamlFile::build_pattern(std::string id, RuleDesc &rd)
{
    auto pit = rd.patterns.find(id);
    if (pit == rd.patterns.end())
        throw Exception::PatternNotFound(id);

    RuleDesc::PatternDesc ptn = pit->second;
    std::unique_ptr<AnchorPattern> aptn = build_anchor(ptn.anchor, rd);

    std::vector<std::shared_ptr<Token>> tokens = {};
    std::vector<std::shared_ptr<BaseProperty>> properties {};
    std::vector<std::shared_ptr<ConditionalAssignment>> assignments {};
    std::vector<size_t> token_idx = {};

    build_pattern_tokens(ptn, rd, tokens);
    build_pattern_properties(ptn, rd, properties, assignments);

    Match::Type type = Match::Type::NONE;

    if (ptn.match_type == "sequential")
        type = Match::Type::SEQUENTIAL;
    else if (ptn.match_type == "random")
        type = Match::Type::RANDOM;
    else
        throw Exception::UnknownMatchType(ptn.match_type);

    std::vector<std::string> asserts = {};

    for (auto t : ptn.tokens)
    {
        auto it = ptn.asserts.find(t);
        if (it != ptn.asserts.end())
        {
            // std::cout << "Adding [assert " << it->second << " / to pattern " << ptn.id << "]" << std::endl;
            asserts.push_back(it->second);
        }
        else
            asserts.push_back("");
    }

    // TODO: Make this into a builder?
    auto p = std::make_shared<Pattern>(ptn.name, ptn.id, *aptn, tokens, asserts, properties, ptn.precedence, type);

    for(auto ds : ptn.dynamic) {
        p->add_conditional_assignment(ds);
    }
    return p;
}

/**
 * @brief Sort token list first by number of tokens, then precedence
 * The more specific pattern should be tried first!
 *
 * @param patterns
 */
void YamlFile::sort_patterns(std::vector<Pattern> patterns)
{
    std::sort(patterns.begin(), patterns.end(), [](Pattern &lhs, Pattern &rhs)
              {
        if(lhs.num_tokens() == rhs.num_tokens()) {
            if(lhs.precedence() > rhs.precedence()){
                return false;
            }
        }
        else if(lhs.num_tokens() < rhs.num_tokens()){
            return false;
        }

        return true; });
}

/**
 * @brief Parse the embedded expression into an intermediate structure
 * used at build time to construct a ConditionalAssignment object
 *
 * In the intermediary structure ORs in the IF clause are represented by a list of lists
 * of conditional boolean statements.
 * the expression A op B AND C op D OR E op F would look like [A op B,C op D],[E op F].
 *
 * For simplicity the parser and lexer rely on significan and useful spacing in the expression
 * the expression "if a == 1 and b > 2 then c = 12" is legal
 * the expression "if a==1 then c=12" is not
 *
 * Expressions cannot yet deal with property values based on property values
 *
 * @param dynamic
 * @param rd
 * @param prop
 * @return true
 * @return false
 */
std::shared_ptr<ConditionalAssignment> YamlFile::parse_expression(std::string dynamic, RuleDesc &rd)
{
    class Reference {
    public:
        Reference()
            : mTk(nullptr)
            , mIntValid(false)
            , mInt(0)
            , mRef("") {}

        void clear() {
            mTk = nullptr;
            mIntValid = false;
            mInt = 0;
            mRef = "";
        }

        void set(int val) {
            mIntValid = true;
            mInt = val;
        }

        void set(std::shared_ptr<Token> val) {
           mTk = val;
        }

        void set(std::string val) {
            mRef = val;
        }

        Condition make_condition(std::shared_ptr<Token> rhs, std::string op) {
            BaseValue rhsVal(rhs);
            Condition cn;
            if(mTk != nullptr) {
                BaseValue lhs(mTk);
                cn = {lhs,op,rhsVal};
            }
            else if(mIntValid) {
                BaseValue lhs(mInt);
                cn = {lhs,op,rhsVal};
            }
            else {
                BaseValue lhs(mRef);
                cn = {lhs,op,rhsVal};
            }

            clear();
            return cn;
        }

        Condition make_condition(int rhs, std::string op) {
            BaseValue rhsVal(rhs);
            Condition cn;
            if(mTk != nullptr) {
                BaseValue lhs(mTk);
                cn = {lhs,op,rhsVal};
            }
            else if(mIntValid) {
                BaseValue lhs(mInt);
                cn = {lhs,op,rhsVal};
            }
            else {
                BaseValue lhs(mRef);
                cn = {lhs,op,rhsVal};
            }

            clear();
            return cn;
        }

        Condition make_condition(std::string rhs, std::string op) {
            BaseValue rhsVal(rhs);
            Condition cn;
            if(mTk != nullptr) {
                BaseValue lhs(mTk);
                cn = {lhs,op,rhsVal};
            }
            else if(mIntValid) {
                BaseValue lhs(mInt);
                cn = {lhs,op,rhsVal};
            }
            else {
                BaseValue lhs(mRef);
                cn = {lhs,op,rhsVal};
            }

            clear();
            return cn;
        }

        bool valid() {
            if(mTk != nullptr || mIntValid || mRef != "")
                return true;
            return false;
        }
    private:
        std::shared_ptr<Token> mTk;
        bool mIntValid;
        int mInt;
        std::string mRef;
    };

    enum Bracket {
        NOT_BRACKET,
        OPEN_BRACKET,
        CLOSE_BRACKET
    } bracket_state = NOT_BRACKET;

    enum String {
        NOT_STR,
        OPEN_STR,
        CLOSE_STR
    } str_state = NOT_STR;

    enum ClauseState {
        NONE,
        IF,
        THEN,
        ELSE
    } clause_state = NONE;

    enum ExpState {
        EXP_NONE,
        EXP_STRING,
        EXP_NUMBER,
        EXP_PROP,
        EXP_FIELD,
        EXP_OP
    } exp_state = EXP_NONE;


    bool complete = false;
    bool space = false;

    std::vector<char> brk_stack {};

    std::string word="";

    Reference lhs;
    std::string op = "";

    Condition condition;

    std::string conj = "";


    // Condition current = nullptr;
    std::shared_ptr<BoolExpression> boolExp = nullptr;
    std::vector<std::shared_ptr<BoolExpression>> cnd_stack {};
    // Last known conjunctions
    std::vector<std::string> conj_stack {};


    // Variables for then and else expression
    std::vector<std::shared_ptr<Assignment>> thenEx {};
    std::vector<std::shared_ptr<Assignment>> elseEx {};

    std::vector<std::shared_ptr<Variable>> vars{};
    std::vector<Assignment::Operator> ops {};

    const char *str = dynamic.data();
    while (*str != '\0') {
        switch (*str) {
        // Significant alpha numeric characters
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '0' ... '9':
        case '_':
        case '.':
        case '$':
        case '=':
        case '>':
        case '<':
        case '+':
        case '-':
        case '/':
        case '*':
        case '!':
        case '?':
            word += *str;
            break;
        case '"':
            if (str_state == OPEN_STR)
                str_state = CLOSE_STR;
            else
                str_state = OPEN_STR;
            break;
        case ' ':
            space = true;
            break;

        case '(':
            // This is where the expression node is created
            bracket_state = OPEN_BRACKET;
            brk_stack.push_back('(');

            break;
        case ')':
            // Abort if there's no matching open bracket
            if (brk_stack.size() > 0) {
                char b = brk_stack.back();
                brk_stack.pop_back();
                if (b != '(')
                    throw Exception::General("Bracket missmatch");
            }
            // This is where the node omn the expression is terminated
            bracket_state = CLOSE_BRACKET;
            break;
        }

        // TODO Deal with ordering and precedence of expressions
        if (str_state == NOT_STR && bracket_state == OPEN_BRACKET) {
            // We would get this when brackets directly follow an if statement
            // as the expression has just been made. In that instance we don't
            // want to store it!
            if(!boolExp->empty()) {
                cnd_stack.push_back(boolExp);
                conj_stack.push_back(conj);
                boolExp = std::make_shared<BoolExpression>();
            }
            bracket_state = NOT_BRACKET;
            complete=false;
        }
        else if (str_state == NOT_STR && bracket_state == CLOSE_BRACKET) {
            // Get back the last expression
            std::shared_ptr<BoolExpression> tmp = cnd_stack.back();
            // And clear it off the stack
            cnd_stack.pop_back();
            auto last_conj = conj_stack.back();

            // Now set the expression that was built as the rhs of this expression
            if(tmp->valid() && boolExp->complete())
                boolExp = std::make_shared<BoolExpression>(tmp, BoolExpression::from_string(last_conj), boolExp);
            else if(boolExp->lhs_only() && tmp->valid())
                boolExp->set_rhs(tmp,BoolExpression::from_string(last_conj));
            else
                throw Exception::General("Unknown state in parser");

            bracket_state = NOT_BRACKET;
            complete=true;
        }
        else if ((str_state == NOT_STR && space) || (str_state == NOT_STR && *(str + 1) == '\0')) // Words and operators
        {
            // If needs to be the first statement to create the structure
            if (word == "if")
            {
                clause_state = ClauseState::IF;
                boolExp = std::make_shared<BoolExpression>();
                complete=false;
            }
            else if (word == "then")
            {
                clause_state = ClauseState::THEN;
                exp_state = EXP_NONE;
            }
            else if (word == "else")
            {
                thenEx.push_back(Assignment::build(vars,ops));
                vars.clear();
                ops.clear();
                clause_state = ClauseState::ELSE;
                exp_state = EXP_NONE;
            }
            else if (word == "and")
            {
                switch (clause_state)
                {
                case ClauseState::IF:
                    break;
                case ClauseState::THEN:
                    thenEx.push_back(Assignment::build(vars,ops));
                    break;
                case ClauseState::ELSE:
                    elseEx.push_back(Assignment::build(vars,ops));
                    break;

                default:
                    throw Exception::General("UNknown state in parser");
                }

                vars.clear();
                ops.clear();

                conj = "and";
                exp_state = EXP_NONE;
            }
            else if (word == "or")
            {
                switch (clause_state)
                {
                case ClauseState::IF:
                    break;
                case ClauseState::THEN:
                case ClauseState::ELSE:
                    throw Exception::General("or is not allowed in assignment (then or else clause) expression");
                    break;

                default:
                    throw Exception::General("Unknown state in parser");
                }
                conj = "or";
                exp_state = EXP_NONE;
            }
            // Comparision operators
            else if (word == "<=" || word == ">=" || word == "?=" || word == "==" || word == "!=" || word == ">") {
                if (clause_state != ClauseState::IF)
                    throw Exception::General("Cannot use "+ word + " operator in assignment");
                else if (exp_state == EXP_OP)
                    throw Exception::General("An operator cannot be followed by another operator");

                // Save the sate and operator
                exp_state = EXP_OP;
                op = word;
            }
            // Assignment operators
            else if (word == "=" || word == "+" || word == "-" || word == "/" || word == "*") {
                if (clause_state == ClauseState::IF)
                    throw Exception::General("Cannot use assignment operator " + word + " in a conditional statement");

                if (exp_state == EXP_OP)
                    throw Exception::General("An operator cannot be followed by another operator");

                // Save the sate and operator
                exp_state = EXP_OP;
                // op = word;

                ops.push_back(Assignment::from_string(word));
            }
            else if (word != "")
            {
                unsigned int base = 0;
                auto it = rd.tk_obj_map.find(word);
                if (it != rd.tk_obj_map.end()) {
                    exp_state = EXP_FIELD;

                    switch(clause_state) {
                        case ClauseState::IF:
                            if(lhs.valid()) {
                                condition = lhs.make_condition(it->second, op);
                                if(boolExp->empty()) {
                                    boolExp->set_lhs(condition);
                                }
                                else if(complete) {
                                    boolExp = std::make_shared<BoolExpression>(condition,BoolExpression::from_string(op),boolExp);
                                    complete = true;
                                }
                                else {
                                    boolExp->set_rhs(condition,BoolExpression::from_string(op));
                                    complete = true;
                                }

                                condition.clear();
                            }
                            else {
                                lhs.set(it->second);
                            }
                            break;
                        case ClauseState::THEN:
                        case ClauseState::ELSE:
                            vars.push_back(std::make_shared<Variable>("anonymous",it->second));
                            break;

                        default:
                            throw Exception::General("Unknown state in expression parser");
                    }
                }
                else if ((base = Numbers::is_number(word)) && base != 0)
                {
                    // Literal number
                    exp_state = EXP_NUMBER;
                    int num = Numbers::to_int(word,base);
                    switch(clause_state) {
                        case ClauseState::IF:
                            if(lhs.valid()) {
                                condition = lhs.make_condition(num, op);
                                if(boolExp->empty()) {
                                    boolExp->set_lhs(condition);
                                }
                                else if(complete) {
                                    boolExp = std::make_shared<BoolExpression>(condition,BoolExpression::from_string(conj),boolExp);
                                    complete = true;
                                }
                                else {
                                    boolExp->set_rhs(condition,BoolExpression::from_string(conj));
                                    complete = true;
                                }

                                condition.clear();
                            }
                            else {
                                lhs.set(num);
                            }
                            break;
                        case ClauseState::THEN:
                        case ClauseState::ELSE:
                            vars.push_back(std::make_shared<Variable>("anonymous",num,true));
                            break;

                        default:
                            throw Exception::General("Unknown state in expression parser");
                    }
                }
                else
                {
                    // Need to test for legal property name!
                    switch (word.at(0))
                    {
                    case '0' ... '9':
                        throw Exception::General("Illegal property name, might that be a string? [" + word + "]");

                    default:
                        break;
                    }
                    // property
                    exp_state = EXP_PROP;
                    vars.push_back(std::make_shared<Variable>(word,"",false));
                }
            }

            // state = START;
            space = false;
            word = "";
        }
        else if (str_state == CLOSE_STR)
        {
            // string literal
            str_state = NOT_STR;
            switch(clause_state) {
                case ClauseState::IF:
                    if(lhs.valid()) {
                        condition = lhs.make_condition(word, op);
                        if(boolExp->empty()) {
                            boolExp->set_lhs(condition);
                        }
                        else if(complete) {
                            boolExp = std::make_shared<BoolExpression>(condition,BoolExpression::from_string(conj),boolExp);
                            complete=true;
                        }
                        else {
                            boolExp->set_rhs(condition,BoolExpression::from_string(conj));
                            complete = true;
                        }

                        condition.clear();
                    }
                    else {
                        lhs.set(word);
                    }
                    break;

                case ClauseState::THEN:
                case ClauseState::ELSE:
                    vars.push_back(std::make_shared<Variable>("anonymous",word,true));
                    break;
                default:
                    throw Exception::General("Unknown state in expression parser");
            }
            word = "";
            exp_state = EXP_STRING;
        }
        else if (str_state == OPEN_STR && space)
        {
            word += *str;
            space = false;
        }

        str++;
    }

    if (brk_stack.size() > 0)
        throw Exception::General("Bracket missmatch in parser");

    if(clause_state == ClauseState::THEN)
        thenEx.push_back(Assignment::build(vars,ops));
    else
        elseEx.push_back(Assignment::build(vars,ops));


    return std::make_shared<ConditionalAssignment>(boolExp,thenEx,elseEx);
}