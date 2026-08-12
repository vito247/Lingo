#pragma once

#include <string>
#include <vector>
#include <cstddef>

#include "token.h"

class Lexer {
public:
    explicit Lexer(const std::string& src);

    std::vector<Token> tokenize();

private:
    std::string source;

    size_t position = 0;
    int line = 1;
    bool atLineStart = true;

    char peek() const;
    char peekNext() const;

    void advance();
    void skipSpaces();
    void skipComment();

    int readIndent();

    std::string readNumber();
    std::string readIdentifier();
    std::string readString();

    TokensType getKeywordType(
        const std::string& word
    );

    Token makeToken(
        TokensType type,
        const std::string& value,
        int indent
    );

    Token nextToken();
};

std::string TokensTypeToString(
    TokensType type
);