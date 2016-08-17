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
/// @file test/gtc/gtc_matrix_transform.cpp
/// @date 2010-09-16 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

int test_perspective()
{
	int Error = 0;

	glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);

	return Error;
}

int test_pick()
{
	int Error = 0;

	glm::mat4 Pick = glm::pickMatrix(glm::vec2(1, 2), glm::vec2(3, 4), glm::ivec4(0, 0, 320, 240));

	return Error;
}

int test_tweakedInfinitePerspective()
{
	int Error = 0;

	glm::mat4 ProjectionA = glm::tweakedInfinitePerspective(45.f, 640.f/480.f, 1.0f);
	glm::mat4 ProjectionB = glm::tweakedInfinitePerspective(45.f, 640.f/480.f, 1.0f, 0.001f);


	return Error;
}

int test_translate()
{
	int Error = 0;

	glm::lowp_vec3 v(1.0);
	glm::lowp_mat4 m(0);
	glm::lowp_mat4 t = glm::translate(m, v);

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_translate();
	Error += test_tweakedInfinitePerspective();
	Error += test_pick();
	Error += test_perspective();

	return Error;
}
