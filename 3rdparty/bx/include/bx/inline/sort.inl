/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SORT_H_HEADER_GUARD
#	error "Must be included from bx/sort.h!"
#endif // BX_SORT_H_HEADER_GUARD

namespace bx
{
	namespace radix_sort_detail
	{
		constexpr uint32_t kBits          = 11;
		constexpr uint32_t kHistogramSize = 1<<kBits;
		constexpr uint32_t kBitMask       = kHistogramSize-1;

	} // namespace radix_sort_detail

	inline void radixSort(uint32_t* _keys, uint32_t* _tempKeys, uint32_t _size)
	{
		uint32_t* keys = _keys;
		uint32_t* tempKeys = _tempKeys;

		uint32_t histogram[radix_sort_detail::kHistogramSize];
		uint16_t shift = 0;
		uint32_t pass = 0;
		for (; pass < 3; ++pass)
		{
			memSet(histogram, 0, sizeof(uint32_t)*radix_sort_detail::kHistogramSize);

			bool sorted = true;
			{
				uint32_t key = keys[0];
				uint32_t prevKey = key;
				for (uint32_t ii = 0; ii < _size; ++ii, prevKey = key)
				{
					key = keys[ii];
					uint16_t index = (key>>shift)&radix_sort_detail::kBitMask;
					++histogram[index];
					sorted &= prevKey <= key;
				}
			}

			if (sorted)
			{
				goto done;
			}

			uint32_t offset = 0;
			for (uint32_t ii = 0; ii < radix_sort_detail::kHistogramSize; ++ii)
			{
				uint32_t count = histogram[ii];
				histogram[ii] = offset;
				offset += count;
			}

			for (uint32_t ii = 0; ii < _size; ++ii)
			{
				uint32_t key = keys[ii];
				uint16_t index = (key>>shift)&radix_sort_detail::kBitMask;
				uint32_t dest = histogram[index]++;
				tempKeys[dest] = key;
			}

			uint32_t* swapKeys = tempKeys;
			tempKeys = keys;
			keys = swapKeys;

			shift += radix_sort_detail::kBits;
		}

done:
		if (0 != (pass&1) )
		{
			// Odd number of passes needs to do copy to the destination.
			memCopy(_keys, _tempKeys, _size*sizeof(uint32_t) );
		}
	}

	template <typename Ty>
	inline void radixSort(uint32_t* _keys, uint32_t* _tempKeys, Ty* _values, Ty* _tempValues, uint32_t _size)
	{
		uint32_t* keys = _keys;
		uint32_t* tempKeys = _tempKeys;
		Ty* values = _values;
		Ty* tempValues = _tempValues;

		uint32_t histogram[radix_sort_detail::kHistogramSize];
		uint16_t shift = 0;
		uint32_t pass = 0;
		for (; pass < 3; ++pass)
		{
			memSet(histogram, 0, sizeof(uint32_t)*radix_sort_detail::kHistogramSize);

			bool sorted = true;
			{
				uint32_t key = keys[0];
				uint32_t prevKey = key;
				for (uint32_t ii = 0; ii < _size; ++ii, prevKey = key)
				{
					key = keys[ii];
					uint16_t index = (key>>shift)&radix_sort_detail::kBitMask;
					++histogram[index];
					sorted &= prevKey <= key;
				}
			}

			if (sorted)
			{
				goto done;
			}

			uint32_t offset = 0;
			for (uint32_t ii = 0; ii < radix_sort_detail::kHistogramSize; ++ii)
			{
				uint32_t count = histogram[ii];
				histogram[ii] = offset;
				offset += count;
			}

			for (uint32_t ii = 0; ii < _size; ++ii)
			{
				uint32_t key = keys[ii];
				uint16_t index = (key>>shift)&radix_sort_detail::kBitMask;
				uint32_t dest = histogram[index]++;
				tempKeys[dest] = key;
				tempValues[dest] = values[ii];
			}

			uint32_t* swapKeys = tempKeys;
			tempKeys = keys;
			keys = swapKeys;

			Ty* swapValues = tempValues;
			tempValues = values;
			values = swapValues;

			shift += radix_sort_detail::kBits;
		}

done:
		if (0 != (pass&1) )
		{
			// Odd number of passes needs to do copy to the destination.
			memCopy(_keys, _tempKeys, _size*sizeof(uint32_t) );
			for (uint32_t ii = 0; ii < _size; ++ii)
			{
				_values[ii] = _tempValues[ii];
			}
		}
	}

