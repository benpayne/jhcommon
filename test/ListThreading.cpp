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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "logging.h"
#include "Thread.h"
#include <memory>

class BitField
{
public:
	BitField( int num_bits ) : mNumBits( num_bits )
	{
		mBits = new uint8_t[ (mNumBits + 7) / 8 ];
		clearAll();
	}

	~BitField()
	{
		delete mBits;
	}
	
	void setBit( int bit )
	{
		if ( bit >= mNumBits )
			return;
		mBits[ bit / 8 ] |= 0x1 << bit % 8;
	}
	
	void clearBit( int bit )
	{
		if ( bit >= mNumBits )
			return;
		mBits[ bit / 8 ] &= (0xff ^ (0x1 << (bit % 8)));
	}

	void setAll()
	{
		memset( mBits, 0xff, (mNumBits + 7) / 8 );
	}
	
	void clearAll()
	{
		memset( mBits, 0, (mNumBits + 7) / 8 );
	}
	
	bool getBit( int bit )
	{
		if ( bit >= mNumBits )
			return false;
		
		if ( mBits[ bit / 8 ] & (0x1 << (bit % 8)))
			return true;
		else
			return false;
	}
	
#if 0
	uint32_t	getRange( int start, int end )
	{
		if ( end - start >= 32 )
			end = start + 31;
	}
#endif
		
private:
	int	mNumBits;
	uint8_t *mBits;
};

class FixedAllocator
{
public:
	FixedAllocator( int initial_size = 16, int block_size ) : mLastChunkSize( initial_size ), mBlockSize( block_size )
	{
		mChunks = new AllocChunk( mLastChunkSize, mBlockSize );
	}
	
	~FixedAllocator()
	{
		AllocChunk *chunk = mChunks;
		
		while ( chunk != NULL )
		{
			AllocChunk *node = chunk;
			chunk = chunk->mNext;
			delete node;
		}
	}
	
	void *alloc() 
	{			
		void *obj = NULL;
		AllocChunk *chunk = mChunks;
		
		while ( chunk != NULL )
		{
			obj = chunk->alloc();
			if ( obj != NULL )
				return obj;
			chunk = chunk->mNext;
		}
		
		mLastChunkSize *= 2;
		AllocChunk *new_block = new AllocChunk( mLastChunkSize, mBlockSize );
		obj = new_block->alloc();
		new_block->mNext = mChunks;
		mChunks = new_block;
		
		return obj;
	}
	
	void free( void *item )
	{
		bool found = false;
		AllocChunk *chunk = mChunks;

		while ( chunk != NULL && !found )
		{
			if ( chunk->free( item ) )
				found = true;

			chunk = chunk->mNext;
		}
		
		if ( found == false )
			printf( "failed to find item\n" );
	}
	
private:
	class AllocChunk
	{
	public:
		AllocChunk( int num_block, int block_size ) : mNext( NULL ), mNumBlocks( num_block ), mBlockSize( block_size ), mBitField( num_block )
		{
			mBlocks = new uint8_t[ mNumBlocks * mBlockSize ];
		}
		
		~AllocChunk()
		{
			delete [] mBlocks;
		}
		
		void* alloc()
		{
			for ( int i = 0; i < mNumBlocks; i++ )
			{
				if ( mBitField.getBit( i ) == false )
				{
					mBitField.setBit( i );
					return (void*)&mBlocks[ i * mBlockSize ];
				}
			}
			
			return NULL;
		} 
		
		bool free( void *block )
		{
			if ( block >= mBlocks && block < (uint8_t*)(mBlocks) + mNumBlocks * mBlockSize )
			{
				int offset = ((uint8_t*)block - mBlocks) / mBlockSize;
				mBitField.clearBit( offset );
				return true;
			}
			
			return false;
		}
	
		AllocChunk *mNext;
		
	private:
		int mNumBlocks;
		int mBlockSize;
		BitField mBitField;
		uint8_t	*mBlocks;
	};

	int mLastChunkSize;
	int mBlockSize;
	AllocChunk	*mChunks;
};

namespace JetHead {
	template<typename T>
	class List
	{
	private:
		class Node;

	public:
		List() : mHead( 0 ), mTail( 0 ), mAllocator( 16, sizeof( Node ) ) {}
		~List() { clear(); }

		class iterator
		{
		public:
			iterator() : mNode( NULL ) {}
			iterator( Node *node ) : mNode( node ) {}
			
			iterator &operator++() { mNode = mNode->mNext; return *this; }
			iterator operator++(int) { mNode = mNode->mNext; return *this; }
			iterator &operator--() { mNode = mNode->mPrev; return *this; }
			iterator operator--(int) { mNode = mNode->mPrev; return *this; }

			bool operator==( const iterator &ptr ) const { return ( mNode == ptr.mNode ); }
			bool operator!=( const iterator &ptr ) const { return ( mNode != ptr.mNode ); }	
			
