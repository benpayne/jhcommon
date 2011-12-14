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

#define JHCOM_DEFINE_IID( name ) static JHCOM::IID getIID() { return JHCOM::IID( name ); }
#define JHCOM_DEFINE_CID( name ) static JHCOM::CID getCID() { return JHCOM::CID( name ); }
#define JHCOM_GET_IID( _class ) _class::getIID()
#define JHCOM_GET_CID( _class ) _class::getCID()

#define JHCOM_INIT_ISUPPORTS	\
	mRefCnt = 0

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
	
	class ISupports
	{
	public:
		virtual int AddRef() = 0;
		virtual int Release() = 0;
		virtual ErrorCode QueryInterface( IID iid, void **interface ) = 0;
		JHCOM_DEFINE_IID( "ISupports" );
	protected:
		virtual ~ISupports() {}
	};
	
	class IFactory : public ISupports
	{
	public:
		virtual ErrorCode CreateInstance( CID cid, IID iid, void **object ) = 0;	
		JHCOM_DEFINE_IID( "IFactory" );

	protected:
		virtual ~IFactory() {}
	};
	
	class IComponentManager : public ISupports
	{
	public:
		virtual ErrorCode LoadLibrary( const char *name ) = 0;	
		
		virtual ErrorCode CreateInstance( CID cid, IID iid, void **object ) = 0;	
		virtual ErrorCode GetService( CID cid, IID iid, void **object ) = 0;	
	
		virtual ErrorCode AddService( CID cid, ISupports *service ) = 0;	
		virtual ErrorCode RemoveService( CID cid ) = 0;
		
		JHCOM_DEFINE_IID( "IComponentManager" );

	protected:
		virtual ~IComponentManager() {}
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

extern "C" JHCOM::ErrorCode JHCOM_RegisterServices( JHCOM::IComponentManager *mgr );

/**
 * Declare the reference count variable and the implementations of the
 * AddRef and QueryInterface methods.
 */

#define JHCOM_DECL_ISUPPORTS                              \
public:                                                   \
	JHCOM::ErrorCode QueryInterface( JHCOM::IID iid, void** object );  \
	int AddRef();                                         \
	int Release();                                        \
protected:                                                \
	int mRefCnt;                                          \
public:


/**
 * Use this macro to implement the AddRef method for a given <i>_class</i>
 * @param _class The name of the class implementing the method
 */
#define JHCOM_IMPL_ADDREF(_class)                                                \
int _class::AddRef()                                 \
{                                                                             \
	/*printf( "Addref %d->%d in %s\n", mRefCnt, mRefCnt+1, #_class );*/ 	\
	++mRefCnt;                                                                  \
	return mRefCnt;                                                             \
}

/**
 * Use this macro to implement the AddRef method for a given <i>_class</i>
 * @param _class The name of the class implementing the method
 */
#define JHCOM_IMPL_ADDREF_DEBUG(_class)                                                \
int _class::AddRef()                                 \
{                                                                             \
	printf( "Addref %d->%d in %s\n", mRefCnt, mRefCnt+1, #_class ); 	\
	++mRefCnt;                                                                  \
	return mRefCnt;                                                             \
}

/**
 * Use this macro to implement the AddRef method for a given <i>_class</i>
 * @param _class The name of the class implementing the method
 */
#define JHCOM_IMPL_ADDREF_TEMPLATE(_class,_params)                                                \
template<_params>									\
int _class::AddRef()                                 \
{                                                                             \
	/*printf( "Addref %d->%d in %s\n", mRefCnt, mRefCnt+1, #_class );*/ 	\
	++mRefCnt;                                                                  \
	return mRefCnt;                                                             \
}

#define JHCOM_DELETE( obj ) delete obj

/**
 * Use this macro to implement the Release method for a given
 * <i>_class</i>.
 * @param _class The name of the class implementing the method
 */
#define JHCOM_IMPL_RELEASE_WITH_DESTROY(_class, _destroy)                        \
int _class::Release()                                \
{                                                                             \
	/*printf( "Release %d->%d in %s\n", mRefCnt, mRefCnt-1, #_class );*/	 \
	--mRefCnt;                                                                  \
	if (mRefCnt == 0) {                                                         \
		_destroy;                                                                 \
		return 0;                                                                 \
	}                                                                           \
	return mRefCnt;                                                             \
}

/**
 * Use this macro to implement the Release method for a given
 * <i>_class</i>.
 * @param _class The name of the class implementing the method
 */
#define JHCOM_IMPL_RELEASE_DEBUG(_class)                        \
int _class::Release()                                \
{                                                                             \
	printf( "Release %d->%d in %s\n", mRefCnt, mRefCnt-1, #_class );	 \
	--mRefCnt;                                                                  \
	if (mRefCnt == 0) {                                                         \
		JHCOM_DELETE(this);                                                                 \
		return 0;                                                                 \
	}                                                                           \
	return mRefCnt;                                                             \
}

/**
 * Use this macro to implement the Release method for a given <i>_class</i>
 * @param _class The name of the class implementing the method
 *
 * A note on the 'stabilization' of the refcnt to one. At that point,
 * the object's refcount will have gone to zero. The object's
 * destructor may trigger code that attempts to QueryInterface() and
 * Release() 'this' again. Doing so will temporarily increment and
 * decrement the refcount. (Only a logic error would make one try to
 * keep a permanent hold on 'this'.)  To prevent re-entering the
 * destructor, we make sure that no balanced refcounting can return
 * the refcount to |0|.
 */
#define JHCOM_IMPL_RELEASE(_class) \
  JHCOM_IMPL_RELEASE_WITH_DESTROY(_class, JHCOM_DELETE(this))

  /**
 * Use this macro to implement the Release method for a given
 * <i>_class</i>.
 * @param _class The name of the class implementing the method
 */
#define JHCOM_IMPL_RELEASE_TEMPLATE(_class, _param)                        \
template<_param> \
int _class::Release()                                \
{                                                                             \
	/*printf( "Release %d->%d in %s\n", mRefCnt, mRefCnt-1, #_class );*/	 \
	--mRefCnt;                                                                  \
	if (mRefCnt == 0) {                                                         \
		JHCOM_DELETE(this);                                                                 \
		return 0;                                                                 \
	}                                                                           \
	return mRefCnt;                                                             \
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Some convenience macros for implementing QueryInterface
 */

/**
 * This implements query interface with two assumptions: First, the
 * class in question implements ISupports and its own interface and
 * nothing else. Second, the implementation of the class's primary
 * inheritance chain leads to its own interface.
 *
 * @param _class The name of the class implementing the method
 * @param _classiiddef The name of the #define symbol that defines the IID
 * for the class (e.g. JHCOM_ISUPPORTS_IID)
 */

#define JHCOM_IMPL_QUERY_HEAD(_class)                                            \
JHCOM::ErrorCode _class::QueryInterface( JHCOM::IID iid, void** object )      \
{                                                                             \
  JHCOM::ISupports* foundInterface;

#define JHCOM_IMPL_QUERY_HEAD_TEMPLATE(_class,_param)                                            \
template<_param> \
JHCOM::ErrorCode _class::QueryInterface( JHCOM::IID iid, void** object )      \
{                                                                             \
  JHCOM::ISupports* foundInterface;

#define JHCOM_IMPL_QUERY_BODY(_interface)                                        \
  if ( iid == _interface::getIID() )                                  \
    foundInterface = static_cast<_interface*>(this);                       \
  else

#define JHCOM_IMPL_QUERY_BODY_CONDITIONAL(_interface, condition)                 \
  if ( (condition) && iid == _interface::getIID() ))                    \
    foundInterface = static_cast<_interface*>(this);                       \
  else

#define JHCOM_IMPL_QUERY_BODY_AMBIGUOUS(_interface, _implClass)                  \
  if ( iid == _interface::getIID() )                                  \
    foundInterface = static_cast<_interface*>( static_cast<_implClass*> (this));       \
  else

#define JHCOM_IMPL_QUERY_TAIL_GUTS                                               \
    foundInterface = NULL;                                                       \
  JHCOM::ErrorCode status;                                                            \
  if ( !foundInterface )                                                      \
    status = JHCOM::kNoInterface;                                                  \
  else                                                                        \
    {                                                                         \
      foundInterface->AddRef();                                              \
      status = JHCOM::kNoError;                                                         \
    }                                                                         \
  *object = foundInterface;                                             \
  return status;                                                              \
}

#define JHCOM_IMPL_QUERY_TAIL_INHERITING(_baseclass)                             \
    foundInterface = 0;                                                       \
  JHCOM::ErrorCode status;                                                            \
  if ( !foundInterface )                                                      \
    status = _baseclass::QueryInterface(aIID, (void**)&foundInterface);       \
  else                                                                        \
  {                                                                         \
    foundInterface->AddRef();                                               \
    status = JHCOM::kNoError;                                                         \
  }                                                                         \
  *object = foundInterface;                                             \
  return status;                                                              \
}

#define JHCOM_IMPL_QUERY_TAIL(_supports_interface)                               \
  JHCOM_IMPL_QUERY_BODY_AMBIGUOUS(ISupports, _supports_interface)              \
  JHCOM_IMPL_QUERY_TAIL_GUTS


  /*
    This is the new scheme.  Using this notation now will allow us to switch to
    a table driven mechanism when it's ready.  Note the difference between this
    and the (currently) underlying JHCOM_IMPL_QUERY_INTERFACE mechanism.  You must
    explicitly mention |ISupports| when using the interface maps.
  */
#define JHCOM_INTERFACE_MAP_BEGIN(_implClass)      JHCOM_IMPL_QUERY_HEAD(_implClass)
#define JHCOM_INTERFACE_MAP_BEGIN_TEMPLATE(_implClass,_template)      JHCOM_IMPL_QUERY_HEAD_TEMPLATE(_implClass, _template)
#define JHCOM_INTERFACE_MAP_ENTRY(_interface)      JHCOM_IMPL_QUERY_BODY(_interface)
#define JHCOM_INTERFACE_MAP_ENTRY_CONDITIONAL(_interface, condition)             \
  JHCOM_IMPL_QUERY_BODY_CONDITIONAL(_interface, condition)
#define JHCOM_INTERFACE_MAP_ENTRY_AGGREGATED(_interface,_aggregate)              \
  JHCOM_IMPL_QUERY_BODY_AGGREGATED(_interface,_aggregate)

#define JHCOM_INTERFACE_MAP_END                    JHCOM_IMPL_QUERY_TAIL_GUTS
#define JHCOM_INTERFACE_MAP_ENTRY_AMBIGUOUS(_interface, _implClass)              \
  JHCOM_IMPL_QUERY_BODY_AMBIGUOUS(_interface, _implClass)
#define JHCOM_INTERFACE_MAP_END_INHERITING(_baseClass)                           \
  JHCOM_IMPL_QUERY_TAIL_INHERITING(_baseClass)
#define JHCOM_INTERFACE_MAP_END_AGGREGATED(_aggregator)                          \
  JHCOM_IMPL_QUERY_TAIL_USING_AGGREGATOR(_aggregator)

#define JHCOM_IMPL_QUERY_INTERFACE0(_class)                                      \
  JHCOM_INTERFACE_MAP_BEGIN(_class)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(JHCOM::ISupports)                                       \
  JHCOM_INTERFACE_MAP_END

#define JHCOM_IMPL_QUERY_INTERFACE1(_class, _i1)                                 \
  JHCOM_INTERFACE_MAP_BEGIN(_class)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(_i1)                                               \
    JHCOM_INTERFACE_MAP_ENTRY_AMBIGUOUS(JHCOM::ISupports, _i1)                        \
  JHCOM_INTERFACE_MAP_END

#define JHCOM_IMPL_QUERY_INTERFACE2(_class, _i1, _i2)                            \
  JHCOM_INTERFACE_MAP_BEGIN(_class)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(_i1)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i2)                                               \
    JHCOM_INTERFACE_MAP_ENTRY_AMBIGUOUS(JHCOM::ISupports, _i1)                        \
  JHCOM_INTERFACE_MAP_END

#define JHCOM_IMPL_QUERY_INTERFACE3(_class, _i1, _i2, _i3)                       \
  JHCOM_INTERFACE_MAP_BEGIN(_class)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(_i1)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i2)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i3)                                               \
    JHCOM_INTERFACE_MAP_ENTRY_AMBIGUOUS(JHCOM::ISupports, _i1)                        \
  JHCOM_INTERFACE_MAP_END

