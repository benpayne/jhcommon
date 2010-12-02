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
#ifndef JH_STRING_H
#define JH_STRING_H

/**
 * \file
 *
 */

#include <string.h>
#include "RefCount.h"

#ifdef USE_JETHEAD_STRING
#define JHSTD	JetHead
#else
#define JHSTD	std
#endif

namespace JetHead
{
	template <class charT>
	class basic_string
	{
	public:
		typedef unsigned size_type;
		static const size_type DEFAULT_ALLOCATION = 32;
		static const size_type npos = -1;
		static const charT null_char = 0;
		
		//! Default constructor, nothing contained in the array, 
		basic_string()
		{
			mData = jh_new Rep( DEFAULT_ALLOCATION );
		}

		//! Copy constructor
		basic_string(const basic_string& rhs)
		{
			assign( rhs );
		}

		basic_string( const charT* s, size_type n = npos )
		{
			assign( s, n );
		}

		//! Clean up allocated data
		~basic_string()
		{
			mData = NULL;
		}
		
		basic_string& operator=( const basic_string& s )
		{
			return assign( s );
		}

		basic_string& operator=( const charT* s )
		{
			return assign( s );
		}

		basic_string& operator=( charT c )
		{
			return assign( &c, 1 );
		}

		basic_string& assign( const basic_string& s )
		{
			mData = s.mData;			
			return *this;
		}

		basic_string& assign( const basic_string& s, size_type pos, size_type n )
		{
			if ( pos == 0 && n >= s.length() )
				assign( s );
			else
				assign( s.data() + pos, n );

			return *this;
		}
		
		basic_string& assign( const charT* s, size_type n = npos )
		{
			if ( n == npos )
				n = strlen( s );
			
			if ( n >= capacity() )
				mData = jh_new Rep( s, n );
			else
				mData->assign( s, n );
			
			return *this;
		}

		const charT& operator[]( const unsigned i ) const
		{
			if ( length() > i )
				return mData->const_at( i );
			else
			{
				// throw out_of_range
				return mData->const_at( length() );  // this should always return a null char.
			}
		}
		
		charT& operator[]( const unsigned i )
		{
			if ( length() > i )
				return mData->at( i );
			else
			{
				// throw out_of_range
				return mData->at( length() );  // this should always return a null char.
			}
		}
		
		basic_string& operator+=( const basic_string& s )
		{
			return append( s );
		}

		basic_string& operator+=( const charT* s )
		{
			return append( s );
		}

		basic_string& operator+=( charT c )
		{
			return append( 1, c );
		}

		basic_string& append( const basic_string& s )
		{
			return append( s.data(), s.length() );
		}

		basic_string& append( const charT* s, size_type n = npos )
		{
			if ( n == npos )
				n = strlen( s );
			
			reserve( length() + n + 1 );
			
			charT *buf = mData->getBuffer();
			memcpy( buf + length(), s, n * sizeof( charT ) );
			mData->setLength( length() + n );
			mData->at( length() ) = null_char;
			return *this;
		}
		
		basic_string& append( size_type n, charT c )
		{
			reserve( length() + n + 1 );
			
			for( size_type i = 0 ; i < n ; i++ )
			{
				mData->at( length() + i ) = c;
			}

			mData->setLength( length() + n );
			mData->at( length() ) = null_char;
			return *this;
		}

		void clear()
		{
			mData.setLength( 0 );
		}

		basic_string substr( size_type pos = 0, size_type n = npos) const
		{
			if ( pos >= length() )
			{
				// Throw out_of_range;
				return basic_string();
			}
			
			if ( pos + n > length() )
				n = length() - pos;
			
			return basic_string( data() + pos, n );
		} 

		const charT* c_str() const
		{
			return mData->getConstBuffer();
		}

		const charT* data() const
		{
			return mData->getConstBuffer();
		}
		
		size_type find( const basic_string& s, size_type pos = 0 ) const
		{
			return find( s.data(), pos, s.length() );
		}

		size_type find( const charT* s, size_type pos = 0 ) const
		{
			return find( s, pos, strlen( s ) );
		}
		
