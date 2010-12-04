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
 
#ifndef JH_REGEX_H_
#define JH_REGEX_H_

#include "jh_vector.h"
#include "jh_string.h"
#include "JetHead.h"

/**
	Regex is a regular expression parsing class.  It takes a regular expression
	in the prepare method.  This method will build a tree representing the 
	RE.  You can call parse use the tree to match a string.  This means that
	you can prepare and RE and then us it many times.  Once parsed you can get
	at the string data captured by calling getData and getNumGroups will tell
	you how many groups have been found.  
	
	This class attempts to match Perl RE syntax.  Though not much testing has
	been done to ensure similar results.  RE process is faily complexe and 
	there is alot of "grey areas" if how data should be processed.  We've 
	errored on the side if simplicity.  So YMMV...
	
	If you looking for an easy way to parse some text, you might want to 
	consider rolling your own parse.  If you have complexe strings to parse
	then this class might be your choice.  Also keep in mind that how you write
	your RE will make a large difference in performance.
 */ 
class Regex
{
public:
	Regex();
	Regex( const JHSTD::string &regex );
	~Regex();
	
	/**
	 * Prepare a regular expression for use.  This take the RE and turns it 
	 *  into an internal represtation so that it can be reused quickly to match
	 *  many strings.  This call must be made before calling parse.
	 */
	JetHead::ErrCode	prepare( const JHSTD::string &regex );

	/**
	 * Parse a string and look for a match to the RE that was prepared by a call
	 *  to prepare repare a regular expression for use.  This starts at the 
	 *  begining of the string and matchs the entire RE.  If additional text
	 *  remains in string after the full match of the RE is found then the 
	 *  result will still be true.
	 */
	 bool	parse( const JHSTD::string &string );

	/**
	 * If the RE contains any groups, the strings for the match of all groups
	 *  can be accessed with this call.  You must have successfully parsed a 
	 *  string for this call to return valid data.  You can use getNumGroups to 
	 *  know how many groups were found in the parse.  
	 */
	const JHSTD::string &getData( int i );
	
	/**
	 * Get the number of groups that were found.  This is used to know what 
	 *  range of number are appropriate to call get data with.  Any number 
	 *  from 0 - (n-1) should return a valid result.
	 */ 
	int getNumGroups();
	
	struct RegexPrepareError
	{
		std::string mErrorString;
		int			mErrorPos;
	};

	/**
	 * In case of a error preparing the RE, this call will return an object
	 *  that contains a string describing the error condition and the position
	 *  of the error.
	 */ 
	const RegexPrepareError &getPrepareError() { return mErrorData; }

	/**
	 * Used for testing code only.  This dumps a string representation of the 
	 *  Internal tree generated after a prepare.
	 */
	JetHead::ErrCode	dumpTree( std::string &dump );
		
private:	
	enum ElmentType {
		TYPE_UNKNOWN,
		TYPE_SEQUENCE,
		TYPE_OR,
		TYPE_TERMINAL,
		TYPE_CLASS
	};
	
	struct Element {
		Element() : mParent( NULL ), mNext( NULL ), mChild( NULL ), 
			mType( TYPE_UNKNOWN ), mRepeatMin( 1 ), mRepeatMax( 1 ), 
			mClassInvert( false ), mGroupNum( -1 ) {}
		~Element() { delete mChild; delete mNext; }
		
		Element *	mParent;
		Element *	mNext;
		Element *	mChild;

		void insertChild( Element *e )
		{
			e->mParent = this;
			e->mNext = NULL;
			if ( mChild != NULL )
			{
				Element *c = mChild;
				while( c->mNext != NULL )
					c = c->mNext;
			
				c->mNext = e;
			}
			else
				mChild = e;
		}
		
		Element *getLastChild()
		{
			if ( mChild != NULL )
			{
				Element *c = mChild;
				while( c->mNext != NULL )
					c = c->mNext;
	
				return c;
			}

			return NULL;
		}
		
		ElmentType	mType;
		char	mTermStartChar;
		char	mTermEndChar;
		int		mRepeatMin;
		int		mRepeatMax;
		bool	mClassInvert;
		int		mGroupNum;
	};

	struct GroupData
	{
		GroupData() : start_pos( 0 ), end_pos( 0 ) {}
		int start_pos;
		int end_pos;
		std::string string;
	};
	
	struct ParseData
	{
		ParseData( const JHSTD::string &s ) : string( s ), cur_pos( 0 ), 
			backtrack_pos( 0 ), match_count( 0 ) {}
		
		JHSTD::string string;
		int cur_pos;
		int backtrack_pos;
		int match_count;
		//JetHead::vector<GroupData>	groups;
	};

	void dumpElement( Element *cur_node, std::string &dump );
	bool processElement( Element *cur_node, ParseData *data );
	bool processTerminal( Element *cur_node, ParseData *data );
	bool processChildrenComplete( Element *cur_node, ParseData *data );
	bool processSubTree( Element *cur_node, ParseData *data );
	void backtrackGroups( ParseData *data );
	
	enum State {
		STATE_INIT,
		STATE_PREPARED,
		STATE_PARSED
	} mState;
	
	Element	mRoot;
	JetHead::vector<GroupData>	mGroups;
	RegexPrepareError mErrorData;
};

#endif // JH_REGEX_H_


