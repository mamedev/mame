/**
 * video hardware for Namco System22
 *
 * todo:
 *
 * - fog bugfixes
 *
 * - SPOT
 *
 * - sprite
 *          xy offset
 *          clipping to window
 *          eliminate garbage (air combat)
 *
 *
 *******************************
czram[0] = 1fff 1fdf 1fbf 1f9f 1f7f 1f5f 1f3f 1f1f 1eff 1edf 1ebf 1e9f 1e7f 1e5f 1e3f 1e1f 1dff 1ddf 1dbf 1d9f 1d7f 1d5f 1d3f 1d1f 1cff 1cdf 1cbf 1c9f 1c7f 1c5f 1c3f 1c1f 1bff 1bdf 1bbf 1b9f 1b7f 1b5f 1b3f 1b1f 1aff 1adf 1abf 1a9f 1a7f 1a5f 1a3f 1a1f 19ff 19df 19bf 199f 197f 195f 193f 191f 18ff 18df 18bf 189f 187f 185f 183f 181f 17ff 17df 17bf 179f 177f 175f 173f 171f 16ff 16df 16bf 169f 167f 165f 163f 161f 15ff 15df 15bf 159f 157f 155f 153f 151f 14ff 14df 14bf 149f 147f 145f 143f 141f 13ff 13df 13bf 139f 137f 135f 133f 131f 12ff 12df 12bf 129f 127f 125f 123f 121f 11ff 11df 11bf 119f 117f 115f 113f 111f 10ff 10df 10bf 109f 107f 105f 103f 101f 0fff 0fdf 0fbf 0f9f 0f7f 0f5f 0f3f 0f1f 0eff 0edf 0ebf 0e9f 0e7f 0e5f 0e3f 0e1f 0dff 0ddf 0dbf 0d9f 0d7f 0d5f 0d3f 0d1f 0cff 0cdf 0cbf 0c9f 0c7f 0c5f 0c3f 0c1f 0bff 0bdf 0bbf 0b9f 0b7f 0b5f 0b3f 0b1f 0aff 0adf 0abf 0a9f 0a7f 0a5f 0a3f 0a1f 09ff 09df 09bf 099f 097f 095f 093f 091f 08ff 08df 08bf 089f 087f 085f 083f 081f 07ff 07df 07bf 079f 077f 075f 073f 071f 06ff 06df 06bf 069f 067f 065f 063f 061f 05ff 05df 05bf 059f 057f 055f 053f 051f 04ff 04df 04bf 049f 047f 045f 043f 041f 03ff 03df 03bf 039f 037f 035f 033f 031f 02ff 02df 02bf 029f 027f 025f 023f 021f 01ff 01df 01bf 019f 017f 015f 013f 011f 00ff 00df 00bf 009f 007f 005f 003f 001f
czram[1] = 0000 0000 0000 0001 0002 0003 0005 0007 0009 000b 000e 0011 0014 0017 001b 001f 0023 0027 002c 0031 0036 003b 0041 0047 004d 0053 005a 0061 0068 006f 0077 007f 0087 008f 0098 00a1 00aa 00b3 00bd 00c7 00d1 00db 00e6 00f1 00fc 0107 0113 011f 012b 0137 0144 0151 015e 016b 0179 0187 0195 01a3 01b2 01c1 01d0 01df 01ef 01ff 020f 021f 0230 0241 0252 0263 0275 0287 0299 02ab 02be 02d1 02e4 02f7 030b 031f 0333 0347 035c 0371 0386 039b 03b1 03c7 03dd 03f3 040a 0421 0438 044f 0467 047f 0497 04af 04c8 04e1 04fa 0513 052d 0547 0561 057b 0596 05b1 05cc 05e7 0603 061f 063b 0657 0674 0691 06ae 06cb 06e9 0707 0725 0743 0762 0781 07a0 07bf 07df 07ff 081f 083f 0860 0881 08a2 08c3 08e5 0907 0929 094b 096e 0991 09b4 09d7 09fb 0a1f 0a43 0a67 0a8c 0ab1 0ad6 0afb 0b21 0b47 0b6d 0b93 0bba 0be1 0c08 0c2f 0c57 0c7f 0ca7 0ccf 0cf8 0d21 0d4a 0d73 0d9d 0dc7 0df1 0e1b 0e46 0e71 0e9c 0ec7 0ef3 0f1f 0f4b 0f77 0fa4 0fd1 0ffe 102b 1059 1087 10b5 10e3 1112 1141 1170 119f 11cf 11ff 122f 125f 1290 12c1 12f2 1323 1355 1387 13b9 13eb 141e 1451 1484 14b7 14eb 151f 1553 1587 15bc 15f1 1626 165b 1691 16c7 16fd 1733 176a 17a1 17d8 180f 1847 187f 18b7 18ef 1928 1961 199a 19d3 1a0d 1a47 1a81 1abb 1af6 1b31 1b6c 1ba7 1be3 1c1f 1c5b 1c97 1cd4 1d11 1d4e 1d8b 1dc9 1e07 1e45 1e83 1ec2 1f01 1f40 1f7f 1fbf 1fff
czram[2] = 003f 007f 00be 00fd 013c 017b 01b9 01f7 0235 0273 02b0 02ed 032a 0367 03a3 03df 041b 0457 0492 04cd 0508 0543 057d 05b7 05f1 062b 0664 069d 06d6 070f 0747 077f 07b7 07ef 0826 085d 0894 08cb 0901 0937 096d 09a3 09d8 0a0d 0a42 0a77 0aab 0adf 0b13 0b47 0b7a 0bad 0be0 0c13 0c45 0c77 0ca9 0cdb 0d0c 0d3d 0d6e 0d9f 0dcf 0dff 0e2f 0e5f 0e8e 0ebd 0eec 0f1b 0f49 0f77 0fa5 0fd3 1000 102d 105a 1087 10b3 10df 110b 1137 1162 118d 11b8 11e3 120d 1237 1261 128b 12b4 12dd 1306 132f 1357 137f 13a7 13cf 13f6 141d 1444 146b 1491 14b7 14dd 1503 1528 154d 1572 1597 15bb 15df 1603 1627 164a 166d 1690 16b3 16d5 16f7 1719 173b 175c 177d 179e 17bf 17df 17ff 181f 183f 185e 187d 189c 18bb 18d9 18f7 1915 1933 1950 196d 198a 19a7 19c3 19df 19fb 1a17 1a32 1a4d 1a68 1a83 1a9d 1ab7 1ad1 1aeb 1b04 1b1d 1b36 1b4f 1b67 1b7f 1b97 1baf 1bc6 1bdd 1bf4 1c0b 1c21 1c37 1c4d 1c63 1c78 1c8d 1ca2 1cb7 1ccb 1cdf 1cf3 1d07 1d1a 1d2d 1d40 1d53 1d65 1d77 1d89 1d9b 1dac 1dbd 1dce 1ddf 1def 1dff 1e0f 1e1f 1e2e 1e3d 1e4c 1e5b 1e69 1e77 1e85 1e93 1ea0 1ead 1eba 1ec7 1ed3 1edf 1eeb 1ef7 1f02 1f0d 1f18 1f23 1f2d 1f37 1f41 1f4b 1f54 1f5d 1f66 1f6f 1f77 1f7f 1f87 1f8f 1f96 1f9d 1fa4 1fab 1fb1 1fb7 1fbd 1fc3 1fc8 1fcd 1fd2 1fd7 1fdb 1fdf 1fe3 1fe7 1fea 1fed 1ff0 1ff3 1ff5 1ff7 1ff9 1ffb 1ffc 1ffd 1ffe 1fff 1fff 1fff
czram[3] = 0000 001f 003f 005f 007f 009f 00bf 00df 00ff 011f 013f 015f 017f 019f 01bf 01df 01ff 021f 023f 025f 027f 029f 02bf 02df 02ff 031f 033f 035f 037f 039f 03bf 03df 03ff 041f 043f 045f 047f 049f 04bf 04df 04ff 051f 053f 055f 057f 059f 05bf 05df 05ff 061f 063f 065f 067f 069f 06bf 06df 06ff 071f 073f 075f 077f 079f 07bf 07df 07ff 081f 083f 085f 087f 089f 08bf 08df 08ff 091f 093f 095f 097f 099f 09bf 09df 09ff 0a1f 0a3f 0a5f 0a7f 0a9f 0abf 0adf 0aff 0b1f 0b3f 0b5f 0b7f 0b9f 0bbf 0bdf 0bff 0c1f 0c3f 0c5f 0c7f 0c9f 0cbf 0cdf 0cff 0d1f 0d3f 0d5f 0d7f 0d9f 0dbf 0ddf 0dff 0e1f 0e3f 0e5f 0e7f 0e9f 0ebf 0edf 0eff 0f1f 0f3f 0f5f 0f7f 0f9f 0fbf 0fdf 0fff 101f 103f 105f 107f 109f 10bf 10df 10ff 111f 113f 115f 117f 119f 11bf 11df 11ff 121f 123f 125f 127f 129f 12bf 12df 12ff 131f 133f 135f 137f 139f 13bf 13df 13ff 141f 143f 145f 147f 149f 14bf 14df 14ff 151f 153f 155f 157f 159f 15bf 15df 15ff 161f 163f 165f 167f 169f 16bf 16df 16ff 171f 173f 175f 177f 179f 17bf 17df 17ff 181f 183f 185f 187f 189f 18bf 18df 18ff 191f 193f 195f 197f 199f 19bf 19df 19ff 1a1f 1a3f 1a5f 1a7f 1a9f 1abf 1adf 1aff 1b1f 1b3f 1b5f 1b7f 1b9f 1bbf 1bdf 1bff 1c1f 1c3f 1c5f 1c7f 1c9f 1cbf 1cdf 1cff 1d1f 1d3f 1d5f 1d7f 1d9f 1dbf 1ddf 1dff 1e1f 1e3f 1e5f 1e7f 1e9f 1ebf 1edf 1eff 1f1f 1f3f 1f5f 1f7f 1f9f 1fbf 1fdf

CZ (NORMAL) 00810000: 00000000 00000000 75550000 00e40000
CZ (OFFSET) 00810000: 7fff8000 7fff8000 75550000 00e40000
CZ (OFF)    00810000: 00000000 00000000 31110000 00e40000

SPOT TABLE test
03F282: 13FC 0000 0082 4011        move.b  #$0, $824011.l
03F28A: 13FC 0000 0082 4015        move.b  #$0, $824015.l
03F292: 13FC 0080 0082 400D        move.b  #$80, $82400d.l
03F29A: 13FC 0001 0082 400E        move.b  #$1, $82400e.l
03F2A2: 13FC 0001 0082 4021        move.b  #$1, $824021.l
03F2AA: 33FC 4038 0080 0000        move.w  #$4038, $800000.l
03F2B2: 06B9 0000 0001 00E0 AB08   addi.l  #$1, $e0ab08.l
*/

#include "driver.h"
#include "eminline.h"
#include "video/rgbutil.h"
#include "includes/namcos22.h"
#include "video/poly.h"

// uncomment this line to render everything as quads
//#define RENDER_AS_QUADS

static int mbSuperSystem22; /* used to conditionally support Super System22-specific features */
static int mbSpotlightEnable;
static UINT16 *namcos22_czram[4];

static poly_manager *poly;

static UINT8
nthbyte( const UINT32 *pSource, int offs )
{
	pSource += offs/4;
	return (pSource[0]<<((offs&3)*8))>>24;
}

static UINT16
nthword( const UINT32 *pSource, int offs )
{
	pSource += offs/2;
	return (pSource[0]<<((offs&1)*16))>>16;
}

static struct
{
	int target;
	int rFogColor;
	int gFogColor;
	int bFogColor;
	int rFogColor2;
	int gFogColor2;
	int bFogColor2;
	int rBackColor;
	int gBackColor;
	int bBackColor;
	int rFadeColor;
	int gFadeColor;
	int bFadeColor;
	int fadeFactor;
	int spot_translucency;
	int poly_translucency;
	int text_translucency;
	int palBase;
} mixer;

