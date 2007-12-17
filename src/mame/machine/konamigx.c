#define VERBOSE 0
#define GX_DEBUG 0

/**************************************************************************
 *
 * machine/konamigx.c - contains various System GX hardware abstractions
 *
 * Currently includes: TMS57002 skipper/simulator to make the sound 68k happy.
 *
 */

#include "driver.h"
#include "video/konamiic.h"
#include "machine/konamigx.h"
#include <math.h>


static struct
{
	UINT8 control;
	UINT8 program[1024];
	UINT32 tables[256];
	int curpos;
	int bytepos;
	UINT32 tmp;
} tms57002;

static int ldw = 0;


void tms57002_init(void)
{
	tms57002.control = 0;
}

static void chk_ldw(void)
{
	ldw = 0;
}

WRITE8_HANDLER( tms57002_control_w )
{
	chk_ldw();

	switch(tms57002.control)
	{
		case 0xf8:
		break;
		case 0xf0:
		break;
	}

	tms57002.control = data;

	switch(data)
	{
		case 0xf8: // Program write
		case 0xf0: // Table write
			tms57002.curpos = 0;
			tms57002.bytepos = 3;
			tms57002.tmp = 0;
		break;
		case 0xf4: // Entry write
			tms57002.curpos = -1;
			tms57002.bytepos = 3;
			tms57002.tmp = 0;
		break;
		case 0xfc: // Checksum (?) Status (?)
			tms57002.bytepos = 3;
			tms57002.tmp = 0;
		break;
		case 0xff: // Standby
		break;
		case 0xfe: // Irq
			/* (place ack of timer IRQ here) */
		break;
		default:
		break;
	}
}

READ8_HANDLER( tms57002_status_r )
{
	chk_ldw();
	return 1;
}

WRITE8_HANDLER( tms57002_data_w )
{
	switch(tms57002.control)
	{
		case 0xf8:
			tms57002.program[tms57002.curpos++] = data;
		break;
		case 0xf0:
			tms57002.tmp |= data << (8*tms57002.bytepos);
			tms57002.bytepos--;
			if (tms57002.bytepos < 0)
			{
				tms57002.bytepos = 3;
				tms57002.tables[tms57002.curpos++] =  tms57002.tmp;
				tms57002.tmp = 0;
			}
		break;
		case 0xf4:
			if (tms57002.curpos == -1)
	  			tms57002.curpos = data;
			else
			{
				tms57002.tmp |= data << (8*tms57002.bytepos);
				tms57002.bytepos--;

				if (tms57002.bytepos < 0)
				{
					tms57002.bytepos = 3;
					tms57002.tables[tms57002.curpos] =  tms57002.tmp;
					tms57002.tmp = 0;
					tms57002.curpos = -1;
				}
			}
		break;
		default:
			ldw++;
		break;
	}
}

READ8_HANDLER( tms57002_data_r )
{
	UINT8 res;

	chk_ldw();

	switch(tms57002.control)
	{
		case 0xfc:
			res = tms57002.tmp >> (8*tms57002.bytepos);
			tms57002.bytepos--;

			if(tms57002.bytepos < 0) tms57002.bytepos = 3;
		return res;
	}

	return 0;
}


READ16_HANDLER( tms57002_data_word_r )
{
	return(tms57002_data_r(0));
}

READ16_HANDLER( tms57002_status_word_r )
{
	return(tms57002_status_r(0));
}

WRITE16_HANDLER( tms57002_control_word_w )
{
	tms57002_control_w(0, data);
}

WRITE16_HANDLER( tms57002_data_word_w )
{
	tms57002_data_w(0, data);
}



/***************************************************************************/
/*                                                                         */
/*                     2nd-Tier GX/MW Graphics Functions                   */
/*                                                                         */
/***************************************************************************/

#if GX_DEBUG
	#define GX_ZBUFW     512
	#define GX_ZBUFH     384
	#define GX_ZPAGESIZE 0x300000
	#define GX_ZBUFSIZE  0x600000
#else
	#define GX_ZBUFW     384
	#define GX_ZBUFH     224
	#define GX_ZPAGESIZE 0x150000
	#define GX_ZBUFSIZE  0x2a0000
#endif

static UINT8 *gx_objzbuf, *gx_shdzbuf;


// Localized K053936/ROZ+
#define K053936_MAX_CHIPS 2

static rectangle K053936_cliprect[K053936_MAX_CHIPS] = {{0,0,0,0},{0,0,0,0}};
static int K053936_offset[K053936_MAX_CHIPS][2] = {{0,0},{0,0}};
static int K053936_clip_enabled[K053936_MAX_CHIPS] = {0,0};


void K053936GP_set_offset(int chip, int xoffs, int yoffs) { K053936_offset[chip][0] = xoffs; K053936_offset[chip][1] = yoffs; }

void K053936GP_clip_enable(int chip, int status) { K053936_clip_enabled[chip] = status; }

void K053936GP_set_cliprect(int chip, int minx, int maxx, int miny, int maxy)
{
	rectangle *cliprect = &K053936_cliprect[chip];
	cliprect->min_x = minx;
	cliprect->max_x = maxx;
	cliprect->min_y = miny;
	cliprect->max_y = maxy;
}

INLINE void K053936GP_copyroz32clip( mame_bitmap *dst_bitmap, mame_bitmap *src_bitmap,
		const rectangle *dst_cliprect, const rectangle *src_cliprect,
		UINT32 _startx,UINT32 _starty,int _incxx,int _incxy,int _incyx,int _incyy,
		int tilebpp, int blend, int clip )
{
	static const int colormask[8]={1,3,7,0xf,0x1f,0x3f,0x7f,0xff};

	int cy, cx;
	int ecx;
	int src_pitch, incxy, incxx;
	int src_minx, src_maxx, src_miny, src_maxy, cmask;
	UINT16 *src_base;
	const pen_t *pal_base;
	UINT32 *dst_ptr;

	int tx, dst_pitch;
	UINT32 *dst_base;
	int starty, incyy, startx, incyx, ty, sx, sy;

	incxy = _incxy; incxx = _incxx; incyy = _incyy; incyx = _incyx;
	starty = _starty; startx = _startx;

	if (src_cliprect && clip) // set source clip range to some extreme values when disabled
	{
		src_minx = src_cliprect->min_x;
		src_maxx = src_cliprect->max_x;
		src_miny = src_cliprect->min_y;
		src_maxy = src_cliprect->max_y;
	}
	else { src_minx = src_miny = -0x10000; src_maxx = src_maxy = 0x10000; }

	if (dst_cliprect) // set target clip range
	{
		sx = dst_cliprect->min_x;
		tx = dst_cliprect->max_x - sx + 1;
		sy = dst_cliprect->min_y;
		ty = dst_cliprect->max_y - sy + 1;

		startx += sx * incxx + sy * incyx;
		starty += sx * incxy + sy * incyy;
	}
	else { sx = sy = 0; tx = dst_bitmap->width; ty = dst_bitmap->height; }

	// adjust entry points and other loop constants
	dst_pitch = dst_bitmap->rowpixels;
	dst_base = (UINT32*)dst_bitmap->base + sy * dst_pitch + sx + tx;
	ecx = tx = -tx;

	tilebpp = (tilebpp-1) & 7;
	pal_base = Machine->remapped_colortable;
	cmask = colormask[tilebpp];

	src_pitch = src_bitmap->rowpixels;
	src_base = src_bitmap->base;

	dst_ptr = dst_base;
	cy = starty;
	cx = startx;

	if (blend > 0)
	{
		dst_base += dst_pitch;		// draw blended
		starty += incyy;
		startx += incyx;

		do {
			do {
				int srcx = (cx >> 16) & 0x1fff;
				int srcy = (cy >> 16) & 0x1fff;
				int pixel;

				cx += incxx;
				cy += incxy;
				if (srcx < src_minx || srcx > src_maxx || srcy < src_miny || srcy > src_maxy)
					continue;

				pixel = src_base[srcy * src_pitch + srcx];
				if (!(pixel & cmask))
					continue;

				dst_ptr[ecx] = alpha_blend32(pal_base[pixel], dst_ptr[ecx]);
			}
			while (++ecx);

			ecx = tx;
			dst_ptr = dst_base; dst_base += dst_pitch;
			cy = starty; starty += incyy;
			cx = startx; startx += incyx;
		} while (--ty);
	}
	else	//  draw solid
	{
		if (blend == 0)
		{
			dst_base += dst_pitch;
			starty += incyy;
			startx += incyx;
		}
		else
		{
			if ((sy & 1) ^ (blend & 1))
			{
				if (ty <= 1) return;

				dst_ptr += dst_pitch;
				cy += incyy;
				cx += incyx;
			}

			if (ty > 1)
			{
				ty >>= 1;
				dst_pitch <<= 1;
				incyy <<= 1;
				incyx <<= 1;

				dst_base = dst_ptr + dst_pitch;
				starty = cy + incyy;
				startx = cx + incyx;
			}
		}

		do {
			do {
				int srcx = (cx >> 16) & 0x1fff;
				int srcy = (cy >> 16) & 0x1fff;
				int pixel;

				cx += incxx;
				cy += incxy;
				if (srcx < src_minx || srcx > src_maxx || srcy < src_miny || srcy > src_maxy)
					continue;

				pixel = src_base[srcy * src_pitch + srcx];
				if (!(pixel & cmask))
					continue;

				dst_ptr[ecx] = pal_base[pixel];
			}
			while (++ecx);

			ecx = tx;
			dst_ptr = dst_base; dst_base += dst_pitch;
			cy = starty; starty += incyy;
			cx = startx; startx += incyx;
		} while (--ty);
	}
}

