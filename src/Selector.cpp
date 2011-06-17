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

#include "jh_types.h"

#include "Selector.h"

#include "logging.h"
#include "jh_memory.h"

#include <unistd.h>
#include <fcntl.h>

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

Selector::Selector( const char *name ) : mLock( true ), 
	mThread( name == NULL ? "Selector" : name, this, &Selector::threadMain ),
	mUpdateFds( false )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	int res = pipe( mPipe );
	
	LOG( "pipe reader %d writer %d", mPipe[ PIPE_READER ], mPipe[ PIPE_WRITER ] );
	
	if ( res != 0 )
		LOG_ERR_FATAL( "failed to create pipe" );	

	mRunning = true;
	mThread.Start();
	mShutdown = false;
}

Selector::~Selector()
{
	TRACE_BEGIN( LOG_LVL_INFO );

	shutdown();
	
	LOG( "Closing pipes" );
	// close the pipes fd's.
	close( mPipe[ PIPE_WRITER ] );
	close( mPipe[ PIPE_READER ] );

	if ( mThread == *Thread::GetCurrent() )
		LOG_ERR_FATAL( "A selector MUST NOT be deleted by its own thread!" );
}

void Selector::shutdown()
{
	if ( not mShutdown )
	{
		mShutdown = true;
		EventDispatcher::sendEvent( jh_new Event( Event::kShutdownEventId,
												  PRIORITY_HIGH ) );

		// Selectors should not be shutdown by their own threads.
		// This is dangerous.
		if ( mThread == *Thread::GetCurrent() )
		{
			LOG_WARN( "%s declining to call Join() on %s",
					  Thread::GetCurrent()->GetName(),
					  mThread.GetName() );
		}
		else
		{
			LOG_NOISE( "%s waiting for thread %s to die",
					   Thread::GetCurrent()->GetName(),
					   mThread.GetName() );
			mThread.Stop();
			mThread.Join();
		}
	}
}

void Selector::addListener( int fd, short events, SelectorListener *listener, jh_ptr_int_t private_data )
{
	TRACE_BEGIN( LOG_LVL_INFO );

	ListenerNode *node = jh_new ListenerNode();
	
	node->mFd = fd;
	node->mEvents = events;
	node->mListener = listener;
	node->mPrivateData = private_data;
	
	AutoLock l( mLock );
	
	mList.push_back( node );	
	updateListeners();	
	
	LOG( "added fd %d events %x, list size %d", fd, events, mList.size() );
}

void Selector::removeListener( int fd, SelectorListener *listener )
{
	TRACE_BEGIN( LOG_LVL_INFO );

	AutoLock l( mLock );
	ListenerNode n( fd, NULL );
	bool update = false;
	
	JetHead::list<ListenerNode*>::iterator i = mList.begin(); 
	while (i != mList.end())
	{
		if (*(*i) == n)
		{
			update = true;
			delete *i;
			i = i.erase();
		} 
		else 
		{
			++i;
		}
	}
		
	if ( update )
	{
		updateListeners();		
	}
}
	
Selector::ListenerNode *Selector::findListener( int fd, SelectorListener *listener )
{
	ListenerNode n( fd, listener );
	for (JetHead::list<ListenerNode*>::iterator i = mList.begin(); 
		 i != mList.end(); ++i)
	{
		if (*(*i) == n)
		{
			return *i;
		}
	}
		
	return NULL;
}

void Selector::updateListeners()
{
	TRACE_BEGIN( LOG_LVL_INFO );
	// if called from the selector, just set a flag to update the Fds list
	// otherwise send an event to update the Fds.  But we also need to wait
	// for this change to be made effective.  Otherwise we could get events
	// for old Fd on a new listener if the same Fd was used for the new file.
	if ( *Thread::GetCurrent() == mThread )
	{
		mUpdateFds = true;
	}
	else if ( mRunning )
	{
		Event *ev = jh_new Event( Event::kSelectorUpdateEventId );
		sendEvent( ev );
		mCondition.Wait( mLock );
	}
	// else threadMain has exited or is exiting so no nothing we are ending.
}
	
