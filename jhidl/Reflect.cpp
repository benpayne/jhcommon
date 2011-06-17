#include "Reflect.h"
#include "JHCOM.h"
#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOISE );

using namespace JHCOM;

/*
	class Method
	{
	public:	
		std::string &getName();
		Class &getReturnType();
		void getParamTypes( std::vector<ClassInfo*> &params );	
		
		//void invoke( ISupports *obj, vector<Object*> &params );
	
	private:
		Method( const string &name, ClassInfo *ret, vector<ClassInfo*> &params );
		
		std::string mName;
		ClassInfo *mRetType;
		std::vector<ClassInfo*> mParamTypes;
	};
	
	class Field
	{
	public:
		string &getName();
		JHClass &getType();
		
		bool  getBool();
		uint64_t getInteger();
		double getFloat();
		int getEnum();
		string getStrings();
		
	private:
		Field( const string &name, JHClass *type, int array_size );
		
		string mName;
		Class *mType;
	};
	
	class TypeInfo
	{
	public:
	TypeInfo();
	};
	
	class ClassInfo
	{
	public:
	        enum ClassType {
			TYPE_INTERFACE,
			TYPE_STRUCT,
			TYPE_ARRAY,
			TYPE_ENUM,
			TYPE_BOOL,
			TYPE_INTTEGER,
			TYPE_FLOAT,
			TYPE_STRING
		};
		
		std::string &getName();
		Method &getMethod( std::string name );
		void getMethods( std::vector<Method*> methods );
		Field &getField( std::string name );
		void getField( std::vector<Method*> methods );
		ClassType getType();
		Class &getArrayType();
		
		bool isPrimitive();
		bool isInterface();
		bool isArray();
		bool isStruct();
		
	protected:
		// contruct a primitive type, int, float, string
		ClassInfo( ClassType type );
		// contruct a for an interface or struct
		ClassInfo( ClassType type, std::string &name );
		// construct an array with element type and size
		ClassInfo( ClassInfo &elem_type );
		// construct an enum with n elements
		//ClassInfo( int num_elem );
		
		void addMethod( const std::string &name, ClassInfo *ret, ClassInfo *params, ... );
		void addField( const std::string &name, ClassInfo *type );
		
	private:
		std::string mName;
		ClassType mType;
		std::vector<Methods> mMethods;
		std::vector<Fields> mFields;
		Class *mArrayElem;
		int mEnumSize;
	};
*/

#if 0
ClassInfo::ClassInfo( ClassType type ) : mType( type ), mArrayElem( NULL ), 
					 mEnumSize( 0 )
{
	if ( ! isPrimitive() )
	{
		LOG_WARN( "Calling Wrong ClassInfo Constructor" );
	}
}

ClassInfo::ClassInfo( ClassType type, std::string &name ) : mName( name ), mType( type ), 
							    mArrayElem( NULL ), 
							    mEnumSize( 0 )
{
	if ( ! isInterface() || ! isStruct() )
	{
		LOG_WARN( "Calling Wrong ClassInfo Constructor" );
	}
}

ClassInfo::ClassInfo( ClassInfo *elem ) : mType( TYPE_ARRAY ), mArrayElem( elem ), 
					  mEnumSize( 0 )
{
}

void ClassInfo::addMethod( const std::string &name, ClassInfo *ret, ClassInfo *params, ... )
{
	std::vector<ClassInfo*> params;
}
#endif 

#include "tinyxml.h"

Method *parseMethod( TypeManager *tm, TiXmlElement *method )
{
	TiXmlElement *name = method->FirstChildElement( "name" );
	
	if ( name == NULL )
		return NULL;

	std::string method_name = name->GetText();

	TiXmlElement *ret = method->FirstChildElement( "returnType" );
	const TypeInfo *ret_type = NULL;
	
	if ( ret != NULL )
	{
		std::string ret_name = ret->GetText();
		ret_type = tm->findType( ret_name );
	}
	
	Method *m = new Method( method_name, ret_type );
	
	std::string async = method->Attribute( "type" );

	if ( async == "async" )
		m->setMethodAsync();
	
	TiXmlElement *param = method->FirstChildElement( "param" );
	while ( param != NULL )
	{
		TiXmlElement *type = param->FirstChildElement( "type" );
		TiXmlElement *name = param->FirstChildElement( "name" );
		
		std::string dir = param->Attribute( "dir" );
		ParamInfo::ParamType d = ParamInfo::PARAM_INOUT;
		
		if ( dir == "in" )
			d = ParamInfo::PARAM_IN;
		else if ( dir == "out" )
			d = ParamInfo::PARAM_OUT;

		if ( type == NULL || name == NULL || dir == "" )
		{
			delete m;
			return NULL;
		}
		
		std::string type_name = type->GetText();
		const TypeInfo *param_type = tm->findType( type_name );

		std::string param_name = name->GetText();
		
		ParamInfo p( param_name, d, param_type );
		m->addParam( p );
		
		param = param->NextSiblingElement( "param" );
	}
	
	return m;
}

TypeInfo *parseClass( TypeManager *tm, TiXmlElement *node )
{
	TiXmlElement *name = node->FirstChildElement( "name" );
	
	if ( name == NULL )
	{
		LOG_WARN( "Class has no name" );
		return NULL;
	}
	
	ClassInfo *ci = new ClassInfo( name->GetText() );
	TypeInfo *ti = new TypeInfo( name->GetText(), TypeInfo::TYPE_INTERFACE, ci );
	
	TiXmlElement *iid = node->FirstChildElement( "iid" );

	if ( iid != NULL )
	{
		std::string str = iid->GetText();
		ci->setIID( str );
	}
	
	TiXmlElement *parent = node->FirstChildElement( "parent" );
	while ( parent != NULL )
	{
		//ci->addParent( ... )
		parent = parent->NextSiblingElement( "parent" );
	}

	TiXmlElement *method = node->FirstChildElement( "method" );
	while ( method != NULL )
	{
		name = method->FirstChildElement( "name" );
		
		Method *m = parseMethod( tm, method );
		
		if ( m != NULL )
		{
			ci->addMethod( *m );
			delete m;
		}
		
		method = method->NextSiblingElement( "method" );
	}
	
	return ti;
}

void TypeManager::loadClassInfo( const std::string &name )
{
	TRACE_BEGIN( LOG_LVL_NOTICE );
	TiXmlDocument d( name.c_str() );
	d.LoadFile();
	
	TiXmlElement *n = d.RootElement();
		
	while ( n != NULL )
	{
		if ( n->ValueStr() == "enum" )
		{
			LOG_NOTICE( "Found Enum" );
		}
		else if ( n->ValueStr() == "struct" )
		{
			LOG_NOTICE( "Found Struct" );
		}
		else if ( n->ValueStr() == "class" )
		{
			LOG_NOTICE( "Found Class" );
			TypeInfo *ti = parseClass( this, n );
			if ( ti != NULL )
				addType( ti );
		}
		else if ( n->ValueStr() == "alias" )
		{
			LOG_NOTICE( "Found Alias" );
		}
		else
			LOG_WARN( "unknown elment %s", n->Value() );
		
		n = n->NextSiblingElement();
	}
}
