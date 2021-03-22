#include "lexer.h"
#include "parser.h"

#include <cassert>
#include <iostream>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

const std::string_view DEFAULT_PROMPT {"user> "};

class ConfigInfo
{
public:
    std::string_view prompt {DEFAULT_PROMPT};
};


class InterpreterState
{
private:
    std::istream* in;
    std::string prompt;

public:
    InterpreterState(const ConfigInfo& config_info_,
                     std::istream* in_) :
        in {in_},
        prompt {config_info_.prompt}
    {
    }

    std::string readLine()
    {
        std::string line;
        std::getline(*in, line);
        return line;
    }

    bool moreInput() const
    {
        return !in->eof();
    }

    void printPrompt()
    {
        std::cout << prompt;
    }

    void printVal(std::string val)
    {
        std::cout << "\n"
                  << val << "\n";
    }
};

void printTree(std::ostream& out, const TreeNode& node)
{
    if(node.kind == NodeKind::ROOT)
    {
        assert(!node.children.empty());
        printTree(out, node.children[0]);
    }
    else if(node.kind == NodeKind::LIST)
    {
        out << "(";
        std::string sep = "";
        for(const auto& child : node.children)
        {
            out << sep;
            printTree(out, child);
            sep = " ";
        }
        out << ")";
    }
    else if(node.kind == NodeKind::VECTOR)
    {
        out << "[";
        std::string sep = "";
        for(const auto& child : node.children)
        {
            out << sep;
            printTree(out, child);
            sep = " ";
        }
        out << "]";
    }
    else if(node.kind == NodeKind::HASHMAP)
    {
        out << "{";
        std::string sep = "";
        for(const auto& child : node.children)
        {
            out << sep;
            printTree(out, child);
            sep = " ";
        }
        out << "}";
    }
    else
    {
        out << node.token.text;
    }
}

class REPLEnv
{
public:
    TreeNode apply(std::string symbol, const std::vector<TreeNode> nodes) const
    {
        if(symbol == "+")
        {
            int acc = 0;
            for(const auto& node: nodes)
            {
                auto num = std::atoi(node.token.text.c_str());
                acc += num;
            }
            return TreeNode(NodeKind::ATOM, Token{TokenKind::NUMBER, std::to_string(acc), 0});
        }
//        else if (symbol == "-")
        // {
        //     return TreeNode(NodeKind::ATOM, Token{TokenKind::NUMBER, "0");
        // }
        else if (symbol == "*")
        {
            int acc = 1;
            for(const auto& node: nodes)
            {
                auto num = std::atoi(node.token.text.c_str());
                acc *= num;
            }
            return TreeNode(NodeKind::ATOM, Token{TokenKind::NUMBER, std::to_string(acc), 0});
        }
        // else if (symbol == "/")
        // {
        //     return TreeNode(NodeKind::ATOM, "0");
        // }
        else
        {
            return TreeNode(NodeKind::ATOM, Token{TokenKind::NUMBER, "0", 0});
        }
    }
};

TreeNode evalAST(const TreeNode& node, const REPLEnv& env)
{
    switch(node.kind)
    {
    case NodeKind::ROOT:
        return evalAST(node.children[0], env);
    case NodeKind::ATOM:
    {
        return node;
    }
    case NodeKind::LIST:
    case NodeKind::VECTOR:
    case NodeKind::HASHMAP:
        if (node.children.empty())
        {
            // Returns a _copy_
            return node;
        }
        else
        {
            const auto& func = node.children[0];
            std::vector<TreeNode> evaluated;
            for (auto it = ++std::begin(node.children); it != std::end(node.children); ++it)
            {
                TreeNode eval_child = evalAST(*it, env);
                evaluated.push_back(eval_child);
            }
            return env.apply(func.token.text, evaluated);
        }
    }
    assert(0);
    return node;
}

int mainLoop(const ConfigInfo& config_info)
{
    InterpreterState state {config_info, &std::cin};
    while(state.moreInput())
    {
        state.printPrompt();
        std::string line = state.readLine();
        Lexer lexer;

        auto tokens = lexer.tokenise(line);

        Parser parser;

        auto parse_result = parser.parse(tokens);

        
        
        if(parse_result.error())
        {
            std::cout << "ERROR: " << parse_result.message() << "\n";
        }
        else
        {
            const auto& root_node = parse_result.get();
            REPLEnv env;
            const auto result = evalAST(root_node, env);
            printTree(std::cout, result);
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    return 0;
}

int main()
{
    ConfigInfo config_info;
    const auto result = mainLoop(config_info);
    return result;
}
