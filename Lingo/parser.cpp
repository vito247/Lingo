#include "parser.h"

#include <stdexcept>
#include <string>
#include <utility>

Parser::Parser(
    const std::vector<Token>& tokens
)
    : tokens(tokens) {
}

const Token& Parser::current() const {
    if (
        position >=
        tokens.size()
    ) {
        return tokens.back();
    }

    return tokens[position];
}

const Token& Parser::previous() const {
    if (position == 0) {
        return tokens[0];
    }

    return tokens[position - 1];
}

bool Parser::check(
    TokensType type
) const {
    return current().type == type;
}

bool Parser::match(
    TokensType type
) {
    if (!check(type)) {
        return false;
    }

    position++;
    return true;
}

const Token& Parser::expect(
    TokensType type,
    const std::string& message
) {
    if (!check(type)) {
        throw std::runtime_error(
            message +
            " at line " +
            std::to_string(current().line)
        );
    }

    return tokens[position++];
}

void Parser::skipNewLines() {
    while (
        match(TokensType::NewLine)
    ) {
    }
}

std::unique_ptr<Program> Parser::parse() {
    auto program =
        std::make_unique<Program>();

    skipNewLines();

    while (
        !check(TokensType::EndOfFile)
    ) {
        program->statements.push_back(
            parseStatement()
        );

        skipNewLines();
    }

    return program;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    skipNewLines();

    switch (current().type) {
    case TokensType::Var:
    case TokensType::Const:
        return parseVariableDeclaration();

    case TokensType::Set:
        return parseAssignment();

    case TokensType::Display:
        return parseDisplay();

    case TokensType::Import:
        return parseImport();

    case TokensType::If:
        return parseIf();

    case TokensType::Repeat:
        return parseRepeat();

    case TokensType::Function:
        return parseFunction();

    case TokensType::Do:
        match(TokensType::Do);
        skipNewLines();
        return parseStatement();

    case TokensType::Throw:
        return parseReturn();

    case TokensType::Identifier:
        return std::make_unique<ExpressionStatement>(
            parseExpression()
        );

    default:
        throw std::runtime_error(
            "Unknown statement at line " +
            std::to_string(current().line)
        );
    }
}

ValueType Parser::parseValueType() {
    if (match(TokensType::Num)) {
        return ValueType::Num;
    }

    if (match(TokensType::Text)) {
        return ValueType::Text;
    }

    if (match(TokensType::Bool)) {
        return ValueType::Bool;
    }

    if (match(TokensType::None)) {
        return ValueType::None;
    }

    throw std::runtime_error(
        "Expected type at line " +
        std::to_string(current().line)
    );
}

std::unique_ptr<Statement>
Parser::parseVariableDeclaration() {
    bool isConst =
        match(TokensType::Const);

    if (!isConst) {
        expect(
            TokensType::Var,
            "Expected Var or Const"
        );
    }

    ValueType type =
        parseValueType();

    const Token& name =
        expect(
            TokensType::Identifier,
            "Expected variable name"
        );

    expect(
        TokensType::Assign,
        "Expected '='"
    );

    auto value =
        parseExpression();

    return std::make_unique<VariableDeclaration>(
        isConst,
        type,
        name.value,
        std::move(value)
    );
}

std::unique_ptr<Statement>
Parser::parseAssignment() {
    expect(
        TokensType::Set,
        "Expected Set"
    );

    std::string name =
        parseNamespacedName();

    expect(
        TokensType::Assign,
        "Expected '='"
    );

    auto value =
        parseExpression();

    return std::make_unique<AssignmentStatement>(
        name,
        std::move(value)
    );
}

std::unique_ptr<Statement>
Parser::parseDisplay() {
    expect(
        TokensType::Display,
        "Expected Display"
    );

    auto expression =
        parseExpression();

    return std::make_unique<DisplayStatement>(
        std::move(expression)
    );
}

std::unique_ptr<Statement>
Parser::parseImport() {
    expect(
        TokensType::Import,
        "Expected Import"
    );

    const Token& path =
        expect(
            TokensType::String,
            "Expected library path"
        );

    expect(
        TokensType::As,
        "Expected As"
    );

    const Token& name =
        expect(
            TokensType::Identifier,
            "Expected namespace name"
        );

    return std::make_unique<ImportStatement>(
        path.value,
        name.value
    );
}

std::string Parser::parseNamespacedName() {
    const Token& first =
        expect(
            TokensType::Identifier,
            "Expected identifier"
        );

    std::string result =
        first.value;

    while (
        match(TokensType::Colon)
    ) {
        const Token& second =
            expect(
                TokensType::Identifier,
                "Expected identifier after ':'"
            );

        result += ":";
        result += second.value;
    }

    return result;
}

std::unique_ptr<Statement>
Parser::parseIf() {
    expect(
        TokensType::If,
        "Expected If"
    );

    auto condition =
        parseExpression();

    auto statement =
        std::make_unique<IfStatement>(
            std::move(condition)
        );

    skipNewLines();

    while (
        !check(TokensType::Else) &&
        !check(TokensType::EndOfFile) &&
        current().indent > 0
    ) {
        statement->body.push_back(
            parseStatement()
        );

        skipNewLines();
    }

    if (match(TokensType::Else)) {
        skipNewLines();

        while (
            !check(TokensType::EndOfFile) &&
            current().indent > 0
        ) {
            statement->elseBody.push_back(
                parseStatement()
            );

            skipNewLines();
        }
    }

    return statement;
}

