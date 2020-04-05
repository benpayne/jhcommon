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
#include <unistd.h>
#include <limits.h>

#include "jh_types.h"
#include "logging.h"
#include "Path.h"
#include "JH_64BitFops.h"
#include "File.h"
#include "jh_vector.h"

using namespace JetHead;

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOISE );

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
	if ( mPath[ mPath.length() - 1 ] != PATH_SEPERATOR )
		mPath.append( 1, PATH_SEPERATOR	);
		
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
		LOG_NOISE( "Removing Dir" );
		res = ::rmdir( mPath.c_str() );
	}
	else
	{
		LOG_NOISE( "Removing File" );
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
	JHSTD::vector<JHSTD::string> parts;
	JHSTD::vector<JHSTD::string> parts2;
	JetHead::split( mPath, "/", parts );
	Path p = parts[ 0 ];
	
	for ( unsigned i = 1; i < parts.size(); i++ )
	{
		if ( !p.exists() )
		{
			ErrCode err = p.mkdir();
			if ( err != kNoError )
				return err;
		}
		
		p.append( parts[ i ] );
	}
		
	return kNoError;
}

//! Return the path part of a file name ../../dir/file.so would return "../../dir"
Path Path::parent()
{	
	Path p = *this;
	p.normalize();

	auto pos = p.mPath.rfind( "/" );

	//if ( pos == mPath.length() - 1 )
	//	pos = mPath.rfind( "/", pos - 1 );
		
	if ( pos == string::npos )
	{
		return Path( "." );
	}
	else
	{
		return Path( p.mPath.substr( 0, pos + 1 ) );
	}
}

//! Return the path part of a file name ../../dir/file.so would return "file.so"
string Path::filename()
{	
	Path p = *this;
	p.normalize();
	
	auto pos = p.mPath.rfind( "/" );

	if ( pos == string::npos )
	{
		return p.mPath;
	}
	else
	{
		return p.mPath.substr( pos + 1, string::npos );
	}
}

//! Return the path part of a file name ../../dir/file.so would return "file"
string Path::fileBasename()
{
	string f = filename();
	auto pos = f.rfind( "." );

	if ( pos == string::npos )
	{
		return f;
	}
	else
	{
		return f.substr( 0, pos );
	}
}

//! Return the path part of a file name ../../dir/file.so would return "so"
string Path::filenameExtention()
{
	string f = filename();
	auto pos = f.rfind( "." );

	if ( pos == string::npos )
	{
		return "";
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

	return res == 0 && (buf.st_mode & S_IFREG);
}

bool Path::isDir()
{
	struct stat buf;
	
	int res = stat( mPath.c_str(), &buf );

	return res == 0 && (buf.st_mode & S_IFDIR);
}

bool Path::exists()
{
	struct stat buf;
	
	int res = stat( mPath.c_str(), &buf );

	return res == 0;
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

bool	Path::isRelative()
{
	return mPath[0] != PATH_SEPERATOR;
}

bool	Path::makeAbsolute()
{
	Path p( getCWD() );
	
	p.append( mPath );
	
	mPath = p.getString();
	
	return true;
}

bool	Path::normalize()
{
	JHSTD::vector<JHSTD::string> parts;
	JHSTD::vector<JHSTD::string> parts2;
	JetHead::split( mPath, "/", parts );
	bool start = true;
	
	for ( unsigned i = 0; i < parts.size(); i++ )
	{
		if ( parts[ i ] == ".." )
		{
			if ( !start )
				parts2.erase( parts2.end()	);
			else
				parts2.push_back( parts[ i ] );
		}
		else if ( parts[ i ] != "." && parts[ i ] != "" )
		{
			start = false;
			parts2.push_back( parts[ i ] );
		}
	}
		
	if ( isRelative() == false )
		mPath.assign( 1, PATH_SEPERATOR );
	else
		mPath.clear();
	
	for ( unsigned i = 0; i < parts2.size() - 1; i++ )
	{
		mPath.append( parts2[ i ] );
		mPath.append( 1, PATH_SEPERATOR );
	}

	mPath.append( parts2[ parts2.size() - 1 ] );
	
	return true;
}

Path	Path::getCWD()
{
	char buffer[ PATH_MAX ];
	Path p( ::getcwd( buffer, PATH_MAX ) );
	return p;
}


