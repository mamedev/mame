INLINE void alpha_cvg_get_cta_ca(UINT8 *comb_alpha);
INLINE void alpha_cvg_get_ncta_ca(UINT8 *comb_alpha);
INLINE void alpha_cvg_get_cta_nca(UINT8 *comb_alpha);
INLINE void alpha_cvg_get_ncta_nca(UINT8 *comb_alpha);

static void (*rdp_alpha_cvg_func[4])(UINT8*) =
{
	alpha_cvg_get_ncta_nca, alpha_cvg_get_ncta_ca, alpha_cvg_get_cta_nca, alpha_cvg_get_cta_ca,
};
