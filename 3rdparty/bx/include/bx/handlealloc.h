/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#ifndef BX_HANDLE_ALLOC_H_HEADER_GUARD
#define BX_HANDLE_ALLOC_H_HEADER_GUARD

#include "bx.h"
#include "allocator.h"

namespace bx
{
	template <uint16_t MaxHandlesT>
	class HandleAllocT
	{
	public:
		static const uint16_t invalid = 0xffff;

		HandleAllocT()
			: m_numHandles(0)
		{
			for (uint16_t ii = 0; ii < MaxHandlesT; ++ii)
			{
				m_handles[ii] = ii;
			}
		}

		~HandleAllocT()
		{
		}

		const uint16_t* getHandles() const
		{
			return m_handles;
		}

		uint16_t getHandleAt(uint16_t _at) const
		{
			return m_handles[_at];
		}

		uint16_t getNumHandles() const
		{
			return m_numHandles;
		}

		uint16_t getMaxHandles() const
		{
			return MaxHandlesT;
		}

		uint16_t alloc()
		{
			if (m_numHandles < MaxHandlesT)
			{
				uint16_t index = m_numHandles;
				++m_numHandles;

				uint16_t handle = m_handles[index];
				uint16_t* sparse = &m_handles[MaxHandlesT];
				sparse[handle] = index;
				return handle;
			}

			return invalid;
		}

		bool isValid(uint16_t _handle)
		{
			uint16_t* sparse = &m_handles[MaxHandlesT];
			uint16_t index = sparse[_handle];

			return index < m_numHandles
				&& m_handles[index] == _handle
				;
		}

		void free(uint16_t _handle)
		{
			BX_CHECK(0 < m_numHandles, "Freeing invalid handle %d.", _handle);
			uint16_t* sparse = &m_handles[MaxHandlesT];
			uint16_t index = sparse[_handle];
			--m_numHandles;
			uint16_t temp = m_handles[m_numHandles];
			m_handles[m_numHandles] = _handle;
			sparse[temp] = index;
			m_handles[index] = temp;
		}

	private:
		uint16_t m_handles[MaxHandlesT*2];
		uint16_t m_numHandles;
	};

	class HandleAlloc
	{
	public:
		static const uint16_t invalid = 0xffff;

		HandleAlloc(uint16_t _maxHandles, void* _handles)
			: m_handles( (uint16_t*)_handles)
			, m_numHandles(0)
			, m_maxHandles(_maxHandles)
		{
			for (uint16_t ii = 0; ii < _maxHandles; ++ii)
			{
				m_handles[ii] = ii;
			}
		}

		~HandleAlloc()
		{
		}

		const uint16_t* getHandles() const
		{
			return m_handles;
		}

		uint16_t getHandleAt(uint16_t _at) const
		{
			return m_handles[_at];
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

				uint16_t handle = m_handles[index];
				uint16_t* sparse = &m_handles[m_maxHandles];
				sparse[handle] = index;
				return handle;
			}

			return invalid;
		}

		bool isValid(uint16_t _handle)
		{
			uint16_t* sparse = &m_handles[m_maxHandles];
			uint16_t index = sparse[_handle];

			return (index < m_numHandles && m_handles[index] == _handle);
		}

		void free(uint16_t _handle)
		{
			uint16_t* sparse = &m_handles[m_maxHandles];
			uint16_t index = sparse[_handle];
			--m_numHandles;
			uint16_t temp = m_handles[m_numHandles];
			m_handles[m_numHandles] = _handle;
			sparse[temp] = index;
			m_handles[index] = temp;
		}

	private:
		uint16_t* m_handles;
		uint16_t m_numHandles;
		uint16_t m_maxHandles;
	};

	inline HandleAlloc* createHandleAlloc(AllocatorI* _allocator, uint16_t _maxHandles)
	{
		uint8_t* ptr = (uint8_t*)BX_ALLOC(_allocator, sizeof(HandleAlloc) + 2*_maxHandles*sizeof(uint16_t) );
		return ::new (ptr) HandleAlloc(_maxHandles, &ptr[sizeof(HandleAlloc)]);
	}

	inline void destroyHandleAlloc(AllocatorI* _allocator, HandleAlloc* _handleAlloc)
	{
		_handleAlloc->~HandleAlloc();
		BX_FREE(_allocator, _handleAlloc);
	}

} // namespace bx

#endif // BX_HANDLE_ALLOC_H_HEADER_GUARD
