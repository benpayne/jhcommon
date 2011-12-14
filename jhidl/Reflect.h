#ifndef JHCOM_REFLECT_H_
#define JHCOM_REFLECT_H_

#include <string>
#include <vector>
#include "RefCount.h"

namespace JHCOM
{
	class ClassInfo;
	class TypeInfo;
	
	class ParamInfo
	{
	public:
		enum ParamType {
			PARAM_IN = 1,
			PARAM_OUT = 2,
			PARAM_INOUT = 3
		};
			
		// Normal Param
		ParamInfo( const std::string &name, ParamType param_type, const TypeInfo *type ) : mName( name ), mParamType( param_type ), mType( type ) {}
		
		const std::string &getName() const
		{
			return mName;
		}
		
		ParamType getParamType() const
		{
			return mParamType;
		}

		const TypeInfo *getType() const
		{	
			return mType;
		}
		
	private:
		std::string	mName;
		ParamType mParamType;
		SmartPtr<const TypeInfo> mType;
	};
	
	class Method
	{
	public:	
		Method( const std::string &name, const TypeInfo *ret_type ) : mName( name ), mRetType( ret_type ), mAsync( false ) {}

		void addParam( const ParamInfo &param )
		{
			mParamTypes.push_back( param );
		}
		
		void setMethodAsync()
		{
			mAsync = true;
		}
		
		const std::string &getName() const
		{
			return mName;
		}

		const TypeInfo *getReturnType() const
		{
			return mRetType;
		}

		void getParamTypes( std::vector<const ParamInfo *> &params ) const
		{
			for ( int i = 0; i < mParamTypes.size(); i++ )
			{
				params.push_back( &mParamTypes[ i ] );
			}
		}
		
		bool isAsync()
		{
			return mAsync;
		}
		
		bool operator==( const Method &other )
		{
			return mName == other.mName;
		}
		
		//void invoke( ISupports *obj, vector<Object*> &params );
		
	private:
		std::string mName;
		SmartPtr<const TypeInfo> mRetType;
		std::vector<ParamInfo> mParamTypes;
		bool mAsync;
	};
	
	class Field
	{
	public:
		Field( const std::string &name, const TypeInfo *type ) : mName( name ), mType( type ) {}

		const std::string &getName() const
		{
			return mName;
		}
		
		const TypeInfo *getType() const
		{		
			return mType;
		}
		
	private:
		std::string mName;
		SmartPtr<const TypeInfo> mType;
	};
	
	class ClassInfo
	{
	public:
		ClassInfo( std::string name ) : mName( name ) {}
		
		//void addMethod( const std::string &name, TypeInfo *ret, TypeInfo *params, ... );
		void addMethod( const Method &method )
		{
			mMethods.push_back( method );
		}
		
		void addField( const std::string &name, const TypeInfo *type )
		{
			mFields.push_back( Field( name, type ) );
		}
		
		void addParent( const ClassInfo *c )
		{
			mParents.push_back( c );
		}
		
		// if this class is a fully specified definition then is should be set "Concrete"
		void setConcrete()
		{
			mConcrete = true;
		}
		
		const std::string &getName() const
		{
			return mName;
		}
		
		const Method *getMethod( std::string name ) const
		{
			const Method *m = NULL;
			for( int i = 0; i < mMethods.size(); i++ )
			{
				if ( mMethods[ i ].getName() == name )
					m = &mMethods[ i ];
			}
			
			for ( int i = 0; m == NULL && i < mParents.size(); i++ )
			{
				m = mParents[ i ]->getMethod( name );
			}

			return m;
		}
		
		void getMethods( std::vector<const Method*> &methods ) const
		{
			for( int i = 0; i < mMethods.size(); i++ )
			{
				methods.push_back( &mMethods[ i ] );
			}
		}

		void getAllMethods( std::vector<const Method*> &methods ) const
		{
			for ( int i = 0; i < mParents.size(); i++ )
			{
				mParents[ i ]->getMethods( methods );
			}

			for( int i = 0; i < mMethods.size(); i++ )
			{
				methods.push_back( &mMethods[ i ] );
			}
		}
		
		const Field *getField( std::string name ) const
		{
			const Field *f = NULL;
			for( int i = 0; i < mFields.size(); i++ )
			{
				if ( mFields[ i ].getName() == name )
					f = &mFields[ i ];
			}
			
			return f;
		}
		
		void getFields( std::vector<const Field*> &fields ) const
		{
			for( int i = 0; i < mFields.size(); i++ )
			{
				fields.push_back( &mFields[ i ] );
			}
		}
		
