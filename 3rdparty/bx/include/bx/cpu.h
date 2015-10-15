/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#ifndef BX_CPU_H_HEADER_GUARD
#define BX_CPU_H_HEADER_GUARD

#include "bx.h"

#if BX_COMPILER_MSVC
#	if BX_PLATFORM_XBOX360
#		include <ppcintrinsics.h>
#		include <xtl.h>
#	else
#		include <math.h> // math.h is included because VS bitches:
						 // warning C4985: 'ceil': attributes not present on previous declaration.
						 // must be included before intrin.h.
#		include <intrin.h>
#		include <windows.h>
#	endif // !BX_PLATFORM_XBOX360
extern "C" void _ReadBarrier();
extern "C" void _WriteBarrier();
extern "C" void _ReadWriteBarrier();
#	pragma intrinsic(_ReadBarrier)
#	pragma intrinsic(_WriteBarrier)
#	pragma intrinsic(_ReadWriteBarrier)
#	pragma intrinsic(_InterlockedExchangeAdd)
#	pragma intrinsic(_InterlockedCompareExchange)
#endif // BX_COMPILER_MSVC

namespace bx
{
	///
	inline void readBarrier()
	{
#if BX_COMPILER_MSVC
		_ReadBarrier();
#else
		asm volatile("":::"memory");
#endif // BX_COMPILER
	}

	///
	inline void writeBarrier()
	{
#if BX_COMPILER_MSVC
		_WriteBarrier();
#else
		asm volatile("":::"memory");
#endif // BX_COMPILER
	}

	///
	inline void readWriteBarrier()
	{
#if BX_COMPILER_MSVC
		_ReadWriteBarrier();
#else
		asm volatile("":::"memory");
#endif // BX_COMPILER
	}

	///
	inline void memoryBarrier()
	{
#if BX_PLATFORM_XBOX360
		__lwsync();
#elif BX_PLATFORM_WINRT
		MemoryBarrier();
#elif BX_COMPILER_MSVC
		_mm_mfence();
#else
		__sync_synchronize();
//		asm volatile("mfence":::"memory");
#endif // BX_COMPILER
	}

	template<typename Ty>
	inline Ty atomicFetchAndAdd(volatile Ty* _ptr, Ty _value);

	template<typename Ty>
	inline Ty atomicAddAndFetch(volatile Ty* _ptr, Ty _value);

	template<typename Ty>
	inline Ty atomicFetchAndSub(volatile Ty* _ptr, Ty _value);

	template<typename Ty>
	inline Ty atomicSubAndFetch(volatile Ty* _ptr, Ty _value);

	template<typename Ty>
	inline Ty atomicCompareAndSwap(volatile void* _ptr, Ty _old, Ty _new);

	template<>
	inline int32_t atomicCompareAndSwap(volatile void* _ptr, int32_t _old, int32_t _new);

	template<>
	inline int64_t atomicCompareAndSwap(volatile void* _ptr, int64_t _old, int64_t _new);

	template<>
	inline int32_t atomicFetchAndAdd<int32_t>(volatile int32_t* _ptr, int32_t _add)
	{
#if BX_COMPILER_MSVC
		return _InterlockedExchangeAdd( (volatile long*)_ptr, _add);
#else
		return __sync_fetch_and_add(_ptr, _add);
#endif // BX_COMPILER_
	}

	template<>
	inline int64_t atomicFetchAndAdd<int64_t>(volatile int64_t* _ptr, int64_t _add)
	{
#if BX_COMPILER_MSVC
#	if _WIN32_WINNT >= 0x600
		return _InterlockedExchangeAdd64( (volatile int64_t*)_ptr, _add);
#	else
		int64_t oldVal;
		int64_t newVal = *(int64_t volatile*)_ptr;
		do
		{
			oldVal = newVal;
			newVal = atomicCompareAndSwap(_ptr, oldVal, newVal + _add);

		} while (oldVal != newVal);

		return oldVal;
#	endif
#else
		return __sync_fetch_and_add(_ptr, _add);
#endif // BX_COMPILER_
	}

	template<>
	inline uint32_t atomicFetchAndAdd<uint32_t>(volatile uint32_t* _ptr, uint32_t _add)
	{
		return uint32_t(atomicFetchAndAdd<int32_t>( (volatile int32_t*)_ptr, int32_t(_add) ) );
	}

	template<>
	inline uint64_t atomicFetchAndAdd<uint64_t>(volatile uint64_t* _ptr, uint64_t _add)
	{
		return uint64_t(atomicFetchAndAdd<int64_t>( (volatile int64_t*)_ptr, int64_t(_add) ) );
	}

	template<>
	inline int32_t atomicAddAndFetch<int32_t>(volatile int32_t* _ptr, int32_t _add)
	{
#if BX_COMPILER_MSVC
		return atomicFetchAndAdd(_ptr, _add) + _add;
#else
		return __sync_add_and_fetch(_ptr, _add);
#endif // BX_COMPILER_
	}

	template<>
	inline int64_t atomicAddAndFetch<int64_t>(volatile int64_t* _ptr, int64_t _add)
	{
#if BX_COMPILER_MSVC
		return atomicFetchAndAdd(_ptr, _add) + _add;
#else
		return __sync_add_and_fetch(_ptr, _add);
#endif // BX_COMPILER_
	}

