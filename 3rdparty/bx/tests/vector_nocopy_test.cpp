/*-
 * Copyright 2012-1015 Matthew Endsley
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
#include <string.h>
#include <stdlib.h>

struct nocopy {
	nocopy() { data = 0; }
	explicit nocopy(const char* s) { data = s; }
	~nocopy() { }
	void swap(nocopy& other) { std::swap(data, other.data); }
	void reset(const char* s) { data = s; }
	const char* release() { const char* ret = data; data = 0; return ret; }

	const char* data;

private:
	nocopy(const nocopy& other);
	nocopy& operator=(const nocopy& other);
};

static inline bool operator==(const nocopy& lhs, const char* rhs) {
	if (lhs.data == 0 && rhs == 0)
		return true;
	if (lhs.data != 0 && rhs != 0)
		return 0 == strcmp(lhs.data, rhs);
	return false;
}

static inline bool operator==(const nocopy& lhs, const nocopy& rhs) {
	if (lhs.data == 0 && rhs.data == 0)
		return true;
	if (lhs.data != 0 && rhs.data != 0)
		return 0 == strcmp(lhs.data, rhs.data);
	return false;
}

TEST(vector_nocopy_constructor) {
	typedef tinystl::vector<nocopy> vector;

	{
		vector v;
		CHECK( v.empty() );
		CHECK( v.size() == 0 );
	}
	{
		vector v(10);
		CHECK( v.size() == 10 );
		for (tinystl::vector<nocopy>::iterator it = v.begin(); it != v.end(); ++it) {
			CHECK( it->data == 0 );
		}
	}
}

TEST(vector_nocopy_pushback) {
	tinystl::vector<nocopy> v;
	v.emplace_back("42");
	v.emplace_back();
	v.back().reset("24");

	CHECK( v.size() == 2 );
	CHECK( v[0] == "42" );
	CHECK( v[1] == "24" );
}

TEST(vector_nocopy_vector) {
	tinystl::vector< tinystl::vector<nocopy> > v(10);

	tinystl::vector< tinystl::vector<nocopy> >::iterator it = v.begin(), end = v.end();
	for (; it != end; ++it) {
		CHECK( (*it).empty() );
		CHECK( (*it).size() == 0 );
		CHECK( (*it).begin() == (*it).end() );
	}
}

TEST(vector_nocopy_swap) {
	tinystl::vector<nocopy> v1;
	v1.emplace_back("12");
	v1.emplace_back("20");

	tinystl::vector<nocopy> v2;
	v2.emplace_back("54");

	v1.swap(v2);

	CHECK(v1.size() == 1);
	CHECK(v2.size() == 2);
	CHECK(v1[0] == "54");
	CHECK(v2[0] == "12");
	CHECK(v2[1] == "20");
}

TEST(vector_nocopy_popback) {
	tinystl::vector<nocopy> v;
	v.emplace_back("12");
	v.emplace_back("24");

	CHECK(v.back() == "24");

	v.pop_back();

	CHECK(v.back() == "12");
	CHECK(v.size() == 1);
}

TEST(vector_nocopy_erase) {
	tinystl::vector<nocopy> v;
	v.emplace_back("1");
	v.emplace_back("2");
	v.emplace_back("3");
	v.emplace_back("4");
	v.emplace_back("5");

	tinystl::vector<nocopy>::iterator it = v.erase(v.begin());
	CHECK(*it == "2");
	CHECK(v.size() == 4);

	it = v.erase(v.end() - 1);
	CHECK(it == v.end());
	CHECK(v.size() == 3);

	v.erase(v.begin() + 1, v.end() - 1);
	CHECK(v.size() == 2);
	CHECK(v[0] == "2");
	CHECK(v[1] == "4");
}

TEST(vector_nocopy_erase_unordered) {
	typedef tinystl::vector<nocopy> vector;
	vector v;
	v.emplace_back("1");
	v.emplace_back("2");
	v.emplace_back("3");
	v.emplace_back("4");
	v.emplace_back("5");

	const char* first = v.front().release();
	vector::iterator it = v.erase_unordered(v.begin());
	CHECK( it == v.begin() );
	CHECK( v.size() == 4 );
	CHECK( std::count(v.begin(), v.end(), first) == 0 );
	for (it = v.begin(); it != v.end(); ++it) {
		CHECK( std::count(v.begin(), v.end(), *it) == 1 );
	}

	const char* last = v.back().release();
	it = v.erase_unordered(v.end() - 1);
	CHECK( it == v.end() );
	CHECK( v.size() == 3 );
	CHECK( std::count(v.begin(), v.end(), last) == 0 );
	for (it = v.begin(); it != v.end(); ++it) {
		CHECK( std::count(v.begin(), v.end(), *it) == 1 );
	}

	first = v.begin()->data;
	last = (v.end() - 1)->data;
	v.erase_unordered(v.begin() + 1, v.end() - 1);
	CHECK( v.size() == 2 );
	CHECK( std::count(v.begin(), v.end(), first) == 1 );
	CHECK( std::count(v.begin(), v.end(), last) == 1 );
}

TEST(vector_nocopy_insert) {
	tinystl::vector<nocopy> v;
	v.emplace_back("1");
	v.emplace_back("2");
	v.emplace_back("3");
	v.emplace_back("4");
	v.emplace_back("5");

	v.emplace(v.begin(), "0");
	CHECK( v.size() == 6 );
	CHECK( v[0] == "0" );
	CHECK( v[1] == "1" );
	CHECK( v[5] == "5" );

	v.emplace(v.end(), "6");
	CHECK( v.size() == 7 );
	CHECK( v.front() == "0" );
	CHECK( v.back() == "6" );
}

TEST(vector_nocopy_iterator) {
	tinystl::vector<nocopy> v(5);
	v[0].reset("1");
	v[1].reset("2");
	v[2].reset("3");
	v[3].reset("4");
	v[4].reset("5");

	const tinystl::vector<nocopy>& cv = v;

	//CHECK(v.data() == &*v.begin());
	//CHECK(v.data() == &v[0]);
	//CHECK(v.data() + v.size() == &*v.end());
	CHECK(v.begin() == cv.begin());
	CHECK(v.end() == cv.end());
	//CHECK(v.data() == cv.data());
}