static void
UpdateVideoMixer( void )
{
	poly_wait(poly, "UpdateVideoMixer");
	memset( &mixer, 0, sizeof(mixer) );
	if( mbSuperSystem22 )
	{
/*
           0 1 2 3  4 5 6 7  8 9 a b  c d e f 10       14       18       1c
00824000: ffffff00 00000000 0000007f 00ff0000 1000ff00 0f000000 00ff007f 00010007 // time crisis
00824000: ffffff00 00000000 1830407f 00800000 0000007f 0f000000 0000037f 00010007 // trans sprite
00824000: ffffff00 00000000 3040307f 00000000 0080007f 0f000000 0000037f 00010007 // trans poly
00824000: ffffff00 00000000 1800187f 00800000 0080007f 0f000000 0000037f 00010007 // trans poly(2)
00824000: ffffff00 00000000 1800187f 00000000 0000007f 0f800000 0000037f 00010007 // trans text
*/
		mixer.rFogColor         = nthbyte( namcos22_gamma, 0x05 );
		mixer.gFogColor         = nthbyte( namcos22_gamma, 0x06 );
		mixer.bFogColor         = nthbyte( namcos22_gamma, 0x07 );
		mixer.rBackColor        = nthbyte( namcos22_gamma, 0x08 );
		mixer.gBackColor        = nthbyte( namcos22_gamma, 0x09 );
		mixer.bBackColor        = nthbyte( namcos22_gamma, 0x0a );
		mixer.spot_translucency = nthbyte( namcos22_gamma, 0x0d );
		mixer.poly_translucency = nthbyte( namcos22_gamma, 0x11 );
		mixer.text_translucency = nthbyte( namcos22_gamma, 0x15 );
		mixer.rFadeColor        = nthbyte( namcos22_gamma, 0x16 );
		mixer.gFadeColor        = nthbyte( namcos22_gamma, 0x17 );
		mixer.bFadeColor        = nthbyte( namcos22_gamma, 0x18 );
		mixer.fadeFactor        = nthbyte( namcos22_gamma, 0x19 );
		mixer.target            = nthbyte( namcos22_gamma, 0x1a );
		mixer.palBase           = nthbyte( namcos22_gamma, 0x1b );
	}
	else
	{
/*
90020000: 4f030000 7f00007f 4d4d4d42 0c00c0c0
90020010: c0010001 00010000 00000000 00000000
90020080: 00010101 01010102 00000000 00000000
900200c0: 00000000 00000000 00000000 03000000
90020100: fff35000 00000000 00000000 00000000
90020180: ff713700 00000000 00000000 00000000
90020200: ff100000 00000000 00000000 00000000
*/
		mixer.palBase     = 0x7f;
		mixer.target      = 0x7;//nthbyte( namcos22_gamma, 0x0002 )*256 + nthbyte( namcos22_gamma, 0x0003 );
		mixer.rFadeColor  = nthbyte( namcos22_gamma, 0x0011 )*256 + nthbyte( namcos22_gamma, 0x0012 );
		mixer.gFadeColor  = nthbyte( namcos22_gamma, 0x0013 )*256 + nthbyte( namcos22_gamma, 0x0014 );
		mixer.bFadeColor  = nthbyte( namcos22_gamma, 0x0015 )*256 + nthbyte( namcos22_gamma, 0x0016 );

		mixer.fadeFactor  = 0x100 - mixer.rFadeColor; // hack
		mixer.rFadeColor  = 0;
		mixer.gFadeColor  = 0;
		mixer.bFadeColor  = 0;

		mixer.rFogColor   = nthbyte( namcos22_gamma, 0x0100 );
		mixer.rFogColor2  = nthbyte( namcos22_gamma, 0x0101 );
		mixer.gFogColor   = nthbyte( namcos22_gamma, 0x0180 );
		mixer.gFogColor2  = nthbyte( namcos22_gamma, 0x0181 );
		mixer.bFogColor   = nthbyte( namcos22_gamma, 0x0200 );
		mixer.bFogColor2  = nthbyte( namcos22_gamma, 0x0201 );

/*  +0x0002.w   Fader Enable(?) (0: disabled)
 *  +0x0011.w   Display Fader (R) (0x0100 = 1.0)
 *  +0x0013.w   Display Fader (G) (0x0100 = 1.0)
 *  +0x0015.w   Display Fader (B) (0x0100 = 1.0)
 *  +0x0100.b   Fog1 Color (R) (world fogging)
 *  +0x0101.b   Fog2 Color (R) (used for heating of brake-disc on RV1)
 *  +0x0180.b   Fog1 Color (G)
 *  +0x0181.b   Fog2 Color (G)
 *  +0x0200.b   Fog1 Color (B)
 *  +0x0201.b   Fog2 Color (B)
 */
	}
}

INLINE UINT8
Clamp256( int v )
{
   if( v<0 )
   {
      v = 0;
   }
   else if( v>255 )
   {
      v = 255;
   }
   return v;
} /* Clamp256 */

#ifdef MAME_DEBUG
static void Dump( const address_space *space, FILE *f, unsigned addr1, unsigned addr2, const char *name );
#endif

static struct
{
   int cx,cy;
   rectangle scissor;
} mClip;

static void
poly3d_Clip( float vx, float vy, float vw, float vh )
{
   int cx = 320+vx;
   int cy = 240+vy;
   mClip.cx = cx;
   mClip.cy = cy;
   mClip.scissor.min_x = cx + vw;
   mClip.scissor.min_y = cy + vh;
   mClip.scissor.max_x = cx - vw;
   mClip.scissor.max_y = cy - vh;
   if( mClip.scissor.min_x<0 )   mClip.scissor.min_x = 0;
   if( mClip.scissor.min_y<0 )   mClip.scissor.min_y = 0;
   if( mClip.scissor.max_x>639 ) mClip.scissor.max_x = 639;
   if( mClip.scissor.max_y>479 ) mClip.scissor.max_y = 479;
}

static void
poly3d_NoClip( void )
{
   mClip.cx = 640/2;
   mClip.cy = 480/2;
   mClip.scissor.min_x = 0;
   mClip.scissor.max_x = 639;
   mClip.scissor.min_y = 0;
   mClip.scissor.max_x = 479;
}

typedef struct
{
   float x,y,z;
   int u,v; /* 0..0xfff */
   int bri; /* 0..0xff */
} Poly3dVertex;

#define MIN_Z (10.0f)

typedef struct
{
	float x,y;
	float u,v,i,z;
} vertex;

typedef struct
{
	float x;
	float u,v,i,z;
} edge;

#define SWAP(A,B) { const void *temp = A; A = B; B = temp; }

static UINT16 *mpTextureTileMap16;
static UINT8 *mpTextureTileMapAttr;
static UINT8 *mpTextureTileData;
static UINT8 mXYAttrToPixel[16][16][16];

INLINE unsigned texel( unsigned x, unsigned y )
{
	unsigned offs = ((y&0xfff0)<<4)|((x&0xff0)>>4);
	unsigned tile = mpTextureTileMap16[offs];
	return mpTextureTileData[(tile<<8)|mXYAttrToPixel[mpTextureTileMapAttr[offs]][x&0xf][y&0xf]];
} /* texel */



typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	rgbint fogColor;
	rgbint fadeColor;
	const pen_t *pens;
	bitmap_t *priority_bitmap;
	int bn;
	UINT16 flags;
	int cmode;
	int fogFactor;
	int fadeFactor;
	const UINT8 *source;		/* sprites */
	int z;
	int alpha;
	int prioverchar;
	int line_modulo;
};


static void renderscanline_uvi_full(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	float z = extent->param[0].start;
	float u = extent->param[1].start;
	float v = extent->param[2].start;
	float i = extent->param[3].start;
	float dz = extent->param[0].dpdx;
	float du = extent->param[1].dpdx;
	float dv = extent->param[2].dpdx;
	float di = extent->param[3].dpdx;
	bitmap_t *bitmap = (bitmap_t *)dest;
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	int bn = extra->bn * 0x1000;
	const pen_t *pens = extra->pens;
	int fogFactor = 0xff - extra->fogFactor;
	int fadeFactor = 0xff - extra->fadeFactor;
	int transFactor = 0xff;
	rgbint fogColor = extra->fogColor;
	rgbint fadeColor = extra->fadeColor;
	int penmask, penshift;
	int prioverchar;
	const UINT8 *pCharPri = BITMAP_ADDR8(extra->priority_bitmap, scanline, 0);
	UINT32 *pDest = BITMAP_ADDR32(bitmap, scanline, 0);
	int x;

	if (extra->cmode & 4)
	{
		prioverchar = 0;
		pens += 0xec + ((extra->cmode & 8) << 1);
		penmask = 0x03;
		penshift = 2 * ~(extra->cmode & 3);
	}
	else if (extra->cmode & 2)
	{
		prioverchar = 0;
		pens += 0xe0 + ((extra->cmode & 8) << 1);
		penmask = 0x0f;
		penshift = 4 * ~(extra->cmode & 1);
	}
	else if (extra->cmode & 1)
	{
		transFactor = 0xff - mixer.poly_translucency;
		prioverchar = 1;
		penmask = 0xff;
		penshift = 0;
	}
	else
	{
		prioverchar = 0;
		penmask = 0xff;
		penshift = 0;
	}

	if (prioverchar)
	{
		for( x=extent->startx; x<extent->stopx; x++ )
		{
			float ooz = 1.0f / z;
			int pen = texel((int)(u * ooz),bn+(int)(v*ooz));
			int shade = i*ooz;
			rgbint rgb;

			rgb_to_rgbint(&rgb, pens[(pen >> penshift) & penmask]);
			rgbint_scale_and_clamp(&rgb, shade << 2);

			if( fogFactor != 0xff )
				rgbint_blend(&rgb, &fogColor, fogFactor);

			if( fadeFactor != 0xff )
				rgbint_blend(&rgb, &fadeColor, fadeFactor);

			if( transFactor != 0xff )
			{
				rgbint dest;
				rgb_to_rgbint(&dest, pDest[x]);
				rgbint_blend(&rgb, &dest, transFactor);
			}

			pDest[x] = rgbint_to_rgb(&rgb);

			u += du;
			v += dv;
			i += di;
			z += dz;
		}
	}
	else
	{
		for( x=extent->startx; x<extent->stopx; x++ )
		{
			if( pCharPri[x]==0 )
			{
				float ooz = 1.0f / z;
				int pen = texel((int)(u * ooz),bn+(int)(v*ooz));
				int shade = i*ooz;
				rgbint rgb;

				rgb_to_rgbint(&rgb, pens[(pen >> penshift) & penmask]);
				rgbint_scale_and_clamp(&rgb, shade << 2);

				if( fogFactor != 0xff )
					rgbint_blend(&rgb, &fogColor, fogFactor);

				if( fadeFactor != 0xff )
					rgbint_blend(&rgb, &fadeColor, fadeFactor);

				pDest[x] = rgbint_to_rgb(&rgb);
			}

			u += du;
			v += dv;
			i += di;
			z += dz;
		}
	}
} /* renderscanline_uvi_full */