void Selector::threadMain()
{
	TRACE_BEGIN( LOG_LVL_INFO );
	bool gotEvent = false;
	struct pollfd	fds[ kMaxPollFds ];
	int				numFds = 0;
	
	fillPollFds( fds, numFds );
	
	// Event will be sent by EventThread on exit
	while( mRunning )
	{
		LOG( "%p on %d files", this, numFds );
		for ( int i = 1; i < numFds; i++ )
		{
			LOG( "%p fd %d", this, fds[ i ].fd );
		}
		
		// set errno to zero
		// this is being set to better monitor the behavior of poll on
		// the 7401 until poll gets fixed
		errno = 0;
		int res = 0;
		
		// test here to ensure that errno cannot be modified before it is tested.
		if ( ( res = poll( fds, numFds, -1 ) ) < 0 )
		{
			// for whatever reason, there are times when poll returns -1,
			// but doesn't set errno
			if (errno == 0)
				LOG_NOTICE( "Poll returned %d, but didn't set errno", res);
			else if (errno == EINTR)
				LOG_NOISE( "Poll was interrupted" );
			else
				LOG_ERR_PERROR( "Poll returned %d", res );
		}
		
		LOG( "%p woke up %d", this, res );
		
		if ( res > 0 )
		{
			LOG( "got %d from poll", res );
			
			for ( int i = 0; i < numFds; i++ )
			{
				if ( fds[ i ].fd == mPipe[ PIPE_READER ] )
				{
					LOG( "got %x on pipe %d", fds[ 0 ].revents, fds[ i ].fd );
					if ( fds[ i ].revents & POLLIN )
					{
						char buf[10];
						read( mPipe[ PIPE_READER ], &buf, 4 );
						
						// We need to handle events after we handle file
						// descriptor polls because one of the events
						// that we handle modifies the current list of
						// file descriptors for poll and we need to handle
						// any that occured before updating them.
						gotEvent = true;
					}			
					else if ( fds[ i ].revents & ( POLLHUP | POLLNVAL ) )
					{
						LOG_ERR_FATAL( "POLLHUP recieved on pipe" );
					}
				}
				else
				{
					LOG_NOISE( "got %x on fd %d", fds[ i ].revents, fds[ i ].fd );
					if ( fds[ i ].revents != 0 )
					{
						// if callListeners removes a listener we need to update 
						//  Fds.  However only set if callListeners returns true
						//  This should not be cleared since one of the listeners
						//  could have called removeListener and that call might 
						//  have set mUpdateFds
						if ( callListeners( fds[ i ].fd, fds[ i ].revents ) )
							mUpdateFds = true;
					}
				}
			}
		}

		// Now that file descriptors have been handled we can deal with
		// events if needed, including updating the poll file descriptor
		// list if it's been changed.
		if ( gotEvent )
		{
			Event *ev = mQueue.PollEvent();

			if ( ev == NULL )
			{
				LOG_WARN( "got NULL event" );
			}
			else if ( ev->getEventId() == Event::kSelectorUpdateEventId )
			{
				LOG( "got kSelectorUpdateEventId" );
				mUpdateFds = true;
				ev->Release();
			}
			else
			{
				LOG( "got event %d", ev->getEventId() );
				bool done = EventDispatcher::handleEvent( ev );
				if ( done )
					mRunning = false;
			}
			
			gotEvent = false;
		}
		
		if ( mUpdateFds )
		{
			fillPollFds( fds, numFds );
			mUpdateFds = false;
		}
		
		mCondition.Broadcast();
	}
	
	LOG_NOTICE( "Thread exiting" );
}

void Selector::fillPollFds( struct pollfd *fds, int &numFds )
{
	AutoLock m( mLock );
	TRACE_BEGIN( LOG_LVL_INFO );

	fds[ 0 ].fd = mPipe[ PIPE_READER ];
	fds[ 0 ].events = POLLIN;

	int j, i = 1;

	for (JetHead::list<ListenerNode*>::iterator listener = mList.begin(); 
		 listener != mList.end(); ++listener)
	{
		// If we already have this fd in our list, skip it
		for ( j = 1; j < i; j++ )
		{
			if ( fds[ j ].fd == (*listener)->mFd )
			{
				fds[ j ].events &= (*listener)->mEvents;
				break;
			}
		}
		
		if ( j == i )
		{
			fds[ i ].fd = (*listener)->mFd;
			fds[ i ].events = (*listener)->mEvents;
			i++;

			if ( i == kMaxPollFds )
			{
				numFds = kMaxPollFds;
				return;
			}
		}
	}
	
	numFds = i;
	
	for ( i = 0; i < numFds; i++ )
	{
		LOG_NOISE( "file entry %d: fd %d events %x", i, fds[ i ].fd, fds[ i ].events );
	}
	
	LOG( "poll on %d fds, size %d", numFds, mList.size() );
}

						
bool Selector::callListeners( int fd, uint32_t events )
{
	AutoLock l( mLock );
	TRACE_BEGIN( LOG_LVL_INFO );
	ListenerNode n( fd, NULL );
	bool result = false;
	
	JetHead::list<ListenerNode*>::iterator listener = mList.begin();
	while (listener != mList.end())
	{
		if (*(*listener) == n)
		{
			LOG( "got event %x %p", events, *listener );
		
			// We want to remove the listener from the list if 
			//  revents includes POLLHUP or POLLNVAL.  But we also
			//  need for the listener to know about these event(s).
			// A "good" listener could take care of this by 
			//  removing himself from the selector.  The lock has 
			//  been made recursive so this is possible without dead
			//  locking.  But a "bad" listener may ignore these 
			//  event entirly.  
			// So in the spirit of "doing the right thing".  We need
			//  to deal with both cases.  However once we 
			//  call the listener's callback, the listener itself
			//  might be destroyed.  But if POLLHUP or POLLNVAL
			//  was recieved and the listener has not been destroyed
			//  we must do it.
			
			// listener might be destroyed if POLLHUP or POLLNVAL
			//  events are recieved.  So we will get any data we 
			//  need from it now. 
			SelectorListener *interface = (*listener)->mListener;
			jh_ptr_int_t pd = (*listener)->mPrivateData;
		
			if ( events & ( POLLHUP | POLLNVAL ) )
			{	
				if ( events & POLLHUP )
					LOG_INFO( "POLLHUP recieved on fd = %d (%p)", fd, *listener );
				
				if ( events & POLLNVAL )
					LOG_WARN( "POLLNVAL recieved on fd = %d (%p)", fd, *listener );
								
				delete *listener;
				listener = listener.erase();

				result = true;
			}
			else
			{
				// Gets incremented by the "erase" in the other branch
				++listener;
			}
			
			if ( interface != NULL )
			{
				LOG_NOISE( "eventsCallback %p %d %d", interface, events, fd );
				interface->processFileEvents( fd, events, pd );
				LOG_NOISE( "eventsCallback done" );
			}
		} else {
			++listener;
		}
	}
	
	return result;
}

const Thread *Selector::getDispatcherThread()
{
	return &mThread;
}

void Selector::wakeThread()
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	char buf[] = "EVNT";
	int res = write( mPipe[ PIPE_WRITER ], &buf, 4 );

	if ( res != 4 )
		LOG_ERR( "write to pipe failed %d", res );
}


