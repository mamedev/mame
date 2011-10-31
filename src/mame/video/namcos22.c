/**
 * video hardware for Namco System22
 *
 * todo (ordered by importance):
 *
 * - emulate slave dsp!
 * - texture u/v mapping is often 1 pixel off, resulting in many glitch lines/gaps between textures
 * - tokyowar tanks are not shootable, same for timecris helicopter, there's still a very small hitbox but almost impossible to hit
 *       (is this related to dsp? or cpu?)
 * - find out how/where vics num_sprites is determined exactly, it causes major sprite problems in airco22b
 *       dirtdash would have this issue too, if not for the current workaround
 * - improve ss22 fogging:
 *       + scene changes too rapidly sometimes, eg. dirtdash snow level finish (see attract), or aquajet going down the waterfall
 *       + 100% fog if you start dirtdash at the hill level
 * - improve ss22 lighting, eg. mountains in alpinr2b selection screen
 * - improve ss22 spot:
 *       + dirtdash spotlight is opaque for a short time when exiting the jungle level
 *       + dirtdash speedometer has wrong colors when in the jungle level
 *       + dirtdash record time message creates a 'gap' in the spotlight when entering the jungle level (possibly just a game bug)
 * - window clipping is wrong in acedrvrw, victlapw
 * - ridgerac waving flag title screen is missing, just an empty beach scenery instead
 * - global offset is wrong in non-super22 servicemode video test, and above that, it flickers in acedrvrw, victlapw
 * - dirtdash polys are broken at the start section of the mountain level
 * - alpinr2b skiier selection screen should have mirrored models (easiest to see with cursor on the red pants guy)
 * - propcycl scoreboard sprite part should fade out in attract mode and just before game over, fader or fog related?
 * - ridgerac fogging isn't applied to the upper/side part of the sky (best seen when driving down a hill), it's fine in ridgera2
 *       czram contents is rather odd here and partly cleared (probably the cause?):
 *        $0000-$0d7f   - gradual increase from $00-$7c
 *        $0d80-$0fff   - $73, huh, why lower?
 *        $1000-$19ff   - $00, huh!? (it's specifically cleared, memsetting czram at boot does not fix the issue)
 *        $1a00-$0dff   - $77
 *        $1e00-$1fff   - $78
 * - using rgbint to set brightness may cause problems if a color channel is 00 (eg. victlapw attract)
 *       (probably a bug in rgbint, not here?)
 *
 * - lots of smaller issues
 *
 *
 *******************************/

#include "emu.h"
#include "video/rgbutil.h"
#include "includes/namcos22.h"
#include "video/poly.h"

/* for debug: allow memdump to file with D key */
#define ALLOW_MEMDUMP	0

#if ALLOW_MEMDUMP
static void Dump( address_space *space, FILE *f, unsigned addr1, unsigned addr2, const char *name )
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
			data[i] = space->read_byte(addr+i );
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


static struct
{
	int flags;
	int rFogColor;
	int gFogColor;
	int bFogColor;
	UINT32 fog_colormask;
	int rFogColor_per_cztype[4];
	int gFogColor_per_cztype[4];
	int bFogColor_per_cztype[4];
	int rPolyFadeColor;
	int gPolyFadeColor;
	int bPolyFadeColor;
	int PolyFade_enabled;
	int rFadeColor;
	int gFadeColor;
	int bFadeColor;
	int fadeFactor;
	int spot_limit;
	int poly_translucency;
	int palBase;
} mixer;

static void
UpdateVideoMixer( running_machine &machine )
{
	int i;
	namcos22_state *state = machine.driver_data<namcos22_state>();
	poly_wait(state->m_poly, "UpdateVideoMixer");
	memset( &mixer, 0, sizeof(mixer) );
#if 0 // show reg contents
	char msg1[0x1000]={0}, msg2[0x1000]={0};
	int st=0x000/16;
	for (i=st;i<(st+3);i++)
	{
		sprintf(msg2,"%04X %08X %08X %08X %08X\n",i*16,state->m_gamma[i*4+0],state->m_gamma[i*4+1],state->m_gamma[i*4+2],state->m_gamma[i*4+3]);
		strcat(msg1,msg2);
	}
	if (1) // + other non-super regs
	if (!state->m_mbSuperSystem22)
	{
		strcat(msg1,"\n");
		for (i=8;i<=0x20;i+=8)
		{
			sprintf(msg2,"%04X %08X %08X %08X %08X\n",i*16,state->m_gamma[i*4+0],state->m_gamma[i*4+1],state->m_gamma[i*4+2],state->m_gamma[i*4+3]);
			strcat(msg1,msg2);
		}
	}
	popmessage("%s",msg1);
#endif

	if( state->m_mbSuperSystem22 )
	{
/*
           0 1 2 3  4 5 6 7  8 9 a b  c d e f 10       14       18       1c
00824000: ffffff00 00000000 0000007f 00ff0000 1000ff00 0f000000 00ff007f 00010007 // time crisis
00824000: ffffff00 00000000 1830407f 00800000 0000007f 0f000000 0000037f 00010007 // trans sprite
00824000: ffffff00 00000000 3040307f 00000000 0080007f 0f000000 0000037f 00010007 // trans poly
00824000: ffffff00 00000000 1800187f 00800000 0080007f 0f000000 0000037f 00010007 // trans poly(2)
00824000: ffffff00 00000000 1800187f 00000000 0000007f 0f800000 0000037f 00010007 // trans text

    00,01,02        polygon fade rgb
    03
    04
    05,06,07        world fog rgb
    08,09,0a        background color
    0b
    0c
    0d              spot factor limit value
    0e              enable spot factor limit
    0f
    10
    11              global polygon alpha factor
    12,13           textlayer alpha pen comparison
    14              textlayer alpha pen mask?
    15              textlayer alpha factor
    16,17,18        global fade rgb
    19              global fade factor
    1a              fade target flags
    1b              textlayer palette base
    1c
    1d
    1e
    1f              layer enable
*/
		mixer.rPolyFadeColor    = nthbyte( state->m_gamma, 0x00 );
		mixer.gPolyFadeColor    = nthbyte( state->m_gamma, 0x01 );
		mixer.bPolyFadeColor    = nthbyte( state->m_gamma, 0x02 ); mixer.PolyFade_enabled = (mixer.rPolyFadeColor == 0xff && mixer.gPolyFadeColor == 0xff && mixer.bPolyFadeColor == 0xff) ? 0 : 1;
		mixer.rFogColor         = nthbyte( state->m_gamma, 0x05 );
		mixer.gFogColor         = nthbyte( state->m_gamma, 0x06 );
		mixer.bFogColor         = nthbyte( state->m_gamma, 0x07 );
		mixer.spot_limit        = nthbyte( state->m_gamma, 0x0d );
		mixer.poly_translucency = nthbyte( state->m_gamma, 0x11 );
		mixer.rFadeColor        = nthbyte( state->m_gamma, 0x16 );
		mixer.gFadeColor        = nthbyte( state->m_gamma, 0x17 );
		mixer.bFadeColor        = nthbyte( state->m_gamma, 0x18 );
		mixer.fadeFactor        = nthbyte( state->m_gamma, 0x19 );
		mixer.flags             = nthbyte( state->m_gamma, 0x1a );
		mixer.palBase           = nthbyte( state->m_gamma, 0x1b ) & 0x7f;

		// put spot-specific flags into high word
		mixer.flags |= state->m_spot_enable << 16;
		mixer.flags |= (nthbyte(state->m_gamma, 0x0e) & 1) << 17;
		mixer.flags |= (state->m_chipselect & 0xc000) << 4;
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

    00,01           display flags
    02
    03
    04
    05
    06
    07              textlayer palette base?
    08,09,0a        textlayer pen c shadow rgb
    0b,0c,0d        textlayer pen d shadow rgb
    0e,0f,10        textlayer pen e shadow rgb
    11,12           global fade factor red
    13,14           global fade factor green
    15,16           global fade factor blue
    80-87           fog color mask?
    100,180,200     fog rgb 0
    101,181,201     fog rgb 1
    102,182,202     fog rgb 2
    103,183,203     fog rgb 3
*/
		mixer.flags             = nthbyte( state->m_gamma, 0x00 )*256 + nthbyte( state->m_gamma, 0x01 );
		mixer.rPolyFadeColor    = nthbyte( state->m_gamma, 0x11 )*256 + nthbyte( state->m_gamma, 0x12 ); // 0x0100 = 1.0
		mixer.gPolyFadeColor    = nthbyte( state->m_gamma, 0x13 )*256 + nthbyte( state->m_gamma, 0x14 );
		mixer.bPolyFadeColor    = nthbyte( state->m_gamma, 0x15 )*256 + nthbyte( state->m_gamma, 0x16 );
		mixer.PolyFade_enabled  = (mixer.rPolyFadeColor == 0x100 && mixer.gPolyFadeColor == 0x100 && mixer.bPolyFadeColor == 0x100) ? 0 : 1;

		// raveracw is the only game using multiple fog colors (city smog, cars under tunnels, brake disc in attract mode)
		mixer.fog_colormask     = state->m_gamma[0x84/4];

		// fog color per cz type
		for (i=0; i<4; i++)
		{
			mixer.rFogColor_per_cztype[i] = nthbyte( state->m_gamma, 0x0100+i );
			mixer.gFogColor_per_cztype[i] = nthbyte( state->m_gamma, 0x0180+i );
			mixer.bFogColor_per_cztype[i] = nthbyte( state->m_gamma, 0x0200+i );
		}

		mixer.palBase = 0x7f;
	}
}

READ32_HANDLER( namcos22_gamma_r )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	return state->m_gamma[offset];
}

WRITE32_HANDLER( namcos22_gamma_w )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	COMBINE_DATA( &state->m_gamma[offset] );
}

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
	mClip.scissor.max_x = cx - vw;
	mClip.scissor.min_y = cy + vh;
	mClip.scissor.max_y = cy - vh;
	if( mClip.scissor.min_x<0 )   mClip.scissor.min_x = 0;
	if( mClip.scissor.max_x>639 ) mClip.scissor.max_x = 639;
	if( mClip.scissor.min_y<0 )   mClip.scissor.min_y = 0;
	if( mClip.scissor.max_y>479 ) mClip.scissor.max_y = 479;
}

static void
sprite_Clip( int min_x, int max_x, int min_y, int max_y )
{
	// cx/cy not used
	mClip.scissor.min_x = min_x;
	mClip.scissor.max_x = max_x;
	mClip.scissor.min_y = min_y;
	mClip.scissor.max_y = max_y;
	if( mClip.scissor.min_x<0 )   mClip.scissor.min_x = 0;
	if( mClip.scissor.max_x>639 ) mClip.scissor.max_x = 639;
	if( mClip.scissor.min_y<0 )   mClip.scissor.min_y = 0;
	if( mClip.scissor.max_y>479 ) mClip.scissor.max_y = 479;
}

static void
poly3d_NoClip( void )
{
	mClip.cx = 320;
	mClip.cy = 240;
	mClip.scissor.min_x = 0;
	mClip.scissor.max_x = 639;
	mClip.scissor.min_y = 0;
	mClip.scissor.max_y = 479;
}

typedef struct
{
	float x,y,z;
	int u,v; /* 0..0xfff */
	int bri; /* 0..0xff */
} Poly3dVertex;

#define MIN_Z (10.0f)


