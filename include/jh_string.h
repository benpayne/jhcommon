/*==============================================================================
  * JetHead Proprietary
  * Copyright 2010 JetHead Development, Inc.
  * UNPUBLISHED WORK
  * ALL RIGHTS RESERVED
  *
  * This software is the confidential and proprietary information of
  * JetHead Development, Inc. ("Proprietary Information").  Any use,
  * reproduction, distribution or disclosure of the software or Proprietary
  * Information, in whole or in part, must comply with the terms of the license
  * agreement, nondisclosure agreement or contract entered into with
  * JetHead providing access to this software.
  *============================================================================*/
#ifndef JH_STRING_H
#define JH_STRING_H

/**
 * \file
 *
 */

#include <string.h>

typedef JetHead::string jh_string;

namespace JetHead
{
	class string
	{
	public:
		typedef unsigned size_type;
		typedef char	charT;
		static const size_type DEFAULT_ALLOCATION = 32;
		static const size_type npos = -1;
		static const charT null_char = 0;
		
		//! Default constructor, nothing contained in the array, 
		string()
		{
			mData = jh_new Rep( DEFAULT_ALLOCATION );
		}

		//! Copy constructor
		string(const string& rhs)
		{
			assign( rhs );
		}

		string( const charT* s, size_type n = npos )
		{
			assign( s, n );
		}

		//! Clean up allocated data
		~string()
		{
			clear();
		}
		
		string& operator=( const string& rhs )
		{
			return assign( rhs );
		}

		string& operator=( const charT* cptr )
		{
			return assign( cptr );
		}

		string& assign( const string& rhs )
		{
			mData = rhs.mData;			
			return *this;
		}

		string& assign( const charT* s, size_type n = npos )
		{
			mData = jh_new Rep( s, n );
			return *this;
		}

		const charT& operator[]( const unsigned i ) const
		{
			if ( capacity() > i )
				return at( i );
			else
			{
				// throw out_of_range
				return mData->at( capacity() - 1 );
			}
		}
		
		charT& operator[]( const unsigned i )
		{
			if ( capacity() > i )
				return mData->at( i );
			else
			{
				// throw out_of_range
				return mData->at( capacity() - 1 );
			}
		}
		
		bool operator==( const string& s ) const
		{
			return compare( s ) == 0;
		}

		bool operator!=( const string& s ) const
		{
			return compare( s ) != 0;
		}
		
		string& operator+=( const string& s )
		{
			return append( s );
		}

		string& operator+=( const charT* s )
		{
			return append( s );
		}

		string& operator+=( charT c )
		{
			return append( c );
		}

		string& append( const string& s )
		{
			append( s.data(), s.length() );
			return *this;
		}

		string& append( const charT* s  )
		{
			append( s, strlen( s ) );
			return *this;
		}

		string& append( const charT* s, size_type n = npos )
		{
			if ( n == npos )
				n = strlen( s );
			
			reserve( length() + n )
			
			charT *buf = mData->getBuffer();
			memcpy( buf + length(), s, n * sizeof( charT ) );
			return *this;
		}
		
		string& append( const charT c )
		{
			append( 1, c );
			return *this;
		}

		string& append( size_type n, charT c )
		{
			reserve( length() + n )
			
			for( int i = 0 ; i < n ; i++ )
			{
				mData->at( length() + i ) = c;
			}

			mData->setLength( length() + n );
			return *this;
		}

		string operator+( const string& rhs ) const
		{
			string temp = string( *this );
			temp.append( rhs.data(), rhs.length() );
			return temp;
		}
	
		void clear()
		{
			mData = NULL;
		}

		string substr( size_type pos = 0, size_type n = npos) const
		{
			if ( pos >= length() )
			{
				// Throw out_of_range;
				return string();
			}
			
			if ( pos + n > length() )
				n = length() - pos;
			
			return string( data() + pos, n );
		} 

		const char* c_str() const
		{
			// fix null termination
			return mData->getBuffer();
		}

		const char* data() const
		{
			return mData->getBuffer();
		}
		
		size_type find( const string& s, size_type pos = 0 ) const
		{
			return find( s->data(), pos, s->length() );
		}

		size_type find( const charT* s, size_type pos = 0 ) const
		{
			return find( s, pos, strlen( s ) );
		}
		
