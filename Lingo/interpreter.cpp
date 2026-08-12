#include "interpreter.h"
#include "lexer.h"
#include "parser.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <utility>
#include <variant>
#include <type_traits>
#include <string>
#include <iterator>

class ReturnException {
public:
    Value value;

    explicit ReturnException(Value value)
        : value(std::move(value)) {
    }
};

Interpreter::Interpreter() {
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
            auto* function =
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
        auto* display =
        dynamic_cast<DisplayStatement*>(
            statement
            )
        ) {
        executeDisplay(display);
        return;
    }

    if (
        auto* importStatement =
        dynamic_cast<ImportStatement*>(
            statement
            )
        ) {
        executeImport(importStatement);
        return;
    }

    if (
        auto* variable =
        dynamic_cast<VariableDeclaration*>(
            statement
            )
        ) {
        executeVariableDeclaration(variable);
        return;
    }

    if (
        auto* assignment =
        dynamic_cast<AssignmentStatement*>(
            statement
            )
        ) {
        executeAssignment(assignment);
        return;
    }

    if (
        auto* ifStatement =
        dynamic_cast<IfStatement*>(
            statement
            )
        ) {
        executeIf(ifStatement);
        return;
    }

    if (
        auto* repeat =
        dynamic_cast<RepeatStatement*>(
            statement
            )
        ) {

        executeRepeat(repeat);
        return;
    }

    if (
        auto* function =
        dynamic_cast<FunctionDeclaration*>(
            statement
            )
        ) {
        executeFunctionDeclaration(function);
        return;
    }

    if (
        auto* returnStatement =
        dynamic_cast<ReturnStatement*>(
            statement
            )
        ) {
        executeReturn(returnStatement);
        return;
    }

    if (
        auto* expression =
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

void Interpreter::executeImport(
    ImportStatement* statement
) {
    if (!statement) {
        throw std::runtime_error(
            "Import statement is null"
        );
    }

    loadLibrary(
        statement->name,
        statement->path
    );
}

void Interpreter::loadLibrary(
    const std::string& namespaceName,
    const std::string& libraryPath
) {
    std::filesystem::path path(
        libraryPath
    );

    if (!std::filesystem::exists(path)) {
        throw std::runtime_error(
            "Library not found: " +
            path.string()
        );
    }

    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Could not open library: " +
            path.string()
        );
    }

    std::string source(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    Lexer lexer(source);

    auto tokens =
        lexer.tokenize();

    Parser parser(tokens);

    auto ast =
        parser.parse();

    if (!ast) {
        throw std::runtime_error(
            "Failed to parse library: " +
            path.string()
        );
    }

    if (
        libraries.find(namespaceName) !=
        libraries.end()
        ) {
        throw std::runtime_error(
            "Library already imported: " +
            namespaceName
        );
    }

    auto library =
        std::make_unique<Library>();

    library->program =
        std::move(ast);

    Library* libraryPtr =
        library.get();

    libraries[
        namespaceName
    ] = std::move(library);

    for (
        const auto& statement :
        libraryPtr->program->statements
        ) {
        registerLibraryStatement(
            namespaceName,
            statement.get()
        );
    }
}

void Interpreter::registerLibraryStatement(
    const std::string& namespaceName,
    Statement* statement
) {
    Library& library =
        *libraries.at(namespaceName);

    if (
        auto* function =
        dynamic_cast<FunctionDeclaration*>(
            statement
            )
        ) {
        library.functions[
            function->name
        ] = function;

        return;
    }

    if (
        auto* variable =
        dynamic_cast<VariableDeclaration*>(
            statement
            )
        ) {
        library.variables[
            variable->name
        ] = variable;

        return;
    }

    throw std::runtime_error(
        "Invalid statement in .llib"
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
        std::move(value),
        statement->isConst
    };
}

void Interpreter::executeAssignment(
    AssignmentStatement* statement
) {
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

    it->second.value =
        evaluateExpression(
            statement->value.get()
        );
}

void Interpreter::executeIf(
    IfStatement* statement
) {
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
        std::get<bool>(condition)
        ) {
        executeBlock(statement->body);
    }
    else {
        executeBlock(statement->elseBody);
    }
}

