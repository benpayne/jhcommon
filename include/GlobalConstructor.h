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

#ifndef JH_GLOBAL_CONSTRUCTOR_H_
#define JH_GLOBAL_CONSTRUCTOR_H_

/**
 * This class is a helper to make initializing code at start easier.  If you 
 *  have a class with some static data that need to be initialized at startup
 *  you can create a static method that initialized your data and then add a
 *  declaration in global scope like this:
 *
 * GLOBAL_CONSTRUCT( &Class::MyInitializer );
 *
 * This will cause Class::MyInitilizer to be executed before your main runs.
 */
class GlobalConstructor
{
public:
	typedef void (*GlobalConstructorFunc)();
	
	GlobalConstructor( GlobalConstructorFunc init ) : mFunc( NULL ) { init(); }	
	GlobalConstructor( GlobalConstructorFunc init, GlobalConstructorFunc destroy ) :
		mFunc( destroy ) { init(); }	

	~GlobalConstructor() { if ( mFunc != NULL ) mFunc(); }

private:
	GlobalConstructorFunc mFunc;
};

#define GLOBAL_CONSTRUCT( construct ) static GlobalConstructor gc_function( construct )
#define GLOBAL_CONSTRUCT_DESTRUCT( construct, destruct ) static GlobalConstructor gc_function( construct, destruct )

#endif // JH_GLOBAL_CONSTRUCTOR_H_
