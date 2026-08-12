#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>

#include "ast.h"

using Value = std::variant<
    int,
    double,
    std::string,
    bool
>;

struct Variable {
    Value value;
    bool isConst;
};

struct Library {
    std::unique_ptr<Program> program;

    std::unordered_map<
        std::string,
        FunctionDeclaration*
    > functions;

    std::unordered_map<
        std::string,
        VariableDeclaration*
    > variables;
};

class Interpreter {
public:
    Interpreter();

    void execute(
        Program* program
    );

private:
    std::unordered_map<
        std::string,
        Variable
    > variables;

    std::unordered_map<
        std::string,
        FunctionDeclaration*
    > functions;

    std::unordered_map<
        std::string,
        std::unique_ptr<Library>
    > libraries;

    void executeStatement(
        Statement* statement
    );

    void executeBlock(
        const std::vector<
        std::unique_ptr<Statement>
        >& statements
    );

    void executeDisplay(
        DisplayStatement* statement
    );

    void executeVariableDeclaration(
        VariableDeclaration* statement
    );

    void executeAssignment(
        AssignmentStatement* statement
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

    void executeImport(
        ImportStatement* statement
    );

    void loadLibrary(
        const std::string& namespaceName,
        const std::string& libraryPath
    );

    void registerLibraryStatement(
        const std::string& namespaceName,
        Statement* statement
    );

    Value evaluateExpression(
        Expression* expression
    );

    Value evaluateBinary(
        const Value& left,
        const Value& right,
        const std::string& op
    );

    Value executeFunctionCall(
        CallExpression* call
    );
};