void Interpreter::executeRepeat(
    RepeatStatement* statement
) {
    Value value =
        evaluateExpression(
            statement->count.get()
        );

    if (
        !std::holds_alternative<int>(value)
        ) {
        throw std::runtime_error(
            "Repeat count must be a number"
        );
    }

    int count =
        std::get<int>(value);

    if (count < 0) {
        throw std::runtime_error(
            "Repeat count cannot be negative"
        );
    }

    for (int i = 0; i < count; i++) {
        executeBlock(statement->body);
    }
}

void Interpreter::executeFunctionDeclaration(
    FunctionDeclaration* statement
) {
    functions[
        statement->name
    ] = statement;
}

void Interpreter::executeReturn(
    ReturnStatement* statement
) {
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
        auto* number =
        dynamic_cast<NumberLiteral*>(
            expression
            )
        ) {
        const std::string& text =
            number->value;

        if (
            text.find('.') !=
            std::string::npos
            ) {
            return std::stod(text);
        }

        return std::stoi(text);
    }

    if (
        auto* stringLiteral =
        dynamic_cast<StringLiteral*>(
            expression
            )
        ) {
        return stringLiteral->value;
    }

    if (
        auto* boolean =
        dynamic_cast<BoolLiteral*>(
            expression
            )
        ) {
        return boolean->value;
    }

    if (
        auto* identifier =
        dynamic_cast<Identifier*>(
            expression
            )
        ) {
        const std::string& name =
            identifier->name;

        // namespace:variable
        auto colon =
            name.find(':');

        if (
            colon !=
            std::string::npos
            ) {
            std::string namespaceName =
                name.substr(0, colon);

            std::string variableName =
                name.substr(colon + 1);

            auto libraryIt =
                libraries.find(
                    namespaceName
                );

            if (
                libraryIt ==
                libraries.end()
                ) {
                throw std::runtime_error(
                    "Unknown library: " +
                    namespaceName
                );
            }

            Library& library =
                *libraryIt->second;

            auto variableIt =
                library.variables.find(
                    variableName
                );

            if (
                variableIt ==
                library.variables.end()
                ) {
                throw std::runtime_error(
                    "Undefined library variable: " +
                    name
                );
            }

            return evaluateExpression(
                variableIt->second->value.get()
            );
        }

        auto it =
            variables.find(name);

        if (
            it == variables.end()
            ) {
            throw std::runtime_error(
                "Undefined variable: " +
                name
            );
        }

        return it->second.value;
    }

    if (
        auto* binary =
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
        auto* call =
        dynamic_cast<CallExpression*>(
            expression
            )
        ) {
        return executeFunctionCall(call);
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
            std::holds_alternative<int>(left) &&
            std::holds_alternative<int>(right)
            ) {
            return
                std::get<int>(left) +
                std::get<int>(right);
        }

        if (
            std::holds_alternative<double>(left) &&
            std::holds_alternative<double>(right)
            ) {
            return
                std::get<double>(left) +
                std::get<double>(right);
        }

        if (
            std::holds_alternative<int>(left) &&
            std::holds_alternative<double>(right)
            ) {
            return
                static_cast<double>(
                    std::get<int>(left)
                    ) +
                std::get<double>(right);
        }

        if (
            std::holds_alternative<double>(left) &&
            std::holds_alternative<int>(right)
            ) {
            return
                std::get<double>(left) +
                static_cast<double>(
                    std::get<int>(right)
                    );
        }

        if (
            std::holds_alternative<std::string>(left) &&
            std::holds_alternative<std::string>(right)
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
        bool leftNumber =
            std::holds_alternative<int>(left) ||
            std::holds_alternative<double>(left);

        bool rightNumber =
            std::holds_alternative<int>(right) ||
            std::holds_alternative<double>(right);

        if (
            !leftNumber ||
            !rightNumber
            ) {
            throw std::runtime_error(
                "Arithmetic operations require numbers"
            );
        }

        bool useDouble =
            std::holds_alternative<double>(left) ||
            std::holds_alternative<double>(right);

        double a =
            std::holds_alternative<double>(left)
            ? std::get<double>(left)
            : static_cast<double>(
                std::get<int>(left)
                );

        double b =
            std::holds_alternative<double>(right)
            ? std::get<double>(right)
            : static_cast<double>(
                std::get<int>(right)
                );

        if (
            op == "/" &&
            b == 0.0
            ) {
            throw std::runtime_error(
                "Division by zero"
            );
        }

        double result;

        if (op == "-")
            result = a - b;
        else if (op == "*")
            result = a * b;
        else
            result = a / b;

        if (!useDouble) {
            return static_cast<int>(result);
        }

        return result;
    }

    if (
        op == "more" ||
        op == "less" ||
        op == ">" ||
        op == "<" ||
        op == ">=" ||
        op == "<="
        ) {
        bool leftNumber =
            std::holds_alternative<int>(left) ||
            std::holds_alternative<double>(left);

        bool rightNumber =
            std::holds_alternative<int>(right) ||
            std::holds_alternative<double>(right);

        if (
            !leftNumber ||
            !rightNumber
            ) {
            throw std::runtime_error(
                "Comparison requires numbers"
            );
        }

        double a =
            std::holds_alternative<double>(left)
            ? std::get<double>(left)
            : static_cast<double>(
                std::get<int>(left)
                );

        double b =
            std::holds_alternative<double>(right)
            ? std::get<double>(right)
            : static_cast<double>(
                std::get<int>(right)
                );

        if (
            op == "more" ||
            op == ">"
            ) {
            return a > b;
        }

        if (op == "less" || op == "<") {
            return a < b;
        }

        if (op == ">=") {
            return a >= b;
        }

        return a <= b;
    }

    if (
        op == "eq" ||
        op == "==" ||
        op == "!="
        ) {
        bool equal = false;

        bool leftNumber =
            std::holds_alternative<int>(left) ||
            std::holds_alternative<double>(left);

        bool rightNumber =
            std::holds_alternative<int>(right) ||
            std::holds_alternative<double>(right);

        if (
            leftNumber &&
            rightNumber
            ) {
            double a =
                std::holds_alternative<double>(left)
                ? std::get<double>(left)
                : static_cast<double>(
                    std::get<int>(left)
                    );

            double b =
                std::holds_alternative<double>(right)
                ? std::get<double>(right)
                : static_cast<double>(
                    std::get<int>(right)
                    );

            equal = a == b;
        }
        else if (
            left.index() ==
            right.index()
            ) {
            if (
                std::holds_alternative<std::string>(left)
                ) {
                equal =
                    std::get<std::string>(left) ==
                    std::get<std::string>(right);
            }
            else if (
                std::holds_alternative<bool>(left)
                ) {
                equal =
                    std::get<bool>(left) ==
                    std::get<bool>(right);
            }
        }

        if (op == "!=") {
            return !equal;
        }

        return equal;
    }

    throw std::runtime_error(
        "Unknown operator: " + op
    );
}

