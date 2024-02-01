#include "CPlusPlusParser.h"
#include "Utils.h"
#include "Options.h"

#include <clang-c/Index.h>

#include <fstream>

bool operator<(const CXFileUniqueID& lft, const CXFileUniqueID& rht)
{
	if (lft.data[0] == rht.data[0]) {
		if (lft.data[1] == rht.data[1]) {
			return lft.data[2] < rht.data[2];
		}

		return lft.data[1] < rht.data[1];
	}

	return lft.data[0] < rht.data[0];
}

bool operator>(const CXFileUniqueID& lft, const CXFileUniqueID& rht)
{
	if (lft.data[0] == rht.data[0]) {
		if (lft.data[1] == rht.data[1])
		{
			return lft.data[2] > rht.data[2];
		}

		return lft.data[1] > rht.data[1];
	}

	return lft.data[0] > rht.data[0];
}

bool operator==(const CXFileUniqueID& lft, const CXFileUniqueID& rht)
{
	return (lft < rht) == false && (lft > rht) == false;
}

bool operator!=(const CXFileUniqueID& lft, const CXFileUniqueID& rht)
{
	return !(lft == rht);
}

ElementVisitor::ElementVisitor(const std::string& name, ElementVisitor* parent
	, const std::string& accessPrefix, VisitorElementType elementType)
	: elementType_(elementType)
	, name_(name)
	, accessPrefix_(accessPrefix)
	, parent_(parent)
{
	if (nullptr != parent) {
		if (VisitorElementType::VisitorElementType_Class == elementType) {
			parent->addDeriveClass(name, this);
		} else if (VisitorElementType::VisitorElementType_Namespace == elementType) {
			parent->addNestedNamespace(name, this);
		}
	}
}

void ElementVisitor::addDeriveClass(const std::string& name, ElementVisitor* derive)
{
	auto it = deriveClasses_.find(name);
	if (deriveClasses_.end() != it) {
		return;
	}
	deriveClasses_.insert(std::make_pair(name, derive));
}

void ElementVisitor::addNestedNamespace(const std::string& name, ElementVisitor* ns)
{
	auto it = nestedNamespaces_.find(name);
	if (nestedNamespaces_.end() != it) {
		return;
	}
	nestedNamespaces_.insert(std::make_pair(name, ns));
}

bool ElementVisitor::hasNestedNamespace(const std::string& name)
{
	auto it = nestedNamespaces_.find(name);
	if (nestedNamespaces_.end() == it) {
		return false;
	}

	return true;
}

ElementVisitor* ElementVisitor::getNestedNamespace(const std::string& name)
{
	auto it = nestedNamespaces_.find(name);
	if (nestedNamespaces_.end() == it) {
		return nullptr;
	}

	return it->second;
}

std::string ElementVisitor::getAccessPrefix() const
{
	return accessPrefix_;
}

bool ElementVisitor::hasDeriveClass(const std::string& name)
{
	auto it = deriveClasses_.find(name);
	if (deriveClasses_.end() == it) {
		return false;
	}

	return true;
}

ElementVisitor* ElementVisitor::getDeriveClass(const std::string& name)
{
	auto it = deriveClasses_.find(name);
	if (deriveClasses_.end() == it) {
		return nullptr;
	}

	return it->second;
}

bool ElementVisitor::existsFunc(const std::string& name)
{
	auto it = funcs_.find(name);
	if (funcs_.end() == it) {
		return false;
	}

	return true;
}

bool ElementVisitor::existsFuncSignature(const std::string& name, const std::string& signature)
{
	auto it = funcs_.find(name);
	if (funcs_.end() == it) {
		return false;
	}

	return it->second.end() != it->second.find(signature);
}

namespace cplusplus
{
	Options opts;

	bool preProcessing;

	std::map<CXFileUniqueID, std::set<int32_t>> exportSourceLocation;

	std::set<CXFileUniqueID> finishedFiles;

	std::set<std::string> exportedClasses;

	static bool isFinishedFile(CXFileUniqueID id)
	{
		auto it = finishedFiles.find(id);
		if (finishedFiles.end() == it) {
			return false;
		}

		return true;
	}

	static bool shouldBeExported(CXFileUniqueID id, int32_t line)
	{
		auto it = exportSourceLocation.find(id);
		if (exportSourceLocation.end() == it) {
			return false;
		}
		return it->second.end() != it->second.find(line);
	}

	static std::string getFuncParamDefaultValue(CXCursor cursor, int32_t idx)
	{
		return "";
	}

