/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_HANDLE_ALLOC_H_HEADER_GUARD
#define BX_HANDLE_ALLOC_H_HEADER_GUARD

#include "bx.h"
#include "allocator.h"

namespace bx
{
	class HandleAlloc
	{
	public:
		static const uint16_t invalid = UINT16_MAX;

		HandleAlloc(uint16_t _maxHandles)
			: m_numHandles(0)
			, m_maxHandles(_maxHandles)
		{
			reset();
		}

		~HandleAlloc()
		{
		}

		const uint16_t* getHandles() const
		{
			return getDensePtr();
		}

		uint16_t getHandleAt(uint16_t _at) const
		{
			return getDensePtr()[_at];
		}

		uint16_t getNumHandles() const
		{
			return m_numHandles;
		}

		uint16_t getMaxHandles() const
		{
			return m_maxHandles;
		}

		uint16_t alloc()
		{
			if (m_numHandles < m_maxHandles)
			{
				uint16_t index = m_numHandles;
				++m_numHandles;

				uint16_t* dense  = getDensePtr();
				uint16_t  handle = dense[index];
				uint16_t* sparse = getSparsePtr();
				sparse[handle] = index;
				return handle;
			}

			return invalid;
		}

		bool isValid(uint16_t _handle) const
		{
			uint16_t* dense  = getDensePtr();
			uint16_t* sparse = getSparsePtr();
			uint16_t  index  = sparse[_handle];

			return index < m_numHandles
				&& dense[index] == _handle
				;
		}

		void free(uint16_t _handle)
		{
			uint16_t* dense  = getDensePtr();
			uint16_t* sparse = getSparsePtr();
			uint16_t index = sparse[_handle];
			--m_numHandles;
			uint16_t temp = dense[m_numHandles];
			dense[m_numHandles] = _handle;
			sparse[temp] = index;
			dense[index] = temp;
		}

		void reset()
		{
			m_numHandles = 0;
			uint16_t* dense = getDensePtr();
			for (uint16_t ii = 0, num = m_maxHandles; ii < num; ++ii)
			{
				dense[ii] = ii;
			}
		}

	private:
		HandleAlloc();

		uint16_t* getDensePtr() const
		{
			uint8_t* ptr = (uint8_t*)reinterpret_cast<const uint8_t*>(this);
			return (uint16_t*)&ptr[sizeof(HandleAlloc)];
		}

		uint16_t* getSparsePtr() const
		{
			return &getDensePtr()[m_maxHandles];
		}

		uint16_t m_numHandles;
		uint16_t m_maxHandles;
	};

	inline HandleAlloc* createHandleAlloc(AllocatorI* _allocator, uint16_t _maxHandles)
	{
		uint8_t* ptr = (uint8_t*)BX_ALLOC(_allocator, sizeof(HandleAlloc) + 2*_maxHandles*sizeof(uint16_t) );
		return ::new (ptr) HandleAlloc(_maxHandles);
	}

	inline void destroyHandleAlloc(AllocatorI* _allocator, HandleAlloc* _handleAlloc)
	{
		_handleAlloc->~HandleAlloc();
		BX_FREE(_allocator, _handleAlloc);
	}

	template <uint16_t MaxHandlesT>
	class HandleAllocT : public HandleAlloc
	{
	public:
		HandleAllocT()
			: HandleAlloc(MaxHandlesT)
		{
		}

		~HandleAllocT()
		{
		}

	private:
		uint16_t m_padding[2*MaxHandlesT];
	};

	template <uint16_t MaxHandlesT>
	class HandleListT
	{
	public:
		static const uint16_t invalid = UINT16_MAX;

		HandleListT()
			: m_front(invalid)
			, m_back(invalid)
		{
			reset();
		}

		void pushBack(uint16_t _handle)
		{
			insertAfter(m_back, _handle);
		}

		uint16_t popBack()
		{
			uint16_t last = invalid != m_back
				? m_back
				: m_front
				;

			if (invalid != last)
			{
				remove(last);
			}

			return last;
		}

		void pushFront(uint16_t _handle)
		{
			insertBefore(m_front, _handle);
		}

		uint16_t popFront()
		{
			uint16_t front = m_front;

			if (invalid != front)
			{
				remove(front);
			}

			return front;
		}

		uint16_t getFront() const
		{
			return m_front;
		}

		uint16_t getBack() const
		{
			return m_back;
		}

		uint16_t getNext(uint16_t _handle) const
		{
			BX_CHECK(isValid(_handle), "Invalid handle %d!", _handle);
			const Link& curr = m_links[_handle];
			return curr.m_next;
		}

