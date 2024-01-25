#include "ArgsParser.h"

#include <iostream>

ArgsParser::ArgsParser()
{
}

ArgsParser::~ArgsParser()
{
}

bool ArgsParser::parse(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg[0] == '-') {
            std::string keyWithPrefix = arg;
            std::string keyWithoutPrefix;
            std::string value;

            if (arg[1] == '-') {
                keyWithoutPrefix = arg.substr(2);
            }
            else {
                keyWithoutPrefix = arg.substr(1);
            }

            size_t pos = arg.find('=');
            if (pos != std::string::npos) {
                keyWithPrefix = arg.substr(0, pos);
                keyWithoutPrefix = arg.substr(1, pos - 1);
                value = arg.substr(pos + 1);
            }
            else if (i + 1 < argc && argv[i + 1][0] != '-') {
                value = argv[++i];
            }

            OptionParam op = { keyWithPrefix, value };
            options_[keyWithoutPrefix].push_back(op);
        }
    }

    return true;
}

bool ArgsParser::has(const std::string& param)
{
    auto it = options_.find(param);
    return it != options_.end();
}

std::string ArgsParser::getOption(const std::string& param)
{
    static std::string defaultValue;

    auto it = options_.find(param);
    if (options_.end() == it) {
        return defaultValue;
    }

    std::string str;

    for (auto optionIt = it->second.begin(); optionIt != it->second.end(); ++optionIt) {
        if (!str.empty()) {
            str += " ";
        }

        str += optionIt->key + " " + optionIt->value;
    }

    return str;
}

void ArgsParser::print()
{
    for (const auto& option : options_) {
        for (const auto& value : option.second) {
            std::cout << "Option: " << value.key << " Value: " << value.value << std::endl;
        }       
    }
}
