#include "lexer.h"

#include <cctype>
#include <type_traits>

Lexer::Lexer(const std::string& src)
    : source(src) {
}

char Lexer::peek() const {
    if (
        position >=
        source.size()
        ) {
        return '\0';
    }

    return source[position];
}

char Lexer::peekNext() const {
    if (
        position + 1 >=
        source.size()
        ) {
        return '\0';
    }

    return source[
        position + 1
    ];
}

void Lexer::advance() {
    if (
        position >=
        source.size()
        ) {
        return;
    }

    if (
        source[position] == '\n'
        ) {
        line++;

        atLineStart = true;
    }

    position++;
}

void Lexer::skipSpaces() {
    while (
        peek() == ' ' ||
        peek() == '\t'
        ) {
        advance();
    }
}

void Lexer::skipComment() {
    if (
        peek() != '#' ||
        peekNext() != '#'
        ) {
        return;
    }

    advance();
    advance();

    while (
        peek() != '\n' &&
        peek() != '\0'
        ) {
        advance();
    }
}

int Lexer::readIndent() {
    int indent = 0;

    while (
        peek() == ' ' ||
        peek() == '\t'
        ) {
        if (
            peek() == '\t'
            ) {
            indent += 4;
        }
        else {
            indent++;
        }

        advance();
    }

    return indent;
}

std::string Lexer::readNumber() {
    std::string result;

    bool hasDot = false;

    while (true) {
        char c = peek();

        if (
            std::isdigit(
                static_cast<unsigned char>(c)
            )
            ) {
            result += c;
            advance();
            continue;
        }

        if (c == '.' && !hasDot) {
            hasDot = true;
            result += c;
            advance();
            continue;
        }

        break;
    }

    return result;
}

std::string Lexer::readIdentifier() {
    std::string result;

    while (
        std::isalnum(
            static_cast<unsigned char>(
                peek()
                )
        ) ||
        peek() == '_'
        ) {
        result += peek();

        advance();
    }

    return result;
}

std::string Lexer::readString() {
    std::string result;

    // Skip opening quote.
    advance();

    while (
        peek() != '"' &&
        peek() != '\0'
        ) {
        if (
            peek() == '\\'
            ) {
            advance();

            char escaped =
                peek();

            if (
                escaped == 'n'
                ) {
                result += '\n';
            }
            else if (
                escaped == 't'
                ) {
                result += '\t';
            }
            else if (
                escaped == '"'
                ) {
                result += '"';
            }
            else if (
                escaped == '\\'
                ) {
                result += '\\';
            }
            else {
                result += escaped;
            }

            advance();

            continue;
        }

        result += peek();

        advance();
    }

    // Skip closing quote.
    if (
        peek() == '"'
        ) {
        advance();
    }

    return result;
}

TokensType Lexer::getKeywordType(
    const std::string& word
) {
    if (
        word == "Var"
        ) {
        return TokensType::Var;
    }

    if (
        word == "Const"
        ) {
        return TokensType::Const;
    }

    if (
        word == "num"
        ) {
        return TokensType::Num;
    }

    if (
        word == "text"
        ) {
        return TokensType::Text;
    }

    if (
        word == "bool"
        ) {
        return TokensType::Bool;
    }

    if (
        word == "none"
        ) {
        return TokensType::None;
    }

    if (
        word == "Function"
        ) {
        return TokensType::Function;
    }

    if (
        word == "Input"
        ) {
        return TokensType::Input;
    }

    if (
        word == "Do"
        ) {
        return TokensType::Do;
    }

    if (
        word == "Throw"
        ) {
        return TokensType::Throw;
    }

    if (
        word == "Display"
        ) {
        return TokensType::Display;
    }

    if (
        word == "Set"
        ) {
        return TokensType::Set;
    }

    if (
        word == "If"
        ) {
        return TokensType::If;
    }

    if (
        word == "Else"
        ) {
        return TokensType::Else;
    }

    if (
        word == "Repeat"
        ) {
        return TokensType::Repeat;
    }

    if (
        word == "more"
        ) {
        return TokensType::More;
    }

    if (
        word == "less"
        ) {
        return TokensType::Less;
    }

    if (
        word == "eq"
        ) {
        return TokensType::Equal;
    }

    if (
        word == "true"
        ) {
        return TokensType::True;
    }

    if (
        word == "false"
        ) {
        return TokensType::False;
    }

    if (
        word == "Import"
        ) {
        return TokensType::Import;
    }

    if (
        word == "As"
        ) {
        return TokensType::As;
    }

    return TokensType::Identifier;
}

