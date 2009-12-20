INLINE void TEXTURE_PIPELINE_BILINEAR_NMID(COLOR* TEX, INT32 SSS, INT32 SST, TILE* tex_tile)
{
#define RELATIVE(x, y) 	((((x) >> 3) - (y)) << 3) | (x & 7);
	INT32 maxs, maxt;
	COLOR t0, t1, t2, t3;
	int sss1, sst1, sss2, sst2;

	INT32 SFRAC = 0, TFRAC = 0, INVSF = 0, INVTF = 0;
	INT32 R32, G32, B32, A32;
	INT32 maxs2, maxt2;
	int upper = 0;

	t0.c = 0;
	t1.c = 0;
	t2.c = 0;
	t3.c = 0;

	sss1 = SSS;
	sst1 = SST;

	texshift(&sss1, &sst1, &maxs, &maxt, tex_tile);

	sss2 = sss1 + 32; sst2 = sst1 + 32;
	maxs2 = ((sss2 >> 3) >= tex_tile->sh);
	maxt2 = ((sst2 >> 3) >= tex_tile->th);

	sss1 = RELATIVE(sss1, tex_tile->sl);
	sst1 = RELATIVE(sst1, tex_tile->tl);
	sss2 = RELATIVE(sss2, tex_tile->sl);
	sst2 = RELATIVE(sst2, tex_tile->tl);

	SFRAC = sss1 & 0x1f;
	TFRAC = sst1 & 0x1f;

	CLAMP(&sss1, &sst1, &SFRAC, &TFRAC, maxs, maxt, tex_tile);
	CLAMP_LIGHT(&sss2, &sst2, maxs2, maxt2, tex_tile);

	MASK(&sss1, &sst1, tex_tile);
	MASK(&sss2, &sst2, tex_tile);

	upper = ((SFRAC + TFRAC) >= 0x20);
	if (upper)
	{
		INVSF = 0x20 - SFRAC;
		INVTF = 0x20 - TFRAC;
	}

	if (!upper)
	{
		FETCH_TEXEL(&t0, sss1, sst1, tex_tile);
	}
	FETCH_TEXEL(&t1, sss2, sst1, tex_tile);
	FETCH_TEXEL(&t2, sss1, sst2, tex_tile);
	if (upper)
	{
		FETCH_TEXEL(&t3, sss2, sst2, tex_tile);
	}

	if (upper)
	{
		R32 = t3.i.r + ((INVSF*(t2.i.r - t3.i.r))>>5) + ((INVTF*(t1.i.r - t3.i.r))>>5);
		TEX->i.r = (R32 < 0) ? 0 : R32;
		G32 = t3.i.g + ((INVSF*(t2.i.g - t3.i.g))>>5) + ((INVTF*(t1.i.g - t3.i.g))>>5);
		TEX->i.g = (G32 < 0) ? 0 : G32;
		B32 = t3.i.b + ((INVSF*(t2.i.b - t3.i.b))>>5) + ((INVTF*(t1.i.b - t3.i.b))>>5);
		TEX->i.b = (B32 < 0) ? 0 : B32;
		A32 = t3.i.a + ((INVSF*(t2.i.a - t3.i.a))>>5) + ((INVTF*(t1.i.a - t3.i.a))>>5);
		TEX->i.a = (A32 < 0) ? 0 : A32;
	}
	else
	{
		R32 = t0.i.r + ((SFRAC*(t1.i.r - t0.i.r))>>5) + ((TFRAC*(t2.i.r - t0.i.r))>>5);
		TEX->i.r = (R32 < 0) ? 0 : R32;
		G32 = t0.i.g + ((SFRAC*(t1.i.g - t0.i.g))>>5) + ((TFRAC*(t2.i.g - t0.i.g))>>5);
		TEX->i.g = (G32 < 0) ? 0 : G32;
		B32 = t0.i.b + ((SFRAC*(t1.i.b - t0.i.b))>>5) + ((TFRAC*(t2.i.b - t0.i.b))>>5);
		TEX->i.b = (B32 < 0) ? 0 : B32;
		A32 = t0.i.a + ((SFRAC*(t1.i.a - t0.i.a))>>5) + ((TFRAC*(t2.i.a - t0.i.a))>>5);
		TEX->i.a = (A32 < 0) ? 0 : A32;
	}
}