INLINE unsigned texel( namcos22_state *state, unsigned x, unsigned y )
{
	unsigned offs = ((y&0xfff0)<<4)|((x&0xff0)>>4);
	unsigned tile = state->m_mpTextureTileMap16[offs];
	return state->m_mpTextureTileData[(tile<<8)|state->m_mXYAttrToPixel[state->m_mpTextureTileMapAttr[offs]][x&0xf][y&0xf]];
} /* texel */



typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	/* poly / sprites */
	running_machine *machine;
	rgbint fogColor;
	rgbint fadeColor;
	rgbint polyColor;
	const pen_t *pens;
	bitmap_t *priority_bitmap;
	int bn;
	int flags;
	int prioverchar;
	int cmode;
	int fadeFactor;
	int pfade_enabled;
	int fogFactor;
	int zfog_enabled;
	int cz_adjust;
	int cz_sdelta;
	const UINT8 *czram;

	/* sprites */
	const UINT8 *source;
	int alpha;
	int line_modulo;
	int flipx;
	int flipy;
};


static void renderscanline_uvi_full(void *destbase, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	namcos22_state *state = extra->machine->driver_data<namcos22_state>();
	float z = extent->param[0].start;
	float u = extent->param[1].start;
	float v = extent->param[2].start;
	float i = extent->param[3].start;
	float dz = extent->param[0].dpdx;
	float du = extent->param[1].dpdx;
	float dv = extent->param[2].dpdx;
	float di = extent->param[3].dpdx;
	bitmap_t *destmap = (bitmap_t *)destbase;
	int bn = extra->bn * 0x1000;
	const pen_t *pens = extra->pens;
	const UINT8 *czram = extra->czram;
	int cz_adjust = extra->cz_adjust;
	int cz_sdelta = extra->cz_sdelta;
	int zfog_enabled = extra->zfog_enabled;
	int fogFactor = 0xff - extra->fogFactor;
	int fadeFactor = 0xff - extra->fadeFactor;
	int alphaFactor = 0xff - mixer.poly_translucency;
	rgbint fogColor = extra->fogColor;
	rgbint fadeColor = extra->fadeColor;
	rgbint polyColor = extra->polyColor;
	int polyfade_enabled = extra->pfade_enabled;
	int penmask = 0xff;
	int penshift = 0;
	int prioverchar = extra->prioverchar;
	UINT32 *dest = BITMAP_ADDR32(destmap, scanline, 0);
	UINT8 *primap = BITMAP_ADDR8(extra->priority_bitmap, scanline, 0);
	int x;

	if (extra->cmode & 4)
	{
		pens += 0xec + ((extra->cmode & 8) << 1);
		penmask = 0x03;
		penshift = 2 * (~extra->cmode & 3);
	}
	else if (extra->cmode & 2)
	{
		pens += 0xe0 + ((extra->cmode & 8) << 1);
		penmask = 0x0f;
		penshift = 4 * (~extra->cmode & 1);
	}

	// slight differences between super and non-super, do the branch here for optimization
	// normal: 1 fader, no alpha, shading after fog
	// super:  2 faders, alpha, shading before fog
	if (state->m_mbSuperSystem22)
	{
		for( x=extent->startx; x<extent->stopx; x++ )
		{
			float ooz = 1.0f / z;
			int pen = texel(state, (int)(u*ooz), bn+(int)(v*ooz));
			// pen = 0x55; // debug: disable textures

			rgbint rgb;
			rgb_to_rgbint(&rgb, pens[pen>>penshift&penmask]);

			// apply shading before fog
			int shade = i*ooz;
			rgbint_scale_immediate_and_clamp(&rgb, shade << 2);

			// per-z distance fogging
			if (zfog_enabled)
			{
				int cz = ooz + cz_adjust;
				// discard low byte and clamp to 0-1fff
				if ((UINT32)cz < 0x200000) cz >>= 8;
				else cz = (cz < 0) ? 0 : 0x1fff;
				if ((fogFactor = czram[cz] + cz_sdelta) > 0)
				{
					if (fogFactor>0xff) fogFactor=0xff;
					rgbint_blend(&rgb, &fogColor, 0xff-fogFactor);
				}
			}
			else if (fogFactor != 0xff) // direct
				rgbint_blend(&rgb, &fogColor, fogFactor);

			if( polyfade_enabled )
				rgbint_scale_channel_and_clamp(&rgb, &polyColor);

			if( fadeFactor != 0xff )
				rgbint_blend(&rgb, &fadeColor, fadeFactor);

			if( alphaFactor != 0xff )
			{
				rgbint mix;
				rgb_to_rgbint(&mix, dest[x]);
				rgbint_blend(&rgb, &mix, alphaFactor);
			}

			dest[x] = rgbint_to_rgb(&rgb);
			primap[x] |= prioverchar;

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
			float ooz = 1.0f / z;
			int pen = texel(state, (int)(u*ooz), bn+(int)(v*ooz));
			// pen = 0x55; // debug: disable textures

			rgbint rgb;
			rgb_to_rgbint(&rgb, pens[pen>>penshift&penmask]);

			// per-z distance fogging
			if (zfog_enabled)
			{
				int cz = ooz + cz_adjust;
				// discard low byte and clamp to 0-1fff
				if ((UINT32)cz < 0x200000) cz >>= 8;
				else cz = (cz < 0) ? 0 : 0x1fff;
				if ((fogFactor = czram[NATIVE_ENDIAN_VALUE_LE_BE(3,0)^cz]) != 0)
					rgbint_blend(&rgb, &fogColor, 0xff-fogFactor);
			}
			else if (fogFactor != 0xff) // direct
				rgbint_blend(&rgb, &fogColor, fogFactor);

			// apply shading after fog
			int shade = i*ooz;
			rgbint_scale_immediate_and_clamp(&rgb, shade << 2);

			if( polyfade_enabled )
				rgbint_scale_channel_and_clamp(&rgb, &polyColor);

			dest[x] = rgbint_to_rgb(&rgb);
			primap[x] |= prioverchar;

			u += du;
			v += dv;
			i += di;
			z += dz;
		}
	}
} /* renderscanline_uvi_full */

static void poly3d_DrawQuad(running_machine &machine, bitmap_t *bitmap, int textureBank, int color, Poly3dVertex pv[4], int flags, int cz_adjust, int direct, int cmode )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(state->m_poly);
	poly_vertex v[4], clipv[6];
	int clipverts;
	int vertnum;

	extra->machine = &machine;
	extra->pfade_enabled = 0;
	extra->zfog_enabled = 0;
	extra->fadeFactor = 0;
	extra->fogFactor = 0;

	extra->pens = &machine.pens[(color&0x7f)<<8];
	extra->priority_bitmap = machine.priority_bitmap;
	extra->bn = textureBank;
	extra->flags = flags;
	extra->cz_adjust = cz_adjust;
	extra->cmode = cmode;
	extra->prioverchar = (state->m_mbSuperSystem22 << 1) | ((cmode & 7) == 1);

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

	if( state->m_mbSuperSystem22 )
	{
		// global fade
		if (mixer.flags&1)
		{
			extra->fadeFactor = mixer.fadeFactor;
			rgb_comp_to_rgbint(&extra->fadeColor, mixer.rFadeColor, mixer.gFadeColor, mixer.bFadeColor);
		}

		// poly fade
		extra->pfade_enabled = mixer.PolyFade_enabled;
		rgb_comp_to_rgbint(&extra->polyColor, mixer.rPolyFadeColor, mixer.gPolyFadeColor, mixer.bPolyFadeColor);

		/* poly fog (not completely accurate yet)

        czram contents, it's basically a big cz compare table

        testmode:
            o_16          0    1    2    3    4    5    6    7 <  >   f8   f9   fa   fb   fc   fd   fe   ff
            czram[0] = 1fff 1fdf 1fbf 1f9f 1f7f 1f5f 1f3f 1f1f .... 00ff 00df 00bf 009f 007f 005f 003f 001f
            czram[1] = 0000 0000 0000 0001 0002 0003 0005 0007 .... 1e45 1e83 1ec2 1f01 1f40 1f7f 1fbf 1fff
            czram[2] = 003f 007f 00be 00fd 013c 017b 01b9 01f7 .... 1ff9 1ffb 1ffc 1ffd 1ffe 1fff 1fff 1fff
            czram[3] = 0000 001f 003f 005f 007f 009f 00bf 00df .... 1eff 1f1f 1f3f 1f5f 1f7f 1f9f 1fbf 1fdf

        airco22b demo mode, fog color: 76 9a c3
            o_16          0    1    2    3    4    5    6    7 <  >   f8   f9   fa   fb   fc   fd   fe   ff
            czram[0] = 0000 00e4 0141 0189 01c6 01fb 022c 0258 .... 13bb 13c4 13cd 13d6 13df 13e7 13f0 13f9

        alpinerd (1st course), fog color: c8 c8 c8
            o_16          0    1    2    3    4    5    6    7 <  >   ec   ed   ee   ef   f0   f1   f2 - ff
            czram[0] = 00c8 00ca 00cc 00ce 00d0 00d2 00d4 00d6 .... 02a0 02a2 02a4 02a6 02a8 1fff 1fff ....

        alpinr2b (1st course), fog color: ff ff ff
        alpinr2b start of race: - gets gradually filled from left to right, initial contents filled with 1fff? - game should be foggy here
            o_16          0    1    2    3    4    5    6    7 <  >   67   68   69   6a   6b   6c   6d - ff
            czram[0] = 01cd 01d7 01e1 01eb 01f5 01ff 0209 0213 .... 05d3 05dd 05e7 05f1 05fb 1fff 1fff ....
            other banks unused, zerofilled
        alpinr2b mid race: - gets gradually filled from right to left, initial contents above - game should not be foggy here
            o_16          0    1    2    3    4    5    6    7 <  >   ec   ed   ee   ef   f0   f1   f2 - ff
            czram[0] = 1ffe 1fff 1fff 1fff 1fff 1fff 1fff 1fff .... 1fff 1fff 1fff 1fff 1fff 1fff 1fff 1fff

        cybrcycc (1st course), fog color: 80 80 c0 - 2nd course has same cz table, but fog color 00 00 00
            o_16          0    1    2    3    4    5    6    7 <  >   d4   d5   d6   d7   d8   d9   da - ff
            czram[0] = 0000 0011 0021 0031 0041 0051 0060 0061 .... 04e0 04e4 04e7 04eb 04ee 1fff 1fff ....

        tokyowar, fog color: 80 c0 ff - it uses cztype 1 too by accident? (becomes fogfactor 0)
            o_16          0    1    2    3    4    5    6    7 <  >   f8   f9   fa   fb   fc   fd   fe   ff
            czram[0] = 0000 01c5 0244 029f 02e7 0325 035b 038b .... 0eaf 0ec7 0ee0 0efc 0f1b 0f3f 0f6a 0faa
            czram[1] = 0000 0000 0000 0000 0000 0000 0000 0000 .... 0000 0000 0000 0000 0000 0000 0000 0000
            czram[2] = 0000 0000 0000 0000 0000 0000 0000 0000 .... 0000 0000 0000 0000 0000 0000 0000 0000
            czram[3] = 0000 00e8 0191 0206 0265 02b7 0301 0345 .... 1c7e 1cbc 1d00 1d4a 1d9c 1dfb 1e70 1f19

        */

		/*  czattr: - assumed that it's write-only
               0    2    4    6    8    a    c    e
            ^^^^ ^^^^ ^^^^ ^^^^                        cz offset, signed16 per cztype 0,1,2,3
                                ^^^^                   flags, nybble per cztype 3,2,1,0 - 4 probably means enable
                                     ^^^^              maincpu ram access bank
                                          ^^^^         flags, nybble per cztype 3,2,1,0 - ?
                                               ^^^^    ? (only set sometimes in timecris)
            0000 0000 0000 0000 7555 0000 00e4 0000 // testmode normal - 0=white to black(mid), 1=white to black(weak), 2=white to black(strong), 3=black to white(mid, reverse of 0)
            7fff 8000 7fff 8000 7555 0000 00e4 0000 // testmode offset - 0=black, 1=white, 2=black, 3=white
            0000 0000 0000 0000 3111 0000 00e4 0000 // testmode off    - 0=white, 1=white, 2=white, 3=white
            0000 0000 0000 0000 4444 0000 0000 0000 // propcycl solitar
            0004 0004 0004 0004 4444 0000 0000 0000 // propcycl out pool
            00a4 00a4 00a4 00a4 4444 0000 0000 0000 // propcycl in pool
            ff80 ff80 ff80 ff80 4444 0000 0000 0000 // propcycl ending
            ff80 ff80 ff80 ff80 0000 0000 0000 0000 // propcycl hs entry
            0000 0000 0000 0000 0b6c 0000 00e4 0000 // cybrcycc
            0000 0000 0000 0000 5554 0000 00e4 0000 // airco22b
            ff01 ff01 0000 0000 4444 0000 0000 0000 // alpinerd
            0000 0000 0000 0000 4455 0000 000a 0000 // alpinr2b
            8001 8001 0000 0000 1111 0000 5555 0000 // aquajet (reg 8 is either 1111 or 5555, reg c is usually interlaced)
            0000 0000 0000 0000 5554 0000 0000 0000 // tokyowar
        */
		if (~color & 0x80)
		{
			int cztype = flags&3;
			if (nthword(state->m_czattr, 4) & (4<<(cztype*4)))
			{
				int delta = (INT16)nthword(state->m_czattr, cztype);
				rgb_comp_to_rgbint(&extra->fogColor, mixer.rFogColor, mixer.gFogColor, mixer.bFogColor);
				if (direct)
				{
					int cz = ((flags&0x1fff00) + cz_adjust) >> 8;
					if (cz < 0) cz = 0;
					else if (cz > 0x1fff) cz = 0x1fff;

					int fogFactor = state->m_recalc_czram[cztype][cz] + delta;
					if (fogFactor>0)
					{
						if (fogFactor>0xff) fogFactor = 0xff;
						extra->fogFactor = fogFactor;
					}
				}
				else
				{
					extra->zfog_enabled = 1;
					extra->cz_sdelta = delta;
					extra->czram = state->m_recalc_czram[cztype];
				}
			}
		}
	}

	else
	{
		// global fade
		if (mixer.flags&1)
		{
			extra->pfade_enabled = mixer.PolyFade_enabled;
			rgb_comp_to_rgbint(&extra->polyColor, mixer.rPolyFadeColor, mixer.gPolyFadeColor, mixer.bPolyFadeColor);
		}

		// poly fog
		if (~color & 0x80)
		{
			int cztype = flags&3;
			int czcolor = cztype & nthbyte(&mixer.fog_colormask, cztype);
			rgb_comp_to_rgbint(&extra->fogColor, mixer.rFogColor_per_cztype[czcolor], mixer.gFogColor_per_cztype[czcolor], mixer.bFogColor_per_cztype[czcolor]);

			if (direct)
			{
				// direct case, cz value is preset
				int cz = ((flags&0x1fff00) + cz_adjust) >> 8;
				if (cz < 0) cz = 0;
				else if (cz > 0x1fff) cz = 0x1fff;
				extra->fogFactor = nthbyte(state->m_czram, cztype<<13|cz);
			}
			else
			{
				extra->zfog_enabled = 1;
				extra->czram = (UINT8*)&state->m_czram[cztype<<(13-2)];
			}
		}
	}

	poly_render_triangle_fan(state->m_poly, bitmap, &mClip.scissor, renderscanline_uvi_full, 4, clipverts, clipv);
}

