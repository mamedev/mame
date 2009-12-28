#define RELATIVE(x, y) 	(((((x) >> 3) - (y)) << 3) | (x & 7))

INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID(INT32 SSS, INT32 SST, TILE* tex_tile)
{
	INT32 maxs, maxt;
	COLOR t0, t1, t2, t3, TEX;
	int sss2, sst2;

	INT32 SFRAC = 0, TFRAC = 0, INVSF = 0, INVTF = 0;
	INT32 R32, G32, B32, A32;
	INT32 maxs2, maxt2;

	texshift(&SSS, &SST, &maxs, &maxt, tex_tile);

	sss2 = SSS + 32; sst2 = SST + 32;
	maxs2 = ((sss2 >> 3) >= tex_tile->sh);
	maxt2 = ((sst2 >> 3) >= tex_tile->th);

	SSS = RELATIVE(SSS, tex_tile->sl);
	SST = RELATIVE(SST, tex_tile->tl);
	sss2 = RELATIVE(sss2, tex_tile->sl);
	sst2 = RELATIVE(sst2, tex_tile->tl);

	SFRAC = SSS & 0x1f;
	TFRAC = SST & 0x1f;

	CLAMP(&SSS, &SST, &SFRAC, &TFRAC, maxs, maxt, tex_tile);
	CLAMP_LIGHT(&sss2, &sst2, maxs2, maxt2, tex_tile);

	MASK(&SSS, &SST, tex_tile);
	MASK(&sss2, &sst2, tex_tile);

	t1.c = FETCH_TEXEL(sss2, SST);
	t2.c = FETCH_TEXEL(SSS, sst2);

	if ((SFRAC + TFRAC) < 0x20)
	{
		t0.c = FETCH_TEXEL(SSS, SST);
		R32 = t0.i.r + ((SFRAC*(t1.i.r - t0.i.r))>>5) + ((TFRAC*(t2.i.r - t0.i.r))>>5);
		TEX.i.r = (R32 < 0) ? 0 : R32;
		G32 = t0.i.g + ((SFRAC*(t1.i.g - t0.i.g))>>5) + ((TFRAC*(t2.i.g - t0.i.g))>>5);
		TEX.i.g = (G32 < 0) ? 0 : G32;
		B32 = t0.i.b + ((SFRAC*(t1.i.b - t0.i.b))>>5) + ((TFRAC*(t2.i.b - t0.i.b))>>5);
		TEX.i.b = (B32 < 0) ? 0 : B32;
		A32 = t0.i.a + ((SFRAC*(t1.i.a - t0.i.a))>>5) + ((TFRAC*(t2.i.a - t0.i.a))>>5);
		TEX.i.a = (A32 < 0) ? 0 : A32;
	}
	else
	{
		INVSF = 0x20 - SFRAC;
		INVTF = 0x20 - TFRAC;
		t3.c = FETCH_TEXEL(sss2, sst2);
		R32 = t3.i.r + ((INVSF*(t2.i.r - t3.i.r))>>5) + ((INVTF*(t1.i.r - t3.i.r))>>5);
		TEX.i.r = (R32 < 0) ? 0 : R32;
		G32 = t3.i.g + ((INVSF*(t2.i.g - t3.i.g))>>5) + ((INVTF*(t1.i.g - t3.i.g))>>5);
		TEX.i.g = (G32 < 0) ? 0 : G32;
		B32 = t3.i.b + ((INVSF*(t2.i.b - t3.i.b))>>5) + ((INVTF*(t1.i.b - t3.i.b))>>5);
		TEX.i.b = (B32 < 0) ? 0 : B32;
		A32 = t3.i.a + ((INVSF*(t2.i.a - t3.i.a))>>5) + ((INVTF*(t1.i.a - t3.i.a))>>5);
		TEX.i.a = (A32 < 0) ? 0 : A32;
	}

	return TEX.c;
}

INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID(INT32 SSS, INT32 SST, TILE* tex_tile)
{
	INT32 maxs, maxt;
	COLOR t0, t1, t2, t3, TEX;
	int sss2, sst2;

	INT32 SFRAC = 0, TFRAC = 0, INVSF = 0, INVTF = 0;
	INT32 R32, G32, B32, A32;
	INT32 maxs2, maxt2;

	texshift(&SSS, &SST, &maxs, &maxt, tex_tile);

	sss2 = SSS + 32; sst2 = SST + 32;
	maxs2 = ((sss2 >> 3) >= tex_tile->sh);
	maxt2 = ((sst2 >> 3) >= tex_tile->th);

	SSS = RELATIVE(SSS, tex_tile->sl);
	SST = RELATIVE(SST, tex_tile->tl);
	sss2 = RELATIVE(sss2, tex_tile->sl);
	sst2 = RELATIVE(sst2, tex_tile->tl);

	SFRAC = SSS & 0x1f;
	TFRAC = SST & 0x1f;

	CLAMP(&SSS, &SST, &SFRAC, &TFRAC, maxs, maxt, tex_tile);
	CLAMP_LIGHT(&sss2, &sst2, maxs2, maxt2, tex_tile);

	MASK(&SSS, &SST, tex_tile);
	MASK(&sss2, &sst2, tex_tile);


	t0.c = FETCH_TEXEL(SSS, SST);
	t1.c = FETCH_TEXEL(sss2, SST);
	t2.c = FETCH_TEXEL(SSS, sst2);
	t3.c = FETCH_TEXEL(sss2, sst2);

	if (SFRAC!= 0x10 || TFRAC != 0x10)
	{
		if ((SFRAC + TFRAC) >= 0x20)
		{
			INVSF = 0x20 - SFRAC;
			INVTF = 0x20 - TFRAC;
			R32 = t3.i.r + ((INVSF*(t2.i.r - t3.i.r))>>5) + ((INVTF*(t1.i.r - t3.i.r))>>5);
			TEX.i.r = (R32 < 0) ? 0 : R32;
			G32 = t3.i.g + ((INVSF*(t2.i.g - t3.i.g))>>5) + ((INVTF*(t1.i.g - t3.i.g))>>5);
			TEX.i.g = (G32 < 0) ? 0 : G32;
			B32 = t3.i.b + ((INVSF*(t2.i.b - t3.i.b))>>5) + ((INVTF*(t1.i.b - t3.i.b))>>5);
			TEX.i.b = (B32 < 0) ? 0 : B32;
			A32 = t3.i.a + ((INVSF*(t2.i.a - t3.i.a))>>5) + ((INVTF*(t1.i.a - t3.i.a))>>5);
			TEX.i.a = (A32 < 0) ? 0 : A32;
		}
		else
		{
			R32 = t0.i.r + ((SFRAC*(t1.i.r - t0.i.r))>>5) + ((TFRAC*(t2.i.r - t0.i.r))>>5);
			TEX.i.r = (R32 < 0) ? 0 : R32;
			G32 = t0.i.g + ((SFRAC*(t1.i.g - t0.i.g))>>5) + ((TFRAC*(t2.i.g - t0.i.g))>>5);
			TEX.i.g = (G32 < 0) ? 0 : G32;
			B32 = t0.i.b + ((SFRAC*(t1.i.b - t0.i.b))>>5) + ((TFRAC*(t2.i.b - t0.i.b))>>5);
			TEX.i.b = (B32 < 0) ? 0 : B32;
			A32 = t0.i.a + ((SFRAC*(t1.i.a - t0.i.a))>>5) + ((TFRAC*(t2.i.a - t0.i.a))>>5);
			TEX.i.a = (A32 < 0) ? 0 : A32;
		}
	}
	else // Is this accurate?
	{
		TEX.i.r = (t0.i.r + t1.i.r + t2.i.r + t3.i.r) >> 2;
		TEX.i.g = (t0.i.g + t1.i.g + t2.i.g + t3.i.g) >> 2;
		TEX.i.b = (t0.i.b + t1.i.b + t2.i.b + t3.i.b) >> 2;
		TEX.i.a = (t0.i.a + t1.i.a + t2.i.a + t3.i.a) >> 2;
	}

	return TEX.c;
}

INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID(INT32 SSS, INT32 SST, TILE* tex_tile)
{
	INT32 maxs, maxt;
	INT32 SFRAC, TFRAC;

	texshift(&SSS, &SST, &maxs, &maxt, tex_tile);
	SSS = RELATIVE(SSS, tex_tile->sl);
	SST = RELATIVE(SST, tex_tile->tl);

	SSS += 0x10;
	SST += 0x10;

	SFRAC = SSS & 0x1f;
	TFRAC = SST & 0x1f;

	CLAMP(&SSS, &SST, &SFRAC, &TFRAC, maxs, maxt, tex_tile);

	MASK(&SSS, &SST, tex_tile);

	/* point sample */
	return FETCH_TEXEL(SSS, SST);
}

