#include "emu.h"
#include "drawgfxm.h"
#include "includes/hng64.h"

#define MAKE_MAME_REEEEAAALLLL_SLOW 0


static void clear3d(running_machine &machine);	// TODO: Inline


static void hng64_mark_all_tiles_dirty( hng64_state *state, int tilemap )
{
	if (tilemap == 0)
	{
		state->m_tilemap0_8x8->mark_all_dirty();
		state->m_tilemap0_16x16->mark_all_dirty();
		state->m_tilemap0_16x16_alt->mark_all_dirty();
	}
	else if (tilemap == 1)
	{
		state->m_tilemap1_8x8->mark_all_dirty();
		state->m_tilemap1_16x16->mark_all_dirty();
		state->m_tilemap1_16x16_alt->mark_all_dirty();
	}
	else if (tilemap == 2)
	{
		state->m_tilemap2_8x8->mark_all_dirty();
		state->m_tilemap2_16x16->mark_all_dirty();
		state->m_tilemap2_16x16_alt->mark_all_dirty();
	}
	else if (tilemap == 3)
	{
		state->m_tilemap3_8x8->mark_all_dirty();
		state->m_tilemap3_16x16->mark_all_dirty();
		state->m_tilemap3_16x16_alt->mark_all_dirty();
	}
}

static void hng64_mark_tile_dirty( hng64_state *state, int tilemap, int tile_index )
{
	if (tilemap == 0)
	{
		state->m_tilemap0_8x8->mark_tile_dirty(tile_index);
		state->m_tilemap0_16x16->mark_tile_dirty(tile_index);
		state->m_tilemap0_16x16_alt->mark_tile_dirty(tile_index);
	}
	else if (tilemap == 1)
	{
		state->m_tilemap1_8x8->mark_tile_dirty(tile_index);
		state->m_tilemap1_16x16->mark_tile_dirty(tile_index);
		state->m_tilemap1_16x16_alt->mark_tile_dirty(tile_index);
	}
	else if (tilemap == 2)
	{
		state->m_tilemap2_8x8->mark_tile_dirty(tile_index);
		state->m_tilemap2_16x16->mark_tile_dirty(tile_index);
		state->m_tilemap2_16x16_alt->mark_tile_dirty(tile_index);
	}
	else if (tilemap == 3)
	{
		state->m_tilemap3_8x8->mark_tile_dirty(tile_index);
		state->m_tilemap3_16x16->mark_tile_dirty(tile_index);
		state->m_tilemap3_16x16_alt->mark_tile_dirty(tile_index);
	}
}


#define PIXEL_OP_REMAP_TRANSPEN_PRIORITY_ADDIIVE32(DEST, PRIORITY, SOURCE)					\
do																					\
{																					\
	UINT32 srcdata = (SOURCE);														\
	if (srcdata != transpen)														\
	{																				\
		if (((1 << ((PRIORITY) & 0x1f)) & pmask) == 0)								\
		{																			\
			UINT32 srcdata2 = paldata[srcdata];										\
																					\
			UINT32 add;                                                             \
			add = (srcdata2 & 0x00ff0000) + (DEST & 0x00ff0000);                    \
			if (add & 0x01000000) DEST = (DEST & 0xff00ffff) | (0x00ff0000);        \
			else DEST = (DEST & 0xff00ffff) | (add & 0x00ff0000);                   \
			add = (srcdata2 & 0x000000ff) + (DEST & 0x000000ff);                    \
			if (add & 0x00000100) DEST = (DEST & 0xffffff00) | (0x000000ff);        \
			else DEST = (DEST & 0xffffff00) | (add & 0x000000ff);                   \
			add = (srcdata2 & 0x0000ff00) + (DEST & 0x0000ff00);                    \
			if (add & 0x00010000) DEST = (DEST & 0xffff00ff) | (0x0000ff00);        \
			else DEST = (DEST & 0xffff00ff) | (add & 0x0000ff00);                   \
		}																			\
		(PRIORITY) = 31;															\
	}																				\
}																					\
while (0)																			\


static void pdrawgfx_transpen_additive(bitmap_rgb32 &dest, const rectangle &cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_ind8 &priority, UINT32 pmask, UINT32 transpen)
{
	const pen_t *paldata;

	assert(dest.valid());
	assert(dest.bpp() == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine().pens[gfx->color_base + gfx->color_granularity * color];

	/* use pen usage to optimize */
	if (gfx->pen_usage != NULL && !gfx->dirty[code])
	{
		UINT32 usage = gfx->pen_usage[code];

		/* fully transparent; do nothing */
		if ((usage & ~(1 << transpen)) == 0)
			return;
	}

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_PRIORITY_ADDIIVE32, UINT8);
}


static void pdrawgfxzoom_transpen_additive(bitmap_rgb32 &dest, const rectangle &cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_ind8 &priority, UINT32 pmask,
		UINT32 transpen)
{
	const pen_t *paldata;

	/* non-zoom case */

	if (scalex == 0x10000 && scaley == 0x10000)
	{
		pdrawgfx_transpen_additive(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, priority, pmask, transpen);
		return;
	}

	assert(dest.valid());
	assert(dest.bpp() == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine().pens[gfx->color_base + gfx->color_granularity * color];

	/* use pen usage to optimize */
	if (gfx->pen_usage != NULL && !gfx->dirty[code])
	{
		UINT32 usage = gfx->pen_usage[code];

		/* fully transparent; do nothing */
		if ((usage & ~(1 << transpen)) == 0)
			return;
	}

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_PRIORITY_ADDIIVE32, UINT8);
}


/*
 * Sprite Format
 * ------------------
 *
 * UINT32 | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *   0    | yyyy yyyy yyyy yyyy xxxx xxxx xxxx xxxx | x/y position
 *   1    | YYYY YYYY YYYY YYYY XXXX XXXX XXXX XXXX | x/y zoom (*)
 *   2    | ---- -zzz zzzz zzzz ---- ---I cccc CCCC | Z-buffer value, 'Inline' chain flag, x/y chain
 *   3    | ---- ---- pppp pppp ---- ---- ---- ---- | palette entry
 *   4    | mmmm -?fF a??? tttt tttt tttt tttt tttt | mosaic factor, unknown (**) , flip bits, additive blending, unknown (***), tile number
 *   5    | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *   6    | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *   7    | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *
 * (*) Fatal Fury WA standard elements are 0x1000-0x1000, all the other games sets 0x100-0x100, related to the bit 27 of sprite regs 0?
 * (**) setted by black squares in ranking screen in Samurai Shodown 64 1, sprite disable?
 * (***) bit 22 is setted on some Fatal Fury WA snow (not all of them), bit 21 is setted on Xrally how to play elements in attract mode
 *
 * Sprite Global Registers
 * -----------------------
 *
 * UINT32 | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *   0    | ---- z--- b--- ---- ---- ---- ---- ---- | zooming mode, bpp select
 *   1    | yyyy yyyy yyyy yyyy xxxx xxxx xxxx xxxx | global sprite offset (ss64 rankings in attract)
 *   2    | ---- ---- ---- ---- ---- ---- ---- ---- |
 *   3    | ---- ---- ---- ---- ---- ---- ---- ---- |
 *   4    | ---- ---- ---- ---- ---- ---- ---- ---- |
 * (anything else is unknown at the current time)
 * Notes:
 * [0]
 * 0xf0000000 setted in both Samurai Shodown
 * 0x00060000 always setted in all the games
 * 0x00010000 setted in POST, sprite disable?
 * [4]
 * 0x0e0 in Samurai Shodown/Xrally games, 0x1c0 in all the others, zooming factor?
 */

static void draw_sprites(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	hng64_state *state = machine.driver_data<hng64_state>();
	const gfx_element *gfx;
	UINT32 *source = state->m_spriteram;
	UINT32 *finish = state->m_spriteram + 0xc000/4;

	// global offsets in sprite regs
	int spriteoffsx = (state->m_spriteregs[1]>>0)&0xffff;
	int spriteoffsy = (state->m_spriteregs[1]>>16)&0xffff;

#if 0
	for (int iii = 0; iii < 0x0f; iii++)
		mame_printf_debug("%.8x ", state->m_videoregs[iii]);
	mame_printf_debug("\n");
#endif

	while( source<finish )
	{
		int tileno,chainx,chainy,xflip;
		int pal,xinc,yinc,yflip;
		UINT16 xpos, ypos;
		int xdrw,ydrw;
		int chaini;
		int zbuf;
		UINT32 zoomx,zoomy;
		float foomX, foomY;
		int blend;
		int disable;



		ypos = (source[0]&0xffff0000)>>16;
		xpos = (source[0]&0x0000ffff)>>0;
		xpos += (spriteoffsx);
		ypos += (spriteoffsy);

		tileno= (source[4]&0x0007ffff);
		blend=  (source[4]&0x00800000);
		yflip=  (source[4]&0x01000000)>>24;
		xflip=  (source[4]&0x02000000)>>25;
		disable=(source[4]&0x04000000)>>26; // ss64 rankings?

		pal =(source[3]&0x00ff0000)>>16;

		chainy=(source[2]&0x0000000f);
		chainx=(source[2]&0x000000f0)>>4;
		chaini=(source[2]&0x00000100);
		zbuf = (source[2]&0x07ff0000)>>16; //?

		zoomy = (source[1]&0xffff0000)>>16;
		zoomx = (source[1]&0x0000ffff)>>0;

		#if 1
		if(zbuf == 0x7ff) //temp kludge to avoid garbage on screen
		{
			source+=8;
			continue;
		}
		#endif
		if(disable)
		{
			source+=8;
			continue;
		}

#if 0
		if (!(source[4] == 0x00000000 || source[4] == 0x000000aa))
			mame_printf_debug("unknown : %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x \n", source[0], source[1], source[2], source[3],
				source[4], source[5], source[6], source[7]);
#endif

		/* Calculate the zoom */
		{
			int zoom_factor;

			/* FIXME: regular zoom mode has precision bugs, can be easily seen in Samurai Shodown 64 intro */
			zoom_factor = (state->m_spriteregs[0] & 0x08000000) ? 0x1000 : 0x100;
			if(!zoomx) zoomx=zoom_factor;
			if(!zoomy) zoomy=zoom_factor;

			/* First, prevent any possible divide by zero errors */
			foomX = (float)(zoom_factor) / (float)zoomx;
			foomY = (float)(zoom_factor) / (float)zoomy;

			zoomx = ((int)foomX) << 16;
			zoomy = ((int)foomY) << 16;

			zoomx += (int)((foomX - floor(foomX)) * (float)0x10000);
			zoomy += (int)((foomY - floor(foomY)) * (float)0x10000);
		}

		if (state->m_spriteregs[0] & 0x00800000) //bpp switch
		{
			gfx= machine.gfx[4];
		}
		else
		{
			gfx= machine.gfx[5];
			tileno>>=1;
			pal&=0xf;
		}

		// Accommodate for chaining and flipping
		if(xflip)
		{
			xinc=-(int)(16.0f*foomX);
			xpos-=xinc*chainx;
		}
		else
		{
			xinc=(int)(16.0f*foomX);
		}

		if(yflip)
		{
			yinc=-(int)(16.0f*foomY);
			ypos-=yinc*chainy;
		}
		else
		{
			yinc=(int)(16.0f*foomY);
		}

#if 0
		if (((source[2] & 0xffff0000) >> 16) == 0x0001)
		{
			popmessage("T %.8x %.8x %.8x %.8x %.8x", source[0], source[1], source[2], source[3], source[4]);
			//popmessage("T %.8x %.8x %.8x %.8x %.8x", source[0], source[1], source[2], source[3], source[4]);
		}
#endif

		for(ydrw=0;ydrw<=chainy;ydrw++)
		{
			for(xdrw=0;xdrw<=chainx;xdrw++)
			{
				INT16 drawx = xpos+(xinc*xdrw);
				INT16 drawy = ypos+(yinc*ydrw);

				// 0x3ff (0x200 sign bit) based on sams64_2 char select
				drawx &= 0x3ff;
				drawy &= 0x3ff;

				if (drawx&0x0200)drawx-=0x400;
				if (drawy&0x0200)drawy-=0x400;

				if (!chaini)
				{
					if (!blend) pdrawgfxzoom_transpen(bitmap,cliprect,gfx,tileno,pal,xflip,yflip,drawx,drawy,zoomx,zoomy/*0x10000*/,machine.priority_bitmap, 0,0);
					else pdrawgfxzoom_transpen_additive(bitmap,cliprect,gfx,tileno,pal,xflip,yflip,drawx,drawy,zoomx,zoomy/*0x10000*/,machine.priority_bitmap, 0,0);
					tileno++;
				}
				else // inline chain mode, used by ss64
				{

					tileno=(source[4]&0x0007ffff);
					pal =(source[3]&0x00ff0000)>>16;

					if (state->m_spriteregs[0] & 0x00800000) //bpp switch
					{
						gfx= machine.gfx[4];
					}
					else
					{
						gfx= machine.gfx[5];
						tileno>>=1;
						pal&=0xf;
					}

					if (!blend) pdrawgfxzoom_transpen(bitmap,cliprect,gfx,tileno,pal,xflip,yflip,drawx,drawy,zoomx,zoomy/*0x10000*/,machine.priority_bitmap, 0,0);
					else pdrawgfxzoom_transpen_additive(bitmap,cliprect,gfx,tileno,pal,xflip,yflip,drawx,drawy,zoomx,zoomy/*0x10000*/,machine.priority_bitmap, 0,0);
					source +=8;
				}

			}
		}

		if (!chaini) source +=8;
	}
}


/* Transition Control Video Registers
 * ----------------------------------
 *
 * UINT32 | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *      0 |                                         |
 *      1 | xxxx xxxx xxxx xxxx yyyy yyyy yyyy yyyy | Min X / Min Y visible area rectangle values
 *      2 | xxxx xxxx xxxx xxxx yyyy yyyy yyyy yyyy | Max X / Max Y visible area rectangle values (added up with the Min X / Min Y)
 *      3 |                                         |
 *      4 |                                         |
 *      5 | ---- ---- ---- ---? ---- --?? ???? ???? | Global Fade In/Fade Out control
 *      6 |                                         |
 *      7 | ---- ---- xxxx xxxx xxxx xxxx xxxx xxxx | Port A of RGB fade (subtraction)
 *      8 |                                         |
 *      9 | ---- ---- ---- ---? ---- ---- ---- ???? | Per-layer Fade In/Fade Out control
 *     10 | ---- ---- xxxx xxxx xxxx xxxx xxxx xxxx | Port B of RGB fade (additive)
 *     11 | xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx | Unknown - looks like an ARGB value - it seems to change when the scene changes
 *     12 |                                         |
 *     13 |                                         |
 *     14 |                                         |
 *     15 |                                         |
 *     16 |                                         |
 *     17 |                                         |
 *     18 | xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx | V-Blank related stuff
 *     19 |                                         |
 *     20 | ---- ---- ---- ---x ---- ---- ---- ---- | Back layer control register?
 *     21 |                                         |
 *     22 |                                         |
 *     23 |                                         |
 *     24 |                                         |
 *
 *
 *
 *  Various bits change depending on what is happening in the scene.
 *  These bits may set which 'layer' is affected by the blending.
 *  Or maybe they adjust the scale of the lightening and darkening...
 *  Or maybe it switches from fading by scaling to fading using absolute addition and subtraction...
 *  Or maybe they set transition type (there seems to be a cute scaling-squares transition in there somewhere)...
 */

/* this is broken for the 'How to Play' screen in Buriki after attract, disabled for now */
static void transition_control(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	hng64_state *state = machine.driver_data<hng64_state>();
	UINT32 *hng64_tcram = state->m_tcram;
	int i, j;

//  float colorScaleR, colorScaleG, colorScaleB;
//  float finR, finG, finB;
	INT32 finR, finG, finB;

	INT32 darkR, darkG, darkB;
	INT32 brigR, brigG, brigB;

	// If either of the fading memory regions is non-zero...
	if (hng64_tcram[0x00000007] != 0x00000000 || hng64_tcram[0x0000000a] != 0x00000000)
	{
		darkR = (INT32)( hng64_tcram[0x00000007]        & 0xff);
		darkG = (INT32)((hng64_tcram[0x00000007] >> 8)  & 0xff);
		darkB = (INT32)((hng64_tcram[0x00000007] >> 16) & 0xff);

		brigR = (INT32)( hng64_tcram[0x0000000a]        & 0xff);
		brigG = (INT32)((hng64_tcram[0x0000000a] >> 8)  & 0xff);
		brigB = (INT32)((hng64_tcram[0x0000000a] >> 16) & 0xff);

		for (i = cliprect.min_x; i < cliprect.max_x; i++)
		{
			for (j = cliprect.min_y; j < cliprect.max_y; j++)
			{
				UINT32* thePixel = &bitmap.pix32(j, i);

				finR = (INT32)RGB_RED(*thePixel);
				finG = (INT32)RGB_GREEN(*thePixel);
				finB = (INT32)RGB_BLUE(*thePixel);

#if 0
				// Apply the darkening pass (0x07)...
				colorScaleR = 1.0f - (float)( hng64_tcram[0x00000007] & 0xff)        / 255.0f;
				colorScaleG = 1.0f - (float)((hng64_tcram[0x00000007] >> 8)  & 0xff) / 255.0f;
				colorScaleB = 1.0f - (float)((hng64_tcram[0x00000007] >> 16) & 0xff) / 255.0f;

				finR = ((float)RGB_RED(*thePixel)   * colorScaleR);
				finG = ((float)RGB_GREEN(*thePixel) * colorScaleG);
				finB = ((float)RGB_BLUE(*thePixel)  * colorScaleB);


				// Apply the lightening pass (0x0a)...
				colorScaleR = 1.0f + (float)( hng64_tcram[0x0000000a] & 0xff)        / 255.0f;
				colorScaleG = 1.0f + (float)((hng64_tcram[0x0000000a] >> 8)  & 0xff) / 255.0f;
				colorScaleB = 1.0f + (float)((hng64_tcram[0x0000000a] >> 16) & 0xff) / 255.0f;

				finR *= colorScaleR;
				finG *= colorScaleG;
				finB *= colorScaleB;


				// Clamp
				if (finR > 255.0f) finR = 255.0f;
				if (finG > 255.0f) finG = 255.0f;
				if (finB > 255.0f) finB = 255.0f;
#endif


				// Subtractive fading
				if (hng64_tcram[0x00000007] != 0x00000000)
				{
					finR -= darkR;
					finG -= darkG;
					finB -= darkB;
				}

				// Additive fading
				if (hng64_tcram[0x0000000a] != 0x00000000)
				{
					finR += brigR;
					finG += brigG;
					finB += brigB;
				}

				// Clamp the high end
				if (finR > 255) finR = 255;
				if (finG > 255) finG = 255;
				if (finB > 255) finB = 255;

				// Clamp the low end
				if (finR < 0) finR = 0;
				if (finG < 0) finG = 0;
				if (finB < 0) finB = 0;

				*thePixel = MAKE_ARGB(255, (UINT8)finR, (UINT8)finG, (UINT8)finB);
			}
		}
	}
}

