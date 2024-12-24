#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "core.h"
#include "env.h"
#include "lexer.h"
#include "parser.h"
#include "result.h"

#include <functional>
#include <span>
#include <stack>
#include <string>
#include <vector>

using EvalResult = Result<TreeNode>;

class REPLEnv
{
    std::stack<Environment> env_stack;

    Environment* leaf_env{nullptr};

public:
    REPLEnv();
    EvalResult apply(std::string symbol, const std::span<const TreeNode> nodes);

    Environment& getCurrentEnv() const
    {
        return *leaf_env;
    }

    void pushEnv()
    {
        env_stack.emplace(leaf_env);
        leaf_env = &env_stack.top();
    }

    void popEnv()
    {
        assert(env_stack.size() > 1);
        env_stack.pop();
        leaf_env = &env_stack.top();
    }

private:
    EvalResult applyDef(const std::span<const TreeNode> nodes);
    EvalResult applyFn(const std::span<const TreeNode> nodes);
    EvalResult applyIf(const std::span<const TreeNode> nodes);
    EvalResult applyLet(const std::span<const TreeNode> nodes);
    EvalResult applyDo(const std::span<const TreeNode> nodes);
};

class GuardedEnv
{
private:
    REPLEnv* env;

public:
    GuardedEnv(REPLEnv* env_) :
        env{env_}
    {
        env->pushEnv();
    }

    ~GuardedEnv()
    {
        env->popEnv();
    }

    GuardedEnv(const GuardedEnv&) = delete;
    GuardedEnv& operator=(const GuardedEnv&) = delete;
};

inline REPLEnv::REPLEnv()
{
    env_stack.emplace();
    leaf_env = &env_stack.top();
    auto& root_env = *leaf_env;
    addCoreFunsToEnv(root_env);
}

EvalResult inline evalAST(const TreeNode& node, REPLEnv& env);

inline EvalResult addDefToEnv(const TreeNode& key, const TreeNode& val, REPLEnv& env)
{
    if (!isSymbol(key))
    {
        std::string error_message = "ERROR: def! without symbol for first param";
        return EvalResult(error_message, key.getToken());
    }

    auto evaluated = evalAST(val, env);
    if (evaluated.error())
        return evaluated;

    env.getCurrentEnv().set(key.symbol(), evaluated.get());

    return evaluated;
}

inline EvalResult REPLEnv::apply(std::string symbol, const std::span<const TreeNode> nodes)
{
    // First check special forms
    if (symbol == "def!")
    {
        return applyDef(nodes);
    }
    else if (symbol == "let*")
    {
        return applyLet(nodes);
    }
    else if (symbol == "if")
    {
        return applyIf(nodes);
    }
    else if (symbol == "do")
    {
        return applyDo(nodes);
    }

    // TODO - ranges?
    std::vector<TreeNode> evaluated;
    for (auto it = std::begin(nodes); it != std::end(nodes); ++it)
    {
        EvalResult eval_child = evalAST(*it, *this);
        if (eval_child.error())
            return eval_child;
        evaluated.push_back(eval_child.get());
    }

    const auto* val = leaf_env->get(symbol);
    if (!val)
    {
        std::string error_message = "ERROR: '" + symbol + "' not found";
        return EvalResult(error_message, Token{TokenKind::NUMBER, "0", 0});
    }
    if (!isFunc(*val))
    {
        std::string error_message = "ERROR: Cannot call '" + symbol + "'";
        return EvalResult(error_message, Token{TokenKind::NUMBER, "0", 0});
    }

    const auto& callable = val->callable();
    return callable(evaluated);
}

// Special Form application implementations
inline EvalResult REPLEnv::applyDef(const std::span<const TreeNode> nodes)
{
    if (nodes.size() != 2)
    {
        std::string error_message = "ERROR: def! without exactly 2 parameters";
        return EvalResult(error_message, Token{TokenKind::NUMBER, "0", 0});
    }
    auto& key = nodes[0];
    auto& val = nodes[1];
    auto evaluated = addDefToEnv(key, val, *this);
    return evaluated;
}