		uint16_t getPrev(uint16_t _handle) const
		{
			BX_CHECK(isValid(_handle), "Invalid handle %d!", _handle);
			const Link& curr = m_links[_handle];
			return curr.m_prev;
		}

		void remove(uint16_t _handle)
		{
			BX_CHECK(isValid(_handle), "Invalid handle %d!", _handle);
			Link& curr = m_links[_handle];

			if (invalid != curr.m_prev)
			{
				Link& prev  = m_links[curr.m_prev];
				prev.m_next = curr.m_next;
			}
			else
			{
				m_front = curr.m_next;
			}

			if (invalid != curr.m_next)
			{
				Link& next  = m_links[curr.m_next];
				next.m_prev = curr.m_prev;
			}
			else
			{
				m_back = curr.m_prev;
			}

			curr.m_prev = invalid;
			curr.m_next = invalid;
		}

		void reset()
		{
			memset(m_links, 0xff, sizeof(m_links) );
		}

	private:
		void insertBefore(uint16_t _before, uint16_t _handle)
		{
			Link& curr = m_links[_handle];
			curr.m_next = _before;

			if (invalid != _before)
			{
				Link& link = m_links[_before];
				if (invalid != link.m_prev)
				{
					Link& prev = m_links[link.m_prev];
					prev.m_next = _handle;
				}

				curr.m_prev = link.m_prev;
				link.m_prev = _handle;
			}

			updateFrontBack(_handle);
		}

		void insertAfter(uint16_t _after, uint16_t _handle)
		{
			Link& curr = m_links[_handle];
			curr.m_prev = _after;

			if (invalid != _after)
			{
				Link& link = m_links[_after];
				if (invalid != link.m_next)
				{
					Link& next = m_links[link.m_next];
					next.m_prev = _handle;
				}

				curr.m_next = link.m_next;
				link.m_next = _handle;
			}

			updateFrontBack(_handle);
		}

		bool isValid(uint16_t _handle) const
		{
			return _handle < MaxHandlesT;
		}

		void updateFrontBack(uint16_t _handle)
		{
			Link& curr = m_links[_handle];

			if (invalid == curr.m_prev)
			{
				m_front = _handle;
			}

			if (invalid == curr.m_next)
			{
				m_back = _handle;
			}
		}

		uint16_t m_front;
		uint16_t m_back;

		struct Link
		{
			uint16_t m_prev;
			uint16_t m_next;
		};

		Link m_links[MaxHandlesT];
	};

	template <uint16_t MaxHandlesT>
	class HandleAllocLruT
	{
	public:
		static const uint16_t invalid = UINT16_MAX;

		HandleAllocLruT()
		{
			reset();
		}

		~HandleAllocLruT()
		{
		}

		const uint16_t* getHandles() const
		{
			return m_alloc.getHandles();
		}

		uint16_t getHandleAt(uint16_t _at) const
		{
			return m_alloc.getHandleAt(_at);
		}

		uint16_t getNumHandles() const
		{
			return m_alloc.getNumHandles();
		}

		uint16_t getMaxHandles() const
		{
			return m_alloc.getMaxHandles();
		}

		uint16_t alloc()
		{
			uint16_t handle = m_alloc.alloc();
			if (invalid != handle)
			{
				m_list.pushFront(handle);
			}
			return handle;
		}

		bool isValid(uint16_t _handle) const
		{
			return m_alloc.isValid(_handle);
		}

		void free(uint16_t _handle)
		{
			BX_CHECK(isValid(_handle), "Invalid handle %d!", _handle);
			m_list.remove(_handle);
			m_alloc.free(_handle);
		}

		void touch(uint16_t _handle)
		{
			BX_CHECK(isValid(_handle), "Invalid handle %d!", _handle);
			m_list.remove(_handle);
			m_list.pushFront(_handle);
		}

		uint16_t getFront() const
		{
			return m_list.getFront();
		}

		uint16_t getBack() const
		{
			return m_list.getBack();
		}

		uint16_t getNext(uint16_t _handle) const
		{
			return m_list.getNext(_handle);
		}

		uint16_t getPrev(uint16_t _handle) const
		{
			return m_list.getPrev(_handle);
		}

		void reset()
		{
			m_list.reset();
			m_alloc.reset();
		}

	private:
		HandleListT<MaxHandlesT>  m_list;
		HandleAllocT<MaxHandlesT> m_alloc;
	};

} // namespace bx

#endif // BX_HANDLE_ALLOC_H_HEADER_GUARD
