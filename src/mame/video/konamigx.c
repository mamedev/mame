/*
 * video/konamigx.c - Konami GX video hardware (here there be dragons, and achocode)
 *
 */

#include "emu.h"
#include "video/konamiic.h"
#include "video/k053250.h"
#include "includes/konamigx.h"


#define GX_DEBUG 0
#define VERBOSE 0

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
	#define GX_ZBUFW     576
	#define GX_ZBUFH     224
	#define GX_ZPAGESIZE (GX_ZBUFW*GX_ZBUFH)
	#define GX_ZBUFSIZE  ((GX_ZBUFW*GX_ZBUFH)*2)
#endif


static UINT8 *gx_objzbuf, *gx_shdzbuf;



static int layer_colorbase[4];
static INT32 gx_tilebanks[8], gx_oldbanks[8];
static int gx_tilemode, gx_rozenable, psac_colorbase, last_psac_colorbase;
static int gx_specialrozenable; // type 1 roz, with voxel height-map, rendered from 2 source tilemaps (which include height data) to temp bitmap for further processing
static int gx_rushingheroes_hack;
static int gx_le2_textcolour_hack;
static tilemap_t *gx_psac_tilemap, *gx_psac_tilemap2;
static bitmap_ind16* type3_roz_temp_bitmap;
static tilemap_t* gx_psac_tilemap_alt;

static int konamigx_has_dual_screen;
int konamigx_current_frame;
INLINE void set_color_555(running_machine &machine, pen_t color, int rshift, int gshift, int bshift, UINT16 data);
static int konamigx_palformat;
static bitmap_rgb32* dualscreen_left_tempbitmap;
static bitmap_rgb32* dualscreen_right_tempbitmap;

/* On Type-1 the K053936 output is rendered to these temporary bitmaps as raw data
   the 'voxel' effect to give the pixels height is a post-process operation on the
   output of the K053936 (this can clearly be seen in videos as large chunks of
   scenary flicker when in the distance due to single pixels in the K053936 output
   becoming visible / invisible due to drawing precision.

   -- however, progress on this has stalled as our K053936 doesn't seem to give
      the right output for post processing, I suspect the game is using some
      unsupported flipping modes (probably due to the way it's hooked up to the
      rest of the chips) which is causing entirely the wrong output.

   -- furthermore machine/konamigx.c (!) contains it's own implementation of
      the K053936_zoom_draw named K053936GP_zoom_draw ... It really shouldn't do,
      epsecially not in 'machine', which isn't meant to be video related.


   */
static bitmap_ind16 *gxtype1_roz_dstbitmap;
static bitmap_ind16 *gxtype1_roz_dstbitmap2;
static rectangle gxtype1_roz_dstbitmapclip;

static void (*game_tile_callback)(running_machine &machine, int layer, int *code, int *color, int *flags);

// Localized K053936/ROZ+
#define K053936_MAX_CHIPS 2

static rectangle K053936_cliprect[K053936_MAX_CHIPS];
static int K053936_offset[K053936_MAX_CHIPS][2] = {{0,0},{0,0}};
static int K053936_clip_enabled[K053936_MAX_CHIPS] = {0,0};


void K053936GP_set_offset(int chip, int xoffs, int yoffs) { K053936_offset[chip][0] = xoffs; K053936_offset[chip][1] = yoffs; }

void K053936GP_clip_enable(int chip, int status) { K053936_clip_enabled[chip] = status; }

void K053936GP_set_cliprect(int chip, int minx, int maxx, int miny, int maxy)
{
	rectangle &cliprect = K053936_cliprect[chip];
	cliprect.set(minx, maxx, miny, maxy);
}

INLINE void K053936GP_copyroz32clip( running_machine &machine,
		bitmap_rgb32 &dst_bitmap, bitmap_ind16 &src_bitmap,
		const rectangle &dst_cliprect, const rectangle &src_cliprect,
		UINT32 _startx,UINT32 _starty,int _incxx,int _incxy,int _incyx,int _incyy,
		int tilebpp, int blend, int alpha, int clip, int pixeldouble_output )
{
	static const int colormask[8]={1,3,7,0xf,0x1f,0x3f,0x7f,0xff};

	int cy, cx;
	int ecx;
	int src_pitch, incxy, incxx;
	int src_minx, src_maxx, src_miny, src_maxy, cmask;
	UINT16 *src_base;
	size_t src_size;

	const pen_t *pal_base;
	int dst_ptr;
	int dst_size;
	int dst_base2;

	int tx, dst_pitch;
	UINT32 *dst_base;
	int starty, incyy, startx, incyx, ty, sx, sy;

	incxy = _incxy; incxx = _incxx; incyy = _incyy; incyx = _incyx;
	starty = _starty; startx = _startx;

	if (clip) // set source clip range to some extreme values when disabled
	{
		src_minx = src_cliprect.min_x;
		src_maxx = src_cliprect.max_x;
		src_miny = src_cliprect.min_y;
		src_maxy = src_cliprect.max_y;
	}
	// this simply isn't safe to do!
	else { src_minx = src_miny = -0x10000; src_maxx = src_maxy = 0x10000; }

	// set target clip range
	sx = dst_cliprect.min_x;
	tx = dst_cliprect.max_x - sx + 1;
	sy = dst_cliprect.min_y;
	ty = dst_cliprect.max_y - sy + 1;

	startx += sx * incxx + sy * incyx;
	starty += sx * incxy + sy * incyy;

	// adjust entry points and other loop constants
	dst_pitch = dst_bitmap.rowpixels();
	dst_base = &dst_bitmap.pix32(0);
	dst_base2 = sy * dst_pitch + sx + tx;
	ecx = tx = -tx;

	tilebpp = (tilebpp-1) & 7;
	pal_base = machine.pens;
	cmask = colormask[tilebpp];

	src_pitch = src_bitmap.rowpixels();
	src_base = &src_bitmap.pix16(0);
	src_size = src_bitmap.width() * src_bitmap.height();
	dst_size = dst_bitmap.width() * dst_bitmap.height();
	dst_ptr = 0;//dst_base;
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
				UINT32 offs;
				offs = srcy * src_pitch + srcx;

				cx += incxx;
				cy += incxy;

				if (offs<0 || offs>=src_size)
					continue;

				if (srcx < src_minx || srcx > src_maxx || srcy < src_miny || srcy > src_maxy)
					continue;

				pixel = src_base[offs];
				if (!(pixel & cmask))
					continue;

				if ((dst_ptr+ecx+dst_base2)<dst_size) dst_base[dst_ptr+ecx+dst_base2] = alpha_blend_r32(pal_base[pixel], dst_base[dst_ptr+ecx+dst_base2], alpha);

				if (pixeldouble_output)
				{
					ecx++;
					if ((dst_ptr+ecx+dst_base2)<dst_size) dst_base[dst_ptr+ecx+dst_base2] = alpha_blend_r32(pal_base[pixel], dst_base[dst_ptr+ecx+dst_base2], alpha);
				}
			}
			while (++ecx < 0);

			ecx = tx;
			dst_ptr += dst_pitch;
			cy = starty; starty += incyy;
			cx = startx; startx += incyx;
		} while (--ty);
	}
	else	//  draw solid
	{
		if (blend == 0)
		{
			dst_ptr += dst_pitch;
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

				dst_ptr += dst_pitch;
				starty = cy + incyy;
				startx = cx + incyx;
			}
		}

		do {
			do {
				int srcx = (cx >> 16) & 0x1fff;
				int srcy = (cy >> 16) & 0x1fff;
				int pixel;
				UINT32 offs;

				offs = srcy * src_pitch + srcx;

				cx += incxx;
				cy += incxy;

				if (offs<0 || offs>=src_size)
					continue;

				if (srcx < src_minx || srcx > src_maxx || srcy < src_miny || srcy > src_maxy)
					continue;

				pixel = src_base[offs];
				if (!(pixel & cmask))
					continue;



				if ((dst_ptr+ecx+dst_base2)<dst_size) dst_base[dst_ptr+ecx+dst_base2] = pal_base[pixel];

				if (pixeldouble_output)
				{
					ecx++;
					if ((dst_ptr+ecx+dst_base2)<dst_size) dst_base[dst_ptr+ecx+dst_base2] = pal_base[pixel];
				}
			}
			while (++ecx < 0);

			ecx = tx;
			dst_ptr += dst_pitch;
			cy = starty; starty += incyy;
			cx = startx; startx += incyx;
		} while (--ty);
	}
}

