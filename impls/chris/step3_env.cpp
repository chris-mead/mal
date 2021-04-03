#include "evaluator.h"
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

int mainLoop(const ConfigInfo& config_info)
{
    InterpreterState state {config_info, &std::cin};
    REPLEnv env;
    while(state.moreInput())
    {
        state.printPrompt();
        std::string line = state.readLine();
        if (line.empty())
            continue;
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
            const auto eval_result = evalAST(root_node, env);
            if(eval_result.error())
            {
                std::cout << "ERROR: " << eval_result.message() << "\n";
            }
            else
            {
                printTree(std::cout, eval_result.get());
            }
            std::cout << "\n";
        }
    }
    return 0;
}

int main()
{
    ConfigInfo config_info;
    const auto result = mainLoop(config_info);
    return result;
}
