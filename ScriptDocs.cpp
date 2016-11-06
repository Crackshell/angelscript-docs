#include "ScriptDocs.h"

#include <angelscript.h>

static void strSplit(const std::string &s, char delim, std::vector<std::string> &elems)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
}

ScriptDocs::ScriptDocs(asIScriptEngine* engine)
{
	m_engine = engine;
	m_stringType = m_engine->GetTypeInfoByDecl("string");
	m_indent = 0;
}

ScriptDocs::~ScriptDocs()
{
}

void ScriptDocs::Write(const char* filename, bool scripts)
{
	m_stream.open(filename, std::ofstream::out);
	m_stream << "// Auto-generated engine docs" << std::endl << std::endl;
	m_stream << "typedef void AnyType;" << std::endl << std::endl;
	WriteFor(m_engine);
	m_stream.close();

	if (scripts)
	{
		m_stream.open(std::string(filename) + "_Scripts", std::ofstream::out);
		m_stream << "// Auto-generated script docs" << std::endl << std::endl;
		m_stream << "typedef void AnyType;" << std::endl << std::endl;
		WriteFor(m_engine->GetModule("Scripts"));
		m_stream.close();
	}
}

void ScriptDocs::WriteFor(asIScriptEngine* engine)
{
	// For each object type
	int nTypes = engine->GetObjectTypeCount();
	for (int i = 0; i < nTypes; i++)
	{
		asITypeInfo* type = engine->GetObjectTypeByIndex(i);
		WriteClass(type);
	}

	// For each enum
	int nEnums = engine->GetEnumCount();
	for (int i = 0; i < nEnums; i++)
	{
		// Get info about the enum
		asITypeInfo* enumType = engine->GetEnumByIndex(i);
		WriteEnum(enumType);
	}

	// For each funcdef
	int nFuncDefs = engine->GetFuncdefCount();
	for (int i = 0; i < nFuncDefs; i++)
	{
		// Get info about the funcdef
		asITypeInfo* funcdefType = engine->GetFuncdefByIndex(i);

		// Write the funcdef
		BeginNamespace(funcdefType->GetNamespace());
		WriteHeader(funcdefType->GetUserData(AS_DOCS_USERDATA), false);
		WriteFunction(funcdefType->GetFuncdefSignature(), true);
	}

	// For each typedef
	int nTypeDefs = engine->GetTypedefCount();
	for (int i = 0; i < nTypeDefs; i++)
	{
		// Get info about the typedef
		asITypeInfo* typedefType = engine->GetTypedefByIndex(i);
		WriteTypedef(typedefType);
	}

	// For each global function
	int nGlobFuncs = engine->GetGlobalFunctionCount();
	for (int i = 0; i < nGlobFuncs; i++)
	{
		// Get info about the function
		asIScriptFunction* func = engine->GetGlobalFunctionByIndex(i);
		WriteGlobalFunction(func);
	}

	// For each global property
	int nGlobProps = engine->GetGlobalPropertyCount();
	for (int i = 0; i < nGlobProps; i++)
	{
		// Get info about the property
		const char* propName, *ns;
		int propTypeID;
		bool isConst;
		engine->GetGlobalPropertyByIndex(i, &propName, &ns, &propTypeID, &isConst, nullptr, nullptr, nullptr);
		WriteGlobalVariable(propName, ns, propTypeID, isConst);
	}

	if (m_inNamespace != "")
	{
		m_indent--;
		m_stream << GetIndent() << "}" << std::endl;
		m_inNamespace = "";
	}
}

void ScriptDocs::WriteFor(asIScriptModule* mod)
{
	// For each object type
	int nTypes = mod->GetObjectTypeCount();
	for (int i = 0; i < nTypes; i++)
	{
		asITypeInfo* type = mod->GetObjectTypeByIndex(i);
		WriteClass(type);
	}

	// For each enum
	int nEnums = mod->GetEnumCount();
	for (int i = 0; i < nEnums; i++)
	{
		// Get info about the enum
		asITypeInfo* enumType = mod->GetEnumByIndex(i);
		WriteEnum(enumType);
	}

	// For each typedef
	int nTypeDefs = mod->GetTypedefCount();
	for (int i = 0; i < nTypeDefs; i++)
	{
		// Get info about the typedef
		asITypeInfo* typedefType = mod->GetTypedefByIndex(i);
		WriteTypedef(typedefType);
	}

	// For each global function
	int nGlobFuncs = mod->GetFunctionCount();
	for (int i = 0; i < nGlobFuncs; i++)
	{
		// Get info about the function
		asIScriptFunction* func = mod->GetFunctionByIndex(i);
		WriteGlobalFunction(func);
	}

	// For each global property
	int nGlobProps = mod->GetGlobalVarCount();
	for (int i = 0; i < nGlobProps; i++)
	{
		// Get info about the property
		const char* propName, *ns;
		int propTypeID;
		bool isConst;
		mod->GetGlobalVar(i, &propName, &ns, &propTypeID, &isConst);
		WriteGlobalVariable(propName, ns, propTypeID, isConst);
	}

	if (m_inNamespace != "")
	{
		m_indent--;
		m_stream << GetIndent() << "}" << std::endl;
		m_inNamespace = "";
	}
}

