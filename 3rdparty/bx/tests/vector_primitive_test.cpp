/*-
 * Copyright 2012 Matthew Endsley
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "test.h"

#include <tinystl/allocator.h>
#include <tinystl/vector.h>

#include <algorithm>

TEST(vector_constructor) {
	typedef tinystl::vector<int> vector;

	{
		vector v;
		CHECK( v.empty() );
		CHECK( v.size() == 0 );
	}
	{
		const int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		vector v(array, array + 10);

		CHECK( v.size() == 10 );
		CHECK( std::equal(v.begin(), v.end(), array) );
	}
	{
		const int value = 127;
		const size_t count = 24;
		vector v(count, value);

		CHECK( v.size() == count );

		vector::iterator it = v.begin(), end = v.end();
		for (; it != end; ++it)
			CHECK(*it == value);
	}
	{
		const size_t count = 24;
		vector v(count);

		CHECK(v.size() == count);
		vector::iterator it = v.begin(), end = v.end();
		for (; it != end; ++it)
			CHECK(*it == 0);
	}
	{
		const int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		vector other(array, array + 10);
		vector v = other;

		CHECK( v.size() == other.size() );
		CHECK( std::equal(v.begin(), v.end(), other.begin()) );
	}
}

TEST(vector_assignment) {
	typedef tinystl::vector<int> vector;

	{
		const int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		vector other(array, array + 10);

		vector v;
		v = other;

		CHECK( v.size() == 10 );
		CHECK( std::equal(v.begin(), v.end(), array) );
		CHECK( other.size() == 10 );
		CHECK( std::equal(v.begin(), v.end(), other.begin()) );
	}
}

TEST(vector_pushback) {
	tinystl::vector<int> v;
	v.push_back(42);

	CHECK(v.size() == 1);
	CHECK(v[0] == 42);
}

TEST(vector_vector) {
	tinystl::vector< tinystl::vector<int> > v(10, tinystl::vector<int>());

	tinystl::vector< tinystl::vector<int> >::iterator it = v.begin(), end = v.end();
	for (; it != end; ++it) {
		CHECK( (*it).empty() );
		CHECK( (*it).size() == 0 );
		CHECK( (*it).begin() == (*it).end() );
	}
}

TEST(vector_swap) {
	tinystl::vector<int> v1;
	v1.push_back(12);
	v1.push_back(20);

	tinystl::vector<int> v2;
	v2.push_back(54);

	v1.swap(v2);

	CHECK(v1.size() == 1);
	CHECK(v2.size() == 2);
	CHECK(v1[0] == 54);
	CHECK(v2[0] == 12);
	CHECK(v2[1] == 20);
}

TEST(vector_popback) {
	tinystl::vector<int> v;
	v.push_back(12);
	v.push_back(24);

	CHECK(v.back() == 24);
	
	v.pop_back();

	CHECK(v.back() == 12);
	CHECK(v.size() == 1);
}

TEST(vector_assign) {
	tinystl::vector<int> v;

	CHECK(v.size() == 0);

	const int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	v.assign(array, array + 10);
	CHECK(v.size() == 10);
	CHECK( std::equal(v.begin(), v.end(), array) );
}

TEST(vector_erase) {
	const int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	tinystl::vector<int> v(array, array + 10);

	tinystl::vector<int>::iterator it = v.erase(v.begin());
	CHECK(*it == 2);
	CHECK(v.size() == 9);
	CHECK( std::equal(v.begin(), v.end(), array + 1) );

	it = v.erase(v.end() - 1);
	CHECK(it == v.end());
	CHECK(v.size() == 8);
	CHECK( std::equal(v.begin(), v.end(), array + 1) );

	v.erase(v.begin() + 1, v.end() - 1);
	CHECK(v.size() == 2);
	CHECK(v[0] == 2);
	CHECK(v[1] == 9);
}

TEST(vector_erase_unordered) {
	const int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	typedef tinystl::vector<int> vector;
	vector v(array, array + 10);

	int first = *(v.begin());
	vector::iterator it = v.erase_unordered(v.begin());
	CHECK(it == v.begin());
	CHECK(v.size() == 9);
	CHECK( std::count(v.begin(), v.end(), first) == 0 );
	for (it = v.begin(); it != v.end(); ++it) {
		CHECK( std::count(v.begin(), v.end(), *it) == 1 );
	}

	int last = *(v.end() - 1);
	it = v.erase_unordered(v.end() - 1);
	CHECK(it == v.end());
	CHECK(v.size() == 8);
	CHECK( std::count(v.begin(), v.end(), last) == 0 );
	for (it = v.begin(); it != v.end(); ++it) {
		CHECK( std::count(v.begin(), v.end(), *it) == 1 );
	}

	first = *(v.begin());
	last = *(v.end() - 1);
	v.erase_unordered(v.begin() + 1, v.end() - 1);
	CHECK(v.size() == 2);
	CHECK( std::count(v.begin(), v.end(), first) == 1 );
	CHECK( std::count(v.begin(), v.end(), last) == 1 );
}

TEST(vector_insert) {
	const int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	tinystl::vector<int> v(array, array + 10);

	v.insert(v.begin(), 0);
	CHECK(v.size() == 11);
	CHECK(v[0] == 0);
	CHECK( std::equal(v.begin() + 1, v.end(), array) );

	v.insert(v.end(), 11);
	CHECK(v.size() == 12);
	CHECK(v[0] == 0);
	CHECK( std::equal(array, array + 10, v.begin() + 1) );
	CHECK(v.back() == 11);

	const int array2[3] = {11, 12, 13};
	const int finalarray[] = {0, 1, 2, 3, 11, 12, 13, 4, 5, 6, 7, 8, 9, 10, 11};
	v.insert(v.begin() + 4, array2, array2 + 3);
	CHECK( v.size() == 15 );
	CHECK( std::equal(v.begin(), v.end(), finalarray) );
}

TEST(vector_iterator) {
	const int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

	tinystl::vector<int> v(array, array + 10);
	const tinystl::vector<int>& cv = v;

	CHECK(v.data() == &*v.begin());
	CHECK(v.data() == &v[0]);
	CHECK(v.data() + v.size() == &*v.end());
	CHECK(v.begin() == cv.begin());
	CHECK(v.end() == cv.end());
	CHECK(v.data() == cv.data());

	tinystl::vector<int> w = v;
	CHECK(v.begin() != w.begin());
	CHECK(v.end() != w.end());
}
