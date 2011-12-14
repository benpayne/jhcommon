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

#ifndef _NEW_JH_LIST_H_
#define _NEW_JH_LIST_H_

/**
 * \file
 *
 * A templated list type.  Supports front/back insert/remove,
 * and other operations via the iterator member class.  DO NOT ADD
 * METHODS TO THIS CLASS IF AT ALL POSSIBLE.  This class exists only
 * because the bloat for STL lists is too large to be suitable in an
 * embedded system environment.  Adding extra stuff in here will be
 * replicated dozens of times throughout the codebase.  
 *
 * Use this if the only other option is STL.  Otherwise, this is
 * probably still heavyweight.
 */

#include "jh_memory.h"

namespace JetHead
{

	template <typename T>
	class list
	{
	public:
		list()
			: mHead(jh_new Node),
			  mTail(jh_new Node)
		{ 
			mHead->next = mTail;
			mHead->prev = NULL;
			mTail->prev = mHead;
			mTail->next = NULL;
		}

		//! Copy constructor, copies by value using list<T>::iterator
		list(const list<T>& rhs)
		{
			mHead = jh_new Node;
			mTail = jh_new Node;

			mHead->next = mTail;
			mHead->prev = NULL;
			mTail->prev = mHead;
			mTail->next = NULL;

			for (iterator i = rhs.begin(); i != rhs.end(); ++i)
			{
				push_back(*i);
			}
		}

		//! Delete all of the allocated Node's in our chain
		~list()
		{
			while (mHead != NULL)
			{
				Node* temp = mHead->next;
				delete mHead;
				mHead = temp;
			}
			mTail = NULL;
		}

		//! Clear everything from the list
		void clear()
		{
			while (not empty())
			{
				begin().erase();
			}
		}

		//! const peek for the front value
		const T& front() const
		{
			return mHead->next->val;
		}

		//! const peek for the end value
		const T& back() const
		{
			return mTail->prev->val;
		}		

		//! non-const peek for the front value
		T& front() 
		{
			return mHead->next->val;
		}

		//! non-const peek for the end value
		T& back() 
		{
			return mTail->prev->val;
		}		

		//! Pull the first element off the front
		void pop_front()
		{
			if (mHead->next == mTail) return;

			Node* temp = mHead->next;
			mHead->next = temp->next;

			delete temp;

			mHead->next->prev = mHead;
		}

		//! Pull the last element off the back of the list
		void pop_back()
		{
			if (mTail->prev == mHead) return;

			Node* temp = mTail->prev;
			mTail->prev = temp->prev;

			delete temp;
			
			mTail->prev->next = mTail;
		}

		//! Add to the front of the list
		void push_front(const T& val)
		{
			Node* newHead = jh_new Node;
			newHead->val = val;
			newHead->next = mHead->next;
			newHead->prev = mHead;

			mHead->next->prev = newHead;
			mHead->next = newHead;
		}

		//! Add to the back of the list
		void push_back(const T& val)
		{
			Node* newTail = jh_new Node;
			newTail->val = val;
			newTail->prev = mTail->prev;
			newTail->next = mTail;

			mTail->prev->next = newTail;
			mTail->prev = newTail;
		}

		//! How much is in the list.  NOTE: This is O(n)
		unsigned size() const
		{
			unsigned ret = 0;
			for (Node* temp = mHead; temp != NULL; temp = temp->next)
			{
				ret++;
			}
			return ret - 2;
		}

		//! Is there anything in the list?  This is O(1)
		bool empty() const
		{
			return (mHead->next == mTail);
		}

	protected:
		// The internal storage type for elements in this list
		struct Node
		{
			T val;
			Node* next;
			Node* prev;
		};		

	public:
		//! A relatively basic iterator, maintains general STL syntax
		class iterator
		{
		public:
			~iterator()
			{
				//mNode = NULL;
			}

			//! Const dereference
			const T& operator * () const
			{
				return mNode->val;
			}

			//! Non-const dereference
			T& operator * ()
			{
				return mNode->val;
			}

			//! Const member-by-pointer
			const T* operator -> () const
			{
				return &(mNode->val);
			}

			//! Non-const member-by-pointer
			T* operator -> ()
			{
				return &(mNode->val);
			}

			//! Move forward along the list
			iterator& operator ++ ()
			{
				if (mNode)
				{
					mNode = mNode->next;
				}
				return *this;
			}

			//! Move backward along the list
			iterator& operator -- ()
			{
				if (mNode)
				{
					mNode = mNode->prev;
				}
				return *this;
			}

			//! Is this pointing at the same as that?
			bool operator == (const iterator& rhs) const
			{
				return mNode == rhs.mNode;
			}
			
			//! Is this _not_ pointing at the same as that?
			bool operator != (const iterator& rhs) const
			{
				return mNode != rhs.mNode;
			}
			
			//! Add a new node after mNode
			void insertAfter(const T& val)
			{
				Node* newNode = new Node;
				newNode->val = val;
				newNode->prev = mNode;
				newNode->next = mNode->next;

				mNode->next->prev = newNode;
				mNode->next = newNode;
			}

			//! Add a new node before mNode
			void insertBefore(const T& val)
			{
				iterator i;
				i.mNode = mNode->prev;
				i.insertAfter(val);
			}

			/**
			 * @brief Erase this node
			 *
			 * @note Invalidates this iterator, returns an iterator
			 * pointing to the next element of the list (STL semantics)
			 */
			iterator erase()
			{
				// Don't segfault, and don't delete head or tail
				// placeholder nodes 
				if (mNode == NULL or 
					mNode->next == NULL or 
					mNode->prev == NULL) 
				{
					return *this;
				}

				Node* prev = mNode->prev;
				Node* next = mNode->next;

				prev->next = next;
				next->prev = prev;

				delete mNode;
				mNode = NULL;

				iterator ret;
				ret.mNode = next;
				return ret;
			}

		protected:
			mutable Node* mNode;
			
			friend class list;
		};

		//! An iterator pointing at the front of the list
		iterator begin()
		{
			iterator ret;
			ret.mNode = mHead->next;
			return ret;
		}

		//! An iterator pointing at the tail (off the end of the list)
		iterator end()
		{
			iterator ret;
			ret.mNode = mTail;
			return ret;
		}

		//! An iterator pointing at head (off the front of the list)
		iterator rbegin()
		{
			iterator ret;
			ret.mNode = mHead;
			return ret;
		}

		//! An iterator pointing at the end of the list
		iterator rend()
		{
			iterator ret;
			ret.mNode = mTail->prev;
			return ret;
		}

		//! A const iterator pointing at the beginning of the list
		const iterator begin() const
		{
			iterator ret;
			ret.mNode = mHead->next;
			return ret;
		}

		//! A const iterator pointing off the end of the list (NULL)
		const iterator end() const
		{
			iterator ret;
			ret.mNode = mTail;
			return ret;
		}

		//! A const iterator pointing off the beginning of the list (NULL)
		const iterator rbegin() const
		{
			iterator ret;
			ret.mNode = mHead;
			return ret;
		}

		//! A const iterator pointing at the end of the list
		const iterator rend() const
		{
			iterator ret;
			ret.mNode = mTail->prev;
			return ret;
		}

	protected:
		//! Front of the list
		Node* mHead;

		//! Back of the list
		Node* mTail;
	};	

};

#endif // _NEW_JH_LIST_H_
