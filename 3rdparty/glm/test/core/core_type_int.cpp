///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Mathematics (glm.g-truc.net)
///
/// Copyright (c) 2005 - 2015 G-Truc Creation (www.g-truc.net)
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
/// @file test/core/core_type_int.cpp
/// @date 2008-08-31 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/glm.hpp>

int test_int_size()
{
	return
		sizeof(glm::int_t) != sizeof(glm::lowp_int) &&
		sizeof(glm::int_t) != sizeof(glm::mediump_int) && 
		sizeof(glm::int_t) != sizeof(glm::highp_int);
}

int test_uint_size()
{
	return
		sizeof(glm::uint_t) != sizeof(glm::lowp_uint) &&
		sizeof(glm::uint_t) != sizeof(glm::mediump_uint) && 
		sizeof(glm::uint_t) != sizeof(glm::highp_uint);
}

int test_int_precision()
{
	return (
		sizeof(glm::lowp_int) <= sizeof(glm::mediump_int) && 
		sizeof(glm::mediump_int) <= sizeof(glm::highp_int)) ? 0 : 1;
}

int test_uint_precision()
{
	return (
		sizeof(glm::lowp_uint) <= sizeof(glm::mediump_uint) && 
		sizeof(glm::mediump_uint) <= sizeof(glm::highp_uint)) ? 0 : 1;
}

int main()
{
	int Error = 0;

	Error += test_int_size();
	Error += test_int_precision();
	Error += test_uint_size();
	Error += test_uint_precision();

	return Error;
}
