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
/// @file test/gtx/gtx_vector_angle.cpp
/// @date 2011-05-15 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/gtc/constants.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <limits>

int test_angle()
{
	int Error = 0;
	
	float AngleA = glm::angle(glm::vec2(1, 0), glm::normalize(glm::vec2(1, 1)));
	Error += glm::epsilonEqual(AngleA, glm::pi<float>() * 0.25f, 0.01f) ? 0 : 1;
	float AngleB = glm::angle(glm::vec3(1, 0, 0), glm::normalize(glm::vec3(1, 1, 0)));
	Error += glm::epsilonEqual(AngleB, glm::pi<float>() * 0.25f, 0.01f) ? 0 : 1;
	float AngleC = glm::angle(glm::vec4(1, 0, 0, 0), glm::normalize(glm::vec4(1, 1, 0, 0)));
	Error += glm::epsilonEqual(AngleC, glm::pi<float>() * 0.25f, 0.01f) ? 0 : 1;

	return Error;
}

int test_orientedAngle_vec2()
{
	int Error = 0;
	
	float AngleA = glm::orientedAngle(glm::vec2(1, 0), glm::normalize(glm::vec2(1, 1)));
	Error += AngleA == glm::pi<float>() * 0.25f ? 0 : 1;
	float AngleB = glm::orientedAngle(glm::vec2(0, 1), glm::normalize(glm::vec2(1, 1)));
	Error += AngleB == -glm::pi<float>() * 0.25f ? 0 : 1;
	float AngleC = glm::orientedAngle(glm::normalize(glm::vec2(1, 1)), glm::vec2(0, 1));
	Error += AngleC == glm::pi<float>() * 0.25f ? 0 : 1;

	return Error;
}

int test_orientedAngle_vec3()
{
	int Error = 0;
	
	float AngleA = glm::orientedAngle(glm::vec3(1, 0, 0), glm::normalize(glm::vec3(1, 1, 0)), glm::vec3(0, 0, 1));
	Error += AngleA == glm::pi<float>() * 0.25f ? 0 : 1;
	float AngleB = glm::orientedAngle(glm::vec3(0, 1, 0), glm::normalize(glm::vec3(1, 1, 0)), glm::vec3(0, 0, 1));
	Error += AngleB == -glm::pi<float>() * 0.25f ? 0 : 1;
	float AngleC = glm::orientedAngle(glm::normalize(glm::vec3(1, 1, 0)), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
	Error += AngleC == glm::pi<float>() * 0.25f ? 0 : 1;

	return Error;
}

int main()
{
	int Error(0);
	
	Error += test_angle();
	Error += test_orientedAngle_vec2();
	Error += test_orientedAngle_vec3();

	return Error;
}


