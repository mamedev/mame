INLINE UINT8 alpha_cvg_get_cta_ca(UINT8 comb_alpha)
{
	UINT32 temp = comb_alpha;
	UINT32 temp2 = curpixel_cvg;
	UINT32 temp3 = 0;

	temp3 = (temp * temp2) + 4;
	curpixel_cvg = temp3 >> 8;

	temp = (temp3 >> 3);

	if (temp > 0xff)
	{
		temp = 0xff;
	}

	return (UINT8)temp;
}

INLINE UINT8 alpha_cvg_get_ncta_ca(UINT8 comb_alpha)
{
	UINT32 temp = comb_alpha;
	UINT32 temp2 = curpixel_cvg;

	temp = temp2 << 5;

	if (temp > 0xff)
	{
		temp = 0xff;
	}

	return (UINT8)temp;
}

INLINE UINT8 alpha_cvg_get_cta_nca(UINT8 comb_alpha)
{
	UINT32 temp = comb_alpha;
	UINT32 temp2 = curpixel_cvg;
	UINT32 temp3 = 0;

	temp3 = (temp * temp2) + 4;
	curpixel_cvg = temp3 >> 8;

	if (temp > 0xff)
	{
		temp = 0xff;
	}

	return (UINT8)temp;
}

INLINE UINT8 alpha_cvg_get_ncta_nca(UINT8 comb_alpha)
{
	UINT32 temp = comb_alpha;

	if (temp > 0xff)
	{
		temp = 0xff;
	}

	return (UINT8)temp;
}
