#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace filesystem {

    void findFilesInDir(const fs::path& dirPath, const std::vector<std::string>& fileExtensions
        , std::vector<fs::path>& outFiles);

    std::vector<fs::path> findFiles(const std::vector<std::string>& dirs, const std::vector<std::string>& extensions);
}