		void getParents( std::vector<const ClassInfo*> &parents ) const 
		{
			for( int i = 0; i < mParents.size(); i++ )
			{
				parents.push_back( mParents[ i ] );
			}			
		}
		
		bool isConcrete()
		{
			return mConcrete;
		}
		
	private:
		std::string mName;
		bool		mConcrete;
		std::vector<const ClassInfo *> mParents;
		std::vector<Method> mMethods;
		std::vector<Field> mFields;
	};
	
	class Alias
	{
	public:
		Alias( const std::string &name, const TypeInfo *info ) : mName( name ), mType( info ) {}
		~Alias() {}
				
		const std::string &getName() const
		{
			return mName;
		}
		
		const TypeInfo *getType() const
		{
			return mType;
		}
				
	private:
		std::string	mName;
		SmartPtr<const TypeInfo> mType;
	};
	
	class Enumeration 
	{
	public:		
		Enumeration( const std::string &name ) : mName( name ) {}
		~Enumeration() {}
		
		void addElement( const std::string &element )
		{
			mElements.push_back( element );
		}
		
		const std::string &getName() const
		{
			return mName;
		}
		
		int getElement( const std::string &name ) const
		{
			for ( int i = 0; i < mElements.size(); i++ )
			{
				if ( name == mElements[ i ] )
					return i;
			}
			return -1;
		}

		const std::string &getElement( int id ) const
		{
			return mElements[ id ];
		}

		void getElements( std::vector<std::string> &elements ) const
		{
			for ( int i = 0; i < mElements.size(); i++ )
			{
				elements.push_back( mElements[ i ] );
			}
		}
		
	private:
		std::string	mName;
		std::vector<std::string> mElements;
	};
	
	class TypeInfo : public RefCount
	{
	public:
		enum Type {
			TYPE_INTERFACE,
			TYPE_STRUCT,
			TYPE_ARRAY,
			TYPE_ENUM,
			TYPE_ALIAS,
			TYPE_BOOL,
			TYPE_INTEGER,
			TYPE_FLOAT,
			TYPE_STRING
		};

		// For creating primative types, bool, int, float and string.
		TypeInfo( Type t ) : mType( t ), mClass( NULL ), mEnum( NULL ), mAlias( NULL ) {}
		// Only valide when first param is array, second is what the array is of.
		//TypeInfo( Type type_array, TypeInfo &elem_type, int size = -1 ) : mType( t ) {}
		// if type is interface or struct, then ClassInfo describes that type.
		TypeInfo( Type t, const ClassInfo *info ) : mType( t ), mClass( info ), mEnum( NULL ), mAlias( NULL ) {}
		// construct an enum with n elements
		TypeInfo( const Enumeration *enumeration ) : mType( TYPE_ENUM ), mClass( NULL ), mEnum( enumeration ), mAlias( NULL ) {}
		// construct an alias 
		TypeInfo( const Alias *alias ) : mType( TYPE_ALIAS ), mClass( NULL ), mEnum( NULL ), mAlias( alias ) {} 
		
		virtual ~TypeInfo() { delete mClass; delete mEnum;  delete mAlias; }

		Type getType() const
		{
			return mType;
		}
		
		const ClassInfo	*getClass() const
		{
			return mClass;
		}

		const Enumeration *getEnum() const
		{
			return mEnum;
		}
		
		const Alias *getAlias() const
		{
			return mAlias;
		}
		
		void setFileInfo( const char *file, int line )
		{
			mFilename = file;
			mLine = line;
		}
		
		const std::string &getFilename() const
		{
			return mFilename;
		}
		
		int getLineNumber() const
		{
			return mLine;
		}
		
		void setNamespace( const std::string &ns )
		{
			mNamespace = ns;
		}
		
		const std::string &getNamespace() const
		{
			return mNamespace;
		}
		
		const std::string getFullName() const
		{
			std::string name;
			
			if ( mNamespace.length() > 0 )
			{	
				name = mNamespace;
				name.append( "::" );
			}
			
			name.append( getName() );
		
			return name;
		}

		const std::string getName() const
		{
			std::string name;
						
			switch( mType )
			{
				case TYPE_INTERFACE:
				case TYPE_STRUCT:
					name.append( mClass->getName() );
					break;
				case TYPE_ALIAS:
					name.append( mAlias->getName() );
					break;
				case TYPE_ARRAY:
					break;
				case TYPE_ENUM:
					name.append( mEnum->getName() );
					break;
				case TYPE_BOOL:
					name = "bool";
					break;
				case TYPE_INTEGER:
					name = "int";
					break;
				case TYPE_FLOAT:
					name = "float";
					break;
				case TYPE_STRING:
					name = "string";
					break;
			}
			
			return name;
		}
		
