#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "lexer.h"
#include "result.h"
#include "parser.h"

#include <string>

using EvalResult = Result<TreeNode>;

class REPLEnv
{
public:
    EvalResult apply(std::string symbol, const std::vector<TreeNode> nodes) const;
};

EvalResult inline REPLEnv::apply(std::string symbol, const std::vector<TreeNode> nodes) const
{
    if(symbol == "+")
    {
        int acc = 0;
        for(const auto& node : nodes)
        {
            auto num = std::atoi(node.token.text.c_str());
            acc += num;
        }
        return TreeNode(NodeKind::ATOM, Token {TokenKind::NUMBER, std::to_string(acc), 0});
    }
    else if(symbol == "-")
    {
        int acc;
        if(nodes.empty())
        {
            acc = 0;
        }
        else
        {
            auto iter = std::begin(nodes);
            acc = std::atoi(iter->token.text.c_str());
            ++iter;
            while(iter != std::end(nodes))
            {
                acc -= std::atoi(iter->token.text.c_str());
                ++iter;
            }
        }
        return TreeNode(NodeKind::ATOM, Token {TokenKind::NUMBER, std::to_string(acc), 0});
    }
    else if(symbol == "*")
    {
        int acc = 1;
        for(const auto& node : nodes)
        {
            auto num = std::atoi(node.token.text.c_str());
            acc *= num;
        }
        return TreeNode(NodeKind::ATOM, Token {TokenKind::NUMBER, std::to_string(acc), 0});
    }
    else if(symbol == "/")
    {
        int acc;
        if(nodes.size() < 2)
        {
            acc = 0;
        }
        else
        {
            auto iter = std::begin(nodes);
            acc = std::atoi(iter->token.text.c_str());
            ++iter;
            while(iter != std::end(nodes))
            {
                int denom = std::atoi(iter->token.text.c_str());
                if(denom == 0)
                    return {"Divide by 0", iter->token};

                acc /= denom;
                ++iter;
            }
        }
        return TreeNode(NodeKind::ATOM, Token {TokenKind::NUMBER, std::to_string(acc), 0});
    }
    else
    {
        return TreeNode(NodeKind::ATOM, Token {TokenKind::NUMBER, "0", 0});
    }
}

EvalResult inline evalAST(const TreeNode& node, const REPLEnv& env)
{
    switch(node.kind)
    {
    case NodeKind::ROOT:
        return evalAST(node.children[0], env);
    case NodeKind::ATOM:
        return node;
    case NodeKind::LIST: {
        if(node.children.empty())
        {
            // Returns a _copy_
            return node;
        }
        const auto& func = node.children[0];
        std::vector<TreeNode> evaluated;
        for(auto it = ++std::begin(node.children); it != std::end(node.children); ++it)
        {
            EvalResult eval_child = evalAST(*it, env);
            if(eval_child.error())
                return eval_child;

            evaluated.push_back(eval_child.get());
        }
        return env.apply(func.token.text, evaluated);
    }
    case NodeKind::VECTOR:
    case NodeKind::HASHMAP: {
        if(node.children.empty())
        {
            // Returns a _copy_
            return node;
        }
        // TODO - Handle Hashmap sanely!
        std::vector<TreeNode> evaluated;
        TreeNode result {node.kind, Token {node.token.kind, node.token.text, 0}};
        for(const auto& child : node.children)
        {
            EvalResult eval_child = evalAST(child, env);
            if(eval_child.error())
                return eval_child;
            result.appendChild(eval_child.get());
        }
        return result;
    }
    }

    assert(0 && "Should not happen");
}

#endif /* EVALUATOR */


