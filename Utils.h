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
                // ��Ӵ�start��end���Ӵ����������
                result.emplace_back(str.substr(start, end - start));
                start = end + 1; // �ƶ�����һ���ַ��������ָ���
                end = str.find(delimiter, start); // ��ʣ����ַ����в�����һ���ָ���
            }

            // ������һ���ָ��Ĳ��֣�����У�
            result.emplace_back(str.substr(start));
        }

        static void splitToSet(std::set<T>& result, const std::string& str, char delimiter)
        {
            size_t start = 0;
            size_t end = str.find(delimiter);

            while (end != std::string_view::npos) {
                // ��Ӵ�start��end���Ӵ����������
                result.emplace_back(str.substr(start, end - start));
                start = end + 1; // �ƶ�����һ���ַ��������ָ���
                end = str.find(delimiter, start); // ��ʣ����ַ����в�����һ���ָ���
            }

            // ������һ���ָ��Ĳ��֣�����У�
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
                // ��Ӵ�start��end���Ӵ����������
                result.emplace_back(std::stoi(str.substr(start, end - start)));
                start = end + 1; // �ƶ�����һ���ַ��������ָ���
                end = str.find(delimiter, start); // ��ʣ����ַ����в�����һ���ָ���
            }

            // ������һ���ָ��Ĳ��֣�����У�
            result.emplace_back(std::stoi(str.substr(start)));
        }

        static void splitToSet(std::set<int>& result, const std::string& str, char delimiter)
        {
            size_t start = 0;
            size_t end = str.find(delimiter);

            while (end != std::string_view::npos) {
                // ��Ӵ�start��end���Ӵ����������
                result.insert(std::stoi(str.substr(start, end - start)));
                start = end + 1; // �ƶ�����һ���ַ��������ָ���
                end = str.find(delimiter, start); // ��ʣ����ַ����в�����һ���ָ���
            }

            // ������һ���ָ��Ĳ��֣�����У�
            result.insert(std::stoi(str.substr(start)));
        }
    };

    std::string getClangString(CXString str);

    std::string generateTempFilename();

    void removeFile(const fs::path& path);

    std::string generateCodeComments(const std::string& funcDecl);
}