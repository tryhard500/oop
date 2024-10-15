#include <iostream>
#include <string>
#include <map>
#include <cassert>

using namespace std;

map<string, int> parse_eval(const string &eval);

class Expression
{
public:
	virtual Expression *derivative(const string &diff_var) const = 0;
	virtual void print(ostream &os) const = 0;
	virtual ~Expression() {}
	virtual int eval(map<string, int> &ev) const = 0;
	virtual Expression *clone() const = 0;
	int eval(const string &eval) const
	{
		map<string, int> ev = parse_eval(eval);
		return this->eval(ev);
	}
	virtual Expression *simplify() const = 0;
	friend ostream &operator<<(ostream &os, const Expression *e)
	{
		e->print(os);
		return os;
	}
	virtual bool equal(const Expression *e) const = 0;
};

class Number : public Expression
{
	const int value;

public:
	Number(int value) : value(value) {}

	Expression *derivative(const string &diff_var) const override
	{
		return new Number(0);
	}

	void print(ostream &os) const override
	{
		os << value;
	}

	int eval(map<string, int> &ev) const override
	{
		return value;
	}

	Expression *clone() const override
	{
		return new Number(value);
	}

	Expression *simplify() const override
	{
		return clone();
	}

	bool equal(const Expression *e) const override
	{
		return (typeid(*e) == typeid(Number)) && ((Number *)e)->value == value;
	}
};

Expression *ONE = new Number(1);
Expression *ZERO = new Number(0);

class Variable : public Expression
{
	const string value;

public:
	Variable(const string &value) : value(value) {}

	Expression *derivative(const string &diff_var) const override
	{
		if (diff_var == value)
		{
			return new Number(1);
		}
		else
		{
			return new Number(0);
		}
	}

	void print(ostream &os) const override
	{
		cout << value;
	}

	int eval(map<string, int> &ev) const override
	{
		return ev.at(value);
	}

	Expression *clone() const override
	{
		return new Variable(value);
	}

	Expression *simplify() const override
	{
		return clone();
	}

	bool equal(const Expression *e) const
	{
		return (typeid(*e) == typeid(Variable)) && ((Variable *)e)->value == value;
	}
};

class Binary : public Expression
{
	const char op;

protected:
	const Expression *e1, *e2;

	Binary(Expression *e1, Expression *e2, char op) : e1(e1), e2(e2), op(op) {}

	static Expression *const_eval(Binary *e)
	{
		if (typeid(*(e->e1)) == typeid(Number) && typeid(*(e->e2)) == typeid(Number))
		{
			map<string, int> ev;
			Expression *b = new Number(e->eval(ev));
			delete e;
			return b;
		}
		return e;
	}

public:
	~Binary()
	{
		delete e1;
		delete e2;
	}

	void print(ostream &os) const override
	{
		os << "(" << e1 << op << e2 << ")";
	}
};

class Add : public Binary
{
public:
	Add(Expression *e1, Expression *e2) : Binary(e1, e2, '+') {}

	Expression *derivative(const string &diff_var) const override
	{
		return new Add(e1->derivative(diff_var), e2->derivative(diff_var));
	}

	int eval(map<string, int> &ev) const override
	{
		return e1->eval(ev) + e2->eval(ev);
	}

	Expression *clone() const override
	{
		return new Add(e1->clone(), e2->clone());
	}

	Expression *simplify() const override
	{
		Expression *te1, *te2;
		te1 = e1->simplify();
		te2 = e2->simplify();
		return const_eval(new Add(te1, te2));
	}

	bool equal(const Expression *e) const
	{
		return (typeid(*e) == typeid(Add)) && ((Add *)e)->e1->equal(e1) && ((Add *)e)->e2->equal(e2);
	}
};

class Sub : public Binary
{
public:
	Sub(Expression *e1, Expression *e2) : Binary(e1, e2, '-') {}

	Expression *derivative(const string &diff_var) const override
	{
		return new Sub(e1->derivative(diff_var), e2->derivative(diff_var));
	}

	int eval(map<string, int> &ev) const override
	{
		return e1->eval(ev) - e2->eval(ev);
	}

	Expression *clone() const override
	{
		return new Sub(e1->clone(), e2->clone());
	}

	Expression *simplify() const override
	{
		Expression *te1, *te2;
		te1 = e1->simplify();
		te2 = e2->simplify();
		if (te1->equal(te2))
		{
			delete te1;
			delete te2;
			return new Number(0);
		}
		return const_eval(new Sub(te1, te2));
	}