Token Lexer::makeToken(
    TokensType type,
    const std::string& value,
    int indent
) {
    return Token(
        type,
        value,
        line,
        indent
    );
}

Token Lexer::nextToken() {
    if (
        position >=
        source.size()
        ) {
        return makeToken(
            TokensType::EndOfFile,
            "",
            0
        );
    }

    /*
        Handle indentation and comments
        at the beginning of a line.
    */
    if (
        atLineStart
        ) {
        int indent =
            readIndent();

        atLineStart = false;

        /*
            Empty line.
        */
        if (
            peek() == '\n'
            ) {
            advance();

            return makeToken(
                TokensType::NewLine,
                "\n",
                indent
            );
        }

        /*
            Comment-only line.
        */
        if (
            peek() == '#' &&
            peekNext() == '#'
            ) {
            skipComment();

            if (
                peek() == '\n'
                ) {
                advance();

                return makeToken(
                    TokensType::NewLine,
                    "\n",
                    indent
                );
            }

            if (
                peek() == '\0'
                ) {
                return makeToken(
                    TokensType::EndOfFile,
                    "",
                    indent
                );
            }
        }

        char c =
            peek();

        if (
            c == '\0'
            ) {
            return makeToken(
                TokensType::EndOfFile,
                "",
                indent
            );
        }

        /*
            Tokens that can appear
            immediately after indentation.
        */
        if (
            std::isdigit(
                static_cast<unsigned char>(
                    c
                    )
            )
            ) {
            return makeToken(
                TokensType::Number,
                readNumber(),
                indent
            );
        }

        if (
            std::isalpha(
                static_cast<unsigned char>(
                    c
                    )
            ) ||
            c == '_'
            ) {
            std::string word =
                readIdentifier();

            return makeToken(
                getKeywordType(word),
                word,
                indent
            );
        }

        if (
            c == '"'
            ) {
            return makeToken(
                TokensType::String,
                readString(),
                indent
            );
        }
    }

    /*
        Ignore spaces between tokens.
    */
    skipSpaces();

    char c =
        peek();

    if (
        c == '\0'
        ) {
        return makeToken(
            TokensType::EndOfFile,
            "",
            0
        );
    }

    /*
        New line.
    */
    if (
        c == '\n'
        ) {
        advance();

        return makeToken(
            TokensType::NewLine,
            "\n",
            0
        );
    }

    int indent = 0;

    /*
        String.
    */
    if (
        c == '"'
        ) {
        return makeToken(
            TokensType::String,
            readString(),
            indent
        );
    }

    /*
        Number.
    */
    if (
        std::isdigit(
            static_cast<unsigned char>(
                c
                )
        )
        ) {
        return makeToken(
            TokensType::Number,
            readNumber(),
            indent
        );
    }

    /*
        Identifier / keyword.
    */
    if (
        std::isalpha(
            static_cast<unsigned char>(
                c
                )
        ) ||
        c == '_'
        ) {
        std::string word =
            readIdentifier();

        return makeToken(
            getKeywordType(word),
            word,
            indent
        );
    }

    /*
        Namespace separator.

        Example:

            math:add()
            math:VERSION
    */
    if (
        c == ':'
        ) {
        advance();

        return makeToken(
            TokensType::Colon,
            ":",
            indent
        );
    }

    /*
        Arrow.
    */
    if (
        c == '-' &&
        peekNext() == '>'
        ) {
        advance();
        advance();

        return makeToken(
            TokensType::Arrow,
            "->",
            indent
        );
    }

    /*
        Assignment / equality.
    */
    if (
        c == '='
        ) {
        advance();

        if (
            peek() == '='
            ) {
            advance();

            return makeToken(
                TokensType::Equal,
                "==",
                indent
            );
        }

        return makeToken(
            TokensType::Assign,
            "=",
            indent
        );
    }

    /*
        Not equal.
    */
    if (
        c == '!'
        ) {
        advance();

        if (
            peek() == '='
            ) {
            advance();

            return makeToken(
                TokensType::NotEqual,
                "!=",
                indent
            );
        }

        return makeToken(
            TokensType::Unknown,
            "!",
            indent
        );
    }

    /*
        Greater / greater equal.
    */
    if (
        c == '>'
        ) {
        advance();

        if (
            peek() == '='
            ) {
            advance();

            return makeToken(
                TokensType::GreaterEqual,
                ">=",
                indent
            );
        }

        return makeToken(
            TokensType::Greater,
            ">",
            indent
        );
    }

    /*
        Less / less equal.
    */
    if (
        c == '<'
        ) {
        advance();

        if (
            peek() == '='
            ) {
            advance();

            return makeToken(
                TokensType::LessEqual,
                "<=",
                indent
            );
        }

        return makeToken(
            TokensType::Less,
            "<",
            indent
        );
    }

    /*
        Plus.
    */
    if (
        c == '+'
        ) {
        advance();

        return makeToken(
            TokensType::Plus,
            "+",
            indent
        );
    }

    /*
        Minus.
    */
    if (
        c == '-'
        ) {
        advance();

        return makeToken(
            TokensType::Minus,
            "-",
            indent
        );
    }

    /*
        Multiply.
    */
    if (
        c == '*'
        ) {
        advance();

        return makeToken(
            TokensType::Multiply,
            "*",
            indent
        );
    }

    /*
        Divide.
    */
    if (
        c == '/'
        ) {
        advance();

        return makeToken(
            TokensType::Divide,
            "/",
            indent
        );
    }

    /*
        Left parenthesis.
    */
    if (
        c == '('
        ) {
        advance();

        return makeToken(
            TokensType::LeftParen,
            "(",
            indent
        );
    }

    /*
        Right parenthesis.
    */
    if (
        c == ')'
        ) {
        advance();

        return makeToken(
            TokensType::RightParen,
            ")",
            indent
        );
    }

    /*
        Left bracket.
    */
    if (
        c == '['
        ) {
        advance();

        return makeToken(
            TokensType::LeftBracket,
            "[",
            indent
        );
    }

    /*
        Right bracket.
    */
    if (
        c == ']'
        ) {
        advance();

        return makeToken(
            TokensType::RightBracket,
            "]",
            indent
        );
    }

    /*
        Comma.
    */
    if (
        c == ','
        ) {
        advance();

        return makeToken(
            TokensType::Comma,
            ",",
            indent
        );
    }

    /*
        Unknown character.
    */
    advance();

    return makeToken(
        TokensType::Unknown,
        std::string(
            1,
            c
        ),
        indent
    );
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        Token token =
            nextToken();

        tokens.push_back(
            token
        );

        if (
            token.type ==
            TokensType::EndOfFile
            ) {
            break;
        }
    }

    return tokens;
}