		size_type find( const charT* s, size_type pos, size_type n ) const
		{
			if ( n == npos )
				n = strlen( s );
			
			for( size_type i = pos ; i < length() - n; i++)
			{
				bool match = true;
				for( size_type j = 0 ; j < n ; j++ )
				{
					if ( mData->const_at( j + i ) != s[ j ] )
					{
						match = false;
						break;
					}
				}
					
				if ( match )
					return i;
			}
			return npos;
		}
		
		size_type find( const charT c, size_type pos = 0 ) const
		{
			for( size_type i = pos ; i < length() ; i++ )
			{
				if( mData->const_at( i ) == c )
					return i;
			}
			return npos;
		}
		
		size_type rfind( const basic_string& str, size_type pos = npos ) const
		{
			return rfind( str.data(), pos, str.length() );
		}

		size_type rfind( const charT* s, size_type pos = npos ) const
		{
			return rfind( s, pos, strlen( s ) );
		}
		
		size_type rfind( const charT* s, size_type pos, size_type n ) const
		{
			if ( n == npos )
				n = strlen( s );
			
			if ( pos == npos || pos > length() )
				pos = length();
			
			for( int i = pos ; i >= 0; i--)
			{
				bool match = true;
				for( size_type j = 0 ; j < n ; j++ )
				{
					if ( mData->const_at( j + i ) != s[ j ] )
					{
						match = false;
						break;
					}
				}
					
				if ( match )
					return i;
			}
			return npos;
		}
		
		size_type rfind( const charT c, size_type pos = npos ) const
		{
			if ( pos == npos )
				pos = length() - 1;
			
			for( int i = pos; i >= 0; i-- )
			{
				if ( mData->const_at( i ) == c )
					return i;
			}
			return npos;
		}

		
		size_type find_first_of( const basic_string& s, size_type pos = 0 ) const
		{
			return find_first_of( s.data(), pos, s.length() );
		}

		size_type find_first_of( const charT *s, size_type pos = 0 ) const
		{
			return find_first_of( s, pos, strlen( s ) );
		}

		size_type find_first_of( const charT *s, size_type pos, size_type n ) const
		{
			if ( n == npos )
				n = strlen( s );
				
			for( size_type i = pos ; i < length(); i++)
			{
				for ( size_type j = 0; j < n; j++ )
				{
					if ( mData->const_at( i ) == s[ j ] )
						return i;
				}
			}
			
			return npos;
		}
	
		size_type find_first_of( const charT c, size_type pos = 0 ) const
		{
			return find( c, pos );
		}

		size_type find_last_of( const basic_string& s, size_type pos = npos ) const
		{
			return find_last_of( s.data(), pos, s.length() );
		}

		size_type find_last_of( const charT *s, size_type pos = npos ) const
		{
			return find_last_of( s, pos, strlen( s ) );
		}

		size_type find_last_of( const charT *s, size_type pos, size_type n ) const
		{
			if ( n == npos )
				n = strlen( s );

			if ( pos == npos )
				pos = length() - 1;

			for( int i = pos; i >= 0; i--)
			{
				for ( size_type j = 0; j < n; j++ )
				{
					if ( mData->const_at( i ) == s[ j ] )
						return i;
				}
			}

			return npos;
		}

		size_type find_last_of( const charT c, size_type pos = npos ) const
		{
			return rfind( c, pos );
		}
		
		basic_string& insert( size_type pos1, const basic_string& str )
		{
			return insert( pos1, str.data(), str.length() );
		}
		
		basic_string& insert( size_type pos1, const basic_string& str, size_type pos2, size_type n )
		{
			return insert( pos1, str.data() + pos2, n );
		}
		
		basic_string& insert( size_type pos1, const charT* s, size_type n )
		{
			if ( n == npos )
				n = strlen( s );
			
			if ( pos1 >= length() )
			{
				// throw out_of_range
				return *this;
			}
			
			Rep *nr = mData->copyOnWrite();
			if ( nr != NULL )
				mData = nr;

			reserve( length() + n + 1 );
			
			for( int i = length() - 1; i >= (int)pos1; i-- )
			{
				mData->at( i + n ) = mData->at( i );
			}
			
			memcpy( mData->getBuffer() + pos1, s, n * sizeof( charT ) );
			
			mData->setLength( length() + n );
			mData->at( length() ) = null_char;
			return *this;
		}
		
		basic_string& insert( size_type pos1, const charT* s )
		{
			return insert( pos1, s, strlen( s ) );
		}
		
