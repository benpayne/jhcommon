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

#ifndef JHCOM_H_
#define JHCOM_H_

#include <stdlib.h>

#define JHCOM_DEFINE_IID( name ) static JHCOM::IID getIID() { return JHCOM::IID( name ); }
#define JHCOM_DEFINE_CID( name ) static JHCOM::CID getCID() { return JHCOM::CID( name ); }
#define JHCOM_GET_IID( _class ) _class::getIID()
#define JHCOM_GET_CID( _class ) _class::getCID()

namespace JHCOM
{
	class ComId
	{
	public:
		ComId( const char *com_id ) : mId( com_id ) {}
		
		bool operator==( const ComId &other ) const;
		
		const char *toString() const { return mId; }
		
	private:
		const char *mId;	
	};

	typedef ComId CID;
	typedef ComId IID;

	enum ErrorCode
	{
		kNoError,
		kNoInterface,
		kNoClass,
		kNoFactory,
		kLoadFailed
	};
	
	/**
	 * ISupports is the base class for all COM objects implemented in C++. 
	 *  It's methods are pur virtual and will be implimented in the concrete 
	 *  class that impiments an interface.  The macros JHCOM_DECL_ISUPPORTSX
	 *  can be used in the derived concrete classes header file ti impliment 
	 *  these routines.
	 */
	class ISupports
	{
	public:
		virtual int AddRef() = 0;
		virtual int Release() = 0;
		virtual void *QueryInterface( IID iid, ErrorCode *err = NULL ) = 0;
		JHCOM_DEFINE_IID( "ISupports" );
	protected:
		virtual ~ISupports() {}
	};
	
	/**
	 * IFactory is the interface used by COM to create an istance of an object
	 *  Since we want the creator to not need compile time linkage, that means
	 *  we can't just "new" the object.  Typically you can impliment a factory
	 *  with GenericFactory (see ComponentManagerUtils.h)
	 */
	class IFactory : public ISupports
	{
	public:
		virtual ErrorCode CreateInstance( CID cid, IID iid, void **obj ) = 0;	
		JHCOM_DEFINE_IID( "IFactory" );

	protected:
		virtual ~IFactory() {}
	};

	/**
	 * IModule when dynamic code is loaded a module interface will be returned
	 *  this represents the set of COM code that is part of a shared lib.  This
	 *  interface will allow you to un-register the modules with the Component
	 *  Manager.  This will mark that module for unloading, when the modules 
	 *  reference count goes to zero.  It is generally recommended that COM 
	 *  objects addref the module that owns them when created and release the 
	 *  module when they are destroyed.  
	 */
	class IModule : public ISupports
	{
	public:
		virtual const char *getCID() = 0;
		virtual ErrorCode loadComponents() = 0;
		virtual ErrorCode unloadComponents() = 0;
		JHCOM_DEFINE_IID( "IModule" );

	protected:
		virtual ~IModule() {}
	};
		
	class IComponentManager : public ISupports
	{
	public:
		virtual ErrorCode LoadLibrary( const char *name ) = 0;	
		virtual ErrorCode RemoveModule( const char *name ) = 0;

		virtual IModule *GetModule( const char *name ) = 0;
		virtual ErrorCode UnloadLibrary( CID cid ) = 0;
		
		virtual ErrorCode CreateInstance( CID cid, IID iid, void **obj ) = 0;	
		virtual ErrorCode GetService( CID cid, IID iid, void **obj ) = 0;	
	
		virtual ErrorCode AddService( CID cid, ISupports *service ) = 0;	
		virtual ErrorCode RemoveService( CID cid ) = 0;
		
		JHCOM_DEFINE_IID( "IComponentManager" );

	protected:
		virtual ~IComponentManager() {}
	};

	class internal_refCount
	{
	public:
		internal_refCount( int c = 0 ) : mCount( c ) {}
		
		int AddRef() { return ++mCount; }
		int Release() { return --mCount; }
		
		operator int() { return mCount; }
	
	private:
		int mCount;
	};
	
	/**
	 * Called by anyone to get the pointer to the componentManager.
	 */
	IComponentManager *getComponentManager();

	/**
	 * should be calle by your main application just before exiting.
	 */
	void destroyComponentManager();
};

//extern "C" ErrorCode JHCOM_RegisterServices( JHCOM::IComponentManager *cm );
extern "C" JHCOM::IModule *JHCOM_GetModule();