	bool equal(const Expression *e) const
	{
		return (typeid(*e) == typeid(Sub)) && ((Sub *)e)->e1->equal(e1) && ((Sub *)e)->e2->equal(e2);
	}
};

class Mul : public Binary
{
public:
	Mul(Expression *e1, Expression *e2) : Binary(e1, e2, '*') {}

	Expression *derivative(const string &diff_var) const override
	{
		return new Add(new Mul(e1->derivative(diff_var), e2->clone()),
					   new Mul(e1->clone(), e2->derivative(diff_var)));
	}

	int eval(map<string, int> &ev) const override
	{
		return e1->eval(ev) * e2->eval(ev);
	}

	Expression *clone() const override
	{
		return new Mul(e1->clone(), e2->clone());
	}

	Expression *simplify() const override
	{
		Expression *te1, *te2;
		te1 = e1->simplify();
		te2 = e2->simplify();
		if (te1->equal(ZERO) || te2->equal(ZERO))
		{
			delete te1;
			delete te2;
			return new Number(0);
		}
		if (te1->equal(ONE))
		{
			delete te1;
			return te2;
		}
		if (te2->equal(ONE))
		{
			delete te2;
			return te1;
		}
		return const_eval(new Mul(te1, te2));
	}

	bool equal(const Expression *e) const
	{
		return (typeid(*e) == typeid(Mul)) && ((Mul *)e)->e1->equal(e1) && ((Mul *)e)->e2->equal(e2);
	}
};

class Div : public Binary
{
public:
	Div(Expression *e1, Expression *e2) : Binary(e1, e2, '/') {}

	Expression *derivative(const string &diff_var) const override
	{
		return new Div(new Sub(new Mul(e1->clone(), e2->derivative(diff_var)),
							   new Mul(e1->derivative(diff_var), e2->clone())),
					   new Mul(e2->clone(), e2->clone()));
	}

	int eval(map<string, int> &ev) const override
	{
		int a = e1->eval(ev);
		int b = e2->eval(ev);
		if (b == 0)
		{
			throw "dividing by zero";
		}
		return a / b;
	}

	Expression *clone() const override
	{
		return new Div(e1->clone(), e2->clone());
	}

	Expression *simplify() const override
	{
		Expression *te1, *te2;
		te1 = e1->simplify();
		te2 = e2->simplify();
		if (te1->equal(ZERO) && !te2->equal(ZERO))
		{
			delete te1;
			delete te2;
			return new Number(0);
		}
		return const_eval(new Div(te1, te2));
	}

	bool equal(const Expression *e) const
	{
		return (typeid(*e) == typeid(Div)) && ((Div *)e)->e1->equal(e1) && ((Div *)e)->e2->equal(e2);
	}
};

enum lex_type
{
	INT,
	BRACE,
	OPERATOR,
	VAR,
	DELIMITER,
	EOL
};

struct Lexem
{
	lex_type type;
	string value;

	Lexem(lex_type type, const string &value)
	{
		this->type = type;
		this->value = value;
	}
};

Lexem next_lex(string::const_iterator &iter)
{
	string lex = "";
	while (*iter != '\0' && *iter != '\n' && *iter == ' ')
		iter++;
	if (*iter == '\0' || *iter == '\n')
	{
		return Lexem(EOL, lex);
	}
	if (isdigit(*iter))
	{
		while (isdigit(*iter))
		{
			lex += *(iter++);
		}
		return Lexem(INT, lex);
	}
	if (isalpha(*iter))
	{
		while (isalnum(*iter) || (*iter) == '_')
		{
			lex += *(iter++);
		}
		return Lexem(VAR, lex);
	}
	lex += *(iter++);
	switch (*(iter - 1))
	{
	case '+':
	case '-':
	case '*':
	case '/':
		return Lexem(OPERATOR, lex);
	case '(':
	case ')':
		return Lexem(BRACE, lex);
	case ';':
		return Lexem(DELIMITER, lex);
	case '<':
		lex += *(iter++);
		assert(lex == "<-");
		return Lexem(DELIMITER, lex);
	}
	return Lexem(EOL, lex);
}

class Parser
{
	Lexem lex = Lexem(EOL, " ");
	string::const_iterator it;

public:
	Parser(const std::string &str)
	{
		it = str.begin();
		lex = next_lex(it);
	}

	Expression *parse()
	{
		return parseExpression();
	}

private:
	void nextLex()
	{
		lex = next_lex(it);
	}

