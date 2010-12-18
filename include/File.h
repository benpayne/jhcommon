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

#include "IReaderWriter.h"
#include "Selector.h"
#include "Path.h"
#include "JetHead.h"

//! @note jh_memoryc is only included here because too many people rely on it
#include "jh_memory.h"		
#include "jh_types.h"
#include "jh_string.h"

namespace JetHead
{
	class File;
	
	class FileListener
	{
	public:
		virtual void handleData( File *f, short event ) = 0;	
	};
	
	/**
	 *	@brief Jethead File class
	 */
	class File : public IReaderWriter, public SelectorListener
	{
	public:
		//! Default constructor, doesn't really do anything.
		File();
	
		//! Close it up if needed
		virtual ~File();
	
		enum OpenFlags {
			OF_READ = 1,
			OF_WRITE = 2,
			OF_RDWR = 3,
			OF_TRUNC = 4,	// if trunc and append are set, append will have no effect.
			OF_APPEND = 8,
			OF_CREATE = 16,
			OF_ODIRECT = 32,
		};
	
		/**
		 *	Open a file giving the filename.  
		 * 
		 *	@param name the Path object of the file to open or create.
		 *	@param flags the flags for the open, see OpenFlags enum.
		 */
		ErrCode open( const Path &name, int flags = OF_READ );

		/**
		 *	Open a file giving the filename.  
		 * 
		 *	@param name the Path object of the file to open or create.
		 *	@param flags the flags for the open, see OpenFlags enum.
		 */
		virtual ErrCode open( const char *name, int flags = OF_READ );
		
		/**
		 *	Close the file allowing this object to open another file.
		 */
		virtual ErrCode close();
	
		/**
		 *	Add a selector listener to the File
		 */
		void setSelector( FileListener *listener, Selector *selector );
	
		//! Add a listener with the specified event mask
		void setSelector( FileListener *listener, Selector *selector, 
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
		uint8_t *mmap();
	
		//! Unmap the memory mapping for this file
		ErrCode	munmap();
	
		//! Make sure everything in the memory mapped buffer gets to disk
		ErrCode 	msync();
		
		/**
		 *	copies all data from caches to disk
		 */
		ErrCode	fsync();
	
		/** Some methods don't have a good way to return f the above methods return an error condition, then this method can
		 *   be called to get the exact error.
		 */
		JetHead::ErrCode	getLastError() { return mError;	}
		
		enum {
			//! files[PIPE_READER] is the read end
			PIPE_READER = 0,

			//! files[PIPE_WRITER] is the write end
			PIPE_WRITER = 1
		};
		
		/** 
		 * pipe - create a pipe, this will give two files back, one for reading 
		 *  and one for writing.  files[ 0 ] is for reading and files[ 1 ] is for 
		 *  writing.
		 * @param files - an array of two File*, item 0 is the read file and item 1 is the write file 
		 */
		 static JetHead::ErrCode pipe( File *files[ 2 ] );
				
	protected:
		/**
		 * The file descriptor from open(2)
		 * @todo Should this be made private?
		 */
		int mFd;
	
		//! Give access to the fd for child classes
		int getFd() { return mFd; }
		
		virtual void processFileEvents( int fd, short events, 
											jh_ptr_int_t private_data );

	private:
		//! constructor for building a File object from an fd.
		File( int fd );
	
		//! used internally to set the error code from errno.
		void setError() const;
		
		Selector 		*mSelector;
		FileListener	*mListener;		
		uint8_t 		*mMapAddress;	
		size_t			mMapLength;		
		int 			mOpenFlags;
		mutable ErrCode mError;
	};
	
	class Directory 
	{
	public:
		Directory();
		
		bool open( const Path &p );
	
		const Path *getEntry( int i );
		
	private:
		JetHead::vector<Path> mData;
		int mNumberEntries;
		void *mDirHandle;
	};
};
	
#endif // JH_FILE_H_
