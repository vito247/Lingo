#pragma once

#include <vector>
#include <memory>

#include "token.h"
#include "ast.h"

class Parser {
public:
    explicit Parser(
        const std::vector<Token>& tokens
    );

    std::unique_ptr<Program> parse();

private:
    const std::vector<Token>& tokens;

    size_t position = 0;

    const Token& current() const;

    const Token& previous() const;

    bool isAtEnd() const;

    const Token& advance();

    bool check(
        TokensType type
    ) const;

    bool match(
        TokensType type
    );

    const Token& expect(
        TokensType type,
        const std::string& message
    );

    void skipNewLines();

    ValueType parseType();

    std::unique_ptr<Expression>
        parseExpression();

    std::unique_ptr<Expression>
        parseComparison();

    std::unique_ptr<Expression>
        parseTerm();

    std::unique_ptr<Expression>
        parseFactor();

    std::unique_ptr<Expression>
        parsePrimary();

    std::unique_ptr<Statement>
        parseStatement();

    std::unique_ptr<Statement>
        parseVariableDeclaration();

    std::unique_ptr<Statement>
        parseAssignment();

    std::unique_ptr<Statement>
        parseDisplay();

    std::unique_ptr<Statement>
        parseIf();

    std::unique_ptr<Statement>
        parseRepeat();

    std::unique_ptr<Statement>
        parseFunction();

    std::unique_ptr<Statement>
        parseReturn();

    std::vector<
        std::unique_ptr<Statement>
    > parseDo(
        int parentIndent
    );

    std::vector<
        FunctionParameter
    > parseInput(
        int parentIndent
    );

    int currentIndent() const;
};