static void poly3d_DrawQuad(running_machine *machine, bitmap_t *bitmap, int textureBank, int color, Poly3dVertex pv[4], UINT16 flags, int direct, int cmode )
{
	poly_extra_data *extra;
	poly_vertex v[4], clipv[6];
	int clipverts;
	int vertnum;

	/* non-direct case: project and z-clip */
	if (!direct)
	{
		for (vertnum = 0; vertnum < 4; vertnum++)
		{
			v[vertnum].x = pv[vertnum].x;
			v[vertnum].y = pv[vertnum].y;
			v[vertnum].p[0] = pv[vertnum].z;
			v[vertnum].p[1] = pv[vertnum].u;
			v[vertnum].p[2] = pv[vertnum].v;
			v[vertnum].p[3] = pv[vertnum].bri;
		}
		clipverts = poly_zclip_if_less(4, v, clipv, 4, MIN_Z);
		if (clipverts < 3)
			return;
		assert(clipverts <= ARRAY_LENGTH(clipv));
		for (vertnum = 0; vertnum < clipverts; vertnum++)
		{
			float ooz = 1.0f / clipv[vertnum].p[0];
			clipv[vertnum].x = mClip.cx + clipv[vertnum].x * ooz;
			clipv[vertnum].y = mClip.cy - clipv[vertnum].y * ooz;
			clipv[vertnum].p[0] = ooz;
			clipv[vertnum].p[1] = (clipv[vertnum].p[1] + 0.5f) * ooz;
			clipv[vertnum].p[2] = (clipv[vertnum].p[2] + 0.5f) * ooz;
			clipv[vertnum].p[3] = (clipv[vertnum].p[3] + 0.5f) * ooz;
		}
	}

	/* direct case: don't clip, and treat pv->z as 1/z */
	else
	{
		clipverts = 4;
		for (vertnum = 0; vertnum < 4; vertnum++)
		{
			float ooz = pv[vertnum].z;
			clipv[vertnum].x = mClip.cx + pv[vertnum].x;
			clipv[vertnum].y = mClip.cy - pv[vertnum].y;
			clipv[vertnum].p[0] = ooz;
			clipv[vertnum].p[1] = (pv[vertnum].u + 0.5f) * ooz;
			clipv[vertnum].p[2] = (pv[vertnum].v + 0.5f) * ooz;
			clipv[vertnum].p[3] = (pv[vertnum].bri + 0.5f) * ooz;
		}
	}

	extra = (poly_extra_data *)poly_get_extra_data(poly);

	extra->pens = &machine->pens[(color&0x7f)<<8];
	extra->priority_bitmap = machine->priority_bitmap;
	extra->bn = textureBank;
	extra->flags = flags;
	extra->cmode = cmode;
	extra->fogFactor = 0;
	extra->fadeFactor = 0;

	if (mixer.target&1)
	{
		extra->fadeFactor = mixer.fadeFactor;
		rgb_comp_to_rgbint(&extra->fadeColor, mixer.rFadeColor, mixer.gFadeColor, mixer.bFadeColor);
	}

	if( mbSuperSystem22 )
	{
		if( !(color&0x80) )
		{
			int cz = flags>>8;
			static const int cztype_remap[4] = { 3,1,2,0 };
			int cztype = flags&3;
			if( nthword(namcos22_czattr,4)&(0x4000>>(cztype*4)) )
			{
				int fogDelta = (INT16)nthword(namcos22_czattr, cztype);
				int fogDensity = fogDelta + namcos22_czram[cztype_remap[cztype]][cz];
				//          if( fogDelta == 0x8000 ) fogDelta = -0x7fff;
				//cz = Clamp256(cz+fogDelta);
				if( fogDensity<0x0000 )
				{
					fogDensity = 0x0000;
				}
				else if( fogDensity>0x1fff )
				{
					fogDensity = 0x1fff;
				}
				extra->fogFactor = fogDensity >> 5;
				rgb_comp_to_rgbint(&extra->fogColor, mixer.rFogColor, mixer.gFogColor, mixer.bFogColor);
			}
		}
	}

#ifdef RENDER_AS_QUADS
	poly_render_quad_fan(poly, bitmap, &mClip.scissor, renderscanline_uvi_full, 4, clipverts, clipv);
#else
	poly_render_triangle_fan(poly, bitmap, &mClip.scissor, renderscanline_uvi_full, 4, clipverts, clipv);
#endif
}

static void renderscanline_sprite(void *destbase, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	int x_index = extent->param[0].start * 65536.0f;
	int y_index = extent->param[1].start * 65536.0f;
	int dx = extent->param[0].dpdx * 65536.0f;
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_t *destmap = (bitmap_t *)destbase;
	const pen_t *pal = extra->pens;
	int prioverchar = extra->prioverchar;
	int z = extra->z;
	int alpha = extra->alpha;
	UINT8 *source = (UINT8 *)extra->source + (y_index>>16) * extra->line_modulo;
	UINT32 *dest = BITMAP_ADDR32(destmap, scanline, 0);
	const UINT8 *pCharPri = BITMAP_ADDR8(extra->priority_bitmap, scanline, 0);
	int x;

	int bFogEnable = 0;
	INT16 fogDelta = 0;
	int fadeEnable = (mixer.target&2) && mixer.fadeFactor;

	if( mbSuperSystem22 )
	{
		fogDelta = nthword(namcos22_czattr, 0 );
		bFogEnable = nthword(namcos22_czattr,4)&0x4000; /* ? */
	}
	else
	{
		bFogEnable = 0;
	}

	for( x=extent->startx; x<extent->stopx; x++ )
	{
		int pen = source[x_index>>16];
		if( pen != 0xff )
		{
			if( pCharPri[x]==0 || prioverchar )
			{
				UINT32 color = pal[pen];
				int r = color>>16;
				int g = (color>>8)&0xff;
				int b = color&0xff;
				if( bFogEnable && z!=0xffff )
				{
					int zc = Clamp256(fogDelta + z);
					UINT16 fogDensity = namcos22_czram[3][zc];
					if( fogDensity>0 )
					{
						int fogDensity2 = 0x2000 - fogDensity;
						r = (r*fogDensity2 + fogDensity*mixer.rFogColor)>>13;
						g = (g*fogDensity2 + fogDensity*mixer.gFogColor)>>13;
						b = (b*fogDensity2 + fogDensity*mixer.bFogColor)>>13;
					}
				}
				if( fadeEnable )
				{
					int fade2 = 0x100-mixer.fadeFactor;
					r = (r*fade2+mixer.fadeFactor*mixer.rFadeColor)>>8;
					g = (g*fade2+mixer.fadeFactor*mixer.gFadeColor)>>8;
					b = (b*fade2+mixer.fadeFactor*mixer.bFadeColor)>>8;
				}
				color = (r<<16)|(g<<8)|b;
				color = alpha_blend_r32(dest[x], color, alpha);
				color&=0xffffff;
				dest[x] = color;
			}
		}
		x_index += dx;
	}
}


static void
mydrawgfxzoom(
	bitmap_t *dest_bmp,const rectangle *clip,const gfx_element *gfx,
	UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
	int scalex, int scaley, int z, int prioverchar, int alpha )
{
	int sprite_screen_height = (scaley*gfx->height+0x8000)>>16;
	int sprite_screen_width = (scalex*gfx->width+0x8000)>>16;
	if (sprite_screen_width && sprite_screen_height && gfx)
	{
		float fsx = sx;
		float fsy = sy;
		float fwidth = gfx->width;
		float fheight = gfx->height;
		float fsw = sprite_screen_width;
		float fsh = sprite_screen_height;
		poly_extra_data *extra;
		poly_vertex vert[4];

		vert[0].x = fsx;
		vert[0].y = fsy;
		vert[0].p[0] = flipx ? fwidth : 0;
		vert[0].p[1] = flipy ? fheight : 0;
		vert[1].x = fsx + fsw;
		vert[1].y = fsy;
		vert[1].p[0] = flipx ? 0 : fwidth;
		vert[1].p[1] = flipy ? fheight : 0;
		vert[2].x = fsx + fsw;
		vert[2].y = fsy + fsh;
		vert[2].p[0] = flipx ? 0 : fwidth;
		vert[2].p[1] = flipy ? 0 : fheight;
		vert[3].x = fsx;
		vert[3].y = fsy + fsh;
		vert[3].p[0] = flipx ? fwidth : 0;
		vert[3].p[1] = flipy ? 0 : fheight;

		extra = (poly_extra_data *)poly_get_extra_data(poly);
		extra->z = z;
		extra->alpha = alpha;
		extra->prioverchar = prioverchar;
		extra->line_modulo = gfx->line_modulo;
		extra->pens = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
		extra->priority_bitmap = gfx->machine->priority_bitmap;
		extra->source = gfx_element_get_data(gfx, code % gfx->total_elements);
#ifdef RENDER_AS_QUADS
		poly_render_quad_fan(poly, dest_bmp, clip, renderscanline_sprite, 2, 4, &vert[0]);
#else
		poly_render_triangle_fan(poly, dest_bmp, clip, renderscanline_sprite, 2, 4, &vert[0]);
#endif
	}
} /* mydrawgfxzoom */

static void
ApplyGamma( running_machine *machine, bitmap_t *bitmap )
{
	int x,y;
	if( mbSuperSystem22 )
	{ /* super system 22 */
#define XORPAT NATIVE_ENDIAN_VALUE_LE_BE(3,0)
		const UINT8 *rlut = (const UINT8 *)&namcos22_gamma[0x100/4];
		const UINT8 *glut = (const UINT8 *)&namcos22_gamma[0x200/4];
		const UINT8 *blut = (const UINT8 *)&namcos22_gamma[0x300/4];
		for( y=0; y<bitmap->height; y++ )
		{
			UINT32 *dest = BITMAP_ADDR32(bitmap, y, 0);
			for( x=0; x<bitmap->width; x++ )
			{
				int rgb = dest[x];
				int r = rlut[XORPAT^((rgb>>16)&0xff)];
				int g = glut[XORPAT^((rgb>>8)&0xff)];
				int b = blut[XORPAT^(rgb&0xff)];
				dest[x] = (r<<16)|(g<<8)|b;
			}
		}
	}
	else
	{ /* system 22 */
		const UINT8 *rlut = 0x000+(const UINT8 *)memory_region(machine, "user1");
		const UINT8 *glut = 0x100+rlut;
		const UINT8 *blut = 0x200+rlut;
		for( y=0; y<bitmap->height; y++ )
		{
			UINT32 *dest = BITMAP_ADDR32(bitmap, y, 0);
			for( x=0; x<bitmap->width; x++ )
			{
				int rgb = dest[x];
				int r = rlut[(rgb>>16)&0xff];
				int g = glut[(rgb>>8)&0xff];
				int b = blut[rgb&0xff];
				dest[x] = (r<<16)|(g<<8)|b;
			}
		}
	}
} /* ApplyGamma */

static void
poly3d_Draw3dSprite( bitmap_t *bitmap, const gfx_element *gfx, int tileNumber, int color, int sx, int sy, int width, int height, int translucency, int zc, UINT32 pri )
{
   int flipx = 0;
   int flipy = 0;
   rectangle clip;
   clip.min_x = 0;
   clip.min_y = 0;
   clip.max_x = 640-1;
   clip.max_y = 480-1;
   mydrawgfxzoom(
      bitmap,
      &clip,
      gfx,
      tileNumber,
      color,
      flipx, flipy,
      sx, sy,
      (width<<16)/32,
      (height<<16)/32,
      zc, pri, 0xff - translucency );
}

#define DSP_FIXED_TO_FLOAT( X ) (((INT16)(X))/(float)0x7fff)
#define SPRITERAM_SIZE (0x9b0000-0x980000)
#define CGRAM_SIZE 0x1e000
#define NUM_CG_CHARS ((CGRAM_SIZE*8)/(64*16)) /* 0x3c0 */

/* 16 bit access to DSP RAM */
static UINT16 namcos22_dspram_bank;
static UINT16 mUpperWordLatch;

static int mbDSPisActive;

/* modal rendering properties */
static INT32 mAbsolutePriority;
static INT32 mObjectShiftValue22;

static UINT16 mPrimitiveID; /* 3d primitive to render */

static float mViewMatrix[4][4];

