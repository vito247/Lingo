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
    return tokens[position];
}

const Token& Parser::previous() const {
    return tokens[position - 1];
}

bool Parser::isAtEnd() const {
    return current().type == TokensType::EndOfFile;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        position++;
    }

    return previous();
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

    advance();

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
            std::to_string(
                current().line
            )
        );
    }

    return advance();
}

void Parser::skipNewLines() {
    while (
        match(
            TokensType::NewLine
        )
        ) {
    }
}

int Parser::currentIndent() const {
    return current().indent;
}

ValueType Parser::parseType() {
    if (
        match(
            TokensType::Num
        )
        ) {
        return ValueType::Num;
    }

    if (
        match(
            TokensType::Text
        )
        ) {
        return ValueType::Text;
    }

    if (
        match(
            TokensType::Bool
        )
        ) {
        return ValueType::Bool;
    }

    if (
        match(
            TokensType::None
        )
        ) {
        return ValueType::None;
    }

    throw std::runtime_error(
        "Expected type at line " +
        std::to_string(
            current().line
        )
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
            advance().value;

        auto right =
            parseTerm();

        expression =
            std::make_unique<
            BinaryExpression
            >(
                op,
                std::move(expression),
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
            advance().value;

        auto right =
            parseFactor();

        expression =
            std::make_unique<
            BinaryExpression
            >(
                op,
                std::move(expression),
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
            advance().value;

        auto right =
            parsePrimary();

        expression =
            std::make_unique<
            BinaryExpression
            >(
                op,
                std::move(expression),
                std::move(right)
            );
    }

    return expression;
}

std::unique_ptr<Expression>
Parser::parsePrimary() {
    if (
        match(
            TokensType::Number
        )
        ) {
        return std::make_unique<
            NumberLiteral
        >(
            std::stoi(
                previous().value
            )
        );
    }

    if (
        match(
            TokensType::String
        )
        ) {
        return std::make_unique<
            StringLiteral
        >(
            previous().value
        );
    }

    if (
        match(
            TokensType::True
        )
        ) {
        return std::make_unique<
            BoolLiteral
        >(
            true
        );
    }

    if (
        match(
            TokensType::False
        )
        ) {
        return std::make_unique<
            BoolLiteral
        >(
            false
        );
    }

    if (
        match(
            TokensType::Identifier
        )
        ) {
        std::string name =
            previous().value;

        if (
            match(
                TokensType::LeftParen
            )
            ) {
            auto call =
                std::make_unique<
                CallExpression
                >(
                    name
                );

            if (
                !check(
                    TokensType::RightParen
                )
                ) {
                do {
                    call->arguments.push_back(
                        parseExpression()
                    );
                } while (
                    match(
                        TokensType::Comma
                    )
                    );
            }

            expect(
                TokensType::RightParen,
                "Expected ')' after arguments"
            );

            return call;
        }

        return std::make_unique<
            Identifier
        >(
            name
        );
    }

    if (
        match(
            TokensType::LeftParen
        )
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
        std::to_string(
            current().line
        )
    );
}

std::unique_ptr<Statement>
Parser::parseVariableDeclaration() {
    bool isConst =
        match(
            TokensType::Const
        );

    if (!isConst) {
        expect(
            TokensType::Var,
            "Expected Var"
        );
    }

    ValueType type =
        parseType();

    std::string name =
        expect(
            TokensType::Identifier,
            "Expected variable name"
        ).value;

    expect(
        TokensType::Assign,
        "Expected '='"
    );

    auto value =
        parseExpression();

    return std::make_unique<
        VariableDeclaration
    >(
        isConst,
        type,
        name,
        std::move(value)
    );
}

std::unique_ptr<Statement>
Parser::parseAssignment() {
    std::string name =
        expect(
            TokensType::Identifier,
            "Expected identifier"
        ).value;

    expect(
        TokensType::Assign,
        "Expected '='"
    );

    auto value =
        parseExpression();

    return std::make_unique<
        AssignmentStatement
    >(
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

    return std::make_unique<
        DisplayStatement
    >(
        parseExpression()
    );
}

std::vector<
    std::unique_ptr<Statement>
>
Parser::parseDo(
    int parentIndent
) {
    expect(
        TokensType::Do,
        "Expected Do"
    );

    if (
        check(
            TokensType::NewLine
        )
        ) {
        advance();
    }

    std::vector<
        std::unique_ptr<Statement>
    > body;

    while (
        !isAtEnd()
        ) {
        if (
            check(
                TokensType::NewLine
            )
            ) {
            advance();

            continue;
        }

        if (
            currentIndent() <=
            parentIndent
            ) {
            break;
        }

        body.push_back(
            parseStatement()
        );

        skipNewLines();
    }

    return body;
}

std::unique_ptr<Statement>
Parser::parseIf() {
    expect(
        TokensType::If,
        "Expected If"
    );

    auto condition =
        parseExpression();

    expect(
        TokensType::Comma,
        "Expected ',' after condition"
    );

    if (
        check(
            TokensType::NewLine
        )
        ) {
        advance();
    }

    int indent =
        currentIndent();

    auto statement =
        std::make_unique<
        IfStatement
        >();

    statement->condition =
        std::move(condition);

    expect(
        TokensType::Do,
        "Expected Do after If"
    );

    if (
        check(
            TokensType::NewLine
        )
        ) {
        advance();
    }

    while (
        !isAtEnd()
        ) {
        if (
            check(
                TokensType::NewLine
            )
            ) {
            advance();

            continue;
        }

        if (
            currentIndent() <=
            indent
            ) {
            break;
        }

        statement->body.push_back(
            parseStatement()
        );

        skipNewLines();
    }

    if (
        check(
            TokensType::Else
        )
        ) {
        advance();

        expect(
            TokensType::Comma,
            "Expected ',' after Else"
        );

        if (
            check(
                TokensType::NewLine
            )
            ) {
            advance();
        }

        expect(
            TokensType::Do,
            "Expected Do after Else"
        );

        if (
            check(
                TokensType::NewLine
            )
            ) {
            advance();
        }

        while (
            !isAtEnd()
            ) {
            if (
                check(
                    TokensType::NewLine
                )
                ) {
                advance();

                continue;
            }

            if (
                currentIndent() <=
                indent
                ) {
                break;
            }

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

    expect(
        TokensType::Comma,
        "Expected ',' after Repeat"
    );

    if (
        check(
            TokensType::NewLine
        )
        ) {
        advance();
    }

    int indent =
        currentIndent();

    auto statement =
        std::make_unique<
        RepeatStatement
        >();

    statement->count =
        std::move(count);

    expect(
        TokensType::Do,
        "Expected Do after Repeat"
    );

    if (
        check(
            TokensType::NewLine
        )
        ) {
        advance();
    }

    while (
        !isAtEnd()
        ) {
        if (
            check(
                TokensType::NewLine
            )
            ) {
            advance();

            continue;
        }

        if (
            currentIndent() <=
            indent
            ) {
            break;
        }

        statement->body.push_back(
            parseStatement()
        );

        skipNewLines();
    }

    return statement;
}

std::vector<
    FunctionParameter
>
Parser::parseInput(
    int parentIndent
) {
    expect(
        TokensType::Input,
        "Expected Input"
    );

    if (
        check(
            TokensType::NewLine
        )
        ) {
        advance();
    }

    std::vector<
        FunctionParameter
    > parameters;

    while (
        !isAtEnd()
        ) {
        if (
            check(
                TokensType::NewLine
            )
            ) {
            advance();

            continue;
        }

        if (
            currentIndent() <=
            parentIndent
            ) {
            break;
        }

        expect(
            TokensType::Var,
            "Expected Var in Input"
        );

        ValueType type =
            parseType();

        std::string name =
            expect(
                TokensType::Identifier,
                "Expected parameter name"
            ).value;

        parameters.push_back({
            type,
            name
            });

        skipNewLines();
    }

    return parameters;
}

std::unique_ptr<Statement>
Parser::parseFunction() {
    expect(
        TokensType::Function,
        "Expected Function"
    );

    auto function =
        std::make_unique<
        FunctionDeclaration
        >();

    function->name =
        expect(
            TokensType::Identifier,
            "Expected function name"
        ).value;

    expect(
        TokensType::Arrow,
        "Expected '->'"
    );

    function->returnType =
        parseType();

    if (
        check(
            TokensType::NewLine
        )
        ) {
        advance();
    }

    int functionIndent =
        currentIndent();

    if (
        check(
            TokensType::Input
        )
        ) {
        function->parameters =
            parseInput(
                functionIndent
            );
    }

    skipNewLines();

    if (
        check(
            TokensType::Do
        )
        ) {
        function->body =
            parseDo(
                functionIndent
            );
    }

    return function;
}

std::unique_ptr<Statement>
Parser::parseReturn() {
    expect(
        TokensType::Throw,
        "Expected Throw"
    );

    return std::make_unique<
        ReturnStatement
    >(
        parseExpression()
    );
}

std::unique_ptr<Statement>
Parser::parseStatement() {
    skipNewLines();

    if (
        check(
            TokensType::Var
        ) ||
        check(
            TokensType::Const
        )
        ) {
        return parseVariableDeclaration();
    }

    if (
        check(
            TokensType::Display
        )
        ) {
        return parseDisplay();
    }

    if (
        check(
            TokensType::Set
        )
        ) {
        advance();

        return parseAssignment();
    }

    if (
        check(
            TokensType::If
        )
        ) {
        return parseIf();
    }

    if (
        check(
            TokensType::Repeat
        )
        ) {
        return parseRepeat();
    }

    if (
        check(
            TokensType::Function
        )
        ) {
        return parseFunction();
    }

    if (
        check(
            TokensType::Throw
        )
        ) {
        return parseReturn();
    }

    throw std::runtime_error(
        "Unknown statement at line " +
        std::to_string(
            current().line
        )
    );
}

std::unique_ptr<Program>
Parser::parse() {
    auto program =
        std::make_unique<
        Program
        >();

    skipNewLines();

    while (
        !isAtEnd()
        ) {
        program->statements.push_back(
            parseStatement()
        );

        skipNewLines();
    }

    return program;
}