			T* operator->() const { return &(mNode->mData); }		
			operator T*() const { return &(mNode->mData); }
		
		private:
			Node	*mNode;

			friend class List;
		};

		class const_iterator
		{
		public:
			const_iterator() : mNode( NULL ) {}
			const_iterator( Node *node ) : mNode( node ) {}
			
			const_iterator &operator++() { mNode = mNode->mNext; return *this; }
			const_iterator operator++(int) { mNode = mNode->mNext; return *this; }
			const_iterator &operator--() { mNode = mNode->mPrev; return *this; }
			const_iterator operator--(int) { mNode = mNode->mPrev; return *this; }

			bool operator==( const const_iterator &ptr ) const { return ( mNode == ptr.mNode ); }
			bool operator!=( const const_iterator &ptr ) const { return ( mNode != ptr.mNode ); }	
			
			const T* operator->() const { return &(mNode->mData); }
			operator const T*() const { return &(mNode->mData); }
		
		private:
			Node	*mNode;

			friend class List;
		};

		iterator begin() 
		{
			return iterator( mHead );
		}

		iterator end() 
		{
			return iterator( NULL );
		}

		const_iterator begin() const
		{
			return const_iterator( mHead );
		}

		const_iterator end() const 
		{
			return const_iterator( NULL );
		}

		iterator find( T &item ) const
		{
			Node *n = mHead;
			while ( n != NULL ) 
			{
				if ( n->mData == item )
					return iterator( n );
				n = n->mNext;
			} 
		
			return iterator();
		}
		
		void push_back( T &item ) 
		{
			Node *n = new ( mAllocator.alloc() ) Node( item );
			if ( mTail != NULL )
				mTail->mNext = n;
			n->mPrev = mTail;
			mTail = n;
			if ( mHead == NULL )
				mHead = n;
				
		}
		
		void push_front( T &item )
		{
			Node *n = new ( mAllocator.alloc() ) Node( item );
			if ( mHead != NULL )
				mHead->mPrev = n;
			n->mNext = mHead;
			mHead = n;
			if ( mTail == NULL )
				mTail = n;
		}

		void insert_before( iterator i, T &item )
		{
			Node *node = i.mNode;
			if ( node == NULL )
				push_back( item );
			else if ( node->mPrev == NULL )
				push_front( item );
			else
			{
				Node *new_node = new ( mAllocator.alloc() ) Node( item );
				new_node->mNext = node;
				new_node->mPrev = node->mPrev;
				node->mPrev->mNext = new_node;
				node->mPrev = new_node;
			}
		}

		void insert_after( iterator i, T &item )
		{
			Node *node = i.mNode;
			if ( node == NULL )
				push_back( item );
			else if ( node->mNext == NULL )
				push_back( item );
			else
			{
				Node *new_node = new ( mAllocator.alloc() ) Node( item );
				new_node->mPrev = node;
				new_node->mNext = node->mNext;
				node->mNext->mPrev = new_node;
				node->mNext = new_node;
			}
		}

		void clear()
		{
			Node *n = mHead;
			Node *cur;
			
			while( n != NULL )
			{
				cur = n;
				n = n->mNext;
				cur->~Node();
				mAllocator.free( cur );
			}
			
			mHead = mTail = NULL;
		}
		
		int size() const
		{
			Node *n = mHead;
			int count = 0;
			
			while( n != NULL )
			{
				count++;
				n = n->mNext;
			}
			
			return count;
		}
		
		void erase( iterator i )
		{
			Node *n = i.mNode;
			
			if ( n == NULL )
				return;
				
			if ( n->mPrev == NULL )
			{
				mHead = n->mNext;
				if ( mHead != NULL )
					mHead->mPrev = NULL;
			}
			else
			{
				n->mPrev->mNext = n->mNext;
			}
			
			if ( n->mNext == NULL )
			{
				mTail = n->mPrev;
				if ( mTail != NULL )
					mTail->mNext = NULL;
			}
			else
			{
				n->mNext->mPrev = n->mPrev;
			}
			
			n->~Node();
			mAllocator.free( n );
		}

	private:
		class Node
		{
		public:
			Node( T &data ) : mNext( NULL ), mPrev( NULL ), mData( data ) {} 
					
			Node	*mNext;
			Node	*mPrev;	
			T		mData;
		};
		
		Node	*mHead;
		Node	*mTail;
		FixedAllocator	mAllocator;
	};
};

using namespace JetHead;

class Item
{
public:
	Item( int num ) : mNum( num ) 
	{ 
		//printf( "created item %d\n", num ); 
	}
	
	virtual ~Item() 
	{
		//printf( "destroyed item %d\n", mNum );
	}
	