// adpoted from generic K053936_zoom_draw()
static void K053936GP_zoom_draw(int chip, UINT16 *ctrl, UINT16 *linectrl,
		mame_bitmap *bitmap, const rectangle *cliprect, tilemap *tmap,
		int tilebpp, int blend)
{
	mame_bitmap *src_bitmap;
	rectangle *src_cliprect;
	UINT16 *lineaddr;

	rectangle my_clip;
	UINT32 startx, starty;
	int incxx, incxy, incyx, incyy, y, maxy, clip;

	src_bitmap = tilemap_get_pixmap(tmap);
	src_cliprect = &K053936_cliprect[chip];
	clip = K053936_clip_enabled[chip];

	if (ctrl[0x07] & 0x0040)    /* "super" mode */
	{
		my_clip.min_x = cliprect->min_x;
		my_clip.max_x = cliprect->max_x;
		y = cliprect->min_y;
		maxy = cliprect->max_y;

		while (y <= maxy)
		{
			lineaddr = linectrl + ( ((y - K053936_offset[chip][1]) & 0x1ff) << 2);
			my_clip.min_y = my_clip.max_y = y;

			startx = (INT16)(lineaddr[0] + ctrl[0x00]) << 8;
			starty = (INT16)(lineaddr[1] + ctrl[0x01]) << 8;
			incxx  = (INT16)(lineaddr[2]);
			incxy  = (INT16)(lineaddr[3]);

			if (ctrl[0x06] & 0x8000) incxx <<= 8;
			if (ctrl[0x06] & 0x0080) incxy <<= 8;

			startx -= K053936_offset[chip][0] * incxx;
			starty -= K053936_offset[chip][0] * incxy;

			K053936GP_copyroz32clip(bitmap, src_bitmap, &my_clip, src_cliprect,
					startx<<5, starty<<5, incxx<<5, incxy<<5, 0, 0,
					tilebpp, blend, clip);
			y++;
		}
	}
	else    /* "simple" mode */
	{
		startx = (INT16)(ctrl[0x00]) << 8;
		starty = (INT16)(ctrl[0x01]) << 8;
		incyx  = (INT16)(ctrl[0x02]);
		incyy  = (INT16)(ctrl[0x03]);
		incxx  = (INT16)(ctrl[0x04]);
		incxy  = (INT16)(ctrl[0x05]);

		if (ctrl[0x06] & 0x4000) { incyx <<= 8; incyy <<= 8; }
		if (ctrl[0x06] & 0x0040) { incxx <<= 8; incxy <<= 8; }

		startx -= K053936_offset[chip][1] * incyx;
		starty -= K053936_offset[chip][1] * incyy;

		startx -= K053936_offset[chip][0] * incxx;
		starty -= K053936_offset[chip][0] * incxy;

		K053936GP_copyroz32clip(bitmap, src_bitmap, cliprect, src_cliprect,
				startx<<5, starty<<5, incxx<<5, incxy<<5, incyx<<5, incyy<<5,
				tilebpp, blend, clip);
	}
}

void K053936GP_0_zoom_draw(mame_bitmap *bitmap, const rectangle *cliprect,
		tilemap *tmap, int tilebpp, int blend)
{
	K053936GP_zoom_draw(0,K053936_0_ctrl,K053936_0_linectrl,bitmap,cliprect,tmap,tilebpp,blend);
}

void K053936GP_1_zoom_draw(mame_bitmap *bitmap, const rectangle *cliprect,
		tilemap *tmap, int tilebpp, int blend)
{
	K053936GP_zoom_draw(1,K053936_1_ctrl,K053936_1_linectrl,bitmap,cliprect,tmap,tilebpp,blend);
}



/*
    Parameter Notes
    ---------------
    clip    : *caller must supply a pointer to target clip rectangle
    alpha   : 0 = invisible, 255 = solid
    drawmode:
        0 = all pens solid
        1 = solid pens only
        2 = all pens solid with alpha blending
        3 = solid pens only with alpha blending
        4 = shadow pens only
        5 = all pens shadow
    zcode   : 0 = closest, 255 = furthest (pixel z-depth), -1 = disable depth buffers and shadows
    pri     : 0 = topmost, 255 = backmost (pixel priority)
*/

