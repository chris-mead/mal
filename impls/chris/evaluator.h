#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "lexer.h"
#include "env.h"
#include "result.h"
#include "parser.h"

#include <functional>
#include <string>
#include <span>
#include <vector>

using EvalResult = Result<TreeNode>;

class REPLEnv
{
    Environment root_env{};

    Environment* leaf_env = &root_env;
    
public:
    REPLEnv();
    EvalResult apply(std::string symbol, const std::span<const TreeNode> nodes) const;

    Environment& getCurrentEnv() const
    {
        return *leaf_env;
    }
};

inline REPLEnv::REPLEnv()
{
    root_env.set("+", [](auto& nodes)
                      {
                          int acc = 0;
                          for(const auto& node : nodes)
                          {
                              auto num = std::atoi(node.token.text.c_str());
                              acc += num;
                          }
                          return TreeNode(NodeKind::ATOM, Token {TokenKind::NUMBER, std::to_string(acc), 0});
                      });
    root_env.set("-", [](auto& nodes)
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
                      });
    
    root_env.set("*", [](auto& nodes)
                      {
                          int acc = 1;
                          for(const auto& node : nodes)
                          {
                              auto num = std::atoi(node.token.text.c_str());
                              acc *= num;
                          }
                          return TreeNode(NodeKind::ATOM, Token {TokenKind::NUMBER, std::to_string(acc), 0});
                      });

    root_env.set("/", [](auto& nodes)
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
                                // TODO - Divide by zero
                                // if(denom == 0)
                                //     return EvalResult{"Divide by 0", Token{TokenKind::NUMBER, "0", 0}};

                                acc /= denom;
                                ++iter;
                            }
                        }
                        return TreeNode(NodeKind::ATOM, Token {TokenKind::NUMBER, std::to_string(acc), 0});
                    });
        
    
}

EvalResult inline evalAST(const TreeNode& node, const REPLEnv& env);

EvalResult inline REPLEnv::apply(std::string symbol, const std::span<const TreeNode> nodes) const
{
    // First check special forms
    if (symbol == "def!")
    {
        if(nodes.size() != 2)
        {
            std::string error_message = "ERROR: def! without exactly 2 parameters";
            return EvalResult(error_message, Token {TokenKind::NUMBER, "0", 0});
        }
        auto symbol = nodes[0];
        auto rest = nodes[1];

        if (symbol.token.kind != TokenKind::SYM)
        {
            std::string error_message = "ERROR: def! without symbol for first param";            
            return EvalResult(error_message, symbol.token);
        }

        auto evaluated = evalAST(rest, *this);
        if (evaluated.error())
            return evaluated;

        leaf_env->set(symbol.token.text, [evaluated](auto&) { return evaluated.get(); });

        return evaluated.get();
    }

    // TODO - ranges?
    std::vector<TreeNode> evaluated;
    for(auto it = std::begin(nodes); it != std::end(nodes); ++it)
    {
        EvalResult eval_child = evalAST(*it, *this);
        if(eval_child.error())
            return eval_child;
        evaluated.push_back(eval_child.get());
    }
    
    const auto* func = leaf_env->get(symbol);
    if (func)
        return (*func)(evaluated);
    std::string error_message = "ERROR: Could not find \"" + symbol + "\"";
    return EvalResult(error_message, Token {TokenKind::NUMBER, "0", 0});
}

EvalResult inline evalAST(const TreeNode& node, const REPLEnv& env)
{
    switch(node.kind)
    {
    case NodeKind::ROOT:
        return evalAST(node.children[0], env);
    case NodeKind::ATOM:
        if (node.token.kind == TokenKind::SYM)
        {
            auto& cur_env = env.getCurrentEnv();
            auto* res = cur_env.get(node.token.text);
            if (res == nullptr)
                return EvalResult("Could not resolve symbol", node.token);
            // TODO - Replace with std::variant
            std::vector<TreeNode> dummy;
            auto* func = res;
            return (*func)(dummy);
        }
        else
        {
            return node;
        }
    case NodeKind::LIST: {
        if(node.children.empty())
        {
            // Returns a _copy_
            return node;
        }
        const auto& func = node.children[0];
        std::vector<TreeNode> evaluated;

        std::span rest{++std::begin(node.children), std::end(node.children)};
        return env.apply(func.token.text, rest);
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