// adapted from generic K053936_zoom_draw()
static void K053936GP_zoom_draw(running_machine &machine,
		int chip, UINT16 *ctrl, UINT16 *linectrl,
		bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tmap,
		int tilebpp, int blend, int alpha, int pixeldouble_output)
{
	UINT16 *lineaddr;

	rectangle my_clip;
	UINT32 startx, starty;
	int incxx, incxy, incyx, incyy, y, maxy, clip;

	bitmap_ind16 &src_bitmap = tmap->pixmap();
	rectangle &src_cliprect = K053936_cliprect[chip];
	clip = K053936_clip_enabled[chip];

	if (ctrl[0x07] & 0x0040)    /* "super" mode */
	{
		my_clip.min_x = cliprect.min_x;
		my_clip.max_x = cliprect.max_x;
		y = cliprect.min_y;
		maxy = cliprect.max_y;

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

			K053936GP_copyroz32clip(machine,
					bitmap, src_bitmap, my_clip, src_cliprect,
					startx<<5, starty<<5, incxx<<5, incxy<<5, 0, 0,
					tilebpp, blend, alpha, clip, pixeldouble_output);
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

		K053936GP_copyroz32clip(machine,
				bitmap, src_bitmap, cliprect, src_cliprect,
				startx<<5, starty<<5, incxx<<5, incxy<<5, incyx<<5, incyy<<5,
				tilebpp, blend, alpha, clip, pixeldouble_output);
	}
}

static void K053936GP_0_zoom_draw(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect,
		tilemap_t *tmap, int tilebpp, int blend, int alpha, int pixeldouble_output)
{
	K053936GP_zoom_draw(machine, 0,K053936_0_ctrl,K053936_0_linectrl,bitmap,cliprect,tmap,tilebpp,blend,alpha, pixeldouble_output);
}