INLINE void zdrawgfxzoom32GP( mame_bitmap *bitmap, const gfx_element *gfx, const rectangle *cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy,
		int scalex, int scaley, int alpha, int drawmode, int zcode, int pri)
{
#define FP     19
#define FPONE  (1<<FP)
#define FPHALF (1<<(FP-1))
#define FPENT  0

	// inner loop
	UINT8  *src_ptr;
	int src_x;
	int eax, ecx;
	int src_fx, src_fdx;
	int shdpen;
	UINT8  z8, db0, p8, db1;
	UINT8  *ozbuf_ptr;
	UINT8  *szbuf_ptr;
	const pen_t *pal_base;
	const pen_t *shd_base;
	UINT32 *dst_ptr;

	// outter loop
	int src_fby, src_fdy, src_fbx;
	UINT8 *src_base;
	int dst_w, dst_h;

	// one-time
	int nozoom, granularity;
	int src_fw, src_fh;
	int dst_minx, dst_maxx, dst_miny, dst_maxy;
	int dst_skipx, dst_skipy, dst_x, dst_y, dst_lastx, dst_lasty;
	int src_pitch, dst_pitch;


	// cull illegal and transparent objects
	if (!scalex || !scaley) return;

	// find shadow pens and cull invisible shadows
	granularity = shdpen = gfx->color_granularity;
	shdpen--;

	if (zcode >= 0)
	{
		if (drawmode == 5) { drawmode = 4; shdpen = 1; }
	}
	else
		if (drawmode >= 4) return;

	// alpha blend necessary?
	if (drawmode & 2)
	{
		if (alpha <= 0) return;
		if (alpha >= 255) drawmode &= ~2;
	}

	// fill internal data structure with default values
	ozbuf_ptr  = gx_objzbuf;
	szbuf_ptr  = gx_shdzbuf;

	src_pitch = 16;
	src_fw    = 16;
	src_fh    = 16;
	src_base  = gfx->gfxdata + (code % gfx->total_elements) * gfx->char_modulo;

	pal_base  = Machine->remapped_colortable + gfx->color_base + (color % gfx->total_colors) * granularity;
	shd_base  = Machine->shadow_table;

	dst_ptr   = bitmap->base;
	dst_pitch = bitmap->rowpixels;
	dst_minx  = cliprect->min_x;
	dst_maxx  = cliprect->max_x;
	dst_miny  = cliprect->min_y;
	dst_maxy  = cliprect->max_y;
	dst_x     = sx;
	dst_y     = sy;

	// cull off-screen objects
	if (dst_x > dst_maxx || dst_y > dst_maxy) return;
	if ((nozoom = (scalex == 0x10000 && scaley == 0x10000)))
	{
		dst_h = dst_w = 16;
		src_fdy = src_fdx = 1;
	}
	else
	{
		dst_w = ((scalex<<4)+0x8000)>>16;
		dst_h = ((scaley<<4)+0x8000)>>16;
		if (!dst_w || !dst_h) return;

		src_fw <<= FP;
		src_fh <<= FP;
		src_fdx = src_fw / dst_w;
		src_fdy = src_fh / dst_h;
	}
	dst_lastx = dst_x + dst_w - 1;
	if (dst_lastx < dst_minx) return;
	dst_lasty = dst_y + dst_h - 1;
	if (dst_lasty < dst_miny) return;

	// clip destination
	dst_skipx = 0;
	eax = dst_minx;  if ((eax -= dst_x) > 0) { dst_skipx = eax;  dst_w -= eax;  dst_x = dst_minx; }
	eax = dst_lastx; if ((eax -= dst_maxx) > 0) dst_w -= eax;
	dst_skipy = 0;
	eax = dst_miny;  if ((eax -= dst_y) > 0) { dst_skipy = eax;  dst_h -= eax;  dst_y = dst_miny; }
	eax = dst_lasty; if ((eax -= dst_maxy) > 0) dst_h -= eax;

	// calculate zoom factors and clip source
	if (nozoom)
	{
		if (!flipx) src_fbx = 0; else { src_fbx = src_fw - 1; src_fdx = -src_fdx; }
		if (!flipy) src_fby = 0; else { src_fby = src_fh - 1; src_fdy = -src_fdy; src_pitch = -src_pitch; }
	}
	else
	{
		if (!flipx) src_fbx = FPENT; else { src_fbx = src_fw - FPENT - 1; src_fdx = -src_fdx; }
		if (!flipy) src_fby = FPENT; else { src_fby = src_fh - FPENT - 1; src_fdy = -src_fdy; }
	}
	src_fbx += dst_skipx * src_fdx;
	src_fby += dst_skipy * src_fdy;

	// adjust insertion points and pre-entry constants
	eax = (dst_y - dst_miny) * GX_ZBUFW + (dst_x - dst_minx) + dst_w;
	db0 = z8 = (UINT8)zcode;
	db1 = p8 = (UINT8)pri;
	ozbuf_ptr += eax;
	szbuf_ptr += eax << 1;
	dst_ptr += dst_y * dst_pitch + dst_x + dst_w;
	dst_w = -dst_w;

	if (!nozoom)
	{
		ecx = src_fby;   src_fby += src_fdy;
		ecx >>= FP;      src_fx = src_fbx;
		src_x = src_fbx; src_fx += src_fdx;
		ecx <<= 4;       src_ptr = src_base;
		src_x >>= FP;    src_ptr += ecx;
		ecx = dst_w;

		if (zcode < 0) // no shadow and z-buffering
		{
			do {
				do {
					eax = src_ptr[src_x];
					src_x = src_fx;
					src_fx += src_fdx;
					src_x >>= FP;
					if (!eax || eax >= shdpen) continue;
					dst_ptr [ecx] = pal_base[eax];
				}
				while (++ecx);

				ecx = src_fby;   src_fby += src_fdy;
				dst_ptr += dst_pitch;
				ecx >>= FP;      src_fx = src_fbx;
				src_x = src_fbx; src_fx += src_fdx;
				ecx <<= 4;       src_ptr = src_base;
				src_x >>= FP;    src_ptr += ecx;
				ecx = dst_w;
			}
			while (--dst_h);
		}
		else
		{
			switch (drawmode)
			{
				case 0:	// all pens solid
					do {
						do {
							eax = src_ptr[src_x];
							src_x = src_fx;
							src_fx += src_fdx;
							src_x >>= FP;
							if (!eax || ozbuf_ptr[ecx] < z8) continue;
							eax = pal_base[eax];
							ozbuf_ptr[ecx] = z8;
							dst_ptr [ecx] = eax;
						}
						while (++ecx);

						ecx = src_fby;   src_fby += src_fdy;
						ozbuf_ptr += GX_ZBUFW;
						dst_ptr += dst_pitch;
						ecx >>= FP;      src_fx = src_fbx;
						src_x = src_fbx; src_fx += src_fdx;
						ecx <<= 4;       src_ptr = src_base;
						src_x >>= FP;    src_ptr += ecx;
						ecx = dst_w;
					}
					while (--dst_h);
					break;

				case 1: // solid pens only
					do {
						do {
							eax = src_ptr[src_x];
							src_x = src_fx;
							src_fx += src_fdx;
							src_x >>= FP;
							if (!eax || eax >= shdpen || ozbuf_ptr[ecx] < z8) continue;
							eax = pal_base[eax];
							ozbuf_ptr[ecx] = z8;
							dst_ptr [ecx] = eax;
						}
						while (++ecx);

						ecx = src_fby;   src_fby += src_fdy;
						ozbuf_ptr += GX_ZBUFW;
						dst_ptr += dst_pitch;
						ecx >>= FP;      src_fx = src_fbx;
						src_x = src_fbx; src_fx += src_fdx;
						ecx <<= 4;       src_ptr = src_base;
						src_x >>= FP;    src_ptr += ecx;
						ecx = dst_w;
					}
					while (--dst_h);
					break;

				case 2: // all pens solid with alpha blending
					do {
						do {
							eax = src_ptr[src_x];
							src_x = src_fx;
							src_fx += src_fdx;
							src_x >>= FP;
							if (!eax || ozbuf_ptr[ecx] < z8) continue;
							ozbuf_ptr[ecx] = z8;

							dst_ptr[ecx] = alpha_blend32(pal_base[eax], dst_ptr[ecx]);
						}
						while (++ecx);

						ecx = src_fby;   src_fby += src_fdy;
						ozbuf_ptr += GX_ZBUFW;
						dst_ptr += dst_pitch;
						ecx >>= FP;      src_fx = src_fbx;
						src_x = src_fbx; src_fx += src_fdx;
						ecx <<= 4;       src_ptr = src_base;
						src_x >>= FP;    src_ptr += ecx;
						ecx = dst_w;
					}
					while (--dst_h);
					break;

				case 3: // solid pens only with alpha blending
					do {
						do {
							eax = src_ptr[src_x];
							src_x = src_fx;
							src_fx += src_fdx;
							src_x >>= FP;
							if (!eax || eax >= shdpen || ozbuf_ptr[ecx] < z8) continue;
							ozbuf_ptr[ecx] = z8;

							dst_ptr[ecx] = alpha_blend32(pal_base[eax], dst_ptr[ecx]);
						}
						while (++ecx);

						ecx = src_fby;   src_fby += src_fdy;
						ozbuf_ptr += GX_ZBUFW;
						dst_ptr += dst_pitch;
						ecx >>= FP;      src_fx = src_fbx;
						src_x = src_fbx; src_fx += src_fdx;
						ecx <<= 4;       src_ptr = src_base;
						src_x >>= FP;    src_ptr += ecx;
						ecx = dst_w;
					}
					while (--dst_h);
					break;

				case 4: // shadow pens only
					do {
						do {
							eax = src_ptr[src_x];
							src_x = src_fx;
							src_fx += src_fdx;
							src_x >>= FP;
							if (eax < shdpen || szbuf_ptr[ecx*2] < z8 || szbuf_ptr[ecx*2+1] <= p8) continue;
							eax = dst_ptr[ecx];
							szbuf_ptr[ecx*2] = z8;
							szbuf_ptr[ecx*2+1] = p8;
							eax = (eax>>9&0x7c00) | (eax>>6&0x03e0) | (eax>>3&0x001f);
							dst_ptr[ecx] = shd_base[eax];
						}
						while (++ecx);

						ecx = src_fby;   src_fby += src_fdy;
						szbuf_ptr += (GX_ZBUFW<<1);
						dst_ptr += dst_pitch;
						ecx >>= FP;      src_fx = src_fbx;
						src_x = src_fbx; src_fx += src_fdx;
						ecx <<= 4;       src_ptr = src_base;
						src_x >>= FP;    src_ptr += ecx;
						ecx = dst_w;
					}
					while (--dst_h);
					break;
			}	// switch (drawmode)
		}	// if (zcode < 0)
	}	// if (!nozoom)
	else
	{
		src_ptr = src_base + (src_fby<<4) + src_fbx;
		src_fdy = src_fdx * dst_w + src_pitch;
		ecx = dst_w;

		if (zcode < 0) // no shadow and z-buffering
		{
			do {
				do {
					eax = *src_ptr;
					src_ptr += src_fdx;
					if (!eax || eax >= shdpen) continue;
					dst_ptr[ecx] = pal_base[eax];
				}
				while (++ecx);

				src_ptr += src_fdy;
				dst_ptr += dst_pitch;
				ecx = dst_w;
			}
			while (--dst_h);
		}
		else
		{
			switch (drawmode)
			{
				case 0: // all pens solid
					do {
						do {
							eax = *src_ptr;
							src_ptr += src_fdx;
							if (!eax || ozbuf_ptr[ecx] < z8) continue;
							eax = pal_base[eax];
							ozbuf_ptr[ecx] = z8;
							dst_ptr[ecx] = eax;
						}
						while (++ecx);

						src_ptr += src_fdy;
						ozbuf_ptr += GX_ZBUFW;
						dst_ptr += dst_pitch;
						ecx = dst_w;
					}
					while (--dst_h);
					break;

				case 1:  // solid pens only
					do {
						do {
							eax = *src_ptr;
							src_ptr += src_fdx;
							if (!eax || eax >= shdpen || ozbuf_ptr[ecx] < z8) continue;
							eax = pal_base[eax];
							ozbuf_ptr[ecx] = z8;
							dst_ptr[ecx] = eax;
						}
						while (++ecx);

						src_ptr += src_fdy;
						ozbuf_ptr += GX_ZBUFW;
						dst_ptr += dst_pitch;
						ecx = dst_w;
					}
					while (--dst_h);
					break;

				case 2: // all pens solid with alpha blending
					do {
						do {
							eax = *src_ptr;
							src_ptr += src_fdx;
							if (!eax || ozbuf_ptr[ecx] < z8) continue;
							ozbuf_ptr[ecx] = z8;

							dst_ptr[ecx] = alpha_blend32(pal_base[eax], dst_ptr[ecx]);
						}
						while (++ecx);

						src_ptr += src_fdy;
						ozbuf_ptr += GX_ZBUFW;
						dst_ptr += dst_pitch;
						ecx = dst_w;
					}
					while (--dst_h);
					break;

				case 3: // solid pens only with alpha blending
					do {
						do {
							eax = *src_ptr;
							src_ptr += src_fdx;
							if (!eax || eax >= shdpen || ozbuf_ptr[ecx] < z8) continue;
							ozbuf_ptr[ecx] = z8;

							dst_ptr[ecx] = alpha_blend32(pal_base[eax], dst_ptr[ecx]);
						}
						while (++ecx);

						src_ptr += src_fdy;
						ozbuf_ptr += GX_ZBUFW;
						dst_ptr += dst_pitch;
						ecx = dst_w;
					}
					while (--dst_h);
					break;

				case 4: // shadow pens only
					do {
						do {
							eax = *src_ptr;
							src_ptr += src_fdx;
							if (eax < shdpen || szbuf_ptr[ecx*2] < z8 || szbuf_ptr[ecx*2+1] <= p8) continue;
							eax = dst_ptr[ecx];
							szbuf_ptr[ecx*2] = z8;
							szbuf_ptr[ecx*2+1] = p8;
							eax = (eax>>9&0x7c00) | (eax>>6&0x03e0) | (eax>>3&0x001f);
							dst_ptr[ecx] = shd_base[eax];
						}
						while (++ecx);

						src_ptr += src_fdy;
						szbuf_ptr += (GX_ZBUFW<<1);
						dst_ptr += dst_pitch;
						ecx = dst_w;
					}
					while (--dst_h);
					break;
			}
		}
	}
#undef FP
#undef FPONE
#undef FPHALF
#undef FPENT
}



