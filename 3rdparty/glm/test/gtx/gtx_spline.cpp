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
/// @file test/gtx/gtx_spline.cpp
/// @date 2013-10-25 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/spline.hpp>

namespace catmullRom
{
	int test()
	{
		int Error(0);

		glm::vec2 Result2 = glm::catmullRom(
			glm::vec2(0.0f, 0.0f),
			glm::vec2(1.0f, 0.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(0.0f, 1.0f), 0.5f);

		glm::vec3 Result3 = glm::catmullRom(
			glm::vec3(0.0f, 0.0f, 0.0f), 
			glm::vec3(1.0f, 0.0f, 0.0f), 
			glm::vec3(1.0f, 1.0f, 0.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f), 0.5f);

		glm::vec4 Result4 = glm::catmullRom(
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 
			glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 
			glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 
			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.5f);

		return Error;
	}
}//catmullRom

namespace hermite
{
	int test()
	{
		int Error(0);

		glm::vec2 Result2 = glm::hermite(
			glm::vec2(0.0f, 0.0f),
			glm::vec2(1.0f, 0.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(0.0f, 1.0f), 0.5f);

		glm::vec3 Result3 = glm::hermite(
			glm::vec3(0.0f, 0.0f, 0.0f), 
			glm::vec3(1.0f, 0.0f, 0.0f), 
			glm::vec3(1.0f, 1.0f, 0.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f), 0.5f);

		glm::vec4 Result4 = glm::hermite(
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 
			glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 
			glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 
			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.5f);

		return Error;
	}
}//catmullRom

namespace cubic
{
	int test()
	{
		int Error(0);

		glm::vec2 Result2 = glm::cubic(
			glm::vec2(0.0f, 0.0f),
			glm::vec2(1.0f, 0.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(0.0f, 1.0f), 0.5f);

		glm::vec3 Result3 = glm::cubic(
			glm::vec3(0.0f, 0.0f, 0.0f), 
			glm::vec3(1.0f, 0.0f, 0.0f), 
			glm::vec3(1.0f, 1.0f, 0.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f), 0.5f);

		glm::vec4 Result = glm::cubic(
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 
			glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 
			glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 
			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.5f);

		return Error;
	}
}//catmullRom

int main()
{
	int Error(0);

	Error += catmullRom::test();
	Error += hermite::test();
	Error += cubic::test();

	return Error;
}