#define JHCOM_IMPL_QUERY_INTERFACE4(_class, _i1, _i2, _i3, _i4)                  \
  JHCOM_INTERFACE_MAP_BEGIN(_class)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(_i1)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i2)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i3)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i4)                                               \
    JHCOM_INTERFACE_MAP_ENTRY_AMBIGUOUS(JHCOM::ISupports, _i1)                        \
  JHCOM_INTERFACE_MAP_END

#define JHCOM_IMPL_QUERY_INTERFACE5(_class, _i1, _i2, _i3, _i4, _i5)             \
  JHCOM_INTERFACE_MAP_BEGIN(_class)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(_i1)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i2)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i3)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i4)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i5)                                               \
    JHCOM_INTERFACE_MAP_ENTRY_AMBIGUOUS(JHCOM::ISupports, _i1)                        \
  JHCOM_INTERFACE_MAP_END

#define JHCOM_IMPL_QUERY_INTERFACE6(_class, _i1, _i2, _i3, _i4, _i5, _i6)        \
  JHCOM_INTERFACE_MAP_BEGIN(_class)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(_i1)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i2)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i3)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i4)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i5)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i6)                                               \
    JHCOM_INTERFACE_MAP_ENTRY_AMBIGUOUS(JHCOM::ISupports, _i1)                        \
  JHCOM_INTERFACE_MAP_END

