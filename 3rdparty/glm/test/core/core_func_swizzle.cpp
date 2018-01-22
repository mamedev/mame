#define GLM_FORCE_MESSAGES
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#if !GLM_HAS_ONLY_XYZW

int test_ivec2_swizzle()
{
	int Error = 0;

	glm::ivec2 A(1, 2);
	glm::ivec2 B = A.yx();
	glm::ivec2 C = B.yx();

	Error += A != B ? 0 : 1;
	Error += A == C ? 0 : 1;

	return Error;
}

int test_ivec3_swizzle()
{
	int Error = 0;

	glm::ivec3 A(1, 2, 3);
	glm::ivec3 B = A.zyx();
	glm::ivec3 C = B.zyx();

	Error += A != B ? 0 : 1;
	Error += A == C ? 0 : 1;

	return Error;
}

int test_ivec4_swizzle()
{
	int Error = 0;

	glm::ivec4 A(1, 2, 3, 4);
	glm::ivec4 B = A.wzyx();
	glm::ivec4 C = B.wzyx();

	Error += A != B ? 0 : 1;
	Error += A == C ? 0 : 1;

	return Error;
}

int test_vec4_swizzle()
{
	int Error = 0;

	glm::vec4 A(1, 2, 3, 4);
	glm::vec4 B = A.wzyx();
	glm::vec4 C = B.wzyx();

	Error += A != B ? 0 : 1;
	Error += A == C ? 0 : 1;

	float f = glm::dot(C.wzyx(), C.xyzw());
	Error += glm::abs(f - 20.f) < 0.01f ? 0 : 1;

	return Error;
}
#endif//!GLM_HAS_ONLY_XYZW

int main()
{
	int Error = 0;

#	if !GLM_HAS_ONLY_XYZW
		Error += test_ivec2_swizzle();
		Error += test_ivec3_swizzle();
		Error += test_ivec4_swizzle();

		Error += test_vec4_swizzle();
#	endif//!GLM_HAS_ONLY_XYZW

	return Error;
}



