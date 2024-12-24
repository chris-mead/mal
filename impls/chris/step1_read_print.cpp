#include "lexer.h"
#include "parser.h"
#include "print.h"

#include <cassert>
#include <iostream>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

const std::string_view DEFAULT_PROMPT{"user> "};

class ConfigInfo
{
public:
    std::string_view prompt{DEFAULT_PROMPT};
};

class InterpreterState
{
private:
    std::istream* in;
    std::string prompt;

public:
    InterpreterState(const ConfigInfo& config_info_,
                     std::istream* in_) :
        in{in_},
        prompt{config_info_.prompt}
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

int mainLoop(const ConfigInfo& config_info)
{
    InterpreterState state{config_info, &std::cin};
    while (state.moreInput())
    {
        state.printPrompt();
        std::string line = state.readLine();
        Lexer lexer;

        auto tokens = lexer.tokenise(line);

        Parser parser;

        auto parse_result = parser.parse(tokens);

        if (parse_result.error())
        {
            std::cout << "ERROR: " << parse_result.message() << "\n";
        }
        else
        {
            const auto& root_node = parse_result.get();
            printTree(std::cout, root_node);
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
