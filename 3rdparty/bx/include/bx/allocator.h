/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_ALLOCATOR_H_HEADER_GUARD
#define BX_ALLOCATOR_H_HEADER_GUARD

#include "bx.h"

#include <memory.h>
#include <string.h> //::memmove
#include <new>

#if BX_CONFIG_ALLOCATOR_CRT
#	include <malloc.h>
#endif // BX_CONFIG_ALLOCATOR_CRT

#if BX_CONFIG_ALLOCATOR_DEBUG
#	define BX_ALLOC(_allocator, _size)                         bx::alloc(_allocator, _size, 0, __FILE__, __LINE__)
#	define BX_REALLOC(_allocator, _ptr, _size)                 bx::realloc(_allocator, _ptr, _size, 0, __FILE__, __LINE__)
#	define BX_FREE(_allocator, _ptr)                           bx::free(_allocator, _ptr, 0, __FILE__, __LINE__)
#	define BX_ALIGNED_ALLOC(_allocator, _size, _align)         bx::alloc(_allocator, _size, _align, __FILE__, __LINE__)
#	define BX_ALIGNED_REALLOC(_allocator, _ptr, _size, _align) bx::realloc(_allocator, _ptr, _size, _align, __FILE__, __LINE__)
#	define BX_ALIGNED_FREE(_allocator, _ptr, _align)           bx::free(_allocator, _ptr, _align, __FILE__, __LINE__)
#	define BX_NEW(_allocator, _type)                           ::new(BX_ALLOC(_allocator, sizeof(_type) ) ) _type
#	define BX_DELETE(_allocator, _ptr)                         bx::deleteObject(_allocator, _ptr, 0, __FILE__, __LINE__)
#	define BX_ALIGNED_NEW(_allocator, _type, _align)           ::new(BX_ALIGNED_ALLOC(_allocator, sizeof(_type), _align) ) _type
#	define BX_ALIGNED_DELETE(_allocator, _ptr, _align)         bx::deleteObject(_allocator, _ptr, _align, __FILE__, __LINE__)
#else
#	define BX_ALLOC(_allocator, _size)                         bx::alloc(_allocator, _size, 0)
#	define BX_REALLOC(_allocator, _ptr, _size)                 bx::realloc(_allocator, _ptr, _size, 0)
#	define BX_FREE(_allocator, _ptr)                           bx::free(_allocator, _ptr, 0)
#	define BX_ALIGNED_ALLOC(_allocator, _size, _align)         bx::alloc(_allocator, _size, _align)
#	define BX_ALIGNED_REALLOC(_allocator, _ptr, _size, _align) bx::realloc(_allocator, _ptr, _size, _align)
#	define BX_ALIGNED_FREE(_allocator, _ptr, _align)           bx::free(_allocator, _ptr, _align)
#	define BX_NEW(_allocator, _type)                           ::new(BX_ALLOC(_allocator, sizeof(_type) ) ) _type
#	define BX_DELETE(_allocator, _ptr)                         bx::deleteObject(_allocator, _ptr, 0)
#	define BX_ALIGNED_NEW(_allocator, _type, _align)           ::new(BX_ALIGNED_ALLOC(_allocator, sizeof(_type), _align) ) _type
#	define BX_ALIGNED_DELETE(_allocator, _ptr, _align)         bx::deleteObject(_allocator, _ptr, _align)
#endif // BX_CONFIG_DEBUG_ALLOC

#ifndef BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT
#	define BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT 8
#endif // BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT

namespace bx
{
	/// Aligns pointer to nearest next aligned address. _align must be power of two.
	inline void* alignPtr(void* _ptr, size_t _extra, size_t _align = BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT)
	{
		union { void* ptr; size_t addr; } un;
		un.ptr = _ptr;
		size_t unaligned = un.addr + _extra; // space for header
		size_t mask = _align-1;
		size_t aligned = BX_ALIGN_MASK(unaligned, mask);
		un.addr = aligned;
		return un.ptr;
	}

	struct BX_NO_VTABLE AllocatorI
	{
		virtual ~AllocatorI() = 0;

