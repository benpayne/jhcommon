#ifndef JHCOM_IDL_H_
#define JHCOM_IDL_H_

#include "JHCOM.h"
#include "RefCount.h"
#include "jh_types.h"
#include <string>
#include <vector>

namespace JHCOM
{
	class Variant
	{
	public:	
		enum Type {
			TYPE_NONE,
			TYPE_CHAR,
			TYPE_INT16,
			TYPE_UINT16,
			TYPE_INT32,
			TYPE_UINT32,
			TYPE_INT64,
			TYPE_UINT64,
			TYPE_FLOAT,
			TYPE_DOUBLE,
			TYPE_STRING,
			TYPE_OBJECT,
		};
		
		Variant() : mIID( NULL ) { mType = TYPE_NONE; }
		Variant( char v ) : mIID( NULL ) { mType = TYPE_CHAR; mData.mChar = v; }
		Variant( int16_t v ) : mIID( NULL ) { mType = TYPE_INT16; mData.mNum16 = v; }
		Variant( uint16_t v ) : mIID( NULL ) { mType = TYPE_UINT16; mData.mUNum16 = v; }
		Variant( int32_t v ) : mIID( NULL ) { mType = TYPE_INT32; mData.mNum32 = v; }
		Variant( uint32_t v ) : mIID( NULL ) { mType = TYPE_UINT32; mData.mUNum32 = v; }
		Variant( int64_t v ) : mIID( NULL ) { mType = TYPE_INT64; mData.mNum64 = v; }
		Variant( uint64_t v ) : mIID( NULL ) { mType = TYPE_UINT64; mData.mUNum64 = v; }
		Variant( float v ) : mIID( NULL ) { mType = TYPE_FLOAT; mData.mFloat = v; }
		Variant( double v ) : mIID( NULL ) { mType = TYPE_DOUBLE; mData.mDouble = v; }
		Variant( const JHCOM::IID &iid, JHCOM::ISupports *v ) : mIID( NULL ) { mType = TYPE_OBJECT; mIID = &iid; mData.obj = new SmartPtr<JHCOM::ISupports>( v ); }
		Variant( const std::string &v ) : mIID( NULL ) { mType = TYPE_STRING; mStr = v; }
		~Variant()
		{
			if ( mType == TYPE_OBJECT )
			{
				SmartPtr<JHCOM::ISupports> *o = (SmartPtr<JHCOM::ISupports> *)mData.obj;
				delete o;
			}
		}
		
		Type getType() const { return mType; }
		const JHCOM::IID &getIID() const { return *mIID; }
		
		char getChar() const { return mData.mChar; }
		char &getCharRef() { return mData.mChar; }
		int16_t getInt16() const { return mData.mNum16; }
		int16_t &getInt16Ref() { return mData.mNum16; }
		uint16_t getUInt16() const { return mData.mUNum16; }
		uint16_t &getUInt16Ref() { return mData.mUNum16; }
		int32_t getInt32() const { return mData.mNum32; }
		int32_t &getInt32Ref() { return mData.mNum32; }
		uint32_t getUInt32() const { return mData.mUNum32; }
		uint32_t &getUInt32Ref() { return mData.mUNum32; }
		int64_t getInt64() const { return mData.mNum64; }
		int64_t &getInt64Ref() { return mData.mNum64; }
		uint64_t getUInt64() const { return mData.mUNum64; }
		uint64_t &getUInt64Ref() { return mData.mUNum64; }
		float getFloat() const { return mData.mFloat; }
		float &getFloatRef() { return mData.mFloat; }
		double getDouble() const { return mData.mDouble; }
		double &getDoubleRef() { return mData.mDouble; }
		JHCOM::ISupports *getObject() { return (JHCOM::ISupports *)mData.obj; }
		const JHCOM::ISupports *getObject() const { return (JHCOM::ISupports *)mData.obj; }
		std::string &getString() { return mStr; }
		const std::string &getString() const { return mStr; }
	
		void setChar( char v ) { mType = TYPE_CHAR; mData.mChar = v; }
		void setInt16( int16_t v ) { mType = TYPE_INT16; mData.mNum16 = v; }
		void setUInt16( uint16_t v ) { mType = TYPE_UINT16; mData.mUNum16 = v; }
		void setInt32( int32_t v ) { mType = TYPE_INT32; mData.mNum32 = v; }
		void setUInt32( uint32_t v ) { mType = TYPE_UINT32; mData.mUNum32 = v; }
		void setInt64( int64_t v ) { mType = TYPE_INT64; mData.mNum64 = v; }
		void setUInt64( uint64_t v ) { mType = TYPE_UINT64; mData.mUNum64 = v; }
		void setFloat( float v ) { mType = TYPE_CHAR; mData.mFloat = v; }
		void setDouble( double v ) { mType = TYPE_CHAR; mData.mDouble = v; }
		void setString( const std::string &s ) { mType = TYPE_CHAR; mStr = s; }
		void setObject( const JHCOM::IID &iid, JHCOM::ISupports *v ) 
		{ 
			mType = TYPE_OBJECT; 
			mIID = &iid; 
			mData.obj = new SmartPtr<JHCOM::ISupports>( v ); 
		}
	
