/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_SORT_H_HEADER_GUARD
#define BX_SORT_H_HEADER_GUARD

#include "bx.h"
#include "math.h"
#include "string.h"

namespace bx
{
	/// The function compares the `_lhs` and `_rhs` values.
	///
	/// @returns Returns value:
	///   - less than zero if `_lhs` is less than `_rhs`
	///   - zero if `_lhs` is equivalent to `_rhs`
	///   - greater than zero if `_lhs` is greater than `_rhs`
	///
	typedef int32_t (*ComparisonFn)(const void* _lhs, const void* _rhs);

	/// The function compares the `_lhs` and `_rhs` values.
	///
	/// @returns Returns value:
	///   - less than zero if `_lhs` is less than `_rhs`
	///   - zero if `_lhs` is equivalent to `_rhs`
	///   - greater than zero if `_lhs` is greater than `_rhs`
	///
	template<typename Ty>
	int32_t compareAscending(const void* _lhs, const void* _rhs);

	/// The function compares the `_lhs` and `_rhs` values.
	///
	/// @returns Returns value:
	///   - less than zero if `_lhs` is greated than `_rhs`
	///   - zero if `_lhs` is equivalent to `_rhs`
	///   - greater than zero if `_lhs` is less than `_rhs`
	///
	template<typename Ty>
	int32_t compareDescending(const void* _lhs, const void* _rhs);

	/// Performs sort (Quick Sort algorithm).
	///
	/// @param _data Pointer to array data.
	/// @param _num Number of elements.
	/// @param _stride Element stride in bytes.
	/// @param _fn Comparison function.
	///
	void quickSort(
		  void* _data
		, uint32_t _num
		, uint32_t _stride
		, const ComparisonFn _fn
		);

	/// Performs sort (Quick Sort algorithm).
	///
	/// @param _data Pointer to array data.
	/// @param _num Number of elements.
	/// @param _stride Element stride in bytes.
	/// @param _fn Comparison function.
	///
	template<typename Ty>
	void quickSort(void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn = compareAscending<Ty>);

	/// Performs sort (Quick Sort algorithm).
	///
	/// @param _data Pointer to array data.
	/// @param _num Number of elements.
	/// @param _fn Comparison function.
	///
	template<typename Ty>
	void quickSort(Ty* _data, uint32_t _num, const ComparisonFn _fn = compareAscending<Ty>);

	/// Performs reordering of duplicate elements in the array in the way that unique elements
	/// are sorted to the front of array, and duplicates are after the return value index.
	///
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _stride Element stride in bytes.
	/// @param _fn Comparison function.
	///
	/// @remarks Array must be sorted!
	///
	/// @returns Returns the count of unique elements.
	///
	uint32_t unique(void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn);

	/// Performs reordering of duplicate elements in the array in the way that unique elements
	/// are sorted to the front of array, and duplicates are after the return value index.
	///
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _stride Element stride in bytes.
	/// @param _fn Comparison function.
	///
	/// @remarks Array must be sorted!
	///
	/// @returns Returns the count of unique elements.
	///
	template<typename Ty>
	uint32_t unique(void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn = compareAscending<Ty>);

	/// Performs reordering of duplicate elements in the array in the way that unique elements
	/// are sorted to the front of array, and duplicates are after the return value index.
	///
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _fn Comparison function.
	///
	/// @remarks Array must be sorted!
	///
	/// @returns Returns the count of unique elements.
	///
	template<typename Ty>
	uint32_t unique(Ty* _data, uint32_t _num, const ComparisonFn _fn = compareAscending<Ty>);

	/// Performs check if array is sorted.
	///
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _stride Element stride in bytes.
	/// @param _fn Comparison function.
	///
	/// @returns Returns `true` if array is sorted, otherwise returns `false`.
	///
	bool isSorted(
		  const void* _data
		, uint32_t _num
		, uint32_t _stride
		, const ComparisonFn _fn
		);

	/// Performs check if array is sorted.
	///
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _stride Element stride in bytes.
	/// @param _fn Comparison function.
	///
	/// @returns Returns `true` if array is sorted, otherwise returns `false`.
	///
	template<typename Ty>
	bool isSorted(const void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn = compareAscending<Ty>);

	/// Performs check if array is sorted.
	///
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _fn Comparison function.
	///
	/// @returns Returns `true` if array is sorted, otherwise returns `false`.
	///
	template<typename Ty>
	bool isSorted(const Ty* _data, uint32_t _num, const ComparisonFn _fn = compareAscending<Ty>);

	/// Returns an index to the first element greater or equal than the `_key` value.
	///
	/// @param _key Pointer to the key to search for.
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _stride Element stride in bytes.
	/// @param _fn Comparison function.
	///
	/// @remarks Array must be sorted!
	///
	/// @returns Returns an index to the first element greater or equal than the `_key` value.
	///
	uint32_t lowerBound(const void* _key, const void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn);

