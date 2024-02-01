#include "Utils.h"

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
}
