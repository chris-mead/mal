#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

#include <cassert>
#include <optional>
#include <stack>

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

#endif /* PARSER_H */
