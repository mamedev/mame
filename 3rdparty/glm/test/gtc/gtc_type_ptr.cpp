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
/// @file test/gtc/gtc_type_ptr.cpp
/// @date 2010-09-16 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/gtc/type_ptr.hpp>

int test_value_ptr_vec()
{
	int Error = 0;

	{
		glm::vec2 v(1.0);
		float * p = glm::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		glm::vec3 v(1.0);
		float * p = glm::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		glm::vec4 v(1.0);
		float * p = glm::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}

	{
		glm::dvec2 v(1.0);
		double * p = glm::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		glm::dvec3 v(1.0);
		double * p = glm::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		glm::dvec4 v(1.0);
		double * p = glm::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}

	return Error;
}

int test_value_ptr_vec_const()
{
	int Error = 0;

	{
		glm::vec2 const v(1.0);
		float const * p = glm::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		glm::vec3 const v(1.0);
		float const * p = glm::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		glm::vec4 const v(1.0);
		float const * p = glm::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}

	{
		glm::dvec2 const v(1.0);
		double const * p = glm::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		glm::dvec3 const v(1.0);
		double const * p = glm::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		glm::dvec4 const v(1.0);
		double const * p = glm::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}

	return Error;
}

int test_value_ptr_mat()
{
	int Error = 0;

	{
		glm::mat2x2 m(1.0);
		float * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat2x3 m(1.0);
		float * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat2x4 m(1.0);
		float * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat3x2 m(1.0);
		float * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat3x3 m(1.0);
		float * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat3x4 m(1.0);
		float * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat4x2 m(1.0);
		float * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat4x3 m(1.0);
		float * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat4x4 m(1.0);
		float * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}

	return Error;
}

int test_value_ptr_mat_const()
{
	int Error = 0;

	{
		glm::mat2x2 const m(1.0);
		float const * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat2x3 const m(1.0);
		float const * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat2x4 const m(1.0);
		float const * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat3x2 const m(1.0);
		float const * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat3x3 const m(1.0);
		float const * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat3x4 const m(1.0);
		float const * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat4x2 const m(1.0);
		float const * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat4x3 const m(1.0);
		float const * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		glm::mat4x4 const m(1.0);
		float const * p = glm::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}

	return Error;
}

int test_make_pointer_mat()
{
	int Error = 0;

	float ArrayA[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	double ArrayB[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

	glm::mat2x2 Mat2x2A = glm::make_mat2x2(ArrayA);
	glm::mat2x3 Mat2x3A = glm::make_mat2x3(ArrayA);
	glm::mat2x4 Mat2x4A = glm::make_mat2x4(ArrayA);
	glm::mat3x2 Mat3x2A = glm::make_mat3x2(ArrayA);
	glm::mat3x3 Mat3x3A = glm::make_mat3x3(ArrayA);
	glm::mat3x4 Mat3x4A = glm::make_mat3x4(ArrayA);
	glm::mat4x2 Mat4x2A = glm::make_mat4x2(ArrayA);
	glm::mat4x3 Mat4x3A = glm::make_mat4x3(ArrayA);
	glm::mat4x4 Mat4x4A = glm::make_mat4x4(ArrayA);

	glm::dmat2x2 Mat2x2B = glm::make_mat2x2(ArrayB);
	glm::dmat2x3 Mat2x3B = glm::make_mat2x3(ArrayB);
	glm::dmat2x4 Mat2x4B = glm::make_mat2x4(ArrayB);
	glm::dmat3x2 Mat3x2B = glm::make_mat3x2(ArrayB);
	glm::dmat3x3 Mat3x3B = glm::make_mat3x3(ArrayB);
	glm::dmat3x4 Mat3x4B = glm::make_mat3x4(ArrayB);
	glm::dmat4x2 Mat4x2B = glm::make_mat4x2(ArrayB);
	glm::dmat4x3 Mat4x3B = glm::make_mat4x3(ArrayB);
	glm::dmat4x4 Mat4x4B = glm::make_mat4x4(ArrayB);

	return Error;
}

int test_make_pointer_vec()
{
	int Error = 0;

	float ArrayA[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	int ArrayB[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	bool ArrayC[] = {true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false};

	glm::vec2 Vec2A = glm::make_vec2(ArrayA);
	glm::vec3 Vec3A = glm::make_vec3(ArrayA);
	glm::vec4 Vec4A = glm::make_vec4(ArrayA);

	glm::ivec2 Vec2B = glm::make_vec2(ArrayB);
	glm::ivec3 Vec3B = glm::make_vec3(ArrayB);
	glm::ivec4 Vec4B = glm::make_vec4(ArrayB);

	glm::bvec2 Vec2C = glm::make_vec2(ArrayC);
	glm::bvec3 Vec3C = glm::make_vec3(ArrayC);
	glm::bvec4 Vec4C = glm::make_vec4(ArrayC);

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_make_pointer_vec();
	Error += test_make_pointer_mat();
	Error += test_value_ptr_vec();
	Error += test_value_ptr_vec_const();
	Error += test_value_ptr_mat();
	Error += test_value_ptr_mat_const();

	return Error;
}
