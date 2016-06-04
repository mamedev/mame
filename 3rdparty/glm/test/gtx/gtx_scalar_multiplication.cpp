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
/// @file test/gtx/gtx_scalar_multiplication.cpp
/// @date 2014-09-22 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/glm.hpp>

#if GLM_HAS_TEMPLATE_ALIASES & !(GLM_COMPILER & GLM_COMPILER_GCC)
#include <glm/gtx/scalar_multiplication.hpp>

int main()
{
	int Error(0);
	glm::vec3 v(0.5, 3.1, -9.1);

	Error += glm::all(glm::equal(v, 1.0 * v)) ? 0 : 1;
	Error += glm::all(glm::equal(v, 1 * v)) ? 0 : 1;
	Error += glm::all(glm::equal(v, 1u * v)) ? 0 : 1;

	glm::mat3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
	glm::vec3 w = 0.5f * m * v;

	Error += glm::all(glm::equal((m*v)/2, w)) ? 0 : 1;
	Error += glm::all(glm::equal(m*(v/2), w)) ? 0 : 1;
	Error += glm::all(glm::equal((m/2)*v, w)) ? 0 : 1;
	Error += glm::all(glm::equal((0.5*m)*v, w)) ? 0 : 1;
	Error += glm::all(glm::equal(0.5*(m*v), w)) ? 0 : 1;

	return Error;
}

#else

int main()
{
	return 0;
}

#endif
