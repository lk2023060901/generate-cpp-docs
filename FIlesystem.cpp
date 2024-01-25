#include "FIlesystem.h"

namespace filesystem {

	void findFilesInDir(const fs::path& dirPath, const std::vector<std::string>& fileExtensions, std::vector<fs::path>& outFiles)
	{
        if (fs::exists(dirPath) && fs::is_directory(dirPath)) {
            for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
                if (fs::is_regular_file(entry)) {
                    auto ext = entry.path().extension().string();
                    if (std::find(fileExtensions.begin(), fileExtensions.end(), ext) != fileExtensions.end()) {
                        outFiles.push_back(entry.path());
                    }
                }
            }
        }
	}

	std::vector<fs::path> findFiles(const std::vector<std::string>& dirs, const std::vector<std::string>& extensions)
	{
        std::vector<fs::path> result;
        for (const auto& dir : dirs) {
            findFilesInDir(dir, extensions, result);
        }
        return result;
	}
}