/***************************************************************************/
/*                                                                         */
/*                 1st-Tier GX/MW Variables and Functions                  */
/*                                                                         */
/***************************************************************************/

// global system ports access
UINT8  konamigx_wrport1_0, konamigx_wrport1_1;
UINT16 konamigx_wrport2;

// frequently used registers
static int K053246_objset1;
static int K053247_vrcbk[4];
static int K053247_coreg, K053247_coregshift, K053247_opset;
static int opri, oinprion;
static int vcblk[6], ocblk;
static int vinmix, vmixon, osinmix, osmixon;


static void konamigx_precache_registers(void)
{
	// (see sprite color coding scheme on p.46 & 47)
	static int coregmasks[5] = {0xf,0xe,0xc,0x8,0x0};
	static int coregshifts[5]= {4,5,6,7,8};
	int i;

	K053246_objset1 = K053246_read_register(5);

	i = K053247_read_register(0x8/2);
	K053247_vrcbk[0] = (i & 0x000f) << 14;
	K053247_vrcbk[1] = (i & 0x0f00) << 6;
	i = K053247_read_register(0xa/2);
	K053247_vrcbk[2] = (i & 0x000f) << 14;
	K053247_vrcbk[3] = (i & 0x0f00) << 6;

	// COREG == OBJSET2+1C == bit8-11 of OPSET ??? (see p.50 last table, needs p.49 to confirm)
	K053247_opset = K053247_read_register(0xc/2);

	i = K053247_opset & 7; if (i > 4) i = 4;

	K053247_coreg = K053247_read_register(0xc/2)>>8 & 0xf;
	K053247_coreg =(K053247_coreg & coregmasks[i]) << 12;

	K053247_coregshift = coregshifts[i];

	opri     = K055555_read_register(K55_PRIINP_8);
	oinprion = K055555_read_register(K55_OINPRI_ON);
	vcblk[0] = K055555_read_register(K55_PALBASE_A);
	vcblk[1] = K055555_read_register(K55_PALBASE_B);
	vcblk[2] = K055555_read_register(K55_PALBASE_C);
	vcblk[3] = K055555_read_register(K55_PALBASE_D);
	vcblk[4] = K055555_read_register(K55_PALBASE_SUB1);
	vcblk[5] = K055555_read_register(K55_PALBASE_SUB2);
	ocblk    = K055555_read_register(K55_PALBASE_OBJ);
	vinmix   = K055555_read_register(K55_BLEND_ENABLES);
	vmixon   = K055555_read_register(K55_VINMIX_ON);
	osinmix  = K055555_read_register(K55_OSBLEND_ENABLES);
	osmixon  = K055555_read_register(K55_OSBLEND_ON);
}

INLINE int K053247GX_combine_c18(int attrib) // (see p.46)
{
	int c18;

	c18 = (attrib & 0xff)<<K053247_coregshift | K053247_coreg;

	if (konamigx_wrport2 & 4) c18 &= 0x3fff; else
	if (!(konamigx_wrport2 & 8)) c18 = (c18 & 0x3fff) | (attrib<<6 & 0xc000);

	return(c18);
}

INLINE int K055555GX_decode_objcolor(int c18) // (see p.59 7.2.2)
{
	int ocb, opon;

	opon  = oinprion<<8 | 0xff;
	ocb   = (ocblk & 7) << 10;
	c18  &= opon;
	ocb  &=~opon;

	return((ocb | c18) >> K053247_coregshift);
}

INLINE int K055555GX_decode_inpri(int c18) // (see p.59 7.2.2)
{
	int op = opri;

	c18 >>= 8;
	op   &= oinprion;
	c18  &=~oinprion;

	return(c18 | op);
}

void konamigx_type2_sprite_callback(int *code, int *color, int *priority)
{
	int num = *code;
	int c18 = *color;

	*code = K053247_vrcbk[num>>14] | (num & 0x3fff);
	c18 = K053247GX_combine_c18(c18);
	*color = K055555GX_decode_objcolor(c18);
	*priority = K055555GX_decode_inpri(c18);
}

void konamigx_dragoonj_sprite_callback(int *code, int *color, int *priority)
{
	int num, op, pri, c18;

	num = *code;
	*code = K053247_vrcbk[num>>14] | (num & 0x3fff);

	c18  = pri = *color;
	op   = opri;
	pri  = (pri & 0x200) ? 4 : pri>>4 & 0xf;
	op  &= oinprion;
	pri &=~oinprion;
	*priority = pri | op;

	c18 = K053247GX_combine_c18(c18);
	*color = K055555GX_decode_objcolor(c18);
}

void konamigx_salmndr2_sprite_callback(int *code, int *color, int *priority)
{
	int num, op, pri, c18;

	num = *code;
	*code = K053247_vrcbk[num>>14] | (num & 0x3fff);

	c18  = pri = *color;
	op   = opri;
	pri  = pri>>4 & 0x3f;
	op  &= oinprion;
	pri &=~oinprion;
	*priority = pri | op;

	c18 = K053247GX_combine_c18(c18);
	*color = K055555GX_decode_objcolor(c18);
}

void konamigx_le2_sprite_callback(int *code, int *color, int *priority)
{
	int num, op, pri;

	num = *code;
	*code = K053247_vrcbk[num>>14] | (num & 0x3fff);

	pri = *color;
	*color &= 0x1f;

	op   = opri;
	pri &= 0xf0;
	op  &= oinprion;
	pri &=~oinprion;
	*priority = pri | op;
}

int K055555GX_decode_vmixcolor(int layer, int *color) // (see p.62 7.2.6 and p.27 3.3)
{
	int vcb, shift, pal, vmx, von, pl45, emx;

	vcb    =  vcblk[layer]<<6;
	shift  =  layer<<1;
	pal    =  *color;
	vmx    =  vinmix>>shift & 3;
	von    =  vmixon>>shift & 3;
	emx    =  pl45 = pal>>4 & 3;
	pal   &=  0xf;
	pl45  &=  von;
	vmx   &=  von;
	pl45 <<=  4;
	emx   &= ~von;
	pal   |=  pl45;
	emx   |=  vmx;
	pal   |=  vcb;

	if (von == 3) emx = -1; // invalidate external mix code if all bits are from internal
	*color =  pal;

	return(emx);
}

int K055555GX_decode_osmixcolor(int layer, int *color) // (see p.63, p.49-50 and p.27 3.3)
{
	int scb, shift, pal, osmx, oson, pl45, emx;

	shift  =  layer<<1;
	pal    =  *color;
	osmx   =  osinmix>>shift & 3;
	oson   =  osmixon>>shift & 3;

	if (layer)
	{
		// layer 1-3 are external tile layers
		scb    =  vcblk[layer+3]<<6;
		emx    =  pl45 = pal>>4 & 3;
		pal   &=  0xf;
		pl45  &=  oson;
		osmx  &=  oson;
		pl45 <<=  4;
		emx   &= ~oson;
		pal   |=  pl45;
		emx   |=  osmx;
		pal   |=  scb;

		if (oson == 3) emx = -1; // invalidate external mix code if all bits are from internal
		*color =  pal;
	}
	else
	{
		// layer 0 is the sprite layer with different attributes decode; detail on p.49 (missing)
		emx   = 0; // K053247_read_register(??)>>? & 3;
		osmx &= oson;
		emx  &=~oson;
		emx  |= osmx;
	}

	return(emx);
}

static void gx_wipezbuf(int noshadow)
{
	UINT8  *zptr;
	int w, h;
	register int ecx;

	w = Machine->screen[0].visarea.max_x - Machine->screen[0].visarea.min_x + 1;
	h = Machine->screen[0].visarea.max_y - Machine->screen[0].visarea.min_y + 1;

	zptr = gx_objzbuf;
	ecx = h;
	do { memset(zptr, -1, w); zptr += GX_ZBUFW; } while (--ecx);

	if (!noshadow)
	{
		zptr = gx_shdzbuf;
		w <<= 1;
		ecx = h;
		do { memset(zptr, -1, w); zptr += (GX_ZBUFW<<1); } while (--ecx);
	}
}

