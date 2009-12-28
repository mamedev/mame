INLINE int alpha_compare_nac_nda(UINT8 comb_alpha)
{
	return 1;
}

INLINE int alpha_compare_nac_da(UINT8 comb_alpha)
{
	return 1;
}

INLINE int alpha_compare_ac_nda(UINT8 comb_alpha)
{
	if (comb_alpha < blend_color.i.a)
	{
		return 0;
	}
	return 1;
}

INLINE int alpha_compare_ac_da(UINT8 comb_alpha)
{
	return comb_alpha < rdp_rand();
}

