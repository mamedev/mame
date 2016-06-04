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
/// @file test/core/func_geometric.cpp
/// @date 2011-01-15 / 2011-09-13
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/geometric.hpp>
#include <glm/vector_relational.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/vec1.hpp>
#include <limits>

namespace length
{
	int test()
	{
		float Length1 = glm::length(glm::vec1(1));
		float Length2 = glm::length(glm::vec2(1, 0));
		float Length3 = glm::length(glm::vec3(1, 0, 0));
		float Length4 = glm::length(glm::vec4(1, 0, 0, 0));

		int Error = 0;

		Error += glm::abs(Length1 - 1.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += glm::abs(Length2 - 1.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += glm::abs(Length3 - 1.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += glm::abs(Length4 - 1.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;

		return Error;
	}
}//namespace length

namespace distance
{
	int test()
	{
		float Distance1 = glm::distance(glm::vec1(1), glm::vec1(1));
		float Distance2 = glm::distance(glm::vec2(1, 0), glm::vec2(1, 0));
		float Distance3 = glm::distance(glm::vec3(1, 0, 0), glm::vec3(1, 0, 0));
		float Distance4 = glm::distance(glm::vec4(1, 0, 0, 0), glm::vec4(1, 0, 0, 0));

		int Error = 0;

		Error += glm::abs(Distance1) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += glm::abs(Distance2) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += glm::abs(Distance3) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += glm::abs(Distance4) < std::numeric_limits<float>::epsilon() ? 0 : 1;

		return Error;
	}
}//namespace distance

namespace dot
{
	int test()
	{
		float Dot1 = glm::dot(glm::vec1(1), glm::vec1(1));
		float Dot2 = glm::dot(glm::vec2(1), glm::vec2(1));
		float Dot3 = glm::dot(glm::vec3(1), glm::vec3(1));
		float Dot4 = glm::dot(glm::vec4(1), glm::vec4(1));

		int Error = 0;

		Error += glm::abs(Dot1 - 1.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += glm::abs(Dot2 - 2.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += glm::abs(Dot3 - 3.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += glm::abs(Dot4 - 4.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;

		return Error;
	}
}//namespace dot

namespace cross
{
	int test()
	{
		glm::vec3 Cross1 = glm::cross(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));
		glm::vec3 Cross2 = glm::cross(glm::vec3(0, 1, 0), glm::vec3(1, 0, 0));

		int Error = 0;

		Error += glm::all(glm::lessThan(glm::abs(Cross1 - glm::vec3(0, 0, 1)), glm::vec3(std::numeric_limits<float>::epsilon()))) ? 0 : 1;
		Error += glm::all(glm::lessThan(glm::abs(Cross2 - glm::vec3(0, 0,-1)), glm::vec3(std::numeric_limits<float>::epsilon()))) ? 0 : 1;

		return Error;
	}
}//namespace cross

namespace normalize
{
	int test()
	{
		glm::vec3 Normalize1 = glm::normalize(glm::vec3(1, 0, 0));
		glm::vec3 Normalize2 = glm::normalize(glm::vec3(2, 0, 0));

		int Error = 0;

		Error += glm::all(glm::lessThan(glm::abs(Normalize1 - glm::vec3(1, 0, 0)), glm::vec3(std::numeric_limits<float>::epsilon()))) ? 0 : 1;
		Error += glm::all(glm::lessThan(glm::abs(Normalize2 - glm::vec3(1, 0, 0)), glm::vec3(std::numeric_limits<float>::epsilon()))) ? 0 : 1;

		return Error;
	}
}//namespace normalize

namespace faceforward
{
	int test()
	{
		int Error = 0;

		{
			glm::vec3 N(0.0f, 0.0f, 1.0f);
			glm::vec3 I(1.0f, 0.0f, 1.0f);
			glm::vec3 Nref(0.0f, 0.0f, 1.0f);
			glm::vec3 F = glm::faceforward(N, I, Nref);
		}

		return Error;
	}
}//namespace faceforward

namespace reflect
{
	int test()
	{
		int Error = 0;

		{
			glm::vec2 A(1.0f,-1.0f);
			glm::vec2 B(0.0f, 1.0f);
			glm::vec2 C = glm::reflect(A, B);
			Error += C == glm::vec2(1.0, 1.0) ? 0 : 1;
		}

		{
			glm::dvec2 A(1.0f,-1.0f);
			glm::dvec2 B(0.0f, 1.0f);
			glm::dvec2 C = glm::reflect(A, B);
			Error += C == glm::dvec2(1.0, 1.0) ? 0 : 1;
		}

		return Error;
	}
}//namespace reflect

namespace refract
{
	int test()
	{
		int Error = 0;

		{
			float A(-1.0f);
			float B(1.0f);
			float C = glm::refract(A, B, 0.5f);
			Error += C == -1.0f ? 0 : 1;
		}

		{
			glm::vec2 A(0.0f,-1.0f);
			glm::vec2 B(0.0f, 1.0f);
			glm::vec2 C = glm::refract(A, B, 0.5f);
			Error += glm::all(glm::epsilonEqual(C, glm::vec2(0.0, -1.0), 0.0001f)) ? 0 : 1;
		}

		{
			glm::dvec2 A(0.0f,-1.0f);
			glm::dvec2 B(0.0f, 1.0f);
			glm::dvec2 C = glm::refract(A, B, 0.5);
			Error += C == glm::dvec2(0.0, -1.0) ? 0 : 1;
		}

		return Error;
	}
}//namespace refract

int main()
{
	int Error(0);

	Error += length::test();
	Error += distance::test();
	Error += dot::test();
	Error += cross::test();
	Error += normalize::test();
	Error += faceforward::test();
	Error += reflect::test();
	Error += refract::test();

	return Error;
}

