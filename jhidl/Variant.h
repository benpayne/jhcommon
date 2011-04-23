#ifndef VARIANT_H_
#define VARIANT_H_

#include "JHCOM.h"
#include "jh_types.h"

class Variant
{
public:	
	enum Type {
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
	
	Variant( char v ) : mIID( NULL ) { mType = TYPE_CHAR; mData.mNum = v; }
	Variant( int16_t v ) : mIID( NULL ) { mType = TYPE_INT16; mData.mNum = v; }
	Variant( uint16_t v ) : mIID( NULL ) { mType = TYPE_UINT16; mData.mUNum = v; }
	Variant( int32_t v ) : mIID( NULL ) { mType = TYPE_INT32; mData.mNum = v; }
	Variant( uint32_t v ) : mIID( NULL ) { mType = TYPE_UINT32; mData.mUNum = v; }
	Variant( int64_t v ) : mIID( NULL ) { mType = TYPE_INT64; mData.mNum = v; }
	Variant( uint64_t v ) : mIID( NULL ) { mType = TYPE_UINT64; mData.mUNum = v; }
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
	
	char getChar() const { return mData.mNum; }
	int16_t getInt16() const { return mData.mNum; }
	uint16_t getUInt16() const { return mData.mUNum; }
	int32_t getInt32() const { return mData.mNum; }
	uint32_t getUInt32() const { return mData.mUNum; }
	int64_t getInt64() const { return mData.mNum; }
	uint64_t getUInt64() const { return mData.mUNum; }
	float getFloat() const { return mData.mFloat; }
	double getDouble() const { return mData.mDouble; }
	JHCOM::ISupports *getObject() { return (JHCOM::ISupports *)mData.obj; }
	const JHCOM::ISupports *getObject() const { return (JHCOM::ISupports *)mData.obj; }
	std::string &getString() { return mStr; }
	const std::string &getString() const { return mStr; }

	void setChar( char v ) { mData.mNum = v; }
	void setInt16( int16_t v ) { mData.mNum = v; }
	void setUInt16( uint16_t v ) { mData.mUNum = v; }
	void setInt32( int16_t v ) { mData.mNum = v; }
	void setUInt32( uint16_t v ) { mData.mUNum = v; }
	void setInt64( int16_t v ) { mData.mNum = v; }
	void setUInt64( uint16_t v ) { mData.mUNum = v; }
	void setFloat( float v ) { mData.mFloat = v; }
	void setDouble( double v ) { mData.mDouble = v; }
	void setString( const std::string &s ) { mStr = s; }

private:
	Type mType;
	const JHCOM::IID *mIID;
	std::string mStr;
	union {
		void *obj;
		int64_t mNum;
		uint64_t mUNum;
		float mFloat;
		double mDouble;
	} mData;
};

#endif // VARIANT_H_

