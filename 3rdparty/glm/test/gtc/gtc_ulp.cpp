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
/// @file test/gtc/gtc_ulp.cpp
/// @date 2011-04-26 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/gtc/ulp.hpp>
#include <limits>

int test_ulp_float_dist()
{
	int Error = 0;

	float A = 1.0f;

	float B = glm::next_float(A);
	Error += A != B ? 0 : 1;
	float C = glm::prev_float(B);
	Error += A == C ? 0 : 1;

	int D = glm::float_distance(A, B);
	Error += D == 1 ? 0 : 1;
	int E = glm::float_distance(A, C);
	Error += E == 0 ? 0 : 1;

	return Error;
}

int test_ulp_float_step()
{
	int Error = 0;

	float A = 1.0f;

	for(int i = 10; i < 1000; i *= 10)
	{
		float B = glm::next_float(A, i);
		Error += A != B ? 0 : 1;
		float C = glm::prev_float(B, i);
		Error += A == C ? 0 : 1;

		int D = glm::float_distance(A, B);
		Error += D == i ? 0 : 1;
		int E = glm::float_distance(A, C);
		Error += E == 0 ? 0 : 1;
	}

	return Error;
}

int test_ulp_double_dist()
{
	int Error = 0;

	double A = 1.0;

	double B = glm::next_float(A);
	Error += A != B ? 0 : 1;
	double C = glm::prev_float(B);
	Error += A == C ? 0 : 1;

	int D = glm::float_distance(A, B);
	Error += D == 1 ? 0 : 1;
	int E = glm::float_distance(A, C);
	Error += E == 0 ? 0 : 1;

	return Error;
}

int test_ulp_double_step()
{
	int Error = 0;

	double A = 1.0;

	for(int i = 10; i < 1000; i *= 10)
	{
		double B = glm::next_float(A, i);
		Error += A != B ? 0 : 1;
		double C = glm::prev_float(B, i);
		Error += A == C ? 0 : 1;

		int D = glm::float_distance(A, B);
		Error += D == i ? 0 : 1;
		int E = glm::float_distance(A, C);
		Error += E == 0 ? 0 : 1;
	}

	return Error;
}

int main()
{
	int Error = 0;
	Error += test_ulp_float_dist();
	Error += test_ulp_float_step();
	Error += test_ulp_double_dist();
	Error += test_ulp_double_step();
	return Error;
}