		size_type find( const charT* s, size_type pos, size_type n ) const
		{
			if ( n == npos )
				n = strlen( s );
			
			for( int i = pos ; i < length() - n; i++)
			{
				bool match = true;
				for( int j = 0 ; j < len ; j++ )
				{
					if ( mData->at( j + i ) != s[ j ] )
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
		
		size_type find(const char c, size_type pos = 0) const
		{
			for(unsigned i = pos ; i < mSize ; i++)
			{
				if(mData[i] == c)
				{
					return i;
				}
			}
			return npos;
		}
		
		size_type rfind( const string& str, size_type pos = npos ) const
		{
			return rfind( str->data(), pos, str->length() );
		}

		size_type rfind( const charT* s, size_type pos = 0 ) const
		{
			return rfind( s, pos, strlen( s ) );
		}
		
		size_type rfind( const charT* s, size_type pos, size_type n ) const
		{
			if ( n == npos )
				n = strlen( s );
			
			for( int i = pos ; i < length() - n; i++)
			{
				bool match = true;
				for( int j = 0 ; j < len ; j++ )
				{
					if ( mData->at( j + i ) != s[ j ] )
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
				pos = length();
			
			for( int i = pos; i != 0; i-- )
			{
				if ( mData->at( i ) == c )
					return i;
			}
			return npos;
		}

		
		size_type find_first_of( const string& s, size_type pos = 0 ) const
		{
			return find_first_of( s->data(), pos, s->length() );
		}

		size_type find_first_of( const charT *s, size_type pos = 0 ) const
		{
			return find_first_of( s, pos, strlen( s ) );
		}

		size_type find_first_of( const charT *s, size_type pos, size_type n ) const
		{
			if ( n == npos )
				n = strlen( s );
				
			for( int i = pos ; i < length(); i++)
			{
				for ( int j = 0; j < n; j++ )
				{
					if ( mData->at( i ) == s[ j ] )
						return i;
				}
			}
		}
	
		size_type find_first_of( const char c, size_type pos = 0 ) const
		{
			return find( c, pos );
		}

		size_type find_last_of( const string& str ) const
		{
			for(unsigned i = mSize ; i != 0 ; i--)
			{
				if(str[0] == mData[i])
				{
					bool match = true;
					for(unsigned j = 0 ; j < str.size() ; j++)
					{
						if(i + j > mSize)
						{
							match = false;
							break;
						}
						if(str[j] != mData[i + j])
						{
							match = false;
							break;
						}
					}
					if(match)
					{
						return i;
					}
				}
			}
			return npos;
		}

		size_type find_last_of( const charT c ) const
		{
			return rfind(c);
		}
		
		string& replace ( size_t pos1, size_t n1, const string& str )
		{
		}
		
		string& replace( size_type pos, size_type n1, size_type n2, charT c )
		{
			int new_len = length() - n1 + n2;

			reserve( new_len );

			if ( pos >= length() )
				return *this;
			
			erase( pos, n1 );

			if ( pos + n1 >= length() )
				append( n2, c );
			else
				insert( pos, n2, c );
			
			return *this;
		}

		string& erase( size_type pos = 0, size_type n = npos )
		{
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
				
				for( int i = pos + n; i < length() ; i++)
				{
					mData[ i - n ] = mData[ i ];
				}
				
				mData->setLength( mData->length - n );
			}
			return *this;
		}

		int compare( const string& rhs ) const
		{
			return compare( 0, npos, rhs.mData->getBuffer(), rhs.length() );
		}

		int compare( const char* cptr )
		{
			return compare( 0, npos, cptr );
		}

		int compare( size_type pos, size_type n, const charT* rstr, size_type rlen = npos ) const
		{
			int str_len = strlen( s );
			int llen = 0;
			
			if ( str_len < rlen )
				rlen = str_len;
			
			if ( n == npos )
				n = length();
			
			llen = n - pos;

			while ( i <= llen & i <= rlen )
			{
				if ( mData->at( i ) > cptr[ i ] )
				{
					return 1;
				}
				else if ( mData->at( i ) < cptr[ i ] )
				{
					return -1;
				}
				i++;
			}
			
			return 0;
		}
		
		void reserve( size_t res = 0 )
		{
			// TODO...
		}
		
		size_type capacity() const
		{
			return mData->capacity();
		}
		
		void resize( size_type n, charT c = 0 )
		{
			reserve( n );
			
			for ( int i = length(); i < n; i++ )
				mData->at( i ) = c;
			
			mData->setLength( n );
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
		class Rep : public RefCount {
		public:
			Rep( int capacity ) : mCapacity( capacity ), mLength( 0 ), mData( NULL ), mRefCount( 0 ) 
			{
				mData = jh_new charT[ capacity ];
			}
			
			Rep( const charT *s, size_type n = npos ) : mCapacity( 0 ), mLength( 0 ), mData( NULL ), mRefCount( 0 ) 
			{
				if ( n == npos )
					n = strlen( str );
				mCapacity = n + 1;
				mLength = n;
				mData = jh_new charT[ n + 1 ];
				memcpy( mData, s, n * sizeof( charT ) );
				mData[ n ] = null_char;
			}
		
			void AddRef() const
			{ 
				mRefCount++;
			}
	
			void Release() const
			{
				mRefCount--;
				if ( mRefCount == 0 )
					delete this;
			}
			
			bool copyOnWrite() { return mRefCount > 1; }
					
			size_type capacity() { return mCapacity; }
			size_type length() { return mLength; }
			
			charT *getBuffer() { return mData; }
			charT &at( size_type i ) { return mData[ i ]; }					
			
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
			int mRefCount;
		};
				
		//! The data we are using
		SmartPtr<Rep> mData;
	};
};

#endif // JH_STRING_H
