// GenerateCppDocs.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <clang-c/Index.h>

#include "Utils.h"
#include "ArgsParser.h"
#include "FIlesystem.h"

int main(int argc, char* argv[])
{
    ArgsParser parser = utils::parseCommandLine(argc, argv);
    if (parser.has("I")) {
        std::cout << parser.getOption("I") << std::endl;
    }

    if (parser.has("include")) {
        std::cout << parser.getOption("include") << std::endl;
    }
    parser.print();

    CXIndex index = clang_createIndex(0, 0);
    uint32_t flags = CXTranslationUnit_SkipFunctionBodies;
    CXTranslationUnit tu;

    return 0;
}
