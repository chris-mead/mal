#include <iostream>
#include <string>

const std::string_view DEFAULT_PROMPT{"user> "};

class ConfigInfo
{
public:
    std::string_view prompt{DEFAULT_PROMPT};
};

int mainLoop(const ConfigInfo& config_info)
{
    bool done = false;
    std::string line;
    while (!done)
    {
        std::cout << config_info.prompt;
        std::getline(std::cin, line);
        if (std::cin.eof())
            break;

        std::cout << "\n"
                  << line << "\n";
    }
    return 0;
}

int main()
{
    ConfigInfo config_info;
    const auto result = mainLoop(config_info);
    return result;
}
