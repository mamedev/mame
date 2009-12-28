INLINE UINT32 FETCH_TEXEL_RGBA4_TLUT_EN0(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT32 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	return rgb16_to_32_lut[tlut[((cached_tpal | p) ^ WORD_ADDR_XOR) << 2]];
}

INLINE UINT32 FETCH_TEXEL_RGBA4_TLUT_EN1(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT32 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	return ia8_to_32_lut[tlut[((cached_tpal | p) ^ WORD_ADDR_XOR) << 2]];
}

INLINE UINT32 FETCH_TEXEL_RGBA4_TLUT_NEN(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT32 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);

	return (cached_tpal | p) * 0x01010101;
}

INLINE UINT32 FETCH_TEXEL_RGBA8_TLUT_EN0(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	return rgb16_to_32_lut[tlut[(TMEM[taddr ^ BYTE_ADDR_XOR] ^ WORD_ADDR_XOR) << 2]];
}

INLINE UINT32 FETCH_TEXEL_RGBA8_TLUT_EN1(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	return ia8_to_32_lut[tlut[(TMEM[taddr ^ BYTE_ADDR_XOR] ^ WORD_ADDR_XOR) << 2]];
}

INLINE UINT32 FETCH_TEXEL_RGBA8_TLUT_NEN(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	return TMEM[taddr ^ BYTE_ADDR_XOR] * 0x01010101;
}

