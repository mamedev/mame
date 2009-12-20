INLINE int alpha_compare_nac_nda(running_machine *machine, UINT8 comb_alpha)
{
	return 1;
}

INLINE int alpha_compare_nac_da(running_machine *machine, UINT8 comb_alpha)
{
	return 1;
}

INLINE int alpha_compare_ac_nda(running_machine *machine, UINT8 comb_alpha)
{
	if (comb_alpha < blend_color.i.a)
	{
		return 0;
	}
	return 1;
}

INLINE int alpha_compare_ac_da(running_machine *machine, UINT8 comb_alpha)
{
	if (comb_alpha < (mame_rand(machine) & 0xff))
	{
		return 0;
	}
	return 1;
}

