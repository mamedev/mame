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
/// @file test/core/func_exponential.cpp
/// @date 2011-01-15 / 2011-09-13
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/gtc/ulp.hpp>
#include <glm/gtc/vec1.hpp>

int test_pow()
{
	int Error(0);

	float A = glm::pow(10.f, 10.f);
	glm::vec1 B = glm::pow(glm::vec1(10.f), glm::vec1(10.f));
	glm::vec2 C = glm::pow(glm::vec2(10.f), glm::vec2(10.f));
	glm::vec3 D = glm::pow(glm::vec3(10.f), glm::vec3(10.f));
	glm::vec4 E = glm::pow(glm::vec4(10.f), glm::vec4(10.f));

	return Error;
}

int test_exp()
{
	int Error(0);

	float A = glm::exp(10.f);
	glm::vec1 B = glm::exp(glm::vec1(10.f));
	glm::vec2 C = glm::exp(glm::vec2(10.f));
	glm::vec3 D = glm::exp(glm::vec3(10.f));
	glm::vec4 E = glm::exp(glm::vec4(10.f));

	return Error;
}

int test_log()
{
	int Error(0);

	float A = glm::log(10.f);
	glm::vec1 B = glm::log(glm::vec1(10.f));
	glm::vec2 C = glm::log(glm::vec2(10.f));
	glm::vec3 D = glm::log(glm::vec3(10.f));
	glm::vec4 E = glm::log(glm::vec4(10.f));

	return Error;
}

int test_exp2()
{
	int Error(0);

	float A = glm::exp2(10.f);
	glm::vec1 B = glm::exp2(glm::vec1(10.f));
	glm::vec2 C = glm::exp2(glm::vec2(10.f));
	glm::vec3 D = glm::exp2(glm::vec3(10.f));
	glm::vec4 E = glm::exp2(glm::vec4(10.f));

	return Error;
}

int test_log2()
{
	int Error(0);

	float A = glm::log2(10.f);
	glm::vec1 B = glm::log2(glm::vec1(10.f));
	glm::vec2 C = glm::log2(glm::vec2(10.f));
	glm::vec3 D = glm::log2(glm::vec3(10.f));
	glm::vec4 E = glm::log2(glm::vec4(10.f));

	return Error;
}

int test_sqrt()
{
	int Error(0);

	float A = glm::sqrt(10.f);
	glm::vec1 B = glm::sqrt(glm::vec1(10.f));
	glm::vec2 C = glm::sqrt(glm::vec2(10.f));
	glm::vec3 D = glm::sqrt(glm::vec3(10.f));
	glm::vec4 E = glm::sqrt(glm::vec4(10.f));

	return Error;
}

int test_inversesqrt()
{
	int Error(0);

	glm::uint ulp(0);
	float diff(0.0f);

	for(float f = 0.001f; f < 10.f; f *= 1.001f)
	{
		glm::lowp_fvec1 u(f);
		glm::lowp_fvec1 lowp_v = glm::inversesqrt(u);
		float defaultp_v = glm::inversesqrt(f);

		ulp = glm::max(glm::float_distance(lowp_v.x, defaultp_v), ulp);
		diff = glm::abs(lowp_v.x - defaultp_v);
	}

	return Error;
}

int main()
{
	int Error(0);

	Error += test_pow();
	Error += test_exp();
	Error += test_log();
	Error += test_exp2();
	Error += test_log2();
	Error += test_sqrt();
	Error += test_inversesqrt();

	return Error;
}

