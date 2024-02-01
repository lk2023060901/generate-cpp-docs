#include "Options.h"
#include "ArgumentParser.h"
#include "Utils.h"

#include <fstream>
#include <iostream>

Options::Options()
    : includeFiles_()
    , includeDirs_()
    , outputDir_()
    , exportMacroKeyword_()
    , display_(false)
    , excludeFiles_()
    , excludeFuncs_()
    , excludeClasses_()
    , excludeVariables_()
    , excludeFields_()
    , includeNamespaces_()
    , excludeNamespaces_()
    , excludeEnumerations_()
    , excludeStaticFunc_(false)
    , flags_()
    , skipNamespaces_(false)
    , skipClasses_(false)
{
}

Options::~Options()
{
}

bool Options::parse(ArgumentParser& parser)
{
    if (parser.has("include_file")) {
        std::vector<std::string> result = parser.getValues("include_file");
        for (const auto& v : result) {
            includeFiles_.push_back(v);
        }
    }

    if (parser.has("include_file_from_file")) {
        std::fstream infile(parser.get("include_file_from_file"));
        std::string file;

        for (; infile >> file;) {
            includeFiles_.push_back(file);
        }
    }

    if (parser.has("include_dirs_from_file")) {
        std::fstream infile(parser.get("include_dirs_from_file"));
        std::string file;

        for (; infile >> file;) {
            includeDirs_.push_back("-I" + file);
        }
    }

    if (parser.has("I")) {
        std::vector<std::string> result = parser.getValues("I");
        for (const auto& v : result) {
            includeDirs_.push_back("-I" + v);
        }
    }

    if (parser.has("o")) {
        outputDir_ = parser.get("o");
    }

    if (parser.has("macro_keyword")) {
        exportMacroKeyword_ = parser.get("macro_keyword");
    }

    if (parser.has("display")) {
        display_ = true;
    }

    if (parser.has("exclude_file")) {
        std::vector<std::string> result = parser.getValues("exclude_file");
        for (const auto& v : result) {
            excludeFiles_.insert(v);
        }
    }

    if (parser.has("exclude_func")) {
        std::vector<std::string> result = parser.getValues("exclude_func");
        for (const auto& v : result) {
            excludeFuncs_.insert(v);
        }
    }

    if (parser.has("exclude_class")) {
        std::vector<std::string> result = parser.getValues("exclude_class");
        for (const auto& v : result) {
            excludeClasses_.insert(v);
        }
    }

    if (parser.has("exclude_var")) {
        std::vector<std::string> result = parser.getValues("exclude_var");
        for (const auto& v : result) {
            excludeVariables_.insert(v);
        }
    }

    if (parser.has("exclude_field")) {
        std::vector<std::string> result = parser.getValues("exclude_field");
        for (const auto& v : result) {
            excludeFields_.insert(v);
        }
    }

    if (parser.has("include_ns")) {
        std::vector<std::string> result = parser.getValues("include_ns");
        for (const auto& v : result) {
            includeNamespaces_.insert(v);
        }
    }

    if (parser.has("exclude_ns")) {
        std::vector<std::string> result = parser.getValues("exclude_ns");
        for (const auto& v : result) {
            excludeNamespaces_.insert(v);
        }
    }

    if (parser.has("exclude_enum")) {
        std::vector<std::string> result = parser.getValues("exclude_enum");
        for (const auto& v : result) {
            excludeEnumerations_.insert(v);
        }
    }

    if (parser.has("exclude_static_method")) {
        excludeStaticFunc_ = true;
    }

    if (parser.has("flag")) {
        utils::Splitter<int>::splitToSet(flags_, parser.get("flag"), ' ');
    }

    if (parser.has("skip_namespace")) {
        skipNamespaces_ = true;
    }

    if (parser.has("skip_class")) {
        skipClasses_ = true;
    }

    if (!exportMacroKeyword_.empty()) {
        flags_.insert(CXTranslationUnit_DetailedPreprocessingRecord);
    }

    return true;
}

std::vector<fs::path> Options::getIncludeFiles() const
{
    return includeFiles_;
}

std::vector<std::string>& Options::getIncludeDirs()
{
    return includeDirs_;
}

fs::path Options::getOutputDir() const
{
    return outputDir_;
}

std::string Options::getExportMacroKeyword() const
{
    return exportMacroKeyword_;
}

bool Options::display() const
{
    return display_;
}

bool Options::excludeStaticFunc() const
{
    return excludeStaticFunc_;
}

int Options::getFlags() const
{
    int flags = 0;
    for (const auto flag : flags_) {
        flags |= flag;
    }

    return flags;
}

bool Options::shouleExportNamespace(const std::string& name) const
{
    if (includeNamespaces_.empty()) {
        return true;
    }

    return includeNamespaces_.end() != includeNamespaces_.find(name);
}

bool Options::shouldSkipNamespace() const
{
    return skipNamespaces_;
}

bool Options::shouldSkipClasses() const
{
    return skipClasses_;
}