/*
 * Sprite Format
 * ------------------
 *
 * Word | Bit(s)           | Use
 * -----+-fedcba9876543210-+----------------
 *   0  | x--------------- | active (show this sprite)
 *   0  | -x-------------- | maintain aspect ratio (when set, zoom y acts on both axis)
 *   0  | --x------------- | flip y
 *   0  | ---x------------ | flip x
 *   0  | ----xxxx-------- | sprite size (see below)
 *   0  | --------xxxxxxxx | zcode
 *   1  | xxxxxxxxxxxxxxxx | sprite code
 *   2  | ------xxxxxxxxxx | y position
 *   3  | ------xxxxxxxxxx | x position
 *   4  | xxxxxxxxxxxxxxxx | zoom y (0x40 = normal, <0x40 = enlarge, >0x40 = reduce)
 *   5  | xxxxxxxxxxxxxxxx | zoom x (0x40 = normal, <0x40 = enlarge, >0x40 = reduce)
 *   6  | x--------------- | mirror y (top half is drawn as mirror image of the bottom)
 *   6  | -x-------------- | mirror x (right half is drawn as mirror image of the left)
 *   6  | --xx------------ | reserved (sprites with these two bits set don't seem to be graphics data at all)
 *   6  | ----xx---------- | shadow code: 0=off, 0x400=preset1, 0x800=preset2, 0xc00=preset3
 *   6  | ------xx-------- | effect code: flicker, upper palette, full shadow...etc. (game dependent)
 *   6  | --------xxxxxxxx | "color", but depends on external connections (implies priority)
 *   7  | xxxxxxxxxxxxxxxx | game dependent
 *
 * shadow enables transparent shadows. Note that it applies to the last sprite pen ONLY.
 * The rest of the sprite remains normal.
 */
#define GX_MAX_SPRITES 512
#define GX_MAX_LAYERS  6
#define GX_MAX_OBJECTS (GX_MAX_SPRITES + GX_MAX_LAYERS)

static struct GX_OBJ { int order, offs, code, color; } *gx_objpool;
static UINT16 *gx_spriteram;
static int gx_objdma, gx_primode;

// mirrored K053247 and K054338 settings
static UINT16 *K053247_ram;
static gfx_element *K053247_gfx;
static void (*K053247_callback)(int *code,int *color,int *priority);
static int K053247_dx, K053247_dy;

static int *K054338_shdRGB;


void K053247GP_set_SpriteOffset(int offsx, int offsy)
{
	K053247_dx = offsx;
	K053247_dy = offsy;
}

void konamigx_mixer_init(int objdma)
{
	gx_objdma = 0;
	gx_primode = 0;

	gx_objzbuf = (UINT8 *)priority_bitmap->base;
	gx_shdzbuf = auto_malloc(GX_ZBUFSIZE);
	gx_objpool = auto_malloc(sizeof(struct GX_OBJ) * (GX_MAX_OBJECTS));

	K053247_export_config(&K053247_ram, &K053247_gfx, &K053247_callback, &K053247_dx, &K053247_dy);
	K054338_export_config(&K054338_shdRGB);

	if (objdma)
	{
		gx_spriteram = auto_malloc(0x1000);
		gx_objdma = 1;
	}
	else
		gx_spriteram = K053247_ram;

	palette_set_shadow_dRGB32(Machine, 3,-80,-80,-80, 0);
	K054338_invert_alpha(1);
}

void konamigx_mixer_primode(int mode)
{
	gx_primode = mode;
}

void konamigx_objdma(void)
{
	if (gx_objdma && gx_spriteram && K053247_ram) memcpy(gx_spriteram, K053247_ram, 0x1000);
}

