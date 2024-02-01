#include "ArgumentParser.h"

#include <iostream>

void ArgumentParser::addArgument(const std::string& name, const std::string& shortName, bool required, const std::string& defaultValue)
{
    Argument arg;
    arg.name = name;
    arg.shortName = shortName;
    arg.required = required;
    if (!defaultValue.empty()) {
        arg.values.push_back(defaultValue); // 添加默认值到列表中
    }

    if (!shortName.empty()) {
        longOptions_[name] = shortName;
    }

    if (!name.empty()) {
        shortOptions_[shortName] = name;
    }

    if (!name.empty()) {
        arguments_[name] = arg;
    }

    if (!shortName.empty()) {
        arguments_[shortName] = arg;
    }  
}

void ArgumentParser::parseArgs(int argc, char* argv[])
{
    std::vector<std::string> args(argv + 1, argv + argc); // Skip program name

    for (size_t i = 0; i < args.size(); ++i) {
        std::string key, shortKey, value;
        if (args[i].substr(0, 2) == "--") { // Long option with '--'
            size_t equalPos = args[i].find('=');
            if (equalPos != std::string::npos) {
                key = args[i].substr(2, equalPos - 2); // Get the key part
                value = args[i].substr(equalPos + 1); // Get the value part
            }
            else {
                key = args[i].substr(2);
                if (i + 1 < args.size() && args[i + 1].front() != '-') {
                    value = args[++i]; // Next argument is value
                }
            }
            if (longOptions_.count(key) > 0) {
                shortKey = longOptions_[key];
            }
        }
        else if (args[i].front() == '-') { // Short option with '-'
            shortKey = args[i].substr(1);
            if (shortOptions_.count(shortKey)) {
                key = shortOptions_[shortKey]; // Convert to long option name
            }
            if (i + 1 < args.size() && args[i + 1].front() != '-') {
                value = args[++i]; // Next argument is value
            }
        }

        if (arguments_.find(key) == arguments_.end() && arguments_.find(shortKey) == arguments_.end()) {
            std::cout << "Unknown option: " << "key: " << key << ", " << "ShortKey: " << shortKey << std::endl;
            return;
        }

        if (!key.empty() || !shortKey.empty()) {           
            if (arguments_.find(key) != arguments_.end()) {
                if (commandLine_.find(key) == commandLine_.end()) {
                    commandLine_[key] = arguments_[key];
                }
                
                commandLine_[key].values.push_back(value.empty() ? "" : value);
            }

            if (arguments_.find(shortKey) != arguments_.end()) {
                if (commandLine_.find(shortKey) == commandLine_.end()) {
                    commandLine_[shortKey] = arguments_[shortKey];
                }
                commandLine_[shortKey].values.push_back(value.empty() ? "" : value);
            }
        }
        else {
            positionalArguments_.push_back(args[i]); // Treat as positional argument
        }
    }

    // Check for required arguments
    for (const auto& arg : commandLine_) {
        if (arg.second.required && arg.second.values.empty()) {
            std::cout << "Missing required argument: " << arg.first << std::endl;
            return;
        }
    }
}

bool ArgumentParser::has(const std::string argName)
{
    return commandLine_.count(argName) > 0;
}

std::string ArgumentParser::get(const std::string& argName)
{
    if (commandLine_.count(argName) > 0) {
        std::string str;

        for (const auto& v : commandLine_[argName].values) {
            if (!str.empty()) {
                str += " ";
            }
            str += v;
        }
        return str;
    }

	return std::string();
}

std::vector<std::string> ArgumentParser::getValues(const std::string& argName)
{
    if (commandLine_.count(argName) > 0) {
        return commandLine_[argName].values;
    }

    return std::vector<std::string>();
}
