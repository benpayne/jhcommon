#include "Reflect.h"

using namespec JHCOM;

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