#define JHCOM_DECL_ISUPPORTS_COMMON \
protected:                                                             \
	JHCOM::internal_refCount mRefCnt;                                  \
public:                                                                \
	int AddRef() { return mRefCnt.AddRef(); }                          \
	int Release() { int c = mRefCnt.Release(); if ( c == 0 ) delete this; return c; } \
	
//
// These macros define the guts of the query interface routines.
//

#define JHCOM_QUERY_INTERFACE_BEGIN                                     \
void *QueryInterface( JHCOM::IID iid, JHCOM::ErrorCode* err )           \
{                                                                       \
	JHCOM::ISupports* foundInterface;

#define JHCOM_QUERY_INTERFACE_BODY(_interface)                          \
	if ( iid == _interface::getIID() )                                  \
		foundInterface = static_cast<_interface*>(this);                \
	else

#define JHCOM_QUERY_INTERFACE_BODY_AMBIGUOUS(_interface, _implClass)    \
	if ( iid == _interface::getIID() )                                  \
		foundInterface = static_cast<_interface*>( static_cast<_implClass*> (this)); \
	else

#define JHCOM_QUERY_INTERFACE_END                                       \
		foundInterface = NULL;                                          \
	JHCOM::ErrorCode status;                                            \
	if ( !foundInterface )                                              \
		status = JHCOM::kNoInterface;                                   \
	else                                                                \
	{                                                                   \
		foundInterface->AddRef();                                       \
		status = JHCOM::kNoError;                                       \
	}                                                                   \
	if ( err != NULL )                                                  \
		*err = status;                                                  \
	return foundInterface;                                              \
}

//
// Macros to define the query interface routines for various numbers of classes.
//
#define JHCOM_IMPL_QUERY_INTERFACE0()                                    \
  JHCOM_QUERY_INTERFACE_BEGIN                                            \
    JHCOM_QUERY_INTERFACE_BODY(JHCOM::ISupports)                         \
  JHCOM_QUERY_INTERFACE_END

#define JHCOM_IMPL_QUERY_INTERFACE1(_i1)                                 \
  JHCOM_QUERY_INTERFACE_BEGIN                                            \
    JHCOM_QUERY_INTERFACE_BODY(_i1)                                      \
    JHCOM_QUERY_INTERFACE_BODY_AMBIGUOUS(JHCOM::ISupports, _i1)          \
  JHCOM_QUERY_INTERFACE_END
  
#define JHCOM_IMPL_QUERY_INTERFACE2(_i1, _i2)                            \
  JHCOM_QUERY_INTERFACE_BEGIN                                            \
    JHCOM_QUERY_INTERFACE_BODY(_i1)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i2)                                      \
    JHCOM_QUERY_INTERFACE_BODY_AMBIGUOUS(JHCOM::ISupports, _i1)                        \
  JHCOM_QUERY_INTERFACE_END

#define JHCOM_IMPL_QUERY_INTERFACE3(_i1, _i2, _i3)                       \
  JHCOM_QUERY_INTERFACE_BEGIN                                            \
    JHCOM_QUERY_INTERFACE_BODY(_i1)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i2)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i3)                                      \
    JHCOM_QUERY_INTERFACE_BODY_AMBIGUOUS(JHCOM::ISupports, _i1)          \
  JHCOM_QUERY_INTERFACE_END

#define JHCOM_IMPL_QUERY_INTERFACE4(_i1, _i2, _i3, _i4)                  \
  JHCOM_QUERY_INTERFACE_BEGIN                                            \
    JHCOM_QUERY_INTERFACE_BODY(_i1)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i2)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i3)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i4)                                      \
    JHCOM_QUERY_INTERFACE_BODY_AMBIGUOUS(JHCOM::ISupports, _i1)          \
  JHCOM_QUERY_INTERFACE_END

#define JHCOM_IMPL_QUERY_INTERFACE5(_i1, _i2, _i3, _i4, _i5)             \
  JHCOM_QUERY_INTERFACE_BEGIN                                            \
    JHCOM_QUERY_INTERFACE_BODY(_i1)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i2)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i3)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i4)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i5)                                      \
    JHCOM_QUERY_INTERFACE_BODY_AMBIGUOUS(JHCOM::ISupports, _i1)          \
  JHCOM_QUERY_INTERFACE_END