// make this a function!
// pppppppp ff--atttt tttttttt tttttttt
#define HNG64_GET_TILE_INFO                                                    \
{                                                                              \
	hng64_state *state = machine.driver_data<hng64_state>();                 \
	UINT16 tilemapinfo = (state->m_videoregs[reg]>>shift)&0xffff;                 \
	int tileno,pal, flip;                                                      \
                                                                               \
	tileno = state->m_videoram[tile_index+(offset/4)];                            \
                                                                               \
	pal = (tileno&0xff000000)>>24;                                             \
	flip =(tileno&0x00c00000)>>22;                                             \
                                                                               \
	if (tileno&0x200000)                                                       \
	{                                                                          \
		tileno = (tileno & state->m_videoregs[0x0b]) | state->m_videoregs[0x0c];     \
	}                                                                          \
                                                                               \
	tileno &= 0x1fffff;                                                        \
                                                                               \
	if (size==0)                                                               \
	{                                                                          \
		if (tilemapinfo&0x400)                                                 \
		{                                                                      \
			SET_TILE_INFO(1,tileno>>1,pal>>4,TILE_FLIPYX(flip));               \
		}                                                                      \
		else                                                                   \
		{                                                                      \
			SET_TILE_INFO(0,tileno, pal,TILE_FLIPYX(flip));                    \
		}                                                                      \
	}                                                                          \
	else                                                                       \
	{                                                                          \
		if (tilemapinfo&0x400)                                                 \
		{                                                                      \
			SET_TILE_INFO(3,tileno>>3,pal>>4,TILE_FLIPYX(flip));               \
		}                                                                      \
		else                                                                   \
		{                                                                      \
			SET_TILE_INFO(2,tileno>>2, pal,TILE_FLIPYX(flip));                 \
		}                                                                      \
	}                                                                          \
}                                                                              \
	                                                                           \

static TILE_GET_INFO( get_hng64_tile0_8x8_info )
{
	int offset = 0x00000;
	int size = 0;
	int reg = 0x02;
	int shift = 16;

	HNG64_GET_TILE_INFO
}

static TILE_GET_INFO( get_hng64_tile0_16x16_info )
{
	int offset = 0x00000;
	int size = 1;
	int reg = 0x02;
	int shift = 16;

	HNG64_GET_TILE_INFO
}

static TILE_GET_INFO( get_hng64_tile1_8x8_info )
{
	int offset = 0x10000;
	int size = 0;
	int reg = 0x02;
	int shift = 0;

	HNG64_GET_TILE_INFO
}

static TILE_GET_INFO( get_hng64_tile1_16x16_info )
{
	int offset = 0x10000;
	int size = 1;
	int reg = 0x02;
	int shift = 0;

	HNG64_GET_TILE_INFO
}

static TILE_GET_INFO( get_hng64_tile2_8x8_info )
{
	int offset = 0x20000;
	int size = 0;
	int reg = 0x03;
	int shift = 16;

	HNG64_GET_TILE_INFO
}

static TILE_GET_INFO( get_hng64_tile2_16x16_info )
{
	int offset = 0x20000;
	int size = 1;
	int reg = 0x03;
	int shift = 16;

	HNG64_GET_TILE_INFO
}

static TILE_GET_INFO( get_hng64_tile3_8x8_info )
{
	int offset = 0x30000;
	int size = 0;
	int reg = 0x03;
	int shift = 0;

	HNG64_GET_TILE_INFO
}

static TILE_GET_INFO( get_hng64_tile3_16x16_info )
{
	int offset = 0x30000;
	int size = 1;
	int reg = 0x03;
	int shift = 0;

	HNG64_GET_TILE_INFO
}


WRITE32_MEMBER(hng64_state::hng64_videoram_w)
{
	int realoff;
	hng64_state *state = machine().driver_data<hng64_state>();
	COMBINE_DATA(&m_videoram[offset]);

	realoff = offset*4;

	if ((realoff>=0) && (realoff<0x10000))
	{
		hng64_mark_tile_dirty(state, 0, offset&0x3fff);
	}
	else if ((realoff>=0x10000) && (realoff<0x20000))
	{
		hng64_mark_tile_dirty(state, 1, offset&0x3fff);
	}
	else if ((realoff>=0x20000) && (realoff<0x30000))
	{
		hng64_mark_tile_dirty(state, 2, offset&0x3fff);
	}
	else if ((realoff>=0x30000) && (realoff<0x40000))
	{
		hng64_mark_tile_dirty(state, 3, offset&0x3fff);
	}

//  if ((realoff>=0x40000)) mame_printf_debug("offsw %08x %08x\n",realoff,data);

	/* 400000 - 7fffff is scroll regs etc. */
}

/* internal set of transparency states for rendering */
typedef enum
{
	HNG64_TILEMAP_NORMAL = 1,
	HNG64_TILEMAP_ADDITIVE,
	HNG64_TILEMAP_ALPHA
} hng64trans_t;


typedef struct _blit_parameters blit_parameters;
struct _blit_parameters
{
	bitmap_rgb32 *			bitmap;
	rectangle			cliprect;
	UINT32				tilemap_priority_code;
	UINT8				mask;
	UINT8				value;
	UINT8				alpha;
	hng64trans_t		drawformat;
};



static void hng64_configure_blit_parameters(blit_parameters *blit, tilemap_t *tmap, bitmap_rgb32 &dest, const rectangle &cliprect, UINT32 flags, UINT8 priority, UINT8 priority_mask, hng64trans_t drawformat)
{
	/* start with nothing */
	memset(blit, 0, sizeof(*blit));

	/* set the target bitmap */
	blit->bitmap = &dest;

	/* if we have a cliprect, copy */
	blit->cliprect = cliprect;

	/* set the priority code and alpha */
	//blit->tilemap_priority_code = priority | (priority_mask << 8) | (tmap->palette_offset << 16); // fixit
	blit->alpha = (flags & TILEMAP_DRAW_ALPHA_FLAG) ? (flags >> 24) : 0xff;

	blit->drawformat = drawformat;

	/* tile priority; unless otherwise specified, draw anything in layer 0 */
	blit->mask = TILEMAP_PIXEL_CATEGORY_MASK;
	blit->value	= flags & TILEMAP_PIXEL_CATEGORY_MASK;

	/* if no layers specified, draw layer 0 */
	if ((flags & (TILEMAP_DRAW_LAYER0 | TILEMAP_DRAW_LAYER1 | TILEMAP_DRAW_LAYER2)) == 0)
		flags |= TILEMAP_DRAW_LAYER0;

	/* OR in the bits from the draw masks */
	blit->mask |= flags & (TILEMAP_DRAW_LAYER0 | TILEMAP_DRAW_LAYER1 | TILEMAP_DRAW_LAYER2);
	blit->value |= flags & (TILEMAP_DRAW_LAYER0 | TILEMAP_DRAW_LAYER1 | TILEMAP_DRAW_LAYER2);

	/* for all-opaque rendering, don't check any of the layer bits */
	if (flags & TILEMAP_DRAW_OPAQUE)
	{
		blit->mask &= ~(TILEMAP_PIXEL_LAYER0 | TILEMAP_PIXEL_LAYER1 | TILEMAP_PIXEL_LAYER2);
		blit->value &= ~(TILEMAP_PIXEL_LAYER0 | TILEMAP_PIXEL_LAYER1 | TILEMAP_PIXEL_LAYER2);
	}

	/* don't check category if requested */
	if (flags & TILEMAP_DRAW_ALL_CATEGORIES)
	{
		blit->mask &= ~TILEMAP_PIXEL_CATEGORY_MASK;
		blit->value &= ~TILEMAP_PIXEL_CATEGORY_MASK;
	}
}

INLINE UINT32 alpha_additive_r32(UINT32 d, UINT32 s, UINT8 level)
{
	UINT32 add;
	add = (s & 0x00ff0000) + (d & 0x00ff0000);
	if (add & 0x01000000) d = (d & 0xff00ffff) | (0x00ff0000);
	else d = (d & 0xff00ffff) | (add & 0x00ff0000);
	add = (s & 0x000000ff) + (d & 0x000000ff);
	if (add & 0x00000100) d = (d & 0xffffff00) | (0x000000ff);
	else d = (d & 0xffffff00) | (add & 0x000000ff);
	add = (s & 0x0000ff00) + (d & 0x0000ff00);
	if (add & 0x00010000) d = (d & 0xffff00ff) | (0x0000ff00);
	else d = (d & 0xffff00ff) | (add & 0x0000ff00);
	return d;
}


/*-------------------------------------------------
    tilemap_draw_roz_core - render the tilemap's
    pixmap to the destination with rotation
    and zoom
-------------------------------------------------*/

#define HNG64_ROZ_PLOT_PIXEL(INPUT_VAL)											        \
do {																		        \
	if (blit->drawformat == HNG64_TILEMAP_NORMAL)									\
		*(UINT32 *)dest = clut[INPUT_VAL];									        \
	else if (blit->drawformat == HNG64_TILEMAP_ADDITIVE)			                \
		*(UINT32 *)dest = alpha_additive_r32(*(UINT32 *)dest, clut[INPUT_VAL], alpha);	\
	else if (blit->drawformat == HNG64_TILEMAP_ALPHA)		                        \
		*(UINT32 *)dest = alpha_blend_r32(*(UINT32 *)dest, clut[INPUT_VAL], alpha);	\
} while (0)

static void hng64_tilemap_draw_roz_core(running_machine& machine, tilemap_t *tmap, const blit_parameters *blit,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, int wraparound)
{
	const pen_t *clut = &machine.pens[blit->tilemap_priority_code >> 16];
	bitmap_ind8 &priority_bitmap = machine.priority_bitmap;
	bitmap_rgb32 &destbitmap = *blit->bitmap;
	bitmap_ind16 &srcbitmap = tmap->pixmap();
	bitmap_ind8 &flagsmap = tmap->flagsmap();
	const int xmask = srcbitmap.width()-1;
	const int ymask = srcbitmap.height()-1;
	const int widthshifted = srcbitmap.width() << 16;
	const int heightshifted = srcbitmap.height() << 16;
	UINT32 priority = blit->tilemap_priority_code;
	UINT8 mask = blit->mask;
	UINT8 value = blit->value;
	UINT8 alpha = blit->alpha;
	UINT32 cx;
	UINT32 cy;
	int x;
	int sx;
	int sy;
	int ex;
	int ey;
	UINT32 *dest;
	UINT8 *pri;
	const UINT16 *src;
	const UINT8 *maskptr;

	/* pre-advance based on the cliprect */
	startx += blit->cliprect.min_x * incxx + blit->cliprect.min_y * incyx;
	starty += blit->cliprect.min_x * incxy + blit->cliprect.min_y * incyy;

	/* extract start/end points */
	sx = blit->cliprect.min_x;
	sy = blit->cliprect.min_y;
	ex = blit->cliprect.max_x;
	ey = blit->cliprect.max_y;

	/* optimized loop for the not rotated case */
	if (incxy == 0 && incyx == 0 && !wraparound)
	{
		/* skip without drawing until we are within the bitmap */
		while (startx >= widthshifted && sx <= ex)
		{
			startx += incxx;
			sx++;
		}

		/* early exit if we're done already */
		if (sx > ex)
			return;

		/* loop over rows */
		while (sy <= ey)
		{
			/* only draw if Y is within range */
			if (starty < heightshifted)
			{
				/* initialize X counters */
				x = sx;
				cx = startx;
				cy = starty >> 16;

				/* get source and priority pointers */
				pri = &priority_bitmap.pix8(sy, sx);
				src = &srcbitmap.pix16(cy);
				maskptr = &flagsmap.pix8(cy);
				dest = &destbitmap.pix32(sy, sx);

				/* loop over columns */
				while (x <= ex && cx < widthshifted)
				{
					/* plot if we match the mask */
					if ((maskptr[cx >> 16] & mask) == value)
					{
						HNG64_ROZ_PLOT_PIXEL(src[cx >> 16]);
						*pri = (*pri & (priority >> 8)) | priority;
					}

					/* advance in X */
					cx += incxx;
					x++;
					dest++;
					pri++;
				}
			}

			/* advance in Y */
			starty += incyy;
			sy++;
		}
	}

	/* wraparound case */
	else if (wraparound)
	{
		/* loop over rows */
		while (sy <= ey)
		{
			/* initialize X counters */
			x = sx;
			cx = startx;
			cy = starty;

			/* get dest and priority pointers */
			dest = &destbitmap.pix32(sy, sx);
			pri = &priority_bitmap.pix8(sy, sx);

			/* loop over columns */
			while (x <= ex)
			{
				/* plot if we match the mask */
				if ((flagsmap.pix8((cy >> 16) & ymask, (cx >> 16) & xmask) & mask) == value)
				{
					HNG64_ROZ_PLOT_PIXEL(srcbitmap.pix16((cy >> 16) & ymask, (cx >> 16) & xmask));
					*pri = (*pri & (priority >> 8)) | priority;
				}

				/* advance in X */
				cx += incxx;
				cy += incxy;
				x++;
				dest++;
				pri++;
			}

			/* advance in Y */
			startx += incyx;
			starty += incyy;
			sy++;
		}
	}

	/* non-wraparound case */
	else
	{
		/* loop over rows */
		while (sy <= ey)
		{
			/* initialize X counters */
			x = sx;
			cx = startx;
			cy = starty;

			/* get dest and priority pointers */
			dest = &destbitmap.pix32(sy, sx);
			pri = &priority_bitmap.pix8(sy, sx);

			/* loop over columns */
			while (x <= ex)
			{
				/* plot if we're within the bitmap and we match the mask */
				if (cx < widthshifted && cy < heightshifted)
					if ((flagsmap.pix8(cy >> 16, cx >> 16) & mask) == value)
					{
						HNG64_ROZ_PLOT_PIXEL(srcbitmap.pix16(cy >> 16, cx >> 16));
						*pri = (*pri & (priority >> 8)) | priority;
					}

				/* advance in X */
				cx += incxx;
				cy += incxy;
				x++;
				dest++;
				pri++;
			}

			/* advance in Y */
			startx += incyx;
			starty += incyy;
			sy++;
		}
	}
}



static void hng64_tilemap_draw_roz_primask(running_machine& machine, bitmap_rgb32 &dest, const rectangle &cliprect, tilemap_t *tmap,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy,
		int wraparound, UINT32 flags, UINT8 priority, UINT8 priority_mask, hng64trans_t drawformat)
{
	blit_parameters blit;

/* notes:
   - startx and starty MUST be UINT32 for calculations to work correctly
   - srcbitmap->width and height are assumed to be a power of 2 to speed up wraparound
   */

	/* skip if disabled */
	//if (!tmap->enable)
	//  return;

g_profiler.start(PROFILER_TILEMAP_DRAW_ROZ);
	/* configure the blit parameters */
	hng64_configure_blit_parameters(&blit, tmap, dest, cliprect, flags, priority, priority_mask, drawformat);

	/* get the full pixmap for the tilemap */
	tmap->pixmap();

	/* then do the roz copy */
	hng64_tilemap_draw_roz_core(machine, tmap, &blit, startx, starty, incxx, incxy, incyx, incyy, wraparound);
g_profiler.stop();
}


INLINE void hng64_tilemap_draw_roz(running_machine& machine, bitmap_rgb32 &dest, const rectangle &cliprect, tilemap_t *tmap,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy,
		int wraparound, UINT32 flags, UINT8 priority, hng64trans_t drawformat)
{
	hng64_tilemap_draw_roz_primask(machine, dest, cliprect, tmap, startx, starty, incxx, incxy, incyx, incyy, wraparound, flags, priority, 0xff, drawformat);
}



