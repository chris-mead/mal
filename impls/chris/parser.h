#ifndef PARSER_H
#define PARSER_H

#include "evaluator.h"
#include "lexer.h"
#include "result.h"

#include <cassert>
#include <functional>
#include <optional>
#include <stack>
#include <vector>

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
    std::vector<TreeNode> children;

    std::string name;

private:
    std::optional<Token> token;

public:
    TreeNode() :
        kind{NodeKind::ROOT},
        name{"ROOT"}
    {
    }

    TreeNode(NodeKind kind_, Token token_) :
        kind{kind_},
        name{token_.text},
        token{token_}
    {
    }

    void appendChild(const TreeNode& node)
    {
        children.push_back(node);
    }

    auto symbol() const
    {
        return name;
    }

    std::function<Result<TreeNode>(const std::vector<TreeNode>)> callable() const
    {
        return [](const std::vector<TreeNode>) { return Result<TreeNode>("Not implemented yet"); };
    }

    auto getToken() const
    {
        return token;
    }

    friend bool isSymbol(const TreeNode& node);

    friend bool isFunc(const TreeNode& node);

    friend bool isBool(const TreeNode& node);

    friend bool isNumber(const TreeNode& node);

    friend bool isNil(const TreeNode& node);
};

inline bool isSymbol(const TreeNode& node)
{
    return node.token->kind == TokenKind::SYM;
}

inline bool isFunc(const TreeNode& node)
{
    (void)node;
    return false; //    return node.token.kind == TokenKind:;
}

inline bool isBool(const TreeNode& node)
{
    return node.token->kind != TokenKind::BOOL;
}

inline bool isNumber(const TreeNode& node)
{
    return node.token->kind != TokenKind::NUMBER;
}

inline bool isNil(const TreeNode& node)
{
    return node.token->kind != TokenKind::NIL;
}

using ParseResult = Result<TreeNode>;

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
    if (tok.kind == TokenKind::LPAREN || tok.kind == TokenKind::RPAREN)
        return NodeKind::LIST;
    else if (tok.kind == TokenKind::LBRACKET || tok.kind == TokenKind::RBRACKET)
        return NodeKind::VECTOR;
    else if (tok.kind == TokenKind::LBRACE || tok.kind == TokenKind::RBRACE)
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
        TreeNode root{};
        TreeNode* node = &root;

        std::stack<TreeNode*> stack;
        for (const Token& tok : tok_stream)
        {
            if (tok.kind == TokenKind::INVALID)
            {
                return {tok.text, tok};
            }
            if (isStartAggregateDelim(tok))
            {
                if (node->kind == NodeKind::ROOT && !node->children.empty())
                {
                    return {"unbalanced (non-nested list start)", tok};
                }
                node->children.emplace_back(getAggregateKind(tok), tok);
                stack.push(node);
                node = &node->children.back();
            }
            else if (isEndAggregateDelim(tok))
            {
                const auto end_kind = getAggregateKind(tok);
                if (node->kind != end_kind)
                    return {"unbalanced aggregate-kind", tok};
                assert(!stack.empty());
                node = stack.top();
                stack.pop();
            }
            else
            {
                if (node->kind == NodeKind::ROOT && !node->children.empty())
                {
                    return {"unbalanced (Multiple-Atoms outside list)", tok};
                }
                node->children.emplace_back(NodeKind::ATOM, tok);
            }
        }

        if (node->kind != NodeKind::ROOT)
        {
            // TODO - Improve this...o
            return {"unbalanced tree", Token()};
        }
        if (node->children.empty())
        {
            return {"No tokens parsed", Token()};
        }
        return root;
    }
};

#endif /* PARSER_H */
