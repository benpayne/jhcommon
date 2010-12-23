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

#ifndef JH_PATH_H_
#define JH_PATH_H_

#include "jh_string.h"
#include "URI.h"
#include "JetHead.h"

namespace JetHead
{
	class Path
	{
	public:
		Path() {}
		Path( const char *path ) : mPath( path ) {}
		Path( const JHSTD::string &path ) : mPath( path ) {}
		Path( const Path &path ) : mPath( path.mPath ) {}
		Path( const URI &uri );
		
		Path &operator=( const char *path ) { mPath = path; return *this; }
		Path &operator=( const JHSTD::string &path ) { mPath = path; return *this;  }
		Path &operator=( const Path &path )  { mPath = path.mPath; return *this; }
		Path &operator=( const URI &uri );
		
		bool append( const char *path ) { return append( JHSTD::string( path ) ); }
		bool append( const JHSTD::string &path );
		bool append( const Path &path ) { return append( path.mPath ); }

		const JHSTD::string &getString() const { return mPath; }
		
		//! Return the path part of a file name ../../dir/file.so would return "../../dir"
		JHSTD::string parent();

		//! Return the path part of a file name ../../dir/file.so would return "file.so"
		JHSTD::string filename();

		//! Return the path part of a file name ../../dir/file.so would return "file"
		JHSTD::string fileBasename();

		//! Return the path part of a file name ../../dir/file.so would return "so"
		JHSTD::string filenameExtention();

		//! Is this path a full path or relative path.
		bool isRelative();
		
		//! If this path is relative, convert it to a full path, based on current working director.
		bool makeAbsolute();

		//! If this path contains un-nessasary links remove them (i.e. "./p1/../p2" would become "./p2" ) 
		bool normalize();
		
		//! Create a file if it does not exist, otherwise change it's modified time to current time.
		ErrCode touch();

		//! Remove a file or empty directory from the file system.
		ErrCode remove();
		
		/**
		 * rename a file or directory with a new name.  Refer to Linux man page
		 *  for rename(2) for details on edge cases.
		 */
		ErrCode rename( const Path &new_name );

		/** Make a directory of this name, only the last element in the path 
		 *    will be created, if a parent directory does not exist this will fail.
		 */
		ErrCode mkdir();

		//! Make a directory of this name and any parent dirs that do not exist.
		ErrCode mkdirs();

		bool isFile();
		bool isDir();
		bool exists();	
		
		ErrCode list( JetHead::vector<Path> &entries );

		URI	toURI();
		
		//! the length of the file.
		jh_off64_t length();
		
		//! get the modified time of the file.
		time_t	getModifiedTime();
		
		static Path getCWD();
		
		static const char PATH_SEPERATOR = '/';

	private:
		JHSTD::string mPath;
	};
};

#endif // JH_PATH_H_
