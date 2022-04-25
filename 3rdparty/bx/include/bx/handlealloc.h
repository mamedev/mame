/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_HANDLE_ALLOC_H_HEADER_GUARD
#define BX_HANDLE_ALLOC_H_HEADER_GUARD

#include "bx.h"
#include "allocator.h"
#include "uint32_t.h"

namespace bx
{
	constexpr uint16_t kInvalidHandle = UINT16_MAX;

	///
	class HandleAlloc
	{
	public:
		///
		HandleAlloc(uint16_t _maxHandles);

		///
		~HandleAlloc();

		///
		const uint16_t* getHandles() const;

		///
		uint16_t getHandleAt(uint16_t _at) const;

		///
		uint16_t getNumHandles() const;

		///
		uint16_t getMaxHandles() const;

		///
		uint16_t alloc();

		///
		bool isValid(uint16_t _handle) const;

		///
		void free(uint16_t _handle);

		///
		void reset();

	private:
		HandleAlloc();

		///
		uint16_t* getDensePtr() const;

		///
		uint16_t* getSparsePtr() const;

		uint16_t m_numHandles;
		uint16_t m_maxHandles;
	};

	///
	HandleAlloc* createHandleAlloc(AllocatorI* _allocator, uint16_t _maxHandles);

	///
	void destroyHandleAlloc(AllocatorI* _allocator, HandleAlloc* _handleAlloc);

	///
	template <uint16_t MaxHandlesT>
	class HandleAllocT : public HandleAlloc
	{
	public:
		///
		HandleAllocT();

		///
		~HandleAllocT();

	private:
		uint16_t m_padding[2*MaxHandlesT];
	};

	///
	template <uint16_t MaxHandlesT>
	class HandleListT
	{
	public:
		///
		HandleListT();

		///
		void pushBack(uint16_t _handle);

		///
		uint16_t popBack();

		///
		void pushFront(uint16_t _handle);

		///
		uint16_t popFront();

		///
		uint16_t getFront() const;

		///
		uint16_t getBack() const;

		///
		uint16_t getNext(uint16_t _handle) const;

		///
		uint16_t getPrev(uint16_t _handle) const;

		///
		void remove(uint16_t _handle);

		///
		void reset();

	private:
		///
		void insertBefore(uint16_t _before, uint16_t _handle);

		///
		void insertAfter(uint16_t _after, uint16_t _handle);

		///
		bool isValid(uint16_t _handle) const;

		///
		void updateFrontBack(uint16_t _handle);

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
		///
		HandleAllocLruT();

		///
		~HandleAllocLruT();

		///
		const uint16_t* getHandles() const;

		///
		uint16_t getHandleAt(uint16_t _at) const;

		///
		uint16_t getNumHandles() const;

		///
		uint16_t getMaxHandles() const;

		///
		uint16_t alloc();

		///
		bool isValid(uint16_t _handle) const;

		///
		void free(uint16_t _handle);

		///
		void touch(uint16_t _handle);

		///
		uint16_t getFront() const;

		///
		uint16_t getBack() const;

		///
		uint16_t getNext(uint16_t _handle) const;

		///
		uint16_t getPrev(uint16_t _handle) const;

		///
		void reset();

	private:
		HandleListT<MaxHandlesT>  m_list;
		HandleAllocT<MaxHandlesT> m_alloc;
	};

	///
	template <uint32_t MaxCapacityT, typename KeyT = uint32_t>
	class HandleHashMapT
	{
	public:
		///
		HandleHashMapT();

		///
		~HandleHashMapT();

		///
		bool insert(KeyT _key, uint16_t _handle);

		///
		bool removeByKey(KeyT _key);

		///
		bool removeByHandle(uint16_t _handle);

		///
		uint16_t find(KeyT _key) const;

		///
		void reset();

		///
		uint32_t getNumElements() const;

		///
		uint32_t getMaxCapacity() const;

		///
		struct Iterator
		{
			uint16_t handle;

		private:
			friend class HandleHashMapT<MaxCapacityT, KeyT>;
			uint32_t pos;
			uint32_t num;
		};

		///
		Iterator first() const;

		///
		bool next(Iterator& _it) const;

	private:
		///
		uint32_t findIndex(KeyT _key) const;

		///
		void removeIndex(uint32_t _idx);

		///
		uint32_t mix(uint32_t _x) const;

		///
		uint64_t mix(uint64_t _x) const;

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
		///
		HandleHashMapAllocT();

		///
		~HandleHashMapAllocT();

		///
		uint16_t alloc(KeyT _key);

		///
		void free(KeyT _key);

		///
		void free(uint16_t _handle);

		///
		uint16_t find(KeyT _key) const;

		///
		const uint16_t* getHandles() const;

		///
		uint16_t getHandleAt(uint16_t _at) const;

		///
		uint16_t getNumHandles() const;

		///
		uint16_t getMaxHandles() const;

		///
		bool isValid(uint16_t _handle) const;

		///
		void reset();

	private:
		HandleHashMapT<MaxHandlesT+MaxHandlesT/2, KeyT> m_table;
		HandleAllocT<MaxHandlesT> m_alloc;
	};

} // namespace bx

#include "inline/handlealloc.inl"

#endif // BX_HANDLE_ALLOC_H_HEADER_GUARD
