#pragma once

#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "ast.h"

using Value = std::variant<
    int,
    std::string,
    bool
>;

struct Variable {
    Value value;
    bool isConst;
};

class Interpreter {
public:
    Interpreter();

    void execute(
        Program* program
    );

private:
    std::map<
        std::string,
        Variable
    > variables;

    std::map<
        std::string,
        FunctionDeclaration*
    > functions;

    void executeBlock(
        const std::vector<
        std::unique_ptr<Statement>
        >& statements
    );

    void executeStatement(
        Statement* statement
    );

    void executeVariableDeclaration(
        VariableDeclaration* statement
    );

    void executeAssignment(
        AssignmentStatement* statement
    );

    void executeDisplay(
        DisplayStatement* statement
    );

    void executeIf(
        IfStatement* statement
    );

    void executeRepeat(
        RepeatStatement* statement
    );

    void executeFunctionDeclaration(
        FunctionDeclaration* statement
    );

    void executeReturn(
        ReturnStatement* statement
    );

    Value executeFunctionCall(
        CallExpression* call
    );

    Value evaluateExpression(
        Expression* expression
    );

    Value evaluateBinary(
        const Value& left,
        const Value& right,
        const std::string& op
    );
};