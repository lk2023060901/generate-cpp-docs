#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <filesystem>

class ArgumentParser;

namespace fs = std::filesystem;

class Options
{
public:

	Options();

	~Options();

public:

	bool parse(ArgumentParser& parser);

	std::vector<fs::path> getIncludeFiles() const;

	std::vector<std::string>& getIncludeDirs();

	fs::path getOutputDir() const;

	std::string getExportMacroKeyword() const;

	bool display() const;

	bool excludeStaticFunc() const;

	int getFlags() const;

	bool shouleExportNamespace(const std::string& name) const;

	bool shouldSkipNamespace() const;

	bool shouldSkipClasses() const;

private:

	std::vector<fs::path> includeFiles_;

	std::vector<std::string> includeDirs_;

	fs::path outputDir_;

	std::string exportMacroKeyword_;

	bool display_;

	std::set<fs::path> excludeFiles_;

	std::set<std::string> excludeFuncs_;

	std::set<std::string> excludeClasses_;

	std::set<std::string> excludeVariables_;

	std::set<std::string> excludeFields_;

	std::set<std::string> includeNamespaces_;

	std::set<std::string> excludeNamespaces_;

	std::set<std::string> excludeEnumerations_;

	bool excludeStaticFunc_;

	std::set<int> flags_;

	bool skipNamespaces_;

	bool skipClasses_;
};

