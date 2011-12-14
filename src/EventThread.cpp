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

#include "EventThread.h"
#include "Timer.h"
#include "logging.h"
#include "jh_memory.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

EventThread::EventThread( const char *name ) : 
	mThread( name == NULL ? "EventThread" : name, this, &EventThread::threadMain )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	mThread.Start();
}

EventThread::~EventThread()
{
	TRACE_BEGIN( LOG_LVL_INFO );
	EventDispatcher::sendEvent( jh_new Event( Event::kShutdownEventId, PRIORITY_HIGH ) );
	LOG_NOISE( "Wait for thread to die" );
	mThread.Join();
}

void EventThread::threadMain()
{
	TRACE_BEGIN( LOG_LVL_NOTICE );
	Event *ev = NULL;
	bool done = false;
	
	while( !done )
	{
		LOG_INFO( "Waiting Event" );

		ev = mQueue.WaitEvent();

		LOG_NOISE( "Got Event" );
		
		if ( ev == NULL )
			LOG_ERR_FATAL( "got a null event\n" );
		
		// not needed, at this time.
		//AutoLock a( mLock );
		
		done = handleEvent( ev );
	}
}