std::unique_ptr<Statement>
Parser::parseRepeat() {
    expect(
        TokensType::Repeat,
        "Expected Repeat"
    );

    auto count =
        parseExpression();

    auto statement =
        std::make_unique<RepeatStatement>(
            std::move(count)
        );

    skipNewLines();

    while (
        !check(TokensType::EndOfFile) &&
        current().indent > 0
    ) {
        statement->body.push_back(
            parseStatement()
        );

        skipNewLines();
    }

    return statement;
}

std::unique_ptr<Statement>
Parser::parseFunction() {
    expect(
        TokensType::Function,
        "Expected Function"
    );

    const Token& name =
        expect(
            TokensType::Identifier,
            "Expected function name"
        );

    auto function =
        std::make_unique<FunctionDeclaration>(
            name.value
        );

    // Function add -> num
    if (match(TokensType::Arrow)) {
        function->returnType =
            parseValueType();
    }

    skipNewLines();

    // Input
    if (match(TokensType::Input)) {
        skipNewLines();

        while (
            !check(TokensType::EndOfFile) &&
            current().indent > 0 &&
            !check(TokensType::Do)
            ) {
            expect(
                TokensType::Var,
                "Expected Var in Input"
            );

            ValueType type =
                parseValueType();

            const Token& parameterName =
                expect(
                    TokensType::Identifier,
                    "Expected parameter name"
                );

            function->parameters.emplace_back(
                type,
                parameterName.value
            );

            skipNewLines();
        }
    }

    // Do
    if (match(TokensType::Do)) {
        skipNewLines();
    }

    while (
        !check(TokensType::EndOfFile) &&
        current().indent > 0
        ) {
        function->body.push_back(
            parseStatement()
        );

        skipNewLines();
    }

    return function;
}

std::unique_ptr<Statement>
Parser::parseReturn() {
    expect(
        TokensType::Throw,
        "Expected Throw"
    );

    auto value =
        parseExpression();

    return std::make_unique<ReturnStatement>(
        std::move(value)
    );
}

std::unique_ptr<Expression>
Parser::parseExpression() {
    return parseComparison();
}

std::unique_ptr<Expression>
Parser::parseComparison() {
    auto expression =
        parseTerm();

    while (
        check(TokensType::More) ||
        check(TokensType::Less) ||
        check(TokensType::Equal) ||
        check(TokensType::NotEqual) ||
        check(TokensType::Greater) ||
        check(TokensType::LessEqual) ||
        check(TokensType::GreaterEqual)
    ) {
        std::string op =
            current().value;

        position++;

        auto right =
            parseTerm();

        expression =
            std::make_unique<BinaryExpression>(
                std::move(expression),
                op,
                std::move(right)
            );
    }

    return expression;
}

std::unique_ptr<Expression>
Parser::parseTerm() {
    auto expression =
        parseFactor();

    while (
        check(TokensType::Plus) ||
        check(TokensType::Minus)
    ) {
        std::string op =
            current().value;

        position++;

        auto right =
            parseFactor();

        expression =
            std::make_unique<BinaryExpression>(
                std::move(expression),
                op,
                std::move(right)
            );
    }

    return expression;
}

std::unique_ptr<Expression>
Parser::parseFactor() {
    auto expression =
        parsePrimary();

    while (
        check(TokensType::Multiply) ||
        check(TokensType::Divide)
    ) {
        std::string op =
            current().value;

        position++;

        auto right =
            parsePrimary();

        expression =
            std::make_unique<BinaryExpression>(
                std::move(expression),
                op,
                std::move(right)
            );
    }

    return expression;
}

std::unique_ptr<Expression>
Parser::parsePrimary() {
    if (
        match(TokensType::Number)
    ) {
        return std::make_unique<NumberLiteral>(
            previous().value
        );
    }

    if (
        match(TokensType::String)
    ) {
        return std::make_unique<StringLiteral>(
            previous().value
        );
    }

    if (
        match(TokensType::True)
    ) {
        return std::make_unique<BoolLiteral>(
            true
        );
    }

    if (
        match(TokensType::False)
    ) {
        return std::make_unique<BoolLiteral>(
            false
        );
    }

    if (
        check(TokensType::Identifier)
    ) {
        return parseIdentifierExpression();
    }

    if (
        match(TokensType::LeftParen)
    ) {
        auto expression =
            parseExpression();

        expect(
            TokensType::RightParen,
            "Expected ')'"
        );

        return expression;
    }

    throw std::runtime_error(
        "Expected expression at line " +
        std::to_string(current().line)
    );
}

std::unique_ptr<Expression>
Parser::parseIdentifierExpression() {
    std::string name =
        parseNamespacedName();

    if (
        match(TokensType::LeftParen)
    ) {
        auto call =
            std::make_unique<CallExpression>(
                name
            );

        if (!check(TokensType::RightParen)) {
            do {
                call->arguments.push_back(
                    parseExpression()
                );
            } while (
                match(TokensType::Comma)
            );
        }

        expect(
            TokensType::RightParen,
            "Expected ')'"
        );

        return call;
    }

    return std::make_unique<Identifier>(
        name
    );
}