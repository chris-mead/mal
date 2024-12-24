#ifndef ENV_H
#define ENV_H

#include "parser.h"

#include <assert.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using Func = std::function<TreeNode(const std::vector<TreeNode>&)>;

class Environment
{
    const Environment* outer{nullptr};
    std::unordered_map<std::string, Func> data;

public:
    Environment(){};

    Environment(const Environment* outer_) :
        outer{outer_}
    {
    }

    Environment(const std::vector<std::string> binds, const std::vector<Func> exprs)
    {
        // TODO: Snazzy STL way to do this I am sure
        auto to_bind = std::min(binds.size(), exprs.size());
        for (decltype(to_bind) i=0; i < to_bind; i++)
        {
            set(binds[i], exprs[i]);
        }
    }

    void set(std::string symbol, Func node);

    const Environment* find(std::string symbol) const;

    const Func* get(std::string symbol) const;
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

inline const Func* Environment::get(std::string symbol) const
{
    const auto* containing_env = find(symbol);
    if (containing_env == nullptr)
        return nullptr;

    auto it = containing_env->data.find(symbol);
    assert(it != std::end(containing_env->data));
    return &(it->second);
}

inline void Environment::set(std::string symbol, Func node)
{
    data[symbol] = std::move(node);
}

#endif /* ENV_H */
