#include "ast.h"

#include <iostream>
#include <utility>

NumberLiteral::NumberLiteral(int value)
    : value(value) {
}
AssignmentStatement::AssignmentStatement(
    std::string name,
    std::unique_ptr<Expression> value
)
    : name(std::move(name)),
    value(std::move(value)) {
}
StringLiteral::StringLiteral(
    std::string value
)
    : value(std::move(value)) {
}

BoolLiteral::BoolLiteral(bool value)
    : value(value) {
}

Identifier::Identifier(
    std::string name
)
    : name(std::move(name)) {
}

BinaryExpression::BinaryExpression(
    std::string op,
    std::unique_ptr<Expression> left,
    std::unique_ptr<Expression> right
)
    : op(std::move(op)),
    left(std::move(left)),
    right(std::move(right)) {
}

CallExpression::CallExpression(
    std::string name
)
    : name(std::move(name)) {
}

VariableDeclaration::VariableDeclaration(
    bool isConst,
    ValueType type,
    std::string name,
    std::unique_ptr<Expression> value
)
    : isConst(isConst),
    type(type),
    name(std::move(name)),
    value(std::move(value)) {
}

DisplayStatement::DisplayStatement(
    std::unique_ptr<Expression> expression
)
    : expression(std::move(expression)) {
}

ReturnStatement::ReturnStatement(
    std::unique_ptr<Expression> value
)
    : value(std::move(value)) {
}

ExpressionStatement::ExpressionStatement(
    std::unique_ptr<Expression> expression
)
    : expression(std::move(expression)) {
}
namespace {

    void printIndent(int indent) {
        for (int i = 0; i < indent; ++i) {
            std::cout << "  ";
        }
    }

    const char* valueTypeToString(ValueType type) {
        switch (type) {
        case ValueType::Num:
            return "num";

        case ValueType::Text:
            return "text";

        case ValueType::Bool:
            return "bool";

        case ValueType::None:
            return "none";
        }

        return "unknown";
    }

}

void Program::print(int indent) const {
    printIndent(indent);
    std::cout << "Program\n";

    for (const auto& statement : statements) {
        if (statement) {
            statement->print(indent + 1);
        }
    }
}

void DisplayStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "DisplayStatement\n";

    if (expression) {
        expression->print(indent + 1);
    }
}

void VariableDeclaration::print(int indent) const {
    printIndent(indent);

    std::cout
        << "VariableDeclaration: "
        << (isConst ? "const " : "var ")
        << valueTypeToString(type)
        << " "
        << name
        << "\n";

    if (value) {
        value->print(indent + 1);
    }
}

void AssignmentStatement::print(int indent) const {
    printIndent(indent);

    std::cout
        << "AssignmentStatement: "
        << name
        << "\n";

    if (value) {
        value->print(indent + 1);
    }
}

void IfStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "IfStatement\n";

    printIndent(indent + 1);
    std::cout << "Condition\n";

    if (condition) {
        condition->print(indent + 2);
    }

    printIndent(indent + 1);
    std::cout << "What\n";

    for (const auto& statement : body) {
        if (statement) {
            statement->print(indent + 2);
        }
    }

    if (!elseBody.empty()) {
        printIndent(indent + 1);
        std::cout << "Else\n";

        for (const auto& statement : elseBody) {
            if (statement) {
                statement->print(indent + 2);
            }
        }
    }
}

void RepeatStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "RepeatStatement\n";

    printIndent(indent + 1);
    std::cout << "Count\n";

    if (count) {
        count->print(indent + 2);
    }

    printIndent(indent + 1);
    std::cout << "What\n";

    for (const auto& statement : body) {
        if (statement) {
            statement->print(indent + 2);
        }
    }
}

void FunctionDeclaration::print(int indent) const {
    printIndent(indent);

    std::cout
        << "FunctionDeclaration: "
        << name
        << " -> "
        << valueTypeToString(returnType)
        << "\n";

    if (!parameters.empty()) {
        printIndent(indent + 1);
        std::cout << "Input\n";

        for (const auto& parameter : parameters) {
            printIndent(indent + 2);

            std::cout
                << valueTypeToString(parameter.type)
                << " "
                << parameter.name
                << "\n";
        }
    }

    printIndent(indent + 1);
    std::cout << "What\n";

    for (const auto& statement : body) {
        if (statement) {
            statement->print(indent + 2);
        }
    }
}

void ReturnStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "Throw\n";

    if (value) {
        value->print(indent + 1);
    }
}

void ExpressionStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "ExpressionStatement\n";

    if (expression) {
        expression->print(indent + 1);
    }
}

void NumberLiteral::print(int indent) const {
    printIndent(indent);

    std::cout
        << "NumberLiteral: "
        << value
        << "\n";
}

void StringLiteral::print(int indent) const {
    printIndent(indent);

    std::cout
        << "StringLiteral: \""
        << value
        << "\"\n";
}

void BoolLiteral::print(int indent) const {
    printIndent(indent);

    std::cout
        << "BoolLiteral: "
        << (value ? "true" : "false")
        << "\n";
}

void Identifier::print(int indent) const {
    printIndent(indent);

    std::cout
        << "Identifier: "
        << name
        << "\n";
}

void BinaryExpression::print(int indent) const {
    printIndent(indent);

    std::cout
        << "BinaryExpression: "
        << op
        << "\n";

    printIndent(indent + 1);
    std::cout << "Left\n";

    if (left) {
        left->print(indent + 2);
    }

    printIndent(indent + 1);
    std::cout << "Right\n";

    if (right) {
        right->print(indent + 2);
    }
}

void CallExpression::print(int indent) const {
    printIndent(indent);

    std::cout
        << "CallExpression: "
        << name
        << "\n";

    for (const auto& argument : arguments) {
        if (argument) {
            argument->print(indent + 1);
        }
    }
}