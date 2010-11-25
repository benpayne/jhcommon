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

#ifndef LISTENER_CONTAINER_H
#define LISTENER_CONTAINER_H

#include "Mutex.h"
#include "jh_list.h"

template <class ListenerType>
class ListenerContainer
{
 public:
	~ListenerContainer()
	{
		mListenerList.clear();
	}
	
	class Invoker
	{
	public:
		virtual void operator()(ListenerType *listener) {} 
	protected:
		virtual ~Invoker() {}
	};

	// Add listener to container
	// NOTE:  Should not be called during handling of listener events.
	void addListener(ListenerType *listener)
	{
		AutoLock lock(mMutex);
		mListenerList.push_back(listener);
	}
	
	// Remove listener from container
	// NOTE:  Should not be called during handling of listener events.
	void removeListener(ListenerType *listener)
	{
		AutoLock lock(mMutex);
		
		// The "typename" is necessary here because templates
		// are dirty: the compiler is not yet "aware" of
		// list<T>::iterator as a class type, so in some
		// horrible sense the compiler thinks that
		// "JetHead::list<ListenerType*>::iterator might be a
		// full statement (or variable declaration) on its
		// own.  Adding the "typename" hint allows the compile
		// to realize that the next token is a type (and this to compile).
		for (typename JetHead::list< ListenerType *>::iterator i = mListenerList.begin();
			 i != mListenerList.end(); ++i)
		{
			ListenerType* lnode = *i;
			
			if (lnode == listener)
			{
				i.erase();
				return;
			}
		}
	}
	
	// Called to invoke all listeners using the Invoker specified
	void invokeListeners(Invoker &invoker)
	{
		AutoLock lock(mMutex);
		
		// See the big comment in removeListener
		for (typename JetHead::list< ListenerType *>::iterator i = mListenerList.begin(); 
			 i != mListenerList.end(); ++i)
		{
			ListenerType* lnode = *i;						
			invoker(lnode);		   
		}
	}
	
 private:
	JetHead::list<ListenerType *> mListenerList;
	Mutex	mMutex;
};

#endif // LISTENERCONTAINER_H
