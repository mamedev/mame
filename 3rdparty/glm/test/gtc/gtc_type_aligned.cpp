#define GLM_FORCE_MESSAGES
#include <glm/glm.hpp>

#if GLM_HAS_ALIGNED_TYPE
#include <glm/gtc/type_aligned.hpp>

GLM_STATIC_ASSERT(glm::detail::is_aligned<glm::aligned_lowp>::value, "aligned_lowp is not aligned");
GLM_STATIC_ASSERT(glm::detail::is_aligned<glm::aligned_mediump>::value, "aligned_mediump is not aligned");
GLM_STATIC_ASSERT(glm::detail::is_aligned<glm::aligned_highp>::value, "aligned_highp is not aligned");
GLM_STATIC_ASSERT(!glm::detail::is_aligned<glm::packed_highp>::value, "packed_highp is aligned");
GLM_STATIC_ASSERT(!glm::detail::is_aligned<glm::packed_mediump>::value, "packed_mediump is aligned");
GLM_STATIC_ASSERT(!glm::detail::is_aligned<glm::packed_lowp>::value, "packed_lowp is aligned");

struct my_vec4_packed
{
	glm::uint32 a;
	glm::vec4 b;
};
GLM_STATIC_ASSERT(sizeof(my_vec4_packed) == sizeof(glm::uint32) + sizeof(glm::vec4), "glm::vec4 packed is not correct");

struct my_vec4_aligned
{
	glm::uint32 a;
	glm::aligned_vec4 b;
};
GLM_STATIC_ASSERT(sizeof(my_vec4_aligned) == sizeof(glm::aligned_vec4) * 2, "glm::vec4 aligned is not correct");

struct my_dvec4_packed
{
	glm::uint64 a;
	glm::dvec4 b;
};
GLM_STATIC_ASSERT(sizeof(my_dvec4_packed) == sizeof(glm::uint64) + sizeof(glm::dvec4), "glm::dvec4 packed is not correct");

struct my_dvec4_aligned
{
	glm::uint64 a;
	glm::aligned_dvec4 b;
};
//GLM_STATIC_ASSERT(sizeof(my_dvec4_aligned) == sizeof(glm::aligned_dvec4) * 2, "glm::dvec4 aligned is not correct");

struct my_ivec4_packed
{
	glm::uint32 a;
	glm::ivec4 b;
};
GLM_STATIC_ASSERT(sizeof(my_ivec4_packed) == sizeof(glm::uint32) + sizeof(glm::ivec4), "glm::ivec4 packed is not correct");

struct my_ivec4_aligned
{
	glm::uint32 a;
	glm::aligned_ivec4 b;
};
GLM_STATIC_ASSERT(sizeof(my_ivec4_aligned) == sizeof(glm::aligned_ivec4) * 2, "glm::ivec4 aligned is not correct");

struct my_u8vec4_packed
{
	glm::uint32 a;
	glm::u8vec4 b;
};
GLM_STATIC_ASSERT(sizeof(my_u8vec4_packed) == sizeof(glm::uint32) + sizeof(glm::u8vec4), "glm::u8vec4 packed is not correct");

int test_copy()
{
	int Error = 0;

	{
		glm::aligned_ivec4 const a(1, 2, 3, 4);
		glm::ivec4 const u(a);

		Error += a.x == u.x ? 0 : 1;
		Error += a.y == u.y ? 0 : 1;
		Error += a.z == u.z ? 0 : 1;
		Error += a.w == u.w ? 0 : 1;
	}

	{
		my_ivec4_aligned a;
		a.b = glm::ivec4(1, 2, 3, 4);

		my_ivec4_packed u;
		u.b = a.b;

		Error += a.b.x == u.b.x ? 0 : 1;
		Error += a.b.y == u.b.y ? 0 : 1;
		Error += a.b.z == u.b.z ? 0 : 1;
		Error += a.b.w == u.b.w ? 0 : 1;
	}

	return Error;
}

int main()
{
	int Error = 0;

	my_vec4_aligned GNA;
	my_dvec4_aligned GNI;

	std::size_t A0 = sizeof(my_dvec4_aligned);
	std::size_t B0 = sizeof(my_dvec4_packed);
	std::size_t C0 = sizeof(glm::aligned_dvec4);

	std::size_t A1 = sizeof(my_vec4_aligned);
	std::size_t B1 = sizeof(my_vec4_packed);
	std::size_t C1 = sizeof(glm::aligned_vec4);

	return Error;
}

#else

int main()
{
	return 0;
}

#endif//GLM_HAS_ALIGNED_TYPE
