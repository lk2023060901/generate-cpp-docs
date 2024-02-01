#ifndef GET_OPT_H
#define GET_OPT_H

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <io.h>
#include <fmt/printf.h>

namespace getopt_utils
{
    inline size_t split(std::vector<std::string>& tokens, const std::string& self, const std::string& delimiters)
    {
        std::string str;
        tokens.clear();
        for(auto& ch: self)
        {
            if(delimiters.find_first_of(ch) != std::string::npos)
            {
                if(str.size())
                    tokens.push_back(str), str = "";
                tokens.push_back(std::string() + ch);
            }
            else
                str += ch;
        }
        return str.empty() ? tokens.size() : (tokens.push_back(str), tokens.size());
    };

    // portable cmdline

} // namespace getopt_utils

// main map class; explicit initialization

struct get_opt final : public std::map<std::string, std::string>
{
    using super = std::map<std::string, std::string>;

    get_opt(int argc, const char** argv)
        : super()
    {
        // reconstruct vector
        std::vector<std::string> args(argc, std::string());
        for(int32_t i = 0; i < argc; ++i)
        {
            args[i] = argv[i];
        }
        // create key=value and key= args as well
        for(auto& it: args)
        {
            std::vector<std::string> tokens;
            auto                     size = getopt_utils::split(tokens, it, "=");

            if(size == 3 && tokens[1] == "=")
                (*this)[tokens[0]] = tokens[2];
            else if(size == 2 && tokens[1] == "=")
                (*this)[tokens[0]] = "";
            else if(size == 1 && tokens[0] != argv[0])
                (*this)[tokens[0]] = "";
        }
        // recreate args
        while(argc--)
        {
            (*this)[std::to_string(argc)] = std::string(argv[argc]);
        }
    }

    get_opt(const std::vector<std::string>& args)
        : super()
    {
        std::vector<const char*> argv;
        for(auto& it: args)
        {
            argv.push_back(it.c_str());
        }
        *this = get_opt(argv.size(), argv.data());
    }

    size_t size() const
    {
        unsigned i = 0;
        while(has(std::to_string(i)))
            ++i;
        return i;
    }

    bool has(const std::string& op) const { return this->find(op) != this->end(); }

    std::string str() const
    {
        std::stringstream ss;
        std::string       sep;
        for(auto& it: *this)
        {
            ss << sep << it.first << "=" << it.second;
            sep = ',';
        }
        return ss.str();
    }

    std::string cmdline() const
    {
        std::stringstream cmd;
        std::string       sep;
        // concatenate args
        for(auto end = size(), arg = end - end; arg < end; ++arg)
        {
            cmd << sep << this->find(std::to_string(arg))->second;
            sep = ' ';
        }
        return cmd.str();
    }
};

#endif /* GET_OPT_H */
