/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

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

	bool isSorted(const void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn)
	{
		const uint8_t* data = (uint8_t*)_data;

		for (uint32_t ii = 1; ii < _num; ++ii)
		{
			int32_t result = _fn(&data[(ii-1)*_stride], &data[ii*_stride]);

			if (0 < result)
			{
				return false;
			}
		}

		return true;
	}

	uint32_t unique(void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn)
	{
		if (0 == _num)
		{
			return 0;
		}

		uint8_t* data = (uint8_t*)_data;

		uint32_t last = 0;

		for (uint32_t ii = 1; ii < _num; ++ii)
		{
			int32_t result = _fn(&data[last*_stride], &data[ii*_stride]);
			BX_ASSERT(0 >= result, "Performing unique on non-sorted array (ii %d, last %d)!", ii, last);

			if (0 > result)
			{
				last++;
				swap(&data[last*_stride], &data[ii*_stride], _stride);
			}
		}

		return last+1;
	}

	uint32_t lowerBound(const void* _key, const void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn)
	{
		uint32_t offset = 0;
		const uint8_t* data = (uint8_t*)_data;

		for (uint32_t ll = _num; offset < ll;)
		{
			const uint32_t idx = (offset + ll) / 2;

			int32_t result = _fn(_key, &data[idx * _stride]);

			if (result <= 0)
			{
				ll = idx;
			}
			else
			{
				offset = idx + 1;
			}
		}

		return offset;
	}

	uint32_t upperBound(const void* _key, const void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn)
	{
		uint32_t offset = 0;
		const uint8_t* data = (uint8_t*)_data;

		for (uint32_t ll = _num; offset < ll;)
		{
			const uint32_t idx = (offset + ll) / 2;

			int32_t result = _fn(_key, &data[idx * _stride]);

			if (result < 0)
			{
				ll = idx;
			}
			else
			{
				offset = idx + 1;
			}
		}

		return offset;
	}

	int32_t binarySearch(const void* _key, const void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn)
	{
		uint32_t offset = 0;
		const uint8_t* data = (uint8_t*)_data;

		for (uint32_t ll = _num; offset < ll;)
		{
			const uint32_t idx = (offset + ll) / 2;

			int32_t result = _fn(_key, &data[idx * _stride]);

			if (result < 0)
			{
				ll = idx;
			}
			else if (result > 0)
			{
				offset = idx + 1;
			}
			else
			{
				return idx;
			}
		}

		return ~offset;
	}

} // namespace bx

