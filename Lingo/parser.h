#pragma once

#include <memory>
#include <vector>

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

    bool check(TokensType type) const;
    bool match(TokensType type);

    const Token& expect(
        TokensType type,
        const std::string& message
    );

    void skipNewLines();

    std::unique_ptr<Statement> parseStatement();

    std::unique_ptr<Statement> parseVariableDeclaration();

    std::unique_ptr<Statement> parseAssignment();

    std::unique_ptr<Statement> parseDisplay();

    std::unique_ptr<Statement> parseImport();

    std::unique_ptr<Statement> parseIf();

    std::unique_ptr<Statement> parseRepeat();

    std::unique_ptr<Statement> parseFunction();

    std::unique_ptr<Statement> parseReturn();

    std::unique_ptr<Expression> parseExpression();

    std::unique_ptr<Expression> parseComparison();

    std::unique_ptr<Expression> parseTerm();

    std::unique_ptr<Expression> parseFactor();

    std::unique_ptr<Expression> parsePrimary();

    std::unique_ptr<Expression> parseIdentifierExpression();

    ValueType parseValueType();

    std::string parseNamespacedName();
};