#define JHCOM_IMPL_QUERY_INTERFACE7(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7)   \
  JHCOM_INTERFACE_MAP_BEGIN(_class)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(_i1)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i2)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i3)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i4)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i5)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i6)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i7)                                               \
    JHCOM_INTERFACE_MAP_ENTRY_AMBIGUOUS(JHCOM::ISupports, _i1)                        \
  JHCOM_INTERFACE_MAP_END

#define JHCOM_IMPL_QUERY_INTERFACE8(_class, _i1, _i2, _i3, _i4, _i5, _i6,        \
                                 _i7, _i8)                                    \
  JHCOM_INTERFACE_MAP_BEGIN(_class)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(_i1)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i2)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i3)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i4)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i5)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i6)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i7)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i8)                                               \
    JHCOM_INTERFACE_MAP_ENTRY_AMBIGUOUS(JHCOM::ISupports, _i1)                        \
  JHCOM_INTERFACE_MAP_END

#define JHCOM_IMPL_QUERY_INTERFACE9(_class, _i1, _i2, _i3, _i4, _i5, _i6,        \
                                 _i7, _i8, _i9)                               \
  JHCOM_INTERFACE_MAP_BEGIN(_class)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(_i1)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i2)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i3)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i4)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i5)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i6)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i7)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i8)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i9)                                               \
    JHCOM_INTERFACE_MAP_ENTRY_AMBIGUOUS(JHCOM::ISupports, _i1)                        \
  JHCOM_INTERFACE_MAP_END

