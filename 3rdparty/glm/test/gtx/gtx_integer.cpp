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
/// @file test/gtx/gtx_integer.cpp
/// @date 2011-10-11 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/exponential.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/integer.hpp>
#include <cstdio>
/*
int test_floor_log2()
{
	int Error = 0;

	for(std::size_t i = 1; i < 1000000; ++i)
	{
		glm::uint A = glm::floor_log2(glm::uint(i));
		glm::uint B = glm::uint(glm::floor(glm::log2(double(i)))); // Will fail with float, lack of accuracy

		Error += A == B ? 0 : 1;
		assert(!Error);
	}

	return Error;
}
*/
int test_log2()
{
	int Error = 0;

	for(std::size_t i = 1; i < 24; ++i)
	{
		glm::uint A = glm::log2(glm::uint(1 << i));
		glm::uint B = glm::uint(glm::log2(double(1 << i)));

		//Error += glm::equalEpsilon(double(A), B, 1.0) ? 0 : 1;
		Error += glm::abs(double(A) - B) <= 24 ? 0 : 1;
		assert(!Error);

		printf("Log2(%d) error A=%d, B=%d\n", 1 << i, A, B);
	}

	printf("log2 error=%d\n", Error);

	return Error;
}

int test_nlz()
{
	int Error = 0;

	for(glm::uint i = 1; i < glm::uint(33); ++i)
		Error += glm::nlz(i) == glm::uint(31u) - glm::findMSB(i) ? 0 : 1;
		//printf("%d, %d\n", glm::nlz(i), 31u - glm::findMSB(i));

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_nlz();
//	Error += test_floor_log2();
	Error += test_log2();

	return Error;
}

