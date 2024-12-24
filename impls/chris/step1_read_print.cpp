#include <cassert>
#include <iostream>
#include <optional>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

const std::string_view DEFAULT_PROMPT {"user> "};

class ConfigInfo
{
public:
    std::string_view prompt {DEFAULT_PROMPT};
};

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
        kind {TokenKind::INVALID}
    {
    }

    Token(TokenKind kind_, std::string_view text_, size_t pos_) :
        kind {kind_},
        text {text_},
        pos {pos_}
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
        std::size_t pos {0};
        lex_char_t look_ahead {LEX_EOF};

        lex_char_t peek() const
        {
            return look_ahead;
        }

    public:
        LexerState(std::string_view text_) :
            text {text_}
        {
            if(!text_.empty())
                look_ahead = text[pos];
        }

        lex_char_t lookAhead() const
        {
            return look_ahead;
        }

        void consume()
        {
            ++pos;
            if(pos < text.length())
                look_ahead = text[pos];
            else
                look_ahead = LEX_EOF;
        }

        Token lparen()
        {
            Token tok {TokenKind::LPAREN, "(", pos};
            consume();
            return tok;
        }

        Token rparen()
        {
            Token tok {TokenKind::RPAREN, ")", pos};
            consume();
            return tok;
        }

        Token lbracket()
        {
            Token tok {TokenKind::LBRACKET, "[", pos};
            consume();
            return tok;
        }

        Token rbracket()
        {
            Token tok {TokenKind::RBRACKET, "]", pos};
            consume();
            return tok;
        }

        Token lbrace()
        {
            Token tok {TokenKind::LBRACE, "{", pos};
            consume();
            return tok;
        }

        Token rbrace()
        {
            Token tok {TokenKind::RBRACE, "}", pos};
            consume();
            return tok;
        }

        Token minus()
        {
            const auto tok_start = pos;
            consume();
            pos--;
            look_ahead = '-';

            if(isDigit(lookAhead()))
            {
                return number();
            }
            else
            {
                return symbol();
            }

            return number();
        }

        Token number()
        {
            const auto tok_start = pos;
            consume();
            auto pos_end = pos;
            while(isDigit(lookAhead()))
            {
                consume();
            }
            std::string_view tok_text = text.substr(tok_start, pos - tok_start);
            return Token {TokenKind::NUMBER, tok_text, tok_start};
        }

        Token symbol()
        {
            const auto tok_start = pos;
            consume();
            auto pos_end = pos;
            while(!isSymEnd(lookAhead()))
            {
                consume();
            }
            // TODO: this _might_ be a keyword... in which case we need to promote it
            std::string_view tok_text = text.substr(tok_start, pos - tok_start);
            return Token {TokenKind::SYM, tok_text, tok_start};
        }

        Token string()
        {
            const auto tok_start = pos;
            consume();
            auto pos_end = pos;
            bool escaped {false};
            while(escaped || !isStringDelim(lookAhead()))
            {
                if(lookAhead() == LEX_EOF)
                {
                    return Token(TokenKind::INVALID, "EOF in string", tok_start);
                }
                if(escaped)
                {
                    escaped = false;
                }
                else
                {
                    if(lookAhead() == '\\')
                        escaped = true;
                }
                consume();
            }
            // Consume the end strng delimiter
            consume();
            // TODO: this _might_ be a keyword... in which case we need to promote it?
            std::string_view tok_text = text.substr(tok_start, pos - tok_start);
            return Token {TokenKind::STRING, tok_text, tok_start};
        }

        void lineComment()
        {
            consume();
            while(lookAhead() != '\n' && lookAhead() != LEX_EOF)
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
        LexerState state {line};

        TokenStream result;

        bool done = false;
        bool in_line_comment = false;
        while(!done)
        {
            const auto c = state.lookAhead();
            if(in_line_comment)
            {
                if(c == '\n')
                    in_line_comment = false;
                state.consume();
                continue;
            }

            switch(c)
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
                if(isLineCommentDelim(c))
                {
                    state.lineComment();
                    continue;
                }
                if(isWS(c))
                {
                    state.consume();
                    continue;
                }
                if(isMinus(c))
                {
                    result.push_back(state.minus());
                }
                else if(isDigit(c))
                {
                    result.push_back(state.number());
                }
                else if(isStringDelim(c))
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

enum class NodeKind
{
    ROOT,
    ATOM,
    LIST,
    VECTOR,
    HASHMAP
};

// This will perform terribly... think about allocators
class TreeNode
{
public:
    NodeKind kind;
    Token token;
    std::vector<TreeNode> children;

    TreeNode() :
        kind {NodeKind::ROOT}
    {
    }

    TreeNode(NodeKind kind_, Token token_) :
        kind {kind_},
        token {token_}
    {
    }

    void appendChild(const TreeNode& node)
    {
        children.push_back(node);
    }
};

class ParseResult
{
    std::optional<TreeNode> result;
    std::string error_message;
    // Shut the compiler up
    Token token {TokenKind::SYM, "", 0};

public:
    ParseResult(std::string error_message_, Token token_) :
        error_message {error_message_},
        token {token_}
    {
    }

    ParseResult(TreeNode node_) :
        result {node_}
    {
    }

    const std::string& message() const
    {
        return error_message;
    }

    bool error() const
    {
        return !bool(result);
    }

    const TreeNode& get()
    {
        return result.value();
    }
};

constexpr bool isStartAggregateDelim(const Token& tok)
{
    return tok.kind == TokenKind::LPAREN || tok.kind == TokenKind::LBRACKET || tok.kind == TokenKind::LBRACE;
}

constexpr bool isEndAggregateDelim(const Token& tok)
{
    return tok.kind == TokenKind::RPAREN || tok.kind == TokenKind::RBRACKET || tok.kind == TokenKind::RBRACE;
}

constexpr NodeKind getAggregateKind(const Token& tok)
{
    if(tok.kind == TokenKind::LPAREN || tok.kind == TokenKind::RPAREN)
        return NodeKind::LIST;
    else if(tok.kind == TokenKind::LBRACKET || tok.kind == TokenKind::RBRACKET)
        return NodeKind::VECTOR;
    else if(tok.kind == TokenKind::LBRACE || tok.kind == TokenKind::RBRACE)
        return NodeKind::HASHMAP;

    // Should not happen!
    assert(0);
    return NodeKind::ROOT;
}

class Parser
{
public:
    ParseResult parse(TokenStream tok_stream)
    {
        TreeNode root {};
        TreeNode* node = &root;

        std::stack<TreeNode*> stack;
        for(const Token& tok : tok_stream)
        {
            if(tok.kind == TokenKind::INVALID)
            {
                return {tok.text, tok};
            }
            if(isStartAggregateDelim(tok))
            {
                if(node->kind == NodeKind::ROOT && !node->children.empty())
                {
                    return {"unbalanced (non-nested list start)", tok};
                }
                node->children.emplace_back(getAggregateKind(tok), tok);
                stack.push(node);
                node = &node->children.back();
            }
            else if(isEndAggregateDelim(tok))
            {
                const auto end_kind = getAggregateKind(tok);
                if(node->kind != end_kind)
                    return {"unbalanced aggregate-kind", tok};
                assert(!stack.empty());
                node = stack.top();
                stack.pop();
            }
            else
            {
                if(node->kind == NodeKind::ROOT && !node->children.empty())
                {
                    return {"unbalanced (Multiple-Atoms outside list)", tok};
                }
                node->children.emplace_back(NodeKind::ATOM, tok);
            }
        }

        if(node->kind != NodeKind::ROOT)
        {
            return {"unbalanced tree", node->token};
        }
        if(node->children.empty())
        {
            return {"No tokens parsed", Token()};
        }
        return root;
    }
};

class InterpreterState
{
private:
    std::istream* in;
    std::string prompt;

public:
    InterpreterState(const ConfigInfo& config_info_,
                     std::istream* in_) :
        in {in_},
        prompt {config_info_.prompt}
    {
    }

    std::string readLine()
    {
        std::string line;
        std::getline(*in, line);
        return line;
    }

    bool moreInput() const
    {
        return !in->eof();
    }

    void printPrompt()
    {
        std::cout << prompt;
    }

    void printVal(std::string val)
    {
        std::cout << "\n"
                  << val << "\n";
    }
};

void printTree(std::ostream& out, const TreeNode& node)
{
    if(node.kind == NodeKind::ROOT)
    {
        assert(!node.children.empty());
        printTree(out, node.children[0]);
    }
    else if(node.kind == NodeKind::LIST)
    {
        out << "(";
        std::string sep = "";
        for(const auto& child : node.children)
        {
            out << sep;
            printTree(out, child);
            sep = " ";
        }
        out << ")";
    }
    else if(node.kind == NodeKind::VECTOR)
    {
        out << "[";
        std::string sep = "";
        for(const auto& child : node.children)
        {
            out << sep;
            printTree(out, child);
            sep = " ";
        }
        out << "]";
    }
    else if(node.kind == NodeKind::HASHMAP)
    {
        out << "{";
        std::string sep = "";
        for(const auto& child : node.children)
        {
            out << sep;
            printTree(out, child);
            sep = " ";
        }
        out << "}";
    }
    else
    {
        out << node.token.text;
    }
}

int mainLoop(const ConfigInfo& config_info)
{
    InterpreterState state {config_info, &std::cin};
    while(state.moreInput())
    {
        state.printPrompt();
        std::string line = state.readLine();
        Lexer lexer;

        auto tokens = lexer.tokenise(line);

        Parser parser;

        auto parse_result = parser.parse(tokens);

        if(parse_result.error())
        {
            std::cout << "ERROR: " << parse_result.message() << "\n";
        }
        else
        {
            const auto& root_node = parse_result.get();
            printTree(std::cout, root_node);
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    return 0;
}

int main()
{
    ConfigInfo config_info;
    const auto result = mainLoop(config_info);
    return result;
}