static void renderscanline_sprite(void *destbase, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	int y_index = extent->param[1].start - extra->flipy;
	float x_index = extent->param[0].start - extra->flipx;
	float dx = extent->param[0].dpdx;
	bitmap_t *destmap = (bitmap_t *)destbase;
	const pen_t *pal = extra->pens;
	int prioverchar = extra->prioverchar;
	int alphaFactor = extra->alpha;
	int fogFactor = 0xff - extra->fogFactor;
	int fadeFactor = 0xff - extra->fadeFactor;
	rgbint fogColor = extra->fogColor;
	rgbint fadeColor = extra->fadeColor;
	UINT8 *source = (UINT8 *)extra->source + y_index * extra->line_modulo;
	UINT32 *dest = BITMAP_ADDR32(destmap, scanline, 0);
	UINT8 *primap = BITMAP_ADDR8(extra->priority_bitmap, scanline, 0);
	int x;

	for( x=extent->startx; x<extent->stopx; x++ )
	{
		int pen = source[(int)x_index];
		if( pen != 0xff )
		{
			rgbint rgb;
			rgb_to_rgbint(&rgb, pal[pen]);

			if( fogFactor != 0xff )
				rgbint_blend(&rgb, &fogColor, fogFactor);

			if( fadeFactor != 0xff )
				rgbint_blend(&rgb, &fadeColor, fadeFactor);

			if( alphaFactor != 0xff )
			{
				rgbint mix;
				rgb_to_rgbint(&mix, dest[x]);
				rgbint_blend(&rgb, &mix, alphaFactor);
			}

			dest[x] = rgbint_to_rgb(&rgb);
			primap[x] |= prioverchar;
		}
		x_index += dx;
	}
}


static void
poly3d_DrawSprite(
	bitmap_t *dest_bmp, const gfx_element *gfx, UINT32 code,
	UINT32 color, int flipx, int flipy, int sx, int sy,
	int scalex, int scaley, int cz_factor, int prioverchar, int alpha )
{
	namcos22_state *state = gfx->machine().driver_data<namcos22_state>();
	int sprite_screen_height = (scaley*gfx->height+0x8000)>>16;
	int sprite_screen_width = (scalex*gfx->width+0x8000)>>16;
	if (sprite_screen_width && sprite_screen_height)
	{
		float fsx = sx;
		float fsy = sy;
		float fwidth = gfx->width;
		float fheight = gfx->height;
		float fsw = sprite_screen_width;
		float fsh = sprite_screen_height;
		poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(state->m_poly);
		poly_vertex vert[4];

		extra->fadeFactor = 0;
		extra->fogFactor = 0;
		extra->flags = 0;

		extra->machine = &gfx->machine();
		extra->alpha = alpha;
		extra->prioverchar = 2 | prioverchar;
		extra->line_modulo = gfx->line_modulo;
		extra->flipx = flipx;
		extra->flipy = flipy;
		extra->pens = &gfx->machine().pens[gfx->color_base + gfx->color_granularity * (color&0x7f)];
		extra->priority_bitmap = gfx->machine().priority_bitmap;
		extra->source = gfx_element_get_data(gfx, code % gfx->total_elements);

		vert[0].x = fsx;
		vert[0].y = fsy;
		vert[0].p[0] = 0;
		vert[0].p[1] = 0;
		vert[1].x = fsx + fsw;
		vert[1].y = fsy;
		vert[1].p[0] = fwidth;
		vert[1].p[1] = 0;
		vert[2].x = fsx + fsw;
		vert[2].y = fsy + fsh;
		vert[2].p[0] = fwidth;
		vert[2].p[1] = fheight;
		vert[3].x = fsx;
		vert[3].y = fsy + fsh;
		vert[3].p[0] = 0;
		vert[3].p[1] = fheight;

		// global fade
		if (mixer.flags&2)
		{
			extra->fadeFactor = mixer.fadeFactor;
			rgb_comp_to_rgbint(&extra->fadeColor, mixer.rFadeColor, mixer.gFadeColor, mixer.bFadeColor);
		}

		// fog, 0xfe is a special case for sprite priority over textlayer
		if (~color & 0x80 && cz_factor > 0 && cz_factor != 0xfe)
		{
			// or does it fetch from poly-cz ram? that will break timecris though
			extra->fogFactor = cz_factor;
			rgb_comp_to_rgbint(&extra->fogColor, mixer.rFogColor, mixer.gFogColor, mixer.bFogColor);
		}

		poly_render_triangle_fan(state->m_poly, dest_bmp, &mClip.scissor, renderscanline_sprite, 2, 4, vert);
	}
} /* poly3d_DrawSprite */

static void
ApplyGamma( running_machine &machine, bitmap_t *bitmap )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	int x,y;
	if( state->m_mbSuperSystem22 )
	{ /* super system 22 */
		const UINT8 *rlut = (const UINT8 *)&state->m_gamma[0x100/4];
		const UINT8 *glut = (const UINT8 *)&state->m_gamma[0x200/4];
		const UINT8 *blut = (const UINT8 *)&state->m_gamma[0x300/4];
		for( y=0; y<bitmap->height; y++ )
		{
			UINT32 *dest = BITMAP_ADDR32(bitmap, y, 0);
			for( x=0; x<bitmap->width; x++ )
			{
				int rgb = dest[x];
				int r = rlut[NATIVE_ENDIAN_VALUE_LE_BE(3,0)^((rgb>>16)&0xff)];
				int g = glut[NATIVE_ENDIAN_VALUE_LE_BE(3,0)^((rgb>>8)&0xff)];
				int b = blut[NATIVE_ENDIAN_VALUE_LE_BE(3,0)^(rgb&0xff)];
				dest[x] = (r<<16)|(g<<8)|b;
			}
		}
	}
	else
	{ /* system 22 */
		const UINT8 *rlut = 0x000+(const UINT8 *)machine.region("user1")->base();
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


#define DSP_FIXED_TO_FLOAT( X ) (((INT16)(X))/(float)0x7fff)
#define SPRITERAM_SIZE (0x9b0000-0x980000)
#define CGRAM_SIZE 0x1e000
#define NUM_CG_CHARS ((CGRAM_SIZE*8)/(64*16)) /* 0x3c0 */

/* modal rendering properties */
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
			int cz_adjust;
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
			int cx_min, cx_max;
			int cy_min, cy_max;
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
MallocSceneNode( running_machine &machine )
{
	struct SceneNode *node = mpFreeSceneNode;
	if( node )
	{ /* use free pool */
		mpFreeSceneNode = node->nextInBucket;
	}
	else
	{
		node = auto_alloc(machine, struct SceneNode);
	}
	memset( node, 0, sizeof(*node) );
	return node;
} /* MallocSceneNode */

static struct SceneNode *
NewSceneNode( running_machine &machine, UINT32 zsortvalue24, SceneNodeType type )
{
	struct SceneNode *node = &mSceneRoot;
	struct SceneNode *prev = NULL;
	int i, hash = 0;
	for( i=0; i<24; i+=RADIX_BITS )
	{
		hash = (zsortvalue24>>20)&RADIX_MASK;
		struct SceneNode *next = node->data.nonleaf.next[hash];
		if( !next )
		{ /* lazily allocate tree node for this radix */
			next = MallocSceneNode(machine);
			next->type = eSCENENODE_NONLEAF;
			node->data.nonleaf.next[hash] = next;
		}
		prev = node;
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
		leaf->nextInBucket = node;
		prev->data.nonleaf.next[hash] = leaf;
		return leaf;
	}
} /* NewSceneNode */


static void RenderSprite(running_machine &machine, bitmap_t *bitmap, struct SceneNode *node )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	int tile = node->data.sprite.tile;
	int col,row;
	int i = 0;
	for( row=0; row<node->data.sprite.numrows; row++ )
	{
		for( col=0; col<node->data.sprite.numcols; col++ )
		{
			int code = tile;
			if( node->data.sprite.linkType == 0xff )
				code += i;
			else
				code += nthword( &state->m_spriteram[0x800/4], i+node->data.sprite.linkType*4 );

			poly3d_DrawSprite(
				bitmap,
				machine.gfx[GFX_SPRITE],
				code,
				node->data.sprite.color,
				node->data.sprite.flipx,
				node->data.sprite.flipy,
				node->data.sprite.xpos+col*node->data.sprite.sizex,
				node->data.sprite.ypos+row*node->data.sprite.sizey,
				(node->data.sprite.sizex<<16)/32,
				(node->data.sprite.sizey<<16)/32,
				node->data.sprite.cz,
				node->data.sprite.pri,
				0xff - node->data.sprite.translucency
			);
			i++;
		} /* next col */
	} /* next row */
} /* RenderSprite */

