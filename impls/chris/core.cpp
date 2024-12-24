#include "core.h"

void addCoreFunsToEnv(Environment& env)
{
    env.set("+", [](auto& nodes) {
        int acc = 0;
        for (const auto& node : nodes)
        {
            auto num = std::atoi(node.token.text.c_str());
            acc += num;
        }
        return TreeNode(NodeKind::ATOM, Token{TokenKind::NUMBER, std::to_string(acc), 0});
    });
    env.set("-", [](auto& nodes) {
        int acc;
        if (nodes.empty())
        {
            acc = 0;
        }
        else
        {
            auto iter = std::begin(nodes);
            acc = std::atoi(iter->token.text.c_str());
            ++iter;
            while (iter != std::end(nodes))
            {
                acc -= std::atoi(iter->token.text.c_str());
                ++iter;
            }
        }
        return TreeNode(NodeKind::ATOM, Token{TokenKind::NUMBER, std::to_string(acc), 0});
    });

    env.set("*", [](auto& nodes) {
        int acc = 1;
        for (const auto& node : nodes)
        {
            auto num = std::atoi(node.token.text.c_str());
            acc *= num;
        }
        return TreeNode(NodeKind::ATOM, Token{TokenKind::NUMBER, std::to_string(acc), 0});
    });

    env.set("/", [](auto& nodes) {
        int acc;
        if (nodes.size() < 2)
        {
            acc = 0;
        }
        else
        {
            auto iter = std::begin(nodes);
            acc = std::atoi(iter->token.text.c_str());
            ++iter;
            while (iter != std::end(nodes))
            {
                int denom = std::atoi(iter->token.text.c_str());
                // TODO - Divide by zero
                // if(denom == 0)
                //     return EvalResult{"Divide by 0", Token{TokenKind::NUMBER, "0", 0}};

                acc /= denom;
                ++iter;
            }
        }
        return TreeNode(NodeKind::ATOM, Token{TokenKind::NUMBER, std::to_string(acc), 0});
    });

    env.set(">", [](auto& nodes) {
        if (nodes.size() < 2)
        {
        }
    });
}

template<T, std::string NAME>
TreeNode BinaryNumericOp(auto& nodes)
{
    if (nodes.size() != 2)
    {
        return "Cannot apply " + name + " with " + nodes.size() + "operands";
    }

    return
};
