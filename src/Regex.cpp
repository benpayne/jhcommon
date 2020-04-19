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
 
#include "jh_Regex.h"
#include "jh_memory.h"
#include "logging.h"
#include <string.h>
#include <sstream>

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOISE );

Regex::Regex() : mState( Regex::STATE_INIT )
{
}

Regex::Regex( const JHSTD::string &regex ) : mState( Regex::STATE_INIT )
{
	prepare( regex );
}

Regex::~Regex()
{
}

#define PARSE_ERROR( str ) \
{ \
	mErrorData.mErrorString = str; \
	LOG_NOTICE( "Parse error at char %d, %s", i, str ); \
	return JetHead::kInvalidRequest; \
}

#define ADD_TERMINAL( start, end ) \
{ \
	if ( state == IN_CLASS_THRU ) {\
		if ( start != end ) \
			PARSE_ERROR( "Invalid range definition" ) \
		else { \
			state = IN_CLASS; \
			new_element = cur_node->getLastChild(); \
			new_element->mTermEndChar = start; \
		} \
	} else { \
		new_element = jh_new Element; \
		new_element->mType = TYPE_TERMINAL; \
		new_element->mTermStartChar = (start); \
		new_element->mTermEndChar = (end); \
		cur_node->insertChild( new_element ); \
		LOG_INFO( "Adding Term %c -> %c", start, end ); \
	} \
}

#define CHECK_CLASS \
{ \
	bool class_created = false; \
	if ( cur_node->mType != TYPE_CLASS ) \
	{ \
		new_element = jh_new Element; \
		new_element->mType = TYPE_CLASS; \
		class_created = true; \
		new_element->mClassInvert = false; \
		cur_node->insertChild( new_element ); \
		cur_node = new_element; \
	}

#define END_CLASS \
	if ( class_created ) \
		cur_node = cur_node->mParent; \
} 



enum {
	IN_UNKNOWN,
	IN_CLASS,
	IN_CLASS_THRU,
	IN_RANGE_MAX,
	IN_RANGE_MIN
};