std::string TokensTypeToString(
    TokensType type
) {
    switch (type) {
    case TokensType::Number:
        return "Number";

    case TokensType::String:
        return "String";

    case TokensType::True:
        return "True";

    case TokensType::False:
        return "False";

    case TokensType::Identifier:
        return "Identifier";

    case TokensType::Var:
        return "Var";

    case TokensType::Const:
        return "Const";

    case TokensType::Num:
        return "Num";

    case TokensType::Text:
        return "Text";

    case TokensType::Bool:
        return "Bool";

    case TokensType::None:
        return "None";

    case TokensType::Display:
        return "Display";

    case TokensType::Set:
        return "Set";

    case TokensType::If:
        return "If";

    case TokensType::Else:
        return "Else";

    case TokensType::Repeat:
        return "Repeat";

    case TokensType::Do:
        return "Do";

    case TokensType::Function:
        return "Function";

    case TokensType::Input:
        return "Input";

    case TokensType::Throw:
        return "Throw";

    case TokensType::Import:
        return "Import";

    case TokensType::As:
        return "As";

    case TokensType::Colon:
        return "Colon";

    case TokensType::Assign:
        return "Assign";

    case TokensType::Arrow:
        return "Arrow";

    case TokensType::Plus:
        return "Plus";

    case TokensType::Minus:
        return "Minus";

    case TokensType::Multiply:
        return "Multiply";

    case TokensType::Divide:
        return "Divide";

    case TokensType::More:
        return "More";

    case TokensType::Less:
        return "Less";

    case TokensType::Equal:
        return "Equal";

    case TokensType::NotEqual:
        return "NotEqual";

    case TokensType::Greater:
        return "Greater";

    case TokensType::LessEqual:
        return "LessEqual";

    case TokensType::GreaterEqual:
        return "GreaterEqual";

    case TokensType::LeftParen:
        return "LeftParen";

    case TokensType::RightParen:
        return "RightParen";

    case TokensType::LeftBracket:
        return "LeftBracket";

    case TokensType::RightBracket:
        return "RightBracket";

    case TokensType::Comma:
        return "Comma";

    case TokensType::NewLine:
        return "NewLine";

    case TokensType::EndOfFile:
        return "EndOfFile";

    case TokensType::Unknown:
        return "Unknown";
    }

    return "Unknown";
}