void konamigx_mixer(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect,
					tilemap *sub1, int sub1flags,
					tilemap *sub2, int sub2flags,
					int mixerflags)
{
	static int xoffset[8] = { 0, 1, 4, 5, 16, 17, 20, 21 };
	static int yoffset[8] = { 0, 2, 8, 10, 32, 34, 40, 42 };
	static int parity = 0;

	int objbuf[GX_MAX_OBJECTS];
	int shadowon[3], shdpri[3], layerid[6], layerpri[6];

	struct GX_OBJ *objpool, *objptr;
	int wrapsize, xwraplim, ywraplim, cltc_shdpri, prflp, disp;
	int xa,ya,ox,oy,zw,zh,flipx,flipy,mirrorx,mirrory,zoomx,zoomy,scalex,scaley,nozoom;
	int screenwidth, flipscreenx, flipscreeny, offx, offy;
	int nobj, i, j, k, l, temp, temp1, temp2, temp3, temp4, count;
	int order, offs, code, color, zcode, pri = 0, spri, spri_min, shdprisel, shadow, alpha, drawmode;


	// abort if object database failed to initialize
	if (!(objpool = gx_objpool)) return;

	// clear screen with backcolor and update flicker pulse
	K054338_fill_backcolor(machine, bitmap, konamigx_wrport1_0 & 0x20);
	parity ^= 1;

	// abort if video has been disabled
	if (!(disp = K055555_read_register(K55_INPUT_ENABLES))) return;
	cltc_shdpri = K054338_read_register(K338_REG_CONTROL);
	if (!(cltc_shdpri & K338_CTL_KILL)) return;

	// demote shadows by one layer when this bit is set??? (see p.73 8.6)
	cltc_shdpri &= K338_CTL_SHDPRI;

	// wipe z-buffer
	if (mixerflags & GXMIX_NOZBUF)
		mixerflags |= GXMIX_NOSHADOW;
	else
		gx_wipezbuf(mixerflags & GXMIX_NOSHADOW);

	// cache global parameters
	konamigx_precache_registers();

	// init OBJSET1 parameters (see p.47 6.2)
	flipscreenx = K053246_objset1 & 1;
	flipscreeny = K053246_objset1 & 2;

	// get "display window" offsets
	offx = (K053246_read_register(0)<<8 | K053246_read_register(1)) & 0x3ff;
	offy = (K053246_read_register(2)<<8 | K053246_read_register(3)) & 0x3ff;

	// init OBJSET2 and mixer parameters (see p.51 and chapter 7)
	layerid[0] = 0; layerid[1] = 1; layerid[2] = 2; layerid[3] = 3; layerid[4] = 4; layerid[5] = 5;

	if (K053247_opset & 0x40)
	{
		wrapsize = 512;
		xwraplim = 512 - 64;
		ywraplim = 512 - 128;
	}
	else
	{
		wrapsize  = 1024;
		xwraplim  = 1024 - 384;
		ywraplim  = 1024 - 512;
	}

	// invert layer priority when this flag is set (not used by any GX game?)
	prflp = K055555_read_register(K55_CONTROL) & K55_CTL_FLIPPRI;

	layerpri[0] = K055555_read_register(K55_PRIINP_0);
	layerpri[1] = K055555_read_register(K55_PRIINP_3);
	layerpri[3] = K055555_read_register(K55_PRIINP_7);
	layerpri[4] = K055555_read_register(K55_PRIINP_9);
	layerpri[5] = K055555_read_register(K55_PRIINP_10);

	if (gx_primode == -1)
	{
		// Lethal Enforcer hack (requires pixel color comparison)
		layerpri[2] = K055555_read_register(K55_PRIINP_3) + 0x20;
		shdprisel = 0x3f;
	}
	else
	{
		layerpri[2] = K055555_read_register(K55_PRIINP_6);
		shdprisel = K055555_read_register(K55_SHD_PRI_SEL);
	}

	// SHDPRISEL filters shadows by different priority comparison methods (UNIMPLEMENTED, see detail on p.66)
	if (!(shdprisel & 0x03)) shadowon[0] = 0;
	if (!(shdprisel & 0x0c)) shadowon[1] = 0;
	if (!(shdprisel & 0x30)) shadowon[2] = 0;

	shdpri[0]   = K055555_read_register(K55_SHAD1_PRI);
	shdpri[1]   = K055555_read_register(K55_SHAD2_PRI);
	shdpri[2]   = K055555_read_register(K55_SHAD3_PRI);

	spri_min = 0;
	shadowon[2] = shadowon[1] = shadowon[0] = 0;

	if (!(mixerflags & GXMIX_NOSHADOW))
	{
		// only enable shadows beyond a +/-7 RGB threshold
		for (j=0,i=0; i<3; j+=3,i++)
		{
			k = K054338_shdRGB[j  ]; if (k < -7 || k > 7) { shadowon[i] = 1; continue; }
			k = K054338_shdRGB[j+1]; if (k < -7 || k > 7) { shadowon[i] = 1; continue; }
			k = K054338_shdRGB[j+2]; if (k < -7 || k > 7) { shadowon[i] = 1; }
		}

		// SHDON specifies layers on which shadows can be projected (see detail on p.65 7.2.8)
		temp = K055555_read_register(K55_SHD_ON);
		for (i=0; i<4; i++) if (!(temp>>i & 1) && spri_min < layerpri[i]) spri_min = layerpri[i]; // HACK

		// update shadows status
		K054338_update_all_shadows(machine);
	}

	// pre-sort layers
	for (j=0; j<5; j++)
	{
		temp1 = layerpri[j];
		for (i=j+1; i<6; i++)
		{
			temp2 = layerpri[i];
			if ((UINT32)temp1 <= (UINT32)temp2)
			{
				layerpri[i] = temp1; layerpri[j] = temp1 = temp2;
				temp2 = layerid[i]; layerid[i] = layerid[j]; layerid[j] = temp2;
			}
		}
	}

	// build object database and create indices
	objptr = objpool;
	nobj = 0;

	for (i=5; i>=0; i--)
	{
		code = layerid[i];
		switch (code)
		{
			/*
                Background layers are represented by negative offset values as follow:

                0+ : normal sprites
                -1 : tile layer A - D
                -2 : K053936 ROZ+ layer 1
                -3 : K053936 ROZ+ layer 2
                -4 : K053250 LVC layer 1
                -5 : K053250 LVC layer 2
            */
			case 4 :
				offs = -128;
				if (sub1flags & 0xf) { if (sub1flags & GXSUB_K053250) offs = -4; else if (sub1) offs = -2; }
			break;
			case 5 :
				offs = -128;
				if (sub2flags & 0xf) { if (sub2flags & GXSUB_K053250) offs = -5; else if (sub2) offs = -3; }
			break;
			default: offs = -1;
		}

		if (offs != -128)
		{
			objptr->order = layerpri[i]<<24;
			objptr->code  = code;
			objptr->offs = offs;
			objptr++;

			objbuf[nobj] = nobj;
			nobj++;
		}
	}

	i = j = 0xff;

	for (offs=0; offs<0x800; offs+=8)
	{
		if (!(gx_spriteram[offs] & 0x8000)) continue;

		zcode = gx_spriteram[offs] & 0xff;

		// invert z-order when opset_pri is set (see p.51 OPSET PRI)
		if (K053247_opset & 0x10) zcode = 0xff - zcode;

		code  = gx_spriteram[offs+1];
		color = k = gx_spriteram[offs+6];
		l     = gx_spriteram[offs+7];

		(*K053247_callback)(&code, &color, &pri);

		/*
            shadow = shadow code
            spri   = shadow priority
            temp1  = add solid object
            temp2  = solid pens draw mode
            temp3  = add shadow object
            temp4  = shadow pens draw mode
        */
		temp4 = temp3 = temp2 = temp1 = spri = shadow = 0;

		if (color & K055555_FULLSHADOW)
		{
			shadow = 3; // use default intensity and color
			spri = pri; // retain host priority
			temp3 = 1; // add shadow
			temp4 = 5; // draw full shadow
		}
		else
		{
			if ((shadow = k>>10 & 3)) // object has shadow?
			{
				if (shadow != 1 || K053246_objset1 & 0x20)
				{
					shadow--;
					temp1 = 1; // add solid
					temp2 = 1; // draw partial solid
					if (shadowon[shadow])
					{
						temp3 = 1; // add shadow
						temp4 = 4; // draw partial shadow
					}
				}
				else
				{
					// drop the entire sprite to shadow if its shadow code is 1 and SD0EN is off (see p.48)
					shadow = 0;
					if (!shadowon[0]) continue;
					temp3 = 1; // add shadow
					temp4 = 5; // draw full shadow
				}
			}
			else
			{
				temp1 = 1; // add solid
				temp2 = 0; // draw full solid
			}

			if (temp1)
			{
				// tag sprite for alpha blending
				if (color>>K055555_MIXSHIFT & 3) temp2 |= 2;
			}

			if (temp3)
			{
				// determine shadow priority
				spri = (K053247_opset & 0x20) ? pri : shdpri[shadow]; // (see p.51 OPSET SDSEL)
			}
		}

		switch (gx_primode & 0xf)
		{
			// Dadandarn zcode suppression
			case  1:
				zcode = 0;
			break;

			// Daisukiss bad shadow filter
			case  4:
				if (k & 0x3000 || k == 0x0800) continue;

			// Tokkae shadow masking (INACCURATE)
			case  5:
				if (spri < spri_min) spri = spri_min;
			break;
		}

		/*
            default sort order:
            fedcba9876543210fedcba9876543210
            xxxxxxxx------------------------ (priority)
            --------xxxxxxxx---------------- (zcode)
            ----------------xxxxxxxx-------- (offset)
            ------------------------xxxx---- (shadow mode)
            ----------------------------xxxx (shadow code)
        */
		if (temp1)
		{
			// add objects with solid or alpha pens
			order = pri<<24 | zcode<<16 | offs<<(8-3) | temp2<<4;
			objptr->order = order;
			objptr->offs  = offs;
			objptr->code  = code;
			objptr->color = color;
			objptr++;

			objbuf[nobj] = nobj;
			nobj++;
		}

		if (temp3 && !(color & K055555_SKIPSHADOW) && !(mixerflags & GXMIX_NOSHADOW))
		{
			// add objects with shadows if enabled
			order = spri<<24 | zcode<<16 | offs<<(8-3) | temp4<<4 | shadow;
			objptr->order = order;
			objptr->offs  = offs;
			objptr->code  = code;
			objptr->color = color;
			objptr++;

			objbuf[nobj] = nobj;
			nobj++;
		}
	}

	// sort objects in decending order (SLOW)
	k = nobj;
	l = nobj - 1;

	for (j=0; j<l; j++)
	{
		temp1 = objbuf[j];
		temp2 = objpool[temp1].order;
		for (i=j+1; i<k; i++)
		{
			temp3 = objbuf[i];
			temp4 = objpool[temp3].order;
			if ((UINT32)temp2 <= (UINT32)temp4) { temp2 = temp4; objbuf[i] = temp1; objbuf[j] = temp1 = temp3; }
		}
	}

	// traverse draw list
	screenwidth = Machine->screen[0].width;

	for (count=0; count<nobj; count++)
	{
		objptr = objpool + objbuf[count];
		order  = objptr->order;
		offs   = objptr->offs;
		code   = objptr->code;
		color  = objptr->color;

		if (offs >= 0)
		{
			if (!(disp & K55_INP_OBJ)) continue;
		}
		else
		{
			i = code<<1;
			j = mixerflags>>i & 3;
			k = 0;

			switch (offs)
			{
				case -1:
				if (disp & (1<<code))
				{
					if (j == GXMIX_BLEND_NONE)  { temp1 = 0xff; temp2 = temp3 = 0; } else
					if (j == GXMIX_BLEND_FORCE) { temp1 = 0x00; temp2 = mixerflags>>(i+16); temp3 = 3; }
					else
					{
						temp1 = vinmix;
						temp2 = vinmix>>i & 3;
						temp3 = vmixon>>i & 3;
					}

					/* blend layer only when:
                        1) vinmix != 0xff
                        2) its internal mix code is set
                        3) all mix code bits are internal(overriden until tile blending has been implemented)
                        4) 0 > alpha < 255;
                    */
					if (temp1!=0xff && temp2 /*&& temp3==3*/)
					{
						temp4 = K054338_set_alpha_level(temp2);

						if (temp4 <= 0) continue;
						if (temp4 < 255) k = TILEMAP_DRAW_ALPHA;
					}

					if (mixerflags & 1<<(code+12)) k |= TILE_LINE_DISABLED;

					K056832_tilemap_draw(machine, bitmap, cliprect, code, k, 0);
				}
				continue;
				case -2:
				case -4:
				if (disp & K55_INP_SUB1)
				{
					if (j == GXMIX_BLEND_NONE)  { temp1 = 0xff; temp2 = temp3 = 0; } else
					if (j == GXMIX_BLEND_FORCE) { temp1 = 0x00; temp2 = mixerflags>>24; temp3 = 3; }
					else
					{
						temp1 = osinmix;
						temp2 = osinmix>>2 & 3;
                        temp3 = osmixon>>2 & 3;
					}

					if (temp1!=0xff && temp2 /*&& temp3==3*/)
					{
						temp4 = K054338_set_alpha_level(temp2);

						if (temp4 <= 0) continue;
						if (temp4 < 255) k = (j == GXMIX_BLEND_FAST) ? ~parity : 1;
					}

					l = sub1flags & 0xf;

					if (offs == -2)
						K053936GP_0_zoom_draw(bitmap, cliprect, sub1, l, k);
					else
						K053250_draw(machine, bitmap, cliprect, 0, vcblk[4]<<l, 0, 0);
				}
				continue;
				case -3:
				case -5:
				if (disp & K55_INP_SUB2)
				{
					if (j == GXMIX_BLEND_NONE)  { temp1 = 0xff; temp2 = temp3 = 0; } else
					if (j == GXMIX_BLEND_FORCE) { temp1 = 0x00; temp2 = mixerflags>>26; temp3 = 3; }
					else
					{
						temp1 = osinmix;
						temp2 = osinmix>>4 & 3;
                        temp3 = osmixon>>4 & 3;
					}

					if (temp1!=0xff && temp2 /*&& temp3==3*/)
					{
						temp4 = K054338_set_alpha_level(temp2);

						if (temp4 <= 0) continue;
						if (temp4 < 255) k = (j == GXMIX_BLEND_FAST) ? ~parity : 1;
					}

					l = sub2flags & 0xf;

					if (offs == -3)
						K053936GP_1_zoom_draw(bitmap, cliprect, sub2, l, k);
					else
						K053250_draw(machine, bitmap, cliprect, 1, vcblk[5]<<l, 0, 0);
				}
				continue;
			}
			continue;
		}

		drawmode = order>>4 & 0xf;

		alpha = 255;
		if (drawmode & 2)
		{
			if ((alpha = color>>K055555_MIXSHIFT & 3)) alpha = K054338_set_alpha_level(alpha);
			if (alpha <= 0) continue;
		}
		color &= K055555_COLORMASK;

		if (drawmode >= 4) palette_set_shadow_mode(Machine, order & 0x0f);

		if (!(mixerflags & GXMIX_NOZBUF))
		{
			zcode = order>>16 & 0xff;
			pri = order>>24 & 0xff;
		}
		else
			zcode = -1; // negative zcode values turn off z-buffering

		xa = ya = 0;
		if (code & 0x01) xa += 1;
		if (code & 0x02) ya += 1;
		if (code & 0x04) xa += 2;
		if (code & 0x08) ya += 2;
		if (code & 0x10) xa += 4;
		if (code & 0x20) ya += 4;
		code &= ~0x3f;

		temp4 = gx_spriteram[offs];

		// mask off the upper 6 bits of coordinate and zoom registers
		oy = gx_spriteram[offs+2] & 0x3ff;
		ox = gx_spriteram[offs+3] & 0x3ff;

		scaley = zoomy = gx_spriteram[offs+4] & 0x3ff;
		if (zoomy) zoomy = (0x400000+(zoomy>>1)) / zoomy;
		else zoomy = 0x800000;
		if (!(temp4 & 0x4000))
		{
			scalex = zoomx = gx_spriteram[offs+5] & 0x3ff;
			if (zoomx) zoomx = (0x400000+(zoomx>>1)) / zoomx;
			else zoomx = 0x800000;
		}
		else { zoomx = zoomy; scalex = scaley; }

		nozoom = (scalex == 0x40 && scaley == 0x40);

		flipx = temp4 & 0x1000;
		flipy = temp4 & 0x2000;

		temp = gx_spriteram[offs+6];
		mirrorx = temp & 0x4000;
		if (mirrorx) flipx = 0; // only applies to x mirror, proven
		mirrory = temp & 0x8000;

		// for Escape Kids (GX975)
		if ( K053246_objset1 & 8 ) // Check only "Bit #3 is '1'?"
		{
			zoomx = zoomx>>1; // Fix sprite width to HALF size
			ox = (ox>>1) + 1; // Fix sprite draw position

			if (flipscreenx) ox += screenwidth;
		}

		if (flipscreenx) { ox = -ox; if (!mirrorx) flipx = !flipx; }
		if (flipscreeny) { oy = -oy; if (!mirrory) flipy = !flipy; }

		// apply wrapping and global offsets
		temp = wrapsize-1;
		ox = ( ox - offx) & temp;
		oy = (-oy - offy) & temp;
		if (ox >= xwraplim) ox -= wrapsize;
		if (oy >= ywraplim) oy -= wrapsize;
		ox += K053247_dx;
		oy += K053247_dy;


		temp = temp4>>8 & 0x0f;
		k = 1 << (temp & 3);
		l = 1 << (temp>>2 & 3);

		ox -= (zoomx * k) >> 13;
		oy -= (zoomy * l) >> 13;

		// substitutes: i=x, j=y, k=w, l=h, temp=code, temp1=fx, temp2=fy, temp3=sx, temp4=sy;
		for (j=0; j<l; j++)
		{
			temp4 = oy + ((zoomy * j + (1<<11)) >> 12);
			zh = (oy + ((zoomy * (j+1) + (1<<11)) >> 12)) - temp4;

			for (i=0; i<k; i++)
			{
				temp3 = ox + ((zoomx * i + (1<<11)) >> 12);
				zw = (ox + ((zoomx * (i+1) + (1<<11)) >> 12)) - temp3;
				temp = code;

				if (mirrorx)
				{
					if ((!flipx)^((i<<1)<k))
					{
						/* mirror left/right */
						temp += xoffset[(k-1-i+xa)&7];
						temp1 = 1;
					}
					else
					{
						temp += xoffset[(i+xa)&7];
						temp1 = 0;
					}
				}
				else
				{
					if (flipx) temp += xoffset[(k-1-i+xa)&7];
					else temp += xoffset[(i+xa)&7];
					temp1 = flipx;
				}

				if (mirrory)
				{
					if ((!flipy)^((j<<1)>=l))
					{
						/* mirror top/bottom */
						temp += yoffset[(l-1-j+ya)&7];
						temp2 = 1;
					}
					else
					{
						temp += yoffset[(j+ya)&7];
						temp2 = 0;
					}
				}
				else
				{
					if (flipy) temp += yoffset[(l-1-j+ya)&7];
					else temp += yoffset[(j+ya)&7];
					temp2 = flipy;
				}

				if (nozoom) { scaley = scalex = 0x10000; } else { scalex = zw << 12; scaley = zh << 12; };

				zdrawgfxzoom32GP(bitmap, K053247_gfx, cliprect,
						temp,
						color,
						temp1,temp2,
						temp3,temp4,
						scalex, scaley, alpha, drawmode, zcode, pri);
			}
		}
	}
}