		/// Allocated, resizes memory block or frees memory.
		///
		/// @param[in] _ptr If _ptr is NULL new block will be allocated.
		/// @param[in] _size If _ptr is set, and _size is 0, memory will be freed.
		/// @param[in] _align Alignment.
		/// @param[in] _file Debug file path info.
		/// @param[in] _line Debug file line info.
		virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line) = 0;
	};

	inline AllocatorI::~AllocatorI()
	{
	}

	inline void* alloc(AllocatorI* _allocator, size_t _size, size_t _align = 0, const char* _file = NULL, uint32_t _line = 0)
	{
		return _allocator->realloc(NULL, _size, _align, _file, _line);
	}

	inline void free(AllocatorI* _allocator, void* _ptr, size_t _align = 0, const char* _file = NULL, uint32_t _line = 0)
	{
		_allocator->realloc(_ptr, 0, _align, _file, _line);
	}

	inline void* realloc(AllocatorI* _allocator, void* _ptr, size_t _size, size_t _align = 0, const char* _file = NULL, uint32_t _line = 0)
	{
		return _allocator->realloc(_ptr, _size, _align, _file, _line);
	}

	static inline void* alignedAlloc(AllocatorI* _allocator, size_t _size, size_t _align, const char* _file = NULL, uint32_t _line = 0)
	{
		size_t total = _size + _align;
		uint8_t* ptr = (uint8_t*)alloc(_allocator, total, 0, _file, _line);
		uint8_t* aligned = (uint8_t*)alignPtr(ptr, sizeof(uint32_t), _align);
		uint32_t* header = (uint32_t*)aligned - 1;
		*header = uint32_t(aligned - ptr);
		return aligned;
	}

	static inline void alignedFree(AllocatorI* _allocator, void* _ptr, size_t /*_align*/, const char* _file = NULL, uint32_t _line = 0)
	{
		uint8_t* aligned = (uint8_t*)_ptr;
		uint32_t* header = (uint32_t*)aligned - 1;
		uint8_t* ptr = aligned - *header;
		free(_allocator, ptr, 0, _file, _line);
	}

	static inline void* alignedRealloc(AllocatorI* _allocator, void* _ptr, size_t _size, size_t _align, const char* _file = NULL, uint32_t _line = 0)
	{
		if (NULL == _ptr)
		{
			return alignedAlloc(_allocator, _size, _align, _file, _line);
		}

		uint8_t* aligned = (uint8_t*)_ptr;
		uint32_t offset = *( (uint32_t*)aligned - 1);
		uint8_t* ptr = aligned - offset;
		size_t total = _size + _align;
		ptr = (uint8_t*)realloc(_allocator, ptr, total, 0, _file, _line);
		uint8_t* newAligned = (uint8_t*)alignPtr(ptr, sizeof(uint32_t), _align);

		if (newAligned == aligned)
		{
			return aligned;
		}

		aligned = ptr + offset;
		::memmove(newAligned, aligned, _size);
		uint32_t* header = (uint32_t*)newAligned - 1;
		*header = uint32_t(newAligned - ptr);
		return newAligned;
	}

	template <typename ObjectT>
	inline void deleteObject(AllocatorI* _allocator, ObjectT* _object, size_t _align = 0, const char* _file = NULL, uint32_t _line = 0)
	{
		if (NULL != _object)
		{
			_object->~ObjectT();
			free(_allocator, _object, _align, _file, _line);
		}
	}

#if BX_CONFIG_ALLOCATOR_CRT
	class CrtAllocator : public AllocatorI
	{
	public:
		CrtAllocator()
		{
		}

		virtual ~CrtAllocator()
		{
		}

		virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line) BX_OVERRIDE
		{
			if (0 == _size)
			{
				if (NULL != _ptr)
				{
					if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= _align)
					{
						::free(_ptr);
						return NULL;
					}

#	if BX_COMPILER_MSVC
					BX_UNUSED(_file, _line);
					_aligned_free(_ptr);
#	else
					bx::alignedFree(this, _ptr, _align, _file, _line);
#	endif // BX_
				}

				return NULL;
			}
			else if (NULL == _ptr)
			{
				if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= _align)
				{
					return ::malloc(_size);
				}

#	if BX_COMPILER_MSVC
				BX_UNUSED(_file, _line);
				return _aligned_malloc(_size, _align);
#	else
				return bx::alignedAlloc(this, _size, _align, _file, _line);
#	endif // BX_
			}

			if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= _align)
			{
				return ::realloc(_ptr, _size);
			}

#	if BX_COMPILER_MSVC
			BX_UNUSED(_file, _line);
			return _aligned_realloc(_ptr, _size, _align);
#	else
			return bx::alignedRealloc(this, _ptr, _size, _align, _file, _line);
#	endif // BX_
		}
	};
#endif // BX_CONFIG_ALLOCATOR_CRT

} // namespace bx

#endif // BX_ALLOCATOR_H_HEADER_GUARD
