#include "headers/genlib.h"
#include "strlib.h"
#include "tokenscanner.h"
#include <map>
#include <string>

using namespace std;

// function and class declaration
class Expression;
int precedence(string token);
Expression *parseExp(TokenScanner &scanner);
Expression *readE(TokenScanner &scanner, int prec);
Expression *readT(TokenScanner &scanner);

// EvaluationContext : store the symbol table
class EvaluationContext {
  public:
    EvaluationContext() {}

    ~EvaluationContext() {}

    bool isDefined(string key) {
        if (symbolTable[key] == 0) {
            Error("Cannot find key " + key);
            return false;
        }
        return true;
    }

    int getValue(string key) {
        if (isDefined(key))
            return symbolTable[key];
        cout << key
             << " does not exist in symbol table. (You might have not "
                "initialized it yet)\n";
    }

    void setValue(string key, int value) { symbolTable[key] = value; }

  private:
    map<string, int>
        symbolTable; // store identifer name , constant value  (As Map)
};

enum ExpressionType { CONSTANT, IDENTIFIER, COMPOUND };

// Abstract Class
class Expression {
  public:
    Expression();

    virtual ~Expression();

    virtual int eval(EvaluationContext &ctx) = 0;

    virtual string toString() = 0;

    virtual ExpressionType getType() = 0;

    virtual int getConstantValue();

    // make them virtual
    virtual string getIdentifierName();

    virtual char getOp();

    virtual Expression *getRhs();

    virtual Expression *getLhs();
};

Expression::Expression() {}

Expression::~Expression() {}

int Expression::getConstantValue() {
    Error(
        "Expression is an abstract class, it cannot produce an constant value");
}

string Expression::getIdentifierName() {
    Error("Expression is an abstract class, it cannot get an identifier name");
}

char Expression::getOp() {
    Error("Expression is an abstract class, it cannot get an operation ");
}

Expression *Expression::getRhs() { Error("Expression is an abstract class"); }

Expression *Expression::getLhs() { Error("Expression is an abstract class"); }

class ConstantExpression : public Expression {
  public:
    ConstantExpression(int value) { this->value = value; }

    ~ConstantExpression() {}

    int eval(EvaluationContext &ctx) { return value; }

    string toString() { return to_string(value); }

    ExpressionType getType() { return CONSTANT; }

    int getConstantValue() { return value; }

  private:
    int value;
};

class IdentifierExpression : public Expression {
  public:
    IdentifierExpression(string name) { this->name = name; }

    int eval(EvaluationContext &ctx) {
        if (ctx.isDefined(name) == false)
            Error(name + " is undefined.");
        return ctx.getValue(name);
    }

    string toString() { return name; }

    ExpressionType getType() { return IDENTIFIER; }

    string getIdentifierName() { return name; }

  private:
    string name;
};

class CompoundExpression : public Expression {
  public:
    CompoundExpression(char op, Expression *lhs, Expression *rhs) {
        this->op = op;
        this->rhs = rhs;
        this->lhs = lhs;
    }

    ~CompoundExpression() {
        delete rhs;
        delete lhs;
    }

    int eval(EvaluationContext &ctx) {
        int right = rhs->eval(ctx);

        if (op == '=') {
            ctx.setValue(lhs->getIdentifierName(), right);
            return right;
        }

        int left = lhs->eval(ctx);

        if (op == '+')
            return left + right;
        if (op == '-')
            return left - right;
        if (op == '/') {
            if (right == 0) {
                // Error("Cannot divide by 0") ;
                cout << " Cannot divide by 0" << endl;
                return -1111;
            }
            return left / right; // TODO: handle zero divisibility
        }
        if (op == '*')
            return left * right;

        Error("Illegal operation in expression ");
        return 0;
    }

    char getOp() { return op; }

    Expression *getRhs() { return rhs; }

    Expression *getLhs() { return lhs; }

    string toString() { return " " + op; }

    ExpressionType getType() { return COMPOUND; }

  private:
    char op;
    Expression *lhs, *rhs;
};

int precedence(string token) {
    if (token == "=")
        return 1;
    if (token == "+" || token == "-")
        return 2;
    if (token == "*" || token == "/")
        return 3;
    return 0;
}

// BNF Grammer
// <<EXPRESSION>> ::= <<TERM>>
// <<EXPRESSION>> ::= <<TERM>> <<OP>>  <<EXPRESSION>>
// <<TERM>> ::= integer
// <<TERM>> ::= identifier
// <<TERM>> ::= ( <<EXPRESSION>> )

Expression *parseExp(TokenScanner &scanner) {
    Expression *exp = readE(scanner, 0);
    if (scanner.hasMoreTokens()) {
        Error("Unexpected token " + scanner.nextToken());
    }
    return exp;
}

Expression *readE(TokenScanner &scanner, int prec) {
    Expression *exp = readT(scanner);
    string token;
    while (true) {
        token = scanner.nextToken();
        int tprec = precedence(token);
        if (tprec <= prec)
            break;
        Expression *rhs = readE(scanner, tprec);
        cout << "** DEBUG  " << token << " **\n " << endl;
        exp = new CompoundExpression(token[0], exp, rhs);
    }

    scanner.saveToken(token);
    return exp;
}

Expression *readT(TokenScanner &scanner) {
    string token = scanner.nextToken();
    TokenScanner::TokenType type = scanner.getTokenType(token);

    if (type == TokenScanner::WORD)
        return new IdentifierExpression(token);
    if (type == TokenScanner::NUMBER)
        return new ConstantExpression(stringToInteger(token));
    if (token != "(")
        Error("Illegal term in expression");

    Expression *exp = readE(scanner, 0);

    if (scanner.nextToken() != ")")
        Error("Unbalanced Parenthesis");

    return exp;
}

int main() {
    EvaluationContext ctx;
    TokenScanner scanner;
    Expression *exp;
    scanner.ignoreWhitespace();
    scanner.scanStrings();

    while (true) {
        exp = NULL;
        try {
            string line;
            cout << "=>";
            getline(cin, line);
            if (line == "quit" || line == "exit")
                break;
            scanner.setInput(line);
            exp = parseExp(scanner);
            int value = exp->eval(ctx);
            cout << value << endl;
        } catch (ErrorException ex) {
            cerr << " Error ";
        }
        if (exp != NULL)
            delete exp;
    }
    return 0;
}
