#ifndef JHCOM_REFLECT_H_
#define JHCOM_REFLECT_H_

#include <string>
#include <vector>
#include "RefCount.h"
#include <stdio.h>

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
			for ( unsigned i = 0; i < mParamTypes.size(); i++ )
			{
				params.push_back( &mParamTypes[ i ] );
			}
		}
		
		bool isAsync() const
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
		ClassInfo( const std::string &name ) : mName( name ), mConcrete( false ) {}
		
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
		
		unsigned	getNumberMethods() const
		{
			return mMethods.size();
		}
		
		const Method *getMethod( unsigned i ) const 
		{
			if ( i < mMethods.size() )
				return &mMethods[ i ];
			else
				return NULL;
		}
		
		const Method *getMethod( std::string name ) const
		{
			const Method *m = NULL;
			for( unsigned i = 0; i < mMethods.size(); i++ )
			{
				if ( mMethods[ i ].getName() == name )
					m = &mMethods[ i ];
			}
			
			for ( unsigned i = 0; m == NULL && i < mParents.size(); i++ )
			{
				m = mParents[ i ]->getMethod( name );
			}

			return m;
		}
		
		void getMethods( std::vector<const Method*> &methods ) const
		{
			for( unsigned i = 0; i < mMethods.size(); i++ )
			{
				methods.push_back( &mMethods[ i ] );
			}
		}

		void getAllMethods( std::vector<const Method*> &methods ) const
		{
			for ( unsigned i = 0; i < mParents.size(); i++ )
			{
				mParents[ i ]->getMethods( methods );
			}

			for( unsigned i = 0; i < mMethods.size(); i++ )
			{
				methods.push_back( &mMethods[ i ] );
			}
		}
		
		const Field *getField( std::string name ) const
		{
			const Field *f = NULL;
			for( unsigned i = 0; i < mFields.size(); i++ )
			{
				if ( mFields[ i ].getName() == name )
					f = &mFields[ i ];
			}
			
			return f;
		}
		
		void getFields( std::vector<const Field*> &fields ) const
		{
			for( unsigned i = 0; i < mFields.size(); i++ )
			{
				fields.push_back( &mFields[ i ] );
			}
		}
		
		void getParents( std::vector<const ClassInfo*> &parents ) const 
		{
			for( unsigned i = 0; i < mParents.size(); i++ )
			{
				parents.push_back( mParents[ i ] );
			}			
		}
		
		const std::string &getName() const
		{
			return mName;
		}
		
		bool isConcrete()
		{
			return mConcrete;
		}
		
		void setIID( std::string &str )
		{
			mIID = str;
		}
		
		const std::string &getIID() const
		{
			return mIID;
		}
		
	private:
		std::string mName;
		bool		mConcrete;
		std::vector<const ClassInfo *> mParents;
		std::vector<Method> mMethods;
		std::vector<Field> mFields;
		std::string	mIID;
	};
	
	class Alias
	{
	public:
		Alias( const TypeInfo *info ) : mType( info ) {}
		~Alias() {}
				
		const TypeInfo *getType() const
		{
			return mType;
		}
				
	private:
		SmartPtr<const TypeInfo> mType;
	};
	
	class Enumeration 
	{
	public:		
		Enumeration() {}
		~Enumeration() {}
		
		void addElement( const std::string &element )
		{
			mElements.push_back( element );
		}
				
		int getElement( const std::string &name ) const
		{
			for ( unsigned i = 0; i < mElements.size(); i++ )
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
			for ( unsigned i = 0; i < mElements.size(); i++ )
			{
				elements.push_back( mElements[ i ] );
			}
		}
		
	private:
		std::vector<std::string> mElements;
	};
	
	class TypeInfo : public RefCount
	{
	public:
		enum Type {
			TYPE_ANY,
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
		TypeInfo( const std::string &name, Type t, int bit_width = 32, bool _signed = true ) : 
			mName( name ), mType( t ), mClass( NULL ), mEnum( NULL ), 
			mAlias( NULL ), mBitWidth( bit_width ), mSigned( _signed ) 
		{
			switch( t )
			{
			case TYPE_BOOL:
				mBitWidth = 1;
				mSigned = false;
				break;
			
			case TYPE_INTEGER:
			case TYPE_FLOAT:
				break;
				
			default:
				mBitWidth = 0;
				break;
			}				
		}
			
		// Only valid when first param is array, second is what the array is of.
		//TypeInfo( Type type_array, TypeInfo &elem_type, int size = -1 ) : mType( t ) {}
		
		// if type is interface or struct, then ClassInfo describes that type.
		TypeInfo( const std::string &name, Type t, const ClassInfo *info ) : 
			mName( name ), mType( t ), mClass( info ), mEnum( NULL ), 
			mAlias( NULL ), mBitWidth( 0 ), mSigned( true ) {}
			
		// construct an enum with n elements
		TypeInfo( const std::string &name, const Enumeration *enumeration ) : 
			mName( name ), mType( TYPE_ENUM ), mClass( NULL ), 
			mEnum( enumeration ), mAlias( NULL ), mBitWidth( 16 ), 
			mSigned( false ) {}
			
		// construct an alias 
		TypeInfo( const std::string &name, const Alias *alias ) : 
			mName( name ), mType( TYPE_ALIAS ), mClass( NULL ), mEnum( NULL ), 
			mAlias( alias ), mBitWidth( 0 ), mSigned( true ) {} 
		
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
			return mName;
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

		bool isNumber() const
		{
			switch( mType )
			{
				case TYPE_BOOL:
				case TYPE_INTEGER:
				case TYPE_FLOAT:
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
		
		bool isString() const
		{
			return mType == TYPE_STRING;
		}

		int	getBitWidth() const
		{
			return mBitWidth;
		}
		
		bool isSigned() const
		{
			return mSigned;
		}
		
	protected:
		virtual ~TypeInfo() { delete mClass; delete mEnum;  delete mAlias; }

	private:
		std::string mName;
		Type mType;
		const ClassInfo	*mClass;
		const Enumeration *mEnum;
		const Alias *mAlias;
		int	mBitWidth;
		bool mSigned;
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
			addType( new TypeInfo( "bool", TypeInfo::TYPE_BOOL ) );
			addType( new TypeInfo( "char", TypeInfo::TYPE_INTEGER, 8 ) );
			addType( new TypeInfo( "int8_t", TypeInfo::TYPE_INTEGER, 8 ) );
			addType( new TypeInfo( "uint8_t", TypeInfo::TYPE_INTEGER, 8, false ) );
			addType( new TypeInfo( "int16_t", TypeInfo::TYPE_INTEGER, 16 ) );
			addType( new TypeInfo( "uint16_t", TypeInfo::TYPE_INTEGER, 16, false ) );
			addType( new TypeInfo( "int32_t", TypeInfo::TYPE_INTEGER, 32 ) );
			addType( new TypeInfo( "uint32_t", TypeInfo::TYPE_INTEGER, 32, false ) );
			addType( new TypeInfo( "int64_t", TypeInfo::TYPE_INTEGER, 64 ) );
			addType( new TypeInfo( "uint64_t", TypeInfo::TYPE_INTEGER, 64, false ) );
			addType( new TypeInfo( "float", TypeInfo::TYPE_FLOAT ) );
			addType( new TypeInfo( "double", TypeInfo::TYPE_FLOAT, 64 ) );
			addType( new TypeInfo( "long double", TypeInfo::TYPE_FLOAT, 128 ) );
			addType( new TypeInfo( "string", TypeInfo::TYPE_STRING ) );			
			addType( new TypeInfo( "any", TypeInfo::TYPE_ANY ) );			
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
			for ( unsigned i = 0; i < mTypes.size(); i++ )
			{
				if ( mTypes[ i ]->getName() == full_name )
					return mTypes[ i ];
			}

			// if not found look in the global namespace 
			for ( unsigned i = 0; i < mTypes.size(); i++ )
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
			for ( unsigned i = 0; i < mTypes.size(); i++ )
			{
				if ( mTypes[ i ]->getFilename() == filename )
					types.push_back( mTypes[ i ] );
			}
		}
		
		void findTypesByNamespace( const std::string &ns, std::vector<SmartPtr<const TypeInfo> > &types )
		{
			for ( unsigned i = 0; i < mTypes.size(); i++ )
			{
				if ( mTypes[ i ]->getNamespace() == ns )
					types.push_back( mTypes[ i ] );
			}
		}
		
		void loadClassInfo( const std::string &name );
		
		void dumpTypes()
		{
			for ( unsigned i = 0; i < mTypes.size(); i++ )
			{
				if ( mTypes[ i ]->isStruct() )
				{	
					std::vector<const Field*> fields;
					printf( "Struct: %s\n", mTypes[ i ]->getName().c_str() );
					mTypes[ i ]->getClass()->getFields( fields );
					for ( unsigned i = 0; i < fields.size(); i++ )
					{
						printf( "\tField: %s %s\n", fields[ i ]->getType()->getName().c_str(), fields[ i ]->getName().c_str() );
					}
				}
				else if ( mTypes[ i ]->isInterface() )
				{	
					std::vector<const Method*> methods;
					printf( "Interface: %s at %s:%d\n", mTypes[ i ]->getName().c_str(), mTypes[ i ]->getFilename().c_str(), mTypes[ i ]->getLineNumber() );
					mTypes[ i ]->getClass()->getAllMethods( methods );
					for ( unsigned i = 0; i < methods.size(); i++ )
					{
						printf( "\tMethod: %s\n", methods[ i ]->getName().c_str() );
					}
				}
				else if ( mTypes[ i ]->isEnum() )
				{
					std::vector<std::string> values;
					printf( "Enum: %s\n", mTypes[ i ]->getName().c_str() );
					mTypes[ i ]->getEnum()->getElements( values );
					for ( unsigned i = 0; i < values.size(); i++ )
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