#define JHCOM_IMPL_QUERY_INTERFACE10(_class, _i1, _i2, _i3, _i4, _i5, _i6,       \
                                  _i7, _i8, _i9, _i10)                        \
  JHCOM_INTERFACE_MAP_BEGIN(_class)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(_i1)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i2)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i3)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i4)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i5)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i6)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i7)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i8)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i9)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i10)                                              \
    JHCOM_INTERFACE_MAP_ENTRY_AMBIGUOUS(JHCOM::ISupports, _i1)                        \
  JHCOM_INTERFACE_MAP_END

#define JHCOM_IMPL_QUERY_INTERFACE11(_class, _i1, _i2, _i3, _i4, _i5, _i6,       \
                                  _i7, _i8, _i9, _i10, _i11)                  \
  JHCOM_INTERFACE_MAP_BEGIN(_class)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(_i1)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i2)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i3)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i4)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i5)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i6)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i7)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i8)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i9)                                               \
    JHCOM_INTERFACE_MAP_ENTRY(_i10)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(_i11)                                              \
    JHCOM_INTERFACE_MAP_ENTRY_AMBIGUOUS(JHCOM::ISupports, _i1)                        \
  JHCOM_INTERFACE_MAP_END

/**
 * Declare that you're going to inherit from something that already
 * implements ISupports, but also implements an additional interface, thus
 * causing an ambiguity. In this case you don't need another mRefCnt, you
 * just need to forward the definitions to the appropriate superclass. E.g.
 *
 * class Bar : public Foo, public nsIBar {  // both provide ISupports
 * public:
 *   JHCOM_DECL_ISUPPORTS_INHERITED
 *   ...other nsIBar and Bar methods...
 * };
 */
