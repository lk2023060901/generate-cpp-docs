#include "CPlusPlusParser.h"
#include "Utils.h"
#include "Options.h"

#include <clang-c/Index.h>

#include <fstream>
#include <iostream>

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

std::string ElementVisitor::getName() const
{
	return name_;
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

void ElementVisitor::addMemberVariable(const std::string& varName, const ElementVisitor::MemberVariableData& data)
{
	auto it = memberVariables_.find(varName);
	if (memberVariables_.end() != it) {
		return;
	}
	memberVariables_.insert(std::make_pair(varName, data));
}

void ElementVisitor::addBaseClassName(const std::string& name, CX_CXXAccessSpecifier kind)
{
	auto it = inheritClasses_.find(name);
	if (inheritClasses_.end() == it) {
		return;
	}
	inheritClasses_.insert(std::make_pair(name, kind));
}

void ElementVisitor::addEnumerationName(const std::string& name)
{
	auto it = enumerations_.find(name);
	if (enumerations_.end() != it) {
		return;
	}
	enumerations_.insert(name);
}

void ElementVisitor::addAliasName(const std::string& name)
{
	aliasNameList_.push_back(name);
}

void ElementVisitor::addFunction(const std::string& funcName, const ElementVisitor::FuncOverloadData& data)
{
	initFuncContainer(funcName);
	auto it = funcs_.find(funcName);
	if (funcs_.end() == it) {
		return;
	}
	auto signatureIt = it->second.find(data.funcSignature);
	if (it->second.end() != signatureIt) {
		return;
	}
	it->second.insert(std::make_pair(data.funcSignature, data));
}

void ElementVisitor::generateDocs()
{
	for (auto it = deriveClasses_.begin(); it != deriveClasses_.end(); ++it) {
		ElementVisitor* element = it->second;
		element->generateClassDocs();
	}
}

void ElementVisitor::initFuncContainer(const std::string& funcName)
{
	auto it = funcs_.find(funcName);
	if (funcs_.end() != it) {
		return;
	}
	funcs_.insert(std::make_pair(funcName, std::map<std::string, FuncOverloadData>()));
}

void ElementVisitor::generateClassDocs()
{
	std::stringstream ss;
	ss << "class " << getName() << "\n{\n\n";

	std::cout << "class: " << getName() << " func: " << funcs_.size() << std::endl;	
	std::map<std::string, std::map<std::string, FuncOverloadData>> privateFuncs;
	std::map<std::string, std::map<std::string, FuncOverloadData>> protectedFuncs;
	std::map<std::string, std::map<std::string, FuncOverloadData>> publicFuncs;

	getFuncsByAccessSpecifierKind(privateFuncs, CX_CXXPrivate);
	getFuncsByAccessSpecifierKind(protectedFuncs, CX_CXXProtected);
	getFuncsByAccessSpecifierKind(publicFuncs, CX_CXXPublic);

	if (!publicFuncs.empty()) {
		ss << "public:\n\n";
		writeFuncs(ss, publicFuncs);
	}

	if (!publicFuncs.empty()) {
		ss << "\n";
	}

	if (!protectedFuncs.empty()) {
		ss << "protected:\n\n";
		writeFuncs(ss, protectedFuncs);
	}

	if (!protectedFuncs.empty()) {
		ss << "\n";
	}

	if (!privateFuncs.empty()) {
		ss << "private:\n\n";
		writeFuncs(ss, privateFuncs);
	}

	if (!privateFuncs.empty()) {
		ss << "\n";
	}

	ss << "};\n";
	fs::path p;
	p.append(cplusplus::opts.getOutputDir().c_str()).append(std::string(getName() + ".h").c_str());
	const std::string path = p.string();

	std::fstream file(path, std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
	file.write(ss.str().c_str(), ss.str().size());
	file.close();
}

void ElementVisitor::getFuncsByAccessSpecifierKind(std::map<std::string, std::map<std::string, FuncOverloadData>>& funcs
	, CX_CXXAccessSpecifier kind)
{
	for (auto it = funcs_.begin(); it != funcs_.end(); ++it) {
		for (auto signatureIt = it->second.begin(); signatureIt != it->second.end()
			; ++signatureIt) {
			if (signatureIt->second.accessSpecifierKind == kind) {
				funcs[it->first].insert(std::make_pair(signatureIt->first, signatureIt->second));
			}
		}
	}
}

void ElementVisitor::getMemberVariablesByAccessSpecifierKind(std::map<std::string, MemberVariableData>& memberVariables
	, CX_CXXAccessSpecifier kind)
{
	for (auto it = memberVariables_.begin(); it != memberVariables_.end(); ++it) {
		if (it->second.accessSpecifierKind == kind) {
			memberVariables_.insert(std::make_pair(it->first, it->second));
		}
	}
}

void ElementVisitor::writeFuncs(std::stringstream& ss, std::map<std::string, std::map<std::string, FuncOverloadData>> funcs)
{
	for (auto it = funcs.begin(); it != funcs.end(); ++it) {
		if (funcs.begin() != it) {
			ss << "\n";
		}
		for (auto signatureIt = it->second.begin(); signatureIt != it->second.end(); ++signatureIt) {
			if (it->second.begin() != signatureIt) {
				ss << "\n";
			}
			const std::string comments = utils::generateCodeComments(signatureIt->second.funcDecl);
			ss << comments << "\n";
		}
	}
}

namespace cplusplus
{
	Options opts;

	bool preProcessing;

	std::map<CXFileUniqueID, std::set<int32_t>> exportSourceLocation;

	std::set<CXFileUniqueID> finishedFiles;

	std::set<std::string> exportedClasses;

	CXTranslationUnit tu;

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

		std::stringstream ss;

		const std::string funcRetType = utils::getClangString(clang_getTypeSpelling(clang_getCursorResultType(cursor)));
		ElementVisitor::FuncOverloadData data;
		data.isStatic = clang_CXXMethod_isStatic(cursor) != 0;;
		data.isConst = clang_CXXMethod_isConst(cursor) != 0;
		data.isConstructor = isConstructor;
		data.funcSignature = funcSignature;
		data.accessSpecifierKind = clang_getCXXAccessSpecifier(cursor);
		data.comments = utils::getClangString(clang_Cursor_getRawCommentText(cursor));
		int numArgs = clang_Cursor_getNumArguments(cursor);

		if (data.isStatic) {
			ss << "static";
		}

		if (!ss.str().empty()) {
			ss << " ";
		}

		ss << funcRetType << " " << funcName << "(";

		for (int i = 0; i < numArgs; ++i) {
			CXCursor varCursor = clang_Cursor_getArgument(cursor, i);
			ElementVisitor::FuncParameter param;
			param.variableName = utils::getClangString(clang_getCursorSpelling(varCursor));;
			param.variableType = utils::getClangString(clang_getTypeSpelling(clang_getCursorType(varCursor)));;
			param.isConst = clang_isConstQualifiedType(clang_getCursorType(varCursor)) != 0;
			param.defaultValue = getFuncParamDefaultValue(varCursor, i);
			data.variables.push_back(param);

			if (0 != i) {
				ss << ", ";
			}

			if (param.isConst) {
				ss << "const ";
			}
			ss << param.variableType << " " << param.variableName;
		}

		ss << ")";

		if (!data.isStatic && data.isConst) {
			ss << " const";
		}

		ss << ";";

		data.funcDecl = ss.str();
		parentElement->addFunction(funcName, data);

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

	static CXChildVisitResult enumerationVisitor(CXCursor cursor, CXCursor parent, CXClientData clientData)
	{
		ElementVisitor* parentElement = static_cast<ElementVisitor*>(clientData);
		CXCursorKind kind = clang_getCursorKind(cursor);
		switch (kind) {
			case CXCursor_EnumDecl:
			case CXCursor_EnumConstantDecl:
			{
				const std::string name = utils::getClangString(clang_getCursorSpelling(cursor));
				parentElement->addEnumerationName(name);
			}
			break;
		}

		return CXChildVisit_Continue;
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
			case CXCursor_Constructor:
			{
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

				functionVisitor(cursor, parentElement, true);
			}
			break;
			case CXCursor_FieldDecl:
			{
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

				std::string varName = utils::getClangString(clang_getCursorSpelling(cursor));
				ElementVisitor::MemberVariableData data;
				data.isStatic = false;
				data.isConst = clang_isConstQualifiedType(clang_getCursorType(cursor)) != 0;
				parentElement->addMemberVariable(varName, data);
			}
			break;
			case CXCursor_CXXBaseSpecifier:
			{
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

				const std::string baseClassName = utils::getClangString(clang_getTypeSpelling(clang_getCursorType(cursor)));
				parentElement->addBaseClassName(baseClassName, accessSpecifierKind);
			}
			break;
			case CXCursor_EnumDecl:
			case CXCursor_EnumConstantDecl:
			{
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

				bool isEnumerationClass = false;
				CXSourceRange range = clang_getCursorExtent(cursor);
				uint32_t num = 0;
				CXToken* tokens = nullptr;
				clang_tokenize(tu, range, &tokens, &num);

				for (uint32_t i = 0; i < num; ++i) {
					CXToken token = tokens[i];
					if (clang_getTokenKind(token) == CXToken_Keyword) {
						if ("class" == utils::getClangString(clang_getTokenSpelling(tu, token))) {
							isEnumerationClass = true;
							break;
						}
					}
				}
				if (isEnumerationClass) {
					const std::string name = utils::getClangString(clang_getCursorSpelling(cursor));
					ElementVisitor* newElement = new ElementVisitor(name, parentElement
						, parentElement->getAccessPrefix() + name, VisitorElementType::VisitorElementType_Namespace);
					clang_visitChildren(cursor, enumerationVisitor, newElement);
				} else {
					clang_visitChildren(cursor, enumerationVisitor, parentElement);
				}
			}
			break;
			case CXCursor_VarDecl:
			{
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

				const std::string varName = utils::getClangString(clang_getCursorSpelling(cursor));
				ElementVisitor::MemberVariableData data;
				data.isStatic = true;
				data.isConst = clang_isConstQualifiedType(clang_getCursorType(cursor)) != 0;
				parentElement->addMemberVariable(varName, data);
			}
			break;
			case CXCursor_FunctionDecl:
			{
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
			case CXCursor_TypedefDecl:
			case CXCursor_TypeAliasDecl:
			{
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

				const std::string aliasName = utils::getClangString(clang_getCursorSpelling(cursor));
				const std::string categoryName = utils::getClangString(clang_getTypeSpelling(clang_getCursorType(cursor)));
				const std::string originalName = utils::getClangString(clang_getTypeSpelling(clang_getCanonicalType(clang_getCursorType(cursor))));

				if (nullptr != parentElement) {
					ElementVisitor* originalElement = parentElement->getDeriveClass(originalName);
					if (nullptr != originalElement) {
						originalElement->addAliasName(aliasName);
					}
				}
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
		tu = clang_parseTranslationUnit(index, tempFilename.c_str()
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

		visitor.generateDocs();
	}
}