static void hng64_drawtilemap(running_machine& machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int tm )
{
	hng64_state *state = machine.driver_data<hng64_state>();
	UINT32 *hng64_videoregs = state->m_videoregs;
	UINT32 *hng64_videoram = state->m_videoram;
	tilemap_t* tilemap = 0;
	UINT32 scrollbase = 0;
	UINT32 tileregs = 0;
	int transmask;
	UINT32 global_tileregs = hng64_videoregs[0x00];

	int debug_blend_enabled = 0;

	int global_dimensions = (global_tileregs&0x03000000)>>24;

	if ( (state->m_additive_tilemap_debug&(1 << tm)))
		debug_blend_enabled = 1;

	if ((global_dimensions != 0) && (global_dimensions != 3))
		popmessage("unsupported global_dimensions on tilemaps");

	if (tm==0)
	{
		scrollbase = (hng64_videoregs[0x04]&0x3fff0000)>>16;
		tileregs   = (hng64_videoregs[0x02]&0xffff0000)>>16;

		if (global_dimensions==0)
		{
			if (tileregs&0x0200)	tilemap = state->m_tilemap0_16x16;
			else tilemap = state->m_tilemap0_8x8;
		}
		else
		{
			if (tileregs&0x0200)	tilemap = state->m_tilemap0_16x16_alt;
			else tilemap = state->m_tilemap0_8x8; // _alt
		}
	}
	else if (tm==1)
	{
		scrollbase = (hng64_videoregs[0x04]&0x00003fff)>>0;
		tileregs   = (hng64_videoregs[0x02]&0x0000ffff)>>0;

		if (global_dimensions==0)
		{
			if (tileregs&0x0200)	tilemap = state->m_tilemap1_16x16;
			else tilemap = state->m_tilemap1_8x8;
		}
		else
		{
			if (tileregs&0x0200)	tilemap = state->m_tilemap1_16x16_alt;
			else tilemap = state->m_tilemap1_8x8; // _alt
		}
	}
	else if (tm==2)
	{
		scrollbase = (hng64_videoregs[0x05]&0x3fff0000)>>16;
		tileregs   = (hng64_videoregs[0x03]&0xffff0000)>>16;

		if (global_dimensions==0)
		{
			if (tileregs&0x0200)	tilemap = state->m_tilemap2_16x16;
			else tilemap = state->m_tilemap2_8x8;
		}
		else
		{
			if (tileregs&0x0200)	tilemap = state->m_tilemap2_16x16_alt;
			else tilemap = state->m_tilemap2_8x8; // _alt
		}
	}
	else if (tm==3)
	{
		scrollbase = (hng64_videoregs[0x05]&0x00003fff)>>0;
		tileregs   = (hng64_videoregs[0x03]&0x0000ffff)>>0;

		if (global_dimensions==0)
		{
			if (tileregs&0x0200)	tilemap = state->m_tilemap3_16x16;
			else tilemap = state->m_tilemap3_8x8;
		}
		else
		{
			if (tileregs&0x0200)	tilemap = state->m_tilemap3_16x16_alt;
			else tilemap = state->m_tilemap3_8x8; // _alt
		}
	}

	// xrally's pink tilemaps make me think this is a tilemap enable bit.
	// fatfurwa makes me think otherwise.
	//if (!(tileregs & 0x0040)) return;

	// set the transmask so our manual copy is correct
	if (tileregs & 0x0400)
		transmask = 0xff;
	else
		transmask = 0xf;

	// buriki tm1 = roz

	// my life would be easier if the roz we're talking about for complex zoom wasn't setting this as well
	if ((tileregs & 0x0800)==0x0000) // floor mode
	{
		/* Floor mode - per pixel simple / complex modes? -- every other line?
          (there doesn't seem to be enough data in Buriki for every line at least)
        */
		//if ((tileregs&0xf000) == 0x1000)
		//{
		//  popmessage("Floor is Active");
		//}
		int line;
		rectangle clip;
		INT32 xtopleft,xmiddle;
		INT32 ytopleft,ymiddle;
		int xinc,yinc;

		const rectangle &visarea = machine.primary_screen->visible_area();
		clip = visarea;

		if (global_tileregs&0x04000000) // globally selects alt scroll register layout???
		{
			/* logic would dictate that this should be the 'complex' scroll register layout,
               but per-line.  That doesn't work however.

               You only have line data for the number of lines on the screen, not enough for
               the complex register layout

               HOWEVER, using the code below doesn't work either.  This might be because
               they have mosaic turned on, and it adopts a new meaning in linescroll modes?

               The code below could also be wrong, and rowscroll simply acts the same in all
               modes, this is hard to know because ss64_2 barely uses it.

            */

			popmessage("Unhandled rowscroll %02x", tileregs>>12);
		}
		else // 'simple' mode with linescroll, used in some ss64_2 levels (assumed to be correct, but doesn't do much with it.. so could be wrong)
		{
			for (line=0;line<448;line++)
			{
				clip.min_y = clip.max_y = line;

				if (hng64_videoregs[0x00]&0x00010000) // disable all scrolling / zoom (test screen) (maybe)
				{
					/* If this bit is active the scroll registers don't seem valid at all?
                       It either disables zooming, or disables use of the scroll registers completely
                       - used at startup
                   */

					xtopleft = 0;
					xmiddle = 256<<16;

					ytopleft = 0;
					ymiddle = 256<<16;
				}
				else
				{
					xtopleft = (hng64_videoram[(0x40000+(line*0x10)+(scrollbase<<4))/4]);
					xmiddle   = (hng64_videoram[(0x40004+(line*0x10)+(scrollbase<<4))/4]); // middle screen point
					ytopleft = (hng64_videoram[(0x40008+(line*0x10)+(scrollbase<<4))/4]);
					ymiddle   = (hng64_videoram[(0x4000c+(line*0x10)+(scrollbase<<4))/4]); // middle screen point
				}

				xinc = (xmiddle - xtopleft) / 512;
				yinc = (ymiddle - ytopleft) / 512;

				hng64_tilemap_draw_roz(machine, bitmap,clip,tilemap,xtopleft,ytopleft,
						xinc<<1,0,0,yinc<<1,
						1,
						0,0, debug_blend_enabled?HNG64_TILEMAP_ADDITIVE:HNG64_TILEMAP_NORMAL);


			}
		}
	}
	else
	{
		if ((tileregs&0xf000))
			popmessage("Tilemap Mosaic? %02x", tileregs>>12);
		// 0x1000 is set up the buriki 2nd title screen with rotating logo and in fatal fury at various times?

		if (global_tileregs&0x04000000) // globally selects alt scroll register layout???
		{
			/* complex zoom mode? */
			/* with this scroll register layout rotation effects are possible
               the most obvious use of rotation is the Buriki One logo after
               attract mode; the text around the outside of the logo is rotated
               onto the screen

               see 1:32 in http://www.youtube.com/watch?v=PoYaHOILuGs

               Xtreme Rally seems to have an issue with this mode on the communication check
               screen at startup, but according to videos that should scroll, and no scroll
               values are updated, so it might be an unrelated bug.

            */

			INT32 xtopleft,xmiddle, xalt;
			INT32 ytopleft,ymiddle, yalt;
			int xinc, xinc2, yinc, yinc2;

			if (0)
				if (tm==2)
					popmessage("X %08x X %08x X %08x Y %08x Y %08x Y %08x",
						hng64_videoram[(0x40000+(scrollbase<<4))/4],
						hng64_videoram[(0x40004+(scrollbase<<4))/4],
						hng64_videoram[(0x40010+(scrollbase<<4))/4],
						/*hng64_videoram[(0x40014+(scrollbase<<4))/4],*/  // unused? (dupe value on fatfurwa, 00 on rest)

						hng64_videoram[(0x40008+(scrollbase<<4))/4],
						hng64_videoram[(0x40018+(scrollbase<<4))/4],
						hng64_videoram[(0x4000c+(scrollbase<<4))/4]);
						/*hng64_videoram[(0x4001c+(scrollbase<<4))/4]);*/ // unused? (dupe value on fatfurwa, 00 on rest)



			xtopleft  = (hng64_videoram[(0x40000+(scrollbase<<4))/4]);
			xalt      = (hng64_videoram[(0x40004+(scrollbase<<4))/4]); // middle screen point
			xmiddle   = (hng64_videoram[(0x40010+(scrollbase<<4))/4]);

			ytopleft     = (hng64_videoram[(0x40008+(scrollbase<<4))/4]);
			yalt         = (hng64_videoram[(0x40018+(scrollbase<<4))/4]); // middle screen point
			ymiddle      = (hng64_videoram[(0x4000c+(scrollbase<<4))/4]);

			xinc = (xmiddle - xtopleft) / 512;
			yinc = (ymiddle - ytopleft) / 512;
			xinc2 = (xalt-xtopleft) / 512;
			yinc2 = (yalt-ytopleft) /512;


			/* manual copy = slooow */
			if (MAKE_MAME_REEEEAAALLLL_SLOW)
			{
				bitmap_ind16 &bm = tilemap->pixmap();
				int bmheight = bm.height();
				int bmwidth = bm.width();
				const pen_t *paldata = machine.pens;
				UINT32* dstptr;
				UINT16* srcptr;
				int xx,yy;


				int tmp = xtopleft;
				int tmp2 = ytopleft;
				//printf("start %08x end %08x start %08x end %08x\n", xtopleft, xmiddle, ytopleft, ymiddle);

				for (yy=0;yy<448;yy++)
				{

					dstptr = &bitmap.pix32(yy);

					tmp = xtopleft;
					tmp2 = ytopleft;

					for (xx=0;xx<512;xx++)
					{
						int realsrcx = (xtopleft>>16)&(bmwidth-1);
						int realsrcy = (ytopleft>>16)&(bmheight-1);
						UINT16 pen;

						srcptr = &bm.pix16(realsrcy);

						pen = srcptr[realsrcx];

						if (pen&transmask)
							*dstptr = paldata[pen];

						xtopleft+= xinc<<1;
						ytopleft+= yinc2<<1;
						++dstptr;
					}

					ytopleft = tmp2 + (yinc<<1);
					xtopleft = tmp + (xinc2<<1);
				}
			}
			else
			{
				hng64_tilemap_draw_roz(machine, bitmap,cliprect,tilemap,xtopleft,ytopleft,
						xinc<<1,yinc2<<1,xinc2<<1,yinc<<1,
						1,
						0,0, debug_blend_enabled?HNG64_TILEMAP_ADDITIVE:HNG64_TILEMAP_NORMAL);
			}

		}
		else
		{
			/* simple zoom mode? - only 4 regs? */
			/* in this mode they can only specify the top left and middle screen points for each tilemap,
               this allows simple zooming, but not rotation */

			INT32 xtopleft,xmiddle;
			INT32 ytopleft,ymiddle;
			int xinc,yinc;

			if (0)
				if (tm==2)
					popmessage("%08x %08x %08x %08x",
						hng64_videoram[(0x40010+(scrollbase<<4))/4],
						hng64_videoram[(0x40014+(scrollbase<<4))/4],
						hng64_videoram[(0x40018+(scrollbase<<4))/4],
						hng64_videoram[(0x4001c+(scrollbase<<4))/4]);

			if (hng64_videoregs[0x00]&0x00010000) // disable all scrolling / zoom (test screen) (maybe)
			{
				/* If this bit is active the scroll registers don't seem valid at all?
                   It either disables zooming, or disables use of the scroll registers completely
                   - used at startup
               */

				xtopleft = 0;
				xmiddle = 256<<16;

				ytopleft = 0;
				ymiddle = 256<<16;
			}
			else
			{

				xtopleft = (hng64_videoram[(0x40000+(scrollbase<<4))/4]);
				xmiddle   = (hng64_videoram[(0x40004+(scrollbase<<4))/4]); // middle screen point
				ytopleft = (hng64_videoram[(0x40008+(scrollbase<<4))/4]);
				ymiddle   = (hng64_videoram[(0x4000c+(scrollbase<<4))/4]); // middle screen point
			}

			xinc = (xmiddle - xtopleft) / 512;
			yinc = (ymiddle - ytopleft) / 512;

			/* manual copy = slooow */
			if (MAKE_MAME_REEEEAAALLLL_SLOW)
			{
				bitmap_ind16 &bm = tilemap->pixmap();
				int bmheight = bm.height();
				int bmwidth = bm.width();
				const pen_t *paldata = machine.pens;
				UINT32* dstptr;
				UINT16* srcptr;
				int xx,yy;

				int tmp = xtopleft;

				//printf("start %08x end %08x start %08x end %08x\n", xtopleft, xmiddle, ytopleft, ymiddle);

				for (yy=0;yy<448;yy++)
				{
					int realsrcy = (ytopleft>>16)&(bmheight-1);

					dstptr = &bitmap.pix32(yy);
					srcptr = &bm.pix16(realsrcy);

					xtopleft = tmp;

					for (xx=0;xx<512;xx++)
					{
						int realsrcx = (xtopleft>>16)&(bmwidth-1);

						UINT16 pen;

						pen = srcptr[realsrcx];

						if (pen&transmask)
							*dstptr = paldata[pen];

						xtopleft+= xinc<<1;
						++dstptr;
					}

					ytopleft+= yinc<<1;
				}
			}
			else
			{
				hng64_tilemap_draw_roz(machine, bitmap,cliprect,tilemap,xtopleft,ytopleft,
						xinc<<1,0,0,yinc<<1,
						1,
						0,0, debug_blend_enabled?HNG64_TILEMAP_ADDITIVE:HNG64_TILEMAP_NORMAL);
			}
		}
	}
}



/*
 * Video Regs Format
 * ------------------
 *
 * UINT32 | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *   0    | ---- -C-- ---- -??Z ---- ---- ---- ---- | unknown (scroll control?) C = Global Complex zoom, ? = Always Set?, Z = Global Zoom Disable?
            0000 0011  - road edge alt 1
            0000 0111  - road edge alt 2
 *   1    | xxxx xxxx xxxx xxxx ---- ---- ---- ---- | looks like it's 0001 most (all) of the time - turns off in buriki intro
 *   1    | ---- ---- ---- ---- oooo oooo oooo oooo | unknown - always seems to be 0000 (fatfurwa)
 *   2    | xxxx xxxx xxxx xxxx ---- ---- ---- ---- | tilemap0 per layer flags
 *   2    | ---- ---- ---- ---- xxxx xxxx xxxx xxxx | tilemap1 per layer flags
 *   3    | xxxx xxxx xxxx xxxx ---- ---- ---- ---- | tilemap2 per layer flags
 *   3    | ---- ---- ---- ---- xxxx xxxx xxxx xxxx | tilemap3 per layer flags
 *   4    | xxxx xxxx xxxx xxxx ---- ---- ---- ---- | tilemap0 offset into tilemap RAM?
 *   4    | ---- ---- ---- ---- xxxx xxxx xxxx xxxx | tilemap1 offset into tilemap RAM
 *   5    | xxxx xxxx xxxx xxxx ---- ---- ---- ---- | tilemap3 offset into tilemap RAM
 *   5    | ---- ---- ---- ---- xxxx xxxx xxxx xxxx | tilemap4 offset into tilemap RAM?
 *   6    | oooo oooo oooo oooo oooo oooo oooo oooo | unknown - always seems to be 000001ff (fatfurwa)
 *   7    | oooo oooo oooo oooo oooo oooo oooo oooo | unknown - always seems to be 000001ff (fatfurwa)
 *   8    | oooo oooo oooo oooo oooo oooo oooo oooo | unknown - always seems to be 80008000 (fatfurwa)
 *   9    | oooo oooo oooo oooo oooo oooo oooo oooo | unknown - always seems to be 00000000 (fatfurwa)
 *   a    | oooo oooo oooo oooo oooo oooo oooo oooo | unknown - always seems to be 00000000 (fatfurwa)
 *   b    | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm | auto animation mask for tilemaps, - use these bits from the original tile number
 *   c    | xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx | auto animation bits for tilemaps, - merge in these bits to auto animate the tilemap
 *   d    | oooo oooo oooo oooo oooo oooo oooo oooo | not used ??
 *   e    | oooo oooo oooo oooo oooo oooo oooo oooo | not used ??

    per tile regs (0x2/0x3)

    // tilemap0 per layer flags
    // 0840 - startup tests, 8x8x4 layer
    // 0cc0 - beast busters 2, 8x8x8 layer
    // 0860 - fatal fury wa
    // 08e0 - fatal fury wa during transitions
    // 0940 - samurai shodown 64
    // 0880 - buriki

    // mmmm dbr? ??e? ????
    // m = mosaic related?
    //  -- they seem to enable mosaic at the same time as rowscroll in several cases (floor in buriki / ff)
    //     and also on the rotating logo in buriki.. does it cause some kind of aliasing side-effect, or.. ?
    // r = tile size (seems correct)
    // b = 4bpp/8bpp (seems correct) (beast busters, samsh64, sasm64 2, xrally switch it for some screens)
    // d = line (floor) mode - buriki, fatafurwa, some backgrounds in ss64_2
    // e = enable according to sams64_2 debug mode, buriki and xrally.. but NOT fatal fury :-(


 */



#define IMPORTANT_DIRTY_TILEFLAG_MASK (0x0600)

