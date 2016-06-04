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
/// @file test/gtx/gtx_fast_square_root.cpp
/// @date 2013-10-25 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/vector_relational.hpp>

int test_fastInverseSqrt()
{
	int Error(0);

	Error += glm::epsilonEqual(glm::fastInverseSqrt(1.0f), 1.0f, 0.01f) ? 0 : 1;
	Error += glm::epsilonEqual(glm::fastInverseSqrt(1.0), 1.0, 0.01) ? 0 : 1;
	Error += glm::all(glm::epsilonEqual(glm::fastInverseSqrt(glm::vec2(1.0f)), glm::vec2(1.0f), 0.01f)) ? 0 : 1;
	Error += glm::all(glm::epsilonEqual(glm::fastInverseSqrt(glm::dvec3(1.0)), glm::dvec3(1.0), 0.01)) ? 0 : 1;
	Error += glm::all(glm::epsilonEqual(glm::fastInverseSqrt(glm::dvec4(1.0)), glm::dvec4(1.0), 0.01)) ? 0 : 1;

	
	return 0;
}

int test_fastDistance()
{
	int Error(0);

	glm::mediump_f32 A = glm::fastDistance(glm::mediump_f32(0.0f), glm::mediump_f32(1.0f));
	glm::mediump_f32 B = glm::fastDistance(glm::mediump_f32vec2(0.0f), glm::mediump_f32vec2(1.0f, 0.0f));
	glm::mediump_f32 C = glm::fastDistance(glm::mediump_f32vec3(0.0f), glm::mediump_f32vec3(1.0f, 0.0f, 0.0f));
	glm::mediump_f32 D = glm::fastDistance(glm::mediump_f32vec4(0.0f), glm::mediump_f32vec4(1.0f, 0.0f, 0.0f, 0.0f));

	Error += glm::epsilonEqual(A, glm::mediump_f32(1.0f), glm::mediump_f32(0.01f)) ? 0 : 1;
	Error += glm::epsilonEqual(B, glm::mediump_f32(1.0f), glm::mediump_f32(0.01f)) ? 0 : 1;
	Error += glm::epsilonEqual(C, glm::mediump_f32(1.0f), glm::mediump_f32(0.01f)) ? 0 : 1;
	Error += glm::epsilonEqual(D, glm::mediump_f32(1.0f), glm::mediump_f32(0.01f)) ? 0 : 1;

	return Error;
}

int main()
{
	int Error(0);

	Error += test_fastInverseSqrt();
	Error += test_fastDistance();

	return Error;
}