static void RenderSceneHelper(running_machine &machine, bitmap_t *bitmap, struct SceneNode *node )
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
						node->data.quad3d.vh
					);
					poly3d_DrawQuad(machine,
						bitmap,
						node->data.quad3d.textureBank,
						node->data.quad3d.color,
						node->data.quad3d.v,
						node->data.quad3d.flags,
						node->data.quad3d.cz_adjust,
						node->data.quad3d.direct,
						node->data.quad3d.cmode
					);
					break;

				case eSCENENODE_SPRITE:
					sprite_Clip(
						node->data.sprite.cx_min,
						node->data.sprite.cx_max,
						node->data.sprite.cy_min,
						node->data.sprite.cy_max
					);
					RenderSprite(machine, bitmap, node );
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

static void RenderScene(running_machine &machine, bitmap_t *bitmap )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	struct SceneNode *node = &mSceneRoot;
	int i;
	for( i=RADIX_BUCKETS-1; i>=0; i-- )
	{
		RenderSceneHelper(machine, bitmap, node->data.nonleaf.next[i] );
		node->data.nonleaf.next[i] = NULL;
	}
	poly3d_NoClip();
	poly_wait(state->m_poly, "DrawPolygons");
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
GetPolyData( namcos22_state *state, INT32 addr )
{
	UINT32 result;
	if( addr<0 || addr>=state->m_mPtRomSize )
	{
		// point ram, only used in ram test?
		if( state->m_mbSuperSystem22 )
		{
			if( addr>=0xf80000 && addr<=0xf9ffff )
				result = state->m_mpPointRAM[addr-0xf80000];
			else return -1;
		}
		else
		{
			if( addr>=0xf00000 && addr<=0xf1ffff )
				result = state->m_mpPointRAM[addr-0xf00000];
			else return -1;
		}
	}
	else
	{
		result = (state->m_mpPolyH[addr]<<16)|(state->m_mpPolyM[addr]<<8)|state->m_mpPolyL[addr];
	}
	if( result&0x00800000 )
		result |= 0xff000000; /* sign extend */
	return result;
} /* GetPolyData */

UINT32
namcos22_point_rom_r( running_machine &machine, offs_t offs )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	return GetPolyData(state, offs & 0x00ffffff);
}


WRITE32_HANDLER( namcos22s_czram_w )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	int bank = nthword(state->m_czattr,0xa/2)&3;
	UINT16 *czram = state->m_banked_czram[bank];
	UINT32 dat = (czram[offset*2]<<16)|czram[offset*2+1];
	UINT32 prev = dat;
	COMBINE_DATA( &dat );
	czram[offset*2] = dat>>16;
	czram[offset*2+1] = dat&0xffff;
	state->m_cz_was_written[bank] |= (prev^dat);
}

READ32_HANDLER( namcos22s_czram_r )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	int bank = nthword(state->m_czattr,0xa/2)&3;
	const UINT16 *czram = state->m_banked_czram[bank];
	return (czram[offset*2]<<16)|czram[offset*2+1];
}

static void namcos22s_recalc_czram( running_machine &machine )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	int i, j, table;
	for (table=0; table<4; table++)
	{
		// as documented above, ss22 czram is 'just' a big compare table
		// this is very slow when emulating, so let's recalculate it to a simpler lookup table
		if (state->m_cz_was_written[table])
		{
			const UINT16 *src = state->m_banked_czram[table];
			UINT8 *dest = state->m_recalc_czram[table];
			int small_val = 0x2000;
			int small_offset = 0;
			int large_val = 0;
			int large_offset = 0;
			int prev = 0x2000;

			for (i=0; i<0x100; i++)
			{
				int val = src[i];

				// discard if larger than 1fff
				if (val>0x1fff) val = prev;
				if (prev>0x1fff)
				{
					prev = val;
					continue;
				}

				int start = prev;
				int end = val;
				if (start>end)
				{
					start = val;
					end = prev;
				}
				prev = val;

				// fill range
				for (j=start; j<end; j++)
					dest[j] = i;

				// remember largest/smallest for later
				if (val<small_val)
				{
					small_val = val;
					small_offset = i;
				}
				if (val>large_val)
				{
					large_val = val;
					large_offset = i;
				}
			}

			// fill possible leftover ranges
			for (j=0; j<small_val; j++)
				dest[j] = small_offset;
			for (j=large_val; j<0x2000; j++)
				dest[j] = large_offset;

			state->m_cz_was_written[table] = 0;
		}
	}
}


static void
InitXYAttrToPixel( namcos22_state *state )
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
				state->m_mXYAttrToPixel[attr][x][y] = (iy<<4)|ix;
			}
		}
	}
} /* InitXYAttrToPixel */

static void
PatchTexture( namcos22_state *state )
{
	int i;
	switch( state->m_gametype )
	{
		case NAMCOS22_RIDGE_RACER:
		case NAMCOS22_RIDGE_RACER2:
		case NAMCOS22_ACE_DRIVER:
		case NAMCOS22_CYBER_COMMANDO:
			for( i=0; i<0x100000; i++ )
			{
				int tile = state->m_mpTextureTileMap16[i];
				int attr = state->m_mpTextureTileMapAttr[i];
				if( (attr&0x1)==0 )
				{
					tile = (tile&0x3fff)|0x8000;
					state->m_mpTextureTileMap16[i] = tile;
				}
			}
			break;

		default:
			break;
	}
} /* PatchTexture */

