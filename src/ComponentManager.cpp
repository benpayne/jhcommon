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

#include "ComponentManager.h"
#include "logging.h"
#include "jh_memory.h"
#include <string.h>
#include <dlfcn.h>

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

JHCOM_IMPL_ISUPPORTS1( ComponentManager, IComponentManager )

ComponentManager::ComponentManager()
{
	JHCOM_INIT_ISUPPORTS;

	// AddRef component manager when it's created to avoid
	// accidental deletion from people storing the pointer
	// in a SmartPtr or otherwise AddRef/Releasing when they
	// are using the ComponentManager.
	AddRef();
}

ComponentManager::~ComponentManager()
{
	// release references to all objects.
	for (JetHead::list<ClassInfo*>::iterator i = mClasses.begin();
		 i != mClasses.end(); ++i)
	{
		(*i)->mClass->Release();
		delete *i;
	}
	// delete all the ClassInfo's
	mClasses.clear();
}
	
ErrorCode ComponentManager::LoadLibrary( const char *name )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	ErrorCode result = kNoError;	

	LOG_NOTICE( "Opening Library: %s", name );
	
	ErrorCode (*LoadLibrary)( IComponentManager *mgr );
	ErrorCode (*RegisterServices)( IComponentManager *mgr );
	void *handle = dlopen( name, RTLD_LAZY );
	
	if ( handle == NULL )
	{
		result = kLoadFailed;
		LOG_WARN( "Failed to open shared lib \"%s\": %s", name, dlerror() );
	}
	else
	{
		LoadLibrary = (ErrorCode (*)(IComponentManager *mgr))dlsym( handle, "JHCOM_LibraryEntry" );
		RegisterServices = (ErrorCode (*)(IComponentManager *mgr))dlsym( handle, "JHCOM_RegisterServices" );

		if ( RegisterServices == NULL || LoadLibrary == NULL )
		{
			result = kLoadFailed;
			LOG_WARN( "Failed to get symbol" );
		}
		else
		{
			LOG( "LoadLibrary is %p", LoadLibrary );
			LOG( "RegisterServices is %p", RegisterServices );
			result = LoadLibrary( this );
			
			if ( result == kNoError )
				result = RegisterServices( this );
		}
	}
		
	return result;
}

ErrorCode ComponentManager::CreateInstance( CID cid, IID iid, void **object )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	ErrorCode result = kNoError;	
	IFactory *factory;
	
	result = GetService( cid, IFactory::getIID(), (void**)&factory );
	
	if ( result == kNoError )
	{
		result = factory->CreateInstance( cid, iid, object );
		factory->Release();
	}
	else
		result = kNoFactory;

	return result;	
}

ErrorCode ComponentManager::GetService( CID cid, IID iid, void **object )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	ErrorCode result = kNoError;
	ClassInfo match( cid );

	ClassInfo *info = NULL;

	for (JetHead::list<ClassInfo*>::iterator i = mClasses.begin(); 
		 i != mClasses.end(); ++i)
	{
		if (*(*i) == match)
		{
			info = *i;
			break;
		}
	}
	
	LOG( "info is %p", info );
	
	if ( info != NULL )
		result = info->mClass->QueryInterface( iid, object );
	else
		result = kNoClass;

	return result;
}

ErrorCode ComponentManager::AddService( CID cid, ISupports *service )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	ClassInfo *info = jh_new ClassInfo( cid, service );
	
	LOG( "Adding cid %s, ISupports %p", cid.toString(), service );
	
	service->AddRef();
	
	mClasses.push_back( info );
	
	return kNoError;
}

ErrorCode ComponentManager::RemoveService( CID cid )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	ClassInfo match( cid );

	for (JetHead::list<ClassInfo*>::iterator i = mClasses.begin(); 
		 i != mClasses.end(); ++i)
	{
		if (*(*i) == match)
		{
			ClassInfo *info = *i;
			info->mClass->Release();
			i.erase();
			return kNoError;
		}
	}
	
	return kNoClass;

	
}

bool JHCOM::ComId::operator==( const ComId &other ) const 
{
	return strcmp( other.mId, mId ) == 0;
} 

static IComponentManager *gManager = NULL;

static bool gMaster = false;

/*
 * If someone calls getComponentManager and one does not exist then this 
 *  component is assumed to be the master for the process.  
 */ 
IComponentManager *JHCOM::getComponentManager()
{
	if ( gManager == NULL )
	{
		gMaster = true;
		gManager = jh_new ComponentManager;
	}
	
	return gManager;
}

void JHCOM::destroyComponentManager()
{
	// if it exits remote the reference added when constructed.
	if ( gManager != NULL )
		gManager->Release();
}

extern "C" ErrorCode JHCOM_LibraryEntry( IComponentManager *mgr )
{
	TRACE_BEGIN( LOG_LVL_NOTICE );
	
	if ( gMaster )
		LOG_ERR_FATAL( "Called on master" );
	
	gManager = mgr;
	
	return kNoError;
}
