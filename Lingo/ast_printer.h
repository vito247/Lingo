#pragma once

#include <iostream>
#include <string>
#include "ast.h"

class ASTPrinter {
private:
    std::ostream& out;

    std::string nextPrefix(
        const std::string& prefix,
        bool isLast
    ) {
        return prefix + (isLast ? "    " : "|   ");
    }

    void printLine(
        const std::string& prefix,
        bool isLast,
        const std::string& label
    ) {
        out << prefix
            << (isLast ? "`-- " : "|-- ")
            << label << '\n';
    }

    std::string valueTypeName(ValueType type) {
        switch (type) {
        case ValueType::Num: return "num";
        case ValueType::Text: return "text";
        case ValueType::Bool: return "bool";
        case ValueType::None: return "none";
        default: return "unknown";
        }
    }

    void printExpression(
        const Expression* expr,
        const std::string& prefix,
        bool isLast
    ) {
        if (!expr) {
            printLine(prefix, isLast, "null");
            return;
        }

        std::string childPrefix = nextPrefix(prefix, isLast);

        if (auto node = dynamic_cast<const NumberLiteral*>(expr)) {
            printLine(prefix, isLast,
                "NumberLiteral: " + node->value);
        }
        else if (auto node = dynamic_cast<const StringLiteral*>(expr)) {
            printLine(prefix, isLast,
                "StringLiteral: \"" + node->value + "\"");
        }
        else if (auto node = dynamic_cast<const BoolLiteral*>(expr)) {
            printLine(prefix, isLast,
                std::string("BoolLiteral: ") +
                (node->value ? "true" : "false"));
        }
        else if (auto node = dynamic_cast<const Identifier*>(expr)) {
            printLine(prefix, isLast,
                "Identifier: " + node->name);
        }
        else if (auto node = dynamic_cast<const BinaryExpression*>(expr)) {
            printLine(prefix, isLast,
                "BinaryExpression: " + node->op);

            printExpression(
                node->left.get(),
                childPrefix,
                false
            );

            printExpression(
                node->right.get(),
                childPrefix,
                true
            );
        }
        else if (auto node = dynamic_cast<const CallExpression*>(expr)) {
            printLine(prefix, isLast,
                "CallExpression: " + node->name);

            for (size_t i = 0; i < node->arguments.size(); i++) {
                printExpression(
                    node->arguments[i].get(),
                    childPrefix,
                    i == node->arguments.size() - 1
                );
            }
        }
        else {
            printLine(prefix, isLast, "UnknownExpression");
        }
    }

    void printStatement(
        const Statement* stmt,
        const std::string& prefix,
        bool isLast
    ) {
        if (!stmt) {
            printLine(prefix, isLast, "null");
            return;
        }

        std::string childPrefix = nextPrefix(prefix, isLast);

        if (auto node = dynamic_cast<const ExpressionStatement*>(stmt)) {
            printLine(prefix, isLast, "ExpressionStatement");

            printExpression(
                node->expression.get(),
                childPrefix,
                true
            );
        }
        else if (auto node = dynamic_cast<const DisplayStatement*>(stmt)) {
            printLine(prefix, isLast, "DisplayStatement");

            printExpression(
                node->expression.get(),
                childPrefix,
                true
            );
        }
        else if (auto node = dynamic_cast<const VariableDeclaration*>(stmt)) {
            std::string label =
                std::string(node->isConst ? "ConstDeclaration: " : "VariableDeclaration: ")
                + valueTypeName(node->type)
                + " " + node->name;

            printLine(prefix, isLast, label);

            printExpression(
                node->value.get(),
                childPrefix,
                true
            );
        }
        else if (auto node = dynamic_cast<const AssignmentStatement*>(stmt)) {
            printLine(prefix, isLast,
                "AssignmentStatement: " + node->name);

            printExpression(
                node->value.get(),
                childPrefix,
                true
            );
        }
        else if (auto node = dynamic_cast<const ImportStatement*>(stmt)) {
            printLine(prefix, isLast,
                "ImportStatement: " + node->name
                + " <- " + node->path);
        }
        else if (auto node = dynamic_cast<const IfStatement*>(stmt)) {
            printLine(prefix, isLast, "IfStatement");

            size_t total = 1 + node->body.size()
                + node->elseBody.size();
            size_t index = 0;

            auto conditionLast = (++index == total);

            printLine(
                childPrefix,
                conditionLast,
                "Condition"
            );

            printExpression(
                node->condition.get(),
                nextPrefix(childPrefix, conditionLast),
                true
            );

            for (const auto& child : node->body) {
                printStatement(
                    child.get(),
                    childPrefix,
                    ++index == total
                );
            }

            if (!node->elseBody.empty()) {
                printLine(
                    childPrefix,
                    false,
                    "ElseBody"
                );

                std::string elsePrefix =
                    nextPrefix(childPrefix, false);

                for (size_t i = 0; i < node->elseBody.size(); i++) {
                    printStatement(
                        node->elseBody[i].get(),
                        elsePrefix,
                        i == node->elseBody.size() - 1
                    );
                }
            }
        }
        else if (auto node = dynamic_cast<const RepeatStatement*>(stmt)) {
            printLine(prefix, isLast, "RepeatStatement");

            printLine(childPrefix, false, "Count");

            printExpression(
                node->count.get(),
                nextPrefix(childPrefix, false),
                true
            );

            printLine(childPrefix, node->body.empty(), "Body");

            std::string bodyPrefix =
                nextPrefix(childPrefix, node->body.empty());

            for (size_t i = 0; i < node->body.size(); i++) {
                printStatement(
                    node->body[i].get(),
                    bodyPrefix,
                    i == node->body.size() - 1
                );
            }
        }
        else if (auto node = dynamic_cast<const FunctionDeclaration*>(stmt)) {
            printLine(prefix, isLast,
                "FunctionDeclaration: " + node->name);

            size_t total = node->parameters.size() + 1;
            size_t index = 0;

            for (const auto& param : node->parameters) {
                printLine(
                    childPrefix,
                    ++index == total,
                    "Parameter: " + valueTypeName(param.type)
                    + " " + param.name
                );
            }

            printLine(
                childPrefix,
                true,
                "ReturnType: " + valueTypeName(node->returnType)
            );

            printLine(childPrefix, node->body.empty(), "Body");

            std::string bodyPrefix =
                nextPrefix(childPrefix, node->body.empty());

            for (size_t i = 0; i < node->body.size(); i++) {
                printStatement(
                    node->body[i].get(),
                    bodyPrefix,
                    i == node->body.size() - 1
                );
            }
        }
        else if (auto node = dynamic_cast<const ReturnStatement*>(stmt)) {
            printLine(prefix, isLast, "ReturnStatement");

            printExpression(
                node->value.get(),
                childPrefix,
                true
            );
        }
        else {
            printLine(prefix, isLast, "UnknownStatement");
        }
    }

public:
    explicit ASTPrinter(std::ostream& output = std::cout)
        : out(output) {
    }

    void print(const Program* program) {
        if (!program) {
            out << "Program: null\n";
            return;
        }

        out << "Program\n";

        for (size_t i = 0; i < program->statements.size(); i++) {
            printStatement(
                program->statements[i].get(),
                "",
                i == program->statements.size() - 1
            );
        }
    }
};