	template<>
	inline uint32_t atomicAddAndFetch<uint32_t>(volatile uint32_t* _ptr, uint32_t _add)
	{
		return uint32_t(atomicAddAndFetch<int32_t>( (volatile int32_t*)_ptr, int32_t(_add) ) );
	}

	template<>
	inline uint64_t atomicAddAndFetch<uint64_t>(volatile uint64_t* _ptr, uint64_t _add)
	{
		return uint64_t(atomicAddAndFetch<int64_t>( (volatile int64_t*)_ptr, int64_t(_add) ) );
	}

	template<>
	inline int32_t atomicFetchAndSub<int32_t>(volatile int32_t* _ptr, int32_t _sub)
	{
#if BX_COMPILER_MSVC
		return atomicFetchAndAdd(_ptr, -_sub);
#else
		return __sync_fetch_and_sub(_ptr, _sub);
#endif // BX_COMPILER_
	}

	template<>
	inline int64_t atomicFetchAndSub<int64_t>(volatile int64_t* _ptr, int64_t _sub)
	{
#if BX_COMPILER_MSVC
		return atomicFetchAndAdd(_ptr, -_sub);
#else
		return __sync_fetch_and_sub(_ptr, _sub);
#endif // BX_COMPILER_
	}

	template<>
	inline uint32_t atomicFetchAndSub<uint32_t>(volatile uint32_t* _ptr, uint32_t _add)
	{
		return uint32_t(atomicFetchAndSub<int32_t>( (volatile int32_t*)_ptr, int32_t(_add) ) );
	}

	template<>
	inline uint64_t atomicFetchAndSub<uint64_t>(volatile uint64_t* _ptr, uint64_t _add)
	{
		return uint64_t(atomicFetchAndSub<int64_t>( (volatile int64_t*)_ptr, int64_t(_add) ) );
	}

	template<>
	inline int32_t atomicSubAndFetch<int32_t>(volatile int32_t* _ptr, int32_t _sub)
	{
#if BX_COMPILER_MSVC
		return atomicFetchAndAdd(_ptr, -_sub) - _sub;
#else
		return __sync_sub_and_fetch(_ptr, _sub);
#endif // BX_COMPILER_
	}

	template<>
	inline int64_t atomicSubAndFetch<int64_t>(volatile int64_t* _ptr, int64_t _sub)
	{
#if BX_COMPILER_MSVC
		return atomicFetchAndAdd(_ptr, -_sub) - _sub;
#else
		return __sync_sub_and_fetch(_ptr, _sub);
#endif // BX_COMPILER_
	}

	template<>
	inline uint32_t atomicSubAndFetch<uint32_t>(volatile uint32_t* _ptr, uint32_t _add)
	{
		return uint32_t(atomicSubAndFetch<int32_t>( (volatile int32_t*)_ptr, int32_t(_add) ) );
	}

	template<>
	inline uint64_t atomicSubAndFetch<uint64_t>(volatile uint64_t* _ptr, uint64_t _add)
	{
		return uint64_t(atomicSubAndFetch<int64_t>( (volatile int64_t*)_ptr, int64_t(_add) ) );
	}

	/// Returns the resulting incremented value.
	template<typename Ty>
	inline Ty atomicInc(volatile Ty* _ptr)
	{
		return atomicAddAndFetch(_ptr, Ty(1) );
	}

	/// Returns the resulting decremented value.
	template<typename Ty>
	inline Ty atomicDec(volatile Ty* _ptr)
	{
		return atomicSubAndFetch(_ptr, Ty(1) );
	}

	///
	template<>
	inline int32_t atomicCompareAndSwap(volatile void* _ptr, int32_t _old, int32_t _new)
	{
#if BX_COMPILER_MSVC
		return _InterlockedCompareExchange( (volatile LONG*)(_ptr), _new, _old);
#else
		return __sync_val_compare_and_swap( (volatile int32_t*)_ptr, _old, _new);
#endif // BX_COMPILER
	}

	///
	template<>
	inline int64_t atomicCompareAndSwap(volatile void* _ptr, int64_t _old, int64_t _new)
	{
#if BX_COMPILER_MSVC
		return _InterlockedCompareExchange64( (volatile LONG64*)(_ptr), _new, _old);
#else
		return __sync_val_compare_and_swap( (volatile int64_t*)_ptr, _old, _new);
#endif // BX_COMPILER
	}

	///
	inline void* atomicExchangePtr(void** _ptr, void* _new)
	{
#if BX_COMPILER_MSVC
		return InterlockedExchangePointer(_ptr, _new);
#else
		return __sync_lock_test_and_set(_ptr, _new);
#endif // BX_COMPILER
	}

	///
	inline int32_t atomicTestAndInc(volatile void* _ptr, int32_t _test)
	{
		int32_t oldVal;
		int32_t newVal = *(int32_t volatile*)_ptr;
		do
		{
			oldVal = newVal;
			newVal = atomicCompareAndSwap(_ptr, oldVal, newVal >= _test ? _test : newVal+1);

		} while (oldVal != newVal);

		return oldVal;
	}

	///
	inline int32_t atomicTestAndDec(volatile void* _ptr, int32_t _test)
	{
		int32_t oldVal;
		int32_t newVal = *(int32_t volatile*)_ptr;
		do
		{
			oldVal = newVal;
			newVal = atomicCompareAndSwap(_ptr, oldVal, newVal <= _test ? _test : newVal-1);

		} while (oldVal != newVal);

		return oldVal;
	}

} // namespace bx

#endif // BX_CPU_H_HEADER_GUARD
