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
/// @file test/core/core_func_swizzle.cpp
/// @date 2011-10-16 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#define GLM_MESSAGES
#define GLM_SWIZZLE
#include <glm/glm.hpp>

int test_ivec2_swizzle()
{
	int Error = 0;

	glm::ivec2 A(1, 2);
	glm::ivec2 B = A.yx();
	glm::ivec2 C = B.yx();

	Error += A != B ? 0 : 1;
	Error += A == C ? 0 : 1;

	return Error;
}

int test_ivec3_swizzle()
{
	int Error = 0;

	glm::ivec3 A(1, 2, 3);
	glm::ivec3 B = A.zyx();
	glm::ivec3 C = B.zyx();

	Error += A != B ? 0 : 1;
	Error += A == C ? 0 : 1;

	return Error;
}

int test_ivec4_swizzle()
{
	int Error = 0;

	glm::ivec4 A(1, 2, 3, 4);
	glm::ivec4 B = A.wzyx();
	glm::ivec4 C = B.wzyx();

	Error += A != B ? 0 : 1;
	Error += A == C ? 0 : 1;

	return Error;
}

int test_vec4_swizzle()
{
	int Error = 0;

	glm::vec4 A(1, 2, 3, 4);
	glm::vec4 B = A.wzyx();
	glm::vec4 C = B.wzyx();

	float f = glm::dot(C.wzyx(), C.xyzw());

	Error += A != B ? 0 : 1;
	Error += A == C ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_ivec2_swizzle();
	Error += test_ivec3_swizzle();
	Error += test_ivec4_swizzle();

	Error += test_vec4_swizzle();

	return Error;
}