	static void functionVisitor(CXCursor cursor, ElementVisitor* parentElement, bool isConstructor)
	{
		const std::string funcName = utils::getClangString(clang_getCursorSpelling(cursor));
		const std::string funcSignature = utils::getClangString(clang_getTypeSpelling(clang_getCursorType(cursor)));

		if (parentElement->existsFuncSignature(funcName, funcSignature)) {
			return;
		}

		const std::string funcRetType = utils::getClangString(clang_getTypeSpelling(clang_getCursorResultType(cursor)));
		ElementVisitor::FuncOverloadData data;
		data.isStatic = clang_CXXMethod_isStatic(cursor) != 0;;
		data.isConst = clang_CXXMethod_isConst(cursor) != 0;
		data.isConstructor = isConstructor;
		data.funcSignature = funcSignature;
		int numArgs = clang_Cursor_getNumArguments(cursor);

		for (int i = 0; i < numArgs; ++i) {
			CXCursor varCursor = clang_Cursor_getArgument(cursor, i);
			ElementVisitor::FuncParameter param;
			param.variableName = utils::getClangString(clang_getCursorSpelling(varCursor));;
			param.variableType = utils::getClangString(clang_getTypeSpelling(clang_getCursorType(varCursor)));;
			param.isConst = clang_isConstQualifiedType(clang_getCursorType(varCursor)) != 0;
			param.defaultValue = getFuncParamDefaultValue(varCursor, i);
			data.variables.push_back(param);
		}

		CXCursor* overriddenCursors;
		uint32_t  overriddenNum = 0;
		clang_getOverriddenCursors(cursor, &overriddenCursors, &overriddenNum);
		for (uint32_t i = 0; i < overriddenNum; ++i) {
			CXCursor overriddenCursor = overriddenCursors[i];
			functionVisitor(overriddenCursor, parentElement, isConstructor);
		}
		if (overriddenNum > 0) {
			clang_disposeOverriddenCursors(overriddenCursors);
		}
	}