static void
matrix3d_Multiply( float A[4][4], float B[4][4] )
{
	float temp[4][4];
	int row,col;

	for( row=0;row<4;row++ )
	{
		for(col=0;col<4;col++)
		{
			float sum = 0.0f;
			int i;
			for( i=0; i<4; i++ )
			{
				sum += A[row][i]*B[i][col];
			}
			temp[row][col] = sum;
		}
	}
	memcpy( A, temp, sizeof(temp) );
} /* matrix3d_Multiply */

static void
matrix3d_Identity( float M[4][4] )
{
	int r,c;

	for( r=0; r<4; r++ )
	{
		for( c=0; c<4; c++ )
		{
			M[r][c] = (r==c)?1.0:0.0;
		}
	}
} /* matrix3d_Identity */

static void
TransformPoint( float *vx, float *vy, float *vz, float m[4][4] )
{
	float x = *vx;
	float y = *vy;
	float z = *vz;
	*vx = m[0][0]*x + m[1][0]*y + m[2][0]*z + m[3][0];
	*vy = m[0][1]*x + m[1][1]*y + m[2][1]*z + m[3][1];
	*vz = m[0][2]*x + m[1][2]*y + m[2][2]*z + m[3][2];
}

static void
TransformNormal( float *nx, float *ny, float *nz, float m[4][4] )
{
	float x = *nx;
	float y = *ny;
	float z = *nz;
	*nx = m[0][0]*x + m[1][0]*y + m[2][0]*z;
	*ny = m[0][1]*x + m[1][1]*y + m[2][1]*z;
	*nz = m[0][2]*x + m[1][2]*y + m[2][2]*z;
}

#define MAX_LIT_SURFACES 32
static UINT8 mLitSurfaceInfo[MAX_LIT_SURFACES];
static INT32 mSurfaceNormalFormat;

static unsigned mLitSurfaceCount;
static unsigned mLitSurfaceIndex;

static int mPtRomSize;
static const UINT8 *mpPolyH;
static const UINT8 *mpPolyM;
static const UINT8 *mpPolyL;

static struct
{
	float zoom, vx, vy, vw, vh;
	float lx,ly,lz; /* unit vector for light direction */
	int ambient; /* 0.0..1.0 */
	int power;	/* 0.0..1.0 */
} mCamera;

typedef enum
{
   eSCENENODE_NONLEAF,
   eSCENENODE_QUAD3D,
   eSCENENODE_SPRITE
} SceneNodeType;

#define RADIX_BITS 4
#define RADIX_BUCKETS (1<<RADIX_BITS)
#define RADIX_MASK (RADIX_BUCKETS-1)

struct SceneNode
{
   SceneNodeType type;
   struct SceneNode *nextInBucket;
   union
   {
      struct
      {
         struct SceneNode *next[RADIX_BUCKETS];
      } nonleaf;

      struct
      {
         float vx,vy,vw,vh;
         int textureBank;
         int color;
         int cmode;
         int flags;
         int direct;
         Poly3dVertex v[4];
      } quad3d;

      struct
      {
         int tile, color, pri;
         int flipx, flipy;
			int linkType;
	      int numcols, numrows;
	      int xpos, ypos;
	      int sizex, sizey;
         int translucency;
         int cz;
      } sprite;
   } data;
};
static struct SceneNode mSceneRoot;
static struct SceneNode *mpFreeSceneNode;

static void
FreeSceneNode( struct SceneNode *node )
{
   node->nextInBucket = mpFreeSceneNode;
   mpFreeSceneNode = node;
} /* FreeSceneNode */

static struct SceneNode *
MallocSceneNode( running_machine *machine )
{
   struct SceneNode *node = mpFreeSceneNode;
   if( node )
   { /* use free pool */
      mpFreeSceneNode = node->nextInBucket;
   }
   else
   {
	  node = alloc_or_die(struct SceneNode);
   }
   memset( node, 0, sizeof(*node) );
   return node;
} /* MallocSceneNode */

static struct SceneNode *
NewSceneNode( running_machine *machine, UINT32 zsortvalue24, SceneNodeType type )
{
   struct SceneNode *node = &mSceneRoot;
   int i;
   for( i=0; i<24; i+=RADIX_BITS )
   {
      int hash = (zsortvalue24>>20)&RADIX_MASK;
      struct SceneNode *next = node->data.nonleaf.next[hash];
      if( !next )
      { /* lazily allocate tree node for this radix */
         next = MallocSceneNode(machine);
         next->type = eSCENENODE_NONLEAF;
         node->data.nonleaf.next[hash] = next;
      }
      node = next;
      zsortvalue24 <<= RADIX_BITS;
   }

   if( node->type == eSCENENODE_NONLEAF )
   { /* first leaf allocation on this branch */
      node->type = type;
      return node;
   }
   else
   {
      struct SceneNode *leaf = MallocSceneNode(machine);
      leaf->type = type;
#if 0
      leaf->nextInBucket = node->nextInBucket;
      node->nextInBucket = leaf;
#else
		/* stable insertion sort */
		leaf->nextInBucket = NULL;
		while( node->nextInBucket )
		{
			node = node->nextInBucket;
		}
		node->nextInBucket = leaf;
#endif
      return leaf;
   }
} /* NewSceneNode */


static void RenderSprite(running_machine *machine, bitmap_t *bitmap, struct SceneNode *node )
{
   int tile = node->data.sprite.tile;
   int col,row;
	int i = 0;
   for( row=0; row<node->data.sprite.numrows; row++ )
   {
      for( col=0; col<node->data.sprite.numcols; col++ )
	{
         int code = tile;
         if( node->data.sprite.linkType == 0xff )
         {
            code += i;
         }
         else
         {
            code += nthword( &machine->generic.spriteram.u32[0x800/4], i+node->data.sprite.linkType*4 );
         }
         poly3d_Draw3dSprite(
               bitmap,
               machine->gfx[GFX_SPRITE],
               code,
               node->data.sprite.color,
               node->data.sprite.xpos+col*node->data.sprite.sizex,
               node->data.sprite.ypos+row*node->data.sprite.sizey,
               node->data.sprite.sizex,
               node->data.sprite.sizey,
               node->data.sprite.translucency,
               node->data.sprite.cz,
               node->data.sprite.pri );
    	i++;
      } /* next col */
   } /* next row */
} /* RenderSprite */

static void RenderSceneHelper(running_machine *machine, bitmap_t *bitmap, struct SceneNode *node )
{
   if( node )
   {
      if( node->type == eSCENENODE_NONLEAF )
      {
         int i;
         for( i=RADIX_BUCKETS-1; i>=0; i-- )
         {
            RenderSceneHelper(machine, bitmap, node->data.nonleaf.next[i] );
         }
         FreeSceneNode( node );
      }
      else
      {
         while( node )
         {
            struct SceneNode *next = node->nextInBucket;

            switch( node->type )
            {
            case eSCENENODE_QUAD3D:
               poly3d_Clip(
                  node->data.quad3d.vx,
                  node->data.quad3d.vy,
                  node->data.quad3d.vw,
                  node->data.quad3d.vh );
               poly3d_DrawQuad(machine,
                  bitmap,
                  node->data.quad3d.textureBank,
                  node->data.quad3d.color,
                  node->data.quad3d.v,
                  node->data.quad3d.flags,
                  node->data.quad3d.direct,
                  node->data.quad3d.cmode );
               break;

            case eSCENENODE_SPRITE:
               poly3d_NoClip();
               RenderSprite(machine, bitmap,node );
               break;

            default:
               fatalerror("invalid node->type");
               break;
            }
            FreeSceneNode( node );
            node = next;
         }
      }
   }
} /* RenderSceneHelper */

static void RenderScene(running_machine *machine, bitmap_t *bitmap )
{
   struct SceneNode *node = &mSceneRoot;
   int i;
   for( i=RADIX_BUCKETS-1; i>=0; i-- )
   {
      RenderSceneHelper(machine, bitmap, node->data.nonleaf.next[i] );
      node->data.nonleaf.next[i] = NULL;
   }
   poly3d_NoClip();
	poly_wait(poly, "DrawPolygons");
} /* RenderScene */

static float
DspFloatToNativeFloat( UINT32 iVal )
{
   INT16 mantissa = (INT16)iVal;
   float result = mantissa;//?((float)mantissa):((float)0x10000);
	int exponent = (iVal>>16)&0xff;
	while( exponent<0x2e )
	{
		result /= 2.0;
		exponent++;
	}
	return result;
} /* DspFloatToNativeFloat */

static INT32
GetPolyData( INT32 addr )
{
	INT32 result;
	if( addr<0 || addr>=mPtRomSize )
	{
		return -1; /* HACK */
	}
	result = (mpPolyH[addr]<<16)|(mpPolyM[addr]<<8)|mpPolyL[addr];
	if( result&0x00800000 )
	{
		result |= 0xff000000; /* sign extend */
	}
	return result;
} /* GetPolyData */

UINT32
namcos22_point_rom_r( offs_t offs )
{
	return GetPolyData(offs);
}

UINT32 *namcos22_cgram;
UINT32 *namcos22_textram;
UINT32 *namcos22_polygonram;
UINT32 *namcos22_gamma;
UINT32 *namcos22_vics_data;
UINT32 *namcos22_vics_control;

/*
                         0    2    4    6    8    a    b
                      ^^^^ ^^^^ ^^^^ ^^^^                        cz offset
                                          ^^^^                   target (4==poly)
                                                    ^^^^         ????
         //00810000:  0000 0000 0000 0000 4444 0000 0000 0000 // solitar
         //00810000:  0000 0000 0000 0000 7555 0000 00e4 0000 // normal
         //00810000:  7fff 8000 7fff 8000 7555 0000 00e4 0000 // offset
         //00810000:  0000 0000 0000 0000 3111 0000 00e4 0000 // off
         //00810000:  0004 0004 0004 0004 4444 0000 0000 0000 // out pool
         //00810000:  00a4 00a4 00a4 00a4 4444 0000 0000 0000 // in pool
         //00810000:  ff80 ff80 ff80 ff80 4444 0000 0000 0000 // ending
         //00810000:  ff80 ff80 ff80 ff80 0000 0000 0000 0000 // hs entry
         //00810000:  ff01 ff01 0000 0000 0000 0000 00e4 0000 // alpine racer
*/
UINT32 *namcos22_czattr;

UINT32 *namcos22_tilemapattr;

static UINT8 *dirtypal;

READ32_HANDLER( namcos22_czram_r )
{
   int bank = nthword(namcos22_czattr,0xa/2);
   const UINT16 *czram = namcos22_czram[bank&3];
   return (czram[offset*2]<<16)|czram[offset*2+1];
}

WRITE32_HANDLER( namcos22_czram_w )
{
   int bank = nthword(namcos22_czattr,0xa/2);
   UINT16 *czram = namcos22_czram[bank&3];
   UINT32 dat = (czram[offset*2]<<16)|czram[offset*2+1];
   COMBINE_DATA( &dat );
   czram[offset*2] = dat>>16;
   czram[offset*2+1] = dat&0xffff;
}

static void
InitXYAttrToPixel( void )
{
	unsigned attr,x,y,ix,iy,temp;
	for( attr=0; attr<16; attr++ )
	{
		for( y=0; y<16; y++ )
		{
			for( x=0; x<16; x++ )
			{
				ix = x; iy = y;
				if( attr&4 ) ix = 15-ix;
				if( attr&2 ) iy = 15-iy;
				if( attr&8 ){ temp = ix; ix = iy; iy = temp; }
				mXYAttrToPixel[attr][x][y] = (iy<<4)|ix;
			}
		}
	}
} /* InitXYAttrToPixel */

static void
PatchTexture( void )
{
	int i;
	switch( namcos22_gametype )
	{
		case NAMCOS22_RIDGE_RACER:
		case NAMCOS22_RIDGE_RACER2:
		case NAMCOS22_ACE_DRIVER:
		case NAMCOS22_CYBER_COMMANDO:
			for( i=0; i<0x100000; i++ )
			{
				int tile = mpTextureTileMap16[i];
				int attr = mpTextureTileMapAttr[i];
				if( (attr&0x1)==0 )
				{
					tile = (tile&0x3fff)|0x8000;
					mpTextureTileMap16[i] = tile;
				}
			}
			break;

		default:
			break;
	}
} /* PatchTexture */

