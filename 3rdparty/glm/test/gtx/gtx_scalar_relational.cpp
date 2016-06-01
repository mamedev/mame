///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Mathematics (glm.g-truc.net)
///
/// Copyright (c) 2005 - 2012 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// Restrictions:
///		By making use of the Software for military purposes, you choose to make
///		a Bunny unhappy.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// @file test/gtx/gtx_scalar_relational.cpp
/// @date 2013-02-04 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/glm.hpp>
#include <glm/gtx/scalar_relational.hpp>
#include <cstdio>

int test_lessThan()
{
	int Error(0);

	Error += glm::lessThan(0, 1) ? 0 : 1;
	Error += glm::lessThan(1, 0) ? 1 : 0;
	Error += glm::lessThan(0, 0) ? 1 : 0;
	Error += glm::lessThan(1, 1) ? 1 : 0;
	Error += glm::lessThan(0.0f, 1.0f) ? 0 : 1;
	Error += glm::lessThan(1.0f, 0.0f) ? 1 : 0;
	Error += glm::lessThan(0.0f, 0.0f) ? 1 : 0;
	Error += glm::lessThan(1.0f, 1.0f) ? 1 : 0;
	Error += glm::lessThan(0.0, 1.0) ? 0 : 1;
	Error += glm::lessThan(1.0, 0.0) ? 1 : 0;
	Error += glm::lessThan(0.0, 0.0) ? 1 : 0;
	Error += glm::lessThan(1.0, 1.0) ? 1 : 0;

	return Error;
}

int test_lessThanEqual()
{
	int Error(0);

	Error += glm::lessThanEqual(0, 1) ? 0 : 1;
	Error += glm::lessThanEqual(1, 0) ? 1 : 0;
	Error += glm::lessThanEqual(0, 0) ? 0 : 1;
	Error += glm::lessThanEqual(1, 1) ? 0 : 1;
	Error += glm::lessThanEqual(0.0f, 1.0f) ? 0 : 1;
	Error += glm::lessThanEqual(1.0f, 0.0f) ? 1 : 0;
	Error += glm::lessThanEqual(0.0f, 0.0f) ? 0 : 1;
	Error += glm::lessThanEqual(1.0f, 1.0f) ? 0 : 1;
	Error += glm::lessThanEqual(0.0, 1.0) ? 0 : 1;
	Error += glm::lessThanEqual(1.0, 0.0) ? 1 : 0;
	Error += glm::lessThanEqual(0.0, 0.0) ? 0 : 1;
	Error += glm::lessThanEqual(1.0, 1.0) ? 0 : 1;

	return Error;
}

int test_greaterThan()
{
	int Error(0);

	Error += glm::greaterThan(0, 1) ? 1 : 0;
	Error += glm::greaterThan(1, 0) ? 0 : 1;
	Error += glm::greaterThan(0, 0) ? 1 : 0;
	Error += glm::greaterThan(1, 1) ? 1 : 0;
	Error += glm::greaterThan(0.0f, 1.0f) ? 1 : 0;
	Error += glm::greaterThan(1.0f, 0.0f) ? 0 : 1;
	Error += glm::greaterThan(0.0f, 0.0f) ? 1 : 0;
	Error += glm::greaterThan(1.0f, 1.0f) ? 1 : 0;
	Error += glm::greaterThan(0.0, 1.0) ? 1 : 0;
	Error += glm::greaterThan(1.0, 0.0) ? 0 : 1;
	Error += glm::greaterThan(0.0, 0.0) ? 1 : 0;
	Error += glm::greaterThan(1.0, 1.0) ? 1 : 0;

	return Error;
}

int test_greaterThanEqual()
{
	int Error(0);

	Error += glm::greaterThanEqual(0, 1) ? 1 : 0;
	Error += glm::greaterThanEqual(1, 0) ? 0 : 1;
	Error += glm::greaterThanEqual(0, 0) ? 0 : 1;
	Error += glm::greaterThanEqual(1, 1) ? 0 : 1;
	Error += glm::greaterThanEqual(0.0f, 1.0f) ? 1 : 0;
	Error += glm::greaterThanEqual(1.0f, 0.0f) ? 0 : 1;
	Error += glm::greaterThanEqual(0.0f, 0.0f) ? 0 : 1;
	Error += glm::greaterThanEqual(1.0f, 1.0f) ? 0 : 1;
	Error += glm::greaterThanEqual(0.0, 1.0) ? 1 : 0;
	Error += glm::greaterThanEqual(1.0, 0.0) ? 0 : 1;
	Error += glm::greaterThanEqual(0.0, 0.0) ? 0 : 1;
	Error += glm::greaterThanEqual(1.0, 1.0) ? 0 : 1;

	return Error;
}

int test_equal()
{
	int Error(0);

	Error += glm::equal(0, 1) ? 1 : 0;
	Error += glm::equal(1, 0) ? 1 : 0;
	Error += glm::equal(0, 0) ? 0 : 1;
	Error += glm::equal(1, 1) ? 0 : 1;
	Error += glm::equal(0.0f, 1.0f) ? 1 : 0;
	Error += glm::equal(1.0f, 0.0f) ? 1 : 0;
	Error += glm::equal(0.0f, 0.0f) ? 0 : 1;
	Error += glm::equal(1.0f, 1.0f) ? 0 : 1;
	Error += glm::equal(0.0, 1.0) ? 1 : 0;
	Error += glm::equal(1.0, 0.0) ? 1 : 0;
	Error += glm::equal(0.0, 0.0) ? 0 : 1;
	Error += glm::equal(1.0, 1.0) ? 0 : 1;

	return Error;
}

int test_notEqual()
{
	int Error(0);

	Error += glm::notEqual(0, 1) ? 0 : 1;
	Error += glm::notEqual(1, 0) ? 0 : 1;
	Error += glm::notEqual(0, 0) ? 1 : 0;
	Error += glm::notEqual(1, 1) ? 1 : 0;
	Error += glm::notEqual(0.0f, 1.0f) ? 0 : 1;
	Error += glm::notEqual(1.0f, 0.0f) ? 0 : 1;
	Error += glm::notEqual(0.0f, 0.0f) ? 1 : 0;
	Error += glm::notEqual(1.0f, 1.0f) ? 1 : 0;
	Error += glm::notEqual(0.0, 1.0) ? 0 : 1;
	Error += glm::notEqual(1.0, 0.0) ? 0 : 1;
	Error += glm::notEqual(0.0, 0.0) ? 1 : 0;
	Error += glm::notEqual(1.0, 1.0) ? 1 : 0;

	return Error;
}

int test_any()
{
	int Error(0);

	Error += glm::any(true) ? 0 : 1;
	Error += glm::any(false) ? 1 : 0;

	return Error;
}

int test_all()
{
	int Error(0);

	Error += glm::all(true) ? 0 : 1;
	Error += glm::all(false) ? 1 : 0;

	return Error;
}

int test_not()
{
	int Error(0);

	Error += glm::not_(true) ? 1 : 0;
	Error += glm::not_(false) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_lessThan();
	Error += test_lessThanEqual();
	Error += test_greaterThan();
	Error += test_greaterThanEqual();
	Error += test_equal();
	Error += test_notEqual();
	Error += test_any();
	Error += test_all();
	Error += test_not();

	return Error;
}