SCREEN_UPDATE_RGB32( hng64 )
{
	hng64_state *state = screen.machine().driver_data<hng64_state>();
	UINT32 *hng64_videoregs = state->m_videoregs;
	UINT32 *hng64_videoram = state->m_videoram;
	UINT32 *hng64_tcram = state->m_tcram;
	UINT32 animmask;
	UINT32 animbits;
	UINT16 tileflags0, tileflags1;
	UINT16 tileflags2, tileflags3;

#if 0
	// press in sams64_2 attract mode for a nice debug screen from the game
	// not sure how functional it is, and it doesn't appear to test everything (rowscroll modes etc.)
	// but it could be useful
	if ( screen.machine().input().code_pressed_once(KEYCODE_L) )
	{
		address_space *space = screen.machine().device("maincpu")->memory().space(AS_PROGRAM);
		space->write_byte(0x2f27c8, 0x2);
	}
#endif

	bitmap.fill(hng64_tcram[0x50/4] & 0x10000 ? get_black_pen(screen.machine()) : screen.machine().pens[0], cliprect); //FIXME: Is the register correct? check with HW tests
	screen.machine().priority_bitmap.fill(0x00, cliprect);

	if (state->m_screen_dis)
		return 0;

	animmask = hng64_videoregs[0x0b];
	animbits = hng64_videoregs[0x0c];
	tileflags0 = hng64_videoregs[0x02]>>16;
	tileflags1 = hng64_videoregs[0x02]&0xffff;
	tileflags2 = hng64_videoregs[0x03]>>16;
	tileflags3 = hng64_videoregs[0x03]&0xffff;

	/* if the auto-animation mask or bits have changed search for tiles using them and mark as dirty */
	if ((state->m_old_animmask != animmask) || (state->m_old_animbits != animbits))
	{
		int tile_index;
		for (tile_index=0;tile_index<128*128;tile_index++)
		{
			if (hng64_videoram[tile_index+(0x00000/4)]&0x200000)
			{
				hng64_mark_tile_dirty(state, 0, tile_index);
			}
			if (hng64_videoram[tile_index+(0x10000/4)]&0x200000)
			{
				hng64_mark_tile_dirty(state, 1, tile_index);
			}
			if (hng64_videoram[tile_index+(0x20000/4)]&0x200000)
			{
				hng64_mark_tile_dirty(state, 2, tile_index);
			}
			if (hng64_videoram[tile_index+(0x30000/4)]&0x200000)
			{
				hng64_mark_tile_dirty(state, 3, tile_index);
			}
		}

		state->m_old_animmask = animmask;
		state->m_old_animbits = animbits;
	}

	if ((state->m_old_tileflags0&IMPORTANT_DIRTY_TILEFLAG_MASK)!=(tileflags0&IMPORTANT_DIRTY_TILEFLAG_MASK))
	{
		hng64_mark_all_tiles_dirty(state, 0);
		state->m_old_tileflags0 = tileflags0;
	}

	if ((state->m_old_tileflags1&IMPORTANT_DIRTY_TILEFLAG_MASK)!=(tileflags1&IMPORTANT_DIRTY_TILEFLAG_MASK))
	{
		hng64_mark_all_tiles_dirty(state, 1);
		state->m_old_tileflags1 = tileflags1;
	}

	if ((state->m_old_tileflags2&IMPORTANT_DIRTY_TILEFLAG_MASK)!=(tileflags2&IMPORTANT_DIRTY_TILEFLAG_MASK))
	{
		hng64_mark_all_tiles_dirty(state, 2);
		state->m_old_tileflags2 = tileflags2;
	}

	if ((state->m_old_tileflags3&IMPORTANT_DIRTY_TILEFLAG_MASK)!=(tileflags3&IMPORTANT_DIRTY_TILEFLAG_MASK))
	{
		hng64_mark_all_tiles_dirty(state, 3);
		state->m_old_tileflags3 = tileflags3;
	}

	// mark all frames as dirty if for some reason we don't trust the above code
	//hng64_mark_all_tiles_dirty(state, 0);
	//hng64_mark_all_tiles_dirty(state, 1);
	//hng64_mark_all_tiles_dirty(state, 2);
	//hng64_mark_all_tiles_dirty(state, 3);

	hng64_drawtilemap(screen.machine(),bitmap,cliprect, 3);
	hng64_drawtilemap(screen.machine(),bitmap,cliprect, 2);
	hng64_drawtilemap(screen.machine(),bitmap,cliprect, 1);
	hng64_drawtilemap(screen.machine(),bitmap,cliprect, 0);

	// 3d really shouldn't be last, but you don't see some cool stuff right now if it's put before sprites.
	{
		int x, y;

		// Blit the color buffer into the primary bitmap
		for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			UINT32 *src = &state->m_colorBuffer3d[y * cliprect.max_x];
			UINT32 *dst = &bitmap.pix32(y, cliprect.min_x);

			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				if(*src & 0xff000000)
					*dst = *src;

				dst++;
				src++;
			}
		}
		//printf("NEW FRAME!\n");   /* Debug - ajg */
	}

	draw_sprites(screen.machine(), bitmap,cliprect);

	if(0)
		transition_control(screen.machine(), bitmap, cliprect);

	if (0)
		popmessage("%08x %08x %08x %08x %08x", state->m_spriteregs[0], state->m_spriteregs[1], state->m_spriteregs[2], state->m_spriteregs[3], state->m_spriteregs[4]);

	if (0)
	popmessage("%08x %08x TR(%04x %04x %04x %04x) SB(%04x %04x %04x %04x) %08x %08x %08x %08x %08x AA(%08x %08x) %08x %08x",
	 hng64_videoregs[0x00],
	 hng64_videoregs[0x01],
	(hng64_videoregs[0x02]>>16)&0xf9ff, // ----  bits we're sure about are masked out
	(hng64_videoregs[0x02]>>0)&0xf9ff,  //  ss64_2 debug mode indicates that 0x0040 is enable!
	(hng64_videoregs[0x03]>>16)&0xf9ff, //   buriki agrees (debug data on text layer) xrally agress (pink layer)
	(hng64_videoregs[0x03]>>0)&0xf9ff,  //   fatal fury doesn't (all backgrounds have it set) joy
	(hng64_videoregs[0x04]>>16)&0xffff,
	(hng64_videoregs[0x04]>>0)&0xffff,
	(hng64_videoregs[0x05]>>16)&0xffff,
	(hng64_videoregs[0x05]>>0)&0xffff,
	 hng64_videoregs[0x06],
	 hng64_videoregs[0x07],
	 hng64_videoregs[0x08],
	 hng64_videoregs[0x09],
	 hng64_videoregs[0x0a],
	 hng64_videoregs[0x0b],
	 hng64_videoregs[0x0c],
	 hng64_videoregs[0x0d],
	 hng64_videoregs[0x0e]);

	if (0)
	popmessage("3D: %08x %08x %08x %08x : %08x %08x %08x %08x : %08x %08x %08x %08x",
		state->m_3dregs[0x00/4], state->m_3dregs[0x04/4], state->m_3dregs[0x08/4], state->m_3dregs[0x0c/4],
		state->m_3dregs[0x10/4], state->m_3dregs[0x14/4], state->m_3dregs[0x18/4], state->m_3dregs[0x1c/4],
		state->m_3dregs[0x20/4], state->m_3dregs[0x24/4], state->m_3dregs[0x28/4], state->m_3dregs[0x2c/4]);

	if (0)
		popmessage("TC: %08x %08x %08x %08x : %08x %08x %08x %08x : %08x %08x %08x %08x : %08x %08x %08x %08x : %08x %08x %08x %08x : %08x %08x %08x %08x",
		hng64_tcram[0x00/4],
		hng64_tcram[0x04/4],
		hng64_tcram[0x08/4], // tilemaps 0/1 ?
		hng64_tcram[0x0c/4], // ss64_2 debug 04000000 = 'half' on tm1   00000004 = 'half' on tm3  (used in transitions?)
		hng64_tcram[0x10/4],
		hng64_tcram[0x14/4],
		hng64_tcram[0x18/4],
		hng64_tcram[0x1c/4],
		hng64_tcram[0x20/4],
		hng64_tcram[0x24/4],
		hng64_tcram[0x28/4],
		hng64_tcram[0x2c/4],
		hng64_tcram[0x30/4],
		hng64_tcram[0x34/4],
		hng64_tcram[0x38/4],
		hng64_tcram[0x3c/4],
		hng64_tcram[0x40/4],
		hng64_tcram[0x44/4],
		hng64_tcram[0x48/4],
		hng64_tcram[0x4c/4],
		hng64_tcram[0x50/4],
		hng64_tcram[0x54/4],
		hng64_tcram[0x58/4],
		hng64_tcram[0x5c/4]);

	if ( screen.machine().input().code_pressed_once(KEYCODE_T) )
	{
		state->m_additive_tilemap_debug ^= 1;
		popmessage("blend changed %02x", state->m_additive_tilemap_debug);
	}
	if ( screen.machine().input().code_pressed_once(KEYCODE_Y) )
	{
		state->m_additive_tilemap_debug ^= 2;
		popmessage("blend changed %02x", state->m_additive_tilemap_debug);
	}
	if ( screen.machine().input().code_pressed_once(KEYCODE_U) )
	{
		state->m_additive_tilemap_debug ^= 4;
		popmessage("blend changed %02x", state->m_additive_tilemap_debug);
	}
	if ( screen.machine().input().code_pressed_once(KEYCODE_I) )
	{
		state->m_additive_tilemap_debug ^= 8;
		popmessage("blend changed %02x", state->m_additive_tilemap_debug);
	}

	return 0;
}

SCREEN_VBLANK( hng64 )
{
	// rising edge
	if (vblank_on)
		clear3d(screen.machine());
}

VIDEO_START( hng64 )
{
	hng64_state *state = machine.driver_data<hng64_state>();
	const rectangle &visarea = machine.primary_screen->visible_area();

	state->m_old_animmask = -1;
	state->m_old_animbits = -1;
	state->m_old_tileflags0 = -1;
	state->m_old_tileflags1 = -1;
	state->m_old_tileflags2 = -1;
	state->m_old_tileflags3 = -1;

	state->m_tilemap0_8x8       = tilemap_create(machine, get_hng64_tile0_8x8_info,   tilemap_scan_rows,  8,   8, 128, 128); /* 128x128x4 = 0x10000 */
	state->m_tilemap0_16x16     = tilemap_create(machine, get_hng64_tile0_16x16_info, tilemap_scan_rows,  16, 16, 128, 128); /* 128x128x4 = 0x10000 */
	state->m_tilemap0_16x16_alt = tilemap_create(machine, get_hng64_tile0_16x16_info, tilemap_scan_rows,  16, 16, 256,  64); /* 128x128x4 = 0x10000 */

	state->m_tilemap1_8x8       = tilemap_create(machine, get_hng64_tile1_8x8_info,   tilemap_scan_rows,  8,   8, 128, 128); /* 128x128x4 = 0x10000 */
	state->m_tilemap1_16x16     = tilemap_create(machine, get_hng64_tile1_16x16_info, tilemap_scan_rows,  16, 16, 128, 128); /* 128x128x4 = 0x10000 */
	state->m_tilemap1_16x16_alt = tilemap_create(machine, get_hng64_tile1_16x16_info, tilemap_scan_rows,  16, 16, 256,  64); /* 128x128x4 = 0x10000 */

	state->m_tilemap2_8x8       = tilemap_create(machine, get_hng64_tile2_8x8_info,   tilemap_scan_rows,  8,   8, 128, 128); /* 128x128x4 = 0x10000 */
	state->m_tilemap2_16x16     = tilemap_create(machine, get_hng64_tile2_16x16_info, tilemap_scan_rows,  16, 16, 128, 128); /* 128x128x4 = 0x10000 */
	state->m_tilemap2_16x16_alt = tilemap_create(machine, get_hng64_tile2_16x16_info, tilemap_scan_rows,  16, 16, 256,  64); /* 128x128x4 = 0x10000 */

	state->m_tilemap3_8x8       = tilemap_create(machine, get_hng64_tile3_8x8_info,   tilemap_scan_rows,  8,   8, 128, 128); /* 128x128x4 = 0x10000 */
	state->m_tilemap3_16x16     = tilemap_create(machine, get_hng64_tile3_16x16_info, tilemap_scan_rows,  16, 16, 128, 128); /* 128x128x4 = 0x10000 */
	state->m_tilemap3_16x16_alt = tilemap_create(machine, get_hng64_tile3_16x16_info, tilemap_scan_rows,  16, 16, 256,  64); /* 128x128x4 = 0x10000 */

	state->m_tilemap0_8x8->set_transparent_pen(0);
	state->m_tilemap0_16x16->set_transparent_pen(0);
	state->m_tilemap0_16x16_alt->set_transparent_pen(0);

	state->m_tilemap1_8x8->set_transparent_pen(0);
	state->m_tilemap1_16x16->set_transparent_pen(0);
	state->m_tilemap1_16x16_alt->set_transparent_pen(0);

	state->m_tilemap2_8x8->set_transparent_pen(0);
	state->m_tilemap2_16x16->set_transparent_pen(0);
	state->m_tilemap2_16x16_alt->set_transparent_pen(0);

	state->m_tilemap3_8x8->set_transparent_pen(0);
	state->m_tilemap3_16x16->set_transparent_pen(0);
	state->m_tilemap3_16x16_alt->set_transparent_pen(0);

	// Debug switch, turn on / off additive blending on a per-tilemap basis
	state->m_additive_tilemap_debug = 0;

	// 3d Buffer Allocation
	state->m_depthBuffer3d = auto_alloc_array(machine, float,  (visarea.max_x + 1)*(visarea.max_y + 1));
	state->m_colorBuffer3d = auto_alloc_array(machine, UINT32, (visarea.max_x + 1)*(visarea.max_y + 1));
}


///////////////
// 3d Engine //
///////////////

struct polyVert
{
	float worldCoords[4];	// World space coordinates (X Y Z 1.0)

	float texCoords[4];		// Texture coordinates (U V 0 1.0) -> OpenGL style...

	float normal[4];		// Normal (X Y Z 1.0)
	float clipCoords[4];	// Homogeneous screen space coordinates (X Y Z W)

	float light[3];			// The intensity of the illumination at this point
};

struct polygon
{
	int n;						// Number of sides
	struct polyVert vert[10];	// Vertices (maximum number per polygon is 10 -> 3+6)

	float faceNormal[4];		// Normal of the face overall - for calculating visibility and flat-shading...
	int visible;				// Polygon visibility in scene

	UINT8 texIndex;				// Which texture to draw from (0x00-0x0f)
	UINT8 texType;				// How to index into the texture
	UINT8 texPageSmall;         // Does this polygon use 'small' texture pages?
	UINT8 texPageHorizOffset;   // If it does use small texture pages, how far is this page horizontally offset?
	UINT8 texPageVertOffset;    // If it does use small texture pages, how far is this page vertically offset?

	UINT32 palOffset;			// The base offset where this object's palette starts.
	UINT32 palPageSize;			// The size of the palette page that is being pointed to.

	UINT32 debugColor;			// Will go away someday.  Used to explicitly color polygons for debugging.
};

static void setIdentity(float *matrix);
static void matmul4(float *product, const float *a, const float *b);
static void vecmatmul4(float *product, const float *a, const float *b);
static float vecDotProduct(const float *a, const float *b);
static void normalize(float* x);

static void performFrustumClip(struct polygon *p);
static void drawShaded(running_machine &machine, struct polygon *p);
//static void plot(running_machine &machine, INT32 x, INT32 y, UINT32 color);
//static void drawline2d(running_machine &machine, INT32 x0, INT32 y0, INT32 x1, INT32 y1, UINT32 color);
//static void DrawWireframe(running_machine &machine, struct polygon *p);

static float uToF(UINT16 input);


////////////////////
// 3d 'Functions' //
////////////////////

static void printPacket(const UINT16* packet, int hex)
{
	if (hex)
	{
		printf("Packet : %04x %04x  2:%04x %04x  4:%04x %04x  6:%04x %04x  8:%04x %04x  10:%04x %04x  12:%04x %04x  14:%04x %04x\n",
				packet[0],  packet[1],
				packet[2],  packet[3],
				packet[4],  packet[5],
				packet[6],  packet[7],
				packet[8],  packet[9],
				packet[10], packet[11],
				packet[12], packet[13],
				packet[14], packet[15]);
	}
	else
	{
		printf("Packet : %04x %3.4f  2:%3.4f %3.4f  4:%3.4f %3.4f  6:%3.4f %3.4f  8:%3.4f %3.4f  10:%3.4f %3.4f  12:%3.4f %3.4f  14:%3.4f %3.4f\n",
				packet[0],            uToF(packet[1] )*128,
				uToF(packet[2] )*128, uToF(packet[3] )*128,
				uToF(packet[4] )*128, uToF(packet[5] )*128,
				uToF(packet[6] )*128, uToF(packet[7] )*128,
				uToF(packet[8] )*128, uToF(packet[9] )*128,
				uToF(packet[10])*128, uToF(packet[11])*128,
				uToF(packet[12])*128, uToF(packet[13])*128,
				uToF(packet[14])*128, uToF(packet[15])*128);
	}
}

// Operation 0001
// Camera transformation.
static void setCameraTransformation(hng64_state *state, const UINT16* packet)
{
	float *cameraMatrix = state->m_cameraMatrix;

	/*//////////////
    // PACKET FORMAT
    // [0]  - 0001 ... ID
    // [1]  - xxxx ... Extrinsic camera matrix
    // [2]  - xxxx ... Extrinsic camera matrix
    // [3]  - xxxx ... Extrinsic camera matrix
    // [4]  - xxxx ... Extrinsic camera matrix
    // [5]  - xxxx ... Extrinsic camera matrix
    // [6]  - xxxx ... Extrinsic camera matrix
    // [7]  - xxxx ... Extrinsic camera matrix
    // [8]  - xxxx ... Extrinsic camera matrix
    // [9]  - xxxx ... Extrinsic camera matrix
    // [10] - xxxx ... Extrinsic camera matrix
    // [11] - xxxx ... Extrinsic camera matrix
    // [12] - xxxx ... Extrinsic camera matrix
    // [13] - ???? ... ? Flips per-frame during fatfurwa 'HNG64'
    // [14] - ???? ... ? Could be some floating-point values during buriki 'door run'
    // [15] - ???? ... ? Same as 13 & 14
    ////////////*/
	// CAMERA TRANSFORMATION MATRIX
	cameraMatrix[0]  = uToF(packet[1]);
	cameraMatrix[4]  = uToF(packet[2]);
	cameraMatrix[8]  = uToF(packet[3]);
	cameraMatrix[3]  = 0.0f;

	cameraMatrix[1]  = uToF(packet[4]);
	cameraMatrix[5]  = uToF(packet[5]);
	cameraMatrix[9]  = uToF(packet[6]);
	cameraMatrix[7]  = 0.0f;

	cameraMatrix[2]  = uToF(packet[7]);
	cameraMatrix[6]  = uToF(packet[8]);
	cameraMatrix[10] = uToF(packet[9]);
	cameraMatrix[11] = 0.0f;

	cameraMatrix[12] = uToF(packet[10]);
	cameraMatrix[13] = uToF(packet[11]);
	cameraMatrix[14] = uToF(packet[12]);
	cameraMatrix[15] = 1.0f;
}

// Operation 0010
// Lighting information
static void setLighting(hng64_state *state, const UINT16* packet)
{
	float *lightVector = state->m_lightVector;

	/*//////////////
    // PACKET FORMAT
    // [0]  - 0010 ... ID
    // [1]  - ???? ... ? Always zero
    // [2]  - ???? ... ? Always zero
    // [3]  - xxxx ... X light vector direction
    // [4]  - xxxx ... Y light vector direction
    // [5]  - xxxx ... Z light vector direction
    // [6]  - ???? ... ? Seems to be another light vector ?
    // [7]  - ???? ... ? Seems to be another light vector ?
    // [8]  - ???? ... ? Seems to be another light vector ?
    // [9]  - xxxx ... Strength according to sams64_2 [0000,01ff]
    // [10] - ???? ... ? Used in fatfurwa
    // [11] - ???? ... ? Used in fatfurwa
    // [12] - ???? ... ? Used in fatfurwa
    // [13] - ???? ... ? Used in fatfurwa
    // [14] - ???? ... ? Used in fatfurwa
    // [15] - ???? ... ? Used in fatfurwa
    ////////////*/
	if (packet[1] != 0x0000) printf("ZOMG!  packet[1] in setLighting function is non-zero!\n");
	if (packet[2] != 0x0000) printf("ZOMG!  packet[2] in setLighting function is non-zero!\n");

	lightVector[0] = uToF(packet[3]);
	lightVector[1] = uToF(packet[4]);
	lightVector[2] = uToF(packet[5]);
	state->m_lightStrength = uToF(packet[9]);
}