#if 0
#define RELATIVE(x, y) 	(((((x) >> 3) - (y)) << 3) | (x & 7))

INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID(INT32 SSS, INT32 SST, TILE* tex_tile)
{
	INT32 maxs, maxt;
	COLOR t0, t1, t2, t3, TEX;
	int sss2, sst2;

	INT32 SFRAC = 0;
	INT32 TFRAC = 0;
	INT32 INVSF = 0;
	INT32 INVTF = 0;
	INT32 R32 = 0;
	INT32 G32 = 0;
	INT32 B32 = 0;
	INT32 A32 = 0;

	texshift(&SSS, &SST, &maxs, &maxt, tex_tile);

	SSS = RELATIVE(SSS, tex_tile->sl);
	SST = RELATIVE(SST, tex_tile->tl);
	sss2 = RELATIVE((SSS + 32), tex_tile->sl);
	sst2 = RELATIVE((SST + 32), tex_tile->tl);

	SFRAC = SSS & 0x1f;
	TFRAC = SST & 0x1f;

	CLAMP(&SSS, &SST, &SFRAC, &TFRAC, maxs, maxt, tex_tile);
	CLAMP_LIGHT(&sss2, &sst2, ((sss2 >> 3) >= tex_tile->sh), ((sst2 >> 3) >= tex_tile->th), tex_tile);

	MASK(&SSS, &SST, tex_tile);
	MASK(&sss2, &sst2, tex_tile);

	t1.c = FETCH_TEXEL(sss2, SST);
	t2.c = FETCH_TEXEL(SSS, sst2);

	if ((SFRAC + TFRAC) >= 0x20)
	{
		INVSF = 0x20 - SFRAC;
		INVTF = 0x20 - TFRAC;
		t3.c = FETCH_TEXEL(sss2, sst2);
		R32 = t3.i.r + ((INVSF*(t2.i.r - t3.i.r))>>5) + ((INVTF*(t1.i.r - t3.i.r))>>5);
		TEX.i.r = (R32 < 0) ? 0 : R32;
		G32 = t3.i.g + ((INVSF*(t2.i.g - t3.i.g))>>5) + ((INVTF*(t1.i.g - t3.i.g))>>5);
		TEX.i.g = (G32 < 0) ? 0 : G32;
		B32 = t3.i.b + ((INVSF*(t2.i.b - t3.i.b))>>5) + ((INVTF*(t1.i.b - t3.i.b))>>5);
		TEX.i.b = (B32 < 0) ? 0 : B32;
		A32 = t3.i.a + ((INVSF*(t2.i.a - t3.i.a))>>5) + ((INVTF*(t1.i.a - t3.i.a))>>5);
		TEX.i.a = (A32 < 0) ? 0 : A32;
	}
	else
	{
		t0.c = FETCH_TEXEL(SSS, SST);
		R32 = t0.i.r + ((SFRAC*(t1.i.r - t0.i.r))>>5) + ((TFRAC*(t2.i.r - t0.i.r))>>5);
		TEX.i.r = (R32 < 0) ? 0 : R32;
		G32 = t0.i.g + ((SFRAC*(t1.i.g - t0.i.g))>>5) + ((TFRAC*(t2.i.g - t0.i.g))>>5);
		TEX.i.g = (G32 < 0) ? 0 : G32;
		B32 = t0.i.b + ((SFRAC*(t1.i.b - t0.i.b))>>5) + ((TFRAC*(t2.i.b - t0.i.b))>>5);
		TEX.i.b = (B32 < 0) ? 0 : B32;
		A32 = t0.i.a + ((SFRAC*(t1.i.a - t0.i.a))>>5) + ((TFRAC*(t2.i.a - t0.i.a))>>5);
		TEX.i.a = (A32 < 0) ? 0 : A32;
	}

	return TEX.c;
}

INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID(INT32 SSS, INT32 SST, TILE* tex_tile)
{
	INT32 maxs, maxt;
	COLOR t0, t1, t2, t3, TEX;
	int sss2, sst2;

	INT32 SFRAC = 0, TFRAC = 0, INVSF = 0, INVTF = 0;
	INT32 R32, G32, B32, A32;
	INT32 maxs2, maxt2;

	texshift(&SSS, &SST, &maxs, &maxt, tex_tile);

	sss2 = SSS + 32; sst2 = SST + 32;
	maxs2 = ((sss2 >> 3) >= tex_tile->sh);
	maxt2 = ((sst2 >> 3) >= tex_tile->th);

	SSS = RELATIVE(SSS, tex_tile->sl);
	SST = RELATIVE(SST, tex_tile->tl);
	sss2 = RELATIVE(sss2, tex_tile->sl);
	sst2 = RELATIVE(sst2, tex_tile->tl);

	SFRAC = SSS & 0x1f;
	TFRAC = SST & 0x1f;

	CLAMP(&SSS, &SST, &SFRAC, &TFRAC, maxs, maxt, tex_tile);
	CLAMP_LIGHT(&sss2, &sst2, maxs2, maxt2, tex_tile);

	MASK(&SSS, &SST, tex_tile);
	MASK(&sss2, &sst2, tex_tile);

	t0.c = FETCH_TEXEL(SSS, SST);
	t1.c = FETCH_TEXEL(sss2, SST);
	t2.c = FETCH_TEXEL(SSS, sst2);
	t3.c = FETCH_TEXEL(sss2, sst2);

	if(SFRAC != 0x10 || TFRAC != 0x10)
	{
		if (((SFRAC + TFRAC) >= 0x20))
		{
			INVSF = 0x20 - SFRAC;
			INVTF = 0x20 - TFRAC;
			R32 = t3.i.r + ((INVSF*(t2.i.r - t3.i.r))>>5) + ((INVTF*(t1.i.r - t3.i.r))>>5);
			TEX.i.r = (R32 < 0) ? 0 : R32;
			G32 = t3.i.g + ((INVSF*(t2.i.g - t3.i.g))>>5) + ((INVTF*(t1.i.g - t3.i.g))>>5);
			TEX.i.g = (G32 < 0) ? 0 : G32;
			B32 = t3.i.b + ((INVSF*(t2.i.b - t3.i.b))>>5) + ((INVTF*(t1.i.b - t3.i.b))>>5);
			TEX.i.b = (B32 < 0) ? 0 : B32;
			A32 = t3.i.a + ((INVSF*(t2.i.a - t3.i.a))>>5) + ((INVTF*(t1.i.a - t3.i.a))>>5);
			TEX.i.a = (A32 < 0) ? 0 : A32;
		}
		else
		{
			R32 = t0.i.r + ((SFRAC*(t1.i.r - t0.i.r))>>5) + ((TFRAC*(t2.i.r - t0.i.r))>>5);
			TEX.i.r = (R32 < 0) ? 0 : R32;
			G32 = t0.i.g + ((SFRAC*(t1.i.g - t0.i.g))>>5) + ((TFRAC*(t2.i.g - t0.i.g))>>5);
			TEX.i.g = (G32 < 0) ? 0 : G32;
			B32 = t0.i.b + ((SFRAC*(t1.i.b - t0.i.b))>>5) + ((TFRAC*(t2.i.b - t0.i.b))>>5);
			TEX.i.b = (B32 < 0) ? 0 : B32;
			A32 = t0.i.a + ((SFRAC*(t1.i.a - t0.i.a))>>5) + ((TFRAC*(t2.i.a - t0.i.a))>>5);
			TEX.i.a = (A32 < 0) ? 0 : A32;
		}
	}
	else // Is this accurate?
	{
		TEX.i.r = (t0.i.r + t1.i.r + t2.i.r + t3.i.r) >> 2;
		TEX.i.g = (t0.i.g + t1.i.g + t2.i.g + t3.i.g) >> 2;
		TEX.i.b = (t0.i.b + t1.i.b + t2.i.b + t3.i.b) >> 2;
		TEX.i.a = (t0.i.a + t1.i.a + t2.i.a + t3.i.a) >> 2;
	}

	return TEX.c;
}

INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID(INT32 SSS, INT32 SST, TILE* tex_tile)
{
	INT32 maxs, maxt;

	texshift(&SSS, &SST, &maxs, &maxt, tex_tile);
	SSS = RELATIVE(SSS, tex_tile->sl) + 0x10;
	SST = RELATIVE(SST, tex_tile->tl) + 0x10;

	CLAMP_QUICK(&SSS, &SST, maxs, maxt, tex_tile->num);

	MASK(&SSS, &SST, tex_tile);

	/* point sample */
	return FETCH_TEXEL(SSS, SST);
}
#endif
