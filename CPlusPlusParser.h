#pragma once

#include "Options.h"

#include <clang-c/Index.h>

#include <set>
#include <map>
#include <vector>

enum class VisitorElementType
{
	VisitorElementType_None,
	VisitorElementType_Namespace,
	VisitorElementType_Class,
	VisitorElementType_Enumeration,
	VisitorElementType_Function,
	VisitorElementType_Parameter,
	VisitorElementType_Max,
};

class ElementVisitor
{
public:

	ElementVisitor(const std::string& name = "", ElementVisitor* parent = nullptr
		, const std::string& accessPrefix = "", VisitorElementType elementType = VisitorElementType::VisitorElementType_None);

public:

	struct FuncParameter
	{
		bool isConst;
		std::string variableType;
		std::string variableName;
		std::string defaultValue;
	};

	struct FuncOverloadData
	{
		bool isConstructor;
		bool isStatic;
		bool isConst;
		std::string funcSignature;
		std::string funcDecl;
		std::vector<FuncParameter> variables;
		std::string comments;
		CX_CXXAccessSpecifier accessSpecifierKind;
	};

	struct MemberVariableData
	{
		bool isStatic;
		bool isConst;
		CX_CXXAccessSpecifier accessSpecifierKind;
	};

public:

	std::string getName() const;

	void addDeriveClass(const std::string& name, ElementVisitor* derive);

	void addNestedNamespace(const std::string& name, ElementVisitor* ns);

	bool hasNestedNamespace(const std::string& name);

	ElementVisitor* getNestedNamespace(const std::string& name);

	std::string getAccessPrefix() const;

	bool hasDeriveClass(const std::string& name);

	ElementVisitor* getDeriveClass(const std::string& name);

	bool existsFunc(const std::string& name);

	bool existsFuncSignature(const std::string& name, const std::string& signature);

	void addMemberVariable(const std::string& varName, const ElementVisitor::MemberVariableData& data);

	void addBaseClassName(const std::string& name, CX_CXXAccessSpecifier kind);

	void addEnumerationName(const std::string& name);

	void addAliasName(const std::string& name);

	void addFunction(const std::string& funcName, const ElementVisitor::FuncOverloadData& data);

	void generateDocs();

private:

	void initFuncContainer(const std::string& funcName);

	void generateClassDocs();

	void getFuncsByAccessSpecifierKind(std::map<std::string, std::map<std::string, FuncOverloadData>>& funcs
		, CX_CXXAccessSpecifier kind);

	void getMemberVariablesByAccessSpecifierKind(std::map<std::string, MemberVariableData>& memberVariables
		, CX_CXXAccessSpecifier kind);

	void writeFuncs(std::stringstream& ss, std::map<std::string, std::map<std::string, FuncOverloadData>> funcs);

private:

	VisitorElementType elementType_;

	std::string name_;

	std::string accessPrefix_;

	ElementVisitor* parent_;

	std::map<std::string, ElementVisitor*> deriveClasses_;

	std::map<std::string, ElementVisitor*> nestedNamespaces_;

	std::vector<std::string> aliasNameList_;

	std::map<std::string, MemberVariableData> memberVariables_;

	std::map<std::string, CX_CXXAccessSpecifier> inheritClasses_;

	std::set<std::string> enumerations_;

	std::map<std::string, std::map<std::string, FuncOverloadData>> funcs_;

	CX_CXXAccessSpecifier accessSpecifierKind;
};

class Options;

namespace cplusplus
{
	void parse();

	extern Options opts;

	extern bool preProcessing;

	extern std::map<CXFileUniqueID, std::set<int32_t>> exportSourceLocation;

	extern std::set<CXFileUniqueID> finishedFiles;

	extern std::set<std::string> exportedClasses;

	extern CXTranslationUnit tu;
};

