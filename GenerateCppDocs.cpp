// GenerateCppDocs.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <clang-c/Index.h>

#include "Utils.h"
#include "ArgsParser.h"
#include "FIlesystem.h"

CXChildVisitResult Visitor(CXCursor cursor, CXCursor parent, CXClientData clientData) {
    // ��ȡ�α�����ࣨkind��
    CXCursorKind cursorKind = clang_getCursorKind(cursor);

    // ��ȡ�α���ı���ʾ
    CXString cursorSpelling = clang_getCursorSpelling(cursor);

    // ��ȡ�α������кź��к�
    CXSourceLocation location = clang_getCursorLocation(cursor);
    unsigned int line, column;
    clang_getSpellingLocation(location, nullptr, &line, &column, nullptr);

    // ��ӡ�α���Ϣ
    printf("Cursor Kind: %s, Spelling: %s, Location: Line %d, Column %d\n",
        clang_getCursorKindSpelling(cursorKind),
        clang_getCString(cursorSpelling),
        line,
        column);

    // �ͷ���Դ
    clang_disposeString(cursorSpelling);

    // ���������ӽڵ�
    return CXChildVisit_Recurse;
}

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

    std::string filename;

    if (parser.has("f")) {
        filename = parser.getOption("f");
    }

    if (filename.empty()) {
        return 0;
    }

    std::vector<const char*> args{
        "-xc++",
        "-std=c++20",
        "-w",
        "-fno-spell-checking",
        "-fsyntax-only",
    };

    CXIndex index = clang_createIndex(0, 0);
    uint32_t flags = CXTranslationUnit_SkipFunctionBodies;
    CXTranslationUnit tu = clang_parseTranslationUnit(index, filename.c_str(), args.data(), args.size(), nullptr, 0, flags);
    CXCursor c = clang_getTranslationUnitCursor(tu);
    clang_visitChildren(c, Visitor, nullptr);
    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return 0;
}