static void K053936GP_1_zoom_draw(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect,
		tilemap_t *tmap, int tilebpp, int blend, int alpha, int pixeldouble_output)
{
	K053936GP_zoom_draw(machine, 1,K053936_1_ctrl,K053936_1_linectrl,bitmap,cliprect,tmap,tilebpp,blend,alpha, pixeldouble_output);
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


INLINE void zdrawgfxzoom32GP(
		bitmap_rgb32 &bitmap, const rectangle &cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy,
		int scalex, int scaley, int alpha, int drawmode, int zcode, int pri)
{
#define FP     19
#define FPONE  (1<<FP)
#define FPHALF (1<<(FP-1))
#define FPENT  0

	// inner loop
	const UINT8  *src_ptr;
	int src_x;
	int eax, ecx;
	int src_fx, src_fdx;
	int shdpen;
	UINT8  z8 = 0, p8 = 0;
	UINT8  *ozbuf_ptr;
	UINT8  *szbuf_ptr;
	const pen_t *pal_base;
	const pen_t *shd_base;
	UINT32 *dst_ptr;

	// outter loop
	int src_fby, src_fdy, src_fbx;
	const UINT8 *src_base;
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
	src_base  = gfx_element_get_data(gfx, code % gfx->total_elements);

	pal_base  = gfx->machine().pens + gfx->color_base + (color % gfx->total_colors) * granularity;
	shd_base  = gfx->machine().shadow_table;

	dst_ptr   = &bitmap.pix32(0);
	dst_pitch = bitmap.rowpixels();
	dst_minx  = cliprect.min_x;
	dst_maxx  = cliprect.max_x;
	dst_miny  = cliprect.min_y;
	dst_maxy  = cliprect.max_y;
	dst_x     = sx;
	dst_y     = sy;

	// cull off-screen objects
	if (dst_x > dst_maxx || dst_y > dst_maxy) return;
	nozoom = (scalex == 0x10000 && scaley == 0x10000);
	if (nozoom)
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
	z8 = (UINT8)zcode;
	p8 = (UINT8)pri;
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

							dst_ptr[ecx] = alpha_blend_r32(pal_base[eax], dst_ptr[ecx], alpha);
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

							dst_ptr[ecx] = alpha_blend_r32(pal_base[eax], dst_ptr[ecx], alpha);
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

							// the shadow tables are 15-bit lookup tables which accept RGB15... lossy, nasty, yuck!
							dst_ptr[ecx] = shd_base[rgb_to_rgb15(eax)];
							//dst_ptr[ecx] =(eax>>3&0x001f);lend_r32( eax, 0x00000000, 128);
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

							dst_ptr[ecx] = alpha_blend_r32(pal_base[eax], dst_ptr[ecx], alpha);
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

							dst_ptr[ecx] = alpha_blend_r32(pal_base[eax], dst_ptr[ecx], alpha);
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

							// the shadow tables are 15-bit lookup tables which accept RGB15... lossy, nasty, yuck!
							dst_ptr[ecx] = shd_base[rgb_to_rgb15(eax)];
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
	static const int coregmasks[5] = {0xf,0xe,0xc,0x8,0x0};
	static const int coregshifts[5]= {4,5,6,7,8};
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

static void konamigx_type2_sprite_callback(running_machine &machine, int *code, int *color, int *priority)
{
	int num = *code;
	int c18 = *color;

	*code = K053247_vrcbk[num>>14] | (num & 0x3fff);
	c18 = K053247GX_combine_c18(c18);
	*color = K055555GX_decode_objcolor(c18);
	*priority = K055555GX_decode_inpri(c18);
}

static void konamigx_dragoonj_sprite_callback(running_machine &machine, int *code, int *color, int *priority)
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

static void konamigx_salmndr2_sprite_callback(running_machine &machine, int *code, int *color, int *priority)
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

static void konamigx_le2_sprite_callback(running_machine &machine, int *code, int *color, int *priority)
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

static int K055555GX_decode_vmixcolor(int layer, int *color) // (see p.62 7.2.6 and p.27 3.3)
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

	if (gx_le2_textcolour_hack)
		if (layer==0)
			pal |= 0x1c0;

	if (von == 3) emx = -1; // invalidate external mix code if all bits are from internal
	*color =  pal;

	return(emx);
}

#ifdef UNUSED_FUNCTION
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
#endif

static void gx_wipezbuf(running_machine &machine, int noshadow)
{
	const rectangle &visarea = machine.primary_screen->visible_area();

	int w = visarea.width();
	int h = visarea.height();

	UINT8 *zptr = gx_objzbuf;
	int ecx = h;

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
UINT16 *K053247_ram;
static gfx_element *K053247_gfx;
static void (*K053247_callback)(running_machine &machine, int *code,int *color,int *priority);
static int K053247_dx, K053247_dy;

static int *K054338_shdRGB;


static void K053247GP_set_SpriteOffset(int offsx, int offsy)
{
	K053247_dx = offsx;
	K053247_dy = offsy;
}

void konamigx_mixer_init(running_machine &machine, int objdma)
{
	gx_objdma = 0;
	gx_primode = 0;

	gx_objzbuf = &machine.priority_bitmap.pix8(0);
	gx_shdzbuf = auto_alloc_array(machine, UINT8, GX_ZBUFSIZE);
	gx_objpool = auto_alloc_array(machine, struct GX_OBJ, GX_MAX_OBJECTS);

	K053247_export_config(&K053247_ram, &K053247_gfx, &K053247_callback, &K053247_dx, &K053247_dy);
	K054338_export_config(&K054338_shdRGB);

	if (objdma)
	{
		gx_spriteram = auto_alloc_array(machine, UINT16, 0x1000/2);
		gx_objdma = 1;
	}
	else
		gx_spriteram = K053247_ram;

	palette_set_shadow_dRGB32(machine, 3,-80,-80,-80, 0);
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

void konamigx_mixer(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect,
					tilemap_t *sub1, int sub1flags,
					tilemap_t *sub2, int sub2flags,
					int mixerflags, bitmap_ind16 *extra_bitmap, int rushingheroes_hack)
{
	static const int xoffset[8] = { 0, 1, 4, 5, 16, 17, 20, 21 };
	static const int yoffset[8] = { 0, 2, 8, 10, 32, 34, 40, 42 };
	static int parity = 0;

	int objbuf[GX_MAX_OBJECTS];
	int shadowon[3], shdpri[3], layerid[6], layerpri[6];

	struct GX_OBJ *objpool, *objptr;
	int wrapsize, xwraplim, ywraplim, cltc_shdpri, /*prflp,*/ disp;
	int xa,ya,ox,oy,zw,zh,flipx,flipy,mirrorx,mirrory,zoomx,zoomy,scalex,scaley,nozoom;
	int screenwidth, flipscreenx, flipscreeny, offx, offy;
	int nobj, i, j, k, l, temp, temp1, temp2, temp3, temp4, count;
	int order, offs, code, color, zcode, pri = 0, spri, spri_min, shdprisel, shadow, alpha, drawmode;

    // buffer can move when it's resized, so refresh the pointer
	gx_objzbuf = &machine.priority_bitmap.pix8(0);

	// abort if object database failed to initialize
	objpool = gx_objpool;
	if (!objpool) return;

	// clear screen with backcolor and update flicker pulse
	K054338_fill_backcolor(machine, bitmap, konamigx_wrport1_0 & 0x20);
	parity ^= 1;

	// abort if video has been disabled
	disp = K055555_read_register(K55_INPUT_ENABLES);
	if (!disp) return;
	cltc_shdpri = K054338_read_register(K338_REG_CONTROL);


	if (!rushingheroes_hack) // Slam Dunk 2 never sets this.  It's either part of the protection, or type4 doesn't use it
	{
		if (!(cltc_shdpri & K338_CTL_KILL)) return;
	}


	// demote shadows by one layer when this bit is set??? (see p.73 8.6)
	cltc_shdpri &= K338_CTL_SHDPRI;

	// wipe z-buffer
	if (mixerflags & GXMIX_NOZBUF)
		mixerflags |= GXMIX_NOSHADOW;
	else
		gx_wipezbuf(machine, mixerflags & GXMIX_NOSHADOW);

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
	//prflp = K055555_read_register(K55_CONTROL) & K55_CTL_FLIPPRI;

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
		K054338_update_all_shadows(machine, rushingheroes_hack);
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
				if (extra_bitmap) offs = -3;
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

		(*K053247_callback)(machine, &code, &color, &pri);

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
			shadow = k>>10 & 3;
			if (shadow) // object has shadow?
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
	screenwidth = machine.primary_screen->width();

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
						if (temp4 < 255) k = TILEMAP_DRAW_ALPHA(temp4);
					}

					if (mixerflags & 1<<(code+12)) k |= K056382_DRAW_FLAG_FORCE_XYSCROLL;

					K056832_tilemap_draw(machine, bitmap, cliprect, code, k, 0);
				}
				continue;
				case -2:
				case -4:
				if ((disp & K55_INP_SUB1) || (rushingheroes_hack))
				{
					int alpha = 255;

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
						alpha = temp4 = K054338_set_alpha_level(temp2);

						if (temp4 <= 0) continue;
						if (temp4 < 255) k = (j == GXMIX_BLEND_FAST) ? ~parity : 1;
					}

					l = sub1flags & 0xf;

					if (offs == -2)
					{
						int pixeldouble_output = 0;
						const rectangle &visarea = machine.primary_screen->visible_area();
						int width = visarea.width();

						if (width>512) // vsnetscr case
							pixeldouble_output = 1;

						K053936GP_0_zoom_draw(machine, bitmap, cliprect, sub1, l, k, alpha, pixeldouble_output);
					}
					else
					{
						machine.device<k053250_t>("k053250_1")->draw(bitmap, cliprect, vcblk[4]<<l, 0, 0);
					}
				}
				continue;
				case -3:
				case -5:
				if (disp & K55_INP_SUB2)
				{
					int alpha = 255;
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
						alpha = temp4 = K054338_set_alpha_level(temp2);

						if (temp4 <= 0) continue;
						if (temp4 < 255) k = (j == GXMIX_BLEND_FAST) ? ~parity : 1;
					}

					l = sub2flags & 0xf;

					if (offs == -3)
					{
						if (extra_bitmap) // soccer superstars roz layer
						{
							int xx,yy;
							int width = machine.primary_screen->width();
							int height = machine.primary_screen->height();
							const pen_t *paldata = machine.pens;

							// the output size of the roz layer has to be doubled horizontally
							// so that it aligns with the sprites and normal tilemaps.  This appears
							// to be done as a post-processing / mixing step effect
							//
							// - todo, use the pixeldouble_output I just added for vsnet instead?
							for (yy=0;yy<height;yy++)
							{
								UINT16* src = &extra_bitmap->pix16(yy);
								UINT32* dst = &bitmap.pix32(yy);
								int shiftpos = 0;
								for (xx=0;xx<width;xx+=2)
								{
									UINT16 dat = src[(((xx/2)+shiftpos))%width];
									if (dat&0xff)
										dst[xx+1] = dst[xx] = paldata[dat];
								}
							}
						}
						else
						{
							int pixeldouble_output = 0;
							K053936GP_1_zoom_draw(machine, bitmap, cliprect, sub2, l, k, alpha, pixeldouble_output);
						}
					}
					else
						machine.device<k053250_t>("k053250_2")->draw(bitmap, cliprect, vcblk[5]<<l, 0, 0);
				}
				continue;
			}
			continue;
		}

		drawmode = order>>4 & 0xf;

		alpha = 255;
		if (drawmode & 2)
		{
			alpha = color>>K055555_MIXSHIFT & 3;
			if (alpha) alpha = K054338_set_alpha_level(alpha);
			if (alpha <= 0) continue;
		}
		color &= K055555_COLORMASK;

		if (drawmode >= 4) palette_set_shadow_mode(machine, order & 0x0f);

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

		ox += K053247_dx;
		oy -= K053247_dy;
		ox = ( ox - offx) & temp;
		oy = (-oy - offy) & temp;
		if (ox >= xwraplim) ox -= wrapsize;
		if (oy >= ywraplim) oy -= wrapsize;


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

				zdrawgfxzoom32GP(
						bitmap, cliprect, K053247_gfx,
						temp,
						color,
						temp1,temp2,
						temp3,temp4,
						scalex, scaley, alpha, drawmode, zcode, pri);
			}
		}
	}
}





