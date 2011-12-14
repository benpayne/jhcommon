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

#ifndef JH_FILE_H_
#define JH_FILE_H_

/**
 * \file 
 *
 * A basic file object, implements the IReaderWriter interface.  Be
 * careful of the open/open2 distinctions.
 */

#include <sys/mman.h>

#include "IReaderWriter.h"
#include "Selector.h"

//! @note jh_memoryc is only included here because too many people rely on it
#include "jh_memory.h"		
#include "jh_types.h"

/**
 *	@brief Jethead File class
 */
class File : public IReaderWriter
{
public:
	//! Default constructor, doesn't really do anything.
	File();

	//! Close it up if needed
	virtual ~File();

	/**
	 *	Open a file giving the filename.  If you are opening a file
	 *	that exists this class will check your permission on the file
	 *	and open it with the least restrictive permissions allowed.
	 *	You can specify other options through the flags param, these
	 *	are the same option that go to the system call open.
	 * 
	 *  @note You probably need to read the description on this
	 *  carefully if you are having problems with opening files.
	 * 
	 *	@param name the name of the file to open or create.
	 *
	 *	@param flags the flags for the open, O_RDWR flags do not need
	 *	to be set.
	 *
	 *	@return the -errno or 0
	 */
	virtual int open( const char *name, int flags = 0 );

	/**
	 * A straight passthrough to open(2).  Doesn't do the
	 * flag-manipulation that File::open does.
	 */
	int open2( const char *name, int flags = 0 );
	
	/**
	 *	Close the file allowing this object to open another file.
	 */
	int close();

	/**
	 *	Add a selector listener to the File
	 */
	void setSelector( SelectorListener *listener, Selector *selector );

	//! Add a listener with the specified event mask
	void setSelector( SelectorListener *listener, Selector *selector, 
					  short events );
	
	//! A passthrough for read(2)
	int read( void *buffer, int len );

	//! A passthrough for write(2)
	int write( const void *buffer, int len );
	
	/**
	 *	Get the file position (64-bit safe)
	 */
	jh_off64_t	getPos() const;
	
	/**
	 *	Seek end of file.  This returns offset to end of file (64-bit safe)
	 */
	jh_off64_t seekEnd();

	/**
	 *	Set the file position (64-bit safe)
	 */
	jh_off64_t setPos( jh_off64_t offset );
	
	/**
	 *	Return the start offset of the file.  For normal files
	 *	this is always 0
	 */
	virtual jh_off64_t getStartOffset() const
	{
		return (jh_off64_t)0LL;
	}
	
	/**
	 *	Get the file length (64-bit safe).  Used in combination with
	 *	the start offset this can be used to get the end offset of
	 *	the file.
	 */
	jh_off64_t	getLength() const;
	
	
	/**
	 *	mmap a file, see the man page mmap(2)
	 *
	 *	@return pointer to buffer or MAP_FAILED
	 */
	uint8_t *mmap( off_t offset, size_t length, 
				   int prot = (PROT_READ | PROT_WRITE) );

	//! Unmap the memory mapping for this file
	void	munmap();

	//! Make sure everything in the memory mapped buffer gets to disk
	void 	msync();
	
	/**
	 *	copies all data from caches to disk
	 */
	void	fsync();
	
	//! Helper for "is this a valid file on disk?"
	static bool isFile( const char *name );

	//! Helper for "is this a valid directory name on disk?"
	static bool isDir( const char *name );

	//! Helper for "is there a directory entry of this name?"
	static bool exists( const char *name );

	//! Helper for "how big is this file?"
	static int size( const char *name );
	
protected:
	/**
	 * The file descriptor from open(2)
	 * @todo Should this be made private?
	 */
	int mFd;

	//! Give access to the fd for child classes
	int getFd() { return mFd; }
	
private:
	//! The selector we are living on
	Selector *mSelector;

	//! The address we are mmaped onto
	uint8_t *mMapAddress;

	//! The size of the mmapped buffer (needed for munmap)
	size_t	mMapLength;
};

#endif // JH_FILE_H_
