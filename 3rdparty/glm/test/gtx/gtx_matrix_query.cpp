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
/// @file test/gtx/gtx_matrix_query.cpp
/// @date 2011-11-22 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/gtx/matrix_query.hpp>

int test_isNull()
{
	int Error(0);
	
	bool TestA = glm::isNull(glm::mat4(0), 0.00001f);
	Error += TestA ? 0 : 1;

	return Error;
}

int test_isIdentity()
{
	int Error(0);
	
	{
		bool TestA = glm::isIdentity(glm::mat2(1), 0.00001f);
		Error += TestA ? 0 : 1;
	}
	{
		bool TestA = glm::isIdentity(glm::mat3(1), 0.00001f);
		Error += TestA ? 0 : 1;
	}
	{
		bool TestA = glm::isIdentity(glm::mat4(1), 0.00001f);
		Error += TestA ? 0 : 1;
	}

	return Error;
}

int test_isNormalized()
{
	int Error(0);

	bool TestA = glm::isNormalized(glm::mat4(1), 0.00001f);
	Error += TestA ? 0 : 1;

	return Error;
}

int test_isOrthogonal()
{
	int Error(0);

	bool TestA = glm::isOrthogonal(glm::mat4(1), 0.00001f);
	Error += TestA ? 0 : 1;

	return Error;
}

int main()
{
	int Error(0);

	Error += test_isNull();
	Error += test_isIdentity();
	Error += test_isNormalized();
	Error += test_isOrthogonal();

	return Error;
}


