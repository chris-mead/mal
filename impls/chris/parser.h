#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"
#include "result.h"

#include <cassert>
#include <functional>
#include <optional>
#include <stack>
#include <vector>

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
                if (isRoot(*node) && !node->empty())
                {
                    return {"unbalanced (non-nested list start)", tok};
                }
                // TODO - Vector + Hashmap
                auto new_node = TreeNode::makeNode<NodeKind::NIL>("nil");
                if (tok.kind == TokenKind::LPAREN)
                {
                    new_node = TreeNode::makeNode<NodeKind::LIST>(tok);
                }
                else if (tok.kind == TokenKind::LBRACKET)
                {
                    new_node = TreeNode::makeNode<NodeKind::VECTOR>(tok);
                }
                else if (tok.kind == TokenKind::LBRACE)
                {
                    new_node = TreeNode::makeNode<NodeKind::HASHMAP>(tok);
                }
                else
                {
                    return {"unknown aggregate type", tok};
                }

                node->appendChild(new_node);
                stack.push(node);
                node = &node->children().back();
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
                if (node->kind == NodeKind::ROOT && !node->empty())
                {
                    return {"unbalanced (Multiple-Atoms outside list)", tok};
                }
                auto new_node = TreeNode::makeNode<NodeKind::NIL>("nil");
                switch (tok.kind)
                {
                case TokenKind::SYM:
                    new_node = TreeNode::makeNode<NodeKind::SYMBOL>(tok.text, tok);
                    break;
                case TokenKind::NUMBER: {
                    // TODO : refactor to be numeric type
                    auto n = std::atoi(tok.text.c_str());
                    new_node = TreeNode::makeNode<NodeKind::NUMBER>(n, tok);
                    break;
                }
                case TokenKind::STRING: {
                    new_node = TreeNode::makeNode<NodeKind::STRING>(tok.text, tok);
                    break;
                }
                case TokenKind::BOOL: {
                    bool b = tok.text == "true";
                    new_node = TreeNode::makeNode<NodeKind::BOOL>(b, tok);
                    break;
                }
                case TokenKind::NIL:
                    // See default above
                    break;
                case TokenKind::INVALID:
                default:
                    // TODO - ERROR HERE!
                    // Shouldn't happen
                    break;
                }
                // TODO : std::move?
                node->appendChild(new_node);
            }
        }

        if (node->kind != NodeKind::ROOT)
        {
            // TODO - Improve this...o
            return {"unbalanced tree", Token()};
        }
        if (node->empty())
        {
            return {"No tokens parsed", Token()};
        }
        return root;
    }
};

#endif /* PARSER_H */