void
namcos22_draw_direct_poly( running_machine &machine, const UINT16 *pSource )
{
	if (machine.video().skip_this_frame()) return;
	namcos22_state *state = machine.driver_data<namcos22_state>();
	int polys_enabled = state->m_mbSuperSystem22 ? nthbyte(state->m_gamma,0x1f)&1 : 1;
	if (!polys_enabled) return;
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
    *    -xxxxxxxxxxxxx-- ZC
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
	UINT32 zsortvalue24 = ((pSource[1]&0xfff)<<12)|(pSource[0]&0xfff);
	struct SceneNode *node = NewSceneNode(machine, zsortvalue24, eSCENENODE_QUAD3D);
	int i, cztype = pSource[3]&3;
	if( state->m_mbSuperSystem22 )
	{
		cztype ^= 3; // ? not sure, but this makes testmode look like on a pcb (only 1 pcb checked)
		node->data.quad3d.cmode       = (pSource[2]&0x00f0)>>4;
		node->data.quad3d.textureBank = (pSource[2]&0x000f);
	}
	else
	{
		node->data.quad3d.cmode       = (pSource[0+4]&0xf000)>>12;
		node->data.quad3d.textureBank = (pSource[1+4]&0xf000)>>12;
	}
	node->data.quad3d.cz_adjust = state->m_cz_adjust;
	node->data.quad3d.flags = (pSource[3]<<6&0x1fff00) | cztype;
	node->data.quad3d.color = (pSource[2]&0xff00)>>8;
	pSource += 4;
	for( i=0; i<4; i++ )
	{
		Poly3dVertex *p = &node->data.quad3d.v[i];
		if( state->m_mbSuperSystem22 )
		{
			p->u = pSource[0] >> 4;
			p->v = pSource[1] >> 4;
		}
		else
		{
			p->u = pSource[0] & 0x0fff;
			p->v = pSource[1] & 0x0fff;
		}

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
			if( state->m_mbSuperSystem22 )
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
Prepare3dTexture( running_machine &machine, void *pTilemapROM, void *pTextureROM )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	int i;
	assert( pTilemapROM && pTextureROM );
	{ /* following setup is Namco System 22 specific */
		const UINT8 *pPackedTileAttr = 0x200000 + (UINT8 *)pTilemapROM;
		UINT8 *pUnpackedTileAttr = auto_alloc_array(machine, UINT8, 0x080000*2);
		{
			InitXYAttrToPixel(state);
			state->m_mpTextureTileMapAttr = pUnpackedTileAttr;
			for( i=0; i<0x80000; i++ )
			{
				*pUnpackedTileAttr++ = (*pPackedTileAttr)>>4;
				*pUnpackedTileAttr++ = (*pPackedTileAttr)&0xf;
				pPackedTileAttr++;
			}
			state->m_mpTextureTileMap16 = (UINT16 *)pTilemapROM;
			state->m_mpTextureTileData = (UINT8 *)pTextureROM;
			PatchTexture(state);
		}
	}
} /* Prepare3dTexture */

static void
DrawSpritesHelper(
	running_machine &machine,
	bitmap_t *bitmap,
	const rectangle *cliprect,
	UINT32 *pBase,
	const UINT32 *pSource,
	const UINT32 *pPal,
	int num_sprites,
	int deltax,
	int deltay,
	int y_lowres )
{
	for( int i=0; i<num_sprites; i++ )
	{
		/*
        pSource[0]
            xxxx.xxxx.xxxx.xxxx | ----.----.----.----  x pos
            ----.----.----.---- | xxxx.xxxx.xxxx.xxxx  y pos

        pSource[1]
            xxxx.xxxx.xxxx.xxxx | ----.----.----.----  x size
            ----.----.----.---- | xxxx.xxxx.xxxx.xxxx  y size

        pSource[2]
            xxxx.x---.----.---- | ----.----.----.----  no function
            ----.-xxx.----.---- | ----.----.----.----  clip target
            ----.----.xxxx.xxxx | ----.----.----.----  linktype
            ----.----.----.---- | xxxx.xx--.----.----  no function(?) - set in airco22b
            ----.----.----.---- | ----.--x-.----.----  right justify
            ----.----.----.---- | ----.---x.----.----  bottom justify
            ----.----.----.---- | ----.----.x---.----  flipx
            ----.----.----.---- | ----.----.-xxx.----  numcols
            ----.----.----.---- | ----.----.----.x---  flipy
            ----.----.----.---- | ----.----.----.-xxx  numrows

        pSource[3]
            xxxx.xxxx.xxxx.xxxx | ----.----.----.----  tile number
            ----.----.----.---- | xxxx.xxxx.----.----  translucency
            ----.----.----.---- | ----.----.xxxx.xxxx  no function(?) - set in timecris

        pPal[0]
            xxxx.xxxx.----.---- | ----.----.----.----  no function
            ----.----.xxxx.xxxx | xxxx.xxxx.xxxx.xxxx  z pos

        pPal[1]
            xxxx.xxxx.----.---- | ----.----.----.----  no function
            ----.----.x---.---- | ----.----.----.----  cz enable
            ----.----.-xxx.xxxx | ----.----.----.----  color
            ----.----.----.---- | xxxx.xxxx.----.----  no function(?) - set in airco22b, propcycl
            ----.----.----.---- | ----.----.xxxx.xxxx  cz factor (fog aka depth cueing)
        */
		int xpos = (pSource[0]>>16) - deltax;
		int ypos = (pSource[0]&0xffff) - deltay;
		int sizex = pSource[1]>>16;
		int sizey = pSource[1]&0xffff;
		UINT32 attrs = pSource[2];
		int flipy = attrs>>3&0x1;
		int numrows = attrs&0x7;
		int linkType = (attrs&0x00ff0000)>>16;
		int flipx = (attrs>>7)&0x1;
		int numcols = (attrs>>4)&0x7;
		UINT32 code = pSource[3];
		int tile = code>>16;
		int translucency = (code&0xff00)>>8;

		UINT32 zcoord = pPal[0]&0x00ffffff;
		int color = pPal[1]>>16;
		int cz = pPal[1]&0xff;

		// priority over textlayer, trusted by testmode and timecris
		int pri = ((pPal[1] & 0xffff) == 0x00fe);

		// set window clipping
		int clip = attrs>>23&0xe;
		int cx_min = -deltax + (INT16)(pBase[0x80|clip]>>16);
		int cx_max = -deltax + (INT16)(pBase[0x80|clip]&0xffff);
		int cy_min = -deltay + (INT16)(pBase[0x81|clip]>>16);
		int cy_max = -deltay + (INT16)(pBase[0x81|clip]&0xffff);

		if (numrows == 0) numrows = 8;
		if (numcols == 0) numcols = 8;

		/* right justify */
		if (attrs & 0x0200)
			xpos -= sizex*numcols-1;

		/* bottom justify */
		if (attrs & 0x0100)
			ypos -= sizey*numrows-1;

		if (flipy)
		{
			ypos += sizey*numrows-1;
			sizey = -sizey;
		}

		if (flipx)
		{
			xpos += sizex*numcols-1;
			sizex = -sizex;
		}

		if (y_lowres)
		{
			sizey *= 2;
			ypos *= 2;
		}

		if (sizex && sizey)
		{
			struct SceneNode *node = NewSceneNode(machine, zcoord, eSCENENODE_SPRITE);

			node->data.sprite.tile = tile;
			node->data.sprite.flipx = flipx;
			node->data.sprite.flipy = flipy;
			node->data.sprite.numcols = numcols;
			node->data.sprite.numrows = numrows;
			node->data.sprite.linkType = linkType;
			node->data.sprite.xpos = xpos;
			node->data.sprite.ypos = ypos;
			node->data.sprite.cx_min = cx_min;
			node->data.sprite.cx_max = cx_max;
			node->data.sprite.cy_min = cy_min;
			node->data.sprite.cy_max = cy_max;
			node->data.sprite.sizex = sizex;
			node->data.sprite.sizey = sizey;
			node->data.sprite.translucency = translucency;
			node->data.sprite.color = color;
			node->data.sprite.cz = cz;
			node->data.sprite.pri = pri;
		}
		pSource += 4;
		pPal += 2;
	}
} /* DrawSpritesHelper */

static void
DrawSprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	UINT32 *spriteram32 = state->m_spriteram;
	const UINT32 *pSource;
	const UINT32 *pPal;

#if 0 // show reg contents
	int i;
	char msg1[0x1000]={0}, msg2[0x1000]={0};
	// 980000-98023f (spriteram header)
	for (i=0x00;i<0x02;i++) {
		sprintf(msg2,"98%04X %08X %08X %08X %08X\n",i*16,spriteram32[i*4+0],spriteram32[i*4+1],spriteram32[i*4+2],spriteram32[i*4+3]);
		strcat(msg1,msg2);
	}
	for (i=0x20;i<0x24;i++) {
		sprintf(msg2,"98%04X %08X %08X %08X %08X\n",i*16,spriteram32[i*4+0],spriteram32[i*4+1],spriteram32[i*4+2],spriteram32[i*4+3]);
		strcat(msg1,msg2);
	}
	strcat(msg1,"\n");
	// 940000-94007c (vics control)
	for (i=0x00;i<0x08;i++) {
		sprintf(msg2,"94%04X %08X %08X %08X %08X\n",i*16,state->m_vics_control[i*4+0],state->m_vics_control[i*4+1],state->m_vics_control[i*4+2],state->m_vics_control[i*4+3]);
		strcat(msg1,msg2);
	}
	if (machine.input().code_pressed(KEYCODE_S))
		popmessage("%s",msg1);
	else popmessage("[S] shows spite/vics regs");
#endif
	/*
        0x980000:   00060000 00010000 02ff0000 000007ff
                       ^                                 enable bits, 7 = disable
                        ^^^^                             base
                             ^^^^                        base + num sprites
                                 ^^^^     ^^^^           deltax
                                               ^^^^      deltay

        0x980010:   00200020 028004ff 032a0509 00000000
                    ^^^^^^^^                             character size?
                             ^^^^^^^^                    window-x related?
                                      ^^^^^^^^           window-y related?

        0x980200-0x98023f:   window clipping registers
        0x980400-0x9805ff:   hzoom table
        0x980600-0x9807ff:   vzoom table
        0x980800-0x980fff:   link table

        eight words per sprite, start address at 0x984000
        additional sorting/color data for sprite at 0x9a0000
    */

	/* 'enable' bits function:
        bit 0:      affects spritecount by 1? (alpinr2b)
        bit 1:      ??? (always set, except in alpinr2b. it's not x-resolution)
        bit 2:      y-resolution? (always set, except in cybrcycc)
        all bits set means off (aquajet) */
	int enable = spriteram32[0]>>16&7;

	int y_lowres = (enable & 4) ? 0 : 1;

	int deltax = (spriteram32[1]&0xffff) + (spriteram32[2]&0xffff) + 0x2d;
	int deltay = (spriteram32[3]>>16) + (0x2a >> y_lowres);

	int base = spriteram32[0] & 0xffff; // alpinesa/alpinr2b
	int num_sprites = (spriteram32[1]>>16) - base;
	num_sprites += (~enable & 1);

	if( num_sprites > 0 && num_sprites < 0x400 && enable != 7 )
	{
		pSource = &spriteram32[0x04000/4 + base*4];
		pPal    = &spriteram32[0x20000/4 + base*2];
		DrawSpritesHelper( machine, bitmap, cliprect, spriteram32, pSource, pPal, num_sprites, deltax, deltay, y_lowres );
	}

	/* VICS RAM provides two additional banks (also many unknown regs here) */
	/*
    0x940000 -x------       sprite chip busy?
    0x940018 xxxx----       clr.w   $940018.l

    0x940030 xxxxxxxx       0x0600000 - enable bits?
    0x940034 xxxxxxxx       0x3070b0f

    0x940040 xxxxxxxx       sprite attribute size             high bit means busy?
    0x940048 xxxxxxxx       sprite attribute list baseaddr    high bit means busy?
    0x940050 xxxxxxxx       sprite color size                 high bit means busy?
    0x940058 xxxxxxxx       sprite color list baseaddr        high bit means busy?

    0x940060..0x94007c      set#2
    */

	// where do the games store the number of sprites to be processed by vics???
	// the current default implementation (using spritelist size) is clearly wrong and causes problems in dirtdash and airco22b
	num_sprites = state->m_vics_control[0x40/4] >> 4 & 0x1ff; // no +1

	// dirtdash sprite list starts at xxx4, number of sprites is stored in xxx0, it doesn't use set#2
	if (state->m_gametype == NAMCOS22_DIRT_DASH) num_sprites = (state->m_vics_data[(state->m_vics_control[0x48/4]&0x4000)/4] & 0xff) + 1;

	if( num_sprites > 0 )
	{
		pSource = &state->m_vics_data[(state->m_vics_control[0x48/4]&0xffff)/4];
		pPal    = &state->m_vics_data[(state->m_vics_control[0x58/4]&0xffff)/4];
		DrawSpritesHelper( machine, bitmap, cliprect, spriteram32, pSource, pPal, num_sprites, deltax, deltay, y_lowres );
	}

	num_sprites = state->m_vics_control[0x60/4] >> 4 & 0x1ff; // no +1
	if( num_sprites > 0 )
	{
		pSource = &state->m_vics_data[(state->m_vics_control[0x68/4]&0xffff)/4];
		pPal    = &state->m_vics_data[(state->m_vics_control[0x78/4]&0xffff)/4];
		DrawSpritesHelper( machine, bitmap, cliprect, spriteram32, pSource, pPal, num_sprites, deltax, deltay, y_lowres );
	}
} /* DrawSprites */

READ32_HANDLER( namcos22s_vics_control_r )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	UINT32 ret = state->m_vics_control[offset];

	switch (offset*4)
	{
		// reg 0, status register?
		// high byte is read in timecris and lower half is expected to be 0
		case 0x00:
			ret = 0;
			break;

		// sprite attr/color size regs: high bit is busy/ready?
		// dirtdash reads these and waits for it to become 0
		case 0x40: case 0x50: case 0x60: case 0x70:
			ret &= 0x7fffffff;
			break;

		default:
			break;
	}
	return ret;
}

WRITE32_HANDLER( namcos22s_vics_control_w )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	COMBINE_DATA(&state->m_vics_control[offset]);
}


static void UpdatePalette(running_machine &machine)
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	int i,j;
	for( i=0; i<NAMCOS22_PALETTE_SIZE/4; i++ )
	{
		if( state->m_dirtypal[i] )
		{
			for( j=0; j<4; j++ )
			{
				int which = i*4+j;
				int r = nthbyte(machine.generic.paletteram.u32,which+0x00000);
				int g = nthbyte(machine.generic.paletteram.u32,which+0x08000);
				int b = nthbyte(machine.generic.paletteram.u32,which+0x10000);
				palette_set_color( machine,which,MAKE_RGB(r,g,b) );
			}
			state->m_dirtypal[i] = 0;
		}
	}
} /* UpdatePalette */


static TILE_GET_INFO( TextTilemapGetInfo )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	UINT16 data = nthword( state->m_textram,tile_index );
   /**
    * xxxx.----.----.---- palette select
    * ----.xx--.----.---- flip
    * ----.--xx.xxxx.xxxx code
    */
	SET_TILE_INFO( GFX_CHAR,data&0x03ff,data>>12,TILE_FLIPYX((data&0x0c00)>>10) );
}

READ32_HANDLER( namcos22_textram_r )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	return state->m_textram[offset];
}

WRITE32_HANDLER( namcos22_textram_w )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	COMBINE_DATA( &state->m_textram[offset] );
	tilemap_mark_tile_dirty( state->m_bgtilemap, offset*2 );
	tilemap_mark_tile_dirty( state->m_bgtilemap, offset*2+1 );
}

