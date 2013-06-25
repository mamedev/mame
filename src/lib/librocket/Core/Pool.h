/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef ROCKETCOREPOOL_H
#define ROCKETCOREPOOL_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Debug.h>

namespace Rocket {
namespace Core {

template < typename PoolType >
class Pool
{
private:
	class PoolNode
	{
	public:
		PoolType object;
		PoolNode* previous;
		PoolNode* next;
	};

	class PoolChunk
	{
	public:
		PoolNode* chunk;
		PoolChunk* next;
	};

public:
	/**
		Iterator objects are used for safe traversal of the allocated
		members of a pool.
	 */
	class Iterator
	{
		friend class Pool< PoolType >;

	public :
		/// Increments the iterator to reference the next node in the
		/// linked list. It is an error to call this function if the
		/// node this iterator references is invalid.
		inline void operator++()
		{
			ROCKET_ASSERT(node != NULL);
			node = node->next;
		}
		/// Returns true if it is OK to deference or increment this
		/// iterator.
		inline operator bool()
		{
			return (node != NULL);
		}

		/// Returns the object referenced by the iterator's current
		/// node.
		inline PoolType& operator*()
		{
			return node->object;
		}
		/// Returns a pointer to the object referenced by the
		/// iterator's current node.
		inline PoolType* operator->()
		{
			return &node->object;
		}

	private:
		// Constructs an iterator referencing the given node.
		inline Iterator(PoolNode* node)
		{
			this->node = node;
		}

		PoolNode* node;
	};

	Pool(int chunk_size = 0, bool grow = false);
	~Pool();

	/// Initialises the pool to a given size.
	void Initialise(int chunk_size, bool grow = false);

	/// Returns the head of the linked list of allocated objects.
	inline Iterator Begin();

	/// Attempts to allocate a deallocated object in the memory pool. If
	/// the process is successful, the newly allocated object is returned.
	/// If the process fails (due to no free objects being available), NULL
	/// is returned.
	inline PoolType* AllocateObject();

	/// Deallocates the object pointed to by the given iterator.
	inline void DeallocateObject(Iterator& iterator);
	/// Deallocates the given object.
	inline void DeallocateObject(PoolType* object);

	/// Returns the number of objects in the pool.
	inline int GetSize() const;
	/// Returns the number of object chunks in the pool.
	inline int GetNumChunks() const;
	/// Returns the number of allocated objects in the pool.
	inline int GetNumAllocatedObjects() const;

private:
	// Creates a new pool chunk and appends its nodes to the beginning of the free list.
	void CreateChunk();

	int chunk_size;
	bool grow;

	PoolChunk* pool;

	// The heads of the two linked lists.
	PoolNode* first_allocated_node;
	PoolNode* first_free_node;

	int num_allocated_objects;
};

#include "Pool.inl"

}
}

#endif