// Operation 0011
// Palette / Model flags?
static void set3dFlags(hng64_state *state, const UINT16* packet)
{
	/*//////////////
    // PACKET FORMAT
    // [0]  - 0011 ... ID
    // [1]  - ???? ...
    // [2]  - ???? ...
    // [3]  - ???? ...
    // [4]  - ???? ...
    // [5]  - ???? ...
    // [6]  - ???? ...
    // [7]  - ???? ...
    // [8]  - xx?? ... Palette offset & ??
    // [9]  - ???? ... ? Very much used - seem to bounce around when characters are on screen
    // [10] - ???? ... ? ''  ''
    // [11] - ???? ... ? ''  ''
    // [12] - ???? ... ? ''  ''
    // [13] - ???? ... ? ''  ''
    // [14] - ???? ... ? ''  ''
    // [15] - ???? ... ? ''  ''
    ////////////*/
	state->m_paletteState3d = (packet[8] & 0xff00) >> 8;
}

// Operation 0012
// Projection Matrix.
static void setCameraProjectionMatrix(hng64_state *state, const UINT16* packet)
{
	float *projectionMatrix = state->m_projectionMatrix;

	/*//////////////
    // PACKET FORMAT
    // [0]  - 0012 ... ID
    // [1]  - ???? ... ? Contains a value in buriki's 'how to play' - probably a projection window/offset.
    // [2]  - ???? ... ? Contains a value in buriki's 'how to play' - probably a projection window/offset.
    // [3]  - ???? ... ? Contains a value
    // [4]  - xxxx ... Camera projection near scale
    // [5]  - xxxx ... Camera projection near height(?)
    // [6]  - xxxx ... Camera projection near width(?)
    // [7]  - xxxx ... Camera projection far scale
    // [8]  - xxxx ... Camera projection far height(?)
    // [9]  - xxxx ... Camera projection far width(?)
    // [10] - xxxx ... Camera projection right
    // [11] - xxxx ... Camera projection left
    // [12] - xxxx ... Camera projection top
    // [13] - xxxx ... Camera projection bottom
    // [14] - ???? ... ? Gets data during buriki door-run
    // [15] - ???? ... ? Gets data during buriki door-run
    ////////////*/

	// Heisted from GLFrustum - 6 parameters...
	float left, right, top, bottom, near_, far_;

	left    = uToF(packet[11]);
	right   = uToF(packet[10]);
	top     = uToF(packet[12]);
	bottom  = uToF(packet[13]);
	near_   = uToF(packet[6]) + (uToF(packet[6]) * uToF(packet[4]));
	far_    = uToF(packet[9]) + (uToF(packet[9]) * uToF(packet[7]));
	// (note are likely not 100% correct - I'm not using one of the parameters)

	projectionMatrix[0]  = (2.0f*near_)/(right-left);
	projectionMatrix[1]  = 0.0f;
	projectionMatrix[2]  = 0.0f;
	projectionMatrix[3]  = 0.0f;

	projectionMatrix[4]  = 0.0f;
	projectionMatrix[5]  = (2.0f*near_)/(top-bottom);
	projectionMatrix[6]  = 0.0f;
	projectionMatrix[7]  = 0.0f;

	projectionMatrix[8]  = (right+left)/(right-left);
	projectionMatrix[9]  = (top+bottom)/(top-bottom);
	projectionMatrix[10] = -((far_+near_)/(far_-near_));
	projectionMatrix[11] = -1.0f;

	projectionMatrix[12] = 0.0f;
	projectionMatrix[13] = 0.0f;
	projectionMatrix[14] = -((2.0f*far_*near_)/(far_-near_));
	projectionMatrix[15] = 0.0f;
}

// Operation 0100
// Polygon rasterization.
static void recoverPolygonBlock(running_machine& machine, const UINT16* packet, struct polygon* polys, int* numPolys)
{
	/*//////////////
    // PACKET FORMAT
    // [0]  - 0100 ... ID
    // [1]  - ?--- ... Flags [?000 = ???
    //                        0?00 = ???
    //                        00?0 = ???
    //                        000? = ???]
    // [1]  - -?-- ... Flags [?000 = ???
    //                        0?00 = ???
    //                        00?0 = ???
    //                        000x = Dynamic palette bit]
    // [1]  - --?- ... Flags [?000 = ???
    //                        0?00 = ???
    //                        00?0 = ???
    //                        000? = ???]
    // [1]  - ---? ... Flags [x000 = Apply lighting bit
    //                        0?00 = ???
    //                        00?0 = ???
    //                        000? = ???]
    // [2]  - xxxx ... offset into ROM
    // [3]  - xxxx ... offset into ROM
    // [4]  - xxxx ... Transformation matrix
    // [5]  - xxxx ... Transformation matrix
    // [6]  - xxxx ... Transformation matrix
    // [7]  - xxxx ... Transformation matrix
    // [8]  - xxxx ... Transformation matrix
    // [9]  - xxxx ... Transformation matrix
    // [10] - xxxx ... Transformation matrix
    // [11] - xxxx ... Transformation matrix
    // [12] - xxxx ... Transformation matrix
    // [13] - xxxx ... Transformation matrix
    // [14] - xxxx ... Transformation matrix
    // [15] - xxxx ... Transformation matrix
    ////////////*/

	hng64_state *state = machine.driver_data<hng64_state>();
	UINT32 size[4];
	UINT32 address[4];
	UINT32 megaOffset;
	float eyeCoords[4];		// ObjectCoords transformed by the modelViewMatrix
//  float clipCoords[4];    // EyeCoords transformed by the projectionMatrix
	float ndCoords[4];		// Normalized device coordinates/clipCoordinates (x/w, y/w, z/w)
	float windowCoords[4];	// Mapped ndCoordinates to screen space
	float cullRay[4];

	float objectMatrix[16];
	setIdentity(objectMatrix);

	struct polygon lastPoly = { 0 };
	const rectangle &visarea = machine.primary_screen->visible_area();

	/////////////////
	// HEADER INFO //
	/////////////////
	// THE OBJECT TRANSFORMATION MATRIX
	objectMatrix[8] = uToF(packet[7]);
	objectMatrix[4] = uToF(packet[8]);
	objectMatrix[0] = uToF(packet[9]);
	objectMatrix[3] = 0.0f;

	objectMatrix[9] = uToF(packet[10]);
	objectMatrix[5] = uToF(packet[11]);
	objectMatrix[1] = uToF(packet[12]);
	objectMatrix[7] = 0.0f;

	objectMatrix[10] = uToF(packet[13]);
	objectMatrix[6 ] = uToF(packet[14]);
	objectMatrix[2 ] = uToF(packet[15]);
	objectMatrix[11] = 0.0f;

	objectMatrix[12] = uToF(packet[4]);
	objectMatrix[13] = uToF(packet[5]);
	objectMatrix[14] = uToF(packet[6]);
	objectMatrix[15] = 1.0f;


	//////////////////////////////////////////////////////////
	// EXTRACT DATA FROM THE ADDRESS POINTED TO IN THE FILE //
	//////////////////////////////////////////////////////////
	/*//////////////////////////////////////////////
    // DIRECTLY-POINTED-TO FORMAT (7 words x 3 ROMs)
    // [0]  - lower word of sub-address 1
    // [1]  - lower word of sub-address 2
    // [2]  - upper word of all sub-addresses
    // [3]  - lower word of sub-address 3
    // [4]  - lower word of sub-address 4
    // [5]  - ???? always 0 ????
    // [6]  - number of chunks in sub-address 1 block
    // [7]  - number of chunks in sub-address 2 block
    // [8]  - ???? always 0 ????
    // [9]  - number of chunks in sub-address 3 block
    // [10] - number of chunks in sub-address 4 block
    // [11] - ? definitely used.
    // [12] - ? definitely used.
    // [13] - ? definitely used.
    // [14] - ? definitely used.
    // [15] - ???? always 0 ????
    // [16] - ???? always 0 ????
    // [17] - ???? always 0 ????
    // [18] - ???? always 0 ????
    // [19] - ???? always 0 ????
    // [20] - ???? always 0 ????
    //////////////////////////////////////////////*/

	// 3d ROM Offset
	UINT16* threeDRoms = (UINT16*)(machine.region("verts")->base());
	UINT32  threeDOffset = (((UINT32)packet[2]) << 16) | ((UINT32)packet[3]);
	UINT16* threeDPointer = &threeDRoms[threeDOffset * 3];

	if (threeDOffset >= machine.region("verts")->bytes())
	{
		printf("Strange geometry packet: (ignoring)\n");
		printPacket(packet, 1);
		return;
	}

#if 0
	// Debug - ajg
	printf("%08x : ", threeDOffset*3*2);
	for (int k = 0; k < 7*3; k++)
	{
		printf("%04x ", threeDPointer[k]);
		if ((k % 3) == 2) printf(" ");
	}
	printf("\n");
#endif

	// There are 4 hunks per address.
	address[0] = threeDPointer[0];
	address[1] = threeDPointer[1];
	megaOffset = threeDPointer[2];

	address[2] = threeDPointer[3];
	address[3] = threeDPointer[4];
	if (threeDPointer[5] != 0x0000) printf("ZOMG!  3dPointer[5] is non-zero!\n");

	size[0]    = threeDPointer[6];
	size[1]    = threeDPointer[7];
	if (threeDPointer[8] != 0x0000) printf("ZOMG!  3dPointer[8] is non-zero!\n");

	size[2]    = threeDPointer[9];
	size[3]    = threeDPointer[10];
	/*           ????         [11]; Used. */

	/*           ????         [12]; Used. */
	/*           ????         [13]; Used. */
	/*           ????         [14]; Used. */

	if (threeDPointer[15] != 0x0000) printf("ZOMG!  3dPointer[15] is non-zero!\n");
	if (threeDPointer[16] != 0x0000) printf("ZOMG!  3dPointer[16] is non-zero!\n");
	if (threeDPointer[17] != 0x0000) printf("ZOMG!  3dPointer[17] is non-zero!\n");

	if (threeDPointer[18] != 0x0000) printf("ZOMG!  3dPointer[18] is non-zero!\n");
	if (threeDPointer[19] != 0x0000) printf("ZOMG!  3dPointer[19] is non-zero!\n");
	if (threeDPointer[20] != 0x0000) printf("ZOMG!  3dPointer[20] is non-zero!\n");

	/* Concatenate the megaOffset with the addresses */
	address[0] |= (megaOffset << 16);
	address[1] |= (megaOffset << 16);
	address[2] |= (megaOffset << 16);
	address[3] |= (megaOffset << 16);

	// Debug - ajg
	//UINT32 tdColor = 0xff000000;
	//if (threeDPointer[14] & 0x0002) tdColor |= 0x00ff0000;
	//if (threeDPointer[14] & 0x0001) tdColor |= 0x0000ff00;
	//if (threeDPointer[14] & 0x0000) tdColor |= 0x000000ff;

	/* For all 4 polygon chunks */
	for (int k = 0; k < 4; k++)
	{
		UINT16* chunkOffset = &threeDRoms[address[k] * 3];
		for (int l = 0; l < size[k]; l++)
		{
			////////////////////////////////////////////
			// GATHER A SINGLE TRIANGLE'S INFORMATION //
			////////////////////////////////////////////
			// SINGLE POLY CHUNK FORMAT
			// [0] ??-- - ???
			// [0] --xx - Chunk type
			//
			// [1] ?--- - Flags [?000 = ???
			//                   0?00 = ???
			//                   00?0 = ???
			//                   000x = low-res texture flag]
			// [1] -x-- - Explicit 0x80 palette index.
			// [1] --x- - Explicit 0x08 palette index.
			// [1] ---x - Texture page (1024x1024 bytes)
			//
			// [2] x--- - Texture Flags [x000 = Uses 4x4 sub-texture pages?
			//                           0?00 = ??? - differen sub-page size?  SNK logo in RoadEdge.  Always on in bbust2.
			//                           00xx = Horizontal sub-texture page index]
			// [2] -?-- - ??? - barely visible (thus far) in roadedge
			// [2] --x- - Texture Flags [?000 = ???
			//                           0xx0 = Vertical sub-texture page index.
			//                           000? = ???]
			// [2] ---? - ???
			//////////////////////////
			UINT8 chunkType = chunkOffset[0] & 0x00ff;

			// Debug - ajg
			if (chunkOffset[0] & 0xff00)
			{
				printf("Weird!  The top byte of the chunkType has a value %04x!\n", chunkOffset[0]);
				continue;
			}

			// Debug - Colors polygons with certain flags bright blue! ajg
			polys[*numPolys].debugColor = 0;
			//polys[*numPolys].debugColor = tdColor;

			// Debug - ajg
			//printf("%d (%08x) : %04x %04x %04x\n", k, address[k]*3*2, chunkOffset[0], chunkOffset[1], chunkOffset[2]);
			//break;

			// TEXTURE
			/* There may be more than just high & low res texture types, so I'm keeping texType as a UINT8. */
			if (chunkOffset[1] & 0x1000) polys[*numPolys].texType = 0x1;
			else						 polys[*numPolys].texType = 0x0;

			polys[*numPolys].texPageSmall       = (chunkOffset[2] & 0x8000) >> 15;	// Just a guess.
			polys[*numPolys].texPageHorizOffset = (chunkOffset[2] & 0x3000) >> 12;
			polys[*numPolys].texPageVertOffset  = (chunkOffset[2] & 0x0060) >> 5;

			polys[*numPolys].texIndex = chunkOffset[1] & 0x000f;


			// PALETTE
			polys[*numPolys].palOffset = 0;
			polys[*numPolys].palPageSize = 0x100;

			/* FIXME: This isn't correct.
                      Buriki & Xrally need this line.  Roads Edge needs it removed.
                      So instead we're looking for a bit that is on for XRally & Buriki, but noone else. */
			if (state->m_3dregs[0x00/4] & 0x2000)
			{
				if (strcmp(machine.basename(), "roadedge"))
					polys[*numPolys].palOffset += 0x800;
			}

			//UINT16 explicitPaletteValue0 = ((chunkOffset[?] & 0x????) >> ?) * 0x800;
			UINT16 explicitPaletteValue1 = ((chunkOffset[1] & 0x0f00) >> 8) * 0x080;
			UINT16 explicitPaletteValue2 = ((chunkOffset[1] & 0x00f0) >> 4) * 0x008;

			// The presence of 0x00f0 *probably* sets 0x10-sized palette addressing.
			if (explicitPaletteValue2) polys[*numPolys].palPageSize = 0x10;

			// Apply the dynamic palette offset if its flag is set, otherwise stick with the fixed one
			if ((packet[1] & 0x0100))
			{
				explicitPaletteValue1 = state->m_paletteState3d * 0x80;
				explicitPaletteValue2 = 0;      // This is probably hiding somewhere in operation 0011
			}

			polys[*numPolys].palOffset += (explicitPaletteValue1 + explicitPaletteValue2);



			UINT8 chunkLength = 0;
			switch(chunkType)
			{
			/*/////////////////////////
            // CHUNK TYPE BITS - These are very likely incorrect.
            // x--- ---- - 1 = Has only 1 vertex (part of a triangle fan/strip)
            // -x-- ---- -
            // --x- ---- -
            // ---x ---- -
            // ---- x--- -
            // ---- -x-- - 1 = Has per-vert UVs
            // ---- --x- -
            // ---- ---x - 1 = Has per-vert normals
            /////////////////////////*/

			// 33 word chunk, 3 vertices, per-vertex UVs & normals, per-face normal
			case 0x05:	// 0000 0101
			case 0x0f:	// 0000 1111
				for (int m = 0; m < 3; m++)
				{
					polys[*numPolys].vert[m].worldCoords[0] = uToF(chunkOffset[3 + (9*m)]);
					polys[*numPolys].vert[m].worldCoords[1] = uToF(chunkOffset[4 + (9*m)]);
					polys[*numPolys].vert[m].worldCoords[2] = uToF(chunkOffset[5 + (9*m)]);
					polys[*numPolys].vert[m].worldCoords[3] = 1.0f;
					polys[*numPolys].n = 3;

					// chunkOffset[6 + (9*m)] is almost always 0080, but it's 0070 for the translucent globe in fatfurwa player select
					polys[*numPolys].vert[m].texCoords[0] = uToF(chunkOffset[7 + (9*m)]);
					polys[*numPolys].vert[m].texCoords[1] = uToF(chunkOffset[8 + (9*m)]);
					polys[*numPolys].vert[m].texCoords[2] = 0.0f;
					polys[*numPolys].vert[m].texCoords[3] = 1.0f;

					polys[*numPolys].vert[m].normal[0] = uToF(chunkOffset[9  + (9*m)]);
					polys[*numPolys].vert[m].normal[1] = uToF(chunkOffset[10 + (9*m)] );
					polys[*numPolys].vert[m].normal[2] = uToF(chunkOffset[11 + (9*m)] );
					polys[*numPolys].vert[m].normal[3] = 0.0f;
				}

				// Redundantly called, but it works...
				polys[*numPolys].faceNormal[0] = uToF(chunkOffset[30]);
				polys[*numPolys].faceNormal[1] = uToF(chunkOffset[31]);
				polys[*numPolys].faceNormal[2] = uToF(chunkOffset[32]);
				polys[*numPolys].faceNormal[3] = 0.0f;

				chunkLength = 33;
				break;


			// 24 word chunk, 3 vertices, per-vertex UVs
			case 0x04:	// 0000 0100
			case 0x0e:	// 0000 1110
			case 0x24:	// 0010 0100
			case 0x2e:	// 0010 1110
				for (int m = 0; m < 3; m++)
				{
					polys[*numPolys].vert[m].worldCoords[0] = uToF(chunkOffset[3 + (6*m)]);
					polys[*numPolys].vert[m].worldCoords[1] = uToF(chunkOffset[4 + (6*m)]);
					polys[*numPolys].vert[m].worldCoords[2] = uToF(chunkOffset[5 + (6*m)]);
					polys[*numPolys].vert[m].worldCoords[3] = 1.0f;
					polys[*numPolys].n = 3;

					// chunkOffset[6 + (6*m)] is almost always 0080, but it's 0070 for the translucent globe in fatfurwa player select
					polys[*numPolys].vert[m].texCoords[0] = uToF(chunkOffset[7 + (6*m)]);
					polys[*numPolys].vert[m].texCoords[1] = uToF(chunkOffset[8 + (6*m)]);
					polys[*numPolys].vert[m].texCoords[2] = 0.0f;
					polys[*numPolys].vert[m].texCoords[3] = 1.0f;

					polys[*numPolys].vert[m].normal[0] = uToF(chunkOffset[21]);
					polys[*numPolys].vert[m].normal[1] = uToF(chunkOffset[22]);
					polys[*numPolys].vert[m].normal[2] = uToF(chunkOffset[23]);
					polys[*numPolys].vert[m].normal[3] = 0.0f;
				}

				// Redundantly called, but it works...
				polys[*numPolys].faceNormal[0] = polys[*numPolys].vert[2].normal[0];
				polys[*numPolys].faceNormal[1] = polys[*numPolys].vert[2].normal[1];
				polys[*numPolys].faceNormal[2] = polys[*numPolys].vert[2].normal[2];
				polys[*numPolys].faceNormal[3] = 0.0f;

				chunkLength = 24;
				break;


			// 15 word chunk, 1 vertex, per-vertex UVs & normals, face normal
			case 0x87:	// 1000 0111
			case 0x97:	// 1001 0111
			case 0xd7:	// 1101 0111
			case 0xc7:	// 1100 0111
				// Copy over the proper vertices from the previous triangle...
				memcpy(&polys[*numPolys].vert[1], &lastPoly.vert[0], sizeof(struct polyVert));
				memcpy(&polys[*numPolys].vert[2], &lastPoly.vert[2], sizeof(struct polyVert));

				// Fill in the appropriate data...
				polys[*numPolys].vert[0].worldCoords[0] = uToF(chunkOffset[3]);
				polys[*numPolys].vert[0].worldCoords[1] = uToF(chunkOffset[4]);
				polys[*numPolys].vert[0].worldCoords[2] = uToF(chunkOffset[5]);
				polys[*numPolys].vert[0].worldCoords[3] = 1.0f;
				polys[*numPolys].n = 3;

				// chunkOffset[6] is almost always 0080, but it's 0070 for the translucent globe in fatfurwa player select
				polys[*numPolys].vert[0].texCoords[0] = uToF(chunkOffset[7]);
				polys[*numPolys].vert[0].texCoords[1] = uToF(chunkOffset[8]);
				polys[*numPolys].vert[0].texCoords[2] = 0.0f;
				polys[*numPolys].vert[0].texCoords[3] = 1.0f;

				polys[*numPolys].vert[0].normal[0] = uToF(chunkOffset[9]);
				polys[*numPolys].vert[0].normal[1] = uToF(chunkOffset[10]);
				polys[*numPolys].vert[0].normal[2] = uToF(chunkOffset[11]);
				polys[*numPolys].vert[0].normal[3] = 0.0f;

				polys[*numPolys].faceNormal[0] = uToF(chunkOffset[12]);
				polys[*numPolys].faceNormal[1] = uToF(chunkOffset[13]);
				polys[*numPolys].faceNormal[2] = uToF(chunkOffset[14]);
				polys[*numPolys].faceNormal[3] = 0.0f;

				chunkLength = 15;
				break;


			// 12 word chunk, 1 vertex, per-vertex UVs
			case 0x86:	// 1000 0110
			case 0x96:	// 1001 0110
			case 0xb6:	// 1011 0110
			case 0xc6:	// 1100 0110
			case 0xd6:	// 1101 0110
				// Copy over the proper vertices from the previous triangle...
				memcpy(&polys[*numPolys].vert[1], &lastPoly.vert[0], sizeof(struct polyVert));
				memcpy(&polys[*numPolys].vert[2], &lastPoly.vert[2], sizeof(struct polyVert));

				polys[*numPolys].vert[0].worldCoords[0] = uToF(chunkOffset[3]);
				polys[*numPolys].vert[0].worldCoords[1] = uToF(chunkOffset[4]);
				polys[*numPolys].vert[0].worldCoords[2] = uToF(chunkOffset[5]);
				polys[*numPolys].vert[0].worldCoords[3] = 1.0f;
				polys[*numPolys].n = 3;

				// chunkOffset[6] is almost always 0080, but it's 0070 for the translucent globe in fatfurwa player select
				polys[*numPolys].vert[0].texCoords[0] = uToF(chunkOffset[7]);
				polys[*numPolys].vert[0].texCoords[1] = uToF(chunkOffset[8]);
				polys[*numPolys].vert[0].texCoords[2] = 0.0f;
				polys[*numPolys].vert[0].texCoords[3] = 1.0f;

				// This normal could be right, but I'm not entirely sure - there is no normal in the 18 bytes!
				polys[*numPolys].vert[0].normal[0] = lastPoly.faceNormal[0];
				polys[*numPolys].vert[0].normal[1] = lastPoly.faceNormal[1];
				polys[*numPolys].vert[0].normal[2] = lastPoly.faceNormal[2];
				polys[*numPolys].vert[0].normal[3] = lastPoly.faceNormal[3];

				polys[*numPolys].faceNormal[0] = lastPoly.faceNormal[0];
				polys[*numPolys].faceNormal[1] = lastPoly.faceNormal[1];
				polys[*numPolys].faceNormal[2] = lastPoly.faceNormal[2];
				polys[*numPolys].faceNormal[3] = lastPoly.faceNormal[3];

				// TODO: I'm not reading 3 necessary words here (maybe face normal) !!!

#if 0
				// DEBUG
				printf("0x?6 : %08x (%d/%d)\n", address[k]*3*2, l, size[k]-1);
				for (int m = 0; m < 13; m++)
					printf("%04x ", chunkOffset[m]);
				printf("\n");

				for (int m = 0; m < 13; m++)
					printf("%3.4f ", uToF(chunkOffset[m]));
				printf("\n\n");
#endif

				chunkLength = 12;
				break;

			default:
				printf("UNKNOWN geometry CHUNK TYPE : %02x\n", chunkType);
				chunkLength = 0;
				break;
			}

			polys[*numPolys].visible = 1;

			// Backup the last polygon (for triangle fans [strips?])
			memcpy(&lastPoly, &polys[*numPolys], sizeof(struct polygon));


			////////////////////////////////////
			// Project and clip               //
			////////////////////////////////////
			// Perform the world transformations...
			// !! Can eliminate this step with a matrix stack (maybe necessary?) !!
			setIdentity(state->m_modelViewMatrix);
			if (state->m_mcu_type != SAMSHO_MCU)
			{
				// The sams64 games transform the geometry in front of a stationary camera.
				// This is fine in sams64_2, since it never calls the 'camera transformation' function
				// (thus using the identity matrix for this transform), but sams64 calls the
				// camera transformation function with rotation values.
				// It remains to be seen what those might do...
				matmul4(state->m_modelViewMatrix, state->m_modelViewMatrix, state->m_cameraMatrix);
			}
			matmul4(state->m_modelViewMatrix, state->m_modelViewMatrix, objectMatrix);

			// LIGHTING
			if (packet[1] & 0x0008 && state->m_lightStrength > 0.0f)
			{
				for (int v = 0; v < 3; v++)
				{
					float transformedNormal[4];
					vecmatmul4(transformedNormal, objectMatrix, polys[*numPolys].vert[v].normal);
					normalize(transformedNormal);
					normalize(state->m_lightVector);

					float intensity = vecDotProduct(transformedNormal, state->m_lightVector) * -1.0f;
					intensity = (intensity <= 0.0f) ? (0.0f) : (intensity);
					intensity *= state->m_lightStrength * 128.0f;    // Turns 0x0100 into 1.0
					intensity *= 128.0;                     // Maps intensity to the range [0.0, 2.0]
					if (intensity >= 255.0f) intensity = 255.0f;

					polys[*numPolys].vert[v].light[0] = intensity;
					polys[*numPolys].vert[v].light[1] = intensity;
					polys[*numPolys].vert[v].light[2] = intensity;
				}
			}
			else
			{
				// Just clear out the light values
				for (int v = 0; v < 3; v++)
				{
					polys[*numPolys].vert[v].light[0] = 0;
					polys[*numPolys].vert[v].light[1] = 0;
					polys[*numPolys].vert[v].light[2] = 0;
				}
			}


			// BACKFACE CULL //
			// EMPIRICAL EVIDENCE SEEMS TO SHOW THE HNG64 HARDWARE DOES NOT BACKFACE CULL //
#if 0
			float cullRay[4];
			float cullNorm[4];

			// Cast a ray out of the camera towards the polygon's point in eyespace.
			vecmatmul4(cullRay, modelViewMatrix, polys[*numPolys].vert[0].worldCoords);
			normalize(cullRay);
			// Dot product that with the normal to see if you're negative...
			vecmatmul4(cullNorm, modelViewMatrix, polys[*numPolys].faceNormal);

			float result = vecDotProduct(cullRay, cullNorm);

			if (result < 0.0f)
				polys[*numPolys].visible = 1;
			else
				polys[*numPolys].visible = 0;
#endif


			// BEHIND-THE-CAMERA CULL //
			vecmatmul4(cullRay, state->m_modelViewMatrix, polys[*numPolys].vert[0].worldCoords);
			if (cullRay[2] > 0.0f)				// Camera is pointing down -Z
			{
				polys[*numPolys].visible = 0;
			}


			// TRANSFORM THE TRIANGLE INTO HOMOGENEOUS SCREEN SPACE //
			if (polys[*numPolys].visible)
			{
				for (int m = 0; m < polys[*numPolys].n; m++)
				{
					// Transform and project the vertex into pre-divided homogeneous coordinates...
					vecmatmul4(eyeCoords, state->m_modelViewMatrix, polys[*numPolys].vert[m].worldCoords);
					vecmatmul4(polys[*numPolys].vert[m].clipCoords, state->m_projectionMatrix, eyeCoords);
				}

				if (polys[*numPolys].visible)
				{
					// Clip the triangles to the view frustum...
					performFrustumClip(&polys[*numPolys]);

					for (int m = 0; m < polys[*numPolys].n; m++)
					{
						// Convert into normalized device coordinates...
						ndCoords[0] = polys[*numPolys].vert[m].clipCoords[0] / polys[*numPolys].vert[m].clipCoords[3];
						ndCoords[1] = polys[*numPolys].vert[m].clipCoords[1] / polys[*numPolys].vert[m].clipCoords[3];
						ndCoords[2] = polys[*numPolys].vert[m].clipCoords[2] / polys[*numPolys].vert[m].clipCoords[3];
						ndCoords[3] = polys[*numPolys].vert[m].clipCoords[3];

						// Final pixel values are garnered here :
						windowCoords[0] = (ndCoords[0]+1.0f) * ((float)(visarea.max_x) / 2.0f) + 0.0f;
						windowCoords[1] = (ndCoords[1]+1.0f) * ((float)(visarea.max_y) / 2.0f) + 0.0f;
						windowCoords[2] = (ndCoords[2]+1.0f) * 0.5f;

						windowCoords[1] = (float)visarea.max_y - windowCoords[1];		// Flip Y

						// Store the points in a list for later use...
						polys[*numPolys].vert[m].clipCoords[0] = windowCoords[0];
						polys[*numPolys].vert[m].clipCoords[1] = windowCoords[1];
						polys[*numPolys].vert[m].clipCoords[2] = windowCoords[2];
						polys[*numPolys].vert[m].clipCoords[3] = ndCoords[3];
					}
				}
			}

			// Advance to the next polygon chunk...
			chunkOffset += chunkLength;

			(*numPolys)++;
		}
	}
}

