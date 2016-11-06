#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <tuple>
#include <algorithm>

class asIScriptEngine;
class asIScriptModule;
class asIScriptFunction;
class asITypeInfo;

enum MemberVisibility
{
	MV_None,
	MV_Public,
	MV_Private,
	MV_Protected,
};

enum MethodTrait
{
	MT_Normal,
	MT_Arithmetic,
	MT_Arithmetic_r,
};

class ScriptDocs
{
public:
	asIScriptEngine* m_engine;
	std::ofstream m_stream;

public:
	ScriptDocs(asIScriptEngine* engine);
	~ScriptDocs();
	void Write(const char* filename, bool scripts = false);

private:
	MemberVisibility m_lastVis;
	asITypeInfo* m_stringType;
	int m_indent;
	std::string m_inNamespace;
	std::string m_lastName;
	std::string m_lastGroup;

private:
	void WriteFor(asIScriptEngine* engine);
	void WriteFor(asIScriptModule* mod);

	void BeginNamespace(const char* ns);
	std::string GetIndent(int offset = 0);
	std::string GetTypeName(int typeID);

	std::tuple<const char*, MethodTrait> GetFunctionName(const std::string &name);

	void WriteClass(asITypeInfo* type);
	void WriteEnum(asITypeInfo* enumType);
	void WriteTypedef(asITypeInfo* typedefType);
	void WriteGlobalFunction(asIScriptFunction* func);
	void WriteGlobalVariable(const char* propName, const char* ns, int propTypeID, bool isConst);

	void WriteHeader(void* p, bool member);
	void WriteFunction(asIScriptFunction* func, bool astypedef = false);
	void WriteVisibility(MemberVisibility vis);
};

class ScriptDocsMetadata
{
public:
	std::string m_description;
	std::string m_group;

	bool m_isMember = false;
};

#define AS_DOCS_USERDATA 100

//TODO: Make these macros less big

#ifdef GENERATE_DOCS

#define AS_DOCS_META(obj) \
	(obj->GetUserData(AS_DOCS_USERDATA) != nullptr \
	? (ScriptDocsMetadata*)obj->GetUserData(AS_DOCS_USERDATA) \
	: new ScriptDocsMetadata())

#define AS_DOCS_TYPE(engine, typeID, group) { \
	assert(typeID >= 0); \
	asITypeInfo* asd_type = engine->GetTypeInfoById(typeID); \
	ScriptDocsMetadata* asd_pmd = AS_DOCS_META(asd_type); \
	asd_pmd->m_group = group; \
	asd_type->SetUserData(asd_pmd, AS_DOCS_USERDATA); \
}

#define AS_DOCS_TYPE_DESC(engine, typeID, group, desc) { \
	assert(typeID >= 0); \
	asITypeInfo* asd_type = engine->GetTypeInfoById(typeID); \
	ScriptDocsMetadata* asd_pmd = AS_DOCS_META(asd_type); \
	asd_pmd->m_description = desc; \
	if (group != nullptr) asd_pmd->m_group = group; \
	asd_type->SetUserData(asd_pmd, AS_DOCS_USERDATA); \
}

#define AS_DOCS_METHOD(engine, funcID, group) { \
	assert(funcID >= 0); \
	asIScriptFunction* asd_func = engine->GetFunctionById(funcID); \
	ScriptDocsMetadata* asd_pmd = AS_DOCS_META(asd_func); \
	asd_pmd->m_group = group; \
	asd_pmd->m_isMember = true; \
	asd_func->SetUserData(asd_pmd, AS_DOCS_USERDATA); \
}

#define AS_DOCS_METHOD_DESC(engine, funcID, group, desc) { \
	assert(funcID >= 0); \
	asIScriptFunction* asd_func = engine->GetFunctionById(funcID); \
	ScriptDocsMetadata* asd_pmd = AS_DOCS_META(asd_func); \
	asd_pmd->m_description = desc; \
	if (group != nullptr) asd_pmd->m_group = group; \
	asd_pmd->m_isMember = true; \
	asd_func->SetUserData(asd_pmd, AS_DOCS_USERDATA); \
}

#define AS_DOCS_FUNC(engine, funcID, group) { \
	assert(funcID >= 0); \
	asIScriptFunction* asd_func = engine->GetFunctionById(funcID); \
	ScriptDocsMetadata* asd_pmd = AS_DOCS_META(asd_func); \
	asd_pmd->m_group = group; \
	asd_func->SetUserData(asd_pmd, AS_DOCS_USERDATA); \
}

#define AS_DOCS_FUNC_DESC(engine, funcID, group, desc) { \
	assert(funcID >= 0); \
	asIScriptFunction* asd_func = engine->GetFunctionById(funcID); \
	ScriptDocsMetadata* asd_pmd = AS_DOCS_META(asd_func); \
	asd_pmd->m_description = desc; \
	if (group != nullptr) asd_pmd->m_group = group; \
	asd_func->SetUserData(asd_pmd, AS_DOCS_USERDATA); \
}

#else

#define AS_DOCS_TYPE(...)
#define AS_DOCS_TYPE_DESC(...)
#define AS_DOCS_METHOD(...)
#define AS_DOCS_METHOD_DESC(...)
#define AS_DOCS_FUNC(...)
#define AS_DOCS_FUNC_DESC(...)

#endif
