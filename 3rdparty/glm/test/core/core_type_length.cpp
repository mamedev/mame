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
/// @file test/core/core_type_length.cpp
/// @date 2011-05-25 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/glm.hpp>

int test_length_mat_non_squared()
{
	int Error = 0;

	Error += glm::mat2x3().length() == 2 ? 0 : 1;
	Error += glm::mat2x4().length() == 2 ? 0 : 1;
	Error += glm::mat3x2().length() == 3 ? 0 : 1;
	Error += glm::mat3x4().length() == 3 ? 0 : 1;
	Error += glm::mat4x2().length() == 4 ? 0 : 1;
	Error += glm::mat4x3().length() == 4 ? 0 : 1;
	
	Error += glm::dmat2x3().length() == 2 ? 0 : 1;
	Error += glm::dmat2x4().length() == 2 ? 0 : 1;
	Error += glm::dmat3x2().length() == 3 ? 0 : 1;
	Error += glm::dmat3x4().length() == 3 ? 0 : 1;
	Error += glm::dmat4x2().length() == 4 ? 0 : 1;
	Error += glm::dmat4x3().length() == 4 ? 0 : 1;
	
	return Error;
}

int test_length_mat()
{
	int Error = 0;
	
	Error += glm::mat2().length() == 2 ? 0 : 1;
	Error += glm::mat3().length() == 3 ? 0 : 1;
	Error += glm::mat4().length() == 4 ? 0 : 1;
	Error += glm::mat2x2().length() == 2 ? 0 : 1;
	Error += glm::mat3x3().length() == 3 ? 0 : 1;
	Error += glm::mat4x4().length() == 4 ? 0 : 1;
	
	Error += glm::dmat2().length() == 2 ? 0 : 1;
	Error += glm::dmat3().length() == 3 ? 0 : 1;
	Error += glm::dmat4().length() == 4 ? 0 : 1;
	Error += glm::dmat2x2().length() == 2 ? 0 : 1;
	Error += glm::dmat3x3().length() == 3 ? 0 : 1;
	Error += glm::dmat4x4().length() == 4 ? 0 : 1;
	
	return Error;
}

int test_length_vec()
{

	int Error = 0;
	
	Error += glm::vec2().length() == 2 ? 0 : 1;
	Error += glm::vec3().length() == 3 ? 0 : 1;
	Error += glm::vec4().length() == 4 ? 0 : 1;

	Error += glm::ivec2().length() == 2 ? 0 : 1;
	Error += glm::ivec3().length() == 3 ? 0 : 1;
	Error += glm::ivec4().length() == 4 ? 0 : 1;

	Error += glm::uvec2().length() == 2 ? 0 : 1;
	Error += glm::uvec3().length() == 3 ? 0 : 1;
	Error += glm::uvec4().length() == 4 ? 0 : 1;	
	
	Error += glm::dvec2().length() == 2 ? 0 : 1;
	Error += glm::dvec3().length() == 3 ? 0 : 1;
	Error += glm::dvec4().length() == 4 ? 0 : 1;
	
	return Error;
}

int main()
{
	int Error = 0;
	
	Error += test_length_vec();
	Error += test_length_mat();
	Error += test_length_mat_non_squared();
	
	return Error;
}

