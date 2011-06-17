
#include "ITest.h"
#include "ComponentManagerUtils.h"
#include "RefCount.h"

#include "jh_memory.h"
#include "logging.h"
#include "Reflect.h"
#include "JHCOM_IDL.h"
#include "EventAgent.h"
#include "Selector.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

using namespace JHCOM;

TypeManager *gTypeMgr = NULL;

class EventConnector : public VariantConnector
{
public:
	EventConnector( IInvoker *invoker, IEventDispatcher *dispatcher ) : 
		mInvoker( invoker ), mDispatcher( dispatcher )
		{}
	
	~EventConnector()
	{
		delete mInvoker;
	}
	
	void call( int method_num, bool async )
	{
		mMethodNum = method_num;
		
		if ( async )
		{
			AsyncEventAgent0<EventConnector> *agent =
				jh_new AsyncEventAgent0<EventConnector>( this, &EventConnector::asyncEventHandler );
			agent->send( mDispatcher );
		}
		else
		{
			SyncEventAgent0<EventConnector> *agent =
				jh_new SyncEventAgent0<EventConnector>( this, &EventConnector::syncEventHandler );
			agent->send( mDispatcher );
		}
	}		
	
private:
	void syncEventHandler()
	{
		mInvoker->call( mMethodNum, mParams, &mReturnValue );
	}

	void asyncEventHandler()
	{
		mInvoker->call( mMethodNum, mParams, NULL );
	}
	
	IInvoker *mInvoker;
	IEventDispatcher *mDispatcher;
	int	mMethodNum;
};

class TestService : public ITest 
{
public:
	TestService() : mNum( 0 ) {}
	
	char getChar() { return '`'; }
	int16_t getNum16() { return -1024; }
	uint16_t getNumU16() { return 1024; }
	int32_t getNum32() { return -100000; }
	uint32_t getNumU32() { return 0xffffffff; }
	int64_t getNum64() { return -10000000000LL; }
	uint64_t getNumU64() { return 10000000000LL; }
	float getNumFloat() { return 1.2345; }
	double getNumDouble() { return 1.23456789; }
	std::string getString() { return "test"; }
	
	void Num16In( int16_t num ) 
	{ 
		if ( num != 10 ) 
			LOG_WARN( "failed to get number" ); 
	}
	
	void Num16Out( int16_t &num ) 
	{ 
		num = 20;
	}
	
	void Num16Inout( int16_t &num )
	{ 
		if ( num != 20 ) 
			LOG_WARN( "failed to get number" ); 
		num = 40;
	}

	void NumU16In( uint16_t num )
	{ 
		if ( num != 10 ) 
			LOG_WARN( "failed to get number" ); 
	}

	void NumU16Out( uint16_t &num )
	{ 
		num = 20;
	}

	void NumU16Inout( uint16_t &num )
	{ 
		if ( num != 20 ) 
			LOG_WARN( "failed to get number" ); 
		num = 40;
	}

	void StringIn( const std::string &name )
	{
		if ( name != "InTest" ) 
			LOG_WARN( "failed to get string" ); 
	}
	
	void StringOut( std::string &name )
	{
		name = "OutTest";
	}
	
	void StringInout( std::string &name )
	{
		if ( name != "OutTest" ) 
			LOG_WARN( "failed to get string" ); 

		name = "InoutTest";
	}
	
	JHCOM_DECL_ISUPPORTS1(ITest)

protected:
	virtual ~TestService();

private:
	int mNum;	
};

TestService::~TestService()
{
	LOG_NOTICE( "Service destroyed" );
}

int main( int argc, char *argv[] )
{
	gTypeMgr = new TypeManager();
	gTypeMgr->loadClassInfo( "ITest.jci" );
	
	IComponentManager *mgr = getComponentManager();
	ITest *remote = jh_new TestService;
	Selector thread("mythread");
	SmartPtr<IInvoker> invoker = ITest::GetInvoker( remote );
	EventConnector *connector = jh_new EventConnector( invoker, &thread );
	SmartPtr<ITest> stubs = ITest::GetStubs( connector );
	mgr->AddService( "TestService", stubs );
	stubs = NULL;
	
	ErrorCode result;

	SmartPtr<ITest> service = doGetService( "TestService", &result );
	
	if ( result == kNoError )
	{
		std::string name = "Bar";
		
		if ( service->getChar() != '`' )
			LOG_WARN( "failed to get char" );			
		if ( service->getNum16() != -1024 )
			LOG_WARN( "failed to get num" );			
		if ( service->getNumU16() != 1024 )
			LOG_WARN( "failed to get num" );			
		if ( service->getNum32() != -100000 )
			LOG_WARN( "failed to get num" );			
		if ( service->getNumU32() != 0xffffffff )
			LOG_WARN( "failed to get num" );			
		if ( service->getNum64() != -10000000000LL )
			LOG_WARN( "failed to get num" );			
		if ( service->getNumU64() != 10000000000LL )
			LOG_WARN( "failed to get num" );			
		if ( service->getNumFloat() != 1.2345 )
			LOG_WARN( "failed to get float" );			
		if ( service->getNumDouble() != 1.23456789 )
			LOG_WARN( "failed to get double" );			
		if ( service->getString() != "test" )
			LOG_WARN( "failed to get string" );			

		int16_t i = 10;
		uint16_t u = 10;
		
		service->Num16In( 10 );
		service->NumU16In( 10 );

		service->Num16Out( i );
		service->NumU16Out( u );

		if ( i != 20 || u != 20 )
			LOG_WARN( "failed to num out" );			

		service->Num16Inout( i );
		service->NumU16Inout( u );

		if ( i != 40 || u != 40 )
			LOG_WARN( "failed to num inout" );			

		std::string s;
		service->StringIn( "InTest" );
		service->StringOut( s );
		if ( s != "OutTest" )
			LOG_WARN( "failed to string out" );						
		service->StringInout( s );
		if ( s != "InoutTest" )
			LOG_WARN( "failed to string inout" );			
	}
	else
		LOG_WARN( "Failed to get service %d", result );

	mgr->RemoveService( "TestService" );
	printf( "remove done\n" );
	
	if ( result == kNoError )
	{
		std::string name = "Done";
		service->StringInout( name );
		service = NULL;
	}
	
	printf( "end\n" );
	
	return 0;
}

