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

#include "IFoo.h"
#include "jh_memory.h"
#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

//
//  FOO SERVICE
//

class FooService : public IFooService
{
public:
	FooService() {}
	virtual ~FooService();
	
	int Run();
	
	JHCOM_DECL_ISUPPORTS	
};

JHCOM_IMPL_ISUPPORTS1( FooService, IFooService )

FooService::~FooService()
{
	LOG_NOTICE( "FooService Dead" );
}

int FooService::Run()
{
	LOG_NOTICE( "RunFooService" );
	return 0;
}

//
// FOO
//

class Foo : public IFoo
{
public:
	Foo() {}
	virtual ~Foo();
	
	int Run();
	
	JHCOM_DECL_ISUPPORTS	
};

JHCOM_IMPL_ISUPPORTS1( Foo, IFoo )

Foo::~Foo()
{
	LOG_NOTICE( "Foo Dead" );
}

int Foo::Run()
{
	LOG_NOTICE( "RunFoo" );
	return 0;
}

//
// FOO FACTORY
//

class FooFactory : public IFactory
{
public:
	virtual ~FooFactory();
	ErrorCode CreateInstance( CID cid, IID iid, void **object );
	
	JHCOM_DECL_ISUPPORTS	
};

JHCOM_IMPL_ISUPPORTS1( FooFactory, IFactory )

FooFactory::~FooFactory()
{
	LOG_NOTICE( "FooFactory Dead" );
}

ErrorCode FooFactory::CreateInstance( CID cid, IID iid, void **object )
{
	TRACE_BEGIN( LOG_LVL_NOTICE );
	ISupports *supports = NULL;
	ErrorCode result = kNoError;
	
	if ( cid == "FooFactory" )
	{
		supports = jh_new Foo();
		result = supports->QueryInterface( iid, object );
	}
	else
	{
		result = kNoClass;
	}
	
	return result;
}

ErrorCode JHCOM_RegisterServices( IComponentManager *mgr )
{
	TRACE_BEGIN( LOG_LVL_NOTICE );
	
	FooService *foo = jh_new FooService;
	mgr->AddService( "FooService", foo );

	FooFactory *fac = jh_new FooFactory;
	mgr->AddService( "FooFactory", fac );
	
	return kNoError;
}