		basic_string& insert( size_type pos1, size_type n, charT c )
		{			
			if ( pos1 >= length() )
			{
				// throw out_of_range
				return *this;
			}

			Rep *nr = mData->copyOnWrite();
			if ( nr != NULL )
				mData = nr;

			reserve( length() + n + 1 );
			
			for( int i = length() - 1; i >= (int)pos1; i-- )
			{
				mData->at( i + n ) = mData->at( i );
			}
			
			for( unsigned i = 0; i < n; i++ )
				mData->at( pos1 + i ) = c;

			mData->setLength( length() + n );
			mData->at( length() ) = null_char;
			return *this;
		}
		
		basic_string& replace( size_type pos1, size_type n1, const basic_string& str )
		{
			return replace( pos1, n1, str.data(), str.length() );
		}

		basic_string& replace( size_type pos1, size_type n1, const basic_string& str, size_t pos2, size_t n2 )
		{
			return replace( pos1, n1, str.data() + pos2, n2 );
		}

		basic_string& replace( size_type pos1, size_type n1, const charT* s, size_type n2 )
		{
			int new_len = length() - n1 + n2;

			if ( pos1 >= length() )
				return *this;
			
			Rep *nr = mData->copyOnWrite();
			if ( nr != NULL )
				mData = nr;
			
			reserve( new_len );
			erase( pos1, n1 );

			if ( pos1 >= length() )
				append( s, n2 );
			else
				insert( pos1, s, n2 );
			
			return *this;
		}

		basic_string& replace( size_type pos1, size_type n1, const charT* s )
		{
			return replace( pos1, n1, s, strlen( s ) );
		}

		basic_string& replace( size_type pos1, size_type n1, size_type n2, charT c )
		{
			int new_len = length() - n1 + n2;

			if ( pos1 >= length() )
				return *this;
						
			Rep *nr = mData->copyOnWrite();
			if ( nr != NULL )
				mData = nr;
			
			reserve( new_len );
			erase( pos1, n1 );

			if ( pos1 >= length() )
				append( n2, c );
			else
				insert( pos1, n2, c );
			
			return *this;
		}
		
		basic_string& erase( size_type pos = 0, size_type n = npos )
		{
			Rep *nr = mData->copyOnWrite();
			if ( nr != NULL )
				mData = nr;
				
			if ( pos >= length() )
				return *this;
			
			// if we erase all the way to the end just truncate 
			if ( n == npos || pos + n == length() )
			{
				if ( pos < length() )
				{
					mData->setLength( pos );
					mData->at( pos ) = null_char; 
				}
			}
			// otherwise we need to copy a chunck from the end back into the whole.
			else
			{
				if ( pos + n > length() )
					n = length() - pos;
				
				for( unsigned i = pos + n; i < length() ; i++)
				{
					mData->at( i - n ) = mData->at( i );
				}
				
				mData->setLength( mData->length() - n );
				mData->at( length() ) = null_char; 
			}
			return *this;
		}

		int compare( const basic_string& rhs ) const
		{
			return compare( 0, npos, rhs.mData->getConstBuffer(), rhs.length() );
		}

		int compare( const char* cptr ) const
		{
			return compare( 0, npos, cptr );
		}

		int compare( size_type pos, size_type n, const charT* s, size_type rlen = npos ) const
		{
			size_type str_len = strlen( s );
			
			if ( str_len < rlen )
				rlen = str_len;
			
			if ( n == npos )
				n = length() - pos;
			
			size_type i = pos;
			while ( i <= n && i <= rlen )
			{
				if ( mData->const_at( i ) > s[ i ] )
				{
					return 1;
				}
				else if ( mData->const_at( i ) < s[ i ] )
				{
					return -1;
				}
				i++;
			}
			
			if ( n > rlen )
				return -1;
			else if ( n < rlen )
				return 1;
			else
				return 0;
		}
		
		void reserve( size_t res = 0 )
		{
			if ( res > capacity() )
			{
				if ( res < ( capacity() * 2 ) )
					res = capacity() * 2;
				
				Rep *new_rep = jh_new Rep( res );
				new_rep->assign( mData->getBuffer(), mData->length() );
				mData = new_rep;
			}
		}
		
		size_type capacity() const
		{
			if ( mData == NULL )
				return 0;
			else
				return mData->capacity();
		}
		
