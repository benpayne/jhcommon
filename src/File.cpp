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

#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#include "jh_types.h"
#include "logging.h"
#include "File.h"
#include "JH_64BitFops.h"

using namespace JetHead;

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

File::File() : mFd( -1 ), mSelector( NULL ), mListener( NULL ),
	mMapAddress( NULL ), mOpenFlags( 0 ), mError( kNoError )
{
}

File::File( int fd ) : mFd( fd ), mSelector( NULL ), mListener( NULL ),
	mMapAddress( NULL ), mOpenFlags( 0 ), mError( kNoError )
{
}

File::~File()
{
	close();
}

JetHead::ErrCode File::open( const Path &name, int flags )
{
	return open( name.getString().c_str(), flags );
}

#define O_RDWR_MASK 3

JetHead::ErrCode File::open( const char *name, int flags )
{
	int sys_flags = 0;

	TRACE_BEGIN( LOG_LVL_INFO );

	LOG( "Flags %x", flags );

	mOpenFlags = flags;

	if ( ( flags & OF_RDWR ) == OF_RDWR )
		sys_flags |= O_RDWR;
	else if ( flags & OF_READ )
		sys_flags |= O_RDONLY;
	else if ( flags & OF_WRITE )
		sys_flags |= O_WRONLY;
	else
		sys_flags |= O_RDONLY;

	// trunc and append only effect write operations.  We'll ignore if not
	//  opening for write.  Since rumor has it some platforms will do strange
	//  things when you open read only with these flags.
	if ( flags & OF_WRITE )
	{
		// if both trunc and append are set, we'll trunc.
		if ( flags & OF_TRUNC )
			sys_flags |= O_TRUNC;
		else if ( flags & OF_APPEND )
			sys_flags |= O_APPEND;
	}

#ifndef PLATFORM_DARWIN
	// This is a Linux specific construct that we support on those systems.
	//  if O_DIRECT is not passed, all calling code should still work on the
	//  non-Linux platform.
	if ( flags & OF_ODIRECT )
		sys_flags |= O_DIRECT;
#endif

	LOG( "sys_flags %x", sys_flags );

	if ( flags & OF_CREATE )
	{
		LOG( "Create" );
		sys_flags |= O_CREAT;
		mFd = ::open( name, sys_flags, 0666 );
	}
	else
		mFd = ::open( name, sys_flags );

	if (mFd == -1)
	{
		return getErrorCode( errno );
	}

	return JetHead::kNoError;
}

JetHead::ErrCode File::close()
{
	TRACE_BEGIN(LOG_LVL_INFO);
	LOG_INFO( "fd %d", mFd );
	if ( mFd != -1 )
	{
		int res = ::close( mFd );
		mFd = -1;
		if ( res == -1 )
		{
			return getErrorCode( errno );
		}
	}
	else
		return JetHead::kNotInitialized;

	return JetHead::kNoError;
}

void File::setSelector( FileListener *listener, Selector *selector )
{
	setSelector( listener, selector, POLLIN );
}

void File::setSelector( FileListener *listener, Selector *selector,
						short events )
{
	TRACE_BEGIN(LOG_LVL_NOTICE);
	if ( mFd != -1 )
	{
		if ( mSelector != NULL )
			mSelector->removeListener( mFd, this );
		mSelector = selector;
		mListener = listener;
		if ( mSelector != NULL )
			mSelector->addListener( mFd, events, this, (jh_ptr_int_t)this );
	}
}


int File::read( void *buffer, int len )
{
	int res = ::read( mFd, buffer, len );

	if ( res == -1 )
		setError();

	return res;
}

int File::write( const void *buffer, int len )
{
	int res = ::write( mFd, buffer, len );

	if ( res == -1 )
		setError();

	return res;
}

jh_off64_t	File::getLength() const
{
	jh_stat64_t file_status;

	int res = jh_fstat64( mFd, &file_status );

	if ( res == -1 )
	{
		setError();
		return (jh_off64_t)-1;
	}

	return file_status.st_size;
}

jh_off64_t File::seekEnd()
{
	jh_off64_t res = jh_lseek( mFd, 0, SEEK_END );

	if ( res == (jh_off64_t)-1 )
	{
		setError();
	}

	return res;
}

jh_off64_t	File::getPos() const
{
	jh_off64_t res = jh_lseek( mFd, 0, SEEK_CUR );

	if ( res == (jh_off64_t)-1 )
	{
		setError();
	}

	return res;
}