INLINE void TEXTURE_PIPELINE_BILINEAR_MID(COLOR* TEX, INT32 SSS, INT32 SST, TILE* tex_tile)
{
#define RELATIVE(x, y) 	((((x) >> 3) - (y)) << 3) | (x & 7);
	INT32 maxs, maxt;
	COLOR t0, t1, t2, t3;
	int sss1, sst1, sss2, sst2;

	INT32 SFRAC = 0, TFRAC = 0, INVSF = 0, INVTF = 0;
	INT32 R32, G32, B32, A32;
	INT32 maxs2, maxt2;
	int upper = 0;

	t0.c = 0;
	t1.c = 0;
	t2.c = 0;
	t3.c = 0;

	sss1 = SSS;
	sst1 = SST;

	texshift(&sss1, &sst1, &maxs, &maxt, tex_tile);

	sss2 = sss1 + 32; sst2 = sst1 + 32;
	maxs2 = ((sss2 >> 3) >= tex_tile->sh);
	maxt2 = ((sst2 >> 3) >= tex_tile->th);

	sss1 = RELATIVE(sss1, tex_tile->sl);
	sst1 = RELATIVE(sst1, tex_tile->tl);
	sss2 = RELATIVE(sss2, tex_tile->sl);
	sst2 = RELATIVE(sst2, tex_tile->tl);

	SFRAC = sss1 & 0x1f;
	TFRAC = sst1 & 0x1f;

	CLAMP(&sss1, &sst1, &SFRAC, &TFRAC, maxs, maxt, tex_tile);
	CLAMP_LIGHT(&sss2, &sst2, maxs2, maxt2, tex_tile);

	MASK(&sss1, &sst1, tex_tile);
	MASK(&sss2, &sst2, tex_tile);

	upper = ((SFRAC + TFRAC) >= 0x20);
	if (upper)
	{
		INVSF = 0x20 - SFRAC;
		INVTF = 0x20 - TFRAC;
	}

	FETCH_TEXEL(&t0, sss1, sst1, tex_tile);
	FETCH_TEXEL(&t1, sss2, sst1, tex_tile);
	FETCH_TEXEL(&t2, sss1, sst2, tex_tile);
	FETCH_TEXEL(&t3, sss2, sst2, tex_tile);

	if (SFRAC!= 0x10 || TFRAC != 0x10)
	{
		if (upper)
		{
			R32 = t3.i.r + ((INVSF*(t2.i.r - t3.i.r))>>5) + ((INVTF*(t1.i.r - t3.i.r))>>5);
			TEX->i.r = (R32 < 0) ? 0 : R32;
			G32 = t3.i.g + ((INVSF*(t2.i.g - t3.i.g))>>5) + ((INVTF*(t1.i.g - t3.i.g))>>5);
			TEX->i.g = (G32 < 0) ? 0 : G32;
			B32 = t3.i.b + ((INVSF*(t2.i.b - t3.i.b))>>5) + ((INVTF*(t1.i.b - t3.i.b))>>5);
			TEX->i.b = (B32 < 0) ? 0 : B32;
			A32 = t3.i.a + ((INVSF*(t2.i.a - t3.i.a))>>5) + ((INVTF*(t1.i.a - t3.i.a))>>5);
			TEX->i.a = (A32 < 0) ? 0 : A32;
		}
		else
		{
			R32 = t0.i.r + ((SFRAC*(t1.i.r - t0.i.r))>>5) + ((TFRAC*(t2.i.r - t0.i.r))>>5);
			TEX->i.r = (R32 < 0) ? 0 : R32;
			G32 = t0.i.g + ((SFRAC*(t1.i.g - t0.i.g))>>5) + ((TFRAC*(t2.i.g - t0.i.g))>>5);
			TEX->i.g = (G32 < 0) ? 0 : G32;
			B32 = t0.i.b + ((SFRAC*(t1.i.b - t0.i.b))>>5) + ((TFRAC*(t2.i.b - t0.i.b))>>5);
			TEX->i.b = (B32 < 0) ? 0 : B32;
			A32 = t0.i.a + ((SFRAC*(t1.i.a - t0.i.a))>>5) + ((TFRAC*(t2.i.a - t0.i.a))>>5);
			TEX->i.a = (A32 < 0) ? 0 : A32;
		}
	}
	else // Is this accurate?
	{
		TEX->i.r = (t0.i.r + t1.i.r + t2.i.r + t3.i.r) >> 2;
		TEX->i.g = (t0.i.g + t1.i.g + t2.i.g + t3.i.g) >> 2;
		TEX->i.b = (t0.i.b + t1.i.b + t2.i.b + t3.i.b) >> 2;
		TEX->i.a = (t0.i.a + t1.i.a + t2.i.a + t3.i.a) >> 2;
	}
}

INLINE void TEXTURE_PIPELINE_NEAREST_NMID(COLOR* TEX, INT32 SSS, INT32 SST, TILE* tex_tile)
{
#define RELATIVE(x, y) 	((((x) >> 3) - (y)) << 3) | (x & 7);
	INT32 maxs, maxt;
	int sss1, sst1;
	INT32 SFRAC, TFRAC;

	sss1 = SSS;
	sst1 = SST;

	texshift(&sss1, &sst1, &maxs, &maxt, tex_tile);
	sss1 = RELATIVE(sss1, tex_tile->sl);
	sst1 = RELATIVE(sst1, tex_tile->tl);

	sss1 += 0x10;
	sst1 += 0x10;

	SFRAC = sss1 & 0x1f;
	TFRAC = sst1 & 0x1f;

	CLAMP(&sss1, &sst1, &SFRAC, &TFRAC, maxs, maxt, tex_tile);

	MASK(&sss1, &sst1, tex_tile);

	/* point sample */
	FETCH_TEXEL(TEX, sss1, sst1, tex_tile);
}
