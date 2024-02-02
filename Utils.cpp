#include "Utils.h"

#include <sstream>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <nlohmann/json.hpp>

namespace utils
{
	void findFilesWithExtensions(std::vector<fs::path>& foundFiles, const std::vector<std::string>& directories
		, const std::vector<std::string>& extensions)
	{
        for (const auto& dir : directories) {
            if (!fs::exists(dir) || !fs::is_directory(dir)) {
                continue; // 如果目录不存在或不是一个目录，则跳过
            }

            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file()) {
                    for (const auto& ext : extensions) {
                        if (entry.path().extension() == ext) {
                            foundFiles.push_back(entry.path()); // 将匹配的文件路径添加到外带参数中
                            break; // 匹配到后，不再检查其他后缀
                        }
                    }
                }
            }
        }
	}

    std::string getFileName(const std::string& path)
    {
        std::string::size_type pos = path.find_last_of("\\/");
        std::string name = path;

        if (pos != std::string::npos)
        {
            name = path.substr(pos + 1);
        }

        return name;
    }

    std::string getClangString(CXString str)
    {
        const char* data = clang_getCString(str);
        std::string result;

        if (nullptr != data)
        {
            result.assign(data);
        }

        clang_disposeString(str);
        return result;
    }

    std::string generateTempFilename()
    {
        std::filesystem::path tempDir = std::filesystem::temp_directory_path();
        // 生成一个随机的文件名
        std::srand(static_cast<unsigned int>(std::time(nullptr))); // 使用当前时间作为随机数种子
        std::string filename = "tempfile_" + std::to_string(std::rand()) + ".cpp";
        // 拼接成完整的临时文件路径
        std::filesystem::path tempFile = tempDir / filename;

        return tempFile.string();
    }

    void removeFile(const fs::path& path)
    {
        std::filesystem::remove(path);
    }

    std::string generateCodeComments(const std::string& funcDecl)
    {
        try {
            // 初始化 curlpp
            curlpp::Cleanup cleaner;
            curlpp::Easy request;

            // 设置请求 URL
            request.setOpt<curlpp::options::Url>("http://47.253.97.14:8380/chat/completions");

            // 设置 HTTP 头部，表明提交的是 JSON 数据
            std::list<std::string> header;
            header.push_back("Content-Type: application/json");
            header.push_back("Authorization: a1a8668cc1db975a10fb0585e43073e5");
            //header.push_back("model: Azure_GPT4");
            request.setOpt<curlpp::options::HttpHeader>(header);

            std::u8string u8Data = u8"{\"messages\": [{\"role\": \"user\",\"content\": \"";
            u8Data += u8"给这个函数生成 Doxygen 风格的中文代码注释。" + std::u8string(funcDecl.begin(), funcDecl.end());
            u8Data += u8"\"}]}";

            std::string jsonData(u8Data.begin(), u8Data.end());

            // 设置 POST 数据
            request.setOpt<curlpp::options::PostFields>(jsonData);
            request.setOpt<curlpp::options::PostFieldSize>(jsonData.length());

            // 执行请求并捕获响应
            std::ostringstream response;
            request.setOpt<curlpp::options::WriteStream>(&response);

            request.perform();

            auto json = nlohmann::json::parse(response.str());
            std::string result;

            // 检查choices是否存在并且是一个数组
            if (json.contains("choices") && json["choices"].is_array()) {
                // 遍历choices数组
                for (const auto& choice : json["choices"]) {
                    // 检查message是否存在并且包含content字段
                    if (choice.contains("message") && choice["message"].is_object()) {
                        auto message = choice["message"];
                        if (message.contains("content") && message["content"].is_string()) {
                            result += message["content"];
                        }
                    }
                }
            }

            return result;
        }
        catch (curlpp::RuntimeError& e) {
            std::cerr << e.what() << std::endl;
        }
        catch (curlpp::LogicError& e) {
            std::cerr << e.what() << std::endl;
        }

        return std::string();
    }
}
