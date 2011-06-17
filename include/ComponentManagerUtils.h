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

#ifndef COMPONENTMANAGERUTILS_H_
#define COMPONENTMANAGERUTILS_H_

#include "RefCount.h"
#include "jh_memory.h"

#ifndef _INCLUDING_LOGGING_IN_HEADER_
#define _INCLUDING_LOGGING_IN_HEADER_
//#warning Do not include logging.h in header files.
#endif
#include "logging.h"

namespace JHCOM
{
	class CreateInstanceHelper : public SmartPtrHelper
	{
	public:
		CreateInstanceHelper( const CID &cid, ErrorCode *err, bool fatal = false ) : 
			mCid( cid ), mError( err ), mFatal( fatal ) {}
		
		virtual ErrorCode operator()( const IID& iid, void** obj ) const	
		{
			IComponentManager *mgr = getComponentManager();
			ErrorCode err = mgr->CreateInstance( mCid, iid, obj );
			if ( mError != NULL )
				*mError = err;
			if ( mFatal && err != kNoError )
				LOG_ERR_FATAL( "Failed to CreateInstance cid %s iid %s", 
					mCid.toString(), iid.toString() );
			return err;
		}
		
	private:
		const CID &mCid;
		ErrorCode *mError;
		bool mFatal;
	};

	class GetServiceHelper : public SmartPtrHelper
	{
	public:
		GetServiceHelper( const CID &cid, ErrorCode *err, bool fatal = false ) : 
			mCid( cid ), mError( err ), mFatal( fatal ) {}
		
		virtual ErrorCode operator()( const IID& iid, void** obj ) const	
		{
			IComponentManager *mgr = getComponentManager();
			ErrorCode err = mgr->GetService( mCid, iid, obj );
			if ( mError != NULL )
				*mError = err;
			if ( mFatal && err != kNoError )
				LOG_ERR_FATAL( "Failed to CreateInstance cid %s iid %s", 
					mCid.toString(), iid.toString() );
			return err;
		}
		
	private:
		const CID &mCid;
		ErrorCode *mError;
		bool mFatal;
	};
	
	class QueryInterfaceHelper : public SmartPtrHelper
	{
	public:
		QueryInterfaceHelper( ISupports *obj, ErrorCode *err = NULL ) : mPtr( obj ), mError( err ) {}
		
		virtual ErrorCode operator()( const IID& iid, void** obj ) const
		{
			ErrorCode err;
			*obj = mPtr->QueryInterface( iid, &err );
			if ( mError != NULL )
				*mError = err;
			return err;
		}
		
	private:
		ISupports *mPtr;
		ErrorCode *mError;
	};
	
	/**
	 * Create an instance of a COM Object in a form that is assignable to a 
	 *  SmartPtr and does the "right" thing when it comes to reference counting.
	 */
	inline const CreateInstanceHelper doCreateInstance( const CID &cid, ErrorCode *err = NULL )
	{
		return CreateInstanceHelper( cid, err );
	}

	/**
	 * Create an instance of a COM Object in a form that is assignable to a 
	 *  SmartPtr and does the "right" thing when it comes to reference counting.
	 *  This function to cause the program to exit if it fails.
	 */
	inline const CreateInstanceHelper doCreateInstanceFatal( const CID &cid )
	{
		return CreateInstanceHelper( cid, NULL, true );
	}
	
	/**
	 * Get a reference to a COM Object in a form that is assignable to a 
	 *  SmartPtr and does the "right" thing when it comes to reference counting.
	 */
	inline const GetServiceHelper doGetService( const CID &cid, ErrorCode *err = NULL )
	{
		return GetServiceHelper( cid, err );
	}

	/**
	 * Get a reference to a COM Object in a form that is assignable to a 
	 *  SmartPtr and does the "right" thing when it comes to reference counting.
	 *  This function to cause the program to exit if it fails.
	 */
	inline const GetServiceHelper doGetServiceFatal( const CID &cid )
	{
		return GetServiceHelper( cid, NULL, true );
	}
	
	/**
	 * Calls QueryInterface on object and returns a reference to the object in a form that is assignable to a 
	 *  SmartPtr and does the "right" thing when it comes to reference counting.
	 */
	inline const QueryInterfaceHelper doQueryInterface( ISupports *obj, ErrorCode *err = NULL )
	{
		return QueryInterfaceHelper( obj, err );
	}

	/**
	 * This class is for createing the object.  Normally it would not be needed
	 *  however with this structure it is possible to specialize this templace 
	 *  class and provide parameters to your constructor.  This is done 
	 *  for APGChanInfo.
	 */
	template<class T>
	class GenericFactoryCreator
	{
	public:
		T *operator()() const { return jh_new T; }		
	};
	
	template<class T>
	class GenericFactory : public IFactory
	{
	public:
		ErrorCode CreateInstance( CID cid, IID iid, void **object )
		{
			SmartPtr<T> supports;
			ErrorCode result = kNoError;
			// this is used instead of new'ing T to allow for template 
			//  specialization to pass params to the contructor.
			GenericFactoryCreator<T>	creator;
			
			if ( cid == JHCOM_GET_CID( T ) )
			{
				supports = creator();
				*object = supports->QueryInterface( iid, &result );
			}
			else
			{
				LOG_WARN( "Asking for %s we are %s", cid.toString(), JHCOM_GET_CID( T ).toString() );
				result = kNoClass;
			}
			
			return result;		
		}
				
		JHCOM_DECL_ISUPPORTS1( IFactory )
	};
	
	class ModuleBase : public IModule
	{
	public:
		ModuleBase( CID cid ) : mCID( cid ) {}
				
		const char *getCID() { return mCID.toString(); }

		JHCOM_DECL_ISUPPORTS1( IModule )
		
	private:
		CID	mCID;
	};
	
};

#endif // COMPONENTMANAGERUTILS_H_