void
namcos22_draw_direct_poly( running_machine *machine, const UINT16 *pSource )
{
   /**
    * word#0:
    *    x--------------- end-of-display-list marker
    *    ----xxxxxxxxxxxx priority (lo)
    *
    * word#1:
    *    ----xxxxxxxxxxxx priority (hi)
    *
    * word#2:
    *    xxxxxxxx-------- PAL (high bit is fog enable)
    *    --------xxxx---- CMODE (color mode for texture unpack)
     *    ------------xxxx BN (texture bank)
    *
    * word#3:
    *    xxxxxxxx-------- ZC
    *    --------------xx depth cueing table select
    *
    * for each vertex:
    *    xxxx xxxx // u,v
     *
    *    xxxx xxxx // sx,sy
     *
    *    xx-- ---- // BRI
     *    --xx xxxx // zpos
     */
	INT32 zsortvalue24 = ((pSource[1]&0xfff)<<12)|(pSource[0]&0xfff);
	struct SceneNode *node = NewSceneNode(machine, zsortvalue24,eSCENENODE_QUAD3D);
	int i;
	node->data.quad3d.flags = ((pSource[3]&0x7f00)*2)|(pSource[3]&3);
	node->data.quad3d.cmode = (pSource[2]&0x00f0)>>4;
	node->data.quad3d.textureBank = pSource[2]&0xf;
	node->data.quad3d.color = (pSource[2]&0xff00)>>8;
	pSource += 4;
	for( i=0; i<4; i++ )
	{
		Poly3dVertex *p = &node->data.quad3d.v[i];

		p->u = pSource[0];
		p->v = pSource[1];
		if( mbSuperSystem22 )
		{
			p->u >>= 4;
			p->v >>= 4;
		}
		p->u &= 0xfff;
		p->v &= 0xfff;

		{
			int mantissa = (INT16)pSource[5];
			float zf = (float)mantissa;
			int exponent = (pSource[4])&0xff;
			if( mantissa )
			{
				while( exponent<0x2e )
				{
					zf /= 2.0;
					exponent++;
				}
				if( mbSuperSystem22 )
					p->z = zf;
				else
					p->z = 1.0f/zf;
			}
			else
			{
				zf = (float)0x10000;
				exponent = 0x40-exponent;
				while( exponent<0x2e )
				{
					zf /= 2.0;
					exponent++;
				}
				p->z = 1.0f/zf;
			}
		}

		p->x = ((INT16)pSource[2]);
		p->y = (-(INT16)pSource[3]);
		p->bri = pSource[4]>>8;
		pSource += 6;
	}
	node->data.quad3d.direct = 1;
	node->data.quad3d.vx = 0;
	node->data.quad3d.vy = 0;
	node->data.quad3d.vw = -320;
	node->data.quad3d.vh = -240;
} /* namcos22_draw_direct_poly */

static void
Prepare3dTexture( running_machine *machine, void *pTilemapROM, void *pTextureROM )
{
    int i;
    assert( pTilemapROM && pTextureROM );
    { /* following setup is Namco System 22 specific */
	      const UINT8 *pPackedTileAttr = 0x200000 + (UINT8 *)pTilemapROM;
	      UINT8 *pUnpackedTileAttr = auto_alloc_array(machine, UINT8, 0x080000*2);
    	{
    	   InitXYAttrToPixel();
	      mpTextureTileMapAttr = pUnpackedTileAttr;
	      for( i=0; i<0x80000; i++ )
	      {
	         *pUnpackedTileAttr++ = (*pPackedTileAttr)>>4;
	         *pUnpackedTileAttr++ = (*pPackedTileAttr)&0xf;
	         pPackedTileAttr++;
	   }
	   mpTextureTileMap16 = (UINT16 *)pTilemapROM;
         mpTextureTileData = (UINT8 *)pTextureROM;
	   PatchTexture();
      }
   }
} /* Prepare3dTexture */

static void
DrawSpritesHelper(
	running_machine *machine,
	bitmap_t *bitmap,
	const rectangle *cliprect,
	const UINT32 *pSource,
	const UINT32 *pPal,
	int num_sprites,
	int deltax,
	int deltay )
{
	int i;
	for( i=0; i<num_sprites; i++ )
	{
		/*
        ----.-x--.----.----.----.----.----.---- hidden?
        ----.--xx.----.----.----.----.----.---- ?
        ----.----.xxxx.xxxx.xxxx.----.----.---- always 0xff0?
        ----.----.----.----.----.--x-.----.---- right justify
        ----.----.----.----.----.---x.----.---- bottom justify
        ----.----.----.----.----.----.x---.---- flipx
        ----.----.----.----.----.----.-xxx.---- numcols
        ----.----.----.----.----.----.----.x--- flipy
        ----.----.----.----.----.----.----.-xxx numrows
        */
		UINT32 attrs = pSource[2];
		if( (attrs&0x04000000)==0 )
		{ /* sprite is not hidden */
			INT32 zcoord = pPal[0];
			int color = pPal[1]>>16;
			int cz = pPal[1]&0xffff;
			UINT32 xypos = pSource[0];
			UINT32 size = pSource[1];
			UINT32 code = pSource[3];
			int xpos = (xypos>>16)-deltax;
			int ypos = (xypos&0xffff)-deltay;
			int sizex = size>>16;
			int sizey = size&0xffff;
			int zoomx = (1<<16)*sizex/0x20;
			int zoomy = (1<<16)*sizey/0x20;
			int flipy = attrs&0x8;
			int numrows = attrs&0x7; /* 0000 0001 1111 1111 0000 0000 fccc frrr */
			int linkType = (attrs&0x00ff0000)>>16;
			int flipx = (attrs>>4)&0x8;
			int numcols = (attrs>>4)&0x7;
			int tile = code>>16;
			int translucency = (code&0xff00)>>8;

			if( numrows==0 )
				numrows = 8;
			if( flipy )
			{
				ypos += sizey*(numrows-1);
				sizey = -sizey;
			}

			if( numcols==0 )
				numcols = 8;
			if( flipx )
			{
				xpos += sizex*(numcols-1);
				sizex = -sizex;
			}

			if( attrs & 0x0200 )
			{ /* right justify */
				xpos -= ((zoomx*numcols*0x20)>>16)-1;
			}
			if( attrs & 0x0100 )
			{ /* bottom justify */
				ypos -= ((zoomy*numrows*0x20)>>16)-1;
			}

			{
				struct SceneNode *node = NewSceneNode(machine, zcoord,eSCENENODE_SPRITE);

//              printf("[%02d]: tile %x pri %x color %x flipX %d flipY %d cols %d rows %d link %d X %d Y %d sX %d sY %d trans %d cz %d\n",
//                  i, tile, cz&0x80, color&0x7f, flipx, flipy, numcols, numrows, linkType, xpos, ypos, sizex, sizey, translucency, cz);

				if (color == 0) color = 0x67;	// extreme hack for Tokyo Wars

				node->data.sprite.tile = tile;
				node->data.sprite.pri = cz&0x80;
				//              node->data.sprite.pri = (color&0x80);
				node->data.sprite.color = color&0x7f;
				node->data.sprite.flipx = flipx;
				node->data.sprite.flipy = flipy;
				node->data.sprite.numcols = numcols;
				node->data.sprite.numrows = numrows;
				node->data.sprite.linkType = linkType;
				node->data.sprite.xpos = xpos;
				node->data.sprite.ypos = ypos;
				node->data.sprite.sizex = sizex;
				node->data.sprite.sizey = sizey;
				node->data.sprite.translucency = translucency;
				node->data.sprite.cz = cz;
			}
		} /* visible sprite */
		pSource += 4;
		pPal += 2;
	}
} /* DrawSpritesHelper */

static void
DrawSprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
/*
// time crisis:
00980000: 00060000 000b0053 03000200 03000000
00980010: 00200020 028004ff 032a0509 00000000
00980200: 000007ff 000007ff 000007ff 032a0509
00980210: 000007ff 000007ff 000007ff 000007ff
00980220: 000007ff 000007ff 000007ff 000007ff
00980230: 000007ff 000007ff 05000500 050a050a

// prop normal
00980000: 00060000 00040053 03000200 03000000
00980010: 00200020 028004ff 032a0509 00000000
00980200: 028004ff 032a0509 028004ff 032a0509
00980210: 028004ff 032a0509 028004ff 032a0509
00980220: 028004ff 032a0509 028004ff 032a0509
00980230: 028004ff 032a0509 028004ff 032a0509

//alpine normal / prop test (-48,-43)
00980000: 00060000 00000000 02ff0000 000007ff
00980010: 00200020 000002ff 000007ff 00000000
00980200: 000007ff 000007ff 000007ff 000007ff
00980210: 000007ff 000007ff 000007ff 000007ff
00980220: 000007ff 000007ff 000007ff 000007ff
00980230: 000007ff 000007ff 000007ff 000007ff


        0x980000:   00060000 00010000 02ff0000 000007ff
                    ^^^^                                7 = disable
                             ^^^^                       num sprites

        0x980010:   00200020 000002ff 000007ff 00000000
                    ^^^^^^^^                            character size?
                             ^^^^                       delta xpos?
                                      ^^^^              delta ypos?

        0x980200:   000007ff 000007ff       delta xpos, delta ypos?
        0x980208:   000007ff 000007ff
        0x980210:   000007ff 000007ff
        0x980218:   000007ff 000007ff
        0x980220:   000007ff 000007ff
        0x980228:   000007ff 000007ff
        0x980230:   000007ff 000007ff
        0x980238:   000007ff 000007ff

        //time crisis
        00980200:  000007ff 000007ff 000007ff 032a0509
        00980210:  000007ff 000007ff 000007ff 000007ff
        00980220:  000007ff 000007ff 000007ff 000007ff
        00980230:  000007ff 000007ff 05000500 050a050a

        0x980400:   hzoom table
        0x980600:   vzoom table

        link table:
        0x980800:   0000 0001 0002 0003 ... 03ff

        eight words per sprite:
        0x984000:   010f 007b   xpos, ypos
        0x984004:   0020 0020   size x, size y
        0x984008:   00ff 0311   00ff, chr x;chr y;flip x;flip y
        0x98400c:   0001 0000   sprite code, translucency
        ...

        additional sorting/color data for sprite:
        0x9a0000:   C381 Z (sort)
        0x9a0004:   palette, C381 ZC (depth cueing)
        ...
    */
	UINT32 *spriteram32 = machine->generic.spriteram.u32;
	int num_sprites = ((spriteram32[0x04/4]>>16)&0x3ff)+1;
	const UINT32 *pSource = &spriteram32[0x4000/4];
	const UINT32 *pPal = &spriteram32[0x20000/4];
	int deltax = spriteram32[0x14/4]>>16;
	int deltay = spriteram32[0x18/4]>>16;
	int enable = spriteram32[0]>>16;

	/* HACK for Tokyo Wars */
	if (deltax == 0 && deltay == 0)
	{
		deltax = 190;
		deltay = 250;
	}

	if( spriteram32[0x14/4] == 0x000002ff &&
	spriteram32[0x18/4] == 0x000007ff )
	{ /* HACK (fixes alpine racer and self test) */
		deltax = 48;
		deltay = 43;
	}

	if( enable==6 /*&& namcos22_gametype!=NAMCOS22_AIR_COMBAT22*/ )
	{
		DrawSpritesHelper( machine, bitmap, cliprect, pSource, pPal, num_sprites, deltax, deltay );
	}

	/* VICS RAM provides two additional banks */
	/*
    0x940000 -x------       sprite chip busy
    0x940018 xxxx----       clr.w   $940018.l

    0x940034 xxxxxxxx       0x3070b0f

    0x940040 xxxxxxxx       sprite attribute size
    0x940048 xxxxxxxx       sprite attribute list baseaddr
    0x940050 xxxxxxxx       sprite color size
    0x940058 xxxxxxxx       sprite color list baseaddr

    0x940060..0x94007c      set#2
    */

	num_sprites = (namcos22_vics_control[0x40/4]&0xffff)/0x10;
	if( num_sprites>=1 )
	{
		pSource = &namcos22_vics_data[(namcos22_vics_control[0x48/4]&0xffff)/4];
		pPal    = &namcos22_vics_data[(namcos22_vics_control[0x58/4]&0xffff)/4];
		DrawSpritesHelper( machine, bitmap, cliprect, pSource, pPal, num_sprites, deltax, deltay );
	}

	num_sprites = (namcos22_vics_control[0x60/4]&0xffff)/0x10;
	if( num_sprites>=1 )
	{
		pSource = &namcos22_vics_data[(namcos22_vics_control[0x68/4]&0xffff)/4];
		pPal    = &namcos22_vics_data[(namcos22_vics_control[0x78/4]&0xffff)/4];
		DrawSpritesHelper( machine, bitmap, cliprect, pSource, pPal, num_sprites, deltax, deltay );
	}
} /* DrawSprites */

