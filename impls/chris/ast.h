#ifndef AST_H
#define AST_H

#include "lexer.h"
#include "result.h"
#include "util.h"

#include <cassert>
#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

enum class NodeKind
{
    ROOT,
    LIST,
    VECTOR,
    HASHMAP,
    SYMBOL,
    STRING,
    NIL,
    NUMBER,
    BOOL,
    FUNC
};

class TreeNode;

using Func_t = std::function<Result<TreeNode>(std::vector<TreeNode>)>;
using Num_t = int;

// This will perform terribly... think about allocators
class TreeNode
{
public:
    // Can't just have a variant as different node kinds
    // use same C++ types!
    NodeKind kind;
    std::string as_string;

private:
    using Data_t = std::variant<std::vector<TreeNode>, // ROOT
                                std::vector<TreeNode>, // LIST
                                std::vector<TreeNode>, // VECTOR
                                std::vector<TreeNode>, // HASHMAP
                                std::string, // SYMBOL
                                std::string, // STRING
                                std::monostate, // NIL
                                Num_t, // NUMBER
                                bool, // BOOL
                                Func_t // FUNC
                                >;
    Data_t data;
    std::optional<Token> token;

    template<NodeKind KIND>
    const auto& lookup() const
    {
        return std::get<to_underlying(KIND)>(data);
    }

    template<NodeKind KIND>
    auto& lookup()
    {
        return std::get<to_underlying(KIND)>(data);
    }

public:
    TreeNode() :
        kind{NodeKind::ROOT},
        as_string{"ROOT"}
    {
    }
    // TODO - variadic forwarding thing?

    template<NodeKind KIND>
    static TreeNode makeNode(const Token& tok_)
    {
        TreeNode result{KIND, tok_};
        result.data.emplace<to_underlying(KIND)>();
        return result;
    }

    template<NodeKind KIND, typename T>
    static TreeNode makeNode(const T& val, const Token& tok_)
    {
        TreeNode result{KIND, tok_};
        result.data.emplace<to_underlying(KIND)>(val);
        return result;
    }

    template<NodeKind KIND>
    static TreeNode makeNode(std::string as_string_)
    {
        TreeNode result{KIND, as_string_};
        result.data.emplace<to_underlying(KIND)>();
        return result;
    }

    template<NodeKind KIND, typename T>
    static TreeNode makeNode(const T& val, std::string as_string_)
    {
        TreeNode result{KIND, as_string_};
        result.data.emplace<to_underlying(KIND)>(val);
        return result;
    }

private:
    TreeNode(NodeKind kind_, const Token& token_) :
        kind{kind_},
        as_string{token_.text},
        token{token_}
    {
    }

    TreeNode(NodeKind kind_, std::string as_string_) :
        kind{kind_},
        as_string{as_string_}
    {
    }

public:
    bool empty() const
    {
        return children().empty();
    }

    // TODO : std::span?
    const std::vector<TreeNode>& children() const
    {
        switch (kind)
        {
        case NodeKind::ROOT:
            return lookup<NodeKind::ROOT>();
        case NodeKind::LIST:
            return lookup<NodeKind::LIST>();
        case NodeKind::VECTOR:
            return lookup<NodeKind::VECTOR>();
        case NodeKind::HASHMAP:
            return lookup<NodeKind::HASHMAP>();
        default:
            assert(0);
            // Shut compiler up, will go bang with std::bad_variant_access
            return lookup<NodeKind::ROOT>();
        }
    }

    std::vector<TreeNode>& children()
    {
        switch (kind)
        {
        case NodeKind::ROOT:
            return lookup<NodeKind::ROOT>();
        case NodeKind::LIST:
            return lookup<NodeKind::LIST>();
        case NodeKind::VECTOR:
            return lookup<NodeKind::VECTOR>();
        case NodeKind::HASHMAP:
            return lookup<NodeKind::HASHMAP>();
        default:
            assert(0);
            // Shut compiler up, will go bang with std::bad_variant_access
            return lookup<NodeKind::ROOT>();
        }
    }

    void appendChild(const TreeNode& node)
    {
        children().push_back(node);
    }

    // void appendChild(TreeNode&& node)
    // {
    //     children().push_back(std::move(node));
    // }

    auto symbol() const
    {
        return lookup<NodeKind::SYMBOL>();
    }

    auto string() const
    {
        return lookup<NodeKind::STRING>();
    }

    auto callable() const
    {
        return lookup<NodeKind::FUNC>();
    }

    auto getBool() const
    {
        return lookup<NodeKind::BOOL>();
    }

    auto getNumber() const
    {
        return lookup<NodeKind::NUMBER>();
    }

    auto getToken() const
    {
        return token;
    }

    friend bool isSymbol(const TreeNode&);

    friend bool isFunc(const TreeNode&);

    friend bool isBool(const TreeNode&);

    friend bool isNumber(const TreeNode&);

    friend bool isNil(const TreeNode&);
};

inline bool isRoot(const TreeNode& node)
{
    return node.kind == NodeKind::ROOT;
}

inline bool isSymbol(const TreeNode& node)
{
    return node.kind == NodeKind::SYMBOL;
}

inline bool isFunc(const TreeNode& node)
{
    return node.kind == NodeKind::FUNC;
}

inline bool isBool(const TreeNode& node)
{
    return node.kind == NodeKind::BOOL;
}

inline bool isNumber(const TreeNode& node)
{
    return node.kind == NodeKind::NUMBER;
}

inline bool isNil(const TreeNode& node)
{
    return node.kind == NodeKind::NIL;
}

inline bool isString(const TreeNode& node)
{
    return node.kind == NodeKind::STRING;
}

#endif /* AST_H */