JetHead::ErrCode	Regex::prepare( const JHSTD::string &regex )
{
	int rlen = regex.length();
	Element *new_element = NULL;
	Element *cur_node = &mRoot;
	
	cur_node->mType = TYPE_SEQUENCE;
	int state = IN_UNKNOWN;
	int range_state;
	int in_group = 0;
	
	for ( int i = 0; i < rlen; i++ )
	{
		switch ( regex[ i ] ) 
		{
			case '\\':
				i += 1;
		
				switch( regex[ i ] )
				{
					case 'w':
						CHECK_CLASS
						ADD_TERMINAL( 'a', 'z' );
						ADD_TERMINAL( 'A', 'Z' );
						ADD_TERMINAL( '0', '9' );
						ADD_TERMINAL( '_', '_' );
						END_CLASS
						break;

					case 'W':
						CHECK_CLASS
						cur_node->mClassInvert = true;
						ADD_TERMINAL( 'a', 'z' );
						ADD_TERMINAL( 'A', 'Z' );
						ADD_TERMINAL( '0', '9' );
						ADD_TERMINAL( '_', '_' );
						END_CLASS
						break;
						
					case 's':
						CHECK_CLASS
						ADD_TERMINAL( ' ', ' ' );
						ADD_TERMINAL( '\t', '\t' );
						ADD_TERMINAL( '\n', '\n' );
						ADD_TERMINAL( '\r', '\r' );
						ADD_TERMINAL( '\f', '\f' );
						END_CLASS
						break;

					case 'S':
						CHECK_CLASS
						cur_node->mClassInvert = true;
						ADD_TERMINAL( ' ', ' ' );
						ADD_TERMINAL( '\t', '\t' );
						ADD_TERMINAL( '\n', '\n' );
						ADD_TERMINAL( '\r', '\r' );
						ADD_TERMINAL( '\f', '\f' );
						END_CLASS
						break;

					case 'd':
						CHECK_CLASS
						ADD_TERMINAL( '0', '9' );
						END_CLASS
						break;

					case 'D':
						CHECK_CLASS
						cur_node->mClassInvert = true;
						ADD_TERMINAL( '0', '9' );
						END_CLASS
						break;

					case 't':
						ADD_TERMINAL( '\t', '\t' );
						break;

					case 'n':
						ADD_TERMINAL( '\n', '\n' );
						break;
					
					case 'r':
						ADD_TERMINAL( '\r', '\r' );
						break;
				
					case 'f':
						ADD_TERMINAL( '\f', '\f' );
						break;
						
					case '0':
					case 'x':
						break;
						
					default:
						ADD_TERMINAL( regex[ i ], regex[ i ] );
						break;
				}
				break;
				
			case '.':
				ADD_TERMINAL( 0, 127 );
				break;
				
			case '*':
				if ( state == IN_CLASS )
					PARSE_ERROR( "Non-excaped * in charater class" )
				new_element = cur_node->getLastChild();
				new_element->mRepeatMin = 0;
				new_element->mRepeatMax = -1;
				assert( new_element->mType == TYPE_CLASS ||new_element->mType == TYPE_OR || new_element->mType == TYPE_TERMINAL );
				break;
				
			case '+':
				if ( state == IN_CLASS )
					PARSE_ERROR( "Non-excaped + in charater class" )
				new_element = cur_node->getLastChild();
				new_element->mRepeatMin = 1;
				new_element->mRepeatMax = -1;
				assert( new_element->mType == TYPE_CLASS ||new_element->mType == TYPE_OR || new_element->mType == TYPE_TERMINAL );
				break;

			case '?':
				if ( state == IN_CLASS )
					PARSE_ERROR( "Non-excaped ? in charater class" )
				new_element = cur_node->getLastChild();
				new_element->mRepeatMin = 0;
				new_element->mRepeatMax = 1;
				assert( new_element->mType == TYPE_CLASS ||new_element->mType == TYPE_OR || new_element->mType == TYPE_TERMINAL );
				break;
				
			case '{':
				if ( state == IN_CLASS )
					PARSE_ERROR( "Non-excaped { in charater class" )
				i += 1;
				new_element = cur_node->getLastChild();
				new_element->mRepeatMin = 0;
				new_element->mRepeatMax = 0;
				assert( new_element->mType == TYPE_CLASS ||new_element->mType == TYPE_OR || new_element->mType == TYPE_TERMINAL );
				range_state = IN_RANGE_MIN;
				while( regex[ i ] != '}' )
				{
					switch( regex[ i ] )
					{
						case ',':
							if ( range_state == IN_RANGE_MIN )
								range_state = IN_RANGE_MAX;
							else
								PARSE_ERROR( "Too many commons in repetition range" )
							break;
							
						default:
							if ( regex[ i ] >= '0' && regex[ i ] <= '9' )
							{
								if ( range_state == IN_RANGE_MIN )
								{
									new_element->mRepeatMin *= 10;
									new_element->mRepeatMin += regex[ i ] - '0';
								}
								else if ( range_state == IN_RANGE_MAX )
								{
									new_element->mRepeatMax *= 10;
									new_element->mRepeatMax += regex[ i ] - '0';
								}
								else
									PARSE_ERROR( "Internal Error" )
							}
							else
								PARSE_ERROR( "Non-numeric value in repetition range" )
					}
					
					i += 1;
				}

				if ( range_state == IN_RANGE_MIN )
					new_element->mRepeatMax = new_element->mRepeatMin;				
				
				LOG_NOTICE( "Range set to %d -> %d", new_element->mRepeatMin, new_element->mRepeatMax );
				break;

			case '[':
				if ( state == IN_CLASS )
					PARSE_ERROR( "Non-excaped [ in charater class" )

				state = IN_CLASS;
				new_element = jh_new Element;
				new_element->mType = TYPE_CLASS;
				if ( regex[ i + 1 ] == '^' )
				{
					new_element->mClassInvert = true;
					i += 1;
				}
				else
					new_element->mClassInvert = false;
				cur_node->insertChild( new_element );
				cur_node = new_element;
				break;
								
			case ']':
				if ( state == IN_CLASS )
				{
					cur_node = cur_node->mParent;
					state = IN_UNKNOWN;
				}
				else if ( state == IN_CLASS_THRU )
					PARSE_ERROR( "Non-escaped square bracket at end of charater range" )
				else
					ADD_TERMINAL( regex[ i ], regex[ i ] );
				break;

			case '-':
				if ( state == IN_CLASS )
				{
					new_element = cur_node->getLastChild();
					if ( new_element->mTermStartChar != new_element->mTermEndChar )
						PARSE_ERROR( "Invalid terminal for start of charater range" )
					state = IN_CLASS_THRU;
				}
				else
					ADD_TERMINAL( regex[ i ], regex[ i ] );
				break;
			
			case '(':
				if ( state == IN_CLASS )
				{
					ADD_TERMINAL( regex[ i ], regex[ i ] );
				}
				else
				{
					in_group += 1;
					new_element = jh_new Element;
					new_element->mType = TYPE_OR;
					cur_node->insertChild( new_element );
					cur_node = new_element;
	
					new_element = jh_new Element;
					new_element->mType = TYPE_SEQUENCE;
					cur_node->insertChild( new_element );
					cur_node = new_element;
				}
				break;

			case '|':
				if ( in_group > 0 )
				{
					new_element = jh_new Element;
					new_element->mType = TYPE_SEQUENCE;
					cur_node->mParent->insertChild( new_element );
					cur_node = new_element;
				}
				else
					ADD_TERMINAL( regex[ i ], regex[ i ] );
				break;

			case ')':
				if ( in_group > 0 )
				{
					in_group -= 1;
					cur_node = cur_node->mParent->mParent;
				}
				else
					ADD_TERMINAL( regex[ i ], regex[ i ] );
				break;
					
			default:
				ADD_TERMINAL( regex[ i ], regex[ i ] );
				break;
		}
	}
			
	mState = Regex::STATE_PREPARED;
	return JetHead::kNoError;
}

