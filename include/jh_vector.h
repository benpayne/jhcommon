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

#ifndef JH_VECTOR_H
#define JH_VECTOR_H

/**
 * \file
 *
 * A templated vector type (self-expanding, memory managed array.)  DO
 * NOT ADD METHODS TO THIS CLASS IF POSSIBLE.  This class exists only
 * because the bloat for STL vectors is too large to be suitable in an
 * embedded system environment.  Adding extra stuff in here will be
 * replicated dozens of times throughout the codebase.
 */

#ifndef USE_JETHEAD_STL
#define JHSTD	std
#include <vector>
#else

#include <stdlib.h>
#include <stdint.h>
#include <memory>

namespace JetHead
{
	template <typename T>
	class vector
	{
	public:
		static const unsigned DEFAULT_ALLOCATION = 32;

		//! Default constructor, nothing contained in the array, 
		vector()
			: mAllocated(DEFAULT_ALLOCATION),

			// Data is allocated as a buffer of bytes, typing is done
			// internally since we don't want anything to trigger a
			// constructor/destructor when we are adjusting our
			// internal buffer
			mData((T*)(new uint8_t[sizeof(T) * mAllocated])),
			mSize(0)
		{

		}

		//! Default constructor, nothing contained in the array, 
		explicit vector(unsigned allocated)
			: mAllocated(allocated),

			// Data is allocated as a buffer of bytes, typing is done
			// internally since we don't want anything to trigger a
			// constructor/destructor when we are adjusting our
			// internal buffer
			mData((T*)(new uint8_t[sizeof(T) * mAllocated])),
			mSize(0)
		{

		}
		
		//! Copy constructor
		vector(const vector<T>& rhs)
			: mAllocated(0),
			mData(NULL),
			mSize(0)
		{
			*this = rhs;
		}

		//! Clean up allocated data
		~vector()
		{
			if (mData)
			{
				// Call all of our destructors for contained objects
				clear();
				
				// Free memory
				delete[] (uint8_t*)mData;
				mData = NULL;
			}
		}
		
		//! Non-const lookup operator
		T& operator [] (unsigned ind)
		{
			// If the index is out of range, return a T& of NULL
			if (ind >= mSize)
				return *(T*)NULL;
			
			return mData[ind];
		}

		//! Const lookup operator
		const T& operator [] (unsigned ind) const
		{
			// If the index is out of range, return a T& of NULL
			if (ind >= mSize)
				return *(T*)NULL;

			return mData[ind];
		}
		
		//! Assignment operator (free up mine, allocate, copy yours)
		vector<T>& operator = (const vector<T>& rhs)
		{
			// Destruct all of my objects
			clear();

			// If my buffer is smaller than his, allocate more space
			if (mAllocated < rhs.mSize)
			{
				sizeup(rhs.mSize);
			}

			// Copy over all of the objects from the other container.
			// This is a placement new on a copy constructor.
			for (unsigned i = 0; i < rhs.mSize; i++)
			{
				new (mData + i) T(rhs.mData[i]);
			}
			
			// Remember how many objects we have stored
			mSize = rhs.mSize;
			return *this;
		}

		//! Add to the end
		void push_back(const T& val)
		{
			// If we've hit our limit, grow the array
			if (mSize == mAllocated)
			{
				sizeup();
			}
 
			// Now that we've got space, placement-new a copy of val
			new(mData + mSize) T(val);

			// And remember that we've stored another object
			mSize++;
		}
		
		//! Erase the item at this index
		void erase(unsigned ind)
		{
			// If the index to erase if out of range, abort.
			if (ind >= mSize)
				abort();
			
			// Copy i+1 onto i for all i >= ind.  This has the effect 
			// of moving everything farther back in the array down one,
			// and leaving two copies of the last element (at mSize
			// and mSize - 1).
			for (unsigned i = ind; i < mSize - 1; i++)
			{
				mData[i] = mData[i + 1];
			}

			// Now destruct the extra copy of the last element
			mData[mSize - 1].~T();

			// And remember that we've stored one fewer object
			mSize--;			
		}
		
		//! Make sure we have at least this much size allocated
		void reserve(unsigned size)
		{
			if (mAllocated < size)
			{
				sizeup(size);
			}
		}

		//! Change the size of the container's in-use section
		void resize(unsigned newSize)
		{
			// If we need more allocation, do it now
			if (mAllocated < newSize)
			{
				if (newSize < mAllocated * 2)
				{
					sizeup();
				} else {
					sizeup(newSize);
				}
			}

			// If we are shrinking (newSize < mSize), destroy
			// everything that is now off the end of the vector
			for (unsigned i = newSize; i < mSize; i++)
			{
				mData[i].~T();
			}

			// If we are growing (mSize < newSize), then call the
			// default constructor on all the objects we just 
			// implicitly created
			for (unsigned i = mSize; i < newSize; i++)
			{
				new(mData + i) T();
			}
			
			// Remember our new size
			mSize = newSize;
		}

		//! How much is in use?
		unsigned size() const { return mSize; }

		//! Is this empty?
		bool empty() const { return mSize == 0; }
		
		//! Make this empty
		void clear() 
		{ 
			// Call destructor-in-place, not delete (we didn't
			// allocate this object with new).  Destroy everything.
			for (unsigned i = 0; i < mSize; i++)
			{
				mData[i].~T();
			}

			// Nothing is left
			mSize = 0; 
		}
		
	private:
		//! Move up to a bigger size, or double if no size specified
		void sizeup(unsigned newSize=0)
		{
			// No size was specified, double the existing size.
			// Doubling a self-expanding array means that storing O(n) 
			// elements takes O(n lg n) worst case.  Adding only a 
			// constant number of elements makes that O(n^2).
			if (newSize == 0) 
			{
				// If we allocated ANYTHING yet, then double it, if 
				// not just use the default size.
				if (mAllocated)
				{
					newSize = mAllocated * 2;
				} else {
					newSize = DEFAULT_ALLOCATION;
				}
			}

			// A new buffer
			T* temp = (T*)(new uint8_t[sizeof(T) * newSize]);

			// If we had data, copy it over
			if (mData)
			{
				// Move all of my data into the new buffer (copy then delete)
				for (unsigned i = 0; i < mSize; i++)
				{
					new(temp + i) T(mData[i]);
					mData[i].~T();
				}
				
				// Delete old buffer
				delete[] (uint8_t*)mData;
			}
			
			// Keep new buffer
			mData = temp;

			// Remember new allocation size
			mAllocated = newSize;
		}

		//! How much space have we allocated
		unsigned mAllocated;

		//! The data we are using
		T* mData;

		//! How much space we are using 
		unsigned mSize;

	};
};
#endif // USE_JETHEAD_STL

#endif // JH_VECTOR_H
