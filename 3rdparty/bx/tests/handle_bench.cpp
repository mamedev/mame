/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include <bx/timer.h>
#include <bx/handlealloc.h>
#include <bx/maputil.h>

#include <tinystl/allocator.h>
#include <tinystl/unordered_map.h>

#include <unordered_map>

#include <stdio.h>
#include <assert.h>

int main()
{
	const uint32_t numElements   = 4<<10;
	const uint32_t numIterations = 16;

	//
	{
		int64_t elapsed = -bx::getHPCounter();

		for (uint32_t ii = 0; ii < numIterations; ++ii)
		{
			typedef tinystl::unordered_map<uint64_t, uint16_t> TinyStlUnorderedMap;
			TinyStlUnorderedMap map;
//			map.reserve(numElements);
			for (uint32_t jj = 0; jj < numElements; ++jj)
			{
				tinystl::pair<TinyStlUnorderedMap::iterator, bool> ok = map.insert(tinystl::make_pair(uint64_t(jj), uint16_t(jj) ) );
				assert(ok.second); BX_UNUSED(ok);
			}

			for (uint32_t jj = 0; jj < numElements; ++jj)
			{
				bool ok = bx::mapRemove(map, uint64_t(jj) );
				assert(ok); BX_UNUSED(ok);
			}

			assert(map.size() == 0);
		}

		elapsed += bx::getHPCounter();
		printf("      TinyStl: %15f\n", double(elapsed) );
	}

	///
	{
		int64_t elapsed = -bx::getHPCounter();

		for (uint32_t ii = 0; ii < numIterations; ++ii)
		{
			typedef std::unordered_map<uint64_t, uint16_t> StdUnorderedMap;
			StdUnorderedMap map;
			map.reserve(numElements);
			for (uint32_t jj = 0; jj < numElements; ++jj)
			{
				std::pair<StdUnorderedMap::iterator, bool> ok = map.insert(std::make_pair(uint64_t(jj), uint16_t(jj) ) );
				assert(ok.second); BX_UNUSED(ok);
			}

			for (uint32_t jj = 0; jj < numElements; ++jj)
			{
				bool ok = bx::mapRemove(map, uint64_t(jj) );
				assert(ok); BX_UNUSED(ok);
			}

			assert(map.size() == 0);
		}

		elapsed += bx::getHPCounter();
		printf("          STL: %15f\n", double(elapsed) );
	}

	///
	{
		int64_t elapsed = -bx::getHPCounter();

		for (uint32_t ii = 0; ii < numIterations; ++ii)
		{
			typedef bx::HandleHashMapT<numElements+numElements/2, uint64_t> HandleHashMap;
			HandleHashMap map;
			for (uint32_t jj = 0; jj < numElements; ++jj)
			{
				bool ok = map.insert(jj, uint16_t(jj) );
				assert(ok); BX_UNUSED(ok);
			}

			for (uint32_t jj = 0; jj < numElements; ++jj)
			{
				bool ok = map.removeByKey(uint64_t(jj) );
				assert(ok); BX_UNUSED(ok);
			}

			assert(map.getNumElements() == 0);
		}

		elapsed += bx::getHPCounter();
		printf("HandleHashMap: %15f\n", double(elapsed) );
	}

	extern void simd_bench();
	simd_bench();

	extern void math_bench();
	math_bench();

	return bx::kExitSuccess;
}