#define JHCOM_IMPL_QUERY_INTERFACE6(_i1, _i2, _i3, _i4, _i5, _i6)        \
  JHCOM_QUERY_INTERFACE_BEGIN                                            \
    JHCOM_QUERY_INTERFACE_BODY(_i1)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i2)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i3)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i4)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i5)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i6)                                      \
    JHCOM_QUERY_INTERFACE_BODY_AMBIGUOUS(JHCOM::ISupports, _i1)          \
  JHCOM_QUERY_INTERFACE_END

#define JHCOM_IMPL_QUERY_INTERFACE7(_i1, _i2, _i3, _i4, _i5, _i6, _i7)   \
  JHCOM_QUERY_INTERFACE_BEGIN                                            \
    JHCOM_QUERY_INTERFACE_BODY(_i1)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i2)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i3)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i4)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i5)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i6)                                      \
    JHCOM_QUERY_INTERFACE_BODY(_i7)                                      \
    JHCOM_QUERY_INTERFACE_BODY_AMBIGUOUS(JHCOM::ISupports, _i1)          \
  JHCOM_QUERY_INTERFACE_END

#define JHCOM_IMPL_QUERY_INTERFACE8(_i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8)  \
  JHCOM_INTERFACE_MAP_BEGIN                                                  \
    JHCOM_QUERY_INTERFACE_BODY(_i1)                                          \
    JHCOM_QUERY_INTERFACE_BODY(_i2)                                          \
    JHCOM_QUERY_INTERFACE_BODY(_i3)                                          \
    JHCOM_QUERY_INTERFACE_BODY(_i4)                                          \
    JHCOM_QUERY_INTERFACE_BODY(_i5)                                          \
    JHCOM_QUERY_INTERFACE_BODY(_i6)                                          \
    JHCOM_QUERY_INTERFACE_BODY(_i7)                                          \
    JHCOM_QUERY_INTERFACE_BODY(_i8)                                          \
    JHCOM_QUERY_INTERFACE_BODY_AMBIGUOUS(JHCOM::ISupports, _i1)              \
  JHCOM_QUERY_INTERFACE_END
  
//
// These macros define the query interface rountine for the class
//  AddRef and Release are now defined in the header.
//

#define JHCOM_DECL_ISUPPORTS0 \
	JHCOM_DECL_ISUPPORTS_COMMON \
	JHCOM_IMPL_QUERY_INTERFACE0 

#define JHCOM_DECL_ISUPPORTS1(_i1) \
	JHCOM_DECL_ISUPPORTS_COMMON \
	JHCOM_IMPL_QUERY_INTERFACE1(_i1) 

#define JHCOM_DECL_ISUPPORTS2(_i1,_i2) \
	JHCOM_DECL_ISUPPORTS_COMMON \
	JHCOM_IMPL_QUERY_INTERFACE1(_i1,_i2) 

#define JHCOM_DECL_ISUPPORTS3(_i1,_i2,_i3) \
	JHCOM_DECL_ISUPPORTS_COMMON \
	JHCOM_IMPL_QUERY_INTERFACE1(_i1,_i2,_i3) 

#define JHCOM_DECL_ISUPPORTS4(_i1,_i2,_i3,_i4) \
	JHCOM_DECL_ISUPPORTS_COMMON \
	JHCOM_IMPL_QUERY_INTERFACE1(_i1,_i2,_i3,_i4) 

#define JHCOM_DECL_ISUPPORTS5(_i1,_i2,_i3,_i4,_i5) \
	JHCOM_DECL_ISUPPORTS_COMMON \
	JHCOM_IMPL_QUERY_INTERFACE1(_i1,_i2,_i3,_i4,_i5) 

#define JHCOM_DECL_ISUPPORTS6(_i1,_i2,_i3,_i4,_i5,_i6) \
	JHCOM_DECL_ISUPPORTS_COMMON \
	JHCOM_IMPL_QUERY_INTERFACE1(_i1,_i2,_i3,_i4,_i5,_i6) 

#define JHCOM_DECL_ISUPPORTS7(_i1,_i2,_i3,_i4,_i5,_i6,_i7) \
	JHCOM_DECL_ISUPPORTS_COMMON \
	JHCOM_IMPL_QUERY_INTERFACE1(_i1,_i2,_i3,_i4,_i5,_i6,_i7) 

#define JHCOM_DECL_ISUPPORTS8(_i1,_i2,_i3,_i4,_i5,_i6,_i7,_i8) \
	JHCOM_DECL_ISUPPORTS_COMMON \
	JHCOM_IMPL_QUERY_INTERFACE1(_i1,_i2,_i3,_i4,_i5,_i6,_i7,_i8) 
	
#endif