READ32_HANDLER( namcos22_tilemapattr_r )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();

	switch (offset)
	{
		case 2:
		{
			UINT16 lo,hi = (state->m_tilemapattr[offset] & 0xffff0000) >> 16;
			// assume current scanline, 0x1ff if in vblank (used in alpinesa)
			// or maybe relative to posirq?
			if (space->machine().primary_screen->vblank()) lo = 0x1ff;
			else lo = space->machine().primary_screen->vpos() >> 1;
			// dirtdash has slowdowns if high bit is clear, why??
			return hi<<16 | lo | 0x8000;
		}

		case 3:
			// don't know, maybe also scanline related
			// timecris reads it everytime the gun triggers and will decline if it's 0xffff
			return 0;

		default:
			break;
	}

	return state->m_tilemapattr[offset];
}

WRITE32_HANDLER( namcos22_tilemapattr_w )
{
	/*
    0.hiword    R/W     x offset
    0.loword    R/W     y offset, bit 9 for interlacing?(cybrcomm, tokyowar)
    1.hiword    R/W     ??? always 0x006e?
    1.loword    ?       unused?
    2.hiword    R/W     posirq scanline? - not hooked up yet
    2.loword    R       assume current scanline
    3.hiword    ?       unused?
    3.loword    R       ???
    */
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	COMBINE_DATA( &state->m_tilemapattr[offset] );
//  popmessage("%08x\n%08x\n%08x\n%08x\n",state->m_tilemapattr[0],state->m_tilemapattr[1],state->m_tilemapattr[2],state->m_tilemapattr[3]);
}


/**
 * Spot RAM affects how the text layer is blended with the scene, it is not yet known exactly how.
 * It isn't directly memory mapped, but rather ports are used to populate and poll it.
 *
 * See Time Crisis "SPOT RAM" self test for sample use, maybe also used in-game, but where?
 * It is also used in Dirt Dash night section. Other games don't seem to use it.
 *
 * 0x860000: set read and write address (TRUSTED by Tokyo Wars POST)
 * 0x860002: write data
 * 0x860004: read data
 * 0x860006: enable
*/

// tokyowar and timecris test ram 000-4ff, but the only practically usable part seems to be 000-3ff
#define SPOTRAM_SIZE (0x800)

/*
RAM looks like it is a 256 * 4 words table
testmode:
 offs: 0000 0001 0002 0003 - 03f4 03f5 03f6 03f7 03f8 03f9 03fa 03fb 03fc 03fd 03fe 03ff
 data: 00fe 00fe 00fe 00fe - 0001 0001 0001 0001 0000 0000 0000 0000 ffff ffff ffff ffff

is the high byte of each word used? it's usually 00, and in dirtdash always 02

low byte of each word:
 byte 0 looks like a blend factor
 bytes 1,2,3 a secondary brightness factor per rgb channel(?)

*/

READ32_HANDLER( namcos22s_spotram_r )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	if (offset == 1)
	{
		// read
		if (state->m_spot_read_address >= SPOTRAM_SIZE)
		{
			state->m_spot_read_address = 0;
		}
		return state->m_spotram[state->m_spot_read_address++] << 16;
	}
	return 0;
}

WRITE32_HANDLER( namcos22s_spotram_w )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	if (offset == 0)
	{
		if (ACCESSING_BITS_16_31)
		{
			// set address
			state->m_spot_read_address  = data>>(16+1);
			state->m_spot_write_address = data>>(16+1);
		}
		else
		{
			// write
			if (state->m_spot_write_address >= SPOTRAM_SIZE)
			{
				state->m_spot_write_address = 0;
			}
			state->m_spotram[state->m_spot_write_address++] = data;
		}
	}
	else
	{
		if (ACCESSING_BITS_0_15)
		{
			// enable
			state->m_spot_enable = data & 1;
		}
	}
}

static void namcos22s_mix_textlayer( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int prival )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	const pen_t *pens = machine.pens;
	UINT16 *src;
	UINT32 *dest;
	UINT8 *pri;
	int x,y;

	// prepare alpha
	UINT8 alpha_check12 = nthbyte(state->m_gamma, 0x12);
	UINT8 alpha_check13 = nthbyte(state->m_gamma, 0x13);
	UINT8 alpha_mask    = nthbyte(state->m_gamma, 0x14);
	UINT8 alpha_factor  = nthbyte(state->m_gamma, 0x15);

	// prepare spot
	int spot_flags = mixer.flags >> 16;
	bool spot_enabled = spot_flags&1 && spot_flags&0xc;
	int spot_limit = (spot_flags&2) ? mixer.spot_limit : 0xff;

	// prepare fader
	bool fade_enabled = mixer.flags&2 && mixer.fadeFactor;
	int fade_factor = 0xff - mixer.fadeFactor;
	rgbint fade_color;
	rgb_comp_to_rgbint(&fade_color, mixer.rFadeColor, mixer.gFadeColor, mixer.bFadeColor);

	// mix textlayer with poly/sprites
	for (y=0;y<480;y++)
	{
		src = BITMAP_ADDR16(state->m_mix_bitmap, y, 0);
		dest = BITMAP_ADDR32(bitmap, y, 0);
		pri = BITMAP_ADDR8(machine.priority_bitmap, y, 0);
		for (x=0;x<640;x++)
		{
			// skip if transparent or under poly/sprite
			if (pri[x] == prival)
			{
				rgbint rgb;
				rgb_to_rgbint(&rgb, pens[src[x]]);

				// apply alpha
				if (alpha_factor)
				{
					UINT8 pen = src[x]&0xff;
					if ((pen&0xf) == alpha_mask || pen == alpha_check12 || pen == alpha_check13)
					{
						rgbint mix;
						rgb_to_rgbint(&mix, dest[x]);
						rgbint_blend(&rgb, &mix, 0xff - alpha_factor);
					}
				}

				// apply spot
				if (spot_enabled)
				{
					UINT8 pen = src[x]&0xff;
					rgbint mix;
					rgb_to_rgbint(&mix, dest[x]);
					if (spot_flags & 8)
					{
						// mix with per-channel brightness
						rgbint shade;
						rgb_comp_to_rgbint(&shade,
							(0xff - (state->m_spotram[pen<<2|1] & 0xff)) << 2,
							(0xff - (state->m_spotram[pen<<2|2] & 0xff)) << 2,
							(0xff - (state->m_spotram[pen<<2|3] & 0xff)) << 2
						);
						rgbint_scale_channel_and_clamp(&mix, &shade);
					}

					int spot_factor = 0xff - (state->m_spotram[pen<<2] & 0xff);
					if (spot_factor < spot_limit)
						rgbint_blend(&rgb, &mix, spot_factor);
				}

				if (fade_enabled)
					rgbint_blend(&rgb, &fade_color, fade_factor);

				dest[x] = rgbint_to_rgb(&rgb);
			}
		}
	}
}

static void namcos22_mix_textlayer( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	const pen_t *pens = machine.pens;
	UINT16 *src;
	UINT32 *dest;
	UINT8 *pri;
	int x,y;

	// prepare fader and shadow factor
	bool fade_enabled = mixer.flags&2 && mixer.PolyFade_enabled;
	bool shadow_enabled = mixer.flags&0x100; // ? (ridgerac is the only game not using shadow)
	rgbint fade_color, rgb_mix[3];

	rgb_comp_to_rgbint(&fade_color, mixer.rPolyFadeColor, mixer.gPolyFadeColor, mixer.bPolyFadeColor);
	rgb_comp_to_rgbint(&rgb_mix[0], nthbyte(state->m_gamma, 0x08), nthbyte(state->m_gamma, 0x09), nthbyte(state->m_gamma, 0x0a)); // pen c
	rgb_comp_to_rgbint(&rgb_mix[1], nthbyte(state->m_gamma, 0x0b), nthbyte(state->m_gamma, 0x0c), nthbyte(state->m_gamma, 0x0d)); // pen d
	rgb_comp_to_rgbint(&rgb_mix[2], nthbyte(state->m_gamma, 0x0e), nthbyte(state->m_gamma, 0x0f), nthbyte(state->m_gamma, 0x10)); // pen e

	// mix textlayer with poly/sprites
	for (y=0;y<480;y++)
	{
		src = BITMAP_ADDR16(state->m_mix_bitmap, y, 0);
		dest = BITMAP_ADDR32(bitmap, y, 0);
		pri = BITMAP_ADDR8(machine.priority_bitmap, y, 0);
		for (x=0;x<640;x++)
		{
			// skip if transparent or under poly
			if (pri[x] == 2)
			{
				// apply shadow
				rgbint rgb;
				switch (src[x] & 0xff)
				{
					case 0xfc:
					case 0xfd:
					case 0xfe:
						if (shadow_enabled)
						{
							rgb_to_rgbint(&rgb, dest[x]);
							rgbint_scale_channel_and_clamp(&rgb, &rgb_mix[(src[x]&0xf)-0xc]);
							break;
						}
						// (fall through)
					default:
						rgb_to_rgbint(&rgb, pens[src[x]]);
						break;
				}

				if (fade_enabled)
					rgbint_scale_channel_and_clamp(&rgb, &fade_color);

				dest[x] = rgbint_to_rgb(&rgb);
			}
		}
	}
}

static void DrawCharacterLayer(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	int scroll_x = (state->m_tilemapattr[0]>>16) - 0x35c;
	int scroll_y = state->m_tilemapattr[0]&0xffff;

	tilemap_set_scrollx( state->m_bgtilemap,0, scroll_x & 0x3ff );
	tilemap_set_scrolly( state->m_bgtilemap,0, scroll_y & 0x3ff );
	tilemap_set_palette_offset( state->m_bgtilemap, mixer.palBase*256 );

	if (state->m_mbSuperSystem22)
	{
		tilemap_draw_primask(state->m_mix_bitmap, cliprect, state->m_bgtilemap, 0, 4, 4);
		namcos22s_mix_textlayer(machine, bitmap, cliprect, 4);
	}
	else
	{
		tilemap_draw_primask(state->m_mix_bitmap, cliprect, state->m_bgtilemap, 0, 2, 3);
		namcos22_mix_textlayer(machine, bitmap, cliprect);
	}
}

/*********************************************************************************************/


static INT32
Signed18( UINT32 value )
{
	INT32 offset = value&0x03ffff;
	if( offset&0x20000 )
	{ /* sign extend */
		offset |= ~0x03ffff;
	}
	return offset;
}

/**
 * @brief render a single quad
 *
 * @param flags
 *     00-1.----.01-0.001- ? (always set/clear)
 *     --x-.----.----.---- ?
 *     ----.xx--.----.---- cz table
 *     ----.--xx.----.---- representative z algorithm?
 *     ----.----.--x-.---- backface cull enable
 *     ----.----.----.---x ?
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
 *      xxxxxxxx -------- -------- flat shading factor
 *      -------- x------- -------- fog enable
 *      -------- -xxxxxxx -------- palette select
 *      -------- -------- xxxxxxxx unused?
 *
 * @param polygonShiftValue22
 *    0x1fbd0 - sky+sea
 *    0x0c350 - mountains
 *    0x09c40 - boats, surf, road, buildings
 *    0x07350 - guardrail
 *    0x061a8 - red car
 */