void ScriptDocs::BeginNamespace(const char* ns)
{
	std::string strNs(ns == nullptr ? "" : ns);

	if (m_inNamespace == strNs)
		return;

	if (m_lastGroup != "")
	{
		m_indent--;
		m_lastGroup = "";
		m_stream << GetIndent() << "//! \\}" << std::endl << std::endl;
	}

	if (m_inNamespace != "")
	{
		m_indent--;
		m_stream << GetIndent() << "}" << std::endl;
	}

	m_inNamespace = strNs;
	if (strNs != "")
	{
		m_stream << GetIndent() << "namespace " << strNs << std::endl;
		m_stream << GetIndent() << "{" << std::endl;
		m_indent++;
	}
}

std::string ScriptDocs::GetIndent(int offset)
{
	std::string ret;
	for (int i = 0; i < m_indent + offset; i++)
		ret += "  ";
	return ret;
}

std::string ScriptDocs::GetTypeName(int typeID)
{
	switch (typeID)
	{
	case -1: return "AnyType";
	case asTYPEID_VOID: return "void";
	case asTYPEID_BOOL: return "bool";
	case asTYPEID_INT8: return "int8";
	case asTYPEID_INT16: return "int16";
	case asTYPEID_INT32: return "int32";
	case asTYPEID_INT64: return "int64";
	case asTYPEID_UINT8: return "uint8";
	case asTYPEID_UINT16: return "uint16";
	case asTYPEID_UINT32: return "uint32";
	case asTYPEID_UINT64: return "uint64";
	case asTYPEID_FLOAT: return "float";
	case asTYPEID_DOUBLE: return "double";
	}

	asITypeInfo* type = m_engine->GetTypeInfoById(typeID);
	std::string ret = type->GetName();

	int nSubTypes = type->GetSubTypeCount();
	if (nSubTypes > 0)
	{
		ret += "<";
		for (int i = 0; i < nSubTypes; i++)
		{
			ret += GetTypeName(type->GetSubTypeId(i));
			if (i + 1 < nSubTypes)
				ret += ", ";
		}
		ret += ">";
	}

	if (typeID & asTYPEID_OBJHANDLE)
		ret += "*";

	return ret;
}

std::tuple<const char*, MethodTrait> ScriptDocs::GetFunctionName(const std::string &name)
{
	//TODO: Some of these could be made better (eg. opCmp covers 4 operators, but we only show operator<, no distinction between opPreInc and opPostInc, etc)
	if (name == "opNeg") return std::make_tuple("operator-", MT_Normal);
	if (name == "opCom") return std::make_tuple("operator~", MT_Normal);
	if (name == "opPreInc" || name == "opPostInc") return std::make_tuple("operator++", MT_Normal);
	if (name == "opPreDec" || name == "opPostDec") return std::make_tuple("operator--", MT_Normal);
	if (name == "opEquals") return std::make_tuple("operator==", MT_Normal);
	if (name == "opCmp") return std::make_tuple("operator<", MT_Normal);

	if (name == "opAssign") return std::make_tuple("operator=", MT_Normal);
	if (name == "opAddAssign") return std::make_tuple("operator+=", MT_Normal);
	if (name == "opSubAssign") return std::make_tuple("operator-=", MT_Normal);
	if (name == "opMulAssign") return std::make_tuple("operator*=", MT_Normal);
	if (name == "opDivAssign") return std::make_tuple("operator/=", MT_Normal);
	if (name == "opModAssign") return std::make_tuple("operator%=", MT_Normal);
	if (name == "opPowAssign") return std::make_tuple("operator**=", MT_Normal);
	if (name == "opAndAssign") return std::make_tuple("operator&=", MT_Normal);
	if (name == "opOrAssign") return std::make_tuple("operator|=", MT_Normal);
	if (name == "opXorAssign") return std::make_tuple("operator^=", MT_Normal);
	if (name == "opShlAssign") return std::make_tuple("operator<<=", MT_Normal);
	if (name == "opShrAssign") return std::make_tuple("operator>>=", MT_Normal);
	if (name == "opUShrAssign") return std::make_tuple("operator>>>=", MT_Normal);

#define BOTH_OPS(str, op) \
if (name == str) return std::make_tuple("operator" op, MT_Arithmetic); \
if (name == str "_r") return std::make_tuple("operator" op, MT_Arithmetic_r);
	BOTH_OPS("opAdd", "+");
	BOTH_OPS("opSub", "-");
	BOTH_OPS("opMul", "*");
	BOTH_OPS("opDiv", "/");
	BOTH_OPS("opMod", "%");
	BOTH_OPS("opPow", "**");
	BOTH_OPS("opAnd", "&");
	BOTH_OPS("opOr", "|");
	BOTH_OPS("opXor", "^");
	BOTH_OPS("opShl", "<<");
	BOTH_OPS("opShr", ">>");
	BOTH_OPS("opUShr", ">>>");

	if (name == "opIndex") return std::make_tuple("operator[]", MT_Normal);
	if (name == "opCall") return std::make_tuple("operator()", MT_Normal);

	return std::make_tuple(name.c_str(), MT_Normal);
}