static void UpdatePaletteS(running_machine *machine) /* for Super System22 - apply gamma correction and preliminary fader support */
{
	int i;
	for( i=0; i<NAMCOS22_PALETTE_SIZE/4; i++ )
	{
		if( dirtypal[i] )
		{
			int j;
			for( j=0; j<4; j++ )
			{
				int which = i*4+j;
				int r = nthbyte(machine->generic.paletteram.u32,which+0x00000);
				int g = nthbyte(machine->generic.paletteram.u32,which+0x08000);
				int b = nthbyte(machine->generic.paletteram.u32,which+0x10000);
				palette_set_color( machine,which,MAKE_RGB(r,g,b) );
			}
			dirtypal[i] = 0;
		}
	}
} /* UpdatePaletteS */

static void UpdatePalette(running_machine *machine) /* for System22 - ignore gamma/fader effects for now */
{
	int i,j;
	for( i=0; i<NAMCOS22_PALETTE_SIZE/4; i++ )
	{
		if( dirtypal[i] )
		{
			for( j=0; j<4; j++ )
			{
				int which = i*4+j;
				int r = nthbyte(machine->generic.paletteram.u32,which+0x00000);
				int g = nthbyte(machine->generic.paletteram.u32,which+0x08000);
				int b = nthbyte(machine->generic.paletteram.u32,which+0x10000);
				palette_set_color( machine,which,MAKE_RGB(r,g,b) );
			}
			dirtypal[i] = 0;
		}
	}
} /* UpdatePalette */

static tilemap_t *bgtilemap;

static TILE_GET_INFO( TextTilemapGetInfo )
{
	UINT16 data = nthword( namcos22_textram,tile_index );
	/**
     * x---.----.----.---- blend
    * xxxx.----.----.---- palette select
    * ----.xx--.----.---- flip
    * ----.--xx.xxxx.xxxx code
    */
	SET_TILE_INFO( GFX_CHAR,data&0x03ff,data>>12,TILE_FLIPYX((data&0x0c00)>>10) );
	if( data&0x8000 )
	{
		tileinfo->category = 1;
	}
} /* TextTilemapGetInfo */

READ32_HANDLER( namcos22_textram_r )
{
	return namcos22_textram[offset];
}

WRITE32_HANDLER( namcos22_textram_w )
{
	COMBINE_DATA( &namcos22_textram[offset] );
	tilemap_mark_tile_dirty( bgtilemap, offset*2 );
	tilemap_mark_tile_dirty( bgtilemap, offset*2+1 );
}

static void
DrawTranslucentCharacters( bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT8 alpha = 0xff-mixer.text_translucency; /* ? */
	tilemap_draw( bitmap, cliprect, bgtilemap, TILEMAP_DRAW_ALPHA(alpha)|1, 0 );
}

static void DrawCharacterLayer(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	INT32 dx = namcos22_tilemapattr[0]>>16;
	INT32 dy = namcos22_tilemapattr[0]&0xffff;
	/**
    * namcos22_tilemapattr[0x4/4] == 0x006e0000
    * namcos22_tilemapattr[0x8/4] == 0x01ff0000
    * namcos22_tilemapattr[0xe/4] == ?
    */
	bitmap_fill(machine->priority_bitmap,cliprect,0);
	tilemap_set_scrollx( bgtilemap,0, (dx-0x35c)&0x3ff );
	tilemap_set_scrolly( bgtilemap,0, dy&0x3ff );
	tilemap_set_palette_offset( bgtilemap, mixer.palBase*256 );
	tilemap_draw( bitmap, cliprect, bgtilemap, 0/*flags*/, 0x1/*priority*/ ); /* opaque */
} /* DrawCharacterLayer */

/*********************************************************************************************/

static int
Cap( int val, int minval, int maxval )
{
   if( val<minval )
   {
      val = minval;
   }
   else if( val>maxval )
   {
      val = maxval;
   }
   return val;
}

#define LSB21 (0x1fffff)
#define LSB18 (0x03ffff)

static INT32
Signed18( UINT32 value )
{
   INT32 offset = value&LSB18;
   if( offset&0x20000 )
   { /* sign extend */
		offset |= ~LSB18;
   }
   return offset;
}

/**
 * @brief render a single quad
 *
 * @param flags
 *     x1.----.----.---- priority over tilemap
 *     --.-xxx.----.---- representative z algorithm?
 *     --.----.-1x-.---- backface cull enable
 *     --.----.----.--1x fog enable?
 *
 *      1163 // sky
 *      1262 // score (front)
 *      1242 // score (hinge)
 *      1243 // ?
 *      1063 // n/a
 *      1243 // various (2-sided?)
 *      1263 // everything else (1-sided?)
 *      1663 // ?
 *
 * @param color
 *      -------- xxxxxxxx unused?
 *      -xxxxxxx -------- palette select
 *      x------- -------- ?
 *
 * @param polygonShiftValue22
 *    0x1fbd0 - sky+sea
 *    0x0c350 - mountins
 *    0x09c40 - boats, surf, road, buildings
 *    0x07350 - guardrail
 *    0x061a8 - red car
 */
static void
BlitQuadHelper(
		running_machine *machine,
		bitmap_t *bitmap,
		unsigned color,
		unsigned addr,
		float m[4][4],
		INT32 polygonShiftValue22, /* 22 bits */
		int flags,
		int packetFormat )
{
	int absolutePriority = mAbsolutePriority;
	UINT32 zsortvalue24;
	float zmin = 0.0f;
	float zmax = 0.0f;
	Poly3dVertex v[4];
	int i;
	int bBackFace = 0;

	for( i=0; i<4; i++ )
	{
		Poly3dVertex *pVerTex = &v[i];
		pVerTex->x = GetPolyData(  8+i*3+addr );
		pVerTex->y = GetPolyData(  9+i*3+addr );
		pVerTex->z = GetPolyData( 10+i*3+addr );
		TransformPoint( &pVerTex->x, &pVerTex->y, &pVerTex->z, m );
	} /* for( i=0; i<4; i++ ) */

	if( (v[2].x*((v[0].z*v[1].y)-(v[0].y*v[1].z)))+
		(v[2].y*((v[0].x*v[1].z)-(v[0].z*v[1].x)))+
		(v[2].z*((v[0].y*v[1].x)-(v[0].x*v[1].y))) >= 0 &&

		(v[0].x*((v[2].z*v[3].y)-(v[2].y*v[3].z)))+
		(v[0].y*((v[2].x*v[3].z)-(v[2].z*v[3].x)))+
		(v[0].z*((v[2].y*v[3].x)-(v[2].x*v[3].y))) >= 0 )
	{
		bBackFace = 1;
	}

	/* backface cull one-sided polygons */
	if( bBackFace && (flags&0x0020) )
		return;

	for( i=0; i<4; i++ )
	{
		Poly3dVertex *pVerTex = &v[i];
		int bri;

		pVerTex->u = GetPolyData(  0+i*2+addr );
		pVerTex->v = GetPolyData(  1+i*2+addr );

		if( i==0 || pVerTex->z > zmax ) zmax = pVerTex->z;
		if( i==0 || pVerTex->z < zmin ) zmin = pVerTex->z;

		if( mLitSurfaceCount )
		{
			bri = mLitSurfaceInfo[mLitSurfaceIndex%mLitSurfaceCount];
			if( mSurfaceNormalFormat == 0x6666 )
			{
				if( i==3 )
					mLitSurfaceIndex++;
			}
			else if( mSurfaceNormalFormat == 0x4000 )
				mLitSurfaceIndex++;
			else
				logerror( "unknown normal format: 0x%x\n", mSurfaceNormalFormat );
		} /* pLitSurfaceInfo */
		else if( packetFormat & 0x40 )
			bri = (GetPolyData(i+addr)>>16)&0xff;
		else
			bri = 0x40;
		pVerTex->bri = bri;
	} /* for( i=0; i<4; i++ ) */

	if( zmin<0.0f ) zmin = 0.0f;
	if( zmax<0.0f ) zmax = 0.0f;

	switch( (flags&0x0f00)>>8 )
	{
		case 0:
			zsortvalue24 = (INT32)zmin;
			break;

		case 1:
			zsortvalue24 = (INT32)zmax;
			break;

		case 2:
		default:
			zsortvalue24 = (INT32)((zmin+zmax)/2.0f);
			break;
	}

	/* relative: representative z + shift values
    * 1x.xxxx.xxxxxxxx.xxxxxxxx fixed z value
    * 0x.xx--.--------.-------- absolute priority shift
    * 0-.--xx.xxxxxxxx.xxxxxxxx z-representative value shift
    */
	if( polygonShiftValue22 & 0x200000 )
		zsortvalue24 = polygonShiftValue22 & LSB21;
	else
	{
		zsortvalue24 += Signed18( polygonShiftValue22 );
		absolutePriority += (polygonShiftValue22&0x1c0000)>>18;
	}

	if( mObjectShiftValue22 & 0x200000 )
		zsortvalue24 = mObjectShiftValue22 & LSB21;
	else
	{
		zsortvalue24 += Signed18( mObjectShiftValue22 );
		absolutePriority += (mObjectShiftValue22&0x1c0000)>>18;
	}

	absolutePriority &= 7;
	zsortvalue24 = Cap(zsortvalue24,0,0x1fffff);
	zsortvalue24 |= (absolutePriority<<21);

	{
		struct SceneNode *node = NewSceneNode(machine, zsortvalue24,eSCENENODE_QUAD3D);
		node->data.quad3d.cmode = (v[0].u>>12)&0xf;
		node->data.quad3d.textureBank = (v[0].v>>12)&0xf;
		node->data.quad3d.color = (color>>8)&0xff;

		{
			INT32 cz = (INT32)((zmin+zmax)/2.0f);
			cz = Clamp256(cz/0x2000);
			node->data.quad3d.flags = (cz<<8)|(flags&3);
		}

		for( i=0; i<4; i++ )
		{
			Poly3dVertex *p = &node->data.quad3d.v[i];
			p->x     = v[i].x*mCamera.zoom;
			p->y     = v[i].y*mCamera.zoom;
			p->z     = v[i].z;
			p->u     = v[i].u&0xfff;
			p->v     = v[i].v&0xfff;
			p->bri   = v[i].bri;
		}
		node->data.quad3d.direct = 0;
		node->data.quad3d.vx = mCamera.vx;
		node->data.quad3d.vy = mCamera.vy;
		node->data.quad3d.vw = mCamera.vw;
		node->data.quad3d.vh = mCamera.vh;
	}
} /* BlitQuadHelper */