/* Run and Gun 2 / Rushing Heroes */
static TILE_GET_INFO( get_gx_psac_tile_info )
{
	int tileno, colour, col, flip = 0;
	if (tile_index&1)
	{
		tileno = gx_psacram[tile_index/2] & 0x00001fff;
		col    =(gx_psacram[tile_index/2] & 0x00002000)>>13;
		if      (gx_psacram[tile_index/2] & 0x00004000) flip |= TILE_FLIPX;
		if      (gx_psacram[tile_index/2] & 0x00008000) flip |= TILE_FLIPY;

	}
	else
	{
		tileno = (gx_psacram[tile_index/2] & 0x1fff0000)>>16;
		col    = (gx_psacram[tile_index/2] & 0x20000000)>>29;
		if       (gx_psacram[tile_index/2] & 0x40000000) flip |= TILE_FLIPX;
		if       (gx_psacram[tile_index/2] & 0x80000000) flip |= TILE_FLIPY;

	}

	colour = (psac_colorbase << 4) + col;

	SET_TILE_INFO(0, tileno, colour, TILE_FLIPYX(flip));
}

UINT32* konamigx_type3_psac2_bank;
static int konamigx_type3_psac2_actual_bank;
//int konamigx_type3_psac2_actual_last_bank = 0;

WRITE32_MEMBER(konamigx_state::konamigx_type3_psac2_bank_w)
{
	// other bits are used for something...

	COMBINE_DATA(&konamigx_type3_psac2_bank[offset]);
	konamigx_type3_psac2_actual_bank = (konamigx_type3_psac2_bank[0] & 0x10000000) >> 28;

	/* handle this by creating 2 roz tilemaps instead, otherwise performance dies completely on dual screen mode
    if (konamigx_type3_psac2_actual_bank!=konamigx_type3_psac2_actual_last_bank)
    {
        gx_psac_tilemap->mark_all_dirty();
        konamigx_type3_psac2_actual_last_bank = konamigx_type3_psac2_actual_bank;
    }
    */
}



/* Soccer Superstars (tile and flip bits now TRUSTED) */
 static TILE_GET_INFO( get_gx_psac3_tile_info )
 {
	int tileno, colour, flip;
	UINT8 *tmap = machine.region("gfx4")->base();

	int base_index = tile_index;

//  if (konamigx_type3_psac2_actual_bank)
//      base_index+=0x20000/2;


	tileno =  tmap[base_index*2] | ((tmap[(base_index*2)+1] & 0x0f)<<8);
	colour = (tmap[(base_index*2)+1]&0xc0)>>6;

	flip = 0;
	if (tmap[(base_index*2)+1] & 0x20) flip |= TILE_FLIPY;
	if (tmap[(base_index*2)+1] & 0x10) flip |= TILE_FLIPX;

	SET_TILE_INFO(0, tileno, colour, flip);
 }

 static TILE_GET_INFO( get_gx_psac3_alt_tile_info )
 {
	int tileno, colour, flip;
	UINT8 *tmap = machine.region("gfx4")->base()+0x20000;

	int base_index = tile_index;

//  if (konamigx_type3_psac2_actual_bank)
//      base_index+=0x20000/2;


	tileno =  tmap[base_index*2] | ((tmap[(base_index*2)+1] & 0x0f)<<8);
	colour = (tmap[(base_index*2)+1]&0xc0)>>6;

	flip = 0;
	if (tmap[(base_index*2)+1] & 0x20) flip |= TILE_FLIPY;
	if (tmap[(base_index*2)+1] & 0x10) flip |= TILE_FLIPX;

	SET_TILE_INFO(0, tileno, colour, flip);
 }


/* PSAC4 */
/* these tilemaps are weird in both format and content, one of them
   doesn't really look like it should be displayed? - it's height data */
static TILE_GET_INFO( get_gx_psac1a_tile_info )
{
	int tileno, colour, flipx,flipy;
	int flip;
	flip=0;
	colour = 0;

	tileno = (gx_psacram[tile_index*2] & 0x00003fff)>>0;

	// scanrows
	//flipx  = (gx_psacram[tile_index*2+1] & 0x00800000)>>23;
	//flipy  = (gx_psacram[tile_index*2+1] & 0x00400000)>>22;
	// scancols
	flipy  = (gx_psacram[tile_index*2+1] & 0x00800000)>>23;
	flipx  = (gx_psacram[tile_index*2+1] & 0x00400000)>>22;

	if (flipx) flip |= TILE_FLIPX;
	if (flipy) flip |= TILE_FLIPY;

	SET_TILE_INFO(1, tileno, colour, flip);
}

static TILE_GET_INFO( get_gx_psac1b_tile_info )
{
	int tileno, colour, flipx,flipy;
	int flip;
	flip=0;

	colour = 0;
	tileno = (gx_psacram[tile_index*2+1] & 0x00003fff)>>0;

	// scanrows
	//flipx  = (gx_psacram[tile_index*2+1] & 0x00800000)>>23;
	//flipy  = (gx_psacram[tile_index*2+1] & 0x00400000)>>22;
	// scancols
	flipy  = (gx_psacram[tile_index*2+1] & 0x00200000)>>21;
	flipx  = (gx_psacram[tile_index*2+1] & 0x00100000)>>20;

	if (flipx) flip |= TILE_FLIPX;
	if (flipy) flip |= TILE_FLIPY;

	SET_TILE_INFO(0, tileno, colour, flip);
}