inline bool asBool(TreeNode& node)
{
    if (node.kind == NodeKind::ATOM)
    {
        if (isNil(node))
            return false;
        else if (isBool(node))
            return node.symbol() == "true";
        else
            return true;
    }
    else
    {
        return true;
    }
}

inline EvalResult makeNil()
{
    return TreeNode(NodeKind::ATOM, Token{TokenKind::NIL, "nil", 0});
}

inline EvalResult REPLEnv::applyDo(const std::span<const TreeNode> nodes)
{
    if (nodes.empty())
    {
        // Q: Or maybe return nil?
        std::string error_message = "ERROR: Cannot apply do to empty list";
        return EvalResult(error_message, Token{TokenKind::NUMBER, "0", 0});
    }

    for (const auto node : nodes.first(nodes.size() - 1))
    {
        // TODO - Error handling?
        evalAST(node, *this);
    }

    const auto& last = nodes[nodes.size() - 1];

    return evalAST(last, *this);
}

inline EvalResult REPLEnv::applyIf(const std::span<const TreeNode> nodes)
{
    if (nodes.size() < 2)
    {
        std::string error_message = "ERROR: if with < 2 parameters";
        return EvalResult(error_message, Token{TokenKind::NUMBER, "0", 0});
    }

    auto& cond = nodes[0];
    auto cond_result = evalAST(cond, *this);

    if (cond_result.error())
        return cond_result;

    if (asBool(cond_result.get()))
        return evalAST(nodes[1], *this);
    else if (nodes.size() >= 3)
        return evalAST(nodes[2], *this);
    else
        return makeNil();
}

inline EvalResult REPLEnv::applyLet(const std::span<const TreeNode> nodes)
{
    // Let's examine the children
    if (nodes.size() != 2)
    {
        std::string error_message = "Empty let";
        return EvalResult(error_message, Token{TokenKind::NUMBER, "0", 0});
    }
    auto& let_node = nodes[0];
    auto& bindings = let_node.children;
    auto& rest = nodes[1];
    if (bindings.size() % 2 != 0)
    {
        std::string error_message = "Let bindings of uneven length";
        return EvalResult(error_message, Token{TokenKind::NUMBER, "0", 0});
    }

    GuardedEnv g{this};
    for (size_t i = 0; i < bindings.size(); i += 2)
    {
        const auto& key = bindings[i];
        const auto& val = bindings[i + 1];
        auto result = addDefToEnv(key, val, *this);
        if (result.error())
        {
            return result;
        }
    }
    auto result = evalAST(rest, *this);
    return result;
}

EvalResult inline evalAST(const TreeNode& node, REPLEnv& env)
{
    switch (node.kind)
    {
    case NodeKind::ROOT:
        return evalAST(node.children[0], env);
    case NodeKind::ATOM:
        if (isSymbol(node))
        {
            auto& cur_env = env.getCurrentEnv();
            auto* res = cur_env.get(node.symbol());
            if (res == nullptr)
                return EvalResult("Could not resolve '" + node.symbol() + "'");
            return *res;
        }
        else
        {
            return node;
        }
    case NodeKind::LIST: {
        if (node.children.empty())
        {
            // Returns a _copy_
            return node;
        }
        const auto& func = node.children[0];
        std::vector<TreeNode> evaluated;

        std::span rest{++std::begin(node.children), std::end(node.children)};
        // TODO: Pass the func... consider lambdas!
        return env.apply(func.symbol(), rest);
    }
    case NodeKind::VECTOR:
    case NodeKind::HASHMAP: {
        if (node.children.empty())
        {
            // Returns a _copy_
            return node;
        }
        // TODO - Handle Hashmap sanely!
        std::vector<TreeNode> evaluated;
        TreeNode result{node.kind, Token{TokenKind::LPAREN, node.symbol(), 0}};
        for (const auto& child : node.children)
        {
            EvalResult eval_child = evalAST(child, env);
            if (eval_child.error())
                return eval_child;
            result.appendChild(eval_child.get());
        }
        return result;
    }
    }

    assert(0 && "Should not happen");
}

#endif /* EVALUATOR */
