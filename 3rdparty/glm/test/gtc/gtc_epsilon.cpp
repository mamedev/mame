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
/// @file test/gtc/gtc_epsilon.cpp
/// @date 2012-09-19 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/vector_relational.hpp>

int test_defined()
{
	glm::epsilonEqual(glm::vec2(), glm::vec2(), glm::vec2());
	glm::epsilonEqual(glm::vec3(), glm::vec3(), glm::vec3());
	glm::epsilonEqual(glm::vec4(), glm::vec4(), glm::vec4());

	glm::epsilonNotEqual(glm::vec2(), glm::vec2(), glm::vec2());
	glm::epsilonNotEqual(glm::vec3(), glm::vec3(), glm::vec3());
	glm::epsilonNotEqual(glm::vec4(), glm::vec4(), glm::vec4());
	
	glm::epsilonEqual(glm::vec2(), glm::vec2(), 0.0f);
	glm::epsilonEqual(glm::vec3(), glm::vec3(), 0.0f);
	glm::epsilonEqual(glm::vec4(), glm::vec4(), 0.0f);
	glm::epsilonEqual(glm::quat(), glm::quat(), 0.0f);

	glm::epsilonNotEqual(glm::vec2(), glm::vec2(), 0.0f);
	glm::epsilonNotEqual(glm::vec3(), glm::vec3(), 0.0f);
	glm::epsilonNotEqual(glm::vec4(), glm::vec4(), 0.0f);
	glm::epsilonNotEqual(glm::quat(), glm::quat(), 0.0f);

	return 0;
}

template <typename T>
int test_equal()
{
	int Error(0);
	
	{
		T A = glm::epsilon<T>();
		T B = glm::epsilon<T>();
		Error += glm::epsilonEqual(A, B, glm::epsilon<T>() * T(2)) ? 0 : 1;
	}

	{
		T A(0);
		T B = static_cast<T>(0) + glm::epsilon<T>();
		Error += glm::epsilonEqual(A, B, glm::epsilon<T>() * T(2)) ? 0 : 1;
	}

	{
		T A(0);
		T B = static_cast<T>(0) - glm::epsilon<T>();
		Error += glm::epsilonEqual(A, B, glm::epsilon<T>() * T(2)) ? 0 : 1;
	}

	{
		T A = static_cast<T>(0) + glm::epsilon<T>();
		T B = static_cast<T>(0);
		Error += glm::epsilonEqual(A, B, glm::epsilon<T>() * T(2)) ? 0 : 1;
	}

	{
		T A = static_cast<T>(0) - glm::epsilon<T>();
		T B = static_cast<T>(0);
		Error += glm::epsilonEqual(A, B, glm::epsilon<T>() * T(2)) ? 0 : 1;
	}

	return Error;
}

int main()
{
	int Error(0);

	Error += test_defined();
	Error += test_equal<float>();
	Error += test_equal<double>();

	return Error;
}