INLINE UINT32 FETCH_TEXEL_RGBA16_TLUT_EN0(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase_s1) + ((t) * (cached_twidth_s1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	return rgb16_to_32_lut[tlut[(TMEM16[(taddr & 0x7ff) ^ WORD_ADDR_XOR] >> 8) << 2]];
}

INLINE UINT32 FETCH_TEXEL_RGBA16_TLUT_EN1(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase_s1) + ((t) * (cached_twidth_s1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	return ia8_to_32_lut[tlut[(TMEM16[(taddr & 0x7ff) ^ WORD_ADDR_XOR] >> 8) << 2]];
}

INLINE UINT32 FETCH_TEXEL_RGBA16_TLUT_NEN(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase_s1) + ((t) * (cached_twidth_s1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	return rgb16_to_32_lut[TMEM16[(taddr & 0x7ff) ^ WORD_ADDR_XOR]];
}

INLINE UINT32 FETCH_TEXEL_RGBA32_TLUT_EN0(UINT32 s, UINT32 t)
{
	int xorval = (fb_size == PIXEL_SIZE_16BIT) ? XOR_SWAP_WORD : XOR_SWAP_DWORD; // Conker's Bad Fur Day, Jet Force Gemini, Super Smash Bros., Mickey's Speedway USA, Ogre Battle, Wave Race, Gex 3, South Park Rally
	UINT32 taddr = (((cached_tbase >> 2) + ((t) * (cached_twidth >> 1)) + (s)) ^ ((t & 1) ? xorval : 0)) & 0x3ff;

	return rgb16_to_32_lut[tlut[(TMEM32[taddr] >> 24) << 2]];
}

INLINE UINT32 FETCH_TEXEL_RGBA32_TLUT_EN1(UINT32 s, UINT32 t)
{
	int xorval = (fb_size == PIXEL_SIZE_16BIT) ? XOR_SWAP_WORD : XOR_SWAP_DWORD; // Conker's Bad Fur Day, Jet Force Gemini, Super Smash Bros., Mickey's Speedway USA, Ogre Battle, Wave Race, Gex 3, South Park Rally
	UINT32 taddr = (((cached_tbase >> 2) + ((t) * (cached_twidth >> 1)) + (s)) ^ ((t & 1) ? xorval : 0)) & 0x3ff;

	return ia8_to_32_lut[tlut[(TMEM32[taddr] >> 24) << 2]];
}

INLINE UINT32 FETCH_TEXEL_RGBA32_TLUT_NEN(UINT32 s, UINT32 t)
{
	int xorval = (fb_size == PIXEL_SIZE_16BIT) ? XOR_SWAP_WORD : XOR_SWAP_DWORD; // Conker's Bad Fur Day, Jet Force Gemini, Super Smash Bros., Mickey's Speedway USA, Ogre Battle, Wave Race, Gex 3, South Park Rally
	UINT32 taddr = (((cached_tbase >> 2) + ((t) * (cached_twidth >> 1)) + (s)) ^ ((t & 1) ? xorval : 0)) & 0x3ff;

	return TMEM32[taddr];
}

INLINE UINT32 FETCH_TEXEL_YUV16(UINT32 s, UINT32 t)
{	// YUV: Bottom of the 9th, Pokemon Stadium, Ogre Battle 64
	INT32 newr = 0;
	INT32 newg = 0;
	INT32 newb = 0;
	UINT32 taddr = ((cached_tbase >> 1) + ((t) * (cached_twidth)) + (s)) ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	UINT32 c1, c2;
	INT32 y;
	INT32 u, v;
	c1 = TMEM16[taddr ^ WORD_ADDR_XOR];
	c2 = TMEM16[taddr]; // other word

	if (!(taddr & 1))
	{
		v = c2 >> 8;
		u = c1 >> 8;
		y = c1 & 0xff;
	}
	else
	{
		v = c1 >> 8;
		u = c2 >> 8;
		y = c1 & 0xff;
	}
	v -= 128;
	u -= 128;

	if (!other_modes.bi_lerp0)
	{
		newr = y + ((k0 * v) >> 8);
		newg = y + ((k1 * u) >> 8) + ((k2 * v) >> 8);
		newb = y + ((k3 * u) >> 8);
	}
	return (((newr < 0) ? 0 : ((newr > 0xff) ? 0xff : newr)) << 24) |
	       (((newg < 0) ? 0 : ((newg > 0xff) ? 0xff : newg)) << 16) |
	       (((newb < 0) ? 0 : ((newb > 0xff) ? 0xff : newb)) <<  8) |
	       0xff;
}

INLINE UINT32 FETCH_TEXEL_CI4_TLUT_EN0(UINT32 s, UINT32 t)
{
	UINT32 b = TMEM[(((cached_tbase + ((t) * cached_twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff) ^ BYTE_ADDR_XOR];
	return rgb16_to_32_lut[tlut[(cached_tpal | (((s) & 1) ? (b & 0xf) : (b >> 4))) << 2]];
}

INLINE UINT32 FETCH_TEXEL_CI4_TLUT_EN1(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT32 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	return ia8_to_32_lut[tlut[(cached_tpal | p) << 2]];
}

INLINE UINT32 FETCH_TEXEL_CI4_TLUT_NEN(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT32 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);

	return (cached_tpal | p) * 0x01010101;
}

INLINE UINT32 FETCH_TEXEL_CI8_TLUT_EN0(UINT32 s, UINT32 t)
{
	return rgb16_to_32_lut[tlut[TMEM[(((cached_tbase + ((t) * cached_twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff) ^ BYTE_ADDR_XOR] << 2]];
}

INLINE UINT32 FETCH_TEXEL_CI8_TLUT_EN1(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	return ia8_to_32_lut[tlut[TMEM[taddr ^ BYTE_ADDR_XOR] << 2]];
}

INLINE UINT32 FETCH_TEXEL_CI8_TLUT_NEN(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	return TMEM[taddr ^ BYTE_ADDR_XOR] * 0x01010101;
}

INLINE UINT32 FETCH_TEXEL_CI16_TLUT_EN0(UINT32 s, UINT32 t)
{	// 16-bit CI is a "valid" mode; some games use it, it behaves the same as 16-bit RGBA
	UINT32 taddr = ((cached_tbase_s1) + ((t) * (cached_twidth_s1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	return rgb16_to_32_lut[tlut[(TMEM16[(taddr & 0x7ff) ^ WORD_ADDR_XOR] >> 8) << 2]];
}

INLINE UINT32 FETCH_TEXEL_CI16_TLUT_EN1(UINT32 s, UINT32 t)
{	// 16-bit CI is a "valid" mode; some games use it, it behaves the same as 16-bit RGBA
	UINT32 taddr = ((cached_tbase_s1) + ((t) * (cached_twidth_s1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	return ia8_to_32_lut[tlut[(TMEM16[(taddr & 0x7ff) ^ WORD_ADDR_XOR] >> 8) << 2]]; // Beetle Adventure Racing, Mount Mayhem
}

INLINE UINT32 FETCH_TEXEL_CI16_TLUT_NEN(UINT32 s, UINT32 t)
{	// 16-bit CI is a "valid" mode; some games use it, it behaves the same as 16-bit RGBA
	UINT32 taddr = ((cached_tbase_s1) + ((t) * (cached_twidth_s1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	return rgb16_to_32_lut[TMEM16[(taddr & 0x7ff) ^ WORD_ADDR_XOR]]; // PGA European Tour (U)
}

INLINE UINT32 FETCH_TEXEL_IA4_TLUT_EN0(UINT32 s, UINT32 t)
{
	UINT32 taddr = (cached_tbase + ((t) * cached_twidth) + (s >> 1)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0);
	UINT32 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	return rgb16_to_32_lut[tlut[(cached_tpal | p) << 2]];
}

INLINE UINT32 FETCH_TEXEL_IA4_TLUT_EN1(UINT32 s, UINT32 t)
{
	UINT32 taddr = (cached_tbase + ((t) * cached_twidth) + (s >> 1)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0);
	UINT32 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	return ia8_to_32_lut[tlut[(cached_tpal | p) << 2]];
}

INLINE UINT32 FETCH_TEXEL_IA4_TLUT_NEN(UINT32 s, UINT32 t)
{
	UINT32 taddr = (cached_tbase + ((t) * cached_twidth) + (s >> 1)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0);
	UINT32 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	UINT32 i = ((p & 0xe) << 4) | ((p & 0xe) << 1) | (p & 0xe >> 2);

	return (i * 0x01010100) | ((p & 0x1) ? 0xff : 0);
}

INLINE UINT32 FETCH_TEXEL_IA8_TLUT_EN0(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	return rgb16_to_32_lut[tlut[TMEM[taddr ^ BYTE_ADDR_XOR] << 2]];
}

INLINE UINT32 FETCH_TEXEL_IA8_TLUT_EN1(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	return ia8_to_32_lut[tlut[TMEM[taddr ^ BYTE_ADDR_XOR] << 2]];
}

INLINE UINT32 FETCH_TEXEL_IA8_TLUT_NEN(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	UINT32 p = TMEM[taddr ^ BYTE_ADDR_XOR];
	UINT32 i = (p >> 4) | (p & 0xf0);

	return (i * 0x01010100) | ((p & 0xf) | ((p << 4) & 0xf0));
}

INLINE UINT32 FETCH_TEXEL_IA16_TLUT_EN0(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase >> 1) + ((t) * (cached_twidth >> 1)) + (s)) ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	return rgb16_to_32_lut[tlut[(TMEM16[taddr ^ WORD_ADDR_XOR] >> 8) << 2]];
}

INLINE UINT32 FETCH_TEXEL_IA16_TLUT_EN1(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase >> 1) + ((t) * (cached_twidth >> 1)) + (s)) ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	return ia8_to_32_lut[tlut[(TMEM16[taddr ^ WORD_ADDR_XOR] >> 8) << 2]];
}

INLINE UINT32 FETCH_TEXEL_IA16_TLUT_NEN(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase >> 1) + ((t) * (cached_twidth >> 1)) + (s)) ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	UINT32 c = TMEM16[taddr ^ WORD_ADDR_XOR];

	return ((c >> 8) * 0x01010100) | (c & 0xff);
}

INLINE UINT32 FETCH_TEXEL_I4_TLUT_EN0(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	UINT32 c = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	c |= (c << 4);

	return rgb16_to_32_lut[tlut[(cached_tpal | c) << 2]];
}

INLINE UINT32 FETCH_TEXEL_I4_TLUT_EN1(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	UINT32 c = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	c |= (c << 4);

	return ia8_to_32_lut[tlut[(cached_tpal | c) << 2]];
}

INLINE UINT32 FETCH_TEXEL_I4_TLUT_NEN(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	UINT32 c = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	c |= (c << 4);

	return (c * 0x01010101);
}

INLINE UINT32 FETCH_TEXEL_I8_TLUT_EN0(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	return rgb16_to_32_lut[tlut[TMEM[taddr ^ BYTE_ADDR_XOR] << 2]];
}

INLINE UINT32 FETCH_TEXEL_I8_TLUT_EN1(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	return ia8_to_32_lut[tlut[TMEM[taddr ^ BYTE_ADDR_XOR] << 2]];
}

INLINE UINT32 FETCH_TEXEL_I8_TLUT_NEN(UINT32 s, UINT32 t)
{
	UINT32 taddr = ((cached_tbase + ((t) * cached_twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	return TMEM[taddr ^ BYTE_ADDR_XOR] * 0x01010101;
}

INLINE UINT32 FETCH_TEXEL_INVALID(UINT32 s, UINT32 t)
{
	printf("Invalid texel mode\n");
	return 0;
}

//typedef void (*rdp_fetch_texel_func)(UINT32 s, UINT32 t);
