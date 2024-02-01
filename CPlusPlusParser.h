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
	};

	struct MemberVariableData
	{
		bool isStatic;
		bool isConst;
	};

public:

	void addDeriveClass(const std::string& name, ElementVisitor* derive);

	void addNestedNamespace(const std::string& name, ElementVisitor* ns);

	bool hasNestedNamespace(const std::string& name);

	ElementVisitor* getNestedNamespace(const std::string& name);

	std::string getAccessPrefix() const;

	bool hasDeriveClass(const std::string& name);

	ElementVisitor* getDeriveClass(const std::string& name);

	bool existsFunc(const std::string& name);

	bool existsFuncSignature(const std::string& name, const std::string& signature);

private:

	VisitorElementType elementType_;

	std::string name_;

	std::string accessPrefix_;

	ElementVisitor* parent_;

	std::map<std::string, ElementVisitor*> deriveClasses_;

	std::map<std::string, ElementVisitor*> nestedNamespaces_;

	std::vector<std::string> aliasNameList_;

	std::map<std::string, MemberVariableData> memberVariables_;

	std::set<std::string> inheritClasses_;

	std::set<std::string> enumerations_;

	std::map<std::string, std::map<std::string, FuncOverloadData>> funcs_;
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
};

