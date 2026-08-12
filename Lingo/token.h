#pragma once

#include <string>
#include <utility>

enum class TokensType {
    Number,
    String,

    True,
    False,

    Identifier,

    Var,
    Const,
    Num,
    Text,
    Bool,
    None,

    Display,
    Set,

    If,
    Else,
    Repeat,
    Do,

    Function,
    Input,
    Throw,

    Import,
    As,
    Colon,

    Assign,
    Arrow,

    Plus,
    Minus,
    Multiply,
    Divide,

    More,
    Less,
    Equal,
    NotEqual,
    Greater,
    LessEqual,
    GreaterEqual,

    LeftParen,
    RightParen,
	LeftBracket,
	RightBracket,
    Comma,

    NewLine,
    EndOfFile,

    Unknown
};

struct Token {
    TokensType type;
    std::string value;
    int line;
    int indent;

    Token(
        TokensType type,
        std::string value,
        int line = 1,
        int indent = 0
    )
        : type(type),
        value(std::move(value)),
        line(line),
        indent(indent) {
    }
};