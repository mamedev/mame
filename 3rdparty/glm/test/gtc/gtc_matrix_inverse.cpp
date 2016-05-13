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
/// @file test/gtc/gtc_matrix_inverse.cpp
/// @date 2010-09-16 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/epsilon.hpp>

int test_affine()
{
	int Error = 0;

	{
		glm::mat3 const M(
			2.f, 0.f, 0.f,
			0.f, 2.f, 0.f,
			0.f, 0.f, 1.f);
		glm::mat3 const A = glm::affineInverse(M);
		glm::mat3 const I = glm::inverse(M);
		glm::mat3 const R = glm::affineInverse(A);

		for(glm::length_t i = 0; i < A.length(); ++i)
		{
			Error += glm::all(glm::epsilonEqual(M[i], R[i], 0.01f)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(A[i], I[i], 0.01f)) ? 0 : 1;
		}
	}

	{
		glm::mat4 const M(
			2.f, 0.f, 0.f, 0.f,
			0.f, 2.f, 0.f, 0.f,
			0.f, 0.f, 2.f, 0.f,
			0.f, 0.f, 0.f, 1.f);
		glm::mat4 const A = glm::affineInverse(M);
		glm::mat4 const I = glm::inverse(M);
		glm::mat4 const R = glm::affineInverse(A);

		for(glm::length_t i = 0; i < A.length(); ++i)
		{
			Error += glm::all(glm::epsilonEqual(M[i], R[i], 0.01f)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(A[i], I[i], 0.01f)) ? 0 : 1;
		}
	}

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_affine();

	return Error;
}
