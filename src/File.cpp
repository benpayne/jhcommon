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
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "jh_types.h"
#include "logging.h"
#include "File.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

#ifdef PLATFORM_DARWIN
#define jh_stat64_t struct stat64
#define jh_stat64	stat64
#define jh_lseek	lseek
#define jh_fstat64	fstat64
#else
#define jh_stat64_t struct stat64
#define jh_stat64	stat64
#define jh_lseek	lseek64
#define jh_fstat64	fstat64
#endif

File::File() : mFd( -1 ), mSelector( NULL ), mMapAddress( NULL )
{
}

File::~File()
{
	close();
}

int File::open( const char *name, int flags )
{
	struct stat file_status;
	int permission = 0;

	TRACE_BEGIN( LOG_LVL_INFO );
	
	int res = stat( name, &file_status );
	
	if ( res != 0 && ( flags & O_CREAT ) )
	{
		// don't add O_RDWR if O_WRONLY specified in flags
		if (!(flags & O_WRONLY))
		{
			flags |= O_RDWR;
		}
		
		mFd = ::open( name, flags, 0666 );
		if (mFd == -1)
		{
			LOG_ERR_PERROR( "Failed, unable to create file %s", name );
			return -errno;
		}
		return 0;
	}
	else if ( res != 0 )
	{
		LOG_ERR_PERROR( "Failed, unable to stat file %s", name );
	}

	if ( file_status.st_uid == geteuid() )
		permission = ( file_status.st_mode >> 6 ) & 6;
	else if ( file_status.st_gid == getegid() )
		permission = ( file_status.st_mode >> 3 ) & 6;
	else
		permission = file_status.st_mode & 6;

	if ( permission == 6 )
		flags |= O_RDWR;
	else if ( permission == 4 )
		flags |= O_RDONLY;
	else if ( permission == 2 )
		flags |= O_WRONLY;
	else
		return -EACCES;
	
	// ignore return since it's set in open2
	open2( name, flags );
	
	LOG_INFO( "file %s, fd %d, device is %d", name, mFd, (int) file_status.st_dev );

	if ( mFd == -1 )
	{
		LOG_ERR_PERROR( "Failed, unable to open file %s", name );
		return -errno;
	}

	return 0;
}

int File::open2( const char *name, int flags )
{
	if ( ( flags & O_CREAT ) and !exists( name ) )
	{
		flags |= O_RDWR;	
		mFd = ::open( name, flags, 0666 );
	}
	else
	{
		mFd = ::open( name, flags );
	}
	
	return 0;
}

int File::close()
{
	TRACE_BEGIN(LOG_LVL_INFO);
	LOG_INFO( "fd %d", mFd );
	if ( mFd != -1 )
	{
		int res = ::close( mFd );
		mFd = -1;
		if ( res == -1 )
		{
			LOG_ERR_PERROR("Failed, close error");
			return -errno;
		}
	}
	else
		return -EBADF;

	return 0;
}

void File::setSelector( SelectorListener *listener, Selector *selector )
{
	setSelector( listener, selector, POLLIN );
}

void File::setSelector( SelectorListener *listener, Selector *selector, 
						short events )
{
	TRACE_BEGIN(LOG_LVL_NOTICE);
	if ( mFd != -1 )
	{
		if ( mSelector != NULL )
			mSelector->removeListener( mFd, listener );
		mSelector = selector;
		if ( mSelector != NULL )
			mSelector->addListener( mFd, events, listener );
	}
}


int File::read( void *buffer, int len )
{
	int res = ::read( mFd, buffer, len );

	if ( res == -1 )
		LOG_WARN_PERROR( "read failed" );
	
	return res;
}

int File::write( const void *buffer, int len )
{
	int res = ::write( mFd, buffer, len );

	if ( res == -1 )
		LOG_WARN_PERROR( "write failed" );
	
	return res;
}
	
jh_off64_t	File::getLength() const
{
	jh_stat64_t file_status;
	
	int res = jh_fstat64( mFd, &file_status );

	if ( res == -1 )
		return (jh_off64_t)-1;
	
	return file_status.st_size;
}

jh_off64_t File::seekEnd()
{
	return jh_lseek( mFd, 0, SEEK_END );
}

jh_off64_t	File::getPos() const
{
	return jh_lseek( mFd, 0, SEEK_CUR );
}

jh_off64_t File::setPos( jh_off64_t offset )
{
	return jh_lseek( mFd, offset, SEEK_SET );
}

bool File::isFile( const char *name )
{
	struct stat buf;
	
	int res = stat( name, &buf );
	
	if ( res == 0 && (buf.st_mode & S_IFREG) )
		return true;
	else 
		return false;
}

bool File::isDir( const char *name )
{
	struct stat buf;
	
	int res = stat( name, &buf );
	
	if ( res == 0 && (buf.st_mode & S_IFDIR) )
		return true;
	else 
		return false;
}

bool File::exists( const char *name )
{
	struct stat buf;
	
	int res = stat( name, &buf );
	
	if ( res == 0 )
		return true;
	else
		return false;
}

int File::size( const char *name )
{
	struct stat buf;

	int res = stat( name, &buf );
	
	if ( res == 0 )
		return buf.st_size;
	else
		return -1;
}

uint8_t *File::mmap( off_t offset, size_t length, int prot )
{
	uint8_t *addr = (uint8_t*)::mmap( NULL, length, prot, MAP_SHARED, mFd, offset );

	if ( addr != MAP_FAILED )
	{
		mMapAddress = addr;
		mMapLength = length;
	}
	else
		LOG_ERR_PERROR( "Failed to map file" );
	
	return addr;
}

void	File::munmap()
{
	if ( mMapAddress != NULL )
		::munmap( mMapAddress, mMapLength );
}

void	File::msync()
{
	if ( mMapAddress != NULL )
		::msync( mMapAddress, mMapLength, MS_ASYNC );
}

void	File::fsync()
{
	::fsync( mFd );	
}

