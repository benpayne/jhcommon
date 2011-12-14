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

#include "ListenerContainer.h"
#include "List++.h"
#include "jh_memory.h"
#include "logging.h"

#include <stdlib.h>
#include <stdio.h>

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

class Foo;

class FooListener
{
public:
	virtual void onFoo(Foo *foo, int a) = 0;
	virtual void onBar(Foo *foo) = 0;
};

class Foo
{
public:
	Foo()
	{
		mListeners = jh_new ListenerContainer<FooListener>();
	}

	~Foo()
	{
		delete mListeners;
	}
	
	void addListener(FooListener *listener)
	{
		mListeners->addListener(listener);
	}
	
	void removeListener(FooListener *listener)
	{
		mListeners->removeListener(listener);
	}

	class onFooInvoker : public ListenerContainer<FooListener>::Invoker
	{
	public:
		onFooInvoker(Foo *foo, int a) : mFoo(foo), mVal(a) {}
		void operator()(FooListener *listener)
		{
			listener->onFoo(mFoo, mVal);
		}
	private:
		Foo *mFoo;
		int mVal;
	};
	
	class onBarInvoker : public ListenerContainer<FooListener>::Invoker
	{
	public:
		onBarInvoker(Foo *foo) : mFoo(foo) {}
		void operator()(FooListener *listener)
		{
			listener->onBar(mFoo);
		}
	private:
		Foo *mFoo;
	};
	
	void DoFoo(int x)
	{
		onFooInvoker invoker(this, x);
		mListeners->invokeListeners(invoker);
	}
	void DoBar()
	{
		onBarInvoker invoker(this);
		mListeners->invokeListeners(invoker);
	}
private:
	ListenerContainer<FooListener>	*mListeners;

};

class Cat : public FooListener
{
public:
	Cat(int value) : mValue(value) {}
	void onFoo(Foo *foo, int a)
	{
		LOG_NOTICE("Cat(%d) foo reports Foo(%p, %d)", mValue, foo, a);
	}
	void onBar(Foo *foo)
	{
		LOG_NOTICE("Cat(%d) foo reports Bar(%p)", mValue, foo);
	}
private:
	int mValue;
};


int main()
{
	Cat *kittens[4];
	Foo *foo = jh_new Foo();
	
	for (int i = 0; i < 4; ++i)
	{
		kittens[i] = jh_new Cat(i);
		foo->addListener(kittens[i]);
	}
	
	LOG_NOTICE("Try some foo");
	getchar();
	
	for (int i = 10; i < 14; ++i)
		foo->DoFoo(i);
	
	LOG_NOTICE("Try some bar");
	getchar();
	
	for (int i = 0; i < 2; ++i);
		foo->DoBar();

	LOG_NOTICE("Remove some listeners");
	for (int i = 0; i < 4; ++i)
	{
		if (i%2 == 0)
			foo->removeListener(kittens[i]);
	}
	
	LOG_NOTICE("Try some foo");
	getchar();
	
	for (int i = 10; i < 11; ++i)
		foo->DoFoo(i);
	
	LOG_NOTICE("Try some bar");
	getchar();
	
	for (int i = 0; i < 2; ++i);
		foo->DoBar();
}
	





