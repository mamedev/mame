// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vmrg(rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t le)
{
#if (defined(__SSE4_1__) || defined(_MSC_VER))
	return _mm_blendv_epi8(vt, vs, le);
#else
	vs = _mm_and_si128(le, vs);
	vt = _mm_andnot_si128(le, vt);
	return _mm_or_si128(vs, vt);
#endif
}
