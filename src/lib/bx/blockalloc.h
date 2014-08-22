/*
 * Copyright 2010-2013 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#ifndef BX_BLOCKALLOC_H_HEADER_GUARD
#define BX_BLOCKALLOC_H_HEADER_GUARD

#include "bx.h"

namespace bx
{
	class BlockAlloc
	{
	public:
		static const uint16_t invalidIndex = 0xffff;
		static const uint32_t minElementSize = 2;

		BlockAlloc()
			: m_data(NULL)
			, m_num(0)
			, m_size(0)
			, m_numFree(0)
			, m_freeIndex(invalidIndex)
		{
		}

		BlockAlloc(void* _data, uint16_t _num, uint16_t _size)
			: m_data(_data)
			, m_num(_num)
			, m_size(_size)
			, m_numFree(_num)
			, m_freeIndex(0)
		{
			char* data = (char*)_data;
			uint16_t* index = (uint16_t*)_data;
			for (uint16_t ii = 0; ii < m_num-1; ++ii)
			{
				*index = ii+1;
				data += m_size;
				index = (uint16_t*)data;
			}
			*index = invalidIndex;
		}

		~BlockAlloc()
		{
		}

		void* alloc()
		{
			if (invalidIndex == m_freeIndex)
			{
				return NULL;
			}

			void* obj = ( (char*)m_data) + m_freeIndex*m_size;
			m_freeIndex = *( (uint16_t*)obj);
			--m_numFree;

			return obj;
		}

		void free(void* _obj)
		{
			uint16_t index = getIndex(_obj);
			BX_CHECK(index < m_num, "index %d, m_num %d", index, m_num);

			*( (uint16_t*)_obj) = m_freeIndex;
			m_freeIndex = index;
			++m_numFree;
		}

		uint16_t getIndex(void* _obj) const
		{
			return (uint16_t)( ( (char*)_obj - (char*)m_data ) / m_size);
		}

		uint16_t getNumFree() const
		{
			return m_numFree;
		}

		void* getFromIndex(uint16_t _index)
		{
			return (char*)m_data + _index*m_size;
		}

	private:
		void* m_data;
		uint16_t m_num;
		uint16_t m_size;
		uint16_t m_numFree;
		uint16_t m_freeIndex;
	};

} // namespace bx

#endif // BX_BLOCKALLOC_H_HEADER_GUARD