		void resize( size_type n, charT c = 0 )
		{
			reserve( n + 1 );
			
			for ( size_type i = length(); i < n; i++ )
				mData->at( i ) = c;
			
			mData->setLength( n );
			mData->at( length() ) = null_char;
		}
	
		//! How much is in use?
		size_type length() const 
		{ 
			return mData->length();
		}

		size_type size() const 
		{
			return mData->length();
		}

		//! Is this empty?
		bool empty() const 
		{ 
			if ( mData->length() == 0 )
				return true;
			else	
				return false;
		}
		
	private:		
		class Rep : public RefCount 
		{
		public:
			Rep( int capacity ) : mCapacity( capacity ), mLength( 0 ), mData( NULL ) 
			{
				if ( capacity == 0 )
					mData = jh_new charT[ 1 ];
				else
					mData = jh_new charT[ capacity ];
					
				mData[ 0 ] = null_char;					
			}
			
			Rep( const charT *s, size_type n = npos ) : mCapacity( 0 ), mLength( 0 ), mData( NULL ) 
			{
				if ( n == npos )
					n = strlen( s );
				mCapacity = n + 1;
				mData = jh_new charT[ n + 1 ];
				assign( s, n );
			}
					
			Rep *copyOnWrite() 
			{ 
				if ( getRefCountForDebug() > 1 )
				{
					Rep *nr = jh_new Rep( mCapacity );
					nr->assign( mData, mLength );
					return nr;
				}
				else
					return NULL;
			}
					
			size_type capacity() const { return mCapacity; }
			size_type length() const { return mLength; }
			
			void assign( const charT *s, size_type n = npos )
			{
				if ( n == npos )
					n = strlen( s );
				if ( n >= mCapacity )
					n = mCapacity - 1;
				mLength = n;
				memcpy( mData, s, n * sizeof( charT ) );
				mData[ n ] = null_char;
			}
			
			charT *getBuffer() { return mData; }
			const charT *getConstBuffer() const { return mData; }

			charT &at( size_type i ) { return mData[ i ]; }					
			const charT &const_at( size_type i ) const { return mData[ i ]; }					
			
			void setLength( size_type l ) { mLength = l; }
			
		protected:
			~Rep()
			{
				delete [] mData;
			}
						
		private:
			size_type mCapacity;
			size_type mLength;
			charT *mData;
		};
		
		//! The data we are using
		SmartPtr<Rep> mData;
	};
	
	template <class charT>
	basic_string<charT> operator+( const basic_string<charT>& s1, const basic_string<charT>& s2 )
	{
		basic_string<charT> temp = basic_string<charT>( s1 );
		temp.append( s2.data(), s2.length() );
		return temp;
	}

	template <class charT>
	basic_string<charT> operator+( const basic_string<charT>& s1, const charT *s2 )
	{
		basic_string<charT> temp = basic_string<charT>( s1 );
		temp.append( s2 );
		return temp;
	}

	template <class charT>
	basic_string<charT> operator+( const charT* s1, const basic_string<charT>& s2 )
	{
		basic_string<charT> temp = basic_string<charT>( s1 );
		temp.append( s2.data(), s2.length() );
		return temp;
	}
	
	template <class charT>
	bool operator==( const basic_string<charT>& s1, const basic_string<charT>& s2 )
	{
		return s1.compare( s2 ) == 0;
	}

	template <class charT>
	bool operator==( const basic_string<charT>& s1, const charT* s2 )
	{
		return s1.compare( s2 ) == 0;
	}

	template <class charT>
	bool operator==( const charT* s1, const basic_string<charT>& s2 )
	{
		return s2.compare( s1 ) == 0;
	}

	template <class charT>
	bool operator!=( const basic_string<charT>& s1, const basic_string<charT>& s2 )
	{
		return s1.compare( s2 ) != 0;
	}

	template <class charT>
	bool operator!=( const basic_string<charT>& s1, const charT* s2 )
	{
		return s1.compare( s2 ) != 0;
	}

	template <class charT>
	bool operator!=( const charT* s1, const basic_string<charT>& s2 )
	{
		return s2.compare( s1 ) != 0;
	}

	typedef basic_string<char> string;
	typedef basic_string<short> wstring;	
};

#endif // JH_STRING_H