/***************************************************************************/
/*                                                                         */
/*                           GX/MW Protections                             */
/*                                                                         */
/***************************************************************************/

// K055550/K053990 protection chips, perform simple memset() and other game logic operations
static UINT16 prot_data[0x20];

READ16_HANDLER( K055550_word_r )
{
	return(prot_data[offset]);
}

WRITE16_HANDLER( K055550_word_w )
{
	UINT32 adr, bsize, count, i, lim;
	int src, tgt, srcend, tgtend, skip, cx1, sx1, wx1, cy1, sy1, wy1, cz1, sz1, wz1, c2, s2, w2;
	int dx, dy, angle;

	COMBINE_DATA(prot_data+offset);

	if (offset == 0 && ACCESSING_MSB)
	{
		data >>= 8;
		switch (data)
		{
			case 0x97: // memset() (Dadandarn at 0x639dc)
			case 0x9f: // memset() (Violent Storm at 0x989c)
				adr   = (prot_data[7] << 16) | prot_data[8];
				bsize = (prot_data[10] << 16) | prot_data[11];
				count = (prot_data[0] & 0xff) + 1;

				lim = adr+bsize*count;
				for(i=adr; i<lim; i+=2)
					program_write_word(i, prot_data[0x1a/2]);
			break;

			// WARNING: The following cases are speculation based with questionable accuracy!(AAT)

			case 0x87: // unknown memory write (Violent Storm at 0x00b6ea)
				// Violent Storm writes the following data to the 55550 in mode 0x87.
				// All values are hardcoded and the write happens each frame during
				// gameplay. It refers to a 32x8-word list at 0x210e00 and seems to
				// be tied with another 13x128-byte table at 0x205080.
				// Both tables appear "check-only" and have little effect on gameplay.
				count =(prot_data[0] & 0xff) + 1;          // unknown ( byte 0x00)
				i     = prot_data[1];                      // unknown ( byte 0x1f)
				adr   = prot_data[7]<<16 | prot_data[8];   // address (dword 0x210e00)
				lim   = prot_data[9];                      // unknown ( word 0x0010)
				src   = prot_data[10]<<16 | prot_data[11]; // unknown (dword zero)
				tgt   = prot_data[12]<<16 | prot_data[13]; // unknown (dword zero)
			break;

			case 0xa0: // update collision detection table (Violent Storm at 0x018b42)
				count = prot_data[0] & 0xff;             // number of objects - 1
				skip  = prot_data[1]>>(8-1);             // words to skip in each entry to reach the "hit list"
				adr   = prot_data[2]<<16 | prot_data[3]; // where the table is located
				bsize = prot_data[5]<<16 | prot_data[6]; // object entry size in bytes

				srcend = adr + bsize * count;
				tgtend = srcend + bsize;

				// let's hope GCC will inline the mem24bew calls
				for (src=adr; src<srcend; src+=bsize)
				{
					cx1 = (short)program_read_word(src);
					sx1 = (short)program_read_word(src + 2);
					wx1 = (short)program_read_word(src + 4);

					cy1 = (short)program_read_word(src + 6);
					sy1 = (short)program_read_word(src + 8);
					wy1 = (short)program_read_word(src +10);

					cz1 = (short)program_read_word(src +12);
					sz1 = (short)program_read_word(src +14);
					wz1 = (short)program_read_word(src +16);

					count = i = src + skip;
					tgt = src + bsize;

					for (; count<tgt; count++) program_write_byte(count, 0);

					for (; tgt<tgtend; i++, tgt+=bsize)
					{
						c2 = (short)program_read_word(tgt);
						s2 = (short)program_read_word(tgt + 2);
						w2 = (short)program_read_word(tgt + 4);
						if (abs((cx1+sx1)-(c2+s2))>=wx1+w2) continue; // X rejection

						c2 = (short)program_read_word(tgt + 6);
						s2 = (short)program_read_word(tgt + 8);
						w2 = (short)program_read_word(tgt +10);
						if (abs((cy1+sy1)-(c2+s2))>=wy1+w2) continue; // Y rejection

						c2 = (short)program_read_word(tgt +12);
						s2 = (short)program_read_word(tgt +14);
						w2 = (short)program_read_word(tgt +16);
						if (abs((cz1+sz1)-(c2+s2))>=wz1+w2) continue; // Z rejection

						program_write_byte(i, 0x80); // collision confirmed
					}
				}
			break;

			case 0xc0: // calculate object "homes-in" vector (Violent Storm at 0x03da9e)
				dx = (short)prot_data[0xc];
				dy = (short)prot_data[0xd];

				// it's not necessary to use lookup tables because Violent Storm
				// only calls the service once per enemy per frame.
				if (dx)
				{
					if (dy)
					{
						angle = (atan((double)dy / dx) * 128.0) / M_PI;
						if (dx < 0) angle += 128;
						i = (angle - 0x40) & 0xff;
					}
					else
						i = (dx > 0) ? 0xc0 : 0x40;
				}
				else
					if (dy > 0) i = 0;
				else
					if (dy < 0) i = 0x80;
				else
					i = mame_rand(Machine) & 0xff; // vector direction indeterminate

				prot_data[0x10] = i;
			break;

			default:
//              logerror("%06x: unknown K055550 command %02x\n", activecpu_get_pc(), data);
			break;
		}
	}
}