void hng64_command3d(running_machine& machine, const UINT16* packet)
{
	hng64_state *state = machine.driver_data<hng64_state>();

	/* A temporary place to put some polygons.  This will optimize away if the compiler's any good. */
	int numPolys = 0;
	struct polygon* polys = auto_alloc_array(machine, struct polygon, 1024*5);

	//printf("packet type : %04x\n", packet[0]);
	switch (packet[0])
	{
	case 0x0000:	// Appears to be a NOP.
		break;

	case 0x0001:	// Camera transformation.
		setCameraTransformation(state, packet);
		break;

	case 0x0010:	// Lighting information.
		//if (packet[9]) printPacket(packet, 1);
		setLighting(state, packet);
		break;

	case 0x0011:	// Palette / Model flags?
		//printPacket(packet, 1); printf("\n");
		set3dFlags(state, packet);
		break;

	case 0x0012:	// Projection Matrix
		//printPacket(packet, 1);
		setCameraProjectionMatrix(state, packet);
		break;

	case 0x0100:
	case 0x0101:	// Geometry with full transformations
		// HACK.  Masks out a piece of geo bbust2's drawShaded() crashes on.
		if (packet[2] == 0x0003 && packet[3] == 0x8f37 && state->m_mcu_type == SHOOT_MCU)
			break;

		recoverPolygonBlock(machine, packet, polys, &numPolys);
		break;

	case 0x0102:	// Geometry with only translation
		// HACK.  Give up on strange calls to 0102.
		if (packet[8] != 0x0102)
		{
			// It appears as though packet[7] might hold the magic #
			// Almost looks like there is a chain mode for these guys.  Same for 0101?
			// printf("WARNING: "); printPacket(packet, 1);
			break;
		}

		// Split the packet and call recoverPolygonBlock on each half.
		UINT16 miniPacket[16];
		memset(miniPacket, 0, sizeof(UINT16)*16);
		for (int i = 0; i < 7; i++) miniPacket[i] = packet[i];
		miniPacket[7] = 0x7fff;
		miniPacket[11] = 0x7fff;
		miniPacket[15] = 0x7fff;
		recoverPolygonBlock(machine, miniPacket, polys, &numPolys);

		memset(miniPacket, 0, sizeof(UINT16)*16);
		for (int i = 0; i < 7; i++) miniPacket[i] = packet[i+8];
		miniPacket[7] = 0x7fff;
		miniPacket[11] = 0x7fff;
		miniPacket[15] = 0x7fff;
		recoverPolygonBlock(machine, miniPacket, polys, &numPolys);
		break;

	case 0x1000:	// Unknown: Some sort of global flags?
		//printPacket(packet, 1); printf("\n");
		break;

	case 0x1001:	// Unknown: Some sort of global flags (a group of 4, actually)?
		//printPacket(packet, 1);
		break;

	default:
		printf("HNG64: Unknown 3d command %04x.\n", packet[0]);
		break;
	}

	/* If there are polygons, rasterize them into the display buffer */
	for (int i = 0; i < numPolys; i++)
	{
		if (polys[i].visible)
		{
			//DrawWireframe(machine, &polys[i]);
			drawShaded(machine, &polys[i]);
		}
	}

	auto_free(machine, polys);
}

static void clear3d(running_machine &machine)
{
	hng64_state *state = machine.driver_data<hng64_state>();
	int i;

	const rectangle &visarea = machine.primary_screen->visible_area();

	// Clear each of the display list buffers after drawing - todo: kill!
	for (i = 0; i < 0x81; i++)
	{
		state->m_dls[0][i] = 0;
		state->m_dls[1][i] = 0;
	}

	// Reset the buffers...
	for (i = 0; i < (visarea.max_x)*(visarea.max_y); i++)
	{
		state->m_depthBuffer3d[i] = 100.0f;
		state->m_colorBuffer3d[i] = MAKE_ARGB(0, 0, 0, 0);
	}

	// Set some matrices to the identity...
	setIdentity(state->m_projectionMatrix);
	setIdentity(state->m_modelViewMatrix);
	setIdentity(state->m_cameraMatrix);
}

/* 3D/framebuffer video registers
 * ------------------------------
 *
 * UINT32 | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *      0 | ???? ???? ???? ???? ccc? ???? ???? ???? | framebuffer color base, 0x311800 in Fatal Fury WA, 0x313800 in Buriki One
 *      1 |                                         |
 *      2 | ???? ???? ???? ???? ???? ???? ???? ???? | camera / framebuffer global x/y? Actively used by Samurai Shodown 64 2
 *      3 | ---- --?x ---- ---- ---- ---- ---- ---- | unknown, unsetted by Buriki One and setted by Fatal Fury WA, buffering mode?
 *   4-11 | ---- ???? ---- ???? ---- ???? ---- ???? | Table filled with 0x0? data
 *
 */

/////////////////////
// 3D UTILITY CODE //
/////////////////////

/* 4x4 matrix multiplication */
static void matmul4(float *product, const float *a, const float *b )
{
	int i;
	for (i = 0; i < 4; i++)
	{
		const float ai0 = a[0  + i];
		const float ai1 = a[4  + i];
		const float ai2 = a[8  + i];
		const float ai3 = a[12 + i];

		product[0  + i] = ai0 * b[0 ] + ai1 * b[1 ] + ai2 * b[2 ] + ai3 * b[3 ];
		product[4  + i] = ai0 * b[4 ] + ai1 * b[5 ] + ai2 * b[6 ] + ai3 * b[7 ];
		product[8  + i] = ai0 * b[8 ] + ai1 * b[9 ] + ai2 * b[10] + ai3 * b[11];
		product[12 + i] = ai0 * b[12] + ai1 * b[13] + ai2 * b[14] + ai3 * b[15];
	}
}

/* vector by 4x4 matrix multiply */
static void vecmatmul4(float *product, const float *a, const float *b)
{
	const float bi0 = b[0];
	const float bi1 = b[1];
	const float bi2 = b[2];
	const float bi3 = b[3];

	product[0] = bi0 * a[0] + bi1 * a[4] + bi2 * a[8 ] + bi3 * a[12];
	product[1] = bi0 * a[1] + bi1 * a[5] + bi2 * a[9 ] + bi3 * a[13];
	product[2] = bi0 * a[2] + bi1 * a[6] + bi2 * a[10] + bi3 * a[14];
	product[3] = bi0 * a[3] + bi1 * a[7] + bi2 * a[11] + bi3 * a[15];
}

static float vecDotProduct(const float *a, const float *b)
{
	return ((a[0]*b[0]) + (a[1]*b[1]) + (a[2]*b[2]));
}

static void setIdentity(float *matrix)
{
	int i;

	for (i = 0; i < 16; i++)
	{
		matrix[i] = 0.0f;
	}

	matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
}

static float uToF(UINT16 input)
{
	float retVal;
	retVal = (float)((INT16)input) / 32768.0f;
	return retVal;

#if 0
	if ((INT16)input < 0)
		retVal = (float)((INT16)input) / 32768.0f;
	else
		retVal = (float)((INT16)input) / 32767.0f;
#endif
}

