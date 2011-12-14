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

#ifndef _JH_THREAD_H_
#define _JH_THREAD_H_

#include <pthread.h>
#include "jh_types.h"
//#include <features.h> non-posix header
#include <stdlib.h>

#ifdef __cplusplus

// This use of namespace was a HACK to get around an issue with DTV MW builds.
//  This hack will be removed soon and don't repeate this pattern is other code.
namespace JHThread {

class Thread
{
public:
	/**
	 * Create a thread. 
	 *
	 * @param name The threads name.
	 * @param priority 
	 */ 
	Thread( const char *name, int priority = 0 );

	/**
	 * Destroy a thread.  This cannot be called from the thread itself.  
	 */ 
	virtual ~Thread();
	
	 /**
	  * Start the thread executing.
	  */
	int32_t Start();
	
	/** 
	 * This will inform thread to stop.  If your class doesn't override OnStop 
	 *  this will cause the thread to exit immidatly by calling pthread_cancel.
	 */
	void Stop();
	
	/**
	 * Get a threads name.
	 */
	const char *GetName() { return mName; }

	/** 
	 * Block the calling thread until this thread exits.  
	 * Unless the underlying join fails, in which case, it returns the
	 * error code immediately, and proceeds, without waiting for the
	 * thread to join.
	 */
	int Join();

	/** 
	 * Test if two instances of the Thread class are the same thread.
	 */ 
	bool operator==( const Thread &t );
	
	/** 
	 * Exit the current running thread
	 */
	static void Exit();
	
	/** 
	 * Get the current running thread
	 */
	static Thread *GetCurrent();
	
	/** 
	 * Init the thread system.  Called one at startup before any threads are 
	 *  created.
	 */
	static void Init();

	/** 
	 * Destroy the thread system.  Called one at shutdown before after all 
	 *  threads are cleaned up.
	 */
	static void Destroy();
	
	/**
	 * DEPRACATED, use Start
	 */
	int32_t StartThread() { return Start(); }
	
	static const int kThreadNameLen = 32;
	
protected:
	/**
	 * Called from the context of the thread after it has been started.  When
	 *  this function exits the thread will stop and Join will return.
	 */
	virtual void OnStart() {}

	/**
	 * Called by Stop so that the thread can be informed to exit.  This differs
	 *  based on what the threar is doing.
	 */
	virtual void OnStop();

	/**
	 * DEPRACATED, impliment OnStart
	 */
	virtual void OnStartThread() {}
	
private:
	Thread( pthread_t thread, const char *name );

	char		mName[ kThreadNameLen ];
	pthread_t	mThread;
	int			mPrio;
	bool		mJoined;
	bool		mSystemThread;
	static pthread_key_t	mThreadKey;
	static bool				mInited;
	
	static void *start_thread( void *arg );
	static void thread_key_destructor( void *arg );
};


/**
 * This is a thread that will call a member function of your class as it's main. 
 *  The template parameter is the name of your class.
 */
template<class T>
class Runnable : public Thread
{
public:
	typedef void (T::*thread_main_type)();

	/**
	 * Construct a runnable.
	 *
	 * @param name The name of the thread.
	 * @param object The instance of T that we will call a function on as the main.
	 * @param thread_main The member function that will be the threads main.
	 * @param hard_stop Controls what happens when we call Stop of the thread.
	 *  If hard stop is true we will call pthread_cancel it kill the thread now.  
	 *  If hard stop is false, the defualt, we will set the stop flag that can be 
	 *  checked by calling CheckStop.
	 */
	Runnable( const char *name, T *object, thread_main_type thread_main, bool hard_stop = false, int priority = 0 ) : 
		Thread( name, priority ), mObject( object ), mFunc( thread_main ), mStop( false ), mHardStop( hard_stop ) 
		{
			if( hard_stop == true )
			{
				//Hard stop is being set. No longer supported. Please fix your code.
				abort();
			}
		}
	
	/**
	 * Check if stop has been called on a non-hard stop instance of Runnable.
	 */
	bool CheckStop() { return mStop; }

private:
	void OnStart() { (mObject->*mFunc)(); }
	void OnStop() { if ( mHardStop ) Thread::OnStop(); else mStop = true; }

	T*					mObject;
	thread_main_type	mFunc;	
	bool				mStop;
	bool				mHardStop;
};

} // namespace JHThread

// More of the namespace hack.  Don't repeat this pattern.  It may seem 
//  pointless to do this and for our code it is.  However this does cause the
//  linkers names to change so that we did conflict with someone elses "Thread"
//  class at dynamic link time.
using namespace JHThread;

#endif

__BEGIN_DECLS

const char *GetThreadName();

__END_DECLS

#endif // _JH_THREAD_H_
