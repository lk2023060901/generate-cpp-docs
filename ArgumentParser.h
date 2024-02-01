#pragma once

#include <string>
#include <map>
#include <vector>

class ArgumentParser
{
public:

    void addArgument(const std::string& name, const std::string& shortName = "", bool required = false
        , const std::string& defaultValue = "");

    void parseArgs(int argc, char* argv[]);

    bool has(const std::string argName);

    std::string get(const std::string& argName);

    std::vector<std::string> getValues(const std::string& argName);

private:
    struct Argument {
        std::string name;
        std::string shortName;
        bool required;
        std::vector<std::string> values; // 使用 vector 来存储多个值
    };

    std::map<std::string, Argument> arguments_; // Long option name to Argument
    std::map<std::string, Argument> commandLine_;
    std::map<std::string, std::string> longOptions_;
    std::map<std::string, std::string> shortOptions_; // Short option name to long option name
    std::vector<std::string> positionalArguments_;
};