	int getNum() { return mNum; }
	void setNum( int num ) { mNum = num; }
	
	bool operator==( const Item &i ) const { return ( mNum == i.mNum ); }
	
private:
	int mNum;
};

class Runner
{
public:
	Runner( List<Item *> &l ) : mList( l ), mThread( "Runner", this, &Runner::thread_main )
	{
		for( int i = 0; i < kNumItems; i++ )
			mItems[ i ] = NULL;
	
		mThread.Start();
	}
	
	~Runner() { mThread.Join(); }
	
	void thread_main()
	{
		int i = 0;
		int list_size = 0;
		
		while( i < kIterations )
		{
			int job = rand() % kNumItems;
			
			if ( mItems[ job ] == NULL )
			{
				mItems[ job ] = new Item( rand() );
				mList.push_back( mItems[ job ] );
				list_size++;
				if ( mList.size() != list_size )
				{
					printf( "List size doesn't match\n" );
					abort();
				}					
			}
			else 
			{
				List<Item*>::iterator i = mList.find( mItems[ job ] );
				if ( i == mList.end() )
				{
					printf( "Failed to find node %d\n", mItems[ job ]->getNum() );
					abort();
				}
				mList.erase( i );
				delete mItems[ job ];
				mItems[ job ] = NULL;
				list_size--;
				if ( mList.size() != list_size )
				{
					printf( "List size doesn't match\n" );
					abort();
				}					
			}
		
			i++;
		}
	}
	
	static const int kNumItems = 1000;
	static const int kIterations = 10000000;
	
private:
	Item				*mItems[ kNumItems ];
	List<Item*>			&mList;
	Runnable<Runner>	mThread;
};

class Searcher
{
public:
	Searcher( List<Item *> &l ) : mList( l ), mThread( "Searcher", this, &Searcher::thread_main )
	{	
		mThread.Start();
	}
	
	~Searcher() { mThread.Join(); }
	
	void thread_main()
	{
		while( 1 )
		{
			List<Item*>::iterator i = mList.begin();
			List<Item*>::iterator high;
			
			for ( i = mList.begin(); i != mList.end(); ++i )
			{
				if ( (*high)->getNum() < (*i)->getNum() )
					high = i;
			}			

			printf( "High number is %d\n", (*i)->getNum() );
			//mList.erase( high );
		}
	}
		
private:
	List<Item*>		&mList;
	Runnable<Searcher>	mThread;
};

void validate_list( List<Item> &l )
{
	if ( l.size() != 5 )
	{
		printf( "Failed to validate size\n" );
		abort();
	}	
	
	int num;
	List<Item>::iterator i;
	
	for ( num = 1, i = l.begin(); i != l.end(); ++i, num++ )
	{
		if ( i->getNum() != num )
		{
			printf( "Failed to validate List\n" );
			abort();
		}	
	}	
}

int main( int argc, const char *argv[] )
{
	List<Item> l;
	List<Item*> l2;
	
	printf( "Starting Threads...\n" );
	
	Item p1( 1 );
	Item p2( 2 );
	Item p3( 3 );
	Item p4( 4 );
	Item p5( 5 );
	
	printf( "Test 1: " );
	fflush( stdout );
	l.push_front( p3 );
	l.push_front( p2 );
	l.push_back( p4 );
	l.push_back( p5 );
	l.push_front( p1 );	
	validate_list( l );	
	printf( "Done\n" );

	printf( "Test 2: " );
	fflush( stdout );
	l.clear();
	l.insert_before( l.end(), p3 );
	l.insert_after( l.end(), p4 );
	l.insert_before( l.begin(), p1 );
	l.insert_after( l.begin(), p2 );
	l.insert_before( l.end(), p5 );
	validate_list( l );	
	printf( "Done\n" );
	
	printf( "Test 3: " );
	fflush( stdout );
	l.clear();
	l.push_front( p5 );
	l.push_front( p4 );
	l.push_front( p3 );
	l.push_front( p2 );
	l.push_front( p1 );
	validate_list( l );	
	printf( "Done\n" );

	printf( "Test 4: " );
	fflush( stdout );
	List<Item>::iterator i = l.find( p1 );
	l.erase( i );	
	i = l.find( p2 );
	l.erase( i );
	i = l.find( p3 );
	l.erase( i );
	i = l.find( p4 );
	l.erase( i );
	i = l.find( p5 );
	l.erase( i );
	
	if ( l.size() != 0 )
	{
		printf( "Failed to validate size\n" );
		abort();
	}
	printf( "Done\n" );
	
	printf( "Test 5: " );
	fflush( stdout );
	Runner	*r1 = new Runner( l2 );
	delete r1;

	printf( "Done\n" );
	
	//Runner	r2( l );
	//Runner	r3( l );
	//Runner	r4( l );
	//Searcher	s1( l );
	
	return 0;
}
