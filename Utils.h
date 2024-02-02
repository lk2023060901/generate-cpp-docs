#pragma once

#include <filesystem>
#include <vector>
#include <set>
#include <string>
#include <string_view>

#include <clang-c/Index.h>

namespace fs = std::filesystem;

namespace utils
{
    void findFilesWithExtensions(std::vector<fs::path>& foundFiles, const std::vector<std::string>& directories
        , const std::vector<std::string>& extensions);

    std::string getFileName(const std::string& path);

    template<typename T>
    struct Splitter
    {
        static void splitToVector(std::vector<T>& result, const std::string& str, char delimiter)
        {
            size_t start = 0;
            size_t end = str.find(delimiter);

            while (end != std::string_view::npos) {
                // 添加从start到end的子串到结果向量
                result.emplace_back(str.substr(start, end - start));
                start = end + 1; // 移动到下一个字符，跳过分隔符
                end = str.find(delimiter, start); // 在剩余的字符串中查找下一个分隔符
            }

            // 添加最后一个分割后的部分（如果有）
            result.emplace_back(str.substr(start));
        }

        static void splitToSet(std::set<T>& result, const std::string& str, char delimiter)
        {
            size_t start = 0;
            size_t end = str.find(delimiter);

            while (end != std::string_view::npos) {
                // 添加从start到end的子串到结果向量
                result.emplace_back(str.substr(start, end - start));
                start = end + 1; // 移动到下一个字符，跳过分隔符
                end = str.find(delimiter, start); // 在剩余的字符串中查找下一个分隔符
            }

            // 添加最后一个分割后的部分（如果有）
            result.emplace_back(str.substr(start));
        }
    };

    template<>
    struct Splitter<int> {
        static void splitToVector(std::vector<int>& result, const std::string& str, char delimiter)
        {
            size_t start = 0;
            size_t end = str.find(delimiter);

            while (end != std::string_view::npos) {
                // 添加从start到end的子串到结果向量
                result.emplace_back(std::stoi(str.substr(start, end - start)));
                start = end + 1; // 移动到下一个字符，跳过分隔符
                end = str.find(delimiter, start); // 在剩余的字符串中查找下一个分隔符
            }

            // 添加最后一个分割后的部分（如果有）
            result.emplace_back(std::stoi(str.substr(start)));
        }

        static void splitToSet(std::set<int>& result, const std::string& str, char delimiter)
        {
            size_t start = 0;
            size_t end = str.find(delimiter);

            while (end != std::string_view::npos) {
                // 添加从start到end的子串到结果向量
                result.insert(std::stoi(str.substr(start, end - start)));
                start = end + 1; // 移动到下一个字符，跳过分隔符
                end = str.find(delimiter, start); // 在剩余的字符串中查找下一个分隔符
            }

            // 添加最后一个分割后的部分（如果有）
            result.insert(std::stoi(str.substr(start)));
        }
    };

    std::string getClangString(CXString str);

    std::string generateTempFilename();

    void removeFile(const fs::path& path);

    std::string generateCodeComments(const std::string& funcDecl);
}