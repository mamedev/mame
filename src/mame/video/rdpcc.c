/*
#if defined(NOISE)
	#define COLOR_COMBINER1_NOISE(color)
		noise_color.i.r = rdp_rand();
		noise_color.i.g = rdp_rand();
		noise_color.i.b = rdp_rand();
		color.i.r = COMBINER_EQUATION(*combiner_rgbsub_a_r[1],*combiner_rgbsub_b_r[1],*combiner_rgbmul_r[1],*combiner_rgbadd_r[1]);
		color.i.g = COMBINER_EQUATION(*combiner_rgbsub_a_g[1],*combiner_rgbsub_b_g[1],*combiner_rgbmul_g[1],*combiner_rgbadd_g[1]);
		color.i.b = COMBINER_EQUATION(*combiner_rgbsub_a_b[1],*combiner_rgbsub_b_b[1],*combiner_rgbmul_b[1],*combiner_rgbadd_b[1]);
		color.i.a = COMBINER_EQUATION(*combiner_alphasub_a[1],*combiner_alphasub_b[1],*combiner_alphamul[1],*combiner_alphaadd[1]);
		alpha_cvg_get(&color.i.a);
#else*/
	#define COLOR_COMBINER1(color) \
		color.i.r = COMBINER_EQUATION(*combiner_rgbsub_a_r[1],*combiner_rgbsub_b_r[1],*combiner_rgbmul_r[1],*combiner_rgbadd_r[1]); \
		color.i.g = COMBINER_EQUATION(*combiner_rgbsub_a_g[1],*combiner_rgbsub_b_g[1],*combiner_rgbmul_g[1],*combiner_rgbadd_g[1]); \
		color.i.b = COMBINER_EQUATION(*combiner_rgbsub_a_b[1],*combiner_rgbsub_b_b[1],*combiner_rgbmul_b[1],*combiner_rgbadd_b[1]); \
		color.i.a = COMBINER_EQUATION(*combiner_alphasub_a[1],*combiner_alphasub_b[1],*combiner_alphamul[1],*combiner_alphaadd[1]); \
		alpha_cvg_get(&color.i.a);
//#endif

/*
#if defined(NOISE)
	#define COLOR_COMBINER2_C0_NOISE(color)
		noise_color.i.r = rdp_rand();
		noise_color.i.g = rdp_rand();
		noise_color.i.b = rdp_rand();
		color.i.r = COMBINER_EQUATION(*combiner_rgbsub_a_r[0],*combiner_rgbsub_b_r[0],*combiner_rgbmul_r[0],*combiner_rgbadd_r[0]);
		color.i.g = COMBINER_EQUATION(*combiner_rgbsub_a_g[0],*combiner_rgbsub_b_g[0],*combiner_rgbmul_g[0],*combiner_rgbadd_g[0]);
		color.i.b = COMBINER_EQUATION(*combiner_rgbsub_a_b[0],*combiner_rgbsub_b_b[0],*combiner_rgbmul_b[0],*combiner_rgbadd_b[0]);
		color.i.a = COMBINER_EQUATION(*combiner_alphasub_a[0],*combiner_alphasub_b[0],*combiner_alphamul[0],*combiner_alphaadd[0]);
		combined_color.c = color.c;
#else*/
	#define COLOR_COMBINER2_C0(color) \
		color.i.r = COMBINER_EQUATION(*combiner_rgbsub_a_r[0],*combiner_rgbsub_b_r[0],*combiner_rgbmul_r[0],*combiner_rgbadd_r[0]); \
		color.i.g = COMBINER_EQUATION(*combiner_rgbsub_a_g[0],*combiner_rgbsub_b_g[0],*combiner_rgbmul_g[0],*combiner_rgbadd_g[0]); \
		color.i.b = COMBINER_EQUATION(*combiner_rgbsub_a_b[0],*combiner_rgbsub_b_b[0],*combiner_rgbmul_b[0],*combiner_rgbadd_b[0]); \
		color.i.a = COMBINER_EQUATION(*combiner_alphasub_a[0],*combiner_alphasub_b[0],*combiner_alphamul[0],*combiner_alphaadd[0]); \
		combined_color.c = color.c;
//#endif


/*#if defined(NOISE)
	#define COLOR_COMBINER2_C1_NOISE(color)
		color.c = texel0_color.c;
		texel0_color.c = texel1_color.c;
		texel1_color.c = color.c;
		noise_color.i.r = rdp_rand();
		noise_color.i.g = rdp_rand();
		noise_color.i.b = rdp_rand();
		color.i.r = COMBINER_EQUATION(*combiner_rgbsub_a_r[1],*combiner_rgbsub_b_r[1],*combiner_rgbmul_r[1],*combiner_rgbadd_r[1]);
		color.i.g = COMBINER_EQUATION(*combiner_rgbsub_a_g[1],*combiner_rgbsub_b_g[1],*combiner_rgbmul_g[1],*combiner_rgbadd_g[1]);
		color.i.b = COMBINER_EQUATION(*combiner_rgbsub_a_b[1],*combiner_rgbsub_b_b[1],*combiner_rgbmul_b[1],*combiner_rgbadd_b[1]);
		color.i.a = COMBINER_EQUATION(*combiner_alphasub_a[1],*combiner_alphasub_b[1],*combiner_alphamul[1],*combiner_alphaadd[1]);
		alpha_cvg_get(&color.i.a);
#else*/
	#define COLOR_COMBINER2_C1(color) \
		color.c = texel0_color.c; \
		texel0_color.c = texel1_color.c; \
		texel1_color.c = color.c; \
		color.i.r = COMBINER_EQUATION(*combiner_rgbsub_a_r[1],*combiner_rgbsub_b_r[1],*combiner_rgbmul_r[1],*combiner_rgbadd_r[1]); \
		color.i.g = COMBINER_EQUATION(*combiner_rgbsub_a_g[1],*combiner_rgbsub_b_g[1],*combiner_rgbmul_g[1],*combiner_rgbadd_g[1]); \
		color.i.b = COMBINER_EQUATION(*combiner_rgbsub_a_b[1],*combiner_rgbsub_b_b[1],*combiner_rgbmul_b[1],*combiner_rgbadd_b[1]); \
		color.i.a = COMBINER_EQUATION(*combiner_alphasub_a[1],*combiner_alphasub_b[1],*combiner_alphamul[1],*combiner_alphaadd[1]); \
		alpha_cvg_get(&color.i.a);
//#endif
