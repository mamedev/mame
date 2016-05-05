/// @ref core
/// @file glm/detail/func_integer_simd.inl

namespace glm{
namespace detail
{
	inline __m128i _mm_bit_interleave_si128(__m128i x)
	{
		__m128i const Mask4 = _mm_set1_epi32(0x0000FFFF);
		__m128i const Mask3 = _mm_set1_epi32(0x00FF00FF);
		__m128i const Mask2 = _mm_set1_epi32(0x0F0F0F0F);
		__m128i const Mask1 = _mm_set1_epi32(0x33333333);
		__m128i const Mask0 = _mm_set1_epi32(0x55555555);

		__m128i Reg1;
		__m128i Reg2;

		// REG1 = x;
		// REG2 = y;
		//Reg1 = _mm_unpacklo_epi64(x, y);
		Reg1 = x;

		//REG1 = ((REG1 << 16) | REG1) & glm::uint64(0x0000FFFF0000FFFF);
		//REG2 = ((REG2 << 16) | REG2) & glm::uint64(0x0000FFFF0000FFFF);
		Reg2 = _mm_slli_si128(Reg1, 2);
		Reg1 = _mm_or_si128(Reg2, Reg1);
		Reg1 = _mm_and_si128(Reg1, Mask4);

		//REG1 = ((REG1 <<  8) | REG1) & glm::uint64(0x00FF00FF00FF00FF);
		//REG2 = ((REG2 <<  8) | REG2) & glm::uint64(0x00FF00FF00FF00FF);
		Reg2 = _mm_slli_si128(Reg1, 1);
		Reg1 = _mm_or_si128(Reg2, Reg1);
		Reg1 = _mm_and_si128(Reg1, Mask3);

		//REG1 = ((REG1 <<  4) | REG1) & glm::uint64(0x0F0F0F0F0F0F0F0F);
		//REG2 = ((REG2 <<  4) | REG2) & glm::uint64(0x0F0F0F0F0F0F0F0F);
		Reg2 = _mm_slli_epi32(Reg1, 4);
		Reg1 = _mm_or_si128(Reg2, Reg1);
		Reg1 = _mm_and_si128(Reg1, Mask2);

		//REG1 = ((REG1 <<  2) | REG1) & glm::uint64(0x3333333333333333);
		//REG2 = ((REG2 <<  2) | REG2) & glm::uint64(0x3333333333333333);
		Reg2 = _mm_slli_epi32(Reg1, 2);
		Reg1 = _mm_or_si128(Reg2, Reg1);
		Reg1 = _mm_and_si128(Reg1, Mask1);

		//REG1 = ((REG1 <<  1) | REG1) & glm::uint64(0x5555555555555555);
		//REG2 = ((REG2 <<  1) | REG2) & glm::uint64(0x5555555555555555);
		Reg2 = _mm_slli_epi32(Reg1, 1);
		Reg1 = _mm_or_si128(Reg2, Reg1);
		Reg1 = _mm_and_si128(Reg1, Mask0);

		//return REG1 | (REG2 << 1);
		Reg2 = _mm_slli_epi32(Reg1, 1);
		Reg2 = _mm_srli_si128(Reg2, 8);
		Reg1 = _mm_or_si128(Reg1, Reg2);
	
		return Reg1;
	}

	inline __m128i _mm_bit_interleave_si128(__m128i x, __m128i y)
	{
		__m128i const Mask4 = _mm_set1_epi32(0x0000FFFF);
		__m128i const Mask3 = _mm_set1_epi32(0x00FF00FF);
		__m128i const Mask2 = _mm_set1_epi32(0x0F0F0F0F);
		__m128i const Mask1 = _mm_set1_epi32(0x33333333);
		__m128i const Mask0 = _mm_set1_epi32(0x55555555);

		__m128i Reg1;
		__m128i Reg2;

		// REG1 = x;
		// REG2 = y;
		Reg1 = _mm_unpacklo_epi64(x, y);

		//REG1 = ((REG1 << 16) | REG1) & glm::uint64(0x0000FFFF0000FFFF);
		//REG2 = ((REG2 << 16) | REG2) & glm::uint64(0x0000FFFF0000FFFF);
		Reg2 = _mm_slli_si128(Reg1, 2);
		Reg1 = _mm_or_si128(Reg2, Reg1);
		Reg1 = _mm_and_si128(Reg1, Mask4);

		//REG1 = ((REG1 <<  8) | REG1) & glm::uint64(0x00FF00FF00FF00FF);
		//REG2 = ((REG2 <<  8) | REG2) & glm::uint64(0x00FF00FF00FF00FF);
		Reg2 = _mm_slli_si128(Reg1, 1);
		Reg1 = _mm_or_si128(Reg2, Reg1);
		Reg1 = _mm_and_si128(Reg1, Mask3);

		//REG1 = ((REG1 <<  4) | REG1) & glm::uint64(0x0F0F0F0F0F0F0F0F);
		//REG2 = ((REG2 <<  4) | REG2) & glm::uint64(0x0F0F0F0F0F0F0F0F);
		Reg2 = _mm_slli_epi32(Reg1, 4);
		Reg1 = _mm_or_si128(Reg2, Reg1);
		Reg1 = _mm_and_si128(Reg1, Mask2);

		//REG1 = ((REG1 <<  2) | REG1) & glm::uint64(0x3333333333333333);
		//REG2 = ((REG2 <<  2) | REG2) & glm::uint64(0x3333333333333333);
		Reg2 = _mm_slli_epi32(Reg1, 2);
		Reg1 = _mm_or_si128(Reg2, Reg1);
		Reg1 = _mm_and_si128(Reg1, Mask1);

		//REG1 = ((REG1 <<  1) | REG1) & glm::uint64(0x5555555555555555);
		//REG2 = ((REG2 <<  1) | REG2) & glm::uint64(0x5555555555555555);
		Reg2 = _mm_slli_epi32(Reg1, 1);
		Reg1 = _mm_or_si128(Reg2, Reg1);
		Reg1 = _mm_and_si128(Reg1, Mask0);

		//return REG1 | (REG2 << 1);
		Reg2 = _mm_slli_epi32(Reg1, 1);
		Reg2 = _mm_srli_si128(Reg2, 8);
		Reg1 = _mm_or_si128(Reg1, Reg2);
	
		return Reg1;
	}
}//namespace detail
}//namespace glm