static void konamigx_type2_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags)
{
	int d = *code;

	*code = (gx_tilebanks[(d & 0xe000)>>13]<<13) + (d & 0x1fff);
	K055555GX_decode_vmixcolor(layer, color);
}

static void konamigx_alpha_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags)
{
	int mixcode;
	int d = *code;

	mixcode = K055555GX_decode_vmixcolor(layer, color);

	if (mixcode < 0)
		*code = (gx_tilebanks[(d & 0xe000)>>13]<<13) + (d & 0x1fff);
	else
	{
		/* save mixcode and mark tile alpha (unimplemented) */
		*code = 0;

		if (VERBOSE)
			popmessage("skipped alpha tile(layer=%d mix=%d)", layer, mixcode);
	}
}

/*
> bits 8-13 are the low priority bits
> i.e. pri 0-5
> pri 6-7 can be either 1, bits 14,15 or bits 16,17
> contro.bit 2 being 0 forces the 1
> when control.bit 2 is 1, control.bit 3 selects between the two
> 0 selects 16,17
> that gives you the entire 8 bits of the sprite priority
> ok, lemme see if I've got this.  bit2 = 0 means the top bits are 11, bit2=1 means the top bits are bits 14/15 (of the whatever word?) else
+16+17?
> bit3=1 for the second

 *   6  | ---------xxxxxxx | "color", but depends on external connections


> there are 8 color lines entering the 5x5
> that means the palette is 4 bits, not 5 as you currently have
> the bits 4-9 are the low priority bits
> bits 10/11 or 12/13 are the two high priority bits, depending on the control word
> and bits 14/15 are the shadow bits
> mix0/1 and brit0/1 come from elsewhere
> they come from the '673 all right, but not from word 6
> and in fact the top address bits are highly suspect
> only 18 of the address bits go to the roms
> the next 2 go to cai0/1 and the next 4 to bk0-3
> (the '246 indexes the roms, the '673 reads the result)
> the roms are 64 bits wide
> so, well, the top bits of the code are suspicious
*/

static void _gxcommoninitnosprites(running_machine &machine)
{
	int i;

	K054338_vh_start(machine);
	K055555_vh_start(machine);

	konamigx_mixer_init(machine, 0);

	for (i = 0; i < 8; i++)
	{
		gx_tilebanks[i] = gx_oldbanks[i] = 0;
	}

	state_save_register_global_array(machine, gx_tilebanks);

	gx_tilemode = 0;

	gx_rozenable = 0;
	gx_specialrozenable = 0;
	gx_rushingheroes_hack = 0;
	gx_le2_textcolour_hack = 0;

	// Documented relative offsets of non-flipped games are (-2, 0, 2, 3),(0, 0, 0, 0).
	// (+ve values move layers to the right and -ve values move layers to the left)
	// In most cases only a constant is needed to add to the X offsets to yield correct
	// displacement. This should be done by the CCU but the CRT timings have not been
	// figured out.
	K056832_set_LayerOffset(0, -2, 0);
	K056832_set_LayerOffset(1,  0, 0);
	K056832_set_LayerOffset(2,  2, 0);
	K056832_set_LayerOffset(3,  3, 0);

	konamigx_has_dual_screen = 0;
	konamigx_current_frame = 0;
}

static void _gxcommoninit(running_machine &machine)
{
	// (+ve values move objects to the right and -ve values move objects to the left)
	K055673_vh_start(machine, "gfx2", K055673_LAYOUT_GX, -26, -23, konamigx_type2_sprite_callback);

	_gxcommoninitnosprites(machine);
}


VIDEO_START(konamigx_5bpp)
{
	if (!strcmp(machine.system().name,"sexyparo") || !strcmp(machine.system().name,"sexyparoa"))
		game_tile_callback = konamigx_alpha_tile_callback;
	else
		game_tile_callback = konamigx_type2_tile_callback;

	K056832_vh_start(machine, "gfx1", K056832_BPP_5, 0, NULL, game_tile_callback, 0);

	_gxcommoninit(machine);

	/* here are some hand tuned per game scroll offsets to go with the per game visible areas,
       i see no better way of doing this for now... */

	if (!strcmp(machine.system().name,"tbyahhoo"))
	{
		K056832_set_UpdateMode(1);
		gx_tilemode = 1;
	} else

	if (!strcmp(machine.system().name,"puzldama"))
	{
		K053247GP_set_SpriteOffset(-46, -23);
		konamigx_mixer_primode(5);
	} else

	if (!strcmp(machine.system().name,"daiskiss"))
	{
		konamigx_mixer_primode(4);
	} else

	if (!strcmp(machine.system().name,"gokuparo") || !strcmp(machine.system().name,"fantjour") || !strcmp(machine.system().name,"fantjoura"))
	{
		K053247GP_set_SpriteOffset(-46, -23);
	} else

	if (!strcmp(machine.system().name,"sexyparo") || !strcmp(machine.system().name,"sexyparoa"))
	{
		K053247GP_set_SpriteOffset(-42, -23);
	}
}

VIDEO_START(winspike)
{
	K056832_vh_start(machine, "gfx1", K056832_BPP_8, 0, NULL, konamigx_alpha_tile_callback, 2);
	K055673_vh_start(machine, "gfx2", K055673_LAYOUT_LE2, -53, -23, konamigx_type2_sprite_callback);

	_gxcommoninitnosprites(machine);
}

VIDEO_START(dragoonj)
{
	K056832_vh_start(machine, "gfx1", K056832_BPP_5, 1, NULL, konamigx_type2_tile_callback, 0);
	K055673_vh_start(machine, "gfx2", K055673_LAYOUT_RNG, -53, -23, konamigx_dragoonj_sprite_callback);

	_gxcommoninitnosprites(machine);

	K056832_set_LayerOffset(0, -2+1, 0);
	K056832_set_LayerOffset(1,  0+1, 0);
	K056832_set_LayerOffset(2,  2+1, 0);
	K056832_set_LayerOffset(3,  3+1, 0);
}

VIDEO_START(le2)
{
	K056832_vh_start(machine, "gfx1", K056832_BPP_8, 1, NULL, konamigx_type2_tile_callback, 0);
	K055673_vh_start(machine, "gfx2", K055673_LAYOUT_LE2, -46, -23, konamigx_le2_sprite_callback);

	_gxcommoninitnosprites(machine);

	konamigx_mixer_primode(-1); // swapped layer B and C priorities?

	gx_le2_textcolour_hack = 1; // force text layer to use the right palette
	K055555_write_reg(K55_INPUT_ENABLES, 1); // it doesn't turn on the video output at first for the test screens, maybe it should default to ON?
}

