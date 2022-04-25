/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/sort.h>

namespace bx
{
	static void quickSortR(void* _pivot, void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn)
	{
		if (2 > _num)
		{
			return;
		}

		memCopy(_pivot, _data, _stride);

		uint8_t* data = (uint8_t*)_data;

		uint32_t ll = 0;
		uint32_t gg = 1;

		for (uint32_t ii = 1; ii < _num;)
		{
			int32_t result = _fn(&data[ii*_stride], _pivot);
			if (0 > result)
			{
				swap(&data[ll*_stride], &data[ii*_stride], _stride);
				++ll;
			}
			else if (0 == result)
			{
				swap(&data[gg*_stride], &data[ii*_stride], _stride);
				++gg;
				++ii;
			}
			else
			{
				++ii;
			}
		}

		quickSortR(_pivot, &data[0         ], ll,      _stride, _fn);
		quickSortR(_pivot, &data[gg*_stride], _num-gg, _stride, _fn);
	}

	void quickSort(void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn)
	{
		uint8_t* pivot = (uint8_t*)alloca(_stride);
		quickSortR(pivot, _data, _num, _stride, _fn);
	}

} // namespace bx

