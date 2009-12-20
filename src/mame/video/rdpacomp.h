INLINE int alpha_compare_nac_nda(UINT8 comb_alpha);
INLINE int alpha_compare_nac_da(UINT8 comb_alpha);
INLINE int alpha_compare_ac_nda(UINT8 comb_alpha);
INLINE int alpha_compare_ac_da(UINT8 comb_alpha);

static int (*rdp_alpha_compare_func[4])(UINT8) =
{
	alpha_compare_nac_nda, alpha_compare_nac_da, alpha_compare_ac_nda, alpha_compare_ac_da
};