VIDEO_START(konamigx_6bpp)
{
	K056832_vh_start(machine, "gfx1", K056832_BPP_6, 0, NULL, konamigx_type2_tile_callback, 0);

	_gxcommoninit(machine);

	if (!strcmp(machine.system().name,"tokkae") || !strcmp(machine.system().name,"tkmmpzdm"))
	{
		K053247GP_set_SpriteOffset(-46, -23);
		konamigx_mixer_primode(5);
	}
}

VIDEO_START(konamigx_type3)
{
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	K056832_vh_start(machine, "gfx1", K056832_BPP_6, 0, NULL, konamigx_type2_tile_callback, 1);
	K055673_vh_start(machine, "gfx2", K055673_LAYOUT_GX6, -132, -23, konamigx_type2_sprite_callback);

	dualscreen_left_tempbitmap = auto_bitmap_rgb32_alloc(machine, width, height);
	dualscreen_right_tempbitmap = auto_bitmap_rgb32_alloc(machine, width, height);

	_gxcommoninitnosprites(machine);

	gx_psac_tilemap = tilemap_create(machine, get_gx_psac3_tile_info, tilemap_scan_cols,  16, 16, 256, 256);
	gx_psac_tilemap_alt = tilemap_create(machine, get_gx_psac3_alt_tile_info, tilemap_scan_cols,  16, 16, 256, 256);

	gx_rozenable = 0;
	gx_specialrozenable = 2;


	/* set up tile layers */
	type3_roz_temp_bitmap = auto_bitmap_ind16_alloc(machine, width, height);


	//gx_psac_tilemap->set_flip(TILEMAP_FLIPX| TILEMAP_FLIPY);

	K053936_wraparound_enable(0, 1);
//  K053936GP_set_offset(0, -30, -1);
	K053936_set_offset(0, -30, +1);

	K056832_set_LayerOffset(0,  -52, 0);
	K056832_set_LayerOffset(1,  -48, 0);
	K056832_set_LayerOffset(2,  -48, 0);
	K056832_set_LayerOffset(3,  -48, 0);

	konamigx_has_dual_screen = 1;
	konamigx_palformat = 1;
}

VIDEO_START(konamigx_type4)
{
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	K056832_vh_start(machine, "gfx1", K056832_BPP_8, 0, NULL, konamigx_type2_tile_callback, 0);
	K055673_vh_start(machine, "gfx2", K055673_LAYOUT_GX6, -79, -24, konamigx_type2_sprite_callback); // -23 looks better in intro

	dualscreen_left_tempbitmap = auto_bitmap_rgb32_alloc(machine, width, height);
	dualscreen_right_tempbitmap = auto_bitmap_rgb32_alloc(machine, width, height);

	_gxcommoninitnosprites(machine);

	gx_psac_tilemap = tilemap_create(machine, get_gx_psac_tile_info, tilemap_scan_cols,  16, 16, 128, 128);
	gx_rozenable = 0;
	gx_specialrozenable = 3;

	K056832_set_LayerOffset(0,  -27, 0);
	K056832_set_LayerOffset(1,  -25, 0);
	K056832_set_LayerOffset(2,  -24, 0);
	K056832_set_LayerOffset(3,  -22, 0);

	K053936_wraparound_enable(0, 0);
	K053936GP_set_offset(0, -36, 1);

	gx_rushingheroes_hack = 1;
	konamigx_has_dual_screen = 1;
	konamigx_palformat = 0;

}

VIDEO_START(konamigx_type4_vsn)
{
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	K056832_vh_start(machine, "gfx1", K056832_BPP_8, 0, NULL, konamigx_type2_tile_callback, 2);   // set djmain_hack to 2 to kill layer association or half the tilemaps vanish on screen 0
	K055673_vh_start(machine, "gfx2", K055673_LAYOUT_GX6, -132, -23, konamigx_type2_sprite_callback);

	dualscreen_left_tempbitmap = auto_bitmap_rgb32_alloc(machine, width, height);
	dualscreen_right_tempbitmap = auto_bitmap_rgb32_alloc(machine, width, height);

	_gxcommoninitnosprites(machine);

	gx_psac_tilemap = tilemap_create(machine, get_gx_psac_tile_info, tilemap_scan_cols,  16, 16, 128, 128);
	gx_rozenable = 0;
	gx_specialrozenable = 3;

	K056832_set_LayerOffset(0,  -52, 0);
	K056832_set_LayerOffset(1,  -48, 0);
	K056832_set_LayerOffset(2,  -48, 0);
	K056832_set_LayerOffset(3,  -48, 0);

	K053936_wraparound_enable(0, 1); // wraparound doesn't work properly with the custom drawing function anyway, see the crowd in vsnet and rushhero
	K053936GP_set_offset(0, -30, 0);

	gx_rushingheroes_hack = 1;
	konamigx_has_dual_screen = 1;
	konamigx_palformat = 0;
}

VIDEO_START(konamigx_type4_sd2)
{
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	K056832_vh_start(machine, "gfx1", K056832_BPP_8, 0, NULL, konamigx_type2_tile_callback, 0);
	K055673_vh_start(machine, "gfx2", K055673_LAYOUT_GX6, -81, -23, konamigx_type2_sprite_callback);

	dualscreen_left_tempbitmap = auto_bitmap_rgb32_alloc(machine, width, height);
	dualscreen_right_tempbitmap = auto_bitmap_rgb32_alloc(machine, width, height);

	_gxcommoninitnosprites(machine);

	gx_psac_tilemap = tilemap_create(machine, get_gx_psac_tile_info, tilemap_scan_cols,  16, 16, 128, 128);
	gx_rozenable = 0;
	gx_specialrozenable = 3;


	K056832_set_LayerOffset(0,  -29, -1);
	K056832_set_LayerOffset(1,  -27, -1);
	K056832_set_LayerOffset(2,  -26, -1);
	K056832_set_LayerOffset(3,  -24, -1);


	K053936_wraparound_enable(0, 0);
	K053936GP_set_offset(0, -36, -1);

	gx_rushingheroes_hack = 1;
	konamigx_has_dual_screen = 1;
	konamigx_palformat = 0;

}


VIDEO_START(konamigx_6bpp_2)
{
	K056832_vh_start(machine, "gfx1", K056832_BPP_6, 1, NULL, konamigx_type2_tile_callback, 0);

	if (!strcmp(machine.system().name,"salmndr2") || !strcmp(machine.system().name,"salmndr2a"))
	{
		K055673_vh_start(machine, "gfx2", K055673_LAYOUT_GX6, -48, -23, konamigx_salmndr2_sprite_callback);

		_gxcommoninitnosprites(machine);
	}
	else
	{
		_gxcommoninit(machine);
	}
}

