#include "interpreter.h"

#include <iostream>
#include <stdexcept>
#include <variant>
#include <utility>

class ReturnException {
public:
    Value value;

    explicit ReturnException(Value value)
        : value(std::move(value)) {
    }
};

Interpreter::Interpreter()
{
}

void Interpreter::execute(
    Program* program
) {
    if (!program) {
        throw std::runtime_error(
            "Program is null"
        );
    }

    for (
        const auto& statement :
        program->statements
        ) {
        if (
            auto function =
            dynamic_cast<FunctionDeclaration*>(
                statement.get()
                )
            ) {
            functions[
                function->name
            ] = function;
        }
    }

    for (
        const auto& statement :
        program->statements
        ) {
        if (
            dynamic_cast<FunctionDeclaration*>(
                statement.get()
                )
            ) {
            continue;
        }

        executeStatement(
            statement.get()
        );
    }
}

void Interpreter::executeStatement(
    Statement* statement
) {
    if (!statement) {
        throw std::runtime_error(
            "Statement is null"
        );
    }

    if (
        auto display =
        dynamic_cast<DisplayStatement*>(
            statement
            )
        ) {
        executeDisplay(
            display
        );

        return;
    }

    if (
        auto variable =
        dynamic_cast<VariableDeclaration*>(
            statement
            )
        ) {
        executeVariableDeclaration(
            variable
        );

        return;
    }

    if (
        auto assignment =
        dynamic_cast<AssignmentStatement*>(
            statement
            )
        ) {
        executeAssignment(
            assignment
        );

        return;
    }

    if (
        auto ifStatement =
        dynamic_cast<IfStatement*>(
            statement
            )
        ) {
        executeIf(
            ifStatement
        );

        return;
    }

    if (
        auto repeat =
        dynamic_cast<RepeatStatement*>(
            statement
            )
        ) {
        executeRepeat(
            repeat
        );

        return;
    }

    if (
        auto function =
        dynamic_cast<FunctionDeclaration*>(
            statement
            )
        ) {
        executeFunctionDeclaration(
            function
        );

        return;
    }

    if (
        auto returnStatement =
        dynamic_cast<ReturnStatement*>(
            statement
            )
        ) {
        executeReturn(
            returnStatement
        );

        return;
    }

    if (
        auto expression =
        dynamic_cast<ExpressionStatement*>(
            statement
            )
        ) {
        evaluateExpression(
            expression->expression.get()
        );

        return;
    }

    throw std::runtime_error(
        "Unknown statement"
    );
}

void Interpreter::executeBlock(
    const std::vector<
    std::unique_ptr<Statement>
    >& statements
) {
    for (
        const auto& statement :
        statements
        ) {
        executeStatement(
            statement.get()
        );
    }
}

void Interpreter::executeDisplay(
    DisplayStatement* statement
) {
    if (!statement) {
        throw std::runtime_error(
            "Display statement is null"
        );
    }

    Value value =
        evaluateExpression(
            statement->expression.get()
        );

    std::visit(
        [](
            const auto& value
            ) {
                std::cout
                    << value
                    << '\n';
        },
        value
    );
}

void Interpreter::executeVariableDeclaration(
    VariableDeclaration* statement
) {
    if (!statement) {
        throw std::runtime_error(
            "Variable declaration is null"
        );
    }

    if (
        variables.find(
            statement->name
        ) != variables.end()
        ) {
        throw std::runtime_error(
            "Variable already exists: " +
            statement->name
        );
    }

    Value value =
        evaluateExpression(
            statement->value.get()
        );

    variables[
        statement->name
    ] = Variable{
        value,
        statement->isConst
    };
}

void Interpreter::executeAssignment(
    AssignmentStatement* statement
) {
    if (!statement) {
        throw std::runtime_error(
            "Assignment statement is null"
        );
    }

    auto it =
        variables.find(
            statement->name
        );

    if (
        it == variables.end()
        ) {
        throw std::runtime_error(
            "Undefined variable: " +
            statement->name
        );
    }

    if (it->second.isConst) {
        throw std::runtime_error(
            "Cannot reassign const variable: " +
            statement->name
        );
    }

    Value value =
        evaluateExpression(
            statement->value.get()
        );

    it->second.value =
        std::move(value);
}

void Interpreter::executeIf(
    IfStatement* statement
) {
    if (!statement) {
        throw std::runtime_error(
            "If statement is null"
        );
    }

    Value condition =
        evaluateExpression(
            statement->condition.get()
        );

    if (
        !std::holds_alternative<bool>(
            condition
        )
        ) {
        throw std::runtime_error(
            "If condition must be boolean"
        );
    }

    if (
        std::get<bool>(
            condition
        )
        ) {
        executeBlock(
            statement->body
        );
    }
    else if (
        !statement->elseBody.empty()
        ) {
        executeBlock(
            statement->elseBody
        );
    }
}

void Interpreter::executeRepeat(
    RepeatStatement* statement
) {
    if (!statement) {
        throw std::runtime_error(
            "Repeat statement is null"
        );
    }

    Value countValue =
        evaluateExpression(
            statement->count.get()
        );

    if (
        !std::holds_alternative<int>(
            countValue
        )
        ) {
        throw std::runtime_error(
            "Repeat count must be a number"
        );
    }

    int count =
        std::get<int>(
            countValue
        );

    if (count < 0) {
        throw std::runtime_error(
            "Repeat count cannot be negative"
        );
    }

    for (
        int i = 0;
        i < count;
        i++
        ) {
        executeBlock(
            statement->body
        );
    }
}

void Interpreter::executeFunctionDeclaration(
    FunctionDeclaration* statement
) {
    if (!statement) {
        throw std::runtime_error(
            "Function declaration is null"
        );
    }

    functions[
        statement->name
    ] = statement;
}