static void
BlitQuadHelper(
		running_machine &machine,
		bitmap_t *bitmap,
		unsigned color,
		unsigned addr,
		float m[4][4],
		INT32 polygonShiftValue22, /* 22 bits */
		int flags,
		int packetFormat )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	int absolutePriority = state->m_mAbsolutePriority;
	INT32 zsortvalue24;
	float zmin = 0.0f;
	float zmax = 0.0f;
	Poly3dVertex v[4];
	int i;

	for( i=0; i<4; i++ )
	{
		Poly3dVertex *pVerTex = &v[i];
		pVerTex->x = GetPolyData( state, 8+i*3+addr );
		pVerTex->y = GetPolyData( state, 9+i*3+addr );
		pVerTex->z = GetPolyData( state, 10+i*3+addr );
		TransformPoint( &pVerTex->x, &pVerTex->y, &pVerTex->z, m );
	}

	/* backface cull one-sided polygons */
	if( flags&0x0020 &&
		(v[2].x*((v[0].z*v[1].y)-(v[0].y*v[1].z)))+
		(v[2].y*((v[0].x*v[1].z)-(v[0].z*v[1].x)))+
		(v[2].z*((v[0].y*v[1].x)-(v[0].x*v[1].y))) >= 0 &&

		(v[0].x*((v[2].z*v[3].y)-(v[2].y*v[3].z)))+
		(v[0].y*((v[2].x*v[3].z)-(v[2].z*v[3].x)))+
		(v[0].z*((v[2].y*v[3].x)-(v[2].x*v[3].y))) >= 0 )
	{
		return;
	}

	for( i=0; i<4; i++ )
	{
		Poly3dVertex *pVerTex = &v[i];
		int bri;

		pVerTex->u = GetPolyData( state, 0+i*2+addr );
		pVerTex->v = GetPolyData( state, 1+i*2+addr );

		if( i==0 || pVerTex->z > zmax ) zmax = pVerTex->z;
		if( i==0 || pVerTex->z < zmin ) zmin = pVerTex->z;

		if( state->m_mLitSurfaceCount )
		{
			// lighting (prelim)
			bri = state->m_mLitSurfaceInfo[state->m_mLitSurfaceIndex%state->m_mLitSurfaceCount];
			if( state->m_mSurfaceNormalFormat == 0x6666 )
			{
				if( i==3 )
					state->m_mLitSurfaceIndex++;
			}
			else if( state->m_mSurfaceNormalFormat == 0x4000 )
				state->m_mLitSurfaceIndex++;
			else
				logerror( "unknown normal format: 0x%x\n", state->m_mSurfaceNormalFormat );
		}
		else if( packetFormat & 0x40 )
		{
			// gourad shading
			bri = (GetPolyData(state, i+addr)>>16)&0xff;
		}
		else
		{
			// flat shading
			bri = color>>16&0xff;
		}

		pVerTex->bri = bri;
	}

	if( zmin<0.0f ) zmin = 0.0f;
	if( zmax<0.0f ) zmax = 0.0f;

	switch (flags & 0x300)
	{
		case 0x000:
			zsortvalue24 = (INT32)zmin;
			break;

		case 0x100:
			zsortvalue24 = (INT32)zmax;
			break;

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
		zsortvalue24 = polygonShiftValue22 & 0x1fffff;
	else
	{
		zsortvalue24 += Signed18( polygonShiftValue22 );
		absolutePriority += (polygonShiftValue22&0x1c0000)>>18;
	}

	if( state->m_mObjectShiftValue22 & 0x200000 )
		zsortvalue24 = state->m_mObjectShiftValue22 & 0x1fffff;
	else
	{
		zsortvalue24 += Signed18( state->m_mObjectShiftValue22 );
		absolutePriority += (state->m_mObjectShiftValue22&0x1c0000)>>18;
	}

	if (zsortvalue24 < 0) zsortvalue24 = 0;
	else if (zsortvalue24 > 0x1fffff) zsortvalue24 = 0x1fffff;
	absolutePriority &= 7;
	zsortvalue24 |= (absolutePriority<<21);

	// allocate quad
	struct SceneNode *node = NewSceneNode(machine, zsortvalue24, eSCENENODE_QUAD3D);
	node->data.quad3d.cmode = (v[0].u>>12)&0xf;
	node->data.quad3d.textureBank = (v[0].v>>12)&0xf;
	node->data.quad3d.color = (color>>8)&0xff;
	node->data.quad3d.flags = flags>>10&3;
	node->data.quad3d.cz_adjust = state->m_cz_adjust;

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
} /* BlitQuadHelper */

static void
RegisterNormals( namcos22_state *state, INT32 addr, float m[4][4] )
{
	int i;
	for( i=0; i<4; i++ )
	{
		float nx = DSP_FIXED_TO_FLOAT(GetPolyData(state, addr+i*3+0));
		float ny = DSP_FIXED_TO_FLOAT(GetPolyData(state, addr+i*3+1));
		float nz = DSP_FIXED_TO_FLOAT(GetPolyData(state, addr+i*3+2));
		float dotproduct;

		/* transform normal vector */
		TransformNormal( &nx, &ny, &nz, m );
		dotproduct = nx*mCamera.lx + ny*mCamera.ly + nz*mCamera.lz;
		if( dotproduct<0.0f )
			dotproduct = 0.0f;
		state->m_mLitSurfaceInfo[state->m_mLitSurfaceCount++] = mCamera.ambient + mCamera.power*dotproduct;
	}
} /* RegisterNormals */

static void
BlitQuads( running_machine &machine, bitmap_t *bitmap, INT32 addr, float m[4][4], INT32 base )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
//  int numAdditionalNormals = 0;
	int chunkLength = GetPolyData(state, addr++);
	int finish = addr + chunkLength;

	if( chunkLength>0x100 )
		fatalerror( "bad packet length" );

	while( addr<finish )
	{
		int packetLength = GetPolyData( state, addr++ );
		int packetFormat = GetPolyData( state, addr+0 );
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
        */
		switch( packetLength )
		{
			case 0x17:
				/**
                * word 0: opcode (8a24c0)
                * word 1: flags
                * word 2: color
                */
				flags = GetPolyData(state, addr+1);
				color = GetPolyData(state, addr+2);
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
				flags = GetPolyData(state, addr+1);
				color = GetPolyData(state, addr+2);
				bias  = GetPolyData(state, addr+3);
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
//              numAdditionalNormals = GetPolyData(state, addr+2);
				state->m_mSurfaceNormalFormat = GetPolyData(state, addr+3);
				state->m_mLitSurfaceCount = 0;
				state->m_mLitSurfaceIndex = 0;
				RegisterNormals( state, addr+4, m );
				break;

			case 0x0d: /* additional normals */
				/*
                300401 (opcode)
                007b09 ffdd04 0004c2
                007a08 ffd968 0001c1
                ff8354 ffe401 000790
                ff84f7 ffdd04 0004c2
                */
				RegisterNormals( state, addr+1, m );
				break;

			default:
				break;
		}
		addr += packetLength;
	}
} /* BlitQuads */

static void
BlitPolyObject( running_machine &machine, bitmap_t *bitmap, int code, float M[4][4] )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	unsigned addr1 = GetPolyData(state, code);
	state->m_mLitSurfaceCount = 0;
	state->m_mLitSurfaceIndex = 0;
	for(;;)
	{
		INT32 addr2 = GetPolyData(state, addr1++);
		if( addr2<0 )
			break;
		BlitQuads( machine, bitmap, addr2, M, code );
	}
} /* BlitPolyObject */

/*******************************************************************************/

READ32_HANDLER( namcos22_dspram_r )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	return state->m_polygonram[offset] | 0xff000000; // only d0-23 are connected
}

WRITE32_HANDLER( namcos22_dspram_w )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	if (mem_mask & 0x00ff0000)
	{
		// sign extend or crop
		mem_mask |= 0xff000000;
		if (data & 0x00800000)
			data |= 0xff000000;
		else
			data &= 0xffffff;
	}

	COMBINE_DATA( &state->m_polygonram[offset] );
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
HandleBB0003( namcos22_state *state, const INT32 *pSource )
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

	state->m_mAbsolutePriority = pSource[0x3]>>16;
	mCamera.vx      = (INT16)(pSource[5]>>16);
	mCamera.vy      = (INT16)pSource[5];
	mCamera.zoom    = DspFloatToNativeFloat(pSource[6]);
	mCamera.vw      = DspFloatToNativeFloat(pSource[7])*mCamera.zoom;
	mCamera.vh      = DspFloatToNativeFloat(pSource[9])*mCamera.zoom;

	state->m_mViewMatrix[0][0] = DSP_FIXED_TO_FLOAT(pSource[0x0c]);
	state->m_mViewMatrix[1][0] = DSP_FIXED_TO_FLOAT(pSource[0x0d]);
	state->m_mViewMatrix[2][0] = DSP_FIXED_TO_FLOAT(pSource[0x0e]);

	state->m_mViewMatrix[0][1] = DSP_FIXED_TO_FLOAT(pSource[0x0f]);
	state->m_mViewMatrix[1][1] = DSP_FIXED_TO_FLOAT(pSource[0x10]);
	state->m_mViewMatrix[2][1] = DSP_FIXED_TO_FLOAT(pSource[0x11]);

	state->m_mViewMatrix[0][2] = DSP_FIXED_TO_FLOAT(pSource[0x12]);
	state->m_mViewMatrix[1][2] = DSP_FIXED_TO_FLOAT(pSource[0x13]);
	state->m_mViewMatrix[2][2] = DSP_FIXED_TO_FLOAT(pSource[0x14]);

	TransformNormal( &mCamera.lx, &mCamera.ly, &mCamera.lz, state->m_mViewMatrix );
} /* HandleBB0003 */

static void
Handle200002( running_machine &machine, bitmap_t *bitmap, const INT32 *pSource )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	if( state->m_mPrimitiveID>=0x45 )
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

		matrix3d_Multiply( m, state->m_mViewMatrix );
		BlitPolyObject( machine, bitmap, state->m_mPrimitiveID, m );
	}
	else if( state->m_mPrimitiveID !=0 && state->m_mPrimitiveID !=2 )
	{
		logerror( "Handle200002:unk code=0x%x\n", state->m_mPrimitiveID );
		// ridgerac title screen waving flag: 0x5
	}
} /* Handle200002 */

static void
Handle300000( namcos22_state *state, const INT32 *pSource )
{
	state->m_mViewMatrix[0][0] = DSP_FIXED_TO_FLOAT(pSource[1]);
	state->m_mViewMatrix[1][0] = DSP_FIXED_TO_FLOAT(pSource[2]);
	state->m_mViewMatrix[2][0] = DSP_FIXED_TO_FLOAT(pSource[3]);

	state->m_mViewMatrix[0][1] = DSP_FIXED_TO_FLOAT(pSource[4]);
	state->m_mViewMatrix[1][1] = DSP_FIXED_TO_FLOAT(pSource[5]);
	state->m_mViewMatrix[2][1] = DSP_FIXED_TO_FLOAT(pSource[6]);

	state->m_mViewMatrix[0][2] = DSP_FIXED_TO_FLOAT(pSource[7]);
	state->m_mViewMatrix[1][2] = DSP_FIXED_TO_FLOAT(pSource[8]);
	state->m_mViewMatrix[2][2] = DSP_FIXED_TO_FLOAT(pSource[9]);
} /* Handle300000 */

static void
Handle233002( namcos22_state *state, const INT32 *pSource )
{
   /*
    00233002
       00000000 // cz adjust (signed24)
       0003dd00 // z bias adjust
       001fffff // far plane?
       00007fff 00000000 00000000
       00000000 00007fff 00000000
       00000000 00000000 00007fff
       00000000 00000000 00000000
   */
	state->m_cz_adjust = (pSource[1] & 0x00800000) ? pSource[1] | 0xff000000 : pSource[1] & 0x00ffffff;
	state->m_mObjectShiftValue22 = pSource[2];
} /* Handle233002 */

