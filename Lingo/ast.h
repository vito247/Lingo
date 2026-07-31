#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

enum class ValueType {
    Num,
    Text,
    Bool,
    None
};

struct ASTNode {
    virtual ~ASTNode() = default;

    virtual void print(
        int indent = 0
    ) const = 0;
};

struct Expression : ASTNode {
};

struct Statement : ASTNode {
};

struct Program : ASTNode {
    std::vector<
        std::unique_ptr<Statement>
    > statements;

    void print(
        int indent = 0
    ) const override;
};

struct NumberLiteral : Expression {
    int value;

    explicit NumberLiteral(
        int value
    );

    void print(
        int indent = 0
    ) const override;
};

struct StringLiteral : Expression {
    std::string value;

    explicit StringLiteral(
        std::string value
    );

    void print(
        int indent = 0
    ) const override;
};

struct BoolLiteral : Expression {
    bool value;

    explicit BoolLiteral(
        bool value
    );

    void print(
        int indent = 0
    ) const override;
};

struct Identifier : Expression {
    std::string name;

    explicit Identifier(
        std::string name
    );

    void print(
        int indent = 0
    ) const override;
};

struct BinaryExpression : Expression {
    std::string op;

    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    BinaryExpression(
        std::string op,
        std::unique_ptr<Expression> left,
        std::unique_ptr<Expression> right
    );

    void print(
        int indent = 0
    ) const override;
};

struct CallExpression : Expression {
    std::string name;

    std::vector<
        std::unique_ptr<Expression>
    > arguments;

    explicit CallExpression(
        std::string name
    );

    void print(
        int indent = 0
    ) const override;
};

struct VariableDeclaration : Statement {
    bool isConst;
    ValueType type;

    std::string name;

    std::unique_ptr<Expression> value;

    VariableDeclaration(
        bool isConst,
        ValueType type,
        std::string name,
        std::unique_ptr<Expression> value
    );

    void print(
        int indent = 0
    ) const override;
};

struct AssignmentStatement : Statement {
    std::string name;

    std::unique_ptr<Expression> value;

    AssignmentStatement(
        std::string name,
        std::unique_ptr<Expression> value
    );

    void print(
        int indent = 0
    ) const override;
};

struct DisplayStatement : Statement {
    std::unique_ptr<Expression> expression;

    explicit DisplayStatement(
        std::unique_ptr<Expression> expression
    );

    void print(
        int indent = 0
    ) const override;
};

struct IfStatement : Statement {
    std::unique_ptr<Expression> condition;

    std::vector<
        std::unique_ptr<Statement>
    > body;

    std::vector<
        std::unique_ptr<Statement>
    > elseBody;

    void print(
        int indent = 0
    ) const override;
};

struct RepeatStatement : Statement {
    std::unique_ptr<Expression> count;

    std::vector<
        std::unique_ptr<Statement>
    > body;

    void print(
        int indent = 0
    ) const override;
};

struct ReturnStatement : Statement {
    std::unique_ptr<Expression> value;

    explicit ReturnStatement(
        std::unique_ptr<Expression> value
    );

    void print(
        int indent = 0
    ) const override;
};

struct FunctionParameter {
    ValueType type;
    std::string name;
};

struct FunctionDeclaration : Statement {
    std::string name;

    ValueType returnType;

    std::vector<
        FunctionParameter
    > parameters;

    std::vector<
        std::unique_ptr<Statement>
    > body;

    void print(
        int indent = 0
    ) const override;
};

struct ExpressionStatement : Statement {
    std::unique_ptr<Expression> expression;

    explicit ExpressionStatement(
        std::unique_ptr<Expression> expression
    );

    void print(
        int indent = 0
    ) const override;
};