	// Expression ::= Term [+ Term] [- Term]
	// Term ::= Factor [* Factor] [/ Factor]
	// Factor ::= INT | VAR | (Expression)

	Expression *parseExpression()
	{
		if (lex.type == INT || lex.type == VAR || lex.value == "(")
		{
			Expression *tmp = parseTerm();
			while (lex.value == "+" || lex.value == "-")
			{
				std::string op = lex.value;
				nextLex();
				Expression *tmp2 = parseTerm();
				if (op == "+")
				{
					tmp = new Add(tmp, tmp2);
				}
				else
				{
					tmp = new Sub(tmp, tmp2);
				}
			}
			return tmp;
		}
		throw "syntax error";
	}

	Expression *parseTerm()
	{
		if (lex.type == INT || lex.type == VAR || lex.value == "(")
		{
			Expression *tmp = parseFactor();
			while (lex.value == "*" || lex.value == "/")
			{
				std::string op = lex.value;
				nextLex();
				Expression *tmp2 = parseFactor();
				if (op == "*")
				{
					tmp = new Mul(tmp, tmp2);
				}
				else
				{
					tmp = new Div(tmp, tmp2);
				}
			}
			return tmp;
		}
		throw "syntax error";
	}

	Expression *parseFactor()
	{
		Expression *tmp;
		if (lex.type == INT)
		{
			tmp = new Number(stoi(lex.value));
			nextLex();
			return tmp;
		}
		else if (lex.type == VAR)
		{
			tmp = new Variable(lex.value);
			nextLex();
			return tmp;
		}
		else if (lex.type == BRACE && lex.value == "(")
		{
			nextLex();
			tmp = parseExpression();
			if (lex.value != ")")
			{
				throw "syntax error";
			}
			nextLex();
			return tmp;
		}
		throw "syntax error";
	}
};

map<string, int> parse_eval(const string &eval)
{
	map<string, int> result;
	string::const_iterator iter = eval.begin();
	string key;
	int value;
	Lexem lex = next_lex(iter);
	while (lex.type != EOL)
	{
		assert(lex.type == VAR);
		key = lex.value;
		lex = next_lex(iter);
		assert(lex.type == DELIMITER && lex.value == "<-");
		lex = next_lex(iter);
		assert(lex.type == INT);
		value = stoi(lex.value);
		result[key] = value;
		lex = next_lex(iter);
		if (lex.type == EOL)
		{
			break;
		}
		assert(lex.type == DELIMITER && lex.value == ";");
		lex = next_lex(iter);
	}

	return result;
}

int main()
{
	Expression *e = new Add(
		new Number(3),
		new Mul(
			new Number(2),
			new Variable("x")));
	e->print(std::cout);
	Expression *de = e->derivative("x");
	int res = e->eval("x <- 10; y <- 13");
	std::cout << res << std::endl;
	return 0;
}

#if 0
#include <iostream>
#include <stack>
#include <sstream>
#include <cctype>

int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

void applyOperator(std::stack<int>& values, char op) {
    int right = values.top(); values.pop();
    int left = values.top(); values.pop();
    int result;
    switch (op) {
        case '+': result = left + right; break;
        case '-': result = left - right; break;
        case '*': result = left * right; break;
        case '/': result = left / right; break;
    }
    values.push(result);
}

int evaluate(const std::string& expression) {
    std::stack<char> operators;
    std::stack<int> values;
    std::istringstream iss(expression);
    char token;

    whil;e (iss >> token) {
        if (std::isdigit(token)) {
            iss.putback(token);
            int value;
            iss >> value;
            values.push(value);
        } else if (token == '(') {
            operators.push(token);
        } else if (token == ')') {
            while (!operators.empty() && operators.top() != '(') {
                applyOperator(values, operators.top());
                operators.pop();
            }
            operators.pop(); // Remove '('
			}
			else if (token == '+' || token == '-' || token == '*' || token == '/') {
            while (!operators.empty() && precedence(operators.top()) >= precedence(token)) {
                applyOperator(values, operators.top());
                operators.pop();
            }
            operators.push(token);
        }
    }

    while (!operators.empty()) {
        applyOperator(values, operators.top());
        operators.pop();
    }

    return values.top();
}

int main() {
    std::string expression = "3 + 5 * ( 2 - 8 )";
    int result = evaluate(expression);
    std::cout << "Результат: " << result << std::endl;
    return 0;
}

Найти еще

#endif