	/// Returns an index to the first element greater or equal than the `_key` value.
	///
	/// @param _key Pointer to the key to search for.
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _fn Comparison function.
	///
	/// @remarks Array must be sorted!
	///
	/// @returns Returns an index to the first element greater or equal than the `_key` value.
	///
	template<typename Ty>
	uint32_t lowerBound(const Ty& _key, const Ty* _data, uint32_t _num, const ComparisonFn _fn = compareAscending<Ty>);

	/// Returns an index to the first element greater or equal than the `_key` value.
	///
	/// @param _key Pointer to the key to search for.
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _stride Element stride in bytes.
	/// @param _fn Comparison function.
	///
	/// @remarks Array must be sorted!
	///
	/// @returns Returns an index to the first element greater or equal than the `_key` value.
	///
	template<typename Ty>
	uint32_t lowerBound(const Ty& _key, const void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn = compareAscending<Ty>);

	/// Returns an index to the first element greater than the `_key` value.
	///
	/// @param _key Pointer to the key to search for.
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _stride Element stride in bytes.
	/// @param _fn Comparison function.
	///
	/// @remarks Array must be sorted!
	///
	/// @returns Returns an index to the first element greater than the `_key` value.
	///
	uint32_t upperBound(const void* _key, const void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn);

	/// Returns an index to the first element greater than the `_key` value.
	///
	/// @param _key Pointer to the key to search for.
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _fn Comparison function.
	///
	/// @remarks Array must be sorted!
	///
	/// @returns Returns an index to the first element greater than the `_key` value.
	///
	template<typename Ty>
	uint32_t upperBound(const Ty& _key, const Ty* _data, uint32_t _num, const ComparisonFn _fn = compareAscending<Ty>);

	/// Returns an index to the first element greater than the `_key` value.
	///
	/// @param _key Pointer to the key to search for.
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _stride Element stride in bytes.
	/// @param _fn Comparison function.
	///
	/// @remarks Array must be sorted!
	///
	/// @returns Returns an index to the first element greater than the `_key` value.
	///
	template<typename Ty>
	uint32_t upperBound(const Ty& _key, const void* _data, uint32_t _num, uint32_t _stride, const ComparisonFn _fn = compareAscending<Ty>);

	/// Performs binary search of a sorted array.
	///
	/// @param _key Pointer to the key to search for.
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _stride Element stride in bytes.
	/// @param _fn Comparison function.
	///
	/// @remarks Array must be sorted!
	///
	/// @returns Returns positive value index of element if found, or negative number that is bitwise
	///   complement (~) of the index of the next element that's larger than item.
	///
	int32_t binarySearch(
		  const void* _key
		, const void* _data
		, uint32_t _num
		, uint32_t _stride
		, const ComparisonFn _fn
		);

	/// Performs binary search of a sorted array.
	///
	/// @param _key Pointer to the key to search for.
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _fn Comparison function.
	///
	/// @remarks Array must be sorted!
	///
	/// @returns Returns positive value index of element if found, or negative number that is bitwise
	///   complement (~) of the index of the next element that's larger than item.
	///
	template<typename Ty>
	int32_t binarySearch(const Ty& _key, const Ty* _data, uint32_t _num, const ComparisonFn _fn = compareAscending<Ty>);

	/// Performs binary search of a sorted array.
	///
	/// @param _key Pointer to the key to search for.
	/// @param _data Pointer to sorted array data.
	/// @param _num Number of elements.
	/// @param _stride Element stride in bytes.
	/// @param _fn Comparison function.
	///
	/// @remarks Array must be sorted!
	///
	/// @returns Returns positive value index of element if found, or negative number that is bitwise
	///   complement (~) of the index of the next element that's larger than item.
	///
	template<typename Ty>
	int32_t binarySearch(const Ty& _key, const void* _data, uint32_t _num, uint32_t _stride = sizeof(Ty), const ComparisonFn _fn = compareAscending<Ty>);

	///
	void radixSort(
		  uint32_t* _keys
		, uint32_t* _tempKeys
		, uint32_t _size
		);

	///
	template <typename Ty>
	void radixSort(
		  uint32_t* _keys
		, uint32_t* _tempKeys
		, Ty* _values
		, Ty* _tempValues
		, uint32_t _size
		);

	///
	void radixSort(
		  uint64_t* _keys
		, uint64_t* _tempKeys
		, uint32_t _size
		);

	///
	template <typename Ty>
	void radixSort(
		  uint64_t* _keys
		, uint64_t* _tempKeys
		, Ty* _values
		, Ty* _tempValues
		, uint32_t _size
		);

} // namespace bx

#include "inline/sort.inl"

#endif // BX_SORT_H_HEADER_GUARD