WRITE16_HANDLER( K053990_martchmp_word_w )
{
	int src_addr, src_count, src_skip;
	int dst_addr, dst_count, dst_skip;
	int mod_addr, mod_count, mod_skip, mod_offs;
	int mode, i, element_size = 1;
	UINT16 mod_val, mod_data;

	COMBINE_DATA(prot_data+offset);

	if (offset == 0x0c && ACCESSING_MSB)
	{
		mode  = (prot_data[0x0d]<<8 & 0xff00) | (prot_data[0x0f] & 0xff);

		switch (mode)
		{
			case 0xffff: // word copy
				element_size = 2;
			case 0xff00: // byte copy
				src_addr  = prot_data[0x0];
				src_addr |= prot_data[0x1]<<16 & 0xff0000;
				dst_addr  = prot_data[0x2];
				dst_addr |= prot_data[0x3]<<16 & 0xff0000;
				src_count = prot_data[0x8]>>8;
				dst_count = prot_data[0x9]>>8;
				src_skip  = prot_data[0xa] & 0xff;
				dst_skip  = prot_data[0xb] & 0xff;

				if ((prot_data[0x8] & 0xff) == 2) src_count <<= 1;
				src_skip += element_size;
				dst_skip += element_size;

				if (element_size == 1)
				for (i=src_count; i; i--)
				{
					program_write_byte(dst_addr, program_read_byte(src_addr));
					src_addr += src_skip;
					dst_addr += dst_skip;
				}
				else for (i=src_count; i; i--)
				{
					program_write_word(dst_addr, program_read_word(src_addr));
					src_addr += src_skip;
					dst_addr += dst_skip;
				}
			break;

			case 0x00ff: // sprite list modifier
				src_addr  = prot_data[0x0];
				src_addr |= prot_data[0x1]<<16 & 0xff0000;
				src_skip  = prot_data[0x1]>>8;
				dst_addr  = prot_data[0x2];
				dst_addr |= prot_data[0x3]<<16 & 0xff0000;
				dst_skip  = prot_data[0x3]>>8;
				mod_addr  = prot_data[0x4];
				mod_addr |= prot_data[0x5]<<16 & 0xff0000;
				mod_skip  = prot_data[0x5]>>8;
				mod_offs  = prot_data[0x8] & 0xff;
				mod_offs<<= 1;
				mod_count = 0x100;

				src_addr += mod_offs;
				dst_addr += mod_offs;

				for (i=mod_count; i; i--)
				{
					mod_val  = program_read_word(mod_addr);
					mod_addr += mod_skip;

					mod_data = program_read_word(src_addr);
					src_addr += src_skip;

					mod_data += mod_val;

					program_write_word(dst_addr, mod_data);
					dst_addr += dst_skip;
				}
			break;

			default:
			break;
		}
	}
}

void konamigx_esc_alert(UINT32 *srcbase, int srcoffs, int count, int mode) // (WARNING: assumed big endianess)
{

// hand-filled but should be close
static UINT8 ztable[7][8] =
{
	{5,4,3,2,1,7,6,0},
	{4,3,2,1,0,7,6,5},
	{4,3,2,1,0,7,6,5},
	{3,2,1,0,5,7,4,6},
	{6,5,1,4,3,7,0,2},
	{5,4,3,2,1,7,6,0},
	{5,4,3,2,1,7,6,0}
};

static UINT8 ptable[7][8] =
{
	{0x00,0x00,0x00,0x10,0x20,0x00,0x00,0x30},
	{0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x20},
	{0x00,0x00,0x00,0x20,0x20,0x00,0x00,0x00},
	{0x10,0x10,0x10,0x20,0x00,0x00,0x10,0x00},
	{0x00,0x00,0x20,0x00,0x10,0x00,0x20,0x20},
	{0x00,0x00,0x00,0x10,0x10,0x00,0x00,0x10},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10}
};

	INT32 data1, data2, i, j, vpos, hpos, voffs, hoffs, vcorr, hcorr, vmask, hmask, magicid;
	UINT32 *src, *srcend, *obj, *objend;
	UINT16 *dst;
	UINT8  *zcode, *pcode;

	if (!count || !srcbase) return;

	if (mode == 0)
	{
		src = srcbase + srcoffs;
		dst = K053247_ram;
		data1 = count<<2;
		data2 = count<<3;
		src += data1; dst += data2; i = -data1; j = -data2;
		do
		{
			data1 = src[i];
			data2 = src[i+1];
			i += 2;
			dst[j+1] = data1;
			dst[j+3] = data2;
			data1  >>= 16;
			data2  >>= 16;
			dst[j]   = data1;
			dst[j+2] = data2;
		}
		while (j += 4);
	}
	else
	{

#define EXTRACT_ODD         \
if((data1=obj[0])&0x8000)   \
{                           \
  i      = data1 & 7;       \
  data1 &= 0xff00;          \
  dst[0] = data1 | zcode[i];\
  data1  = obj[1];          \
  dst[1] = data1>>16;       \
  vpos   = data1 & 0xffff;  \
  data1  = obj[2];          \
  vpos  += voffs;           \
  dst[4] = data1;           \
  vpos  &= vmask;           \
  hpos   = data1>>16;       \
  data1  = obj[3];          \
  hpos  += hoffs;           \
  dst[2] = vpos;            \
  dst[3] = hpos;            \
  dst[5] = data1>>16;       \
  i      = pcode[i];        \
  dst[6] = data1| i<<4;     \
  dst += 8;                 \
  if (!(--j)) return;       \
}

#define EXTRACT_EVEN         \
if((data1=obj[0])&0x80000000)\
{                            \
  dst[1] = data1;            \
  data1>>= 16;               \
  i      = data1 & 7;        \
  data1 &= 0xff00;           \
  dst[0] = data1 | zcode[i]; \
  data1  = obj[1];           \
  hpos   = data1 & 0xffff;   \
  vpos   = data1>>16;        \
  hpos  += hoffs;            \
  vpos  += voffs;            \
  data1  = obj[2];           \
  vpos  &= vmask;            \
  dst[3] = hpos;             \
  dst[2] = vpos;             \
  dst[5] = data1;            \
  dst[4] = data1>>16;        \
  data1  = obj[3]>>16;       \
  i      = pcode[i];         \
  dst[6] = data1 | i<<4;     \
  dst += 8;                  \
  if (!(--j)) return;        \
}

		// These suspecious looking flags might tell the ESC chip what zcode/priority combos to use.
		// At the beginning of each sprite chunk there're at least three pointers to the main ROM but
		// I can't make out anything meaningful.
		magicid = srcbase[0x71f0/4];

		hmask = vmask = 0x3ff;
		if (magicid != 0x11010111)
		{
			switch (magicid)
			{
				case 0x10010801: i = 6; break;
				case 0x11010010: i = 5; vmask = 0x1ff; break;
				case 0x01111018: i = 4; break;
				case 0x10010011: i = 3;
					if ((srcbase[0x1c75]&0xff)==32) K055555_write_reg(K55_BLEND_ENABLES,36); // (TEMPORARY)
				break;
				case 0x11010811: i = 2; break;
				case 0x10000010: i = 1; break;
				default:         i = 0;
			}
			vcorr = srcbase[0x26a0/4] & 0xffff;
			hcorr = srcbase[0x26a4/4] >> 16;
			hcorr -= 10;
		}
		else
			hcorr = vcorr = i = 0;

		zcode = ztable[i];
		pcode = ptable[i];

		dst = K053247_ram;
		j = 256;

		// decode Vic-Viper
		if (srcbase[0x049c/4] & 0xffff0000)
		{
			hoffs = srcbase[0x0502/4] & 0xffff;
			voffs = srcbase[0x0506/4] & 0xffff;
			hoffs -= hcorr;
			voffs -= vcorr;
			obj = &srcbase[0x049e/4];
			EXTRACT_ODD
			obj = &srcbase[0x04ae/4];
			EXTRACT_ODD
			obj = &srcbase[0x04be/4];
			EXTRACT_ODD
		}

		// decode Lord British (the designer must be a Richard Garriot fan too:)
		if (srcbase[0x0848/4] & 0x0000ffff)
		{
			hoffs = srcbase[0x08b0/4]>>16;
			voffs = srcbase[0x08b4/4]>>16;
			hoffs -= hcorr;
			voffs -= vcorr;
			obj = &srcbase[0x084c/4];
			EXTRACT_EVEN
			obj = &srcbase[0x085c/4];
			EXTRACT_EVEN
			obj = &srcbase[0x086c/4];
			EXTRACT_EVEN
		}

		// decode common sprites
		src = srcbase + srcoffs;
		srcend = src + count * 0x30;
		do
		{
			if (!src[0] || !(i = src[7] & 0xf)) continue; // reject retired or zero-element groups
			i <<= 2;
			hoffs = src[5]>>16;
			voffs = src[6]>>16;
			hoffs -= hcorr;
			voffs -= vcorr;
			obj = src + 8;
			objend = obj + i;

			do
			{
				EXTRACT_EVEN
			}
			while ((obj+=4)<objend);
		}
		while ((src+=0x30)<srcend);

		// clear residual data
		if (j) do { *dst = 0; dst += 8; } while (--j);
	}

#undef EXTRACT_ODD
#undef EXTRACT_EVEN
}