jh_off64_t File::setPos( jh_off64_t offset )
{
	jh_off64_t res = jh_lseek( mFd, offset, SEEK_SET );

	if ( res == (jh_off64_t)-1 )
	{
		setError();
	}

	return res;
}

uint8_t *File::mmap()
{
	int prot = PROT_READ;
	size_t length = getLength();

	if ( ( mOpenFlags & OF_RDWR ) == OF_RDWR )
		prot |= PROT_WRITE;
	else if ( mOpenFlags & OF_WRITE )
		prot = PROT_WRITE;

	uint8_t *addr = (uint8_t*)::mmap( NULL, length, prot, MAP_SHARED, mFd, 0 );

	if ( addr != MAP_FAILED )
	{
		mMapAddress = addr;
		mMapLength = length;
	}
	else
	{
		setError();
		addr = NULL;
	}

	return addr;
}

JetHead::ErrCode	File::munmap()
{
	if ( mMapAddress != NULL )
	{
		if ( ::munmap( mMapAddress, mMapLength ) != 0 )
			return getErrorCode( errno );
	}
	else
	{
		return JetHead::kInvalidRequest;
	}

	return JetHead::kNoError;
}

JetHead::ErrCode	File::msync()
{
	if ( mMapAddress != NULL )
	{
		if ( ::msync( mMapAddress, mMapLength, MS_ASYNC ) != 0 )
			return getErrorCode( errno );
	}
	else
	{
		return JetHead::kInvalidRequest;
	}

	return JetHead::kNoError;
}

JetHead::ErrCode	File::fsync()
{
	int res = ::fsync( mFd );

	if ( res == -1 )
		return getErrorCode( errno );

	return kNoError;
}


JetHead::ErrCode	File::pipe( File *pipe_files[ 2 ] )
{
	int fds[ 2 ];
	int res = ::pipe( fds );

	if ( res == -1 )
		return 	getErrorCode( errno );

	pipe_files[ 0 ] = jh_new File( fds[ 0 ] );
	pipe_files[ 1 ] = jh_new File( fds[ 1 ] );

	return kNoError;
}

void File::processFileEvents( int fd, short events, jh_ptr_int_t private_data )
{
	File *f = (File*)private_data;

	if ( f->mListener != NULL )
	{
		f->mListener->handleData( f, events );
	}
}

void	File::setError() const
{
	mError = getErrorCode( errno );
}

#define DIR_HANDLE ((DIR*)mDirHandle)

static const int gNumberEntries = 50;

Directory::Directory() : mNumberEntries( -1 ), mDirHandle( NULL )
{
}

bool Directory::open( const Path &p )
{
	if ( mDirHandle != NULL )
		return false;

	mDirHandle = opendir( p.getString().c_str() );

	mData.reserve( gNumberEntries );

	struct dirent dir;
	struct dirent *dir_ptr = &dir;
	int i = 0;

	// fill with the first gNumberEntries files.  This is done to keep this
	//  from taking too much time if the directory has 1000's of files.
	while ( dir_ptr != NULL && i < gNumberEntries )
	{
		if ( readdir_r( DIR_HANDLE, &dir, &dir_ptr ) != 0 )
			return false;

		if ( dir_ptr != NULL )
		{
			mData[ i ] = dir_ptr->d_name;
			i += 1;
		}
	}

	if ( dir_ptr == NULL )
		mNumberEntries = i + 1;

	return true;
}

const Path *Directory::getEntry( int i )
{
	if ( mNumberEntries != -1 && i >= mNumberEntries )
		return NULL;

	if ( i >= (int)mData.size() )
	{
		int start = mData.size();
		int read_size = i - start;

		if ( read_size < gNumberEntries )
			read_size = gNumberEntries;

		mData.reserve( start + read_size );

		struct dirent dir;
		struct dirent *dir_ptr = &dir;
		int j = 0;

		// fill with the first gNumberEntries files.  This is done to keep this
		//  from taking too much time if the directory has 1000's of files.
		while ( dir_ptr != NULL && j < read_size )
		{
			if ( readdir_r( DIR_HANDLE, &dir, &dir_ptr ) != 0 )
				return NULL;

			if ( dir_ptr != NULL )
			{
				mData[ start + j ] = dir_ptr->d_name;
				j += 1;
			}
		}

		if ( dir_ptr == NULL )
			mNumberEntries = j + 1;
	}

	if ( i >= (int)mData.size() )
		return NULL;
	else
		return &mData[ i ];
}