void ScriptDocs::WriteClass(asITypeInfo* type)
{
	// Write the class
	BeginNamespace(type->GetNamespace());

	WriteHeader(type->GetUserData(AS_DOCS_USERDATA), false);

	int nSubTypes = type->GetSubTypeCount();
	if (nSubTypes > 0)
	{
		m_stream << GetIndent() << "template<";
		for (int j = 0; j < nSubTypes; j++)
		{
			m_stream << "class " << type->GetSubType(j)->GetName();
			if (j + 1 < nSubTypes)
				m_stream << ", ";
		}
		m_stream << ">" << std::endl;
	}

	std::string inheritance = "";

	asITypeInfo* baseType = type->GetBaseType();
	if (baseType != nullptr)
	{
		inheritance = std::string(" : public ") + baseType->GetName();
		if (type->GetInterfaceCount() > 0)
			inheritance += ", ";
	}

	int nInterfaces = type->GetInterfaceCount();
	for (int i = 0; i < nInterfaces; i++)
	{
		if (inheritance == "")
			inheritance = " : ";
		inheritance += std::string("public ") + type->GetInterface(i)->GetName();
		if (i + 1 < nInterfaces)
			inheritance += ", ";
	}

	m_stream << GetIndent() << "class " << type->GetName() << inheritance << std::endl;
	m_stream << GetIndent() << "{" << std::endl;
	m_indent++;

	m_lastVis = MV_None;

	// For each property
	int nProps = type->GetPropertyCount();
	for (int i = 0; i < nProps; i++)
	{
		// Get info about the property
		const char* propName;
		int propTypeID, propOffset;
		bool propPrivate, propProtected, propReference;

		type->GetProperty(i, &propName, &propTypeID, &propPrivate, &propProtected, &propOffset, &propReference);

		// Kind of a hack: Exclude if this property is inherited from one of its base classes
		bool isInherited = false;
		while (baseType != nullptr)
		{
			int nBaseProps = baseType->GetPropertyCount();
			for (int j = 0; j < nBaseProps; j++)
			{
				int basePropOffset;
				baseType->GetProperty(j, nullptr, nullptr, nullptr, nullptr, &basePropOffset);
				if (basePropOffset == propOffset)
				{
					isInherited = true;
					break;
				}
			}
			if (isInherited)
				break;
			baseType = baseType->GetBaseType();
		}
		if (isInherited)
			continue;

		// Get the visibility
		MemberVisibility vis = MV_Public;
		if (propPrivate) vis = MV_Private;
		else if (propProtected) vis = MV_Protected;

		WriteVisibility(vis);

		//TODO: Set description for properties (they don't have an asITypeInfo object)
		//WriteHeader(type->GetUserData(AS_DOCS_USERDATA_DESC));

		// Write the property
		m_stream << GetIndent() << GetTypeName(propTypeID);
		if (propReference)
			m_stream << "&";
		m_stream << " " << propName << ";" << std::endl;
	}

	m_lastVis = MV_None;

	// For each method
	int nMethods = type->GetMethodCount();
	for (int j = 0; j < nMethods; j++)
	{
		// Write the function
		asIScriptFunction* func = type->GetMethodByIndex(j);

		// Filter inherited methods
		if (func->GetObjectType() != type)
			continue;

		WriteHeader(func->GetUserData(AS_DOCS_USERDATA), true);
		WriteFunction(func);
	}

	m_indent--;
	m_stream << GetIndent() << "};" << std::endl;
}