	inline void radixSort(uint64_t* _keys, uint64_t* _tempKeys, uint32_t _size)
	{
		uint64_t* keys = _keys;
		uint64_t* tempKeys = _tempKeys;

		uint32_t histogram[radix_sort_detail::kHistogramSize];
		uint16_t shift = 0;
		uint32_t pass = 0;
		for (; pass < 6; ++pass)
		{
			memSet(histogram, 0, sizeof(uint32_t)*radix_sort_detail::kHistogramSize);

			bool sorted = true;
			{
				uint64_t key = keys[0];
				uint64_t prevKey = key;
				for (uint32_t ii = 0; ii < _size; ++ii, prevKey = key)
				{
					key = keys[ii];
					uint16_t index = (key>>shift)&radix_sort_detail::kBitMask;
					++histogram[index];
					sorted &= prevKey <= key;
				}
			}

			if (sorted)
			{
				goto done;
			}

			uint32_t offset = 0;
			for (uint32_t ii = 0; ii < radix_sort_detail::kHistogramSize; ++ii)
			{
				uint32_t count = histogram[ii];
				histogram[ii] = offset;
				offset += count;
			}

			for (uint32_t ii = 0; ii < _size; ++ii)
			{
				uint64_t key = keys[ii];
				uint16_t index = (key>>shift)&radix_sort_detail::kBitMask;
				uint32_t dest = histogram[index]++;
				tempKeys[dest] = key;
			}

			uint64_t* swapKeys = tempKeys;
			tempKeys = keys;
			keys = swapKeys;

			shift += radix_sort_detail::kBits;
		}

done:
		if (0 != (pass&1) )
		{
			// Odd number of passes needs to do copy to the destination.
			memCopy(_keys, _tempKeys, _size*sizeof(uint64_t) );
		}
	}

	template <typename Ty>
	inline void radixSort(uint64_t* _keys, uint64_t* _tempKeys, Ty* _values, Ty* _tempValues, uint32_t _size)
	{
		uint64_t* keys = _keys;
		uint64_t* tempKeys = _tempKeys;
		Ty* values = _values;
		Ty* tempValues = _tempValues;

		uint32_t histogram[radix_sort_detail::kHistogramSize];
		uint16_t shift = 0;
		uint32_t pass = 0;
		for (; pass < 6; ++pass)
		{
			memSet(histogram, 0, sizeof(uint32_t)*radix_sort_detail::kHistogramSize);

			bool sorted = true;
			{
				uint64_t key = keys[0];
				uint64_t prevKey = key;
				for (uint32_t ii = 0; ii < _size; ++ii, prevKey = key)
				{
					key = keys[ii];
					uint16_t index = (key>>shift)&radix_sort_detail::kBitMask;
					++histogram[index];
					sorted &= prevKey <= key;
				}
			}

			if (sorted)
			{
				goto done;
			}

			uint32_t offset = 0;
			for (uint32_t ii = 0; ii < radix_sort_detail::kHistogramSize; ++ii)
			{
				uint32_t count = histogram[ii];
				histogram[ii] = offset;
				offset += count;
			}

			for (uint32_t ii = 0; ii < _size; ++ii)
			{
				uint64_t key = keys[ii];
				uint16_t index = (key>>shift)&radix_sort_detail::kBitMask;
				uint32_t dest = histogram[index]++;
				tempKeys[dest] = key;
				tempValues[dest] = values[ii];
			}

			uint64_t* swapKeys = tempKeys;
			tempKeys = keys;
			keys = swapKeys;

			Ty* swapValues = tempValues;
			tempValues = values;
			values = swapValues;

			shift += radix_sort_detail::kBits;
		}

done:
		if (0 != (pass&1) )
		{
			// Odd number of passes needs to do copy to the destination.
			memCopy(_keys, _tempKeys, _size*sizeof(uint64_t) );
			for (uint32_t ii = 0; ii < _size; ++ii)
			{
				_values[ii] = _tempValues[ii];
			}
		}
	}

} // namespace bx