	static CXChildVisitResult cursorVisitor(CXCursor cursor, CXCursor parent, CXClientData clientData)
	{
		ElementVisitor* parentElement = static_cast<ElementVisitor*>(clientData);
		CXCursorKind kind = clang_getCursorKind(cursor);

		if (preProcessing && clang_isPreprocessing(kind) == 0) {
			preProcessing = false;

			if (opts.getExportMacroKeyword().empty()) {
				return CXChildVisit_Continue;
			}

			if (exportSourceLocation.empty()) {
				return CXChildVisit_Break;
			}
		}

		switch (kind) {
			case CXCursor_MacroExpansion:
			{
				if (opts.getExportMacroKeyword().empty()) {
					return CXChildVisit_Continue;
				}

				const std::string macroName = utils::getClangString(clang_getCursorSpelling(cursor));

				if (opts.getExportMacroKeyword() != macroName) {
					break;
				}

				CXSourceLocation loc = clang_getCursorLocation(cursor);
				CXFile file;
				uint32_t line;
				uint32_t column;
				uint32_t offset;
				clang_getExpansionLocation(loc, &file, &line, &column, &offset);

				CXFileUniqueID id;
				clang_getFileUniqueID(file, &id);

				if (isFinishedFile(id))
				{
					break;
				}

				exportSourceLocation[id].insert(line);
			}
			break;
			case CXCursor_Namespace:
			{
				if (opts.shouldSkipNamespace()) {
					break;
				}

				CXSourceLocation loc = clang_getCursorLocation(cursor);
				if (0 != clang_Location_isInSystemHeader(loc)) {
					break;
				}

				CXFile file;
				unsigned line;
				unsigned column;
				unsigned offset;
				clang_getExpansionLocation(loc, &file, &line, &column, &offset);
				CXFileUniqueID id;
				clang_getFileUniqueID(file, &id);

				if (isFinishedFile(id))
				{
					break;
				}

				if (!shouldBeExported(id, line)) {
					break;
				}

				const std::string namespaceName = utils::getClangString(clang_getCursorSpelling(cursor));

				if (!opts.shouleExportNamespace(namespaceName)) {
					break;
				}

				if (!parentElement->hasNestedNamespace(namespaceName)) {
					ElementVisitor* newElement = new ElementVisitor(namespaceName, parentElement
						, parentElement->getAccessPrefix() + namespaceName, VisitorElementType::VisitorElementType_Namespace);
					clang_visitChildren(cursor, cursorVisitor, newElement);
				} else {
					clang_visitChildren(cursor, cursorVisitor, parentElement->getNestedNamespace(namespaceName));
				}
			}
			break;
			case CXCursor_StructDecl:
			case CXCursor_ClassDecl:
			{
				if (opts.shouldSkipClasses()) {
					break;
				}

				CXSourceLocation loc = clang_getCursorLocation(cursor);
				if (clang_Location_isInSystemHeader(loc) != 0) {
					break;
				}

				CX_CXXAccessSpecifier accessSpecifierKind = clang_getCXXAccessSpecifier(cursor);
				CXFile file;
				unsigned line;
				unsigned column;
				unsigned offset;
				clang_getExpansionLocation(loc, &file, &line, &column, &offset);
				CXFileUniqueID id;
				clang_getFileUniqueID(file, &id);

				if (isFinishedFile(id))
				{
					break;
				}

				if (!shouldBeExported(id, line)) {
					break;
				}

				const std::string className = utils::getClangString(clang_getCursorSpelling(cursor));
				const std::string categoryName = utils::getClangString(clang_getTypeSpelling(clang_getCursorType(cursor)));

				if (!parentElement->hasDeriveClass(className)) {
					ElementVisitor* newElement = new ElementVisitor(className, parentElement
						, categoryName, VisitorElementType::VisitorElementType_Class);
					exportedClasses.insert(className);
					clang_visitChildren(cursor, cursorVisitor, newElement);
				} else {

				}
			}
			break;
			case CXCursor_CXXMethod:
			{
				if (opts.excludeStaticFunc() && clang_CXXMethod_isStatic(cursor) != 0) {
					break;
				}
				CXSourceLocation loc = clang_getCursorLocation(cursor);
				if (clang_Location_isInSystemHeader(loc) != 0) {
					break;
				}
				CX_CXXAccessSpecifier accessSpecifierKind = clang_getCXXAccessSpecifier(cursor);
				CXFile file;
				unsigned line;
				unsigned column;
				unsigned offset;
				clang_getExpansionLocation(loc, &file, &line, &column, &offset);
				CXFileUniqueID id;
				clang_getFileUniqueID(file, &id);

				if (isFinishedFile(id))
				{
					break;
				}

				if (!shouldBeExported(id, line)) {
					break;
				}

				functionVisitor(cursor, parentElement, false);
			}
			break;
		}

		return CXChildVisit_Continue;
	}

	static CXChildVisitResult displayVisitor(CXCursor cursor, CXCursor parent, CXClientData clientData)
	{
		return CXChildVisit_Continue;
	}

	static void inclusionsVisitor(CXFile includedFile, CXSourceLocation* inclusionStack
		, uint32_t includeLen, CXClientData clientData)
	{
	}

	void parse()
	{
		std::vector<const char*> args{
			"-xc++",
			"-std=c++20",
			"-w",
			"-fno-spell-checking",
			"-fsyntax-only",
		};

		std::vector<std::string> includeDirs;

		for (const auto& p : opts.getIncludeDirs()) {
			args.push_back(p.c_str());
		}

		ElementVisitor visitor;
		CXIndex index = clang_createIndex(0, 0);
		const std::string tempFilename = utils::generateTempFilename();
		std::fstream tempFile(tempFilename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

		for (const auto& file : opts.getIncludeFiles()) {
			tempFile << "#include \"" << utils::getFileName(file.string()) << "\"\n";
			tempFile.flush();
		}

		uint32_t flags = CXTranslationUnit_SkipFunctionBodies | CXTranslationUnit_KeepGoing | CXTranslationUnit_Incomplete
			| CXTranslationUnit_IgnoreNonErrorsFromIncludedFiles | opts.getFlags();
		CXTranslationUnit tu = clang_parseTranslationUnit(index, tempFilename.c_str()
			, args.data(), static_cast<int>(args.size()), 0, 0, flags);
		CXCursor cursor = clang_getTranslationUnitCursor(tu);
		if (opts.display()) {
			clang_visitChildren(cursor, displayVisitor, &visitor);
		}
		else {
			preProcessing = true;
			clang_visitChildren(cursor, cursorVisitor, &visitor);
			exportSourceLocation.clear();
		}

		clang_getInclusions(tu, inclusionsVisitor, 0);
		clang_disposeTranslationUnit(tu);
		clang_disposeIndex(index);

		tempFile.close();
		utils::removeFile(tempFilename);
	}
}