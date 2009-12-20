INLINE int alpha_compare_nac_nda(running_machine *machine, UINT8 comb_alpha);
INLINE int alpha_compare_nac_da(running_machine *machine, UINT8 comb_alpha);
INLINE int alpha_compare_ac_nda(running_machine *machine, UINT8 comb_alpha);
INLINE int alpha_compare_ac_da(running_machine *machine, UINT8 comb_alpha);

static int (*rdp_alpha_compare_func[4])(running_machine *, UINT8) =
{
	alpha_compare_nac_nda, alpha_compare_nac_da, alpha_compare_ac_nda, alpha_compare_ac_da
};
