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

#ifndef _JH_LIST_H_
#define _JH_LIST_H_

#include "jh_types.h"
#include <stdio.h>
#include <unistd.h>

class List
{
public:
	List() { mHead.mNext = &mTail; mTail.mPrev = &mHead; }
	virtual ~List() {}

	class Node
	{
	public:
		Node() : mNext( NULL ), mPrev( NULL ) {} 
		virtual ~Node() {}
	
		Node *next() 
		{ 
			if ( mNext->mNext != NULL )
				return mNext;
			else
				return NULL;
		}
		
		Node *prev()
		{ 
			if ( mPrev->mPrev != NULL )
				return mPrev;
			else
				return NULL;
		}
	
		void insertAfter( Node *node )
		{ 
			node->mNext = mNext;
			node->mPrev = this;
			mNext->mPrev = node;
			mNext = node;
		}
		void insertBefore( Node *node )
		{ 
			node->mNext = this;
			node->mPrev = mPrev;
			mPrev->mNext = node;
			mPrev = node;
		}
		
		void remove()
		{
			mNext->mPrev = mPrev;
			mPrev->mNext = mNext;
		}
		
	private:
		Node *mNext;
		Node *mPrev;
		
		friend class List;
	};
		
	void push_back( Node *node ) { mTail.insertBefore( node ); }
	void push_front( Node *node ) { mHead.insertAfter( node ); }
	
	List::Node *getHead()
	{ 
		if ( mHead.mNext != &mTail ) 
			return mHead.mNext; 
		else 
			return NULL;
	}
	
	List::Node *getTail()
	{ 
		if ( mTail.mPrev != &mHead ) 
			return mTail.mPrev; 
		else 
			return NULL;
	}
	
	int	size()
	{
		int size = 0;
		Node *node = getHead();
		
		while( node )
		{
			node = node->next();
			size++;
		}
		
		return size;
	}
	
	bool empty()
	{
		if ( getHead() == NULL )
			return true;
		else
			return false;
	}
	
	void clear()
	{
		Node *node = getHead();

		while( node )
		{
			Node *tmp = node;
			node = node->next();
			delete tmp;
		}

		mHead.mNext = &mTail; mTail.mPrev = &mHead;
	}

	template<class NodeClass>
	static NodeClass *next( NodeClass *cnode )
	{
		List::Node *node = cnode->next();
		return static_cast<NodeClass*>( node );
	}

	template<class NodeClass>
	static NodeClass *prev( NodeClass *cnode )
	{
		List::Node *node = cnode->prev();
		return static_cast<NodeClass*>( node );
	}
	
	template<class NodeClass>
	static NodeClass *find( List &l, NodeClass &match )
	{
		List::Node *node = l.getHead();
		return find( l, match, static_cast<NodeClass*>( node ) );
	}
	
	template<class NodeClass>
	static NodeClass *find( List &l, NodeClass &match, NodeClass *start )
	{
		List::Node *node = start;
	
		while ( node != NULL )
		{
			NodeClass *c = static_cast<NodeClass*>( node );
			
			if ( *c == match )
				return c;
			
			node = node->next();
		}
			
		return NULL;
	}

private:
	Node mHead;
	Node mTail;
};

template<class NodeClass>
class TypedList
{
public:
	void push_back( NodeClass *node ) { mList.push_back( node ); }
	void push_front( NodeClass *node ) { mList.push_front( node ); }
	
	NodeClass *getHead() 
	{
		return static_cast<NodeClass*>( mList.getHead() );
	}

	NodeClass *getTail() 
	{
		return static_cast<NodeClass*>( mList.getTail() );
	}
	
	int	size() { return mList.size(); }	
	bool empty() { return mList.empty(); }
	void clear() { mList.clear(); }
	
	NodeClass *next( NodeClass *cnode )
	{
		return static_cast<NodeClass*>( cnode->next() );
	}

	NodeClass *prev( NodeClass *cnode )
	{
		return static_cast<NodeClass*>( cnode->prev() );
	}

	NodeClass *find( NodeClass &match )
	{
		return find( match, getHead() );
	}
	
	NodeClass *find( NodeClass &match, NodeClass *start )
	{
		List::Node *node = start;
	
		while ( node != NULL )
		{
			NodeClass *c = static_cast<NodeClass*>( node );
			
			if ( *c == match )
				return c;
			
			node = node->next();
		}
			
		return NULL;
	}
	
	bool remove( NodeClass &match )
	{
		NodeClass *node = find(match);
		if (node)
		{
			node->remove();
			delete node;			
			return true;
		}
		return false;
	}

	template<class ftor>
	void foreach( ftor &func )
	{
		NodeClass *node = getHead();
	
		while ( node != NULL )
		{
			NodeClass *c = node;
			node = next( node );
			
			func( c );			
		}
	}
	

private:
	List	mList;
};

#endif // _JH_EVENTQUEUE_H_
