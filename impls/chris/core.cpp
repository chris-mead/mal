
#include "core.h"
#include "ast.h"
#include "env.h"
#include "evaluator.h"

void addCoreFunsToEnv(Environment& env)
{
    Func_t add_impl = [](const auto& nodes) -> EvalResult {
        int acc = 0;
        for (const auto& node : nodes)
        {
            if (!isNumber(node))
                return Result<TreeNode>("'" + node.as_string + "' not a number");
            acc += node.getNumber();
        }
        return TreeNode::makeNode<NodeKind::NUMBER>(acc, "NUM");
    };
    env.set("+", TreeNode::makeNode<NodeKind::FUNC>(add_impl, "#FUNC+"));

    Func_t sub_impl = [](const auto& nodes) -> EvalResult {
        int acc;
        if (nodes.empty())
        {
            acc = 0;
        }
        else
        {
            auto iter = std::begin(nodes);
            if (!isNumber(*iter))
                return Result<TreeNode>("'" + iter->as_string + "' not a number");
            acc = iter->getNumber();
            ++iter;
            while (iter != std::end(nodes))
            {
                if (!isNumber(*iter))
                    return Result<TreeNode>("'" + iter->as_string + "' not a number");
                acc -= iter->getNumber();
                ++iter;
            }
        }
        return TreeNode::makeNode<NodeKind::NUMBER>(acc, std::to_string(acc));
    };
    env.set("-", TreeNode::makeNode<NodeKind::FUNC>(sub_impl, "#FUNC-"));

    Func_t mul_impl = [](const auto& nodes) -> EvalResult {
        int acc = 1;
        for (const auto& node : nodes)
        {
            if (!isNumber(node))
                return Result<TreeNode>("'" + node.as_string + "' not a number");
            acc *= node.getNumber();
        }
        return TreeNode::makeNode<NodeKind::NUMBER>(acc, std::to_string(acc));
    };
    env.set("*", TreeNode::makeNode<NodeKind::FUNC>(mul_impl, "#FUNC*"));

    Func_t div_impl = [](const auto& nodes) -> EvalResult {
        int acc;
        if (nodes.size() < 2)
        {
            acc = 0;
        }
        else
        {
            auto iter = std::begin(nodes);
            if (!isNumber(*iter))
                return Result<TreeNode>("'" + iter->as_string + "' not a number");
            acc = iter->getNumber();
            ++iter;
            while (iter != std::end(nodes))
            {
                if (!isNumber(*iter))
                    return Result<TreeNode>("'" + iter->as_string + "' not a number");

                int denom = iter->getNumber();

                if (denom == 0)
                    return EvalResult{"Division by 0"};
                acc /= denom;
                ++iter;
            }
        }
        return TreeNode::makeNode<NodeKind::NUMBER>(acc, std::to_string(acc));
    };
    env.set("/", TreeNode::makeNode<NodeKind::FUNC>(div_impl, "#FUNC/"));

    // env.set(">", [](auto& nodes) {
    //     if (nodes.size() < 2)
    //     {
    //     }
    //     });
}

// template<T, std::string NAME>
// TreeNode BinaryNumericOp(auto& nodes)
// {
//     if (nodes.size() != 2)
//     {
//         return "Cannot apply " + name + " with " + nodes.size() + "operands";
//     }

//     return
// };
