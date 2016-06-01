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
/// @file test/core/core_setup_precision.cpp
/// @date 2011-05-31 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#define GLM_FORCE_INLINE
#define GLM_PRECISION_HIGHP_FLOAT
#include <glm/glm.hpp>
#include <glm/ext.hpp>

static int test_mat()
{
	int Error = 0;

	Error += sizeof(glm::mat2) == sizeof(glm::highp_mat2) ? 0 : 1;
	Error += sizeof(glm::mat3) == sizeof(glm::highp_mat3) ? 0 : 1;
	Error += sizeof(glm::mat4) == sizeof(glm::highp_mat4) ? 0 : 1;

	Error += sizeof(glm::mat2x2) == sizeof(glm::highp_mat2x2) ? 0 : 1;
	Error += sizeof(glm::mat2x3) == sizeof(glm::highp_mat2x3) ? 0 : 1;
	Error += sizeof(glm::mat2x4) == sizeof(glm::highp_mat2x4) ? 0 : 1;
	Error += sizeof(glm::mat3x2) == sizeof(glm::highp_mat3x2) ? 0 : 1;
	Error += sizeof(glm::mat3x3) == sizeof(glm::highp_mat3x3) ? 0 : 1;
	Error += sizeof(glm::mat3x4) == sizeof(glm::highp_mat3x4) ? 0 : 1;
	Error += sizeof(glm::mat4x2) == sizeof(glm::highp_mat4x2) ? 0 : 1;
	Error += sizeof(glm::mat4x3) == sizeof(glm::highp_mat4x3) ? 0 : 1;
	Error += sizeof(glm::mat4x4) == sizeof(glm::highp_mat4x4) ? 0 : 1;

	return Error;
}

static int test_vec()
{
	int Error = 0;

	Error += sizeof(glm::vec2) == sizeof(glm::highp_vec2) ? 0 : 1;
	Error += sizeof(glm::vec3) == sizeof(glm::highp_vec3) ? 0 : 1;
	Error += sizeof(glm::vec4) == sizeof(glm::highp_vec4) ? 0 : 1;

	return Error;
}

static int test_dvec()
{
	int Error = 0;
	
	Error += sizeof(glm::dvec2) == sizeof(glm::highp_dvec2) ? 0 : 1;
	Error += sizeof(glm::dvec3) == sizeof(glm::highp_dvec3) ? 0 : 1;
	Error += sizeof(glm::dvec4) == sizeof(glm::highp_dvec4) ? 0 : 1;
	
	return Error;
}

int main()
{
	int Error = 0;

	Error += test_mat();
	Error += test_vec();
	Error += test_dvec();
	
	return Error;
}
