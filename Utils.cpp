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
                continue; // ���Ŀ¼�����ڻ���һ��Ŀ¼��������
            }

            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file()) {
                    for (const auto& ext : extensions) {
                        if (entry.path().extension() == ext) {
                            foundFiles.push_back(entry.path()); // ��ƥ����ļ�·����ӵ����������
                            break; // ƥ�䵽�󣬲��ټ��������׺
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
        // ����һ��������ļ���
        std::srand(static_cast<unsigned int>(std::time(nullptr))); // ʹ�õ�ǰʱ����Ϊ���������
        std::string filename = "tempfile_" + std::to_string(std::rand()) + ".cpp";
        // ƴ�ӳ���������ʱ�ļ�·��
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
            // ��ʼ�� curlpp
            curlpp::Cleanup cleaner;
            curlpp::Easy request;

            // �������� URL
            request.setOpt<curlpp::options::Url>("http://47.253.97.14:8380/chat/completions");

            // ���� HTTP ͷ���������ύ���� JSON ����
            std::list<std::string> header;
            header.push_back("Content-Type: application/json");
            header.push_back("Authorization: a1a8668cc1db975a10fb0585e43073e5");
            //header.push_back("model: Azure_GPT4");
            request.setOpt<curlpp::options::HttpHeader>(header);

            std::u8string u8Data = u8"{\"messages\": [{\"role\": \"user\",\"content\": \"";
            u8Data += u8"������������� Doxygen �������Ĵ���ע�͡�" + std::u8string(funcDecl.begin(), funcDecl.end());
            u8Data += u8"\"}]}";

            std::string jsonData(u8Data.begin(), u8Data.end());

            // ���� POST ����
            request.setOpt<curlpp::options::PostFields>(jsonData);
            request.setOpt<curlpp::options::PostFieldSize>(jsonData.length());

            // ִ�����󲢲�����Ӧ
            std::ostringstream response;
            request.setOpt<curlpp::options::WriteStream>(&response);

            request.perform();

            auto json = nlohmann::json::parse(response.str());
            std::string result;

            // ���choices�Ƿ���ڲ�����һ������
            if (json.contains("choices") && json["choices"].is_array()) {
                // ����choices����
                for (const auto& choice : json["choices"]) {
                    // ���message�Ƿ���ڲ��Ұ���content�ֶ�
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
