#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

enum class TokenKind
{
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    SYM,
    NUMBER,
    STRING,
    BOOL,
    INVALID
};

class Token
{
public:
    TokenKind kind;
    std::string text;
    size_t pos; // In the parent string

    Token() :
        kind{TokenKind::INVALID}
    {
    }

    Token(TokenKind kind_, std::string_view text_, size_t pos_) :
        kind{kind_},
        text{text_},
        pos{pos_}
    {
    }
};

using lex_char_t = std::string::value_type;
constexpr lex_char_t LEX_EOF = std::char_traits<lex_char_t>::eof();

using TokenStream = std::vector<Token>;

inline bool isWS(lex_char_t c)
{
    return std::isspace(c) || c == ',';
}

inline bool isLineCommentDelim(lex_char_t c)
{
    return c == ';';
}

inline bool isMinus(lex_char_t c)
{
    return c == '-';
}

inline bool isDigit(lex_char_t c)
{
    return c >= '0' && c <= '9';
}

inline bool isSymEnd(lex_char_t c)
{
    return isWS(c) || c == '(' || c == ')' || c == LEX_EOF;
}

inline bool isStringDelim(lex_char_t c)
{
    return c == '"';
}

class Lexer
{
private:
    class LexerState
    {
        std::string_view text;
        std::size_t pos{0};
        lex_char_t look_ahead{LEX_EOF};

    public:
        LexerState(std::string_view text_) :
            text{text_}
        {
            if (!text_.empty())
                look_ahead = text[pos];
        }

        lex_char_t lookAhead() const
        {
            return look_ahead;
        }

        void consume()
        {
            ++pos;
            if (pos < text.length())
                look_ahead = text[pos];
            else
                look_ahead = LEX_EOF;
        }

        void unconsume()
        {
            if (pos == 0)
                return;

            --pos;
            if (pos < text.length())
                look_ahead = text[pos];
            else
                look_ahead = LEX_EOF;
        }

        Token lparen()
        {
            Token tok{TokenKind::LPAREN, "(", pos};
            consume();
            return tok;
        }

        Token rparen()
        {
            Token tok{TokenKind::RPAREN, ")", pos};
            consume();
            return tok;
        }

        Token lbracket()
        {
            Token tok{TokenKind::LBRACKET, "[", pos};
            consume();
            return tok;
        }

        Token rbracket()
        {
            Token tok{TokenKind::RBRACKET, "]", pos};
            consume();
            return tok;
        }

        Token lbrace()
        {
            Token tok{TokenKind::LBRACE, "{", pos};
            consume();
            return tok;
        }

        Token rbrace()
        {
            Token tok{TokenKind::RBRACE, "}", pos};
            consume();
            return tok;
        }

        Token minus()
        {
            // This is a bit finicky as we need to look further ahead to
            // disambiguate negative literal from symbol starting with '-'
            consume();
            bool is_digit = isDigit(lookAhead());
            unconsume();
            if (is_digit)
            {
                return number();
            }
            else
            {
                return symbol();
            }
        }

        Token number()
        {
            const auto tok_start = pos;
            consume();
            while (isDigit(lookAhead()))
            {
                consume();
            }
            std::string_view tok_text = text.substr(tok_start, pos - tok_start);
            return Token{TokenKind::NUMBER, tok_text, tok_start};
        }

        Token symbol()
        {
            const auto tok_start = pos;
            consume();
            while (!isSymEnd(lookAhead()))
            {
                consume();
            }
            // TODO: this _might_ be a keyword... in which case we need to promote it
            std::string_view tok_text = text.substr(tok_start, pos - tok_start);
            return Token{TokenKind::SYM, tok_text, tok_start};
        }

        Token string()
        {
            const auto tok_start = pos;
            consume();
            bool escaped{false};
            while (escaped || !isStringDelim(lookAhead()))
            {
                if (lookAhead() == LEX_EOF)
                {
                    return Token(TokenKind::INVALID, "EOF in string", tok_start);
                }
                if (escaped)
                {
                    escaped = false;
                }
                else
                {
                    if (lookAhead() == '\\')
                        escaped = true;
                }
                consume();
            }
            // Consume the end strng delimiter
            consume();
            // TODO: this _might_ be a keyword... in which case we need to promote it?
            std::string_view tok_text = text.substr(tok_start, pos - tok_start);
            return Token{TokenKind::STRING, tok_text, tok_start};
        }

        void lineComment()
        {
            consume();
            while (lookAhead() != '\n' && lookAhead() != LEX_EOF)
            {
                consume();
            }
            // Consume the newline
            consume();
            return;
        }
    };

public:
    TokenStream tokenise(std::string line)
    {
        LexerState state{line};

        TokenStream result;

        bool done = false;
        bool in_line_comment = false;
        while (!done)
        {
            const auto c = state.lookAhead();
            if (in_line_comment)
            {
                if (c == '\n')
                    in_line_comment = false;
                state.consume();
                continue;
            }

            switch (c)
            {
            case EOF:
                done = true;
                break;
            case '(': {
                result.push_back(state.lparen());
                break;
            }
            case ')': {
                result.push_back(state.rparen());
                break;
            }
            case '[': {
                result.push_back(state.lbracket());
                break;
            }
            case ']': {
                result.push_back(state.rbracket());
                break;
            }
            case '{': {
                result.push_back(state.lbrace());
                break;
            }
            case '}': {
                result.push_back(state.rbrace());
                break;
            }
            default:
                if (isLineCommentDelim(c))
                {
                    state.lineComment();
                    continue;
                }
                if (isWS(c))
                {
                    state.consume();
                    continue;
                }
                if (isMinus(c))
                {
                    result.push_back(state.minus());
                }
                else if (isDigit(c))
                {
                    result.push_back(state.number());
                }
                else if (isStringDelim(c))
                {
                    result.push_back(state.string());
                }
                else
                {
                    result.push_back(state.symbol());
                    // done = true;
                    // std::cout << "CARPET: Unrecognised char " << c << " (" << int(c) << ")\n";
                }
                break;
            }
        }
        // We do all this up front, meh

        return result;
    }
};

#endif /* LEXERH_H */