		bool isPrimitive() const
		{
			switch( mType )
			{
				case TYPE_BOOL:
				case TYPE_INTEGER:
				case TYPE_FLOAT:
				case TYPE_STRING:
					return true;
				default:
					return false;
			}
		}
		
		bool isInterface() const
		{
			return mType == TYPE_INTERFACE;
		}
		
		bool isArray() const
		{
			return mType == TYPE_ARRAY;
		}

		bool isEnum() const
		{
			return mType == TYPE_ENUM;
		}
		
		bool isStruct() const
		{
			return mType == TYPE_STRUCT;
		}

		bool isAlias() const
		{
			return mType == TYPE_ALIAS;
		}
		
	private:
		Type mType;
		const ClassInfo	*mClass;
		const Enumeration *mEnum;
		const Alias *mAlias;
		std::string mNamespace;
		std::string mFilename;
		int mLine;
	};
	
	
	class TypeManager
	{
	public:
		TypeManager() 
		{
			// add built-in types
			addType( new TypeInfo( TypeInfo::TYPE_BOOL ) );
			addType( new TypeInfo( TypeInfo::TYPE_INTEGER ) );
			addType( new TypeInfo( TypeInfo::TYPE_FLOAT ) );
			addType( new TypeInfo( TypeInfo::TYPE_STRING ) );			
			mTypes.reserve( 100 );
		}
		
		~TypeManager() {}
		
		const TypeInfo *findType( const std::string &name, const std::string &ns = "" )
		{
			std::string full_name;
			if ( ns.length() > 0 )
			{	
				full_name = ns;
				full_name.append( "::" );
				full_name.append( name );
			}
			else
				full_name = name;
			
			// First try to find this name in the current namespace.
			for ( int i = 0; i < mTypes.size(); i++ )
			{
				if ( mTypes[ i ]->getName() == full_name )
					return mTypes[ i ];
			}

			// if not found look in the global namespace 
			for ( int i = 0; i < mTypes.size(); i++ )
			{
				if ( mTypes[ i ]->getName() == name )
					return mTypes[ i ];
			}
			
			return NULL;
		}

		void addType( const TypeInfo *type )
		{
			mTypes.push_back( type );
		}
		
		void findTypesInFile( const std::string &filename, std::vector<SmartPtr<const TypeInfo> > &types )
		{
			for ( int i = 0; i < mTypes.size(); i++ )
			{
				if ( mTypes[ i ]->getFilename() == filename )
					types.push_back( mTypes[ i ] );
			}
		}
		
		void findTypesByNamespace( const std::string &ns, std::vector<SmartPtr<const TypeInfo> > &types )
		{
			for ( int i = 0; i < mTypes.size(); i++ )
			{
				if ( mTypes[ i ]->getNamespace() == ns )
					types.push_back( mTypes[ i ] );
			}
		}
		
		void dumpTypes()
		{
			for ( int i = 0; i < mTypes.size(); i++ )
			{
				if ( mTypes[ i ]->isStruct() )
				{	
					std::vector<const Field*> fields;
					printf( "Struct: %s\n", mTypes[ i ]->getName().c_str() );
					mTypes[ i ]->getClass()->getFields( fields );
					for ( int i = 0; i < fields.size(); i++ )
					{
						printf( "\tField: %s %s\n", fields[ i ]->getType()->getName().c_str(), fields[ i ]->getName().c_str() );
					}
				}
				else if ( mTypes[ i ]->isInterface() )
				{	
					std::vector<const Method*> methods;
					printf( "Interface: %s at %s:%d\n", mTypes[ i ]->getName().c_str(), mTypes[ i ]->getFilename().c_str(), mTypes[ i ]->getLineNumber() );
					mTypes[ i ]->getClass()->getAllMethods( methods );
					for ( int i = 0; i < methods.size(); i++ )
					{
						printf( "\tMethod: %s\n", methods[ i ]->getName().c_str() );
					}
				}
				else if ( mTypes[ i ]->isEnum() )
				{
					std::vector<std::string> values;
					printf( "Enum: %s\n", mTypes[ i ]->getName().c_str() );
					mTypes[ i ]->getEnum()->getElements( values );
					for ( int i = 0; i < values.size(); i++ )
					{
						printf( "\tValue: %s\n", values[ i ].c_str() );
					}
				}
				else
					printf( "Type: %s\n", mTypes[ i ]->getName().c_str() );
			}
		}
		
	private:
		std::vector<SmartPtr<const TypeInfo> > mTypes;
	};	
};

#endif // JHCOM_REFLECT_H_



