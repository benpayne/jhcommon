
#include "ITest.h"
#include "ComponentManagerUtils.h"
#include "RefCount.h"

#include "jh_memory.h"
#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

using namespace JHCOM;

class TestService : public ITest 
{
public:
	void PrintNumber( int num );
	void PrintName( std::string name );

protected:
	virtual ~TestService();
	
	JHCOM_DECL_ISUPPORTS
};

JHCOM_IMPL_ISUPPORTS1( TestService, ITest )

TestService::~TestService()
{
	printf( "Service destroyed\n" );
}

void TestService::PrintNumber( int num )
{
	printf( "number is %d\n", num );
}

void TestService::PrintName( std::string name )
{
	printf( "name is %s\n", name.c_str() );
}

#define mfunc( num ) virtual void stub##num() { printf( "stub" #num "\n" ); }

class NullStubs : public ISupports
{
public:
	NullStubs( ISupports *obj, IID &iid ) : mObj( obj ), mIID( iid ) {}
	
	mfunc( 0 );
	mfunc( 1 );
	mfunc( 2 );
	mfunc( 3 );
	mfunc( 4 );
	mfunc( 5 );

	JHCOM_DECL_ISUPPORTS
	
protected:
	virtual ~NullStubs() { printf( "destructor\n" ); }

	SmartPtr<ISupports> mObj;
	IID	mIID;
};

int NullStubs::AddRef()
{
	++mRefCnt;
	return mRefCnt;
}

int NullStubs::Release()
{
	--mRefCnt;
	if (mRefCnt == 0) 
	{
		delete this;
		return 0;
	}
	return mRefCnt;
}

ErrorCode NullStubs::QueryInterface( IID iid, void** object )
{
	ISupports* foundInterface;
	ErrorCode result = mObj->QueryInterface( iid, (void**)&foundInterface );
	
	if ( result == kNoError )
	{
		ISupports* stubs = jh_new NullStubs( foundInterface, iid );
		foundInterface->Release();
		stubs->AddRef();
		*object = stubs;
	}
	
	return result;
}

int main( int argc, char *argv[] )
{
	printf( "HelloWorld\n" );

	IComponentManager *mgr = getComponentManager();
	ITest *remote = jh_new TestService;
	NullStubs *stubs = jh_new NullStubs( remote, JHCOM_GET_IID( ITest ) );
	ITest *s = (ITest*)stubs;
	mgr->AddService( "TestService", s );
	
	ErrorCode result;

	SmartPtr<ITest> service = doGetService( "TestService", &result );
	
	if ( result == kNoError )
	{
		service->PrintNumber( 5 );
		service->PrintName( "Bar" );

	}
	else
		LOG_WARN( "Failed to get service %d", result );

	mgr->RemoveService( "TestService" );
	printf( "remove done\n" );
	
	if ( result == kNoError )
	{
		service->PrintName( "Done" );
		service = NULL;
	}
	
	printf( "end\n" );
	
	return 0;
}

