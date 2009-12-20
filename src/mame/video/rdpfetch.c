INLINE void FETCH_TEXEL_RGBA4_TLUT_EN0(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT8 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	UINT16 c = tlut[(((tpal << 4) | p) ^ WORD_ADDR_XOR) << 2];

	color->i.r = ((c >> 8) & 0xf8) | (c >> 13);
	color->i.g = ((c >> 3) & 0xf8) | ((c >>  8) & 0x07);
	color->i.b = ((c << 2) & 0xf8) | ((c >>  3) & 0x07);
	color->i.a = (c & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_RGBA4_TLUT_EN1(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT8 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	UINT16 c = tlut[(((tpal << 4) | p) ^ WORD_ADDR_XOR) << 2];

	color->i.r = color->i.g = color->i.b = (c >> 8) & 0xff;
	color->i.a = c & 0xff;
}

INLINE void FETCH_TEXEL_RGBA4_TLUT_NEN(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT8 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);

	color->i.r = color->i.g = color->i.b = color->i.a = (tpal << 4) | p;
}

INLINE void FETCH_TEXEL_RGBA8_TLUT_EN0(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT8 p = TMEM[taddr ^ BYTE_ADDR_XOR];
	UINT16 c = tlut[(p ^ WORD_ADDR_XOR) << 2];

	color->i.r = ((c >> 8) & 0xf8) | (c >> 13);
	color->i.g = ((c >> 3) & 0xf8) | ((c >>  8) & 0x07);
	color->i.b = ((c << 2) & 0xf8) | ((c >>  3) & 0x07);
	color->i.a = (c & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_RGBA8_TLUT_EN1(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT8 p = TMEM[taddr ^ BYTE_ADDR_XOR];
	UINT16 c = tlut[(p ^ WORD_ADDR_XOR) << 2];

	color->i.r = color->i.g = color->i.b = (c >> 8) & 0xff;
	color->i.a = c & 0xff;
}

INLINE void FETCH_TEXEL_RGBA8_TLUT_NEN(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT8 p = TMEM[taddr ^ BYTE_ADDR_XOR];

	color->i.r = color->i.g = color->i.b = color->i.a = p;
}

INLINE void FETCH_TEXEL_RGBA16_TLUT_EN0(COLOR *color, int s, int t)
{
	int taddr = ((tbase>>1) + ((t) * (twidth>>1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	UINT16 c = TMEM16[(taddr & 0x7ff) ^ WORD_ADDR_XOR]; // PGA European Tour (U)

	c = tlut[(c >> 8) << 2];
	color->i.r = ((c >> 8) & 0xf8) | (c >> 13);
	color->i.g = ((c >> 3) & 0xf8) | ((c >>  8) & 0x07);
	color->i.b = ((c << 2) & 0xf8) | ((c >>  3) & 0x07);
	color->i.a = (c & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_RGBA16_TLUT_EN1(COLOR *color, int s, int t)
{
	int taddr = ((tbase>>1) + ((t) * (twidth>>1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	UINT16 c = TMEM16[(taddr & 0x7ff) ^ WORD_ADDR_XOR]; // PGA European Tour (U)

	c = tlut[(c >> 8) << 2];
	color->i.r = color->i.g = color->i.b = (c >> 8) & 0xff;
	color->i.a = c & 0xff;
}

INLINE void FETCH_TEXEL_RGBA16_TLUT_NEN(COLOR *color, int s, int t)
{
	int taddr = ((tbase>>1) + ((t) * (twidth>>1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	UINT16 c = TMEM16[(taddr & 0x7ff) ^ WORD_ADDR_XOR]; // PGA European Tour (U)

	color->i.r = ((c >> 8) & 0xf8) | (c >> 13);
	color->i.g = ((c >> 3) & 0xf8) | ((c >>  8) & 0x07);
	color->i.b = ((c << 2) & 0xf8) | ((c >>  3) & 0x07);
	color->i.a = (c & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_RGBA32_TLUT_EN0(COLOR *color, int s, int t)
{
	int xorval = (fb_size == PIXEL_SIZE_16BIT) ? XOR_SWAP_WORD : XOR_SWAP_DWORD; // Conker's Bad Fur Day, Jet Force Gemini, Super Smash Bros., Mickey's Speedway USA, Ogre Battle, Wave Race, Gex 3, South Park Rally
	int taddr = (((tbase >> 2) + ((t) * (twidth >> 1)) + (s)) ^ ((t & 1) ? xorval : 0)) & 0x3ff;
	UINT32 c = TMEM32[taddr];

	c = tlut[(c >> 24) << 2];
	color->i.r = ((c >> 8) & 0xf8) | (c >> 13);
	color->i.g = ((c >> 3) & 0xf8) | ((c >>  8) & 0x07);
	color->i.b = ((c << 2) & 0xf8) | ((c >>  3) & 0x07);
	color->i.a = (c & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_RGBA32_TLUT_EN1(COLOR *color, int s, int t)
{
	int xorval = (fb_size == PIXEL_SIZE_16BIT) ? XOR_SWAP_WORD : XOR_SWAP_DWORD; // Conker's Bad Fur Day, Jet Force Gemini, Super Smash Bros., Mickey's Speedway USA, Ogre Battle, Wave Race, Gex 3, South Park Rally
	int taddr = (((tbase >> 2) + ((t) * (twidth >> 1)) + (s)) ^ ((t & 1) ? xorval : 0)) & 0x3ff;
	UINT32 c = TMEM32[taddr];

	c = tlut[(c >> 24) << 2];
	color->i.r = color->i.g = color->i.b = (c >> 8) & 0xff;
	color->i.a = c & 0xff;
}

INLINE void FETCH_TEXEL_RGBA32_TLUT_NEN(COLOR *color, int s, int t)
{
	int xorval = (fb_size == PIXEL_SIZE_16BIT) ? XOR_SWAP_WORD : XOR_SWAP_DWORD; // Conker's Bad Fur Day, Jet Force Gemini, Super Smash Bros., Mickey's Speedway USA, Ogre Battle, Wave Race, Gex 3, South Park Rally
	int taddr = (((tbase >> 2) + ((t) * (twidth >> 1)) + (s)) ^ ((t & 1) ? xorval : 0)) & 0x3ff;
	UINT32 c = TMEM32[taddr];

	color->c = c;
}

INLINE void FETCH_TEXEL_YUV16(COLOR *color, int s, int t)
{	// YUV: Bottom of the 9th, Pokemon Stadium, Ogre Battle 64
	INT32 newr = 0;
	INT32 newg = 0;
	INT32 newb = 0;
	int taddr = ((tbase >> 1) + ((t) * (twidth)) + (s)) ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	UINT16 c1, c2;
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
	color->i.r = (newr < 0) ? 0 : ((newr > 0xff) ? 0xff : newr);
	color->i.g = (newg < 0) ? 0 : ((newg > 0xff) ? 0xff : newg);
	color->i.b = (newb < 0) ? 0 : ((newb > 0xff) ? 0xff : newb);
	color->i.a = 0xff;
}

INLINE void FETCH_TEXEL_CI4_TLUT_EN0(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT8 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	UINT16 c = tlut[((tpal << 4) | p) << 2];

	color->i.r = ((c >> 8) & 0xf8) | (c >> 13);
	color->i.g = ((c >> 3) & 0xf8) | ((c >>  8) & 0x07);
	color->i.b = ((c << 2) & 0xf8) | ((c >>  3) & 0x07);
	color->i.a = (c & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_CI4_TLUT_EN1(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT8 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	UINT16 c = tlut[((tpal << 4) | p) << 2];

	color->i.r = color->i.g = color->i.b = (c >> 8) & 0xff;
	color->i.a = c & 0xff;
}

INLINE void FETCH_TEXEL_CI4_TLUT_NEN(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT8 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);

	color->i.r = color->i.g = color->i.b = color->i.a = (tpal << 4) | p;
}

INLINE void FETCH_TEXEL_CI8_TLUT_EN0(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT8 p = TMEM[taddr ^ BYTE_ADDR_XOR];
	UINT16 c = tlut[p << 2];

	color->i.r = ((c >> 8) & 0xf8) | (c >> 13);
	color->i.g = ((c >> 3) & 0xf8) | ((c >>  8) & 0x07);
	color->i.b = ((c << 2) & 0xf8) | ((c >>  3) & 0x07);
	color->i.a = (c & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_CI8_TLUT_EN1(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT8 p = TMEM[taddr ^ BYTE_ADDR_XOR];
	UINT16 c = tlut[p << 2];

	color->i.r = color->i.g = color->i.b = (c >> 8) & 0xff;
	color->i.a = c & 0xff;
	color->c = 0xffffffff;
}

INLINE void FETCH_TEXEL_CI8_TLUT_NEN(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
	UINT8 p = TMEM[taddr ^ BYTE_ADDR_XOR];

	color->i.r = color->i.g = color->i.b = color->i.a = p;
}

INLINE void FETCH_TEXEL_CI16_TLUT_EN0(COLOR *color, int s, int t)
{	// 16-bit CI is a "valid" mode; some games use it, it behaves the same as 16-bit RGBA
	int taddr = ((tbase>>1) + ((t) * (twidth>>1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	UINT16 c = TMEM16[(taddr & 0x7ff) ^ WORD_ADDR_XOR]; // PGA European Tour (U)

	c = tlut[(c >> 8) << 2];
	color->i.r = ((c >> 8) & 0xf8) | (c >> 13);
	color->i.g = ((c >> 3) & 0xf8) | ((c >>  8) & 0x07);
	color->i.b = ((c << 2) & 0xf8) | ((c >>  3) & 0x07);
	color->i.a = (c & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_CI16_TLUT_EN1(COLOR *color, int s, int t)
{	// 16-bit CI is a "valid" mode; some games use it, it behaves the same as 16-bit RGBA
	int taddr = ((tbase>>1) + ((t) * (twidth>>1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	UINT16 c = TMEM16[(taddr & 0x7ff) ^ WORD_ADDR_XOR]; // PGA European Tour (U)

	// Beetle Adventure Racing, Mount Mayhem
	c = tlut[(c >> 8) << 2];
	color->i.r = color->i.g = color->i.b = (c >> 8) & 0xff;
	color->i.a = c & 0xff;
}

INLINE void FETCH_TEXEL_CI16_TLUT_NEN(COLOR *color, int s, int t)
{	// 16-bit CI is a "valid" mode; some games use it, it behaves the same as 16-bit RGBA
	int taddr = ((tbase>>1) + ((t) * (twidth>>1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	UINT16 c = TMEM16[(taddr & 0x7ff) ^ WORD_ADDR_XOR]; // PGA European Tour (U)

	color->i.r = ((c >> 8) & 0xf8) | (c >> 13);
	color->i.g = ((c >> 3) & 0xf8) | ((c >>  8) & 0x07);
	color->i.b = ((c << 2) & 0xf8) | ((c >>  3) & 0x07);
	color->i.a = (c & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_IA4_TLUT_EN0(COLOR *color, int s, int t)
{
	int taddr = (tbase + ((t) * twidth) + (s >> 1)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0);
	UINT8 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	UINT16 c = tlut[((tpal << 4) | p) << 2];

	color->i.r = ((c >> 8) & 0xf8) | (c >> 13);
	color->i.g = ((c >> 3) & 0xf8) | ((c >>  8) & 0x07);
	color->i.b = ((c << 2) & 0xf8) | ((c >>  3) & 0x07);
	color->i.a = (c & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_IA4_TLUT_EN1(COLOR *color, int s, int t)
{
	int taddr = (tbase + ((t) * twidth) + (s >> 1)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0);
	UINT8 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	UINT16 c = tlut[((tpal << 4) | p) << 2];

	color->i.r = color->i.g = color->i.b = (c >> 8) & 0xff;
	color->i.a = c & 0xff;
}

INLINE void FETCH_TEXEL_IA4_TLUT_NEN(COLOR *color, int s, int t)
{
	int taddr = (tbase + ((t) * twidth) + (s >> 1)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0);
	UINT8 p = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	UINT8 i = ((p & 0xe) << 4) | ((p & 0xe) << 1) | (p & 0xe >> 2);

	color->i.r = i;
	color->i.g = i;
	color->i.b = i;
	color->i.a = (p & 0x1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_IA8_TLUT_EN0(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	UINT8 p = TMEM[taddr ^ BYTE_ADDR_XOR];
	UINT16 c = tlut[p << 2];

	color->i.r = ((c >> 8) & 0xf8) | (c >> 13);
	color->i.g = ((c >> 3) & 0xf8) | ((c >>  8) & 0x07);
	color->i.b = ((c << 2) & 0xf8) | ((c >>  3) & 0x07);
	color->i.a = (c & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_IA8_TLUT_EN1(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	UINT8 p = TMEM[taddr ^ BYTE_ADDR_XOR];
	UINT16 c = tlut[p << 2];

	color->i.r = color->i.g = color->i.b = (c >> 8) & 0xff;
	color->i.a = c & 0xff;
}

INLINE void FETCH_TEXEL_IA8_TLUT_NEN(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	UINT8 p = TMEM[taddr ^ BYTE_ADDR_XOR];
	UINT8 i = (p >> 4) | (p & 0xf0);

	color->i.r = i;
	color->i.g = i;
	color->i.b = i;
	color->i.a = (p & 0xf) | ((p << 4) & 0xf0);
}

INLINE void FETCH_TEXEL_IA16_TLUT_EN0(COLOR *color, int s, int t)
{
	int taddr = ((tbase >> 1) + ((t) * (twidth >> 1)) + (s)) ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	UINT16 c = TMEM16[taddr ^ WORD_ADDR_XOR];

	c = tlut[(c >> 8) << 2];
	color->i.r = ((c >> 8) & 0xf8) | (c >> 13);
	color->i.g = ((c >> 3) & 0xf8) | ((c >>  8) & 0x07);
	color->i.b = ((c << 2) & 0xf8) | ((c >>  3) & 0x07);
	color->i.a = (c & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_IA16_TLUT_EN1(COLOR *color, int s, int t)
{
	int taddr = ((tbase >> 1) + ((t) * (twidth >> 1)) + (s)) ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	UINT16 c = TMEM16[taddr ^ WORD_ADDR_XOR];

	c = tlut[(c >> 8) << 2];
	color->i.r = c >> 8;
	color->i.g = c >> 8;
	color->i.b = c >> 8;
	color->i.a = c & 0xff;
}

INLINE void FETCH_TEXEL_IA16_TLUT_NEN(COLOR *color, int s, int t)
{
	int taddr = ((tbase >> 1) + ((t) * (twidth >> 1)) + (s)) ^ ((t & 1) ? XOR_SWAP_WORD : 0);
	UINT16 c = TMEM16[taddr ^ WORD_ADDR_XOR];
	UINT8 i = (c >> 8);

	color->i.r = i;
	color->i.g = i;
	color->i.b = i;
	color->i.a = c & 0xff;
}

INLINE void FETCH_TEXEL_I4_TLUT_EN0(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	UINT8 c = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	UINT16 k;
	c |= (c << 4);
	k = tlut[((tpal << 4) | c) << 2];

	color->i.r = ((k >> 8) & 0xf8) | (k >> 13);
	color->i.g = ((k >> 3) & 0xf8) | ((k >>  8) & 0x07);
	color->i.b = ((k << 2) & 0xf8) | ((k >>  3) & 0x07);
	color->i.a = (k & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_I4_TLUT_EN1(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	UINT8 c = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	UINT16 k;

	c |= (c << 4);
	k = tlut[((tpal << 4) | c) << 2];
	color->i.r = color->i.g = color->i.b = (k >> 8) & 0xff;
	color->i.a = k & 0xff;
}

INLINE void FETCH_TEXEL_I4_TLUT_NEN(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	UINT8 c = ((s) & 1) ? (TMEM[taddr ^ BYTE_ADDR_XOR] & 0xf) : (TMEM[taddr ^ BYTE_ADDR_XOR] >> 4);
	c |= (c << 4);

	color->i.r = c;
	color->i.g = c;
	color->i.b = c;
	color->i.a = c;
}

INLINE void FETCH_TEXEL_I8_TLUT_EN0(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	UINT8 c = TMEM[taddr ^ BYTE_ADDR_XOR];
	UINT16 k = tlut[ c << 2];

	color->i.r = ((k >> 8) & 0xf8) | (k >> 13);
	color->i.g = ((k >> 3) & 0xf8) | ((k >>  8) & 0x07);
	color->i.b = ((k << 2) & 0xf8) | ((k >>  3) & 0x07);
	color->i.a = (k & 1) ? 0xff : 0;
}

INLINE void FETCH_TEXEL_I8_TLUT_EN1(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	UINT8 c = TMEM[taddr ^ BYTE_ADDR_XOR];
	UINT16 k = tlut[ c << 2];

	color->i.r = color->i.g = color->i.b = (k >> 8) & 0xff;
	color->i.a = k & 0xff;
}

INLINE void FETCH_TEXEL_I8_TLUT_NEN(COLOR *color, int s, int t)
{
	int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
	UINT8 c = TMEM[taddr ^ BYTE_ADDR_XOR];

	color->i.r = c;
	color->i.g = c;
	color->i.b = c;
	color->i.a = c;
}

INLINE void FETCH_TEXEL_INVALID(COLOR *color, int s, int t)
{
	//printf("Invalid texel mode\n");
}

//typedef void (*rdp_fetch_texel_func)(COLOR *color, int s, int t);

static void (*rdp_fetch_texel_func[128])(COLOR*, int, int) =
{
	// 4-bit accessors
	FETCH_TEXEL_RGBA4_TLUT_NEN, FETCH_TEXEL_RGBA4_TLUT_NEN, FETCH_TEXEL_RGBA4_TLUT_EN0, FETCH_TEXEL_RGBA4_TLUT_EN1,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_CI4_TLUT_NEN, 	FETCH_TEXEL_CI4_TLUT_NEN, 	FETCH_TEXEL_CI4_TLUT_EN0, 	FETCH_TEXEL_CI4_TLUT_EN1,
	FETCH_TEXEL_IA4_TLUT_NEN,	FETCH_TEXEL_IA4_TLUT_NEN,	FETCH_TEXEL_IA4_TLUT_EN0,	FETCH_TEXEL_IA4_TLUT_EN1,
	FETCH_TEXEL_I4_TLUT_NEN,	FETCH_TEXEL_I4_TLUT_NEN,	FETCH_TEXEL_I4_TLUT_EN0,	FETCH_TEXEL_I4_TLUT_EN1,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,

	// 8-bit accessors
	FETCH_TEXEL_RGBA8_TLUT_NEN, FETCH_TEXEL_RGBA8_TLUT_NEN, FETCH_TEXEL_RGBA8_TLUT_EN0, FETCH_TEXEL_RGBA8_TLUT_EN1,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_CI8_TLUT_NEN, 	FETCH_TEXEL_CI8_TLUT_NEN, 	FETCH_TEXEL_CI8_TLUT_EN0, 	FETCH_TEXEL_CI8_TLUT_EN1,
	FETCH_TEXEL_IA8_TLUT_NEN,	FETCH_TEXEL_IA8_TLUT_NEN,	FETCH_TEXEL_IA8_TLUT_EN0,	FETCH_TEXEL_IA8_TLUT_EN1,
	FETCH_TEXEL_I8_TLUT_NEN,	FETCH_TEXEL_I8_TLUT_NEN,	FETCH_TEXEL_I8_TLUT_EN0,	FETCH_TEXEL_I8_TLUT_EN1,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,

	// 16-bit accessors
	FETCH_TEXEL_RGBA16_TLUT_NEN,FETCH_TEXEL_RGBA16_TLUT_NEN,FETCH_TEXEL_RGBA16_TLUT_EN0,FETCH_TEXEL_RGBA16_TLUT_EN1,
	FETCH_TEXEL_YUV16, 			FETCH_TEXEL_YUV16, 			FETCH_TEXEL_YUV16, 			FETCH_TEXEL_YUV16,
	FETCH_TEXEL_CI16_TLUT_NEN, 	FETCH_TEXEL_CI16_TLUT_NEN, 	FETCH_TEXEL_CI16_TLUT_EN0, 	FETCH_TEXEL_CI16_TLUT_EN1,
	FETCH_TEXEL_IA16_TLUT_NEN,	FETCH_TEXEL_IA16_TLUT_NEN,	FETCH_TEXEL_IA16_TLUT_EN0,	FETCH_TEXEL_IA16_TLUT_EN1,
	FETCH_TEXEL_INVALID,		FETCH_TEXEL_INVALID,		FETCH_TEXEL_INVALID,		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,

	// 32-bit accessors
	FETCH_TEXEL_RGBA32_TLUT_NEN,FETCH_TEXEL_RGBA32_TLUT_NEN,FETCH_TEXEL_RGBA32_TLUT_EN0,FETCH_TEXEL_RGBA32_TLUT_EN1,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_INVALID,		FETCH_TEXEL_INVALID,		FETCH_TEXEL_INVALID,		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_INVALID,		FETCH_TEXEL_INVALID,		FETCH_TEXEL_INVALID,		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,
	FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID, 		FETCH_TEXEL_INVALID,
};