static void normalize(float* x)
{
	double l2 = (x[0]*x[0]) + (x[1]*x[1]) + (x[2]*x[2]);
	double l = sqrt(l2);

	x[0] = (float)(x[0] / l);
	x[1] = (float)(x[1] / l);
	x[2] = (float)(x[2] / l);
}



///////////////////////////
// POLYGON CLIPPING CODE //
///////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// The remainder of the code in this file is heavily                             //
//   influenced by, and sometimes copied verbatim from Andrew Zaferakis' SoftGL  //
//   rasterizing system.                                                         //
//                                                                               //
//   Andrew granted permission for its use in MAME in October of 2004.           //
///////////////////////////////////////////////////////////////////////////////////

// Refer to the clipping planes as numbers
#define HNG64_LEFT   0
#define HNG64_RIGHT  1
#define HNG64_TOP    2
#define HNG64_BOTTOM 3
#define HNG64_NEAR   4
#define HNG64_FAR    5


static int Inside(struct polyVert *v, int plane)
{
	switch(plane)
	{
	case HNG64_LEFT:
		return (v->clipCoords[0] >= -v->clipCoords[3]) ? 1 : 0;
	case HNG64_RIGHT:
		return (v->clipCoords[0] <=  v->clipCoords[3]) ? 1 : 0;

	case HNG64_TOP:
		return (v->clipCoords[1] <=  v->clipCoords[3]) ? 1 : 0;
	case HNG64_BOTTOM:
		return (v->clipCoords[1] >= -v->clipCoords[3]) ? 1 : 0;

	case HNG64_NEAR:
		return (v->clipCoords[2] <=  v->clipCoords[3]) ? 1 : 0;
	case HNG64_FAR:
		return (v->clipCoords[2] >= -v->clipCoords[3]) ? 1 : 0;
	}

	return 0;
}

static void Intersect(struct polyVert *input0, struct polyVert *input1, struct polyVert *output, int plane)
{
	float t = 0.0f;

	float *Iv0 = input0->clipCoords;
	float *Iv1 = input1->clipCoords;
	float *Ov  = output->clipCoords;

	float *It0 = input0->texCoords;
	float *It1 = input1->texCoords;
	float *Ot  = output->texCoords;

	float *Il0 = input0->light;
	float *Il1 = input1->light;
	float *Ol  = output->light;

	switch(plane)
	{
	case HNG64_LEFT:
		t = (Iv0[0]+Iv0[3]) / (-Iv1[3]+Iv0[3]-Iv1[0]+Iv0[0]);
		break;
	case HNG64_RIGHT:
		t = (Iv0[0]-Iv0[3]) / (Iv1[3]-Iv0[3]-Iv1[0]+Iv0[0]);
		break;
	case HNG64_TOP:
		t = (Iv0[1]-Iv0[3]) / (Iv1[3]-Iv0[3]-Iv1[1]+Iv0[1]);
		break;
	case HNG64_BOTTOM:
		t = (Iv0[1]+Iv0[3]) / (-Iv1[3]+Iv0[3]-Iv1[1]+Iv0[1]);
		break;
	case HNG64_NEAR:
		t = (Iv0[2]-Iv0[3]) / (Iv1[3]-Iv0[3]-Iv1[2]+Iv0[2]);
		break;
	case HNG64_FAR:
		t = (Iv0[2]+Iv0[3]) / (-Iv1[3]+Iv0[3]-Iv1[2]+Iv0[2]);
		break;
	}

	Ov[0] = Iv0[0] + (Iv1[0] - Iv0[0]) * t;
	Ov[1] = Iv0[1] + (Iv1[1] - Iv0[1]) * t;
	Ov[2] = Iv0[2] + (Iv1[2] - Iv0[2]) * t;
	Ov[3] = Iv0[3] + (Iv1[3] - Iv0[3]) * t;

	Ot[0] = It0[0] + (It1[0] - It0[0]) * t;
	Ot[1] = It0[1] + (It1[1] - It0[1]) * t;
	Ot[2] = It0[2] + (It1[2] - It0[2]) * t;
	Ot[3] = It0[3] + (It1[3] - It0[3]) * t;

	Ol[0] = Il0[0] + (Il1[0] - Il0[0]) * t;
	Ol[1] = Il0[1] + (Il1[1] - Il0[1]) * t;
	Ol[2] = Il0[2] + (Il1[2] - Il0[2]) * t;
}

static void performFrustumClip(struct polygon *p)
{
	int i, j, k;
	//////////////////////////////////////////////////////////////////////////
	// Clip against the volumes defined by the homogeneous clip coordinates //
	//////////////////////////////////////////////////////////////////////////

	struct polygon temp;

	struct polyVert *v0;
	struct polyVert *v1;
	struct polyVert *tv;

	temp.n = 0;

	// Skip near and far clipping planes ?
	for (j = 0; j <= HNG64_BOTTOM; j++)
	{
		for (i = 0; i < p->n; i++)
		{
			k = (i+1) % p->n; // Index of next vertex

			v0 = &p->vert[i];
			v1 = &p->vert[k];

			tv = &temp.vert[temp.n];

			if (Inside(v0, j) && Inside(v1, j))							// Edge is completely inside the volume...
			{
				memcpy(tv, v1, sizeof(struct polyVert));
				temp.n++;
			}

			else if (Inside(v0, j) && !Inside(v1, j))					// Edge goes from in to out...
			{
				Intersect(v0, v1, tv, j);
				temp.n++;
			}

			else if (!Inside(v0, j) && Inside(v1, j))					// Edge goes from out to in...
			{
				Intersect(v0, v1, tv, j);
				memcpy(&temp.vert[temp.n+1], v1, sizeof(struct polyVert));
				temp.n+=2;
			}
		}

		p->n = temp.n;

		for (i = 0; i < temp.n; i++)
		{
			memcpy(&p->vert[i], &temp.vert[i], sizeof(struct polyVert));
		}

		temp.n = 0;
	}
}


/////////////////////////
// wireframe rendering //
/////////////////////////
#ifdef UNUSED_FUNCTION
static void plot(running_machine &machine, INT32 x, INT32 y, UINT32 color)
{
	UINT32* cb = &(colorBuffer3d[(y * machine.primary_screen->visible_area().max_x) + x]);
	*cb = color;
}

// Stolen from http://en.wikipedia.org/wiki/Bresenham's_line_algorithm (no copyright denoted) - the non-optimized version
static void drawline2d(running_machine &machine, INT32 x0, INT32 y0, INT32 x1, INT32 y1, UINT32 color)
{
#define SWAP(a,b) tmpswap = a; a = b; b = tmpswap;

	INT32 i;
	INT32 steep = 1;
	INT32 sx, sy;  /* step positive or negative (1 or -1) */
	INT32 dx, dy;  /* delta (difference in X and Y between points) */
	INT32 e;

	/*
    * inline swap. On some architectures, the XOR trick may be faster
    */
	INT32 tmpswap;

	/*
    * optimize for vertical and horizontal lines here
    */

	dx = abs(x1 - x0);
	sx = ((x1 - x0) > 0) ? 1 : -1;
	dy = abs(y1 - y0);
	sy = ((y1 - y0) > 0) ? 1 : -1;

	if (dy > dx)
	{
		steep = 0;
		SWAP(x0, y0);
		SWAP(dx, dy);
		SWAP(sx, sy);
	}

	e = (dy << 1) - dx;

	for (i = 0; i < dx; i++)
	{
		if (steep)
		{
			plot(machine, x0, y0, color);
		}
		else
		{
			plot(machine, y0, x0, color);
		}
		while (e >= 0)
		{
			y0 += sy;
			e -= (dx << 1);
		}

		x0 += sx;
		e += (dy << 1);
	}
#undef SWAP
}

static void DrawWireframe(running_machine &machine, struct polygon *p)
{
	int j;
	for (j = 0; j < p->n; j++)
	{
		// mame_printf_debug("now drawing : %f %f %f, %f %f %f\n", p->vert[j].clipCoords[0], p->vert[j].clipCoords[1], p->vert[j].clipCoords[2], p->vert[(j+1)%p->n].clipCoords[0], p->vert[(j+1)%p->n].clipCoords[1], p->vert[(j+1)%p->n].clipCoords[2]);
		// mame_printf_debug("%f %f %f %f\n", p->vert[j].clipCoords[0], p->vert[j].clipCoords[1], p->vert[(j+1)%p->n].clipCoords[0], p->vert[(j+1)%p->n].clipCoords[1]);
		UINT32 color = MAKE_ARGB((UINT8)255, (UINT8)255, (UINT8)0, (UINT8)0);
		drawline2d(machine, p->vert[j].clipCoords[0], p->vert[j].clipCoords[1], p->vert[(j+1)%p->n].clipCoords[0], p->vert[(j+1)%p->n].clipCoords[1], color);
	}

	// SHOWS THE CLIPPING //
#if 0
	for (int j = 1; j < p->n-1; j++)
	{
		drawline2d(p->vert[0].clipCoords[0],   p->vert[0].clipCoords[1],   p->vert[j].clipCoords[0],   p->vert[j].clipCoords[1],   255, bitmap);
		drawline2d(p->vert[j].clipCoords[0],   p->vert[j].clipCoords[1],   p->vert[j+1].clipCoords[0], p->vert[j+1].clipCoords[1], 255, bitmap);
		drawline2d(p->vert[j+1].clipCoords[0], p->vert[j+1].clipCoords[1], p->vert[0].clipCoords[0],   p->vert[0].clipCoords[1],   255, bitmap);
	}
#endif
}
#endif

///////////////////////
// polygon rendering //
///////////////////////

struct polygonRasterOptions
{
	UINT8 texType;
	UINT8 texIndex;
	UINT8 texPageSmall;
	UINT8 texPageHorizOffset;
	UINT8 texPageVertOffset;
	int palOffset;
	int palPageSize;
	int debugColor;
};

/*********************************************************************/
/**   FillSmoothTexPCHorizontalLine                                 **/
/**     Input: Color Buffer (framebuffer), depth buffer, width and  **/
/**            height of framebuffer, starting, and ending values   **/
/**            for x and y, constant y.  Fills horizontally with    **/
/**            z,r,g,b interpolation.                               **/
/**                                                                 **/
/**     Output: none                                                **/
/*********************************************************************/
INLINE void FillSmoothTexPCHorizontalLine(running_machine &machine,
										  const polygonRasterOptions& prOptions,
										  int x_start, int x_end, int y, float z_start, float z_delta,
										  float w_start, float w_delta, float r_start, float r_delta,
										  float g_start, float g_delta, float b_start, float b_delta,
										  float s_start, float s_delta, float t_start, float t_delta)
{
	hng64_state *state = machine.driver_data<hng64_state>();
	float*  db = &(state->m_depthBuffer3d[(y * machine.primary_screen->visible_area().max_x) + x_start]);
	UINT32* cb = &(state->m_colorBuffer3d[(y * machine.primary_screen->visible_area().max_x) + x_start]);

	UINT8 paletteEntry = 0;
	float t_coord, s_coord;
	const UINT8 *gfx = machine.region("textures")->base();
	const UINT8 *textureOffset = &gfx[prOptions.texIndex * 1024 * 1024];

	for (; x_start <= x_end; x_start++)
	{
		if (z_start < (*db))
		{
			// MULTIPLY BACK THROUGH BY W
			t_coord = t_start / w_start;
			s_coord = s_start / w_start;

			if ((prOptions.debugColor & 0xff000000) == 0x01000000)
			{
				// UV COLOR MODE
				*cb = MAKE_ARGB(255, (UINT8)(s_coord*255.0f), (UINT8)(t_coord*255.0f), (UINT8)(0));
				*db = z_start;
			}
			else if ((prOptions.debugColor & 0xff000000) == 0x02000000)
			{
				// Lit
				*cb = MAKE_ARGB(255, (UINT8)(r_start/w_start), (UINT8)(g_start/w_start), (UINT8)(b_start/w_start));
				*db = z_start;
			}
			else if ((prOptions.debugColor & 0xff000000) == 0xff000000)
			{
				// DEBUG COLOR MODE
				*cb = prOptions.debugColor;
				*db = z_start;
			}
			else
			{
				float textureS = 0.0f;
				float textureT = 0.0f;

				// Standard & Half-Res textures
				if (prOptions.texType == 0x0)
				{
					textureS = s_coord * 1024.0f;
					textureT = t_coord * 1024.0f;
				}
				else if (prOptions.texType == 0x1)
				{
					textureS = s_coord * 512.0f;
					textureT = t_coord * 512.0f;
				}

				// Small-Page textures
				if (prOptions.texPageSmall)
				{
					textureT = fmod(textureT, 256.0f);
					textureS = fmod(textureS, 256.0f);

					textureT += (256.0f * prOptions.texPageHorizOffset);
					textureS += (256.0f * prOptions.texPageVertOffset);
				}
				paletteEntry = textureOffset[((int)textureS)*1024 + (int)textureT];

				// Naieve Alpha Implementation (?) - don't draw if you're at texture index 0...
				if (paletteEntry != 0)
				{
					// The color out of the texture
					paletteEntry %= prOptions.palPageSize;
					UINT32 color = machine.pens[prOptions.palOffset + paletteEntry];

					// Apply the lighting
					float rIntensity = (r_start/w_start) / 255.0f;
					float gIntensity = (g_start/w_start) / 255.0f;
					float bIntensity = (b_start/w_start) / 255.0f;
					float red   = RGB_RED(color)   * rIntensity;
					float green = RGB_GREEN(color) * gIntensity;
					float blue  = RGB_BLUE(color)  * bIntensity;

					// Clamp and finalize
					red = RGB_RED(color) + red;
					green = RGB_GREEN(color) + green;
					blue = RGB_BLUE(color) + blue;

					if (red >= 255) red = 255;
					if (green >= 255) green = 255;
					if (blue >= 255) blue = 255;

					color = MAKE_ARGB(255, (UINT8)red, (UINT8)green, (UINT8)blue);

					*cb = color;
					*db = z_start;
				}
			}
		}
		db++;
		cb++;
		z_start += z_delta;
		w_start += w_delta;
		r_start += r_delta;
		g_start += g_delta;
		b_start += b_delta;
		s_start += s_delta;
		t_start += t_delta;
	}
}

