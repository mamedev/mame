/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_HANDLE_ALLOC_H_HEADER_GUARD
#define BX_HANDLE_ALLOC_H_HEADER_GUARD

#include "bx.h"
#include "allocator.h"
#include "uint32_t.h"

namespace bx
{
	///
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

	///
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

	///
	template <uint16_t MaxHandlesT>
	class HandleListT
	{
	public:
		static const uint16_t invalid = UINT16_MAX;

		HandleListT()
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
			m_front = invalid;
			m_back  = invalid;
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

	///
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

	///
	template <uint32_t MaxCapacityT, typename KeyT = uint32_t>
	class HandleHashMapT
	{
	public:
		static const uint16_t invalid = UINT16_MAX;

		HandleHashMapT()
			: m_maxCapacity(MaxCapacityT)
		{
			reset();
		}

		~HandleHashMapT()
		{
		}

		bool insert(KeyT _key, uint16_t _handle)
		{
			if (invalid == _handle)
			{
				return false;
			}

			const KeyT hash = mix(_key);
			const uint32_t firstIdx = hash % MaxCapacityT;
			uint32_t idx = firstIdx;
			do
			{
				if (m_handle[idx] == invalid)
				{
					m_key[idx]    = _key;
					m_handle[idx] = _handle;
					++m_numElements;
					return true;
				}

				if (m_key[idx] == _key)
				{
					return false;
				}

				idx = (idx + 1) % MaxCapacityT;

			} while (idx != firstIdx);

			return false;
		}

		bool removeByKey(KeyT _key)
		{
			uint32_t idx = findIndex(_key);
			if (UINT32_MAX != idx)
			{
				removeIndex(idx);
				return true;
			}

			return false;
		}

		bool removeByHandle(uint16_t _handle)
		{
			if (invalid != _handle)
			{
				for (uint32_t idx = 0; idx < MaxCapacityT; ++idx)
				{
					if (m_handle[idx] == _handle)
					{
						removeIndex(idx);
					}
				}
			}

			return false;
		}

		uint16_t find(KeyT _key) const
		{
			uint32_t idx = findIndex(_key);
			if (UINT32_MAX != idx)
			{
				return m_handle[idx];
			}

			return invalid;
		}

		void reset()
		{
			memset(m_handle, 0xff, sizeof(m_handle) );
			m_numElements = 0;
		}

		uint32_t getNumElements() const
		{
			return m_numElements;
		}

		uint32_t getMaxCapacity() const
		{
			return m_maxCapacity;
		}

		struct Iterator
		{
			uint16_t handle;

		private:
			friend class HandleHashMapT<MaxCapacityT, KeyT>;
			uint32_t pos;
			uint32_t num;
		};

		Iterator first() const
		{
			Iterator it;
			it.handle = invalid;
			it.pos    = 0;
			it.num    = m_numElements;

			if (0 == it.num)
			{
				return it;
			}

			++it.num;
			next(it);
			return it;
		}

		bool next(Iterator& _it) const
		{
			if (0 == _it.num)
			{
				return false;
			}

			for (
				;_it.pos < MaxCapacityT && invalid == m_handle[_it.pos]
				; ++_it.pos
				);
			_it.handle = m_handle[_it.pos];
			++_it.pos;
			--_it.num;
			return true;
		}

	private:
		uint32_t findIndex(KeyT _key) const
		{
			const KeyT hash = mix(_key);

			const uint32_t firstIdx = hash % MaxCapacityT;
			uint32_t idx = firstIdx;
			do
			{
				if (m_handle[idx] == invalid)
				{
					return UINT32_MAX;
				}

				if (m_key[idx] == _key)
				{
					return idx;
				}

				idx = (idx + 1) % MaxCapacityT;

			} while (idx != firstIdx);

			return UINT32_MAX;
		}

		void removeIndex(uint32_t _idx)
		{
			m_handle[_idx] = invalid;
			--m_numElements;

			for (uint32_t idx = (_idx + 1) % MaxCapacityT
				; m_handle[idx] != invalid
				; idx = (idx + 1) % MaxCapacityT)
			{
				if (m_handle[idx] != invalid)
				{
					const KeyT key = m_key[idx];
					if (idx != findIndex(key) )
					{
						const uint16_t handle = m_handle[idx];
						m_handle[idx] = invalid;
						--m_numElements;
						insert(key, handle);
					}
				}
			}
		}

		uint32_t mix(uint32_t _x) const
		{
			const uint32_t tmp0   = uint32_mul(_x,   UINT32_C(2246822519) );
			const uint32_t tmp1   = uint32_rol(tmp0, 13);
			const uint32_t result = uint32_mul(tmp1, UINT32_C(2654435761) );
			return result;
		}

		uint64_t mix(uint64_t _x) const
		{
			const uint64_t tmp0   = uint64_mul(_x,   UINT64_C(14029467366897019727) );
			const uint64_t tmp1   = uint64_rol(tmp0, 31);
			const uint64_t result = uint64_mul(tmp1, UINT64_C(11400714785074694791) );
			return result;
		}

		uint32_t m_maxCapacity;
		uint32_t m_numElements;

		KeyT     m_key[MaxCapacityT];
		uint16_t m_handle[MaxCapacityT];
	};

	///
	template <uint16_t MaxHandlesT, typename KeyT = uint32_t>
	class HandleHashMapAllocT
	{
	public:
		static const uint16_t invalid = UINT16_MAX;

		HandleHashMapAllocT()
		{
			reset();
		}

		~HandleHashMapAllocT()
		{
		}

		uint16_t alloc(KeyT _key)
		{
			uint16_t handle = m_alloc.alloc();
			if (invalid == handle)
			{
				return invalid;
			}

			bool ok = m_table.insert(_key, handle);
			if (!ok)
			{
				m_alloc.free(handle);
				return invalid;
			}

			return handle;
		}

		void free(KeyT _key)
		{
			uint16_t handle = m_table.find(_key);
			if (invalid == handle)
			{
				return;
			}

			m_table.removeByKey(_key);
			m_alloc.free(handle);
		}

		void free(uint16_t _handle)
		{
			m_table.removeByHandle(_handle);
			m_alloc.free(_handle);
		}

		uint16_t find(KeyT _key) const
		{
			return m_table.find(_key);
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

		bool isValid(uint16_t _handle) const
		{
			return m_alloc.isValid(_handle);
		}

		void reset()
		{
			m_table.reset();
			m_alloc.reset();
		}

	private:
		HandleHashMapT<MaxHandlesT+MaxHandlesT/2, KeyT> m_table;
		HandleAllocT<MaxHandlesT> m_alloc;
	};

} // namespace bx

#endif // BX_HANDLE_ALLOC_H_HEADER_GUARD