	private:
		Type mType;
		const JHCOM::IID *mIID;
		std::string mStr;
		union {
			void *obj;
			char mChar;
			int16_t mNum16;
			uint16_t mUNum16;
			int32_t mNum32;
			uint32_t mUNum32;			
			int64_t mNum64;
			uint64_t mUNum64;
			float mFloat;
			double mDouble;
		} mData;
	};
	
	class IConnector
	{
	public:
		IConnector() {}
		
		virtual void pushChar( char num ) = 0;
		virtual void pushInt16( int16_t num ) = 0;
		virtual void pushUInt16( uint16_t num ) = 0;
		virtual void pushInt32( int32_t num ) = 0;
		virtual void pushUInt32( uint32_t num ) = 0;
		virtual void pushInt64( int64_t num ) = 0;
		virtual void pushUInt64( uint64_t num ) = 0;
		virtual void pushFloat( float num ) = 0;
		virtual void pushDouble( double num ) = 0;
		virtual void pushString( const std::string &str ) = 0;

		virtual char getParamChar( int param_num ) = 0;
		virtual int16_t getParamInt16( int param_num ) = 0;
		virtual uint16_t getParamUInt16( int param_num ) = 0;
		virtual int32_t getParamInt32( int param_num ) = 0;
		virtual uint32_t getParamUInt32( int param_num ) = 0;
		virtual int64_t getParamInt64( int param_num ) = 0;
		virtual uint64_t getParamUInt64( int param_num ) = 0;
		virtual float getParamFloat( int param_num ) = 0;
		virtual double getParamDouble( int param_num ) = 0;
		virtual std::string &getParamString( int param_num ) = 0;
		
		virtual void reset() = 0;
		virtual void call( int method_num ) = 0;
	};
	
	class VariantConnector : public IConnector
	{
	public:
		VariantConnector() {}
	
		void pushChar( char num )
		{
			mParams.push_back( Variant( num ) );
		}
		
		void pushUInt16( uint16_t num )
		{
			mParams.push_back( Variant( num ) );
		}
	
		void pushInt16( int16_t num )
		{
			mParams.push_back( Variant( num ) );
		}
	
		void pushUInt32( uint32_t num )
		{
			mParams.push_back( Variant( num ) );
		}
	
		void pushInt32( int32_t num )
		{
			mParams.push_back( Variant( num ) );
		}
		
		void pushUInt64( uint64_t num )
		{
			mParams.push_back( Variant( num ) );
		}
	
		void pushInt64( int64_t num )
		{
			mParams.push_back( Variant( num ) );
		}
	
		void pushFloat( float num )
		{
			mParams.push_back( Variant( num ) );
		}
	
		void pushDouble( double num )
		{
			mParams.push_back( Variant( num ) );
		}
		
		void pushString( const std::string &str )
		{
			mParams.push_back( Variant( str ) );		
		}
	
		void pushObject( const JHCOM::IID &iid, JHCOM::ISupports *v )
		{
			mParams.push_back( Variant( iid, v ) );		
		}
		
		char getParamChar( int param_num )
		{
			if ( param_num == -1 )
				return mReturnValue.getChar();
			else
				return mParams[ param_num ].getChar();
		}
	
		int16_t getParamInt16( int param_num )
		{
			if ( param_num == -1 )
				return mReturnValue.getInt16();
			else
				return mParams[ param_num ].getInt16();
		}
	
		uint16_t getParamUInt16( int param_num )
		{
			if ( param_num == -1 )
				return mReturnValue.getUInt16();
			else
				return mParams[ param_num ].getUInt16();
		}
	
		int32_t getParamInt32( int param_num )
		{
			if ( param_num == -1 )
				return mReturnValue.getInt32();
			else
				return mParams[ param_num ].getInt32();
		}
	
		uint32_t getParamUInt32( int param_num )
		{
			if ( param_num == -1 )
				return mReturnValue.getUInt32();
			else
				return mParams[ param_num ].getUInt32();
		}
		
		int64_t getParamInt64( int param_num )
		{
			if ( param_num == -1 )
				return mReturnValue.getInt64();
			else
				return mParams[ param_num ].getInt64();
		}
	
		uint64_t getParamUInt64( int param_num )
		{
			if ( param_num == -1 )
				return mReturnValue.getUInt64();
			else
				return mParams[ param_num ].getUInt64();
		}
	
		float getParamFloat( int param_num )
		{
			if ( param_num == -1 )
				return mReturnValue.getFloat();
			else
				return mParams[ param_num ].getFloat();
		}
	
		double getParamDouble( int param_num )
		{
			if ( param_num == -1 )
				return mReturnValue.getDouble();
			else
				return mParams[ param_num ].getDouble();
		}
		
		std::string &getParamString( int param_num )
		{
			if ( param_num == -1 )
				return mReturnValue.getString();
			else
				return mParams[ param_num ].getString();
		}
	
		JHCOM::ISupports *getParamObject( int param_num )
		{
			if ( param_num == -1 )
				return mReturnValue.getObject();
			else
				return mParams[ param_num ].getObject();
		}
		
		void reset()
		{
			mParams.clear();
		}
			
	protected:
		std::vector<Variant> mParams;
		Variant mReturnValue;	
	};
	
	class IHandler : public RefCount
	{
	public:
		virtual void call( int method_num, std::vector<Variant> &in_params, 
			Variant *ret_value = NULL ) = 0;
	};	
}

#endif // JHCOM_IDL_H_

