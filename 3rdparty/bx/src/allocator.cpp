/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include <bx/allocator.h>

#include <malloc.h>

#ifndef BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT
#	define BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT 8
#endif // BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT

namespace bx
{
	DefaultAllocator::DefaultAllocator()
	{
	}

	DefaultAllocator::~DefaultAllocator()
	{
	}

	void* DefaultAllocator::realloc(void* _ptr, size_t _size, size_t _align, const char* _filePath, uint32_t _line)
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
				BX_UNUSED(_filePath, _line);
				_aligned_free(_ptr);
#	else
				alignedFree(this, _ptr, _align, Location(_filePath, _line) );
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
			BX_UNUSED(_filePath, _line);
			return _aligned_malloc(_size, _align);
#	else
			return alignedAlloc(this, _size, _align, Location(_filePath, _line) );
#	endif // BX_
		}

		if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= _align)
		{
			return ::realloc(_ptr, _size);
		}

#	if BX_COMPILER_MSVC
		BX_UNUSED(_filePath, _line);
		return _aligned_realloc(_ptr, _size, _align);
#	else
		return alignedRealloc(this, _ptr, _size, _align, Location(_filePath, _line) );
#	endif // BX_
	}

} // namespace bx
