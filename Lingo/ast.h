#pragma once

#include <memory>
#include <string>
#include <vector>

enum class ValueType {
    Num,
    Text,
    Bool,
    None
};

class Expression {
public:
    virtual ~Expression() = default;
};

class Statement {
public:
    virtual ~Statement() = default;
};

class Program {
public:
    std::vector<std::unique_ptr<Statement>> statements;
};

/*
    Literals
*/

class NumberLiteral : public Expression {
public:
    std::string value;

    explicit NumberLiteral(
        const std::string& value
    )
        : value(value) {
    }
};

class StringLiteral : public Expression {
public:
    std::string value;

    explicit StringLiteral(
        const std::string& value
    )
        : value(value) {
    }
};

class BoolLiteral : public Expression {
public:
    bool value;

    explicit BoolLiteral(
        bool value
    )
        : value(value) {
    }
};

class Identifier : public Expression {
public:
    std::string name;

    explicit Identifier(
        const std::string& name
    )
        : name(name) {
    }
};

/*
    Binary expression
*/

class BinaryExpression : public Expression {
public:
    std::unique_ptr<Expression> left;
    std::string op;
    std::unique_ptr<Expression> right;

    BinaryExpression(
        std::unique_ptr<Expression> left,
        const std::string& op,
        std::unique_ptr<Expression> right
    )
        : left(std::move(left)),
        op(op),
        right(std::move(right)) {
    }
};

/*
    Function call
*/

class CallExpression : public Expression {
public:
    std::string name;

    std::vector<
        std::unique_ptr<Expression>
    > arguments;

    CallExpression(
        const std::string& name
    )
        : name(name) {
    }
};

/*
    Statements
*/

class ExpressionStatement : public Statement {
public:
    std::unique_ptr<Expression> expression;

    explicit ExpressionStatement(
        std::unique_ptr<Expression> expression
    )
        : expression(std::move(expression)) {
    }
};

class DisplayStatement : public Statement {
public:
    std::unique_ptr<Expression> expression;

    explicit DisplayStatement(
        std::unique_ptr<Expression> expression
    )
        : expression(std::move(expression)) {
    }
};

class VariableDeclaration : public Statement {
public:
    bool isConst;
    ValueType type;
    std::string name;
    std::unique_ptr<Expression> value;

    VariableDeclaration(
        bool isConst,
        ValueType type,
        const std::string& name,
        std::unique_ptr<Expression> value
    )
        : isConst(isConst),
        type(type),
        name(name),
        value(std::move(value)) {
    }
};

class AssignmentStatement : public Statement {
public:
    std::string name;
    std::unique_ptr<Expression> value;

    AssignmentStatement(
        const std::string& name,
        std::unique_ptr<Expression> value
    )
        : name(name),
        value(std::move(value)) {
    }
};

class ImportStatement : public Statement {
public:
    std::string path;
    std::string name;

    ImportStatement(
        const std::string& path,
        const std::string& name
    )
        : path(path),
        name(name) {
    }
};

class IfStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;

    std::vector<
        std::unique_ptr<Statement>
    > body;

    std::vector<
        std::unique_ptr<Statement>
    > elseBody;

    explicit IfStatement(
        std::unique_ptr<Expression> condition
    )
        : condition(std::move(condition)) {
    }
};

class RepeatStatement : public Statement {
public:
    std::unique_ptr<Expression> count;

    std::vector<
        std::unique_ptr<Statement>
    > body;

    explicit RepeatStatement(
        std::unique_ptr<Expression> count
    )
        : count(std::move(count)) {
    }
};

class Parameter {
public:
    ValueType type;
    std::string name;

    Parameter(
        ValueType type,
        const std::string& name
    )
        : type(type),
        name(name) {
    }
};

class FunctionDeclaration : public Statement {
public:
    std::string name;

    std::vector<Parameter> parameters;

    ValueType returnType;

    std::vector<
        std::unique_ptr<Statement>
    > body;

    explicit FunctionDeclaration(
        const std::string& name
    )
        : name(name),
        returnType(ValueType::None) {
    }
};

class ReturnStatement : public Statement {
public:
    std::unique_ptr<Expression> value;

    explicit ReturnStatement(
        std::unique_ptr<Expression> value
    )
        : value(std::move(value)) {
    }
};