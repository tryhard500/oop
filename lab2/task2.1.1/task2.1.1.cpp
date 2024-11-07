/*
НГУ ММФ МКН 3 семестр
Лаборатораня работа по программированию (ООП)
Задание 2, Задача "2.1.1 (усложнённый) Символьное дифференцирование + Доп. задание"
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stack>

class Expression
{
private:
    static std::map<std::string, int> parse_eval(const std::string &meaning)
    {
        std::map<std::string, int> result;
        size_t i = 0;
        std::string var;
        int value = 0;

        while (i < meaning.size())
        {
            while (meaning[i] != ' ')
            {
                var += meaning[i];
                i++;
            }

            i += 4;

            while (meaning[i] != ';' && i < meaning.size())
            {
                value = value * 10 + (meaning[i] - '0');
                i++;
            }

            result[var] = value;
            var = "";
            value = 0;

            i += 2;
        }
        return result;
    }

public:
    virtual void print(std::ostream &os) const = 0;
    virtual Expression *derivative(const std::string &var) const = 0;
    virtual int eval(const std::map<std::string, int> &vars) const = 0;
    virtual Expression *eval() const = 0;
    int eval(const std::string &meaning)
    {
        std::map<std::string, int> vars = parse_eval(meaning);
        return this->eval(vars);
    }
    virtual Expression *simplify() const = 0;
    virtual ~Expression() {}
};

class Number : public Expression
{
private:
    const int value_;

public:
    Number(int value) : value_(value) {}

    void print(std::ostream &os) const override
    {
        os << value_;
    }

    Expression *derivative(const std::string &var) const override
    {
        return new Number(0);
    }

    Expression *eval() const override
    {
        return new Number(value_);
    }

    int eval(const std::map<std::string, int> &vars) const override
    {
        return value_;
    }

    Expression *simplify() const override
    {
        return new Number(value_);
    }

    int get_value()
    {
        return value_;
    }
};

class Variable : public Expression
{
private:
    const std::string name_;

public:
    Variable(std::string name) : name_(name) {}

    void print(std::ostream &os) const override
    {
        os << name_;
    }

    Expression *derivative(const std::string &var) const override
    {
        if (name_ == var)
            return new Number(1);
        else
            return new Number(0);
    }

    Expression *eval() const override
    {
        return new Variable(name_);
    }

    int eval(const std::map<std::string, int> &vars) const override
    {
        return vars.at(name_);
    }

    Expression *simplify() const override
    {
        return new Variable(name_);
    }

    std::string get_name()
    {
        return name_;
    }
};

class Add : public Expression
{
private:
    Expression *left_, *right_;

public:
    Add(Expression *left, Expression *right) : left_(left), right_(right) {}

    void print(std::ostream &os) const override
    {
        os << '(';
        left_->print(os);
        os << '+';
        right_->print(os);
        os << ')';
    }

    Expression *derivative(const std::string &var) const override
    {
        return new Add(left_->derivative(var), right_->derivative(var));
    }

    Expression *eval() const override
    {
        Expression *left_expr = left_->simplify();
        Expression *right_expr = right_->simplify();
        if (typeid(*left_expr) == typeid(Number) &&
            typeid(*right_expr) == typeid(Number))
        {
            return new Number(left_expr->eval("") + right_expr->eval(""));
        }
        return new Add(left_expr, right_expr);
    }

    int eval(const std::map<std::string, int> &vars) const override
    {
        return left_->eval(vars) + right_->eval(vars);
    }

    ~Add() override
    {
        delete left_;
        delete right_;
    }

    Expression *simplify() const override
    {
        Expression *left_expr = left_->simplify();
        Expression *right_expr = right_->simplify();
        Expression *result_expr = new Add(left_expr, right_expr);
        return result_expr->eval();
    }
};

class Sub : public Expression
{
private:
    Expression *left_, *right_;

public:
    Sub(Expression *left, Expression *right) : left_(left), right_(right) {}

    void print(std::ostream &os) const override
    {
        os << '(';
        left_->print(os);
        os << '-';
        right_->print(os);
        os << ')';
    }

    Expression *derivative(const std::string &var) const override
    {
        return new Sub(left_->derivative(var), right_->derivative(var));
    }

    Expression *eval() const override
    {
        Expression *left_expr = left_->simplify();
        Expression *right_expr = right_->simplify();
        if (typeid(*left_expr) == typeid(Number) &&
            typeid(*right_expr) == typeid(Number))
        {
            return new Number(left_expr->eval("") - right_expr->eval(""));
        }
        return new Sub(left_expr, right_expr);
    }

    int eval(const std::map<std::string, int> &vars) const override
    {
        return left_->eval(vars) - right_->eval(vars);
    }

    ~Sub() override
    {
        delete left_;
        delete right_;
    }

    Expression *simplify() const override
    {
        Expression *left_expr = left_->simplify();
        Expression *right_expr = right_->simplify();

        if (typeid(*left_expr) == typeid(Variable) &&
            typeid(*right_expr) == typeid(Variable) &&
            static_cast<Variable *>(left_expr)->get_name() == static_cast<Variable *>(right_expr)->get_name())
        {
            return new Number(0);
        }

        Expression *result_expr = new Sub(left_expr, right_expr);
        return result_expr->eval();
    }
};

class Mul : public Expression
{
private:
    Expression *left_, *right_;

public:
    Mul(Expression *left, Expression *right) : left_(left), right_(right) {}

    void print(std::ostream &os) const override
    {
        os << '(';
        left_->print(os);
        os << '*';
        right_->print(os);
        os << ')';
    }
    Expression *derivative(const std::string &var) const override
    {
        return new Add(new Mul(left_->derivative(var), right_),
                       new Mul(left_, right_->derivative(var)));
    }

    Expression *eval() const override
    {
        Expression *left_expr = left_->simplify();
        Expression *right_expr = right_->simplify();
        if (typeid(*left_expr) == typeid(Number) &&
            typeid(*right_expr) == typeid(Number))
        {
            return new Number(left_expr->eval("") * right_expr->eval(""));
        }
        return new Mul(left_expr, right_expr);
    }

    int eval(const std::map<std::string, int> &vars) const override
    {
        return left_->eval(vars) * right_->eval(vars);
    }

    ~Mul() override
    {
        delete left_;
        delete right_;
    }

    Expression *simplify() const override
    {
        Expression *left_expr = left_->simplify();
        Expression *right_expr = right_->simplify();

        if ((typeid(*left_expr) == typeid(Number) && static_cast<Number *>(left_expr)->get_value() == 0) ||
            (typeid(*right_expr) == typeid(Number) && static_cast<Number *>(right_expr)->get_value() == 0))
        {
            delete left_expr;
            delete right_expr;
            return new Number(0);
        }
        if (typeid(*left_expr) == typeid(Number) && static_cast<Number *>(left_expr)->get_value() == 1)
        {
            delete left_expr;
            return right_expr->eval();
        }
        if (typeid(*right_expr) == typeid(Number) && static_cast<Number *>(right_expr)->get_value() == 1)
        {
            delete right_expr;
            return left_expr->eval();
        }

        Expression *result_expr = new Mul(left_expr, right_expr);
        return result_expr->eval();
    }
};

class Div : public Expression
{
private:
    Expression *left_, *right_;

public:
    Div(Expression *left, Expression *right) : left_(left), right_(right) {}

    void print(std::ostream &os) const override
    {
        os << '(';
        left_->print(os);
        os << '/';
        right_->print(os);
        os << ')';
    }

    Expression *derivative(const std::string &var) const override
    {
        return new Div(new Sub(new Mul(left_->derivative(var), right_),
                               new Mul(left_, right_->derivative(var))),
                       new Mul(right_, right_));
    }

    Expression *eval() const override
    {
        Expression *left_expr = left_->simplify();
        Expression *right_expr = right_->simplify();
        if (typeid(*left_expr) == typeid(Number) &&
            typeid(*right_expr) == typeid(Number) &&
            static_cast<Number *>(right_expr)->get_value() != 0)
        {
            return new Number(left_expr->eval("") / right_expr->eval(""));
        }
        return new Mul(left_expr, right_expr);
    }

    int eval(const std::map<std::string, int> &vars) const override
    {
        try
        {
            return left_->eval(vars) / right_->eval(vars);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    ~Div() override
    {
        delete left_;
        delete right_;
    }

    Expression *simplify() const override
    {
        Expression *left_expr = left_->simplify();
        Expression *right_expr = right_->simplify();

        if ((typeid(*left_expr) == typeid(Number) && static_cast<Number *>(left_expr)->get_value() == 0) &&
            (typeid(*right_expr) == typeid(Number) && static_cast<Number *>(right_expr)->get_value() != 0))
        {
            delete left_expr;
            delete right_expr;
            return new Number(0);
        }

        Expression *result_expr = new Div(left_expr, right_expr);
        return result_expr->eval();
    }
};

class Parser
{
private:
    std::string str_;

    bool is_number(const std::string &str)
    {
        for (char ch : str)
            if (!isdigit(ch))
                return false;
        return true;
    }

    bool is_variable(const std::string &str)
    {
        for (char ch : str)
            if (!isalpha(ch))
                return false;
        return true;
    }

    bool is_operation(const char operation)
    {
        switch (operation)
        {
        case '+':
        case '-':
        case '*':
        case '/':
            return true;
            break;

        default:
            return false;
            break;
        }
    }

    struct sub_expr
    {
        std::string l_expr;
        std::string r_expr;
        char sign;
    };

    sub_expr parse_to_sub_expr(const std::string &str)
    {
        sub_expr sub_expr_map;
        int bracket_count = 0;
        for (size_t i = 0; i < str.size(); i++)
        {
            if (bracket_count == 0 && is_operation(str[i]))
            {
                sub_expr_map.l_expr = str.substr(0, i);
                sub_expr_map.sign = str[i];
                sub_expr_map.r_expr = str.substr(i + 1, str.size() - i);
            }
            if (str[i] == '(')
                bracket_count++;
            if (str[i] == ')')
                bracket_count--;
        }
        return sub_expr_map;
    }

    Expression *parse_str(std::string str)
    {
        if (str[0] == '(')
        {
            str = str.substr(1, str.size() - 2);
        }

        if (is_number(str))
            return new Number(atoi(str.c_str()));
        if (is_variable(str))
            return new Variable(str);
        else
        {
            sub_expr sub_expr_map = parse_to_sub_expr(str);
            switch (sub_expr_map.sign)
            {
            case '+':
                return new Add(parse_str(sub_expr_map.l_expr), parse_str(sub_expr_map.r_expr));
                break;
            case '-':
                return new Sub(parse_str(sub_expr_map.l_expr), parse_str(sub_expr_map.r_expr));
                break;
            case '*':
                return new Mul(parse_str(sub_expr_map.l_expr), parse_str(sub_expr_map.r_expr));
                break;
            case '/':
                return new Div(parse_str(sub_expr_map.l_expr), parse_str(sub_expr_map.r_expr));
                break;
            }
        }
    }

public:
    Parser(std::string str) : str_(str) {}

    Expression *parse()
    {
        return parse_str(str_);
    }
};

class Smart_Parser
{
private:
    std::string str_;

    enum class Token_type
    {
        NUMBER = 'N',
        VARIABLE = 'V',
        ADD = '+',
        SUB = '-',
        MUL = '*',
        DIV = '/',
        L_BRACKET = '(',
        R_BRACKET = ')',
    };

    struct Token
    {
        Token_type type_;
        std::string value_;

        Token(Token_type type, std::string value) : type_(type), value_(value) {}
    };

    std::vector<Token> tokenize(std::string str)
    {
        std::vector<Token> tokens_array;
        std::string::const_iterator iter = str.begin();
        while (iter != str.end())
        {
            std::string lexem = "";
            if (*iter == '\0' || *iter == '\n')
                break;
            if (isdigit(*iter))
            {
                while (isdigit(*iter))
                {
                    lexem += *(iter++);
                }
                tokens_array.push_back(Token(Token_type::NUMBER, lexem));
                lexem.clear();
            }
            if (isalpha(*iter))
            {
                while (isalpha(*iter))
                {
                    lexem += *(iter++);
                }
                tokens_array.push_back(Token(Token_type::VARIABLE, lexem));
                lexem.clear();
            }
            lexem += *(iter++);
            switch (*(iter - 1))
            {
            case '+':
                tokens_array.push_back(Token(Token_type::ADD, lexem));
                lexem.clear();
                break;
            case '-':
                tokens_array.push_back(Token(Token_type::SUB, lexem));
                lexem.clear();
                break;
            case '*':
                tokens_array.push_back(Token(Token_type::MUL, lexem));
                lexem.clear();
                break;
            case '/':
                tokens_array.push_back(Token(Token_type::DIV, lexem));
                lexem.clear();
                break;
            case '(':
                tokens_array.push_back(Token(Token_type::L_BRACKET, lexem));
                lexem.clear();
                break;
            case ')':
                tokens_array.push_back(Token(Token_type::R_BRACKET, lexem));
                lexem.clear();
                break;
            }
        }
        return tokens_array;
    }

    int priority(Token_type operation)
    {
        switch (operation)
        {
        case Token_type::ADD:
        case Token_type::SUB:
            return 1;
            break;
        case Token_type::MUL:
        case Token_type::DIV:
            return 2;
            break;
        default:
            return 0;
            break;
        }
    }

    void apply_operator(std::stack<Expression *> &expression_stack, Token_type operation_type)
    {
        Expression *left_expression = expression_stack.top();
        expression_stack.pop();
        Expression *right_expression = expression_stack.top();
        expression_stack.pop();

        switch (operation_type)
        {
        case Token_type::ADD:
            expression_stack.push(new Add(right_expression, left_expression));
            break;
        case Token_type::SUB:
            expression_stack.push(new Sub(right_expression, left_expression));
            break;
        case Token_type::MUL:
            expression_stack.push(new Mul(right_expression, left_expression));
            break;
        case Token_type::DIV:
            expression_stack.push(new Div(right_expression, left_expression));
            break;
        }
    }

    Expression *expression_construct(std::vector<Token> tokens_array)
    {
        std::stack<Expression *> expression_stack;
        std::stack<Token_type> operation_stack;

        for (auto token : tokens_array)
        {
            if (token.type_ == Token_type::NUMBER)
            {
                expression_stack.push(new Number(atoi(token.value_.c_str())));
            }
            else if (token.type_ == Token_type::VARIABLE)
            {
                expression_stack.push(new Variable(token.value_));
            }
            else if (token.type_ == Token_type::L_BRACKET)
            {
                operation_stack.push(Token_type::L_BRACKET);
            }
            else if (token.type_ == Token_type::R_BRACKET)
            {
                while (!operation_stack.empty() && operation_stack.top() != Token_type::L_BRACKET)
                {
                    apply_operator(expression_stack, operation_stack.top());
                    operation_stack.pop();
                }
                operation_stack.pop();
            }
            else if (token.type_ == Token_type::ADD || token.type_ == Token_type::SUB ||
                     token.type_ == Token_type::MUL || token.type_ == Token_type::DIV)
            {
                while (!operation_stack.empty() && priority(operation_stack.top()) >= priority(token.type_))
                {
                    apply_operator(expression_stack, operation_stack.top());
                    operation_stack.pop();
                }
                operation_stack.push(token.type_);
            }
        }

        while (!operation_stack.empty())
        {
            apply_operator(expression_stack, operation_stack.top());
            operation_stack.pop();
        }

        return expression_stack.top();
    }

public:
    Smart_Parser(std::string str) : str_(str) {}

    Expression *smart_parse()
    {
        return expression_construct(tokenize(str_));
    }
};

Expression *parse(std::string str)
{
    Parser parser(str);
    return parser.parse();
}

Expression *smart_parse(std::string str)
{
    Smart_Parser smart_parser(str);
    return smart_parser.smart_parse();
}

int main()
{
#if 0
    Expression *e = new Add(
        new Number(3),
        new Mul(
            new Number(2),
            new Variable("x")));
    e->print(std::cout);
    
    std::cout << std::endl;
    Expression *de = e->derivative("x");
    de->print(std::cout);
    std::cout << std::endl;
    int res = e->eval("x <- 10; y <- 13");
    std::cout << res << std::endl;
#endif

#if 0
    std::ofstream out("input.txt");
    out << "(2-2)+(t-t)*x+1*y+1*z+0*a+b*0+(6*5+1)";
    out.close();

    std::ifstream in("input.txt");
    std::string str;
    in >> str;
    in.close();

    Expression *e = smart_parse(str);
    e->print(std::cout);
    std::cout << std::endl;
    Expression *se = e->simplify();

    out.open("output.txt");
    se->print(out);
    se->print(std::cout);
    std::cout << std::endl;
    out.close();
#endif

#if 1
    std::ifstream in("input.txt");
    std::string str;
    in >> str;
    in.close();

    Expression *exp = smart_parse(str);
    Expression *dexp = exp->derivative("x");

    std::ofstream out("output.txt");
    dexp->print(out);
    out.close();

#endif
    return 0;
}