static void
RegisterNormals( INT32 addr, float m[4][4] )
{
	int i;
	for( i=0; i<4; i++ )
	{
		float nx = DSP_FIXED_TO_FLOAT(GetPolyData(addr+i*3+0));
		float ny = DSP_FIXED_TO_FLOAT(GetPolyData(addr+i*3+1));
		float nz = DSP_FIXED_TO_FLOAT(GetPolyData(addr+i*3+2));
		float dotproduct;

		/* transform normal vector */
		TransformNormal( &nx, &ny, &nz, m );
		dotproduct = nx*mCamera.lx + ny*mCamera.ly + nz*mCamera.lz;
		if( dotproduct<0.0f )
			dotproduct = 0.0f;
		mLitSurfaceInfo[mLitSurfaceCount++] = mCamera.ambient + mCamera.power*dotproduct;
	}
} /* RegisterNormals */

static void
BlitQuads( running_machine *machine, bitmap_t *bitmap, INT32 addr, float m[4][4], INT32 base )
{
	int numAdditionalNormals = 0;
	int chunkLength = GetPolyData(addr++);
	int finish = addr + chunkLength;

	if( chunkLength>0x100 )
		fatalerror( "bad packet length" );

	while( addr<finish )
	{
		int packetLength = GetPolyData( addr++ );
		int packetFormat = GetPolyData( addr+0 );
		int flags, color, bias;

		/**
        * packetFormat:
        *      800000 final packet in chunk
        *      080000 ?
        *      020000 color word exists?
        *      010000 z-offset word exists?
        *      002000 ?
        *      001000 z-offset word exists?
        *      000400 ?
        *      000080 tex# or UV or CMODE?
        *      000040 use I
        *      000001 ?
        *
        * flags:
        *      1042 (always set)
        *      0c00 depth-cueing mode (brake-disc(2108)=001e43, model font)
        *      0200 usually 1
        *      0100 ?
        *      0040 1 ... polygon palette?
        *      0020 cull backface
        *      0002 ?
        *      0001 ?
        *
        * color:
        *      ff0000 type?
        *      008000 depth-cueing off
        *      007f00 palette#
        */
		switch( packetLength )
		{
			case 0x17:
				/**
                * word 0: opcode (8a24c0)
                * word 1: flags
                * word 2: color
                */
				flags = GetPolyData(addr+1);
				color = GetPolyData(addr+2);
				bias = 0;
				BlitQuadHelper( machine,bitmap,color,addr+3,m,bias,flags,packetFormat );
				break;

			case 0x18:
				/**
                * word 0: opcode (0b3480 for first N-1 quads or 8b3480 for final quad in primitive)
                * word 1: flags
                * word 2: color
                * word 3: depth bias
                */
				flags = GetPolyData(addr+1);
				color = GetPolyData(addr+2);
				bias  = GetPolyData(addr+3);
				BlitQuadHelper( machine,bitmap,color,addr+4,m,bias,flags,packetFormat );
				break;

			case 0x10: /* vertex lighting */
				/*
                333401 (opcode)
                000000  [count] [type]
                000000  000000  007fff // normal vector
                000000  000000  007fff // normal vector
                000000  000000  007fff // normal vector
                000000  000000  007fff // normal vector
                */
				numAdditionalNormals = GetPolyData(addr+2);
				mSurfaceNormalFormat = GetPolyData(addr+3);
				mLitSurfaceCount = 0;
				mLitSurfaceIndex = 0;
				RegisterNormals( addr+4, m );
				break;

			case 0x0d: /* additional normals */
				/*
                300401 (opcode)
                007b09 ffdd04 0004c2
                007a08 ffd968 0001c1
                ff8354 ffe401 000790
                ff84f7 ffdd04 0004c2
                */
				RegisterNormals( addr+1, m );
				break;

			default:
				break;
		}
		addr += packetLength;
	}
} /* BlitQuads */

static void
BlitPolyObject( running_machine *machine, bitmap_t *bitmap, int code, float M[4][4] )
{
	unsigned addr1 = GetPolyData(code);
	mLitSurfaceCount = 0;
	mLitSurfaceIndex = 0;
	for(;;)
	{
		INT32 addr2 = GetPolyData(addr1++);
		if( addr2<0 )
			break;
		BlitQuads( machine, bitmap, addr2, M, code );
	}
} /* BlitPolyObject */

/*******************************************************************************/

READ32_HANDLER( namcos22_dspram_r )
{
	return namcos22_polygonram[offset];
}

WRITE32_HANDLER( namcos22_dspram_w )
{
	COMBINE_DATA( &namcos22_polygonram[offset] );
}

/*******************************************************************************/

/**
 * master DSP can write directly to render device via port 0xc.
 * This is used for "direct drawn" polygons, and "direct draw from point rom"
 * feature - both opcodes exist in Ridge Racer's display-list processing
 *
 * record format:
 *  header (3 words)
 *      polygonShiftValue22
 *      color
 *      flags
 *
 *  per-vertex data (4*6 words)
 *      u,v
 *      sx,sy
 *      intensity;z.exponent
 *      z.mantissa
 *
 * master DSP can specify 3d objects indirectly (along with view transforms),
 * via the "transmit" PDP opcode.  the "render device" sends quad data to the slave DSP
 * viewspace clipping and projection
 *
 * most "3d object" references are 0x45 and greater.  references less than 0x45 are "special"
 * commands, using a similar point rom format.  the point rom header may point to point ram.
 *
 * slave DSP reads records via port 4
 * its primary purpose is applying lighting calculations
 * the slave DSP forwards draw commands to a "draw device"
 */

/*******************************************************************************/

/**
 * 0xfffd
 * 0x0: transform
 * 0x1
 * 0x2
 * 0x5: transform
 * >=0x45: draw primitive
 */
static void
HandleBB0003( const INT32 *pSource )
{
   /*
        bb0003 or 3b0003

        14.00c8            light.ambient     light.power
        01.0000            ?                 light.dx
        06.5a82            window priority   light.dy
        00.a57e            ?                 light.dz

        c8.0081            vx=200,vy=129
        29.6092            zoom = 772.5625
        1e.95f8 1e.95f8            0.5858154296875   0.5858154296875 // 452
        1e.b079 1e.b079            0.6893463134765   0.6893463134765 // 532
        29.58e8                   711.25 (border? see time crisis)

        7ffe 0000 0000
        0000 7ffe 0000
        0000 0000 7ffe
    */
	mCamera.ambient = pSource[0x1]>>16;
	mCamera.power   = pSource[0x1]&0xffff;

	mCamera.lx       = DSP_FIXED_TO_FLOAT(pSource[0x2]);
	mCamera.ly       = DSP_FIXED_TO_FLOAT(pSource[0x3]);
	mCamera.lz       = DSP_FIXED_TO_FLOAT(pSource[0x4]);

   mAbsolutePriority = pSource[0x3]>>16;
   mCamera.vx      = (INT16)(pSource[5]>>16);
   mCamera.vy      = (INT16)pSource[5];
   mCamera.zoom    = DspFloatToNativeFloat(pSource[6]);
   mCamera.vw      = DspFloatToNativeFloat(pSource[7])*mCamera.zoom;
   mCamera.vh      = DspFloatToNativeFloat(pSource[9])*mCamera.zoom;

	mViewMatrix[0][0] = DSP_FIXED_TO_FLOAT(pSource[0x0c]);
	mViewMatrix[1][0] = DSP_FIXED_TO_FLOAT(pSource[0x0d]);
	mViewMatrix[2][0] = DSP_FIXED_TO_FLOAT(pSource[0x0e]);

	mViewMatrix[0][1] = DSP_FIXED_TO_FLOAT(pSource[0x0f]);
	mViewMatrix[1][1] = DSP_FIXED_TO_FLOAT(pSource[0x10]);
	mViewMatrix[2][1] = DSP_FIXED_TO_FLOAT(pSource[0x11]);

	mViewMatrix[0][2] = DSP_FIXED_TO_FLOAT(pSource[0x12]);
	mViewMatrix[1][2] = DSP_FIXED_TO_FLOAT(pSource[0x13]);
	mViewMatrix[2][2] = DSP_FIXED_TO_FLOAT(pSource[0x14]);

	TransformNormal( &mCamera.lx, &mCamera.ly, &mCamera.lz, mViewMatrix );
} /* HandleBB0003 */

static void
Handle200002( running_machine *machine, bitmap_t *bitmap, const INT32 *pSource )
{
	if( mPrimitiveID>=0x45 )
	{
		float m[4][4]; /* row major */

		matrix3d_Identity( m );

		m[0][0] = DSP_FIXED_TO_FLOAT(pSource[0x1]);
		m[1][0] = DSP_FIXED_TO_FLOAT(pSource[0x2]);
		m[2][0] = DSP_FIXED_TO_FLOAT(pSource[0x3]);

		m[0][1] = DSP_FIXED_TO_FLOAT(pSource[0x4]);
		m[1][1] = DSP_FIXED_TO_FLOAT(pSource[0x5]);
		m[2][1] = DSP_FIXED_TO_FLOAT(pSource[0x6]);

		m[0][2] = DSP_FIXED_TO_FLOAT(pSource[0x7]);
		m[1][2] = DSP_FIXED_TO_FLOAT(pSource[0x8]);
		m[2][2] = DSP_FIXED_TO_FLOAT(pSource[0x9]);

		m[3][0] = pSource[0xa]; /* xpos */
		m[3][1] = pSource[0xb]; /* ypos */
		m[3][2] = pSource[0xc]; /* zpos */

      matrix3d_Multiply( m, mViewMatrix );
		BlitPolyObject( machine, bitmap, mPrimitiveID, m );
	}
   else if( mPrimitiveID !=0 && mPrimitiveID !=2 )
   {
      logerror( "Handle200002:unk code=0x%x\n", mPrimitiveID );
   }
} /* Handle200002 */

static void
Handle300000( const INT32 *pSource )
{ /* set view transform */
	mViewMatrix[0][0] = DSP_FIXED_TO_FLOAT(pSource[1]);
	mViewMatrix[1][0] = DSP_FIXED_TO_FLOAT(pSource[2]);
	mViewMatrix[2][0] = DSP_FIXED_TO_FLOAT(pSource[3]);

	mViewMatrix[0][1] = DSP_FIXED_TO_FLOAT(pSource[4]);
	mViewMatrix[1][1] = DSP_FIXED_TO_FLOAT(pSource[5]);
	mViewMatrix[2][1] = DSP_FIXED_TO_FLOAT(pSource[6]);

	mViewMatrix[0][2] = DSP_FIXED_TO_FLOAT(pSource[7]);
	mViewMatrix[1][2] = DSP_FIXED_TO_FLOAT(pSource[8]);
	mViewMatrix[2][2] = DSP_FIXED_TO_FLOAT(pSource[9]);
} /* Handle300000 */

static void
Handle233002( const INT32 *pSource )
{ /* set modal rendering options */
   /*
    00233002
       00000000 // zc adjust?
       0003dd00 // z bias adjust
       001fffff // far plane?
       00007fff 00000000 00000000
       00000000 00007fff 00000000
       00000000 00000000 00007fff
       00000000 00000000 00000000
   */
   mObjectShiftValue22 = pSource[2];
} /* Handle233002 */