//----------------------------------------------------------------------------
// Given 3D triangle ABC in screen space with clipped coordinates within the following
// bounds: x in [0,W], y in [0,H], z in [0,1]. The origin for (x,y) is in the bottom
// left corner of the pixel grid. z=0 is the near plane and z=1 is the far plane,
// so lesser values are closer. The coordinates of the pixels are evenly spaced
// in x and y 1 units apart starting at the bottom-left pixel with coords
// (0.5,0.5). In other words, the pixel sample point is in the center of the
// rectangular grid cell containing the pixel sample. The framebuffer has
// dimensions width x height (WxH). The Color buffer is a 1D array (row-major
// order) with 3 unsigned chars per pixel (24-bit color). The Depth buffer is
// a 1D array (also row-major order) with a float value per pixel
// For a pixel location (x,y) we can obtain
// the Color and Depth array locations as: Color[(((int)y)*W+((int)x))*3]
// (for the red value, green is offset +1, and blue is offset +2 and
// Depth[((int)y)*W+((int)x)]. Fills the pixels contained in the triangle
// with the global current color and the properly linearly interpolated depth
// value (performs Z-buffer depth test before writing new pixel).
// Pixel samples that lie inside the triangle edges are filled with
// a bias towards the minimum values (samples that lie exactly on a triangle
// edge are filled only for minimum x values along a horizontal span and for
// minimum y values, samples lying on max values are not filled).
// Per-vertex colors are RGB floating point triplets in [0.0,255.0]. The vertices
// include their w-components for use in linearly interpolating perspectively
// correct color (RGB) and texture-coords (st) across the face of the triangle.
// A texture image of RGB floating point triplets of size TWxWH is also given.
// Texture colors are normalized RGB values in [0,1].
//   clamp and repeat wrapping modes : Wrapping={0,1}
//   nearest and bilinear filtering: Filtering={0,1}
//   replace and modulate application modes: Function={0,1}
//---------------------------------------------------------------------------
static void RasterizeTriangle_SMOOTH_TEX_PC(running_machine &machine,
											float A[4], float B[4], float C[4],
											float Ca[3], float Cb[3], float Cc[3], // PER-VERTEX RGB COLORS
											float Ta[2], float Tb[2], float Tc[2], // PER-VERTEX (S,T) TEX-COORDS
											const polygonRasterOptions& prOptions)
{
	// Get our order of points by increasing y-coord
	float *p_min = ((A[1] <= B[1]) && (A[1] <= C[1])) ? A : ((B[1] <= A[1]) && (B[1] <= C[1])) ? B : C;
	float *p_max = ((A[1] >= B[1]) && (A[1] >= C[1])) ? A : ((B[1] >= A[1]) && (B[1] >= C[1])) ? B : C;
	float *p_mid = ((A != p_min) && (A != p_max)) ? A : ((B != p_min) && (B != p_max)) ? B : C;

	// Perspectively correct color interpolation, interpolate r/w, g/w, b/w, then divide by 1/w at each pixel (A[3] = 1/w)
	float ca[3], cb[3], cc[3];
	float ta[2], tb[2], tc[2];

	float *c_min;
	float *c_mid;
	float *c_max;

	// We must keep the tex coords straight with the point ordering
	float *t_min;
	float *t_mid;
	float *t_max;

	// Find out control points for y, this divides the triangle into upper and lower
	int   y_min;
	int   y_max;
	int   y_mid;

	// Compute the slopes of each line, and color this is used to determine the interpolation
	float x1_slope;
	float x2_slope;
	float z1_slope;
	float z2_slope;
	float w1_slope;
	float w2_slope;
	float r1_slope;
	float r2_slope;
	float g1_slope;
	float g2_slope;
	float b1_slope;
	float b2_slope;
	float s1_slope;
	float s2_slope;
	float t1_slope;
	float t2_slope;

	// Compute the t values used in the equation Ax = Ax + (Bx - Ax)*t
	// We only need one t, because it is only used to compute the start.
	// Create storage for the interpolated x and z values for both lines
	// also for the RGB interpolation
	float t;
	float x1_interp;
	float z1_interp;
	float w1_interp;
	float r1_interp;
	float g1_interp;
	float b1_interp;
	float s1_interp;
	float t1_interp;

	float x2_interp;
	float z2_interp;
	float w2_interp;
	float r2_interp;
	float g2_interp;
	float b2_interp;
	float s2_interp;
	float t2_interp;

	// Create storage for the horizontal interpolation of z and RGB color and its starting points
	// This is used to fill the triangle horizontally
	int   x_start,     x_end;
	float z_interp_x,  z_delta_x;
	float w_interp_x,  w_delta_x;
	float r_interp_x,  r_delta_x;
	float g_interp_x,  g_delta_x;
	float b_interp_x,  b_delta_x;
	float s_interp_x,  s_delta_x;
	float t_interp_x,  t_delta_x;

	ca[0] = Ca[0]; ca[1] = Ca[1]; ca[2] = Ca[2];
	cb[0] = Cb[0]; cb[1] = Cb[1]; cb[2] = Cb[2];
	cc[0] = Cc[0]; cc[1] = Cc[1]; cc[2] = Cc[2];

	// Perspectively correct tex interpolation, interpolate s/w, t/w, then divide by 1/w at each pixel (A[3] = 1/w)
	ta[0] = Ta[0]; ta[1] = Ta[1];
	tb[0] = Tb[0]; tb[1] = Tb[1];
	tc[0] = Tc[0]; tc[1] = Tc[1];

	// We must keep the colors straight with the point ordering
	c_min = (p_min == A) ? ca : (p_min == B) ? cb : cc;
	c_mid = (p_mid == A) ? ca : (p_mid == B) ? cb : cc;
	c_max = (p_max == A) ? ca : (p_max == B) ? cb : cc;

	// We must keep the tex coords straight with the point ordering
	t_min = (p_min == A) ? ta : (p_min == B) ? tb : tc;
	t_mid = (p_mid == A) ? ta : (p_mid == B) ? tb : tc;
	t_max = (p_max == A) ? ta : (p_max == B) ? tb : tc;

	// Find out control points for y, this divides the triangle into upper and lower
	y_min  = (((int)p_min[1]) + 0.5 >= p_min[1]) ? (int)p_min[1] : ((int)p_min[1]) + 1;
	y_max  = (((int)p_max[1]) + 0.5 <  p_max[1]) ? (int)p_max[1] : ((int)p_max[1]) - 1;
	y_mid  = (((int)p_mid[1]) + 0.5 >= p_mid[1]) ? (int)p_mid[1] : ((int)p_mid[1]) + 1;

	// Compute the slopes of each line, and color this is used to determine the interpolation
	x1_slope = (p_max[0] - p_min[0]) / (p_max[1] - p_min[1]);
	x2_slope = (p_mid[0] - p_min[0]) / (p_mid[1] - p_min[1]);
	z1_slope = (p_max[2] - p_min[2]) / (p_max[1] - p_min[1]);
	z2_slope = (p_mid[2] - p_min[2]) / (p_mid[1] - p_min[1]);
	w1_slope = (p_max[3] - p_min[3]) / (p_max[1] - p_min[1]);
	w2_slope = (p_mid[3] - p_min[3]) / (p_mid[1] - p_min[1]);
	r1_slope = (c_max[0] - c_min[0]) / (p_max[1] - p_min[1]);
	r2_slope = (c_mid[0] - c_min[0]) / (p_mid[1] - p_min[1]);
	g1_slope = (c_max[1] - c_min[1]) / (p_max[1] - p_min[1]);
	g2_slope = (c_mid[1] - c_min[1]) / (p_mid[1] - p_min[1]);
	b1_slope = (c_max[2] - c_min[2]) / (p_max[1] - p_min[1]);
	b2_slope = (c_mid[2] - c_min[2]) / (p_mid[1] - p_min[1]);
	s1_slope = (t_max[0] - t_min[0]) / (p_max[1] - p_min[1]);
	s2_slope = (t_mid[0] - t_min[0]) / (p_mid[1] - p_min[1]);
	t1_slope = (t_max[1] - t_min[1]) / (p_max[1] - p_min[1]);
	t2_slope = (t_mid[1] - t_min[1]) / (p_mid[1] - p_min[1]);

	// Compute the t values used in the equation Ax = Ax + (Bx - Ax)*t
	// We only need one t, because it is only used to compute the start.
	// Create storage for the interpolated x and z values for both lines
	// also for the RGB interpolation
	t = (((float)y_min) + 0.5 - p_min[1]) / (p_max[1] - p_min[1]);
	x1_interp = p_min[0] + (p_max[0] - p_min[0]) * t;
	z1_interp = p_min[2] + (p_max[2] - p_min[2]) * t;
	w1_interp = p_min[3] + (p_max[3] - p_min[3]) * t;
	r1_interp = c_min[0] + (c_max[0] - c_min[0]) * t;
	g1_interp = c_min[1] + (c_max[1] - c_min[1]) * t;
	b1_interp = c_min[2] + (c_max[2] - c_min[2]) * t;
	s1_interp = t_min[0] + (t_max[0] - t_min[0]) * t;
	t1_interp = t_min[1] + (t_max[1] - t_min[1]) * t;

	t = (((float)y_min) + 0.5 - p_min[1]) / (p_mid[1] - p_min[1]);
	x2_interp = p_min[0] + (p_mid[0] - p_min[0]) * t;
	z2_interp = p_min[2] + (p_mid[2] - p_min[2]) * t;
	w2_interp = p_min[3] + (p_mid[3] - p_min[3]) * t;
	r2_interp = c_min[0] + (c_mid[0] - c_min[0]) * t;
	g2_interp = c_min[1] + (c_mid[1] - c_min[1]) * t;
	b2_interp = c_min[2] + (c_mid[2] - c_min[2]) * t;
	s2_interp = t_min[0] + (t_mid[0] - t_min[0]) * t;
	t2_interp = t_min[1] + (t_mid[1] - t_min[1]) * t;

	// First work on the bottom half of the triangle
	// I'm using y_min as the incrementer because it saves space and we don't need it anymore
	for (; y_min < y_mid; y_min++) {

		// We always want to fill left to right, so we have 2 main cases
		// Compute the integer starting and ending points and the appropriate z by
		// interpolating.  Remember the pixels are in the middle of the grid, i.e. (0.5,0.5,0.5)
		if (x1_interp < x2_interp) {
			x_start    = ((((int)x1_interp) + 0.5) >= x1_interp) ? (int)x1_interp : ((int)x1_interp) + 1;
			x_end      = ((((int)x2_interp) + 0.5) <  x2_interp) ? (int)x2_interp : ((int)x2_interp) - 1;
			z_delta_x  = (z2_interp - z1_interp) / (x2_interp - x1_interp);
			w_delta_x  = (w2_interp - w1_interp) / (x2_interp - x1_interp);
			r_delta_x  = (r2_interp - r1_interp) / (x2_interp - x1_interp);
			g_delta_x  = (g2_interp - g1_interp) / (x2_interp - x1_interp);
			b_delta_x  = (b2_interp - b1_interp) / (x2_interp - x1_interp);
			s_delta_x  = (s2_interp - s1_interp) / (x2_interp - x1_interp);
			t_delta_x  = (t2_interp - t1_interp) / (x2_interp - x1_interp);
			t          = (x_start + 0.5 - x1_interp) / (x2_interp - x1_interp);
			z_interp_x = z1_interp + (z2_interp - z1_interp) * t;
			w_interp_x = w1_interp + (w2_interp - w1_interp) * t;
			r_interp_x = r1_interp + (r2_interp - r1_interp) * t;
			g_interp_x = g1_interp + (g2_interp - g1_interp) * t;
			b_interp_x = b1_interp + (b2_interp - b1_interp) * t;
			s_interp_x = s1_interp + (s2_interp - s1_interp) * t;
			t_interp_x = t1_interp + (t2_interp - t1_interp) * t;

		} else {
			x_start    = ((((int)x2_interp) + 0.5) >= x2_interp) ? (int)x2_interp : ((int)x2_interp) + 1;
			x_end      = ((((int)x1_interp) + 0.5) <  x1_interp) ? (int)x1_interp : ((int)x1_interp) - 1;
			z_delta_x  = (z1_interp - z2_interp) / (x1_interp - x2_interp);
			w_delta_x  = (w1_interp - w2_interp) / (x1_interp - x2_interp);
			r_delta_x  = (r1_interp - r2_interp) / (x1_interp - x2_interp);
			g_delta_x  = (g1_interp - g2_interp) / (x1_interp - x2_interp);
			b_delta_x  = (b1_interp - b2_interp) / (x1_interp - x2_interp);
			s_delta_x  = (s1_interp - s2_interp) / (x1_interp - x2_interp);
			t_delta_x  = (t1_interp - t2_interp) / (x1_interp - x2_interp);
			t          = (x_start + 0.5 - x2_interp) / (x1_interp - x2_interp);
			z_interp_x = z2_interp + (z1_interp - z2_interp) * t;
			w_interp_x = w2_interp + (w1_interp - w2_interp) * t;
			r_interp_x = r2_interp + (r1_interp - r2_interp) * t;
			g_interp_x = g2_interp + (g1_interp - g2_interp) * t;
			b_interp_x = b2_interp + (b1_interp - b2_interp) * t;
			s_interp_x = s2_interp + (s1_interp - s2_interp) * t;
			t_interp_x = t2_interp + (t1_interp - t2_interp) * t;
		}

		// Pass the horizontal line to the filler, this could be put in the routine
		// then interpolate for the next values of x and z
		FillSmoothTexPCHorizontalLine(machine, prOptions,
			x_start, x_end, y_min, z_interp_x, z_delta_x, w_interp_x, w_delta_x,
			r_interp_x, r_delta_x, g_interp_x, g_delta_x, b_interp_x, b_delta_x,
			s_interp_x, s_delta_x, t_interp_x, t_delta_x);
		x1_interp += x1_slope;   z1_interp += z1_slope;
		x2_interp += x2_slope;   z2_interp += z2_slope;
		r1_interp += r1_slope;   r2_interp += r2_slope;
		g1_interp += g1_slope;   g2_interp += g2_slope;
		b1_interp += b1_slope;   b2_interp += b2_slope;
		w1_interp += w1_slope;   w2_interp += w2_slope;
		s1_interp += s1_slope;   s2_interp += s2_slope;
		t1_interp += t1_slope;   t2_interp += t2_slope;
	}

	// Now do the same thing for the top half of the triangle.
	// We only need to recompute the x2 line because it changes at the midpoint
	x2_slope = (p_max[0] - p_mid[0]) / (p_max[1] - p_mid[1]);
	z2_slope = (p_max[2] - p_mid[2]) / (p_max[1] - p_mid[1]);
	w2_slope = (p_max[3] - p_mid[3]) / (p_max[1] - p_mid[1]);
	r2_slope = (c_max[0] - c_mid[0]) / (p_max[1] - p_mid[1]);
	g2_slope = (c_max[1] - c_mid[1]) / (p_max[1] - p_mid[1]);
	b2_slope = (c_max[2] - c_mid[2]) / (p_max[1] - p_mid[1]);
	s2_slope = (t_max[0] - t_mid[0]) / (p_max[1] - p_mid[1]);
	t2_slope = (t_max[1] - t_mid[1]) / (p_max[1] - p_mid[1]);

	t = (((float)y_mid) + 0.5 - p_mid[1]) / (p_max[1] - p_mid[1]);
	x2_interp = p_mid[0] + (p_max[0] - p_mid[0]) * t;
	z2_interp = p_mid[2] + (p_max[2] - p_mid[2]) * t;
	w2_interp = p_mid[3] + (p_max[3] - p_mid[3]) * t;
	r2_interp = c_mid[0] + (c_max[0] - c_mid[0]) * t;
	g2_interp = c_mid[1] + (c_max[1] - c_mid[1]) * t;
	b2_interp = c_mid[2] + (c_max[2] - c_mid[2]) * t;
	s2_interp = t_mid[0] + (t_max[0] - t_mid[0]) * t;
	t2_interp = t_mid[1] + (t_max[1] - t_mid[1]) * t;

	// We've seen this loop before haven't we?
	// I'm using y_mid as the incrementer because it saves space and we don't need it anymore
	for (; y_mid <= y_max; y_mid++) {

		if (x1_interp < x2_interp) {
			x_start    = ((((int)x1_interp) + 0.5) >= x1_interp) ? (int)x1_interp : ((int)x1_interp) + 1;
			x_end      = ((((int)x2_interp) + 0.5) <  x2_interp) ? (int)x2_interp : ((int)x2_interp) - 1;
			z_delta_x  = (z2_interp - z1_interp) / (x2_interp - x1_interp);
			w_delta_x  = (w2_interp - w1_interp) / (x2_interp - x1_interp);
			r_delta_x  = (r2_interp - r1_interp) / (x2_interp - x1_interp);
			g_delta_x  = (g2_interp - g1_interp) / (x2_interp - x1_interp);
			b_delta_x  = (b2_interp - b1_interp) / (x2_interp - x1_interp);
			s_delta_x  = (s2_interp - s1_interp) / (x2_interp - x1_interp);
			t_delta_x  = (t2_interp - t1_interp) / (x2_interp - x1_interp);
			t          = (x_start + 0.5 - x1_interp) / (x2_interp - x1_interp);
			z_interp_x = z1_interp + (z2_interp - z1_interp) * t;
			w_interp_x = w1_interp + (w2_interp - w1_interp) * t;
			r_interp_x = r1_interp + (r2_interp - r1_interp) * t;
			g_interp_x = g1_interp + (g2_interp - g1_interp) * t;
			b_interp_x = b1_interp + (b2_interp - b1_interp) * t;
			s_interp_x = s1_interp + (s2_interp - s1_interp) * t;
			t_interp_x = t1_interp + (t2_interp - t1_interp) * t;

		} else {
			x_start    = ((((int)x2_interp) + 0.5) >= x2_interp) ? (int)x2_interp : ((int)x2_interp) + 1;
			x_end      = ((((int)x1_interp) + 0.5) <  x1_interp) ? (int)x1_interp : ((int)x1_interp) - 1;
			z_delta_x  = (z1_interp - z2_interp) / (x1_interp - x2_interp);
			w_delta_x  = (w1_interp - w2_interp) / (x1_interp - x2_interp);
			r_delta_x  = (r1_interp - r2_interp) / (x1_interp - x2_interp);
			g_delta_x  = (g1_interp - g2_interp) / (x1_interp - x2_interp);
			b_delta_x  = (b1_interp - b2_interp) / (x1_interp - x2_interp);
			s_delta_x  = (s1_interp - s2_interp) / (x1_interp - x2_interp);
			t_delta_x  = (t1_interp - t2_interp) / (x1_interp - x2_interp);
			t          = (x_start + 0.5 - x2_interp) / (x1_interp - x2_interp);
			z_interp_x = z2_interp + (z1_interp - z2_interp) * t;
			w_interp_x = w2_interp + (w1_interp - w2_interp) * t;
			r_interp_x = r2_interp + (r1_interp - r2_interp) * t;
			g_interp_x = g2_interp + (g1_interp - g2_interp) * t;
			b_interp_x = b2_interp + (b1_interp - b2_interp) * t;
			s_interp_x = s2_interp + (s1_interp - s2_interp) * t;
			t_interp_x = t2_interp + (t1_interp - t2_interp) * t;
		}

		// Pass the horizontal line to the filler, this could be put in the routine
		// then interpolate for the next values of x and z
		FillSmoothTexPCHorizontalLine(machine, prOptions,
			x_start, x_end, y_mid, z_interp_x, z_delta_x, w_interp_x, w_delta_x,
			r_interp_x, r_delta_x, g_interp_x, g_delta_x, b_interp_x, b_delta_x,
			s_interp_x, s_delta_x, t_interp_x, t_delta_x);
		x1_interp += x1_slope;   z1_interp += z1_slope;
		x2_interp += x2_slope;   z2_interp += z2_slope;
		r1_interp += r1_slope;   r2_interp += r2_slope;
		g1_interp += g1_slope;   g2_interp += g2_slope;
		b1_interp += b1_slope;   b2_interp += b2_slope;
		w1_interp += w1_slope;   w2_interp += w2_slope;
		s1_interp += s1_slope;   s2_interp += s2_slope;
		t1_interp += t1_slope;   t2_interp += t2_slope;
	}
}

static void drawShaded(running_machine &machine, struct polygon *p)
{
	// The perspective-correct texture divide...
	// !!! There is a very good chance the HNG64 hardware does not do perspective-correct texture-mapping !!!
	int j;
	for (j = 0; j < p->n; j++)
	{
		p->vert[j].clipCoords[3] = 1.0f / p->vert[j].clipCoords[3];
		p->vert[j].light[0]      = p->vert[j].light[0]     * p->vert[j].clipCoords[3];
		p->vert[j].light[1]      = p->vert[j].light[1]     * p->vert[j].clipCoords[3];
		p->vert[j].light[2]      = p->vert[j].light[2]     * p->vert[j].clipCoords[3];
		p->vert[j].texCoords[0]  = p->vert[j].texCoords[0] * p->vert[j].clipCoords[3];
		p->vert[j].texCoords[1]  = p->vert[j].texCoords[1] * p->vert[j].clipCoords[3];
	}

	// Set up the struct that will pass the polygon's options around.
	polygonRasterOptions prOptions;
	prOptions.texType = p->texType;
	prOptions.texIndex = p->texIndex;
	prOptions.palOffset = p->palOffset;
	prOptions.palPageSize = p->palPageSize;
	prOptions.debugColor = p->debugColor;
	prOptions.texPageSmall = p->texPageSmall;
	prOptions.texPageHorizOffset = p->texPageHorizOffset;
	prOptions.texPageVertOffset = p->texPageVertOffset;

	for (j = 1; j < p->n-1; j++)
	{
		RasterizeTriangle_SMOOTH_TEX_PC(machine,
										p->vert[0].clipCoords, p->vert[j].clipCoords, p->vert[j+1].clipCoords,
										p->vert[0].light,      p->vert[j].light,      p->vert[j+1].light,
										p->vert[0].texCoords,  p->vert[j].texCoords,  p->vert[j+1].texCoords,
										prOptions);
	}
}



