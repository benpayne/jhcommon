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

#ifndef JH_FDREADERWRITER_H_
#define JH_FDREADERWRITER_H_


#include "IReaderWriter.h"

/**
 *	@brief Simple ReaderWriter to wrap a file descriptor
 *
 * This object provides the IReaderWriter interface on a simple
 *  linux file descriptor.  In most cases your better of using on of
 *  jhcommons other IReaderWriter implemtor classes like File, or Socket.
 *  but if you have some other type of RD or just have to open something
 *  yourself this will allow you to use the other jhcommon constructs.
 *  Be aware that the fd is not closed bt this class automatically at 
 *  descruction, you are responsible for ensuring that some closes the FD.
 */
class FdReaderWriter : public IReaderWriter
{
public:
	FdReaderWriter( int fd );
	virtual ~FdReaderWriter();

	/**
	 * Set a selector and start listening for file events on this fd.
	 *  This will listen for POLLIN, if you want other event see overloaded 
	 *  version below.
	 */
	void setSelector( SelectorListener *listener, Selector *selector );

	/**
	 * Set a selector and start listening for file events on this fd.
	 *  This will listen for whatever you specify in events.  This should
	 *  be valid value for the poll system call.  This include POLLIN, 
	 *  POLLOUT, etc...
	 */
	void setSelector( SelectorListener *listener, Selector *selector, short events );

	/**
	 * Read bytes from the fd
	 */
	int read( void *buffer, int length );
	
	/**
	 * Write bytes to the fd
	 */
	int write( const void *buffer, int length );

	/**
	 * Close the fd, note that when this class is destroyed it will NOT
	 *  close the fd, you must do that yourself.
	 */
	int close();
	
private:
	int		mFd;
	Selector *mSelector;
};

#endif // JH_FDREADERWRITER_H_