static void
SimulateSlaveDSP( running_machine *machine, bitmap_t *bitmap )
{
	const INT32 *pSource = 0x300 + (INT32 *)namcos22_polygonram;
	INT16 len;

	matrix3d_Identity( mViewMatrix );

	if( mbSuperSystem22 )
	{
		pSource += 4; /* FFFE 0400 */
	}
	else
	{
		pSource--;
	}

	for(;;)
	{
		INT16 marker, next;
		mPrimitiveID = *pSource++;
		len  = (INT16)*pSource++;

		switch( len )
		{
		case 0x15:
			HandleBB0003( pSource ); /* define viewport */
			break;

		case 0x10:
			Handle233002( pSource ); /* set modal rendering options */
			break;

		case 0x0a:
			Handle300000( pSource ); /* modify view transform */
			break;

		case 0x0d:
			Handle200002( machine, bitmap, pSource ); /* render primitive */
			break;

		default:
         logerror( "unk 3d data(%d) addr=0x%x!", len, (int)(pSource-(INT32*)namcos22_polygonram) );
         {
            int i;
            for( i=0; i<len; i++ )
            {
               logerror( " %06x", pSource[i]&0xffffff );
            }
            logerror( "\n" );
         }
			return;
		}

		/* hackery! commands should be streamed, not parsed here */
		pSource += len;
		marker = (INT16)*pSource++; /* always 0xffff */
		next   = (INT16)*pSource++; /* link to next command */
		if( (next&0x7fff) != (pSource - (INT32 *)namcos22_polygonram) )
		{ /* end of list */
			break;
		}
	} /* for(;;) */
} /* SimulateSlaveDSP */

static void
DrawPolygons( running_machine *machine, bitmap_t *bitmap )
{
	if( mbDSPisActive )
	{
		SimulateSlaveDSP( machine, bitmap );
		poly_wait(poly, "DrawPolygons");
	}
} /* DrawPolygons */

void
namcos22_enable_slave_simulation( void )
{
	mbDSPisActive = 1;
}

/*********************************************************************************************/

READ32_HANDLER( namcos22_cgram_r )
{
	return namcos22_cgram[offset];
}

WRITE32_HANDLER( namcos22_cgram_w )
{
	COMBINE_DATA( &namcos22_cgram[offset] );
	gfx_element_mark_dirty(space->machine->gfx[GFX_CHAR],offset/32);
}

READ32_HANDLER( namcos22_gamma_r )
{
	return namcos22_gamma[offset];
}

WRITE32_HANDLER( namcos22_gamma_w )
{
	COMBINE_DATA( &namcos22_gamma[offset] );
}

READ32_HANDLER( namcos22_paletteram_r )
{
	return space->machine->generic.paletteram.u32[offset];
}

WRITE32_HANDLER( namcos22_paletteram_w )
{
	COMBINE_DATA( &space->machine->generic.paletteram.u32[offset] );
	dirtypal[offset&(0x7fff/4)] = 1;
}

static void namcos22_reset(running_machine *machine)
{
	memset(&mSceneRoot, 0, sizeof(mSceneRoot));
	mpFreeSceneNode = NULL;
}

static void namcos22_exit(running_machine *machine)
{
	while (mpFreeSceneNode != NULL)
	{
		struct SceneNode *node = mpFreeSceneNode;
		mpFreeSceneNode = node->nextInBucket;
		free(node);
	}

	poly_free(poly);
}

static VIDEO_START( common )
{
	int code;

	bgtilemap = tilemap_create( machine, TextTilemapGetInfo,tilemap_scan_rows,16,16,64,64 );
		tilemap_set_transparent_pen( bgtilemap, 0xf );

	mbDSPisActive = 0;
	memset( namcos22_polygonram, 0xcc, 0x20000 );

	for (code = 0; code < machine->gfx[GFX_TEXTURE_TILE]->total_elements; code++)
		gfx_element_decode(machine->gfx[GFX_TEXTURE_TILE], code);
	Prepare3dTexture(machine, memory_region(machine, "textilemap"), machine->gfx[GFX_TEXTURE_TILE]->gfxdata );
	dirtypal = auto_alloc_array(machine, UINT8, NAMCOS22_PALETTE_SIZE/4);
	mPtRomSize = memory_region_length(machine, "pointrom")/3;
	mpPolyL = memory_region(machine, "pointrom");
	mpPolyM = mpPolyL + mPtRomSize;
	mpPolyH = mpPolyM + mPtRomSize;

#ifdef RENDER_AS_QUADS
	poly = poly_alloc(machine, 4000, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);
#else
	poly = poly_alloc(machine, 4000, sizeof(poly_extra_data), 0);
#endif
	add_reset_callback(machine, namcos22_reset);
	add_exit_callback(machine, namcos22_exit);

	gfx_element_set_source(machine->gfx[GFX_CHAR], (UINT8 *)namcos22_cgram);
}

VIDEO_START( namcos22 )
{
   mbSuperSystem22 = 0;
   VIDEO_START_CALL(common);
}

VIDEO_START( namcos22s )
{
   mbSuperSystem22 = 1;
   namcos22_czram[0] = auto_alloc_array(machine, UINT16, 0x200/2 );
   namcos22_czram[1] = auto_alloc_array(machine, UINT16, 0x200/2 );
   namcos22_czram[2] = auto_alloc_array(machine, UINT16, 0x200/2 );
   namcos22_czram[3] = auto_alloc_array(machine, UINT16, 0x200/2 );

   memset(namcos22_czram[0], 0, 0x200);
   memset(namcos22_czram[1], 0, 0x200);
   memset(namcos22_czram[2], 0, 0x200);
   memset(namcos22_czram[3], 0, 0x200);

   VIDEO_START_CALL(common);
}

VIDEO_UPDATE( namcos22s )
{
	UINT32 bgColor;
	UpdateVideoMixer();
	bgColor = (mixer.rBackColor<<16)|(mixer.gBackColor<<8)|mixer.bBackColor;
	bitmap_fill( bitmap, cliprect , bgColor);
	UpdatePaletteS(screen->machine);
	DrawCharacterLayer(screen->machine, bitmap, cliprect );
	DrawPolygons( screen->machine, bitmap );
	DrawSprites( screen->machine, bitmap, cliprect );
	RenderScene(screen->machine, bitmap );
	DrawTranslucentCharacters( bitmap, cliprect );
	ApplyGamma( screen->machine, bitmap );

#ifdef MAME_DEBUG
   if( input_code_pressed(screen->machine, KEYCODE_D) )
   {
      FILE *f = fopen( "dump.txt", "wb" );
      if( f )
      {
         const address_space *space = cputag_get_address_space(screen->machine, "maincpu", ADDRESS_SPACE_PROGRAM);

         {
            int i,bank;
            for( bank=0; bank<4; bank++ )
            {
               fprintf( f, "czram[%d] =", bank );
               for( i=0; i<256; i++ )
               {
                  fprintf( f, " %04x", namcos22_czram[bank][i] );
               }
               fprintf( f, "\n" );
            }
         }

         Dump(space, f,0x810000, 0x81000f, "cz attr" );
         Dump(space, f,0x820000, 0x8202ff, "unk_ac" );
         Dump(space, f,0x824000, 0x8243ff, "gamma");
         Dump(space, f,0x828000, 0x83ffff, "palette" );
         Dump(space, f,0x8a0000, 0x8a000f, "tilemap_attr");
         Dump(space, f,0x880000, 0x89ffff, "cgram/textram");
         Dump(space, f,0x900000, 0x90ffff, "vics_data");
         Dump(space, f,0x940000, 0x94007f, "vics_control");
         Dump(space, f,0x980000, 0x9affff, "sprite374" );
         Dump(space, f,0xc00000, 0xc1ffff, "polygonram");
         fclose( f );
      }
      while( input_code_pressed(screen->machine, KEYCODE_D) ){}
   }
#endif
	return 0;
}

VIDEO_UPDATE( namcos22 )
{
	UpdateVideoMixer();
	bitmap_fill( bitmap, cliprect , get_black_pen(screen->machine));
	UpdatePalette(screen->machine);
	DrawCharacterLayer(screen->machine, bitmap, cliprect );
	DrawPolygons( screen->machine, bitmap );
	RenderScene(screen->machine, bitmap);
	DrawTranslucentCharacters( bitmap, cliprect );
	ApplyGamma( screen->machine, bitmap );

#ifdef MAME_DEBUG
   if( input_code_pressed(screen->machine, KEYCODE_D) )
   {
      FILE *f = fopen( "dump.txt", "wb" );
      if( f )
      {
         const address_space *space = cputag_get_address_space(screen->machine, "maincpu", ADDRESS_SPACE_PROGRAM);

//         Dump(space, f,0x90000000, 0x90000003, "led?" );
//         Dump(space, f,0x90010000, 0x90017fff, "cz_ram");
//         Dump(space, f,0x900a0000, 0x900a000f, "tilemap_attr");
         Dump(space, f,0x90020000, 0x90027fff, "gamma");
//         Dump(space, f,0x70000000, 0x7001ffff, "polygonram");
         fclose( f );
      }
      while( input_code_pressed(screen->machine, KEYCODE_D) ){}
   }
#endif
	return 0;
}

WRITE16_HANDLER( namcos22_dspram16_bank_w )
{
	COMBINE_DATA( &namcos22_dspram_bank );
}

READ16_HANDLER( namcos22_dspram16_r )
{
	UINT32 value = namcos22_polygonram[offset];
	switch( namcos22_dspram_bank )
	{
	case 0:
		value &= 0xffff;
		break;

	case 1:
		value>>=16;
		break;

	case 2:
		mUpperWordLatch = value>>16;
		value &= 0xffff;
		break;

	default:
		break;
	}
	return (UINT16)value;
} /* namcos22_dspram16_r */

WRITE16_HANDLER( namcos22_dspram16_w )
{
	UINT32 value = namcos22_polygonram[offset];
	UINT16 lo = value&0xffff;
	UINT16 hi = value>>16;
	switch( namcos22_dspram_bank )
	{
	case 0:
		COMBINE_DATA( &lo );
		break;

	case 1:
		COMBINE_DATA( &hi );
		break;

	case 2:
		COMBINE_DATA( &lo );
		hi = mUpperWordLatch;
		break;

	default:
		break;
	}
	namcos22_polygonram[offset] = (hi<<16)|lo;
} /* namcos22_dspram16_w */

#ifdef MAME_DEBUG
static void
Dump( const address_space *space, FILE *f, unsigned addr1, unsigned addr2, const char *name )
{
   unsigned addr;
   fprintf( f, "%s:\n", name );
   for( addr=addr1; addr<=addr2; addr+=16 )
   {
      UINT8 data[16];
      int bHasNonZero = 0;
      int i;
      for( i=0; i<16; i++ )
      {
         data[i] = memory_read_byte(space, addr+i );
         if( data[i] )
         {
            bHasNonZero = 1;
         }
      }
      if( bHasNonZero )
      {
         fprintf( f,"%08x:", addr );
         for( i=0; i<16; i++ )
         {
            if( (i&0x03)==0 )
            {
               fprintf( f, " " );
            }
            fprintf( f, "%02x", data[i] );
         }
         fprintf( f, "\n" );
      }
   }
   fprintf( f, "\n" );
}
#endif

/**
 * 4038 spot enable?
 * 0828 pre-initialization
 * 0838 post-initialization
 **********************************************
 * upload:
 *   #bits data
 *    0010 FEC0
 *    0010 FF10
 *    0004 0004
 *    0004 000E
 *    0003 0007
 *    0002 0002
 *    0002 0003
 *    0001 0001
 *    0001 0001
 *    0001 0000
 *    0001 0001
 *    0001 0001
 *    0001 0000
 *    0001 0000
 *    0001 0000
 *    0001 0001
 **********************************************
 *    0008 00EA // 0x0ff
 *    000A 0364 // 0x3ff
 *    000A 027F // 0x3ff
 *    0003 0005 // 0x007
 *    0001 0001 // 0x001
 *    0001 0001 // 0x001
 *    0001 0001 // 0x001
 **********************************************
 */
WRITE32_HANDLER(namcos22_port800000_w)
{
   /* 00000011011111110000100011111111001001111110111110110001 */
   UINT16 word = data>>16;
   logerror( "%x: C304/C399: 0x%04x\n", cpu_get_previouspc(space->cpu), word );
   if( word == 0x4038 )
   {
      mbSpotlightEnable = 1;
   }
   else
   {
      mbSpotlightEnable = 0;
   }
} /* namcos22_port800000_w */