static void
SimulateSlaveDSP( running_machine &machine, bitmap_t *bitmap )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	const INT32 *pSource = 0x300 + (INT32 *)state->m_polygonram;
	INT16 len;

	matrix3d_Identity( state->m_mViewMatrix );

	if( state->m_mbSuperSystem22 )
	{
		pSource += 4; /* FFFE 0400 */
	}
	else
	{
		pSource--;
	}

	for(;;)
	{
		INT16 next;
		state->m_mPrimitiveID = *pSource++;
		len  = (INT16)*pSource++;

		switch( len )
		{
		case 0x15:
			HandleBB0003( state, pSource ); /* define viewport */
			break;

		case 0x10:
			Handle233002( state, pSource ); /* set modal rendering options */
			break;

		case 0x0a:
			Handle300000( state, pSource ); /* modify view transform */
			break;

		case 0x0d:
			Handle200002( machine, bitmap, pSource ); /* render primitive */
			break;

		default:
			logerror( "unk 3d data(%d) addr=0x%x!", len, (int)(pSource-(INT32*)state->m_polygonram) );
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
		pSource++; /* always 0xffff */
		next = (INT16)*pSource++; /* link to next command */
		if( (next&0x7fff) != (pSource - (INT32 *)state->m_polygonram) )
		{ /* end of list */
			break;
		}
	} /* for(;;) */
} /* SimulateSlaveDSP */

static void
DrawPolygons( running_machine &machine, bitmap_t *bitmap )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	if( state->m_mbDSPisActive )
	{
		SimulateSlaveDSP( machine, bitmap );
		poly_wait(state->m_poly, "DrawPolygons");
	}
} /* DrawPolygons */

void
namcos22_enable_slave_simulation( running_machine &machine, int enable )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	state->m_mbDSPisActive = enable;
}

/*********************************************************************************************/

READ32_HANDLER( namcos22_cgram_r )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	return state->m_cgram[offset];
}

WRITE32_HANDLER( namcos22_cgram_w )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	COMBINE_DATA( &state->m_cgram[offset] );
	gfx_element_mark_dirty(space->machine().gfx[GFX_CHAR],offset/32);
}

READ32_HANDLER( namcos22_paletteram_r )
{
	return space->machine().generic.paletteram.u32[offset];
}

WRITE32_HANDLER( namcos22_paletteram_w )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	COMBINE_DATA( &space->machine().generic.paletteram.u32[offset] );
	state->m_dirtypal[offset&(0x7fff/4)] = 1;
}

static void namcos22_reset(running_machine &machine)
{
	memset(&mSceneRoot, 0, sizeof(mSceneRoot));
	mpFreeSceneNode = NULL;
}

static void namcos22_exit(running_machine &machine)
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	poly_free(state->m_poly);
}

static VIDEO_START( common )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	int code;

	state->m_mix_bitmap = auto_bitmap_alloc(machine,640,480,BITMAP_FORMAT_INDEXED16);
	state->m_bgtilemap = tilemap_create( machine, TextTilemapGetInfo,tilemap_scan_rows,16,16,64,64 );
	tilemap_set_transparent_pen(state->m_bgtilemap, 0xf);

	state->m_mbDSPisActive = 0;
	memset( state->m_polygonram, 0xcc, 0x20000 );

	for (code = 0; code < machine.gfx[GFX_TEXTURE_TILE]->total_elements; code++)
		gfx_element_decode(machine.gfx[GFX_TEXTURE_TILE], code);

	Prepare3dTexture(machine, machine.region("textilemap")->base(), machine.gfx[GFX_TEXTURE_TILE]->gfxdata );
	state->m_dirtypal = auto_alloc_array(machine, UINT8, NAMCOS22_PALETTE_SIZE/4);
	state->m_mPtRomSize = machine.region("pointrom")->bytes()/3;
	state->m_mpPolyL = machine.region("pointrom")->base();
	state->m_mpPolyM = state->m_mpPolyL + state->m_mPtRomSize;
	state->m_mpPolyH = state->m_mpPolyM + state->m_mPtRomSize;

	state->m_poly = poly_alloc(machine, 4000, sizeof(poly_extra_data), 0);
	machine.add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(namcos22_reset), &machine));
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(namcos22_exit), &machine));

	gfx_element_set_source(machine.gfx[GFX_CHAR], (UINT8 *)state->m_cgram);
}

VIDEO_START( namcos22 )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	state->m_mbSuperSystem22 = 0;
	VIDEO_START_CALL(common);
}

VIDEO_START( namcos22s )
{
	namcos22_state *state = machine.driver_data<namcos22_state>();
	state->m_mbSuperSystem22 = 1;

	// init spotram
	state->m_spotram = auto_alloc_array(machine, UINT16, SPOTRAM_SIZE);
	memset(state->m_spotram, 0, SPOTRAM_SIZE*2);

	// init czram tables
	int table;
	for (table=0; table<4; table++)
	{
		state->m_banked_czram[table] = auto_alloc_array(machine, UINT16, 0x100);
		memset(state->m_banked_czram[table], 0, 0x100*2);
		state->m_recalc_czram[table] = auto_alloc_array(machine, UINT8, 0x2000);
		memset(state->m_recalc_czram[table], 0, 0x2000);
		state->m_cz_was_written[table] = 0;
	}

	VIDEO_START_CALL(common);
}

SCREEN_UPDATE( namcos22s )
{
	namcos22_state *state = screen->machine().driver_data<namcos22_state>();
	UpdateVideoMixer(screen->machine());
	UpdatePalette(screen->machine());
	namcos22s_recalc_czram(screen->machine());
	bitmap_fill(screen->machine().priority_bitmap, cliprect, 0);

	// background color
	rgbint bg_color;
	rgb_comp_to_rgbint(&bg_color, nthbyte(state->m_gamma,0x08), nthbyte(state->m_gamma,0x09), nthbyte(state->m_gamma,0x0a));
	if (mixer.flags&1 && mixer.fadeFactor)
	{
		rgbint fade_color;
		rgb_comp_to_rgbint(&fade_color, mixer.rFadeColor, mixer.gFadeColor, mixer.bFadeColor);
		rgbint_blend(&bg_color, &fade_color, 0xff - mixer.fadeFactor);
	}
	bitmap_fill( bitmap, cliprect, rgbint_to_rgb(&bg_color));

	// layers
	UINT8 layer = nthbyte(state->m_gamma,0x1f);
	if (layer&4) DrawCharacterLayer(screen->machine(), bitmap, cliprect);
	if (layer&2) DrawSprites(screen->machine(), bitmap, cliprect);
	if (layer&1) DrawPolygons(screen->machine(), bitmap);
	RenderScene(screen->machine(), bitmap );
	if (layer&4) namcos22s_mix_textlayer(screen->machine(), bitmap, cliprect, 6);
	ApplyGamma(screen->machine(), bitmap);

	// debug stuff
#if ALLOW_MEMDUMP
	if( screen->machine().input().code_pressed(KEYCODE_D) )
	{
		FILE *f = fopen( "dump.txt", "wb" );
		if( f )
		{
			address_space *space = screen->machine().device("maincpu")->memory().space(AS_PROGRAM);

			if (1) // czram
			{
				int i,bank;
				for( bank=0; bank<4; bank++ )
				{
					fprintf( f, "czram[%d] =", bank );
					for( i=0; i<256; i++ )
					{
						fprintf( f, " %04x", state->m_banked_czram[bank][i] );
					}
					fprintf( f, "\n" );
				}
				fprintf( f, "\n" );
			}

			if (0) // spotram
			{
				int i;
				fprintf(f, "spotram:\n");
				for (i=0; i<256; i++)
				{
					fprintf(f, "%02X: %04X %04X %04X %04X\n", i, state->m_spotram[i*4+0], state->m_spotram[i*4+1], state->m_spotram[i*4+2], state->m_spotram[i*4+3]);
				}
				fprintf(f, "\n");
			}

			Dump(space, f,0x810000, 0x81000f, "cz attr" );
			Dump(space, f,0x824000, 0x8243ff, "gamma");
			//Dump(space, f,0x828000, 0x83ffff, "palette" );
			//Dump(space, f,0x8a0000, 0x8a000f, "tilemap_attr");
			//Dump(space, f,0x880000, 0x89ffff, "cgram/textram");
			//Dump(space, f,0x900000, 0x90ffff, "vics_data");
			//Dump(space, f,0x940000, 0x94007f, "vics_control");
			//Dump(space, f,0x980000, 0x9affff, "sprite374" );
			//Dump(space, f,0xc00000, 0xc1ffff, "polygonram");
			fclose( f );
		}
		while( screen->machine().input().code_pressed(KEYCODE_D) ){}
	}
#endif

//  popmessage("%08X %08X %08X %08X",state->m_czattr[0],state->m_czattr[1],state->m_czattr[2],state->m_czattr[3]);

	return 0;
}

SCREEN_UPDATE( namcos22 )
{
	UpdateVideoMixer(screen->machine());
	UpdatePalette(screen->machine());
	bitmap_fill(screen->machine().priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));
	DrawPolygons(screen->machine(), bitmap);
	RenderScene(screen->machine(), bitmap);
	DrawCharacterLayer(screen->machine(), bitmap, cliprect);
	ApplyGamma(screen->machine(), bitmap);

#if ALLOW_MEMDUMP
	if( screen->machine().input().code_pressed(KEYCODE_D) )
	{
		FILE *f = fopen( "dump.txt", "wb" );
		if( f )
		{
			address_space *space = screen->machine().device("maincpu")->memory().space(AS_PROGRAM);

			//Dump(space, f,0x90000000, 0x90000003, "led?" );
			Dump(space, f,0x90010000, 0x90017fff, "cz_ram");
			//Dump(space, f,0x900a0000, 0x900a000f, "tilemap_attr");
			Dump(space, f,0x90020000, 0x90027fff, "gamma");
			//Dump(space, f,0x70000000, 0x7001ffff, "polygonram");
			fclose( f );
		}
		while( screen->machine().input().code_pressed(KEYCODE_D) ){}
	}
#endif

	return 0;
}

WRITE16_HANDLER( namcos22_dspram16_bank_w )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	COMBINE_DATA( &state->m_dspram_bank );
}

READ16_HANDLER( namcos22_dspram16_r )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	UINT32 value = state->m_polygonram[offset];
	switch( state->m_dspram_bank )
	{
	case 0:
		value &= 0xffff;
		break;

	case 1:
		value>>=16;
		break;

	case 2:
		state->m_mUpperWordLatch = value>>16;
		value &= 0xffff;
		break;

	default:
		break;
	}
	return (UINT16)value;
} /* namcos22_dspram16_r */

WRITE16_HANDLER( namcos22_dspram16_w )
{
	namcos22_state *state = space->machine().driver_data<namcos22_state>();
	UINT32 value = state->m_polygonram[offset];
	UINT16 lo = value&0xffff;
	UINT16 hi = value>>16;
	switch( state->m_dspram_bank )
	{
	case 0:
		COMBINE_DATA( &lo );
		break;

	case 1:
		COMBINE_DATA( &hi );
		break;

	case 2:
		COMBINE_DATA( &lo );
		hi = state->m_mUpperWordLatch;
		break;

	default:
		break;
	}
	state->m_polygonram[offset] = (hi<<16)|lo;
} /* namcos22_dspram16_w */
