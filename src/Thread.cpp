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

/*
 * Base Thread Implementation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include "Thread.h"
#include "logging.h"
#include "jh_memory.h"
#include "Mutex.h"
#include "GlobalConstructor.h"
#include "Timer.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

pthread_key_t Thread::mThreadKey;
bool Thread::mInited = false;

pthread_mutex_t Mutex::mCriticalSection;

//GLOBAL_CONSTRUCT_DESTRUCT( &Thread::Init, &Thread::Destroy );
GLOBAL_CONSTRUCT( &Thread::Init );

void *Thread::start_thread( void *arg )
{
	Thread *thread = (Thread*)arg;

	pthread_setspecific( mThreadKey, thread );

	TRACE_BEGIN( LOG_LVL_INFO );
	
	// The cancel type was set to async to support hard_stop. No longer need 
	//  and causes problems.
	//pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );	
	
	LOG( "started pid %d tid %d", getpid(), (long)pthread_self() ); 
	thread->OnStartThread();
	thread->OnStart();
	
	return NULL;
}

Thread::Thread( const char *name, int priority ) : mJoined( false ), 
	mSystemThread( false )
{
	strncpy( mName, name, kThreadNameLen );
	mName[ kThreadNameLen - 1 ] = '\0';
	mPrio = priority;
}

/*
 * This constructor is only used by GetCurrent.  When GetCurrent fails to get
 *  the Thread * from pthread_getspecific it will assume this is a thread not
 *  create by this thread class (aka a system thread) and use this constructor
 *  to create a Thread class for it.  When a system thread exits pthreads will
 *  call the thread_key_desctructor.  If this is a system thread the destructor
 *  can destroy the Thread class.  However if it is a user thread 
 *  (aka non-system thread) we need to wait for the user to destroy.
 */
Thread::Thread( pthread_t thread, const char *name ) : mJoined( false ),
	mSystemThread( true )
{
	if ( name == NULL )
		snprintf( mName, kThreadNameLen, "t0x%lx", thread );
	else
	{
		strncpy( mName, name, kThreadNameLen );
		mName[ kThreadNameLen - 1 ] = '\0';
	}
	
	mSystemThread = true;
	
	pthread_setspecific( mThreadKey, this );	
}

/*
 * Only join if this is a user thread.
 */
Thread::~Thread()
{
	if ( not mSystemThread )
		Join();
}

void Thread::Init()
{
#ifdef GCHEAP_ENABLED
	GCHeap::InitCheck();
#endif	
	
	if ( !mInited )
	{
		if ( pthread_key_create( &mThreadKey, &thread_key_destructor ) != 0 )
			LOG_ERR( "Failed to create pthread key" );
	
		mInited = true;
	
		jh_new Thread( pthread_self(), "main" );  // known to leak...
		Mutex::Init();
	}	
}

void Thread::Destroy()
{
	TRACE_BEGIN( LOG_LVL_NOTICE );
	
	mInited = false;
	
	//delete Thread::GetCurrent();
	Mutex::Destroy();
	TimerManager::destroyManager();
	
#ifdef GCHEAP_ENABLED
	GCHeap::defaultHeap->~GCHeap();
	GCHeap::defaultHeap = NULL;
#endif
}


/*
 * If this is a system thread delete it since we created in it GetCurrent and
 *  the use is not tracking it.
 */
void Thread::thread_key_destructor( void *arg )
{
	Thread *t = reinterpret_cast<Thread*>( arg );
	if ( t->mSystemThread )
	{
		delete t;
	}
}

/*
 * Try a get the thread class from the "key" by calling pthread_getspecific.
 *  If NULL is returned then this thread was not create by this class and is 
 *  therefore a system thread.  In that case we'll create a instace of thread
 *  to hold it's name and allow the caller to use it like any other thread.
 * This thread will be cleaned up when the thread exits.
 */
Thread *Thread::GetCurrent()
{
	Thread *t = reinterpret_cast<Thread*>( pthread_getspecific( mThreadKey ) );
	
	if ( t == NULL )
	{
		t = jh_new Thread( pthread_self(), NULL );
	}
	
	return t;
}

/*
 * This is really not needed since Thread * will be unique, but it could be 
 *  useful later.
 */
bool Thread::operator==( const Thread &t )
{
	return mThread == t.mThread;
}

/*
 * Don't join twice to the same thread, we will silently ignore that.  
 */
int Thread::Join()
{
	int result = 0;
	
	if ( !mJoined )
	{
		if (*this == *GetCurrent())
		{
			LOG_ERR_FATAL("Thread %s attempting to join self: THIS WOULD DEADLOCK",
						  GetName());
			return -1;
		}
		
		result = pthread_join( mThread, NULL );
		
		if ( result == 0 )
			mJoined = true;
		else
			LOG_ERR("pthread_join(%s) FAILED with error %d", GetName(), result);
	}
	
	return result;
}

void Thread::Exit()
{
	pthread_exit( NULL );
}

int32_t Thread::Start()
{
	TRACE_BEGIN( LOG_LVL_INFO );

	int32_t rc = 0;
	pthread_attr_t attrs;
	struct sched_param prio;
	
	prio.sched_priority = sched_get_priority_max( SCHED_RR );
	
	LOG_INFO( "Creating Thread %s", GetName() );
	
	pthread_attr_init( &attrs );
	
	if ( mPrio > 0 && geteuid() == 0 )
	{
		pthread_attr_setschedpolicy( &attrs, SCHED_RR );
		pthread_attr_setschedparam( &attrs, &prio );
	}
	
	rc = pthread_create( &mThread, &attrs, start_thread, (void *)this );
	if ( rc != 0 )
	{
		LOG_ERR( "Error from pthread_create() is [%d]", rc );
	}
	
	LOG( "Thread create %d", rc );
	
	return rc;
}

void Thread::Stop()
{
	OnStop();
}

static const char gUnknownThread[] = "Unknown";

const char *GetThreadName()
{
	Thread *t = Thread::GetCurrent();
	
	if ( t == NULL )
		return gUnknownThread;
	
	return t->GetName();
}

void Thread::OnStop()
{
	TRACE_BEGIN( LOG_LVL_INFO );	
	pthread_cancel( mThread );
}

