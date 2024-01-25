#pragma once

#include <string>
#include <map>
#include <vector>

struct OptionParam
{
    std::string key;
    std::string value;
};

class ArgsParser
{
public:

    ArgsParser();

    ~ArgsParser();

public:

    bool parse(int argc, char* argv[]);

    bool has(const std::string& param);

    std::string getOption(const std::string& param);

public:

    void print();

private:

    std::map<std::string, std::vector<OptionParam>> options_;
};