#define JHCOM_DECL_ISUPPORTS_INHERITED                                           \
public:                                                                       \
  JHCOM::ErrorCode QueryInterface(REFNSIID aIID,                                    \
                            void** aInstancePtr);                             \
  int AddRef(void);                                         \
  int Release(void);                                        \

/**
 * These macros can be used in conjunction with JHCOM_DECL_ISUPPORTS_INHERITED
 * to implement the ISupports methods, forwarding the invocations to a
 * superclass that already implements ISupports.
 *
 * Note that I didn't make these inlined because they're virtual methods.
 */

#define JHCOM_IMPL_ADDREF_INHERITED(Class, Super)                                \
int Class::AddRef(void)                                  \
{                                                                             \
  return Super::AddRef();                                                     \
}                                                                             \

#define JHCOM_IMPL_RELEASE_INHERITED(Class, Super)                               \
int Class::Release(void)                                 \
{                                                                             \
  return Super::Release();                                                    \
}                                                                             \

#define JHCOM_IMPL_QUERY_INTERFACE_INHERITED0(Class, Super)                      \
  JHCOM_IMPL_QUERY_HEAD(Class)                                                   \
  JHCOM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define JHCOM_IMPL_QUERY_INTERFACE_INHERITED1(Class, Super, i1)                  \
  JHCOM_IMPL_QUERY_HEAD(Class)                                                   \
  JHCOM_IMPL_QUERY_BODY(i1)                                                      \
  JHCOM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define JHCOM_IMPL_QUERY_INTERFACE_INHERITED2(Class, Super, i1, i2)              \
  JHCOM_IMPL_QUERY_HEAD(Class)                                                   \
  JHCOM_IMPL_QUERY_BODY(i1)                                                      \
  JHCOM_IMPL_QUERY_BODY(i2)                                                      \
  JHCOM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define JHCOM_IMPL_QUERY_INTERFACE_INHERITED3(Class, Super, i1, i2, i3)          \
  JHCOM_IMPL_QUERY_HEAD(Class)                                                   \
  JHCOM_IMPL_QUERY_BODY(i1)                                                      \
  JHCOM_IMPL_QUERY_BODY(i2)                                                      \
  JHCOM_IMPL_QUERY_BODY(i3)                                                      \
  JHCOM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define JHCOM_IMPL_QUERY_INTERFACE_INHERITED4(Class, Super, i1, i2, i3, i4)      \
  JHCOM_IMPL_QUERY_HEAD(Class)                                                   \
  JHCOM_IMPL_QUERY_BODY(i1)                                                      \
  JHCOM_IMPL_QUERY_BODY(i2)                                                      \
  JHCOM_IMPL_QUERY_BODY(i3)                                                      \
  JHCOM_IMPL_QUERY_BODY(i4)                                                      \
  JHCOM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define JHCOM_IMPL_QUERY_INTERFACE_INHERITED5(Class,Super,i1,i2,i3,i4,i5)        \
  JHCOM_IMPL_QUERY_HEAD(Class)                                                   \
  JHCOM_IMPL_QUERY_BODY(i1)                                                      \
  JHCOM_IMPL_QUERY_BODY(i2)                                                      \
  JHCOM_IMPL_QUERY_BODY(i3)                                                      \
  JHCOM_IMPL_QUERY_BODY(i4)                                                      \
  JHCOM_IMPL_QUERY_BODY(i5)                                                      \
  JHCOM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define JHCOM_IMPL_QUERY_INTERFACE_INHERITED6(Class,Super,i1,i2,i3,i4,i5,i6)     \
  JHCOM_IMPL_QUERY_HEAD(Class)                                                   \
  JHCOM_IMPL_QUERY_BODY(i1)                                                      \
  JHCOM_IMPL_QUERY_BODY(i2)                                                      \
  JHCOM_IMPL_QUERY_BODY(i3)                                                      \
  JHCOM_IMPL_QUERY_BODY(i4)                                                      \
  JHCOM_IMPL_QUERY_BODY(i5)                                                      \
  JHCOM_IMPL_QUERY_BODY(i6)                                                      \
  JHCOM_IMPL_QUERY_TAIL_INHERITING(Super)                                        \

