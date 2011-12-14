/*
 * Copyright (c) 2010, JetHead Development, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the JetHead Development nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef REF_COUNT_H_
#define REF_COUNT_H_

#include "Mutex.h"
#include "JHCOM.h"

class RefCount
{
public:
	RefCount() : mRefCount( 0 ) {}
	
	void AddRef() const
	{ 
		Mutex::EnterCriticalSection();
		mRefCount++;
		//printf( "%p AddRef %d\n", this, mRefCount );
		Mutex::ExitCriticalSection();
	}
	
	void Release() const
	{
		Mutex::EnterCriticalSection();
		mRefCount--; 
		//printf( "%p Release %d\n", this, mRefCount );
		if ( mRefCount == 0 )
		{
			Mutex::ExitCriticalSection();
			onRefCountZero();
		}
		else
			Mutex::ExitCriticalSection();
	}
	
	int getRefCountForDebug() const { return mRefCount; }
 
protected:
	virtual void onRefCountZero() const { delete this; }
	virtual ~RefCount() {}

private:
	mutable int mRefCount;
};

class SmartPtrHelper
{
public:
	virtual JHCOM::ErrorCode operator()( const JHCOM::IID&, void** ) const { return JHCOM::kNoError; }
protected:
	virtual ~SmartPtrHelper() {}  // just for compile warning	
};

template<class T>
class SmartPtr
{
public:
	SmartPtr() : mObj( NULL ) {}
	SmartPtr( T *obj ) : mObj( obj ) { if ( mObj != NULL ) mObj->AddRef(); }
	SmartPtr( const SmartPtr &ptr ) : mObj( ptr.mObj ) { if ( mObj != NULL ) mObj->AddRef(); }
	SmartPtr( const SmartPtrHelper &helper ) : mObj( NULL ) { helper( JHCOM_GET_IID( T ), (void**)&mObj ); }
	~SmartPtr() { if ( mObj != NULL ) mObj->Release(); }

	SmartPtr &operator=( const SmartPtr &ptr ) 
	{
		// Use SmartPtr &operator=(T *obj) to handle the reference counting
		*this = ptr.mObj;
		return *this;
	}

	SmartPtr &operator=( const SmartPtrHelper &helper )
	{
		if ( mObj != NULL )
			mObj->Release();
		mObj = NULL;
		helper( JHCOM_GET_IID( T ), (void**)&mObj );
		return *this;
	}
	
	SmartPtr &operator=( T *obj )	
	{ 
		//printf( "SmartPtr = start\n" );
		if ( obj != NULL)
		{
			obj->AddRef();
		}

		if ( mObj != NULL )
			mObj->Release();
		mObj = obj;
		
		//printf( "SmartPtr = stop\n" );
		return *this;
	}
	
	bool operator==( const T *obj ) const { return ( mObj == obj ); }
	bool operator==( const SmartPtr &ptr ) const { return ( mObj == ptr.mObj ); }
	bool operator!=( const T *obj ) const { return ( mObj != obj ); }
	bool operator!=( const SmartPtr &ptr ) const { return ( mObj != ptr.mObj ); }
	
	T* operator->() { return mObj; }
	const T* operator->() const { return mObj; }
	
	operator T*() { return mObj; }
	operator const T*() const { return mObj; }
	
	const T* getObjectForDebug() const { return mObj; }
	
private:
	T *mObj;	
};

#endif // REF_COUNT_H_