void ScriptDocs::WriteEnum(asITypeInfo* enumType)
{
	// Write the enum
	BeginNamespace(enumType->GetNamespace());
	WriteHeader(enumType->GetUserData(AS_DOCS_USERDATA), false);
	m_stream << GetIndent() << "enum " << enumType->GetName() << std::endl;
	m_stream << GetIndent() << "{" << std::endl;
	m_indent++;

	// For each enum value
	int nValues = enumType->GetEnumValueCount();
	for (int j = 0; j < nValues; j++)
	{
		int value;
		const char* valueName = enumType->GetEnumValueByIndex(j, &value);

		//TODO: Set description for enum values (they don't have an asITypeInfo object)
		//WriteHeader(enumType->GetUserData(AS_DOCS_USERDATA_DESC));

		// Write the value
		m_stream << GetIndent() << valueName << " = " << value << "," << std::endl;
	}

	m_indent--;
	m_stream << GetIndent() << "};" << std::endl;
}

void ScriptDocs::WriteTypedef(asITypeInfo* typedefType)
{
	// Write the typedef
	BeginNamespace(typedefType->GetNamespace());
	WriteHeader(typedefType->GetUserData(AS_DOCS_USERDATA), false);
	m_stream << GetIndent() << "typedef " << GetTypeName(typedefType->GetTypedefTypeId());
	m_stream << " " << typedefType->GetName() << ";";
}

void ScriptDocs::WriteGlobalFunction(asIScriptFunction* func)
{
	// Write the function
	BeginNamespace(func->GetNamespace());
	WriteHeader(func->GetUserData(AS_DOCS_USERDATA), false);
	WriteFunction(func);
}

void ScriptDocs::WriteGlobalVariable(const char* propName, const char* ns, int propTypeID, bool isConst)
{
	// Write the property
	BeginNamespace(ns);

	//TODO: Set description for global properties (they don't have an asITypeInfo object)
	//WriteHeader(type->GetUserData(AS_DOCS_USERDATA_DESC));

	m_stream << GetIndent();
	if (isConst)
		m_stream << "const ";
	m_stream << GetTypeName(propTypeID);
	m_stream << " " << propName << ";" << std::endl;
}

void ScriptDocs::WriteHeader(void* p, bool member)
{
	if (p == nullptr)
	{
		if (member)
		{
			if (m_lastName != "")
			{
				m_lastName = "";
				m_stream << GetIndent() << "//! \\name" << std::endl << std::endl;
			}
		}
		else
		{
			if (m_lastGroup != "")
			{
				m_indent--;
				m_lastGroup = "";
				m_stream << GetIndent() << "//! \\}" << std::endl << std::endl;
			}
		}
		return;
	}

	ScriptDocsMetadata &meta = *(ScriptDocsMetadata*)p;

	if (meta.m_group != "" || meta.m_description != "")
	{
		if (member)
		{
			if (meta.m_group != m_lastName)
			{
				m_lastName = meta.m_group;
				if (meta.m_group != "")
					m_stream << GetIndent() << "//! \\name " << meta.m_group << std::endl << std::endl;
				else
					m_stream << GetIndent() << "//! \\name" << std::endl << std::endl;
			}
		}
		else
		{
			if (meta.m_group != m_lastGroup)
			{
				if (m_lastGroup != "")
				{
					m_indent--;
					m_stream << GetIndent() << "//! \\}" << std::endl << std::endl;
				}

				m_lastGroup = meta.m_group;

				if (meta.m_group != "")
				{
					m_stream << GetIndent() << "//! \\addtogroup " << meta.m_group << " " << meta.m_group << std::endl;
					m_stream << GetIndent() << "//! \\{" << std::endl;
					m_indent++;
				}
			}
		}

		if (meta.m_description != "")
		{
			std::vector<std::string> lines;
			strSplit(meta.m_description, '\n', lines);
			for (auto line : lines)
				m_stream << GetIndent() << "//! " << line << std::endl;
		}
	}
}

