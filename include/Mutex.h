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

#ifndef _JH_MUTEX_H_
#define _JH_MUTEX_H_

#include <pthread.h>
#include "jh_types.h"
#include "Thread.h"

#define DebugLock()		TraceLock( __FILE__, __LINE__ )
#define DebugUnlock()	TraceUnlock( __FILE__, __LINE__ )

#define DebugAutoLock( mutex )	AutoLock _auto_lock_( mutex, __FILE__, __LINE__ )

/**
 *	The Mutex object allows locking of critical sections of code to prevent
 *	concurrency errors.
 */
class Mutex
{
 public:
	//! Create a new mutex (optionally recursive,  See pthread_mutex(3))
	Mutex( bool recursive = false );

	//! Create a named mutex object, nicer for error messages
	Mutex( const char *name, bool recursive = false );
	
	//! Clean up the mutex
	~Mutex();
	

	//! This method will lock this mutex, blocking until the lock is available
	void Lock();
	
	//! This method will unlock this mutex.
	void Unlock();
	
	//! Grab the lock, print out trace info if the grab fails
	void TraceLock( const char *file, int line );


	//! This method will unlock this mutex, print out trace info on error
	void TraceUnlock( const char *file, int line_num );

	//! Is this lock held by this thread?
	bool isLocked();
	
	//! What thread has locked this?
	const char *getOwner();
	
	/**
	 * Global intialization of Mutex objects, automatically handled
	 * elsewhere. you probably don't need to touch this.
	 */
	static void Init()
	{
		if ( not mInited )
		{
			mInited = true;
			pthread_mutex_init( &mCriticalSection, NULL );
		}
#if 0
		else
		{
			printf( "Don't call this init, it should only be called by jhcommon.\n" );
			abort();
		}
#endif
	}

	/**
	 * Global destruction of Mutex objects, automatically handled
	 * elsewhere. you probably don't need to touch this.
	 */
	static void Destroy()
	{
		pthread_mutex_destroy( &mCriticalSection );
	}

	/**
	 * @brief Grab the global lock.
	 *
	 * Mutex::mCriticalSection is used in places scattered around the
	 * codebase as a global lock.  It should NOT be held for longer
	 * than a couple of instructions.  
	 *
	 * @note Since this is used in exotic under-the-hood places like
	 * RefCount, it is strongly suggested that you not muck with this.
	 *
	 */
	static void EnterCriticalSection()
	{
		pthread_mutex_lock( &mCriticalSection );
	}
	
	/**
	 * @brief Release the global lock.
	 *
	 * Mutex::mCriticalSection is used in places scattered around the
	 * codebase as a global lock.  It should NOT be held for longer
	 * than a couple of instructions.  
	 *
	 * @note Since this is used in exotic under-the-hood places like
	 * RefCount, it is strongly suggested that you not muck with this.
	 *
	 */	
	static void ExitCriticalSection()
	{
		pthread_mutex_unlock( &mCriticalSection );
	}
	
 private:
	/**
	 * Create the internal lock that will be used.  Pay attention to
	 * the mRecursive member to determine which attributes to set.
	 */
	void create_lock();

	//! The lock for this particular Mutex object
	pthread_mutex_t	mMutex;
	
	//! Who lock this lock (or NULL if it is unlocked)
	Thread *mLockedBy;

	//! The name of this lock (if any)
	const char *mName;

	/**
	 * Is this a recursive lock?  (That is, can it be held repeatedly
	 * by the same thread, or will grabbing it twice by any thread
	 * cause an error?).
	 */
	bool mRecursive;

	/**
	 * If we are tracing, this is supposed to be the name of the file
	 * where the lock was last grabbed
	 */
	const char *mLockFile;

	/**
	 * If we are tracing, this is supposed to be the line number in
	 * the file where the lock was last grabbed.
	 */
	int mLockLine;
	
	//! The static lock used globally
	static pthread_mutex_t mCriticalSection;
	static bool mInited;
	
	friend class Condition;
};


//! A class that automatically locks and unlocks based on C++ scoping
class AutoLock
{
 public:
	//! Lock the provided mutex
	AutoLock( Mutex &m );

	//! Lock the provided mutex and update tracing information
	AutoLock( Mutex &m, const char *file, int line );
	
	//! Unlock the mutex when we go out of scope
	~AutoLock();

	//! Manually re-lock the mutex that we are wrapping
	void Lock();

	//! Manually unlock the mutex that we are wrapping
	void Unlock();
	
 private:
	//! The mutex we are holding
	Mutex &mMutex;

	//! if the mutex is locked
	bool mLocked;
	
	//! Filename for trace info
	const char *mFile;
	
	//! Line number for trace info
	int mLine;
};

//! Used to build an AutoLock out of anything that has Lock and Unlock methods
template<class T>
class GenericAutoLock
{
 public:
	//! Lock it and keep track of which lock we just locked.
	GenericAutoLock( T &m )
		: mMutex( m )
	{
		mMutex.Lock();
	}

	//! Going out of scope, unlock
	~GenericAutoLock()
	{
		mMutex.Unlock();
	}
	
	//! Manually relock it 
	void Lock()
	{
		mMutex.Lock();
	}
	
	//! Manually unlock it
	void Unlock()
	{
		mMutex.Unlock();
	}
	
 private:
	//! The generic lock we are wrapping
	T &mMutex;
};

#endif // _JH_MUTEX_H_
