#include <glm/glm.hpp>

int test_aligned()
{
	int Error = 0;

	size_t size1_aligned = sizeof(glm::detail::storage<int, 1, true>::type);
	Error += size1_aligned == 1 ? 0 : 1;
	size_t size2_aligned = sizeof(glm::detail::storage<int, 2, true>::type);
	Error += size2_aligned == 2 ? 0 : 1;
	size_t size4_aligned = sizeof(glm::detail::storage<int, 4, true>::type);
	Error += size4_aligned == 4 ? 0 : 1;
	size_t size8_aligned = sizeof(glm::detail::storage<int, 8, true>::type);
	Error += size8_aligned == 8 ? 0 : 1;
	size_t size16_aligned = sizeof(glm::detail::storage<int, 16, true>::type);
	Error += size16_aligned == 16 ? 0 : 1;
	size_t size32_aligned = sizeof(glm::detail::storage<int, 32, true>::type);
	Error += size32_aligned == 32 ? 0 : 1;
	size_t size64_aligned = sizeof(glm::detail::storage<int, 64, true>::type);
	Error += size64_aligned == 64 ? 0 : 1;

#	if GLM_HAS_ALIGNOF

	size_t align1_aligned = alignof(glm::detail::storage<int, 1, true>::type);
	Error += align1_aligned == 1 ? 0 : 1;
	size_t align2_aligned = alignof(glm::detail::storage<int, 2, true>::type);
	Error += align2_aligned == 2 ? 0 : 1;
	size_t align4_aligned = alignof(glm::detail::storage<int, 4, true>::type);
	Error += align4_aligned == 4 ? 0 : 1;
	size_t align8_aligned = alignof(glm::detail::storage<int, 8, true>::type);
	Error += align8_aligned == 8 ? 0 : 1;
	size_t align16_aligned = alignof(glm::detail::storage<int, 16, true>::type);
	Error += align16_aligned == 16 ? 0 : 1;
	size_t align32_aligned = alignof(glm::detail::storage<int, 32, true>::type);
	Error += align32_aligned == 32 ? 0 : 1;
	size_t align64_aligned = alignof(glm::detail::storage<int, 64, true>::type);
	Error += align64_aligned == 64 ? 0 : 1;

#	elif GLM_COMPILER & GLM_COMPILER_GCC

	size_t align1_aligned = __alignof__(glm::detail::storage<int, 1, true>::type);
	Error += align1_aligned == 1 ? 0 : 1;
	size_t align2_aligned = __alignof__(glm::detail::storage<int, 2, true>::type);
	Error += align2_aligned == 2 ? 0 : 1;
	size_t align4_aligned = __alignof__(glm::detail::storage<int, 4, true>::type);
	Error += align4_aligned == 4 ? 0 : 1;
	size_t align8_aligned = __alignof__(glm::detail::storage<int, 8, true>::type);
	Error += align8_aligned == 8 ? 0 : 1;
	size_t align16_aligned = __alignof__(glm::detail::storage<int, 16, true>::type);
	Error += align16_aligned == 16 ? 0 : 1;
	size_t align32_aligned = __alignof__(glm::detail::storage<int, 32, true>::type);
	Error += align32_aligned == 32 ? 0 : 1;
	size_t align64_aligned = __alignof__(glm::detail::storage<int, 64, true>::type);
	Error += align64_aligned == 64 ? 0 : 1;

#	endif //GLM_HAS_ALIGNOF

	return Error;
}

int test_unaligned()
{
	int Error = 0;

	size_t size1_unaligned = sizeof(glm::detail::storage<int, 1, false>::type);
	Error += size1_unaligned == 1 ? 0 : 1;
	size_t size2_unaligned = sizeof(glm::detail::storage<int, 2, false>::type);
	Error += size2_unaligned == 2 ? 0 : 1;
	size_t size4_unaligned = sizeof(glm::detail::storage<int, 4, false>::type);
	Error += size4_unaligned == 4 ? 0 : 1;
	size_t size8_unaligned = sizeof(glm::detail::storage<int, 8, false>::type);
	Error += size8_unaligned == 8 ? 0 : 1;
	size_t size16_unaligned = sizeof(glm::detail::storage<int, 16, false>::type);
	Error += size16_unaligned == 16 ? 0 : 1;
	size_t size32_unaligned = sizeof(glm::detail::storage<int, 32, false>::type);
	Error += size32_unaligned == 32 ? 0 : 1;
	size_t size64_unaligned = sizeof(glm::detail::storage<int, 64, false>::type);
	Error += size64_unaligned == 64 ? 0 : 1;

#	if GLM_HAS_ALIGNOF

	size_t align1_unaligned = alignof(glm::detail::storage<int, 1, false>::type);
	Error += align1_unaligned == 1 ? 0 : 1;
	size_t align2_unaligned = alignof(glm::detail::storage<int, 2, false>::type);
	Error += align2_unaligned == 1 ? 0 : 1;
	size_t align4_unaligned = alignof(glm::detail::storage<int, 4, false>::type);
	Error += align4_unaligned == 1 ? 0 : 1;
	size_t align8_unaligned = alignof(glm::detail::storage<int, 8, false>::type);
	Error += align8_unaligned == 1 ? 0 : 1;
	size_t align16_unaligned = alignof(glm::detail::storage<int, 16, false>::type);
	Error += align16_unaligned == 1 ? 0 : 1;
	size_t align32_unaligned = alignof(glm::detail::storage<int, 32, false>::type);
	Error += align32_unaligned == 1 ? 0 : 1;
	size_t align64_unaligned = alignof(glm::detail::storage<int, 64, false>::type);
	Error += align64_unaligned == 1 ? 0 : 1;

#	elif GLM_COMPILER & GLM_COMPILER_GCC

	size_t align1_unaligned = __alignof__(glm::detail::storage<int, 1, false>::type);
	Error += align1_unaligned == 1 ? 0 : 1;
	size_t align2_unaligned = __alignof__(glm::detail::storage<int, 2, false>::type);
	Error += align2_unaligned == 1 ? 0 : 1;
	size_t align4_unaligned = __alignof__(glm::detail::storage<int, 4, false>::type);
	Error += align4_unaligned == 1 ? 0 : 1;
	size_t align8_unaligned = __alignof__(glm::detail::storage<int, 8, false>::type);
	Error += align8_unaligned == 1 ? 0 : 1;
	size_t align16_unaligned = __alignof__(glm::detail::storage<int, 16, false>::type);
	Error += align16_unaligned == 1 ? 0 : 1;
	size_t align32_unaligned = __alignof__(glm::detail::storage<int, 32, false>::type);
	Error += align32_unaligned == 1 ? 0 : 1;
	size_t align64_unaligned = __alignof__(glm::detail::storage<int, 64, false>::type);
	Error += align64_unaligned == 1 ? 0 : 1;

#	endif //GLM_HAS_ALIGNOF

	return Error;
}


int main()
{
	int Error = 0;

	Error += test_aligned();
	Error += test_unaligned();

	return Error;
}
