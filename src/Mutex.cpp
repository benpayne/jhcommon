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

#include "Mutex.h"

#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

bool Mutex::mInited = false;

Mutex::Mutex( bool recursive )
	: mLockedBy( NULL ), mName( NULL ), mRecursive( recursive )
{
	create_lock();
}

Mutex::Mutex( const char *name, bool recursive )
	: mLockedBy( NULL ), mName( name ), mRecursive( recursive )
{
	create_lock();
}

Mutex::~Mutex()
{
	// Destroy the mutex
	int res = pthread_mutex_destroy( &mMutex );
	
	if ( res )
	{
		if ( res == EINVAL )
			LOG_ERR_FATAL("pthread_mutex_destroy() failed with EINVAL");
		else if ( res == EBUSY )
			LOG_ERR_FATAL("pthread_mutex_destroy() failed with EBUSY");
		else
			LOG_ERR_FATAL("pthread_mutex_destroy() failed with %d", res);
	}
}
	
void Mutex::Lock()
{
	TraceLock( "unknown", 0 );
}

void Mutex::Unlock()
{
	TraceUnlock( "unknown", 0 );
}
	
void Mutex::TraceLock( const char *file, int line )
{
	int res = pthread_mutex_lock( &mMutex );
	
	mLockedBy = Thread::GetCurrent();
	
	if ( res )
	{
		if ( res == EDEADLK )			
		{
			LOG_ERR_FATAL( "Lock %s already taken at %s:%d", 
						   mName, mLockFile, mLockLine );
		}
		else
		{
			LOG_ERR_FATAL( "Lock %s failed with error %d at %s:%d", 
						   mName, res, mLockFile, mLockLine );
		}
	}
	
	mLockFile = file;
	mLockLine = line;
}

void Mutex::TraceUnlock( const char *file, int line_num )
{
	mLockedBy = NULL;
	int res = pthread_mutex_unlock( &mMutex );
	
	if ( res )
	{
		LOG_ERR_FATAL( "Unlock lock %s that isn't locked at %s:%d "
					   "(held by %s@%s:%d) error %d", 
					   mName, file, line_num, getOwner(), 
					   mLockFile, mLockLine, res );
	}
}

bool Mutex::isLocked()
{
	return ( mLockedBy == Thread::GetCurrent() );
}

const char *Mutex::getOwner()
{
	if ( mLockedBy != NULL )
		return mLockedBy->GetName();
	else 
		return NULL;
}

#ifdef PLATFORM_DARWIN
#define JH_PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE
#define JH_PTHREAD_MUTEX_ERRORCHECK PTHREAD_MUTEX_ERRORCHECK
#else
#define JH_PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#define JH_PTHREAD_MUTEX_ERRORCHECK PTHREAD_MUTEX_ERRORCHECK_NP
#endif

void Mutex::create_lock()
{
	int res = 0;

	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	if ( mRecursive )
		res = pthread_mutexattr_settype( &attr, JH_PTHREAD_MUTEX_RECURSIVE );
	else
		res = pthread_mutexattr_settype( &attr, JH_PTHREAD_MUTEX_ERRORCHECK );
	
	// Initialize the mutex in the default state 
	res = pthread_mutex_init( &mMutex, &attr );

	if ( res )
	{
		if ( res == EAGAIN )
			LOG_ERR_FATAL("pthread_mutex_init() failed with EAGAIN");
		else if ( res == EBUSY )
			LOG_ERR_FATAL("pthread_mutex_init() failed with EBUSY");
		else if ( res == EINVAL )
			LOG_ERR_FATAL("pthread_mutex_init() failed with EINVAL");
		else
			LOG_ERR_FATAL("pthread_mutex_init() failed with %d", res);
	}

	res = pthread_mutexattr_destroy( &attr );

	if ( res )
	{
		if ( res == EINVAL )
			LOG_ERR_FATAL("pthread_mutexattr_destroy() failed with EINVAL");
		else
			LOG_ERR_FATAL("pthread_mutexattr_destroy() failed with %d", res);
	}
}


AutoLock::AutoLock( Mutex &m )
	: mMutex( m ), mFile( "AutoLock" ), mLine( 0 )
{
	mMutex.TraceLock( "AutoLock", 0 );
}

AutoLock::AutoLock( Mutex &m, const char *file, int line )
	: mMutex( m ), mFile( file ), mLine( line )
{
	mMutex.TraceLock( file, line );
}

AutoLock::~AutoLock()
{
	mMutex.TraceUnlock( mFile, mLine );
}

void AutoLock::Lock()
{
	mMutex.Lock();
}

void AutoLock::Unlock()
{
	mMutex.Unlock();
}
