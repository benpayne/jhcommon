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
#include <utime.h>

#include "jh_types.h"
#include "logging.h"
#include "Path.h"
#include "JH_64BitFops.h"
#include "File.h"

using namespace JetHead;

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

using JHSTD::string;

Path::Path( const URI &uri )
{
}

Path &Path::operator=( const URI &uri )
{
	return *this;
}

bool Path::append( const JHSTD::string &path )
{
	mPath.append( path );
	return true;
}

ErrCode Path::touch()
{
	if ( isFile() )
	{
		int res = ::utime( mPath.c_str(), NULL );
		if ( res != 0 )
		{
			return getErrorCode( errno );
		}
	}
	else
	{
		File f;
		ErrCode err = f.open( mPath.c_str(), File::OF_RDWR | File::OF_CREATE );
		if ( err == kNoError )
			f.close();
		else 
			return err;
	}

	return kNoError;
}

ErrCode Path::remove()
{
	int res = 0;
	if ( isDir() )
	{
		res = ::rmdir( mPath.c_str() );
	}
	else
	{
		res = ::unlink( mPath.c_str() );
	}
	
	if ( res != 0 )
	{
		return getErrorCode( errno );
	}
	
	return kNoError;
}

ErrCode Path::rename( const Path &new_name )
{
	int res = ::rename( mPath.c_str(), new_name.mPath.c_str() );
	
	if ( res != 0 )
	{
		return getErrorCode( errno );
	}
	
	return kNoError;
}

ErrCode Path::mkdir()
{
	int res = ::mkdir( mPath.c_str(), 0777 );
	
	if ( res != 0 )
	{
		return getErrorCode( errno );
	}
	
	return kNoError;	
}

ErrCode Path::mkdirs()
{
	// TODO...
	return kNoError;
}

//! Return the path part of a file name ../../dir/file.so would return "../../dir"
string Path::parent()
{	
	unsigned pos = mPath.rfind( "/" );

	if ( pos == string::npos )
	{
		return mPath;
	}
	else
	{
		return mPath.substr( 0, pos + 1 );
	}
}

//! Return the path part of a file name ../../dir/file.so would return "file.so"
string Path::filename()
{	
	unsigned pos = mPath.rfind( "/" );

	if ( pos == string::npos )
	{
		return mPath;
	}
	else
	{
		return mPath.substr( pos + 1 );
	}
}

//! Return the path part of a file name ../../dir/file.so would return "file"
string Path::fileBasename()
{
	string f = filename();
	unsigned pos = f.rfind( "." );

	if ( pos == string::npos )
	{
		return f;
	}
	else
	{
		return f.substr( 0, pos - 1 );
	}
}

//! Return the path part of a file name ../../dir/file.so would return "so"
string Path::filenameExtention()
{
	string f = filename();
	unsigned pos = f.rfind( "." );

	if ( pos == string::npos )
	{
		return f;
	}
	else
	{
		return f.substr( pos + 1 );
	}
}

bool Path::isFile()
{
	struct stat buf;
	
	int res = stat( mPath.c_str(), &buf );
	
	if ( res == 0 && (buf.st_mode & S_IFREG) )
		return true;
	else 
		return false;
}

bool Path::isDir()
{
	struct stat buf;
	
	int res = stat( mPath.c_str(), &buf );
	
	if ( res == 0 && (buf.st_mode & S_IFDIR) )
		return true;
	else 
		return false;
}

bool Path::exists()
{
	struct stat buf;
	
	int res = stat( mPath.c_str(), &buf );
	
	if ( res == 0 )
		return true;
	else
		return false;
}

jh_off64_t Path::length()
{
	jh_stat64_t buf;

	int res = jh_stat64( mPath.c_str(), &buf );
	
	if ( res == 0 )
		return buf.st_size;
	else
		return (jh_off64_t)-1;
}


