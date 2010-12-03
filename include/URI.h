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

#ifndef JH_URI_H_	
#define JH_URI_H_

#include "jh_string.h"
#include "jh_vector.h"

/**
 Class to parse and generate URI's.  The names of methods follow the naming 
  used in RFC 2396 for "generic URIs".   In breif this is 
   scheme://authority/path?query#fragment.  The query part of the URI follow
   the format of param1=value&param2=value, with as many params as needed.
 */
class URI
{
public:
	URI();
	URI( const char *uri );
	URI( const JHSTD::string &uri );
	URI( const URI &other );
	~URI();
	
	const URI &operator=( const char *uri );
	const URI &operator=( const URI &other );
	
	bool operator == (const URI &other) const;
	bool operator != (const URI &other) const;

	const JHSTD::string &getScheme() const;
	void setScheme( const JHSTD::string &scheme );

	const JHSTD::string &getAuthority() const;
	void setAuthority( const JHSTD::string &authority );
	JHSTD::string getHost() const;
	int getPort() const;

	const JHSTD::string &getPath() const;
	void setPath( const JHSTD::string &path, bool relative = false );

	JHSTD::string getPathAndQuery() const;

	const JHSTD::string &getQuery() const;
	void setQuery( const JHSTD::string &query );
	void appendQueryParam( const JHSTD::string &key, const JHSTD::string &value );
	void removeQueryParam( const JHSTD::string &key );
	const JHSTD::string &getQueryParam( const JHSTD::string &key ) const;

	const JHSTD::string &getFragment() const;
	void setFragment(const JHSTD::string &fragment );
	
	const JHSTD::string &getString() const;
	bool setString( const char *uri );
	bool setString( JHSTD::string &uri );
	
	bool isRelative() const { return mRelative; }

	void clear();
	
private:
	bool parseString();
	void buildString() const;
	bool parseQuery();
	void buildQuery() const;
	
	void copyValues(const URI& other);
	
	struct QueryParam 
	{
		QueryParam() {}
		
		QueryParam(const JHSTD::string& key, const JHSTD::string &value) 
		: mKey(key), mParam(value)
		{
		}
		
		QueryParam(const QueryParam& other)
		:	mKey(other.mKey), mParam(other.mParam)
		{
		}
		
		const QueryParam& operator=(const QueryParam& other)
		{
			mKey = other.mKey;
			mParam = other.mParam;
			return *this;
		}
		
		JHSTD::string mKey;
		JHSTD::string mParam;
	};

	int findParam( const JHSTD::string &key ) const;
	
	bool mRelative;
	mutable bool mModified;
	mutable bool mModifiedQuery;
	mutable JHSTD::string mFullString;
	mutable JHSTD::string mQueryString;
	JHSTD::string mScheme;
	JHSTD::string mAuthority;
	JHSTD::string mPath;
	JHSTD::string mFragment;
	JetHead::vector<QueryParam> mQueryParams;
};

#endif // JH_URI_H_