void ScriptDocs::WriteFunction(asIScriptFunction* func, bool astypedef)
{
	// Get info about the function
	asDWORD dwReturnTypeFlags;
	int funcReturnTypeID = func->GetReturnTypeId(&dwReturnTypeFlags);

	// Get the visibility
	MemberVisibility vis = MV_Public;
	if (func->IsPrivate()) vis = MV_Private;
	else if (func->IsProtected()) vis = MV_Protected;

	WriteVisibility(vis);

	// Special case: explicit and implicit casting has a different syntax in C++.
	//TODO: This duplicates if both opConv and opCast are set (but why would you want to have both?)
	std::string funcName = func->GetName();
	if (funcName == "opCast" || funcName == "opConv" || funcName == "opImplCast" || funcName == "opImplConv")
	{
		m_stream << GetIndent();
		if (funcName == "opCast" || funcName == "opConv")
			m_stream << "explicit ";
		m_stream << "operator " << GetTypeName(funcReturnTypeID) << "();" << std::endl;
		return;
	}

	// Write the function
	m_stream << GetIndent();
	if (astypedef)
		m_stream << "typedef ";

	if (dwReturnTypeFlags & asTYPEID_OBJHANDLE)
		m_stream << GetTypeName(funcReturnTypeID) << "*";
	else if (dwReturnTypeFlags & asTYPEID_HANDLETOCONST)
		m_stream << "const " << GetTypeName(funcReturnTypeID) << "*";
	else
		m_stream << GetTypeName(funcReturnTypeID);

	// Get name info
	MethodTrait trait = MT_Normal;
	if (!astypedef)
	{
		auto funcNameInfo = GetFunctionName(funcName);
		funcName = std::get<0>(funcNameInfo);
		trait = std::get<1>(funcNameInfo);
	}

	// Maybe write it with a typedef syntax
	if (astypedef)
		m_stream << " (*" << funcName << ")(";
	else
		m_stream << " " << funcName << "(";

	// Get parameter count
	int nParams = func->GetParamCount();

	// Special case: For methods marked with arithmatic trait, we need to add the "this" parameter.
	// string operator+(string, float); // MT_Arithmatic
	// string operator+(float, string); // MT_Arithmatic_r
	if (trait == MT_Arithmetic)
	{
		auto typeObject = func->GetObjectType();
		if (typeObject != nullptr)
		{
			m_stream << GetTypeName(typeObject->GetTypeId());
			if (nParams > 0)
				m_stream << ", ";
		}
	}

	// For each function parameter
	for (int i = 0; i < nParams; i++)
	{
		// Get info about the parameter
		const char* paramName;
		const char* paramDefault;
		int paramTypeID;
		asDWORD paramFlags;
		func->GetParam(i, &paramTypeID, &paramFlags, &paramName, &paramDefault);

		// Write the parameter to file
		if (paramFlags & asTM_CONST)
			m_stream << "const ";
		m_stream << GetTypeName(paramTypeID);
		if (paramFlags & asTM_INOUTREF)
			m_stream << "&";

		if (paramName != nullptr && strlen(paramName) > 0)
			m_stream << " " << paramName;

		if (paramDefault != nullptr)
		{
			// Angelscript returns this to us in space-delimited keywords, eg: "- 1", "vec2 ( 0 , 0 )", "Foo :: Bar"
			if (paramTypeID == m_stringType->GetTypeId())
				m_stream << " = " << paramDefault;
			else
			{
				// Remove spaces
				std::string strParamDefault(paramDefault);
				strParamDefault.erase(std::remove_if(strParamDefault.begin(), strParamDefault.end(), ::isspace), strParamDefault.end());
				m_stream << " = " << strParamDefault;
			}
		}

		// Maybe another parameter
		if (i + 1 < nParams)
			m_stream << ", ";
	}

	if (trait == MT_Arithmetic_r)
	{
		auto typeObject = func->GetObjectType();
		if (typeObject != nullptr)
		{
			if (nParams > 0)
				m_stream << ", ";
			m_stream << GetTypeName(typeObject->GetTypeId());
		}
	}

	m_stream << ");" << std::endl;
}

void ScriptDocs::WriteVisibility(MemberVisibility vis)
{
	if (vis != m_lastVis)
	{
		m_lastVis = vis;
		switch (vis)
		{
		case MV_Public: m_stream << GetIndent(-1) << "public:"; break;
		case MV_Private: m_stream << GetIndent(-1) << "private:"; break;
		case MV_Protected: m_stream << GetIndent(-1) << "protected:"; break;
		}
		m_stream << std::endl;
	}
}