VIDEO_START(opengolf)
{
	K056832_vh_start(machine, "gfx1", K056832_BPP_5, 0, NULL, konamigx_type2_tile_callback, 0);
	K055673_vh_start(machine, "gfx2", K055673_LAYOUT_GX6, -53, -23, konamigx_type2_sprite_callback);

	_gxcommoninitnosprites(machine);

	K056832_set_LayerOffset(0, -2+1, 0);
	K056832_set_LayerOffset(1,  0+1, 0);
	K056832_set_LayerOffset(2,  2+1, 0);
	K056832_set_LayerOffset(3,  3+1, 0);

	gx_psac_tilemap = tilemap_create(machine, get_gx_psac1a_tile_info, tilemap_scan_cols,  16, 16, 128, 128);
	gx_psac_tilemap2 = tilemap_create(machine, get_gx_psac1b_tile_info, tilemap_scan_cols,  16, 16, 128, 128);

	// transparency will be handled manually in post-processing
	//gx_psac_tilemap->set_transparent_pen(0);
	//gx_psac_tilemap2->set_transparent_pen(0);

	gx_rozenable = 0;
	gx_specialrozenable = 1;

	gxtype1_roz_dstbitmap =  auto_bitmap_ind16_alloc(machine,512,512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing
	gxtype1_roz_dstbitmap2 = auto_bitmap_ind16_alloc(machine,512,512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing


	gxtype1_roz_dstbitmapclip.set(0, 512-1, 0, 512-1);


	K053936_wraparound_enable(0, 1);
	K053936GP_set_offset(0, 0, 0);

	// urgh.. the priority bitmap is global, and because our temp bitmaps are bigger than the screen, this causes issues.. so just allocate something huge
	// until there is a better solution, or priority bitmap can be specified manually.
	machine.priority_bitmap.allocate(2048,2048);

}

VIDEO_START(racinfrc)
{
	K056832_vh_start(machine, "gfx1", K056832_BPP_6, 0, NULL, konamigx_type2_tile_callback, 0);
	K055673_vh_start(machine, "gfx2", K055673_LAYOUT_GX, -53, -23, konamigx_type2_sprite_callback);

	_gxcommoninitnosprites(machine);

	K056832_set_LayerOffset(0, -2+1, 0);
	K056832_set_LayerOffset(1,  0+1, 0);
	K056832_set_LayerOffset(2,  2+1, 0);
	K056832_set_LayerOffset(3,  3+1, 0);

	gx_psac_tilemap = tilemap_create(machine, get_gx_psac1a_tile_info, tilemap_scan_cols,  16, 16, 128, 128);
	gx_psac_tilemap2 = tilemap_create(machine, get_gx_psac1b_tile_info, tilemap_scan_cols,  16, 16, 128, 128);

	// transparency will be handled manually in post-processing
	//gx_psac_tilemap->set_transparent_pen(0);
	//gx_psac_tilemap2->set_transparent_pen(0);

	gx_rozenable = 0;
	gx_specialrozenable = 1;

	gxtype1_roz_dstbitmap =  auto_bitmap_ind16_alloc(machine,512,512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing
	gxtype1_roz_dstbitmap2 = auto_bitmap_ind16_alloc(machine,512,512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing


	gxtype1_roz_dstbitmapclip.set(0, 512-1, 0, 512-1);


	K053936_wraparound_enable(0, 1);
	K053936GP_set_offset(0, 0, 0);

	// urgh.. the priority bitmap is global, and because our temp bitmaps are bigger than the screen, this causes issues.. so just allocate something huge
	// until there is a better solution, or priority bitmap can be specified manually.
	machine.priority_bitmap.allocate(2048,2048);


}

SCREEN_UPDATE_RGB32(konamigx)
{
	int i, newbank, newbase, dirty, unchained;

	/* if any banks are different from last render, we need to flush the planes */
	for (dirty = 0, i = 0; i < 8; i++)
	{
		newbank = gx_tilebanks[i];
		if (gx_oldbanks[i] != newbank) { gx_oldbanks[i] = newbank; dirty = 1; }
	}

	if (gx_tilemode == 0)
	{
		// driver approximates tile update in mode 0 for speed
		unchained = K056832_get_LayerAssociation();
		for (i=0; i<4; i++)
		{
			newbase = K055555_get_palette_index(i)<<6;
			if (layer_colorbase[i] != newbase)
			{
				layer_colorbase[i] = newbase;

				if (unchained)
					K056832_mark_plane_dirty(i);
				else
					dirty = 1;
			}
		}
	}
	else
	{
		// K056832 does all the tracking in mode 1 for accuracy (Twinbee needs this)
	}

	// sub2 is PSAC colorbase on GX
	if (gx_rozenable)
	{
		last_psac_colorbase = psac_colorbase;
		psac_colorbase = K055555_get_palette_index(6);

		if (psac_colorbase != last_psac_colorbase)
		{
			gx_psac_tilemap->mark_all_dirty();
			if (gx_rozenable == 3)
			{
				gx_psac_tilemap2->mark_all_dirty();
			}
		}
	}

	if (dirty) K056832_MarkAllTilemapsDirty();

	// Type-1
	if (gx_specialrozenable == 1)
	{
		K053936_0_zoom_draw(*gxtype1_roz_dstbitmap, gxtype1_roz_dstbitmapclip,gx_psac_tilemap, 0,0,0); // height data
		K053936_0_zoom_draw(*gxtype1_roz_dstbitmap2,gxtype1_roz_dstbitmapclip,gx_psac_tilemap2,0,0,0); // colour data (+ some voxel height data?)
	}



	if (gx_specialrozenable==3)
	{
		konamigx_mixer(screen.machine(), bitmap, cliprect, gx_psac_tilemap, GXSUB_8BPP,0,0,  0, 0, gx_rushingheroes_hack);
	}
	// hack, draw the roz tilemap if W is held
	// todo: fix so that it works with the mixer without crashing(!)
	else if (gx_specialrozenable == 2)
	{
		// we're going to throw half of this away anyway in post-process, so only render what's needed
		rectangle temprect;
		temprect = cliprect;
		temprect.max_x = cliprect.min_x+320;

		if (konamigx_type3_psac2_actual_bank == 1) K053936_0_zoom_draw(*type3_roz_temp_bitmap, temprect,gx_psac_tilemap_alt, 0,0,0); // soccerss playfield
		else K053936_0_zoom_draw(*type3_roz_temp_bitmap, temprect,gx_psac_tilemap, 0,0,0); // soccerss playfield


		konamigx_mixer(screen.machine(), bitmap, cliprect, 0, 0, 0, 0, 0, type3_roz_temp_bitmap, gx_rushingheroes_hack);
	}
	else
	{
		konamigx_mixer(screen.machine(), bitmap, cliprect, 0, 0, 0, 0, 0, 0, gx_rushingheroes_hack);
	}



	/* Hack! draw type-1 roz layer here for testing purposes only */
	if (gx_specialrozenable == 1)
	{
		const pen_t *paldata = screen.machine().pens;

		if ( screen.machine().input().code_pressed(KEYCODE_W) )
		{
			int y,x;

			// make it flicker, to compare positioning
			//if (screen.frame_number() & 1)
			{

				for (y=0;y<256;y++)
				{
					//UINT16* src = &gxtype1_roz_dstbitmap->pix16(y);

					//UINT32* dst = &bitmap.pix32(y);
					// ths K053936 rendering should probably just be flipped
					// this is just kludged to align the racing force 2d logo
					UINT16* src = &gxtype1_roz_dstbitmap2->pix16(y+30);
					UINT32* dst = &bitmap.pix32(256-y);

					for (x=0;x<512;x++)
					{
						UINT16 dat = src[x];
						dst[x] = paldata[dat];
					}
				}
			}

		}

	}

	return 0;
}

SCREEN_UPDATE_RGB32(konamigx_left)
{
	/* the video gets demuxed by a board which plugs into the jamma connector */
	konamigx_state *state = screen.machine().driver_data<konamigx_state>();
	konamigx_current_frame^=1;

	if (konamigx_current_frame==1)
	{
		int offset=0;

		if (konamigx_palformat==1)
		{
			for (offset=0;offset<0x4000/4;offset++)
			{
				UINT32 coldat = state->m_generic_paletteram_32[offset];

				set_color_555(screen.machine(), offset*2, 0, 5, 10,coldat >> 16);
				set_color_555(screen.machine(), offset*2+1, 0, 5, 10,coldat & 0xffff);
			}
		}
		else
		{
			for (offset=0;offset<0x8000/4;offset++)
			{
				int r,g,b;

				r = (state->m_generic_paletteram_32[offset] >>16) & 0xff;
				g = (state->m_generic_paletteram_32[offset] >> 8) & 0xff;
				b = (state->m_generic_paletteram_32[offset] >> 0) & 0xff;

				palette_set_color(screen.machine(),offset,MAKE_RGB(r,g,b));
			}
		}

		SCREEN_UPDATE_NAME(konamigx)(NULL, screen, downcast<bitmap_rgb32 &>(*dualscreen_left_tempbitmap), cliprect);
		copybitmap(bitmap, *dualscreen_left_tempbitmap, 0, 0, 0, 0, cliprect);
	}
	else
	{
		copybitmap(bitmap, *dualscreen_left_tempbitmap, 0, 0, 0, 0, cliprect);
	}

	return 0;
}

SCREEN_UPDATE_RGB32(konamigx_right)
{
	if (konamigx_current_frame==1)
	{
		copybitmap(bitmap, *dualscreen_right_tempbitmap, 0, 0, 0, 0, cliprect);
	}
	else
	{

		int offset=0;

		if (konamigx_palformat==1)
		{
			for (offset=0;offset<0x4000/4;offset++)
			{
				UINT32 coldat = gx_subpaletteram32[offset];

				set_color_555(screen.machine(), offset*2, 0, 5, 10,coldat >> 16);
				set_color_555(screen.machine(), offset*2+1, 0, 5, 10,coldat & 0xffff);
			}
		}
		else
		{
			for (offset=0;offset<0x8000/4;offset++)
			{
				int r,g,b;

				r = (gx_subpaletteram32[offset] >>16) & 0xff;
				g = (gx_subpaletteram32[offset] >> 8) & 0xff;
				b = (gx_subpaletteram32[offset] >> 0) & 0xff;

				palette_set_color(screen.machine(),offset,MAKE_RGB(r,g,b));
			}
		}

		SCREEN_UPDATE_NAME(konamigx)(NULL, screen, downcast<bitmap_rgb32 &>(*dualscreen_right_tempbitmap), cliprect);
		copybitmap(bitmap, *dualscreen_right_tempbitmap, 0, 0, 0, 0, cliprect);
	}

	return 0;
}


WRITE32_MEMBER(konamigx_state::konamigx_palette_w)
{
	int r,g,b;

	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	r = (m_generic_paletteram_32[offset] >>16) & 0xff;
	g = (m_generic_paletteram_32[offset] >> 8) & 0xff;
	b = (m_generic_paletteram_32[offset] >> 0) & 0xff;

	palette_set_color(machine(),offset,MAKE_RGB(r,g,b));
}

#ifdef UNUSED_FUNCTION
WRITE32_MEMBER(konamigx_state::konamigx_palette2_w)
{
	int r,g,b;

	COMBINE_DATA(&gx_subpaletteram32[offset]);

	r = (gx_subpaletteram32[offset] >>16) & 0xff;
	g = (gx_subpaletteram32[offset] >> 8) & 0xff;
	b = (gx_subpaletteram32[offset] >> 0) & 0xff;

	offset += (0x8000/4);

	palette_set_color(machine(),offset,MAKE_RGB(r,g,b));
}
#endif

INLINE void set_color_555(running_machine &machine, pen_t color, int rshift, int gshift, int bshift, UINT16 data)
{
	palette_set_color_rgb(machine, color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}

#ifdef UNUSED_FUNCTION
// main monitor for type 3
WRITE32_MEMBER(konamigx_state::konamigx_555_palette_w)
{
	UINT32 coldat;
	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	coldat = m_generic_paletteram_32[offset];

	set_color_555(machine(), offset*2, 0, 5, 10,coldat >> 16);
	set_color_555(machine(), offset*2+1, 0, 5, 10,coldat & 0xffff);
}

// sub monitor for type 3
WRITE32_MEMBER(konamigx_state::konamigx_555_palette2_w)
{
	UINT32 coldat;
	COMBINE_DATA(&gx_subpaletteram32[offset]);
	coldat = gx_subpaletteram32[offset];

	offset += (0x4000/4);

	set_color_555(machine(), offset*2, 0, 5, 10,coldat >> 16);
	set_color_555(machine(), offset*2+1, 0, 5, 10,coldat & 0xffff);
}
#endif


WRITE32_MEMBER(konamigx_state::konamigx_tilebank_w)
{
	if (ACCESSING_BITS_24_31)
		gx_tilebanks[offset*4] = (data>>24)&0xff;
	if (ACCESSING_BITS_16_23)
		gx_tilebanks[offset*4+1] = (data>>16)&0xff;
	if (ACCESSING_BITS_8_15)
		gx_tilebanks[offset*4+2] = (data>>8)&0xff;
	if (ACCESSING_BITS_0_7)
		gx_tilebanks[offset*4+3] = data&0xff;
}

// type 1 RAM-based PSAC tilemap
WRITE32_MEMBER(konamigx_state::konamigx_t1_psacmap_w)
{
	COMBINE_DATA(&gx_psacram[offset]);
	gx_psac_tilemap->mark_tile_dirty(offset/2);
	gx_psac_tilemap2->mark_tile_dirty(offset/2);
}

// type 4 RAM-based PSAC tilemap
WRITE32_MEMBER(konamigx_state::konamigx_t4_psacmap_w)
{
	COMBINE_DATA(&gx_psacram[offset]);

	gx_psac_tilemap->mark_tile_dirty(offset*2);
	gx_psac_tilemap->mark_tile_dirty((offset*2)+1);
}

