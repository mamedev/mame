#if defined(NOISE)
	INLINE UINT32 COLOR_COMBINER1_NOISE(void)
#else
	INLINE UINT32 COLOR_COMBINER1_NNOISE(void)
#endif
{
	COLOR c;
#if defined(NOISE)
	noise_color.i.r = rdp_rand();
	noise_color.i.g = rdp_rand();
	noise_color.i.b = rdp_rand();
#endif

	c.i.r = COMBINER_EQUATION(*combiner_rgbsub_a_r[1],*combiner_rgbsub_b_r[1],*combiner_rgbmul_r[1],*combiner_rgbadd_r[1]);
	c.i.g = COMBINER_EQUATION(*combiner_rgbsub_a_g[1],*combiner_rgbsub_b_g[1],*combiner_rgbmul_g[1],*combiner_rgbadd_g[1]);
	c.i.b = COMBINER_EQUATION(*combiner_rgbsub_a_b[1],*combiner_rgbsub_b_b[1],*combiner_rgbmul_b[1],*combiner_rgbadd_b[1]);
	c.i.a = COMBINER_EQUATION(*combiner_alphasub_a[1],*combiner_alphasub_b[1],*combiner_alphamul[1],*combiner_alphaadd[1]);

	//Alpha coverage combiner
	alpha_cvg_get(&c.i.a);

	return c.c;
}

#if defined(NOISE)
	INLINE UINT32 COLOR_COMBINER2_C0_NOISE(void)
#else
	INLINE UINT32 COLOR_COMBINER2_C0_NNOISE(void)
#endif
{
	COLOR c;
#if defined(NOISE)
	noise_color.i.r = rdp_rand();
	noise_color.i.g = rdp_rand();
	noise_color.i.b = rdp_rand();
#endif

	c.i.r = COMBINER_EQUATION(*combiner_rgbsub_a_r[0],*combiner_rgbsub_b_r[0],*combiner_rgbmul_r[0],*combiner_rgbadd_r[0]);
	c.i.g = COMBINER_EQUATION(*combiner_rgbsub_a_g[0],*combiner_rgbsub_b_g[0],*combiner_rgbmul_g[0],*combiner_rgbadd_g[0]);
	c.i.b = COMBINER_EQUATION(*combiner_rgbsub_a_b[0],*combiner_rgbsub_b_b[0],*combiner_rgbmul_b[0],*combiner_rgbadd_b[0]);
	c.i.a = COMBINER_EQUATION(*combiner_alphasub_a[0],*combiner_alphasub_b[0],*combiner_alphamul[0],*combiner_alphaadd[0]);
	//COMBINER_EQUATION(&c->i.r, combiner_rgbsub_a_r[0],combiner_rgbsub_b_r[0],combiner_rgbmul_r[0],combiner_rgbadd_r[0]);
	//COMBINER_EQUATION(&c->i.g, combiner_rgbsub_a_g[0],combiner_rgbsub_b_g[0],combiner_rgbmul_g[0],combiner_rgbadd_g[0]);
	//COMBINER_EQUATION(&c->i.b, combiner_rgbsub_a_b[0],combiner_rgbsub_b_b[0],combiner_rgbmul_b[0],combiner_rgbadd_b[0]);
	//COMBINER_EQUATION(&c->i.a, combiner_alphasub_a[0],combiner_alphasub_b[0],combiner_alphamul[0],combiner_alphaadd[0]);

	combined_color.c = c.c;

	return c.c;
}

#if defined(NOISE)
	INLINE UINT32 COLOR_COMBINER2_C1_NOISE(void)
#else
	INLINE UINT32 COLOR_COMBINER2_C1_NNOISE(void)
#endif
{
	COLOR c;
	c.c = texel0_color.c;
	texel0_color.c = texel1_color.c;
	texel1_color.c = c.c;

#if defined(NOISE)
	noise_color.i.r = rdp_rand();
	noise_color.i.g = rdp_rand();
	noise_color.i.b = rdp_rand();
#endif

	c.i.r = COMBINER_EQUATION(*combiner_rgbsub_a_r[1],*combiner_rgbsub_b_r[1],*combiner_rgbmul_r[1],*combiner_rgbadd_r[1]);
	c.i.g = COMBINER_EQUATION(*combiner_rgbsub_a_g[1],*combiner_rgbsub_b_g[1],*combiner_rgbmul_g[1],*combiner_rgbadd_g[1]);
	c.i.b = COMBINER_EQUATION(*combiner_rgbsub_a_b[1],*combiner_rgbsub_b_b[1],*combiner_rgbmul_b[1],*combiner_rgbadd_b[1]);
	c.i.a = COMBINER_EQUATION(*combiner_alphasub_a[1],*combiner_alphasub_b[1],*combiner_alphamul[1],*combiner_alphaadd[1]);
	//COMBINER_EQUATION(&c->i.r, combiner_rgbsub_a_r[1],combiner_rgbsub_b_r[1],combiner_rgbmul_r[1],combiner_rgbadd_r[1]);
	//COMBINER_EQUATION(&c->i.g, combiner_rgbsub_a_g[1],combiner_rgbsub_b_g[1],combiner_rgbmul_g[1],combiner_rgbadd_g[1]);
	//COMBINER_EQUATION(&c->i.b, combiner_rgbsub_a_b[1],combiner_rgbsub_b_b[1],combiner_rgbmul_b[1],combiner_rgbadd_b[1]);
	//COMBINER_EQUATION(&c->i.a, combiner_alphasub_a[1],combiner_alphasub_b[1],combiner_alphamul[1],combiner_alphaadd[1]);

	alpha_cvg_get(&c.i.a);

	return c.c;
}
