#ifndef ENV_H
#define ENV_H

#include "ast.h"

#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Environment
{
    const Environment* outer{nullptr};
    std::unordered_map<std::string, TreeNode> data;

public:
    Environment(){};

    Environment(const Environment* outer_) :
        outer{outer_}
    {
    }

    Environment(const std::vector<std::string> binds, const std::vector<TreeNode> exprs)
    {
        // TODO: Snazzy STL way to do this I am sure
        auto to_bind = std::min(binds.size(), exprs.size());
        for (decltype(to_bind) i = 0; i < to_bind; i++)
        {
            set(binds[i], exprs[i]);
        }
    }

    // TODO: move + copy semantics
    void set(std::string symbol, TreeNode node);

    const Environment* find(std::string symbol) const;

    const TreeNode* get(std::string symbol) const;
};

// TODO - std::optional?
inline const Environment* Environment::find(std::string symbol) const
{
    if (data.contains(symbol))
        return this;
    else if (outer != nullptr)
        return outer->find(symbol);
    else
        return nullptr;
}

inline const TreeNode* Environment::get(std::string symbol) const
{
    const auto* containing_env = find(symbol);
    if (containing_env == nullptr)
        return nullptr;

    auto it = containing_env->data.find(symbol);
    assert(it != std::end(containing_env->data));
    return &(it->second);
}

inline void Environment::set(std::string symbol, TreeNode node)
{
    // TODO: Realy?
    data[symbol] = std::move(node);
}

#endif /* ENV_H */