Value Interpreter::executeFunctionCall(
    CallExpression* call
) {
    const std::string& name =
        call->name;

    FunctionDeclaration* function =
        nullptr;

    // namespace:function
    auto colon =
        name.find(':');

    if (
        colon !=
        std::string::npos
        ) {
        std::string namespaceName =
            name.substr(0, colon);

        std::string functionName =
            name.substr(colon + 1);

        auto libraryIt =
            libraries.find(
                namespaceName
            );

        if (
            libraryIt ==
            libraries.end()
            ) {
            throw std::runtime_error(
                "Unknown library: " +
                namespaceName
            );
        }

        Library& library =
            *libraryIt->second;

        auto functionIt =
            library.functions.find(
                functionName
            );

        if (
            functionIt ==
            library.functions.end()
            ) {
            throw std::runtime_error(
                "Undefined function: " +
                name
            );
        }

        function =
            functionIt->second;
    }
    else {

        auto it =
            functions.find(name);

        if (
            it == functions.end()
            ) {
            throw std::runtime_error(
                "Undefined function: " +
                name
            );
        }

        function =
            it->second;
    }

    if (
        call->arguments.size() !=
        function->parameters.size()
        ) {
        throw std::runtime_error(
            "Argument count mismatch in function: " +
            name
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
        std::move(variables);

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
            std::move(oldVariables);

        return returnValue.value;
    }
    catch (...) {
        variables =
            std::move(oldVariables);

        throw;
    }

    variables =
        std::move(oldVariables);

    return 0;
}