JetHead::ErrCode	Regex::dumpTree( std::string &dump )
{
	if ( mState != Regex::STATE_PARSED && mState != Regex::STATE_PREPARED )
		return JetHead::kNotInitialized;

	Element *cur_node = &mRoot;
	
	while( cur_node != NULL )
	{
		dumpElement( cur_node, dump );
		
		if ( cur_node->mChild != NULL )
		{
			cur_node = cur_node->mChild;
		}
		else if ( cur_node->mNext != NULL )
		{
			cur_node = cur_node->mNext;
		}
		else 
		{
			bool done = false;
			while ( !done )
			{
				if ( cur_node->mParent == NULL )
				{
					done = true;
					cur_node = NULL;
				}
				else
				{
					cur_node = cur_node->mParent;
					if ( cur_node->mNext != NULL )
					{
						cur_node = cur_node->mNext;
						done = true;
					}
				}
			}
		}
	}
	
	return JetHead::kNoError;
}

void	Regex::dumpElement( Element *cur_node, std::string &dump )
{
	std::stringstream strm( std::stringstream::in | std::stringstream::out );
	
	switch ( cur_node->mType )
	{
		case TYPE_SEQUENCE:
			LOG_INFO( "Seq" );
			strm << "[S]";
			break;
			
		case TYPE_OR:
			LOG_INFO( "Or" );
			strm << "[O]";
			if ( cur_node->mRepeatMin != cur_node->mRepeatMax )
				strm << "{" << cur_node->mRepeatMin << "," << cur_node->mRepeatMax << "}";
			break;

		case TYPE_TERMINAL:
			LOG_INFO( "Terminal" );
			strm << "[T:";
			if ( cur_node->mTermStartChar == 0 && cur_node->mTermEndChar == 127 )
				strm << ".";
			else if ( cur_node->mTermStartChar == cur_node->mTermEndChar )
			{
				if ( cur_node->mTermStartChar < ' ' )
					strm << "0x" << std::hex << (int)cur_node->mTermStartChar << std::dec;
				else
					strm << cur_node->mTermStartChar;
			}
			else
			{
				strm << cur_node->mTermStartChar << "-" << cur_node->mTermEndChar;
			}
			strm << "]";

			if ( cur_node->mRepeatMin != cur_node->mRepeatMax )
				strm << "{" << cur_node->mRepeatMin << "," << cur_node->mRepeatMax << "}";
			break;

		case TYPE_CLASS:
			LOG_INFO( "Class" );
			
			if ( cur_node->mClassInvert )
				strm << "[C^]";
			else
				strm << "[C]";
				
			if ( cur_node->mRepeatMin != 1 || cur_node->mRepeatMax != 1 )
				strm << "{" << cur_node->mRepeatMin << "," << cur_node->mRepeatMax << "}";
			break;
			
		default:
			LOG_ERR_FATAL( "Bad element Type" );
			break;
	}

	dump.append( strm.str() );
}

bool	Regex::parse( const JHSTD::string &string )
{	
	if ( mState != Regex::STATE_PARSED && mState != Regex::STATE_PREPARED )
		return JetHead::kNotInitialized;
	
	Element *cur_node = &mRoot;
	ParseData data( string );
	
	bool res = processSubTree( cur_node, &data );

	if ( res )
		mState = Regex::STATE_PARSED;

	return res;
}

bool	Regex::processSubTree( Element *cur_node, ParseData *data )
{
	Element *top_node = cur_node;
	
	while( cur_node != NULL )
	{
		if ( processElement( cur_node, data ) == false )
			return false;
		
		if ( cur_node->mType != TYPE_CLASS && cur_node->mType != TYPE_OR && cur_node->mChild != NULL )
		{
			cur_node = cur_node->mChild;
		}
		else if ( cur_node->mNext != NULL )
		{
			cur_node = cur_node->mNext;
		}
		else 
		{
			bool done = false;
			while ( !done )
			{
				if ( top_node == cur_node->mParent )
				{
					done = true;
					cur_node = NULL;
				}
				else
				{
					cur_node = cur_node->mParent;
					if ( cur_node->mNext != NULL )
					{
						cur_node = cur_node->mNext;
						done = true;
					}
				}
			}
		}
	}
	
	return true;
}


#define CUR_CHAR	data->string[ data->cur_pos ]

bool	Regex::processTerminal( Element *cur_node, ParseData *data )
{
	if ( cur_node->mType == TYPE_TERMINAL )
	{
		if ( CUR_CHAR >= cur_node->mTermStartChar && CUR_CHAR <= cur_node->mTermEndChar )
			return true;
	}
	else if ( cur_node->mType == TYPE_CLASS )
	{
		bool invert = cur_node->mClassInvert;
		bool match = invert;
		Element *child_node = cur_node->mChild;
		
		while ( child_node != NULL )
		{
			if ( !invert )
			{
				if ( CUR_CHAR >= child_node->mTermStartChar && CUR_CHAR <= child_node->mTermEndChar )
					return true;
			}
			else
			{
				if ( CUR_CHAR >= child_node->mTermStartChar && CUR_CHAR <= child_node->mTermEndChar )
					return false;
			}
			
			child_node = child_node->mNext;
		}
		
		if ( match )
			return true;
		else
			return false;
	}

	return false;
}

void	Regex::backtrackGroups( ParseData *data )
{
	for ( int i = 0; i < (int)mGroups.size(); i++ )
	{
		if ( mGroups[ i ].end_pos > data->cur_pos )
		{
			std::string &s = mGroups[ i ].string;
			s.erase( s.length() - ( mGroups[ i ].end_pos - data->cur_pos ) );
			mGroups[ i ].end_pos = data->cur_pos;
		}
	}
}

bool	Regex::processElement( Element *cur_node, ParseData *data )
{	
	if ( cur_node->mType == TYPE_OR )
	{
		cur_node->mGroupNum = mGroups.size();
		mGroups.resize( mGroups.size() + 1 );
		mGroups[ cur_node->mGroupNum ].start_pos = data->cur_pos;

		LOG_INFO( "Group %d open at %d", cur_node->mGroupNum, data->cur_pos );

		Element *child_node = cur_node->mChild;
		
		while ( child_node != NULL )
		{
			if ( processSubTree( child_node, data ) )
			{
				int len = data->cur_pos - mGroups[ cur_node->mGroupNum ].start_pos;

				mGroups[ cur_node->mGroupNum ].end_pos = data->cur_pos;
				mGroups[ cur_node->mGroupNum ].string.assign( data->string, 
					mGroups[ cur_node->mGroupNum ].start_pos, len );
		
				LOG_INFO( "Group %d closing, with size of %d and value of %s", cur_node->mGroupNum, len, mGroups[ cur_node->mGroupNum ].string.c_str() );
				LOG_INFO( "Group %d start %d, end %d", cur_node->mGroupNum, mGroups[ cur_node->mGroupNum ].start_pos, mGroups[ cur_node->mGroupNum ].end_pos );
				return true;
			}
			child_node = child_node->mNext;
		}
		
		return false;
	}
	if ( cur_node->mType == TYPE_SEQUENCE )
	{
		return true;
	}
	else if ( cur_node->mType == TYPE_TERMINAL || cur_node->mType == TYPE_CLASS )
	{
		assert( cur_node->mParent->mType == TYPE_SEQUENCE );
		
		bool backtrack = false;
		bool res;
		int start_pos = data->cur_pos;
		
		do {
			data->match_count = 0;
			res = processTerminal( cur_node, data );
		
			while ( ( cur_node->mRepeatMax == -1 || data->match_count < cur_node->mRepeatMax ) && res == true )
			{
				data->cur_pos += 1;
				data->match_count += 1;
				res = processTerminal( cur_node, data );
			}
		
			backtrack = false;
			
			if ( res == false )
			{
				if ( data->match_count >= cur_node->mRepeatMin )
					return true;
				else if ( start_pos > data->backtrack_pos )
				{
					LOG_NOTICE( "backtrack %d %d", data->cur_pos, data->backtrack_pos );
					start_pos -= 1;
					data->cur_pos = start_pos;
					backtrack = true;
					backtrackGroups( data );
				}
			}
		} while ( backtrack );

		if ( res && data->match_count >= cur_node->mRepeatMin )
		{
			data->cur_pos += 1;
			data->match_count += 1;
			data->backtrack_pos = data->cur_pos - ( data->match_count - cur_node->mRepeatMin );
			return true;
		}
	}
	
	return false;
}

const JHSTD::string &Regex::getData( int i )
{
	static JHSTD::string null_string( "" );

	if ( mState != Regex::STATE_PARSED )
		return null_string;

	if ( i >= (int)mGroups.size() )
		return null_string;

	return mGroups[ i ].string;
}

