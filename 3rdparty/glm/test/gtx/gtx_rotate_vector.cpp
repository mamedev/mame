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
/// @file test/gtx/gtx_rotate_vector.cpp
/// @date 2011-05-16 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>

int test_rotate()
{
	int Error = 0;

	glm::vec2 A = glm::rotate(glm::vec2(1, 0), glm::pi<float>() * 0.5f);
	glm::vec3 B = glm::rotate(glm::vec3(1, 0, 0), glm::pi<float>() * 0.5f, glm::vec3(0, 0, 1));
	glm::vec4 C = glm::rotate(glm::vec4(1, 0, 0, 1), glm::pi<float>() * 0.5f, glm::vec3(0, 0, 1));
	glm::vec3 D = glm::rotateX(glm::vec3(1, 0, 0), glm::pi<float>() * 0.5f);
	glm::vec4 E = glm::rotateX(glm::vec4(1, 0, 0, 1), glm::pi<float>() * 0.5f);
	glm::vec3 F = glm::rotateY(glm::vec3(1, 0, 0), glm::pi<float>() * 0.5f);
	glm::vec4 G = glm::rotateY(glm::vec4(1, 0, 0, 1), glm::pi<float>() * 0.5f);
	glm::vec3 H = glm::rotateZ(glm::vec3(1, 0, 0), glm::pi<float>() * 0.5f);
	glm::vec4 I = glm::rotateZ(glm::vec4(1, 0, 0,1 ), glm::pi<float>() * 0.5f);
	glm::mat4 O = glm::orientation(glm::normalize(glm::vec3(1)), glm::vec3(0, 0, 1));

	return Error;
}

int test_rotateX()
{
	int Error = 0;

	glm::vec3 D = glm::rotateX(glm::vec3(1, 0, 0), glm::pi<float>() * 0.5f);
	glm::vec4 E = glm::rotateX(glm::vec4(1, 0, 0, 1), glm::pi<float>() * 0.5f);

	return Error;
}

int test_rotateY()
{
	int Error = 0;

	glm::vec3 F = glm::rotateY(glm::vec3(1, 0, 0), glm::pi<float>() * 0.5f);
	glm::vec4 G = glm::rotateY(glm::vec4(1, 0, 0, 1), glm::pi<float>() * 0.5f);

	return Error;
}


int test_rotateZ()
{
	int Error = 0;

	glm::vec3 H = glm::rotateZ(glm::vec3(1, 0, 0), glm::pi<float>() * 0.5f);
	glm::vec4 I = glm::rotateZ(glm::vec4(1, 0, 0,1 ), glm::pi<float>() * 0.5f);

	return Error;
}

int test_orientation()
{
	int Error = 0;

	glm::mat4 O = glm::orientation(glm::normalize(glm::vec3(1)), glm::vec3(0, 0, 1));

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_rotate();
	Error += test_rotateX();
	Error += test_rotateY();
	Error += test_rotateZ();
	Error += test_orientation();

	return Error;
}