void Interpreter::executeReturn(
    ReturnStatement* statement
) {
    if (!statement) {
        throw std::runtime_error(
            "Return statement is null"
        );
    }

    if (!statement->value) {
        throw ReturnException(
            0
        );
    }

    Value value =
        evaluateExpression(
            statement->value.get()
        );

    throw ReturnException(
        std::move(value)
    );
}

Value Interpreter::evaluateExpression(
    Expression* expression
) {
    if (!expression) {
        throw std::runtime_error(
            "Expression is null"
        );
    }

    if (
        auto number =
        dynamic_cast<NumberLiteral*>(
            expression
            )
        ) {
        return number->value;
    }

    if (
        auto stringLiteral =
        dynamic_cast<StringLiteral*>(
            expression
            )
        ) {
        return stringLiteral->value;
    }

    if (
        auto boolean =
        dynamic_cast<BoolLiteral*>(
            expression
            )
        ) {
        return boolean->value;
    }

    if (
        auto identifier =
        dynamic_cast<Identifier*>(
            expression
            )
        ) {
        auto it =
            variables.find(
                identifier->name
            );

        if (
            it == variables.end()
            ) {
            throw std::runtime_error(
                "Undefined variable: " +
                identifier->name
            );
        }

        return it->second.value;
    }

    if (
        auto binary =
        dynamic_cast<BinaryExpression*>(
            expression
            )
        ) {
        Value left =
            evaluateExpression(
                binary->left.get()
            );

        Value right =
            evaluateExpression(
                binary->right.get()
            );

        return evaluateBinary(
            left,
            right,
            binary->op
        );
    }

    if (
        auto call =
        dynamic_cast<CallExpression*>(
            expression
            )
        ) {
        return executeFunctionCall(
            call
        );
    }

    throw std::runtime_error(
        "Unknown expression"
    );
}

Value Interpreter::evaluateBinary(
    const Value& left,
    const Value& right,
    const std::string& op
) {
    if (op == "+") {
        if (
            std::holds_alternative<int>(
                left
            ) &&
            std::holds_alternative<int>(
                right
            )
            ) {
            return
                std::get<int>(left) +
                std::get<int>(right);
        }

        if (
            std::holds_alternative<std::string>(
                left
            ) &&
            std::holds_alternative<std::string>(
                right
            )
            ) {
            return
                std::get<std::string>(left) +
                std::get<std::string>(right);
        }

        throw std::runtime_error(
            "Cannot add these types"
        );
    }

    if (
        op == "-" ||
        op == "*" ||
        op == "/"
        ) {
        if (
            !std::holds_alternative<int>(
                left
            ) ||
            !std::holds_alternative<int>(
                right
            )
            ) {
            throw std::runtime_error(
                "Arithmetic operations require numbers"
            );
        }

        int a =
            std::get<int>(
                left
            );

        int b =
            std::get<int>(
                right
            );

        if (op == "-") {
            return a - b;
        }

        if (op == "*") {
            return a * b;
        }

        if (op == "/") {
            if (b == 0) {
                throw std::runtime_error(
                    "Division by zero"
                );
            }

            return a / b;
        }
    }

    if (
        op == "more" ||
        op == "less"
        ) {
        if (
            !std::holds_alternative<int>(
                left
            ) ||
            !std::holds_alternative<int>(
                right
            )
            ) {
            throw std::runtime_error(
                "Comparison requires numbers"
            );
        }

        int a =
            std::get<int>(
                left
            );

        int b =
            std::get<int>(
                right
            );

        if (op == "more") {
            return a > b;
        }

        return a < b;
    }

    if (
        op == "eq" ||
        op == "not"
        ) {
        if (
            left.index() !=
            right.index()
            ) {
            return op == "not";
        }

        bool equal = false;

        if (
            std::holds_alternative<int>(
                left
            )
            ) {
            equal =
                std::get<int>(left) ==
                std::get<int>(right);
        }
        else if (
            std::holds_alternative<std::string>(
                left
            )
            ) {
            equal =
                std::get<std::string>(left) ==
                std::get<std::string>(right);
        }
        else if (
            std::holds_alternative<bool>(
                left
            )
            ) {
            equal =
                std::get<bool>(left) ==
                std::get<bool>(right);
        }

        if (op == "eq") {
            return equal;
        }

        return !equal;
    }

    throw std::runtime_error(
        "Unknown operator: " +
        op
    );
}

Value Interpreter::executeFunctionCall(
    CallExpression* call
) {
    if (!call) {
        throw std::runtime_error(
            "Call expression is null"
        );
    }

    auto it =
        functions.find(
            call->name
        );

    if (
        it == functions.end()
        ) {
        throw std::runtime_error(
            "Undefined function: " +
            call->name
        );
    }

    FunctionDeclaration* function =
        it->second;

    if (
        call->arguments.size() !=
        function->parameters.size()
        ) {
        throw std::runtime_error(
            "Argument count mismatch in function: " +
            call->name
        );
    }

    std::vector<Value> arguments;

    for (
        const auto& argument :
        call->arguments
        ) {
        arguments.push_back(
            evaluateExpression(
                argument.get()
            )
        );
    }

    auto oldVariables =
        std::move(
            variables
        );

    variables.clear();

    try {
        for (
            size_t i = 0;
            i < function->parameters.size();
            i++
            ) {
            variables[
                function->parameters[i].name
            ] = Variable{
                arguments[i],
                false
            };
        }

        executeBlock(
            function->body
        );
    }
    catch (
        const ReturnException& returnValue
        ) {
        variables =
            std::move(
                oldVariables
            );

        return returnValue.value;
    }

    variables =
        std::move(
            oldVariables
        );

    return 0;
}