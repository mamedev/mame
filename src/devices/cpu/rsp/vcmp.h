// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_veq_vge_vlt_vne(UINT32 iw, rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t zero, rsp_vec_t *le, rsp_vec_t eq, rsp_vec_t sign)
{
	rsp_vec_t equal = _mm_cmpeq_epi16(vs, vt);

	if (iw & 0x2) // VNE & VGE
	{
		if (iw & 0x1) // VGE
		{
			rsp_vec_t gt = _mm_cmpgt_epi16(vs, vt);
			rsp_vec_t equalsign = _mm_and_si128(eq, sign);

			equal = _mm_andnot_si128(equalsign, equal);
			*le = _mm_or_si128(gt, equal);
		}
		else // VNE
		{
			rsp_vec_t nequal = _mm_cmpeq_epi16(equal, zero);

			*le = _mm_and_si128(eq, equal);
			*le = _mm_or_si128(*le, nequal);
		}
	}
	else // VEQ & VLT
	{
		if (iw & 0x1) // VEQ
		{
			*le = _mm_andnot_si128(eq, equal);
		}
		else // VLT
		{
			rsp_vec_t lt = _mm_cmplt_epi16(vs, vt);

			equal = _mm_and_si128(eq, equal);
			equal = _mm_and_si128(sign, equal);
			*le = _mm_or_si128(lt, equal);
		}
	}

#if (defined(__SSE4_1__) || defined(_MSC_VER))
	return _mm_blendv_epi8(vt, vs, *le);
#else
	vs = _mm_and_si128(*le, vs);
	vt = _mm_andnot_si128(*le, vt);
	return _mm_or_si128(vs, vt);
#endif
}
