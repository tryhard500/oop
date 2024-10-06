#include <iostream>
#include <fstream>
#include <string>
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
    int eval(const std::string &meaning)
    {
        std::map<std::string, int> vars = parse_eval(meaning);
        return this->eval(vars);
    }
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

    int eval(const std::map<std::string, int> &vars) const override
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

    int eval(const std::map<std::string, int> &vars) const override
    {
        return vars.at(name_);
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

    int eval(const std::map<std::string, int> &vars) const override
    {
        return left_->eval(vars) + right_->eval(vars);
    }

    ~Add()
    {
        delete left_;
        delete right_;
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

    int eval(const std::map<std::string, int> &vars) const override
    {
        return left_->eval(vars) - right_->eval(vars);
    }

    ~Sub()
    {
        delete left_;
        delete right_;
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

    int eval(const std::map<std::string, int> &vars) const override
    {
        return left_->eval(vars) * right_->eval(vars);
    }

    ~Mul()
    {
        delete left_;
        delete right_;
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

    int eval(const std::map<std::string, int> &vars) const override
    {
        return left_->eval(vars) / right_->eval(vars);
    }

    ~Div()
    {
        delete left_;
        delete right_;
    }
};

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

Expression *parse(std::string &str)
{
    if (str[0] == '(')
    {
        str = str.substr(1, str.size() - 2);
    }

    // Expression *exp;
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
            return new Add(parse(sub_expr_map.l_expr), parse(sub_expr_map.r_expr));
            break;
        case '-':
            return new Sub(parse(sub_expr_map.l_expr), parse(sub_expr_map.r_expr));
        case '*':
            return new Mul(parse(sub_expr_map.l_expr), parse(sub_expr_map.r_expr));
        case '/':
            return new Div(parse(sub_expr_map.l_expr), parse(sub_expr_map.r_expr));
        }
    }
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

    std::ifstream in("input.txt");
    std::string str;
    in >> str;
    in.close();

    Expression *exp = parse(str);
    Expression *dexp = exp->derivative("x");

    std::ofstream out("output.txt");
    dexp->print(out);
    out.close();
    return 0;
}