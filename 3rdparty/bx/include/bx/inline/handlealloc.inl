/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_HANDLE_ALLOC_H_HEADER_GUARD
#	error "Must be included from bx/handlealloc.h!"
#endif // BX_HANDLE_ALLOC_H_HEADER_GUARD

namespace bx
{
	inline HandleAlloc::HandleAlloc(uint16_t _maxHandles)
		: m_numHandles(0)
		, m_maxHandles(_maxHandles)
	{
		reset();
	}

	inline HandleAlloc::~HandleAlloc()
	{
	}

	inline const uint16_t* HandleAlloc::getHandles() const
	{
		return getDensePtr();
	}

	inline uint16_t HandleAlloc::getHandleAt(uint16_t _at) const
	{
		return getDensePtr()[_at];
	}

	inline uint16_t HandleAlloc::getNumHandles() const
	{
		return m_numHandles;
	}

	inline uint16_t HandleAlloc::getMaxHandles() const
	{
		return m_maxHandles;
	}

	inline uint16_t HandleAlloc::alloc()
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

		return kInvalidHandle;
	}

	inline bool HandleAlloc::isValid(uint16_t _handle) const
	{
		uint16_t* dense  = getDensePtr();
		uint16_t* sparse = getSparsePtr();
		uint16_t  index  = sparse[_handle];

		return index < m_numHandles
			&& dense[index] == _handle
			;
	}

	inline void HandleAlloc::free(uint16_t _handle)
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

	inline void HandleAlloc::reset()
	{
		m_numHandles = 0;
		uint16_t* dense = getDensePtr();
		for (uint16_t ii = 0, num = m_maxHandles; ii < num; ++ii)
		{
			dense[ii] = ii;
		}
	}

	inline uint16_t* HandleAlloc::getDensePtr() const
	{
		uint8_t* ptr = (uint8_t*)reinterpret_cast<const uint8_t*>(this);
		return (uint16_t*)&ptr[sizeof(HandleAlloc)];
	}

	inline uint16_t* HandleAlloc::getSparsePtr() const
	{
		return &getDensePtr()[m_maxHandles];
	}

	inline HandleAlloc* createHandleAlloc(AllocatorI* _allocator, uint16_t _maxHandles)
	{
		uint8_t* ptr = (uint8_t*)BX_ALLOC(_allocator, sizeof(HandleAlloc) + 2*_maxHandles*sizeof(uint16_t) );
		return BX_PLACEMENT_NEW(ptr, HandleAlloc)(_maxHandles);
	}

	inline void destroyHandleAlloc(AllocatorI* _allocator, HandleAlloc* _handleAlloc)
	{
		_handleAlloc->~HandleAlloc();
		BX_FREE(_allocator, _handleAlloc);
	}

	template <uint16_t MaxHandlesT>
	inline HandleAllocT<MaxHandlesT>::HandleAllocT()
		: HandleAlloc(MaxHandlesT)
	{
	}

	template <uint16_t MaxHandlesT>
	inline HandleAllocT<MaxHandlesT>::~HandleAllocT()
	{
	}

	template <uint16_t MaxHandlesT>
	inline HandleListT<MaxHandlesT>::HandleListT()
	{
		reset();
	}

	template <uint16_t MaxHandlesT>
	inline void HandleListT<MaxHandlesT>::pushBack(uint16_t _handle)
	{
		insertAfter(m_back, _handle);
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleListT<MaxHandlesT>::popBack()
	{
		uint16_t last = kInvalidHandle != m_back
			? m_back
			: m_front
			;

		if (kInvalidHandle != last)
		{
			remove(last);
		}

		return last;
	}

	template <uint16_t MaxHandlesT>
	inline void HandleListT<MaxHandlesT>::pushFront(uint16_t _handle)
	{
		insertBefore(m_front, _handle);
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleListT<MaxHandlesT>::popFront()
	{
		uint16_t front = m_front;

		if (kInvalidHandle != front)
		{
			remove(front);
		}

		return front;
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleListT<MaxHandlesT>::getFront() const
	{
		return m_front;
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleListT<MaxHandlesT>::getBack() const
	{
		return m_back;
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleListT<MaxHandlesT>::getNext(uint16_t _handle) const
	{
		BX_CHECK(isValid(_handle), "Invalid handle %d!", _handle);
		const Link& curr = m_links[_handle];
		return curr.m_next;
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleListT<MaxHandlesT>::getPrev(uint16_t _handle) const
	{
		BX_CHECK(isValid(_handle), "Invalid handle %d!", _handle);
		const Link& curr = m_links[_handle];
		return curr.m_prev;
	}

	template <uint16_t MaxHandlesT>
	inline void HandleListT<MaxHandlesT>::remove(uint16_t _handle)
	{
		BX_CHECK(isValid(_handle), "Invalid handle %d!", _handle);
		Link& curr = m_links[_handle];

		if (kInvalidHandle != curr.m_prev)
		{
			Link& prev  = m_links[curr.m_prev];
			prev.m_next = curr.m_next;
		}
		else
		{
			m_front = curr.m_next;
		}

		if (kInvalidHandle != curr.m_next)
		{
			Link& next  = m_links[curr.m_next];
			next.m_prev = curr.m_prev;
		}
		else
		{
			m_back = curr.m_prev;
		}

		curr.m_prev = kInvalidHandle;
		curr.m_next = kInvalidHandle;
	}

	template <uint16_t MaxHandlesT>
	inline void HandleListT<MaxHandlesT>::reset()
	{
		memSet(m_links, 0xff, sizeof(m_links) );
		m_front = kInvalidHandle;
		m_back  = kInvalidHandle;
	}

	template <uint16_t MaxHandlesT>
	inline void HandleListT<MaxHandlesT>::insertBefore(uint16_t _before, uint16_t _handle)
	{
		Link& curr = m_links[_handle];
		curr.m_next = _before;

		if (kInvalidHandle != _before)
		{
			Link& link = m_links[_before];
			if (kInvalidHandle != link.m_prev)
			{
				Link& prev = m_links[link.m_prev];
				prev.m_next = _handle;
			}

			curr.m_prev = link.m_prev;
			link.m_prev = _handle;
		}

		updateFrontBack(_handle);
	}

	template <uint16_t MaxHandlesT>
	inline void HandleListT<MaxHandlesT>::insertAfter(uint16_t _after, uint16_t _handle)
	{
		Link& curr = m_links[_handle];
		curr.m_prev = _after;

		if (kInvalidHandle != _after)
		{
			Link& link = m_links[_after];
			if (kInvalidHandle != link.m_next)
			{
				Link& next = m_links[link.m_next];
				next.m_prev = _handle;
			}

			curr.m_next = link.m_next;
			link.m_next = _handle;
		}

		updateFrontBack(_handle);
	}

	template <uint16_t MaxHandlesT>
	inline bool HandleListT<MaxHandlesT>::isValid(uint16_t _handle) const
	{
		return _handle < MaxHandlesT;
	}

	template <uint16_t MaxHandlesT>
	inline void HandleListT<MaxHandlesT>::updateFrontBack(uint16_t _handle)
	{
		Link& curr = m_links[_handle];

		if (kInvalidHandle == curr.m_prev)
		{
			m_front = _handle;
		}

		if (kInvalidHandle == curr.m_next)
		{
			m_back = _handle;
		}
	}

	template <uint16_t MaxHandlesT>
	inline HandleAllocLruT<MaxHandlesT>::HandleAllocLruT()
	{
		reset();
	}

	template <uint16_t MaxHandlesT>
	inline HandleAllocLruT<MaxHandlesT>::~HandleAllocLruT()
	{
	}

	template <uint16_t MaxHandlesT>
	inline const uint16_t* HandleAllocLruT<MaxHandlesT>::getHandles() const
	{
		return m_alloc.getHandles();
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleAllocLruT<MaxHandlesT>::getHandleAt(uint16_t _at) const
	{
		return m_alloc.getHandleAt(_at);
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleAllocLruT<MaxHandlesT>::getNumHandles() const
	{
		return m_alloc.getNumHandles();
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleAllocLruT<MaxHandlesT>::getMaxHandles() const
	{
		return m_alloc.getMaxHandles();
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleAllocLruT<MaxHandlesT>::alloc()
	{
		uint16_t handle = m_alloc.alloc();
		if (kInvalidHandle != handle)
		{
			m_list.pushFront(handle);
		}
		return handle;
	}

	template <uint16_t MaxHandlesT>
	inline bool HandleAllocLruT<MaxHandlesT>::isValid(uint16_t _handle) const
	{
		return m_alloc.isValid(_handle);
	}

	template <uint16_t MaxHandlesT>
	inline void HandleAllocLruT<MaxHandlesT>::free(uint16_t _handle)
	{
		BX_CHECK(isValid(_handle), "Invalid handle %d!", _handle);
		m_list.remove(_handle);
		m_alloc.free(_handle);
	}

	template <uint16_t MaxHandlesT>
	inline void HandleAllocLruT<MaxHandlesT>::touch(uint16_t _handle)
	{
		BX_CHECK(isValid(_handle), "Invalid handle %d!", _handle);
		m_list.remove(_handle);
		m_list.pushFront(_handle);
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleAllocLruT<MaxHandlesT>::getFront() const
	{
		return m_list.getFront();
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleAllocLruT<MaxHandlesT>::getBack() const
	{
		return m_list.getBack();
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleAllocLruT<MaxHandlesT>::getNext(uint16_t _handle) const
	{
		return m_list.getNext(_handle);
	}

	template <uint16_t MaxHandlesT>
	inline uint16_t HandleAllocLruT<MaxHandlesT>::getPrev(uint16_t _handle) const
	{
		return m_list.getPrev(_handle);
	}

	template <uint16_t MaxHandlesT>
	inline void HandleAllocLruT<MaxHandlesT>::reset()
	{
		m_list.reset();
		m_alloc.reset();
	}

	template <uint32_t MaxCapacityT, typename KeyT>
	inline HandleHashMapT<MaxCapacityT, KeyT>::HandleHashMapT()
		: m_maxCapacity(MaxCapacityT)
	{
		reset();
	}

	template <uint32_t MaxCapacityT, typename KeyT>
	inline HandleHashMapT<MaxCapacityT, KeyT>::~HandleHashMapT()
	{
	}

	template <uint32_t MaxCapacityT, typename KeyT>
	inline bool HandleHashMapT<MaxCapacityT, KeyT>::insert(KeyT _key, uint16_t _handle)
	{
		if (kInvalidHandle == _handle)
		{
			return false;
		}

		const KeyT hash = mix(_key);
		const uint32_t firstIdx = hash % MaxCapacityT;
		uint32_t idx = firstIdx;
		do
		{
			if (m_handle[idx] == kInvalidHandle)
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

	template <uint32_t MaxCapacityT, typename KeyT>
	inline bool HandleHashMapT<MaxCapacityT, KeyT>::removeByKey(KeyT _key)
	{
		uint32_t idx = findIndex(_key);
		if (UINT32_MAX != idx)
		{
			removeIndex(idx);
			return true;
		}

		return false;
	}

	template <uint32_t MaxCapacityT, typename KeyT>
	inline bool HandleHashMapT<MaxCapacityT, KeyT>::removeByHandle(uint16_t _handle)
	{
		if (kInvalidHandle != _handle)
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

	template <uint32_t MaxCapacityT, typename KeyT>
	inline uint16_t HandleHashMapT<MaxCapacityT, KeyT>::find(KeyT _key) const
	{
		uint32_t idx = findIndex(_key);
		if (UINT32_MAX != idx)
		{
			return m_handle[idx];
		}

		return kInvalidHandle;
	}

	template <uint32_t MaxCapacityT, typename KeyT>
	inline void HandleHashMapT<MaxCapacityT, KeyT>::reset()
	{
		memSet(m_handle, 0xff, sizeof(m_handle) );
		m_numElements = 0;
	}

	template <uint32_t MaxCapacityT, typename KeyT>
	inline uint32_t HandleHashMapT<MaxCapacityT, KeyT>::getNumElements() const
	{
		return m_numElements;
	}

	template <uint32_t MaxCapacityT, typename KeyT>
	inline uint32_t HandleHashMapT<MaxCapacityT, KeyT>::getMaxCapacity() const
	{
		return m_maxCapacity;
	}

	template <uint32_t MaxCapacityT, typename KeyT>
	inline typename HandleHashMapT<MaxCapacityT, KeyT>::Iterator HandleHashMapT<MaxCapacityT, KeyT>::first() const
	{
		Iterator it;
		it.handle = kInvalidHandle;
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

	template <uint32_t MaxCapacityT, typename KeyT>
	inline bool HandleHashMapT<MaxCapacityT, KeyT>::next(Iterator& _it) const
	{
		if (0 == _it.num)
		{
			return false;
		}

		for (
			;_it.pos < MaxCapacityT && kInvalidHandle == m_handle[_it.pos]
			; ++_it.pos
			);
		_it.handle = m_handle[_it.pos];
		++_it.pos;
		--_it.num;
		return true;
	}

	template <uint32_t MaxCapacityT, typename KeyT>
	inline uint32_t HandleHashMapT<MaxCapacityT, KeyT>::findIndex(KeyT _key) const
	{
		const KeyT hash = mix(_key);

		const uint32_t firstIdx = hash % MaxCapacityT;
		uint32_t idx = firstIdx;
		do
		{
			if (m_handle[idx] == kInvalidHandle)
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

	template <uint32_t MaxCapacityT, typename KeyT>
	inline void HandleHashMapT<MaxCapacityT, KeyT>::removeIndex(uint32_t _idx)
	{
		m_handle[_idx] = kInvalidHandle;
		--m_numElements;

		for (uint32_t idx = (_idx + 1) % MaxCapacityT
				; m_handle[idx] != kInvalidHandle
				; idx = (idx + 1) % MaxCapacityT)
		{
			if (m_handle[idx] != kInvalidHandle)
			{
				const KeyT key = m_key[idx];
				if (idx != findIndex(key) )
				{
					const uint16_t handle = m_handle[idx];
					m_handle[idx] = kInvalidHandle;
					--m_numElements;
					insert(key, handle);
				}
			}
		}
	}

	template <uint32_t MaxCapacityT, typename KeyT>
	inline uint32_t HandleHashMapT<MaxCapacityT, KeyT>::mix(uint32_t _x) const
	{
		const uint32_t tmp0   = uint32_mul(_x,   UINT32_C(2246822519) );
		const uint32_t tmp1   = uint32_rol(tmp0, 13);
		const uint32_t result = uint32_mul(tmp1, UINT32_C(2654435761) );
		return result;
	}

	template <uint32_t MaxCapacityT, typename KeyT>
	inline uint64_t HandleHashMapT<MaxCapacityT, KeyT>::mix(uint64_t _x) const
	{
		const uint64_t tmp0   = uint64_mul(_x,   UINT64_C(14029467366897019727) );
		const uint64_t tmp1   = uint64_rol(tmp0, 31);
		const uint64_t result = uint64_mul(tmp1, UINT64_C(11400714785074694791) );
		return result;
	}

	template <uint16_t MaxHandlesT, typename KeyT>
	inline HandleHashMapAllocT<MaxHandlesT, KeyT>::HandleHashMapAllocT()
	{
		reset();
	}

	template <uint16_t MaxHandlesT, typename KeyT>
	inline HandleHashMapAllocT<MaxHandlesT, KeyT>::~HandleHashMapAllocT()
	{
	}

	template <uint16_t MaxHandlesT, typename KeyT>
	inline uint16_t HandleHashMapAllocT<MaxHandlesT, KeyT>::alloc(KeyT _key)
	{
		uint16_t handle = m_alloc.alloc();
		if (kInvalidHandle == handle)
		{
			return kInvalidHandle;
		}

		bool ok = m_table.insert(_key, handle);
		if (!ok)
		{
			m_alloc.free(handle);
			return kInvalidHandle;
		}

		return handle;
	}

	template <uint16_t MaxHandlesT, typename KeyT>
	inline void HandleHashMapAllocT<MaxHandlesT, KeyT>::free(KeyT _key)
	{
		uint16_t handle = m_table.find(_key);
		if (kInvalidHandle == handle)
		{
			return;
		}

		m_table.removeByKey(_key);
		m_alloc.free(handle);
	}

	template <uint16_t MaxHandlesT, typename KeyT>
	inline void HandleHashMapAllocT<MaxHandlesT, KeyT>::free(uint16_t _handle)
	{
		m_table.removeByHandle(_handle);
		m_alloc.free(_handle);
	}

	template <uint16_t MaxHandlesT, typename KeyT>
	inline uint16_t HandleHashMapAllocT<MaxHandlesT, KeyT>::find(KeyT _key) const
	{
		return m_table.find(_key);
	}

	template <uint16_t MaxHandlesT, typename KeyT>
	inline const uint16_t* HandleHashMapAllocT<MaxHandlesT, KeyT>::getHandles() const
	{
		return m_alloc.getHandles();
	}

	template <uint16_t MaxHandlesT, typename KeyT>
	inline uint16_t HandleHashMapAllocT<MaxHandlesT, KeyT>::getHandleAt(uint16_t _at) const
	{
		return m_alloc.getHandleAt(_at);
	}

	template <uint16_t MaxHandlesT, typename KeyT>
	inline uint16_t HandleHashMapAllocT<MaxHandlesT, KeyT>::getNumHandles() const
	{
		return m_alloc.getNumHandles();
	}

	template <uint16_t MaxHandlesT, typename KeyT>
	inline uint16_t HandleHashMapAllocT<MaxHandlesT, KeyT>::getMaxHandles() const
	{
		return m_alloc.getMaxHandles();
	}

	template <uint16_t MaxHandlesT, typename KeyT>
	inline bool HandleHashMapAllocT<MaxHandlesT, KeyT>::isValid(uint16_t _handle) const
	{
		return m_alloc.isValid(_handle);
	}

	template <uint16_t MaxHandlesT, typename KeyT>
	inline void HandleHashMapAllocT<MaxHandlesT, KeyT>::reset()
	{
		m_table.reset();
		m_alloc.reset();
	}

} // namespace bx
