#include "Utils.h"

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
}
