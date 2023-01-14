/*
 * Copyright 2010-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_ALLOCATOR_H_HEADER_GUARD
#define BX_ALLOCATOR_H_HEADER_GUARD

#include "bx.h"
#include "uint32_t.h"

#if BX_CONFIG_ALLOCATOR_DEBUG
#	define BX_ALLOC(_allocator, _size)                         bx::alloc(_allocator, _size, 0, __FILE__, __LINE__)
#	define BX_REALLOC(_allocator, _ptr, _size)                 bx::realloc(_allocator, _ptr, _size, 0, __FILE__, __LINE__)
#	define BX_FREE(_allocator, _ptr)                           bx::free(_allocator, _ptr, 0, __FILE__, __LINE__)
#	define BX_ALIGNED_ALLOC(_allocator, _size, _align)         bx::alloc(_allocator, _size, _align, __FILE__, __LINE__)
#	define BX_ALIGNED_REALLOC(_allocator, _ptr, _size, _align) bx::realloc(_allocator, _ptr, _size, _align, __FILE__, __LINE__)
#	define BX_ALIGNED_FREE(_allocator, _ptr, _align)           bx::free(_allocator, _ptr, _align, __FILE__, __LINE__)
#	define BX_DELETE(_allocator, _ptr)                         bx::deleteObject(_allocator, _ptr, 0, __FILE__, __LINE__)
#	define BX_ALIGNED_DELETE(_allocator, _ptr, _align)         bx::deleteObject(_allocator, _ptr, _align, __FILE__, __LINE__)
#else
#	define BX_ALLOC(_allocator, _size)                         bx::alloc(_allocator, _size, 0)
#	define BX_REALLOC(_allocator, _ptr, _size)                 bx::realloc(_allocator, _ptr, _size, 0)
#	define BX_FREE(_allocator, _ptr)                           bx::free(_allocator, _ptr, 0)
#	define BX_ALIGNED_ALLOC(_allocator, _size, _align)         bx::alloc(_allocator, _size, _align)
#	define BX_ALIGNED_REALLOC(_allocator, _ptr, _size, _align) bx::realloc(_allocator, _ptr, _size, _align)
#	define BX_ALIGNED_FREE(_allocator, _ptr, _align)           bx::free(_allocator, _ptr, _align)
#	define BX_DELETE(_allocator, _ptr)                         bx::deleteObject(_allocator, _ptr, 0)
#	define BX_ALIGNED_DELETE(_allocator, _ptr, _align)         bx::deleteObject(_allocator, _ptr, _align)
#endif // BX_CONFIG_DEBUG_ALLOC

#define BX_NEW(_allocator, _type)                 BX_PLACEMENT_NEW(BX_ALLOC(_allocator, sizeof(_type) ), _type)
#define BX_ALIGNED_NEW(_allocator, _type, _align) BX_PLACEMENT_NEW(BX_ALIGNED_ALLOC(_allocator, sizeof(_type), _align), _type)
#define BX_PLACEMENT_NEW(_ptr, _type)             ::new(bx::PlacementNew, _ptr) _type

namespace bx
{
	struct    PlacementNewTag {};
	constexpr PlacementNewTag PlacementNew;

} // namespace bx

void* operator new(size_t, bx::PlacementNewTag, void* _ptr);
void  operator delete(void*, bx::PlacementNewTag, void*) throw();

namespace bx
{
	/// Abstract allocator interface.
	///
	struct BX_NO_VTABLE AllocatorI
	{
		///
		virtual ~AllocatorI() = 0;

		/// Allocates, resizes, or frees memory block.
		///
		/// @remark
		///  - Allocate memory block: _ptr == NULL && size > 0
		///  -   Resize memory block: _ptr != NULL && size > 0
		///  -     Free memory block: _ptr != NULL && size == 0
		///
		/// @param[in] _ptr If _ptr is NULL new block will be allocated. If _ptr is not-NULL, and
		///   _size is not 0, memory block will be resized.
		/// @param[in] _size If _ptr is set, and _size is 0, memory will be freed.
		/// @param[in] _align Alignment.
		/// @param[in] _file Debug file path info.
		/// @param[in] _line Debug file line info.
		///
		virtual void* realloc(
			  void* _ptr
			, size_t _size
			, size_t _align
			, const char* _file
			, uint32_t _line
			) = 0;
	};

	///
	class DefaultAllocator : public AllocatorI
	{
	public:
		///
		DefaultAllocator();

		///
		virtual ~DefaultAllocator();

		///
		virtual void* realloc(
			  void* _ptr
			, size_t _size
			, size_t _align
			, const char* _file
			, uint32_t _line
			) override;
	};

	/// Aligns pointer to nearest next aligned address. _align must be power of two.
	void* alignPtr(
		  void* _ptr
		, size_t _extra
		, size_t _align = 0
		);

	/// Allocate memory.
	void* alloc(
		  AllocatorI* _allocator
		, size_t _size
		, size_t _align = 0
		, const char* _file = NULL
		, uint32_t _line = 0
		);

	/// Free memory.
	void free(
		  AllocatorI* _allocator
		, void* _ptr
		, size_t _align = 0
		, const char* _file = NULL
		, uint32_t _line = 0
		);

	/// Resize memory block.
	void* realloc(
		  AllocatorI* _allocator
		, void* _ptr
		, size_t _size
		, size_t _align = 0
		, const char* _file = NULL
		, uint32_t _line = 0
		);

	/// Allocate memory with specific alignment.
	void* alignedAlloc(
		  AllocatorI* _allocator
		, size_t _size
		, size_t _align
		, const char* _file = NULL
		, uint32_t _line = 0
		);

	/// Free memory that was allocated with aligned allocator.
	void alignedFree(
		  AllocatorI* _allocator
		, void* _ptr
		, size_t /*_align*/
		, const char* _file = NULL
		, uint32_t _line = 0
		);

	/// Resize memory block that was allocated with aligned allocator.
	void* alignedRealloc(
		  AllocatorI* _allocator
		, void* _ptr
		, size_t _size
		, size_t _align
		, const char* _file = NULL
		, uint32_t _line = 0
		);

	/// Delete object with specific allocator.
	template <typename ObjectT>
	void deleteObject(
		  AllocatorI* _allocator
		, ObjectT* _object
		, size_t _align = 0
		, const char* _file = NULL
		, uint32_t _line = 0
		);

} // namespace bx

#include "inline/allocator.inl"

#endif // BX_ALLOCATOR_H_HEADER_GUARD
