INLINE void alpha_cvg_get_cta_ca(UINT8 *comb_alpha)
{
	UINT32 temp = *comb_alpha;
	UINT32 temp2 = curpixel_cvg;

	UINT32 temp3 = (temp * temp2) + 4;
	curpixel_cvg = temp3 >> 8;

	temp = (temp3 >> 3);

	if (temp > 0xff)
	{
		*comb_alpha = 0xff;
	}
	else
	{
		*comb_alpha = (UINT8)temp;
	}
}

INLINE void alpha_cvg_get_ncta_ca(UINT8 *comb_alpha)
{
	UINT32 temp2 = curpixel_cvg;
	UINT32 temp = temp2 << 5;

	if (temp > 0xff)
	{
		*comb_alpha = 0xff;
	}
	else
	{
		*comb_alpha = (UINT8)temp;
	}
}

INLINE void alpha_cvg_get_cta_nca(UINT8 *comb_alpha)
{
	UINT32 temp = *comb_alpha;
	UINT32 temp2 = curpixel_cvg;
	UINT32 temp3 = (temp * temp2) + 4;

	curpixel_cvg = temp3 >> 8;

	if (temp > 0xff)
	{
		*comb_alpha = 0xff;
	}
	else
	{
		*comb_alpha = (UINT8)temp;
	}
}

INLINE void alpha_cvg_get_ncta_nca(UINT8 *comb_alpha)
{
	// DO nothing
}