#define JHCOM_IMPL_QUERY_INTERFACE_TEMPLATE1(_class, _param, _i1)                                 \
  JHCOM_INTERFACE_MAP_BEGIN_TEMPLATE(_class,_param)                                              \
    JHCOM_INTERFACE_MAP_ENTRY(_i1)                                               \
    JHCOM_INTERFACE_MAP_ENTRY_AMBIGUOUS(JHCOM::ISupports, _i1)                        \
  JHCOM_INTERFACE_MAP_END

/**
 * Convenience macros for implementing all ISupports methods for
 * a simple class.
 * @param _class The name of the class implementing the method
 * @param _classiiddef The name of the #define symbol that defines the IID
 * for the class (e.g. JHCOM_ISUPPORTS_IID)
 */

#define JHCOM_IMPL_ISUPPORTS0_DEBUG(_class)                                            \
  JHCOM_IMPL_ADDREF_DEBUG(_class)                                                      \
  JHCOM_IMPL_RELEASE_DEBUG(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE0(_class)

#define JHCOM_IMPL_ISUPPORTS0(_class)                                            \
  JHCOM_IMPL_ADDREF(_class)                                                      \
  JHCOM_IMPL_RELEASE(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE0(_class)

#define JHCOM_IMPL_ISUPPORTS1_DEBUG(_class, _interface)                                \
  JHCOM_IMPL_ADDREF_DEBUG(_class)                                                      \
  JHCOM_IMPL_RELEASE_DEBUG(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE1(_class, _interface)

#define JHCOM_IMPL_ISUPPORTS1(_class, _interface)                                \
  JHCOM_IMPL_ADDREF(_class)                                                      \
  JHCOM_IMPL_RELEASE(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE1(_class, _interface)

#define JHCOM_IMPL_ISUPPORTS2_DEBUG(_class, _i1, _i2)                                  \
  JHCOM_IMPL_ADDREF_DEBUG(_class)                                                      \
  JHCOM_IMPL_RELEASE_DEBUG(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE2(_class, _i1, _i2)

#define JHCOM_IMPL_ISUPPORTS2(_class, _i1, _i2)                                  \
  JHCOM_IMPL_ADDREF(_class)                                                      \
  JHCOM_IMPL_RELEASE(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE2(_class, _i1, _i2)

#define JHCOM_IMPL_ISUPPORTS3(_class, _i1, _i2, _i3)                             \
  JHCOM_IMPL_ADDREF(_class)                                                      \
  JHCOM_IMPL_RELEASE(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE3(_class, _i1, _i2, _i3)

#define JHCOM_IMPL_ISUPPORTS4(_class, _i1, _i2, _i3, _i4)                        \
  JHCOM_IMPL_ADDREF(_class)                                                      \
  JHCOM_IMPL_RELEASE(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE4(_class, _i1, _i2, _i3, _i4)

#define JHCOM_IMPL_ISUPPORTS5(_class, _i1, _i2, _i3, _i4, _i5)                   \
  JHCOM_IMPL_ADDREF(_class)                                                      \
  JHCOM_IMPL_RELEASE(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE5(_class, _i1, _i2, _i3, _i4, _i5)

#define JHCOM_IMPL_ISUPPORTS6(_class, _i1, _i2, _i3, _i4, _i5, _i6)              \
  JHCOM_IMPL_ADDREF(_class)                                                      \
  JHCOM_IMPL_RELEASE(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE6(_class, _i1, _i2, _i3, _i4, _i5, _i6)

#define JHCOM_IMPL_ISUPPORTS7(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7)         \
  JHCOM_IMPL_ADDREF(_class)                                                      \
  JHCOM_IMPL_RELEASE(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE7(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7)

#define JHCOM_IMPL_ISUPPORTS8(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8)    \
  JHCOM_IMPL_ADDREF(_class)                                                      \
  JHCOM_IMPL_RELEASE(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE8(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8)

#define JHCOM_IMPL_ISUPPORTS9(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8,    \
                           _i9)                                               \
  JHCOM_IMPL_ADDREF(_class)                                                      \
  JHCOM_IMPL_RELEASE(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE9(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8, _i9)

#define JHCOM_IMPL_ISUPPORTS10(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8,   \
                            _i9, _i10)                                        \
  JHCOM_IMPL_ADDREF(_class)                                                      \
  JHCOM_IMPL_RELEASE(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE10(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8,   \
                            _i9, _i10)

#define JHCOM_IMPL_ISUPPORTS11(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8,   \
                            _i9, _i10, _i11)                                  \
  JHCOM_IMPL_ADDREF(_class)                                                      \
  JHCOM_IMPL_RELEASE(_class)                                                     \
  JHCOM_IMPL_QUERY_INTERFACE11(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8,   \
                            _i9, _i10, _i11)

#define JHCOM_IMPL_ISUPPORTS_INHERITED0(Class, Super)                            \
    JHCOM_IMPL_QUERY_INTERFACE_INHERITED0(Class, Super)                          \
    JHCOM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    JHCOM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

#define JHCOM_IMPL_ISUPPORTS_INHERITED1(Class, Super, i1)                        \
    JHCOM_IMPL_QUERY_INTERFACE_INHERITED1(Class, Super, i1)                      \
    JHCOM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    JHCOM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

#define JHCOM_IMPL_ISUPPORTS_INHERITED2(Class, Super, i1, i2)                    \
    JHCOM_IMPL_QUERY_INTERFACE_INHERITED2(Class, Super, i1, i2)                  \
    JHCOM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    JHCOM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

#define JHCOM_IMPL_ISUPPORTS_INHERITED3(Class, Super, i1, i2, i3)                \
    JHCOM_IMPL_QUERY_INTERFACE_INHERITED3(Class, Super, i1, i2, i3)              \
    JHCOM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    JHCOM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

#define JHCOM_IMPL_ISUPPORTS_INHERITED4(Class, Super, i1, i2, i3, i4)            \
    JHCOM_IMPL_QUERY_INTERFACE_INHERITED4(Class, Super, i1, i2, i3, i4)          \
    JHCOM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    JHCOM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

#define JHCOM_IMPL_ISUPPORTS_INHERITED5(Class, Super, i1, i2, i3, i4, i5)        \
    JHCOM_IMPL_QUERY_INTERFACE_INHERITED5(Class, Super, i1, i2, i3, i4, i5)      \
    JHCOM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    JHCOM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

#define JHCOM_IMPL_ISUPPORTS_INHERITED6(Class, Super, i1, i2, i3, i4, i5, i6)    \
    JHCOM_IMPL_QUERY_INTERFACE_INHERITED6(Class, Super, i1, i2, i3, i4, i5, i6)  \
    JHCOM_IMPL_ADDREF_INHERITED(Class, Super)                                    \
    JHCOM_IMPL_RELEASE_INHERITED(Class, Super)                                   \

/**
 * This is for when you have a templated class implementing a COM interface.
 *  This will create template friendly version of QuertInterface, AddRef and
 *  Release.
 * 
 * Usage is:
 *  JHCOM_IMPL_QUERY_INTERFACE_TEMPLATE1( Foo<T>, class T, IFoo )
 */

#define JHCOM_IMPL_ISUPPORTS_TEMPLATE1(Class, Template, i1)                        \
    JHCOM_IMPL_QUERY_INTERFACE_TEMPLATE1(Class, Template, i1)                      \
    JHCOM_IMPL_ADDREF_TEMPLATE(Class, Template)                                    \
    JHCOM_IMPL_RELEASE_TEMPLATE(Class, Template)                                   \


#endif

