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
/// @file test/gtx/gtx_range.cpp
/// @date 2014-09-19 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

#if GLM_HAS_RANGE_FOR

#include <glm/gtx/range.hpp>

int testVec()
{
	int Error(0);
	glm::vec3 v(1, 2, 3);

	int count = 0;
	for(float x : v){ count++; }
	Error += count == 3 ? 0 : 1;

	for(float& x : v){ x = 0; }
	Error += glm::all(glm::equal(v, glm::vec3(0, 0, 0))) ? 0 : 1;
	return Error;
}

int testMat()
{
	int Error(0);
	glm::mat4x3 m(1);

	int count = 0;
	for(float x : m){ count++; }
	Error += count == 12 ? 0 : 1;

	for(float& x : m){ x = 0; }
	glm::vec4 v(1, 1, 1, 1);
	Error += glm::all(glm::equal(m*v, glm::vec3(0, 0, 0))) ? 0 : 1;
	return Error;
}

int main()
{
	int Error(0);
	Error += testVec();
	Error += testMat();
	return Error;
}

#else

int main()
{
	return 0;
}

#endif//GLM_HAS_RANGE_FOR
