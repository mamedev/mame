/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_ALLOCATOR_H_HEADER_GUARD
#	error "Must be included from bx/allocator.h"
#endif // BX_ALLOCATOR_H_HEADER_GUARD

inline void* operator new(size_t, bx::PlacementNewTag, void* _ptr)
{
	return _ptr;
}

inline void operator delete(void*, bx::PlacementNewTag, void*) throw()
{
}

namespace bx
{
	inline AllocatorI::~AllocatorI()
	{
	}

	inline void* alignPtr(void* _ptr, size_t _extra, size_t _align)
	{
		union { void* ptr; uintptr_t addr; } un;
		un.ptr = _ptr;
		uintptr_t unaligned = un.addr + _extra; // space for header
		uintptr_t aligned = bx::alignUp(unaligned, int32_t(_align) );
		un.addr = aligned;
		return un.ptr;
	}

	inline void* alloc(AllocatorI* _allocator, size_t _size, size_t _align, const char* _file, uint32_t _line)
	{
		return _allocator->realloc(NULL, _size, _align, _file, _line);
	}

	inline void free(AllocatorI* _allocator, void* _ptr, size_t _align, const char* _file, uint32_t _line)
	{
		_allocator->realloc(_ptr, 0, _align, _file, _line);
	}

	inline void* realloc(AllocatorI* _allocator, void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line)
	{
		return _allocator->realloc(_ptr, _size, _align, _file, _line);
	}

	inline void* alignedAlloc(AllocatorI* _allocator, size_t _size, size_t _align, const char* _file, uint32_t _line)
	{
		const size_t align = max(_align, sizeof(uint32_t) );;
		const size_t total = _size + align;
		uint8_t* ptr = (uint8_t*)alloc(_allocator, total, 0, _file, _line);
		uint8_t* aligned = (uint8_t*)alignPtr(ptr, sizeof(uint32_t), align);
		uint32_t* header = (uint32_t*)aligned - 1;
		*header = uint32_t(aligned - ptr);
		return aligned;
	}

	inline void alignedFree(AllocatorI* _allocator, void* _ptr, size_t _align, const char* _file, uint32_t _line)
	{
		BX_UNUSED(_align);
		uint8_t* aligned = (uint8_t*)_ptr;
		uint32_t* header = (uint32_t*)aligned - 1;
		uint8_t* ptr = aligned - *header;
		free(_allocator, ptr, 0, _file, _line);
	}

	inline void* alignedRealloc(AllocatorI* _allocator, void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line)
	{
		if (NULL == _ptr)
		{
			return alignedAlloc(_allocator, _size, _align, _file, _line);
		}

		uint8_t* aligned = (uint8_t*)_ptr;
		uint32_t offset = *( (uint32_t*)aligned - 1);
		uint8_t* ptr = aligned - offset;

		const size_t align = max(_align, sizeof(uint32_t) );;
		const size_t total = _size + align;
		ptr = (uint8_t*)realloc(_allocator, ptr, total, 0, _file, _line);
		uint8_t* newAligned = (uint8_t*)alignPtr(ptr, sizeof(uint32_t), align);

		if (newAligned == aligned)
		{
			return aligned;
		}

		aligned = ptr + offset;
		memMove(newAligned, aligned, _size);
		uint32_t* header = (uint32_t*)newAligned - 1;
		*header = uint32_t(newAligned - ptr);
		return newAligned;
	}

	template <typename ObjectT>
	inline void deleteObject(AllocatorI* _allocator, ObjectT* _object, size_t _align, const char* _file, uint32_t _line)
	{
		if (NULL != _object)
		{
			_object->~ObjectT();
			free(_allocator, _object, _align, _file, _line);
		}
	}

} // namespace bx
