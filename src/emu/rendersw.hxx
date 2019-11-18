// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    rendersw.inc

    Software-only rasterization system.

***************************************************************************/


#include "emucore.h"
#include "eminline.h"
#include "video/rgbutil.h"
#include "render.h"


template<typename _PixelType, int _SrcShiftR, int _SrcShiftG, int _SrcShiftB, int _DstShiftR, int _DstShiftG, int _DstShiftB, bool _NoDestRead = false, bool _BilinearFilter = false>
class software_renderer
{
private:
	// internal structs
	struct quad_setup_data
	{
		s32 dudx, dvdx, dudy, dvdy;
		s32 startu, startv;
		s32 startx, starty;
		s32 endx, endy;
	};

	// internal helpers
	static inline bool is_opaque(float alpha) { return (alpha >= (_NoDestRead ? 0.5f : 1.0f)); }
	static inline bool is_transparent(float alpha) { return (alpha < (_NoDestRead ? 0.5f : 0.0001f)); }
	static inline rgb_t apply_intensity(int intensity, rgb_t color) { return color.scale8(intensity); }
	static inline float round_nearest(float f) { return floor(f + 0.5f); }

	// destination pixels are written based on the values of the template parameters
	static inline _PixelType dest_assemble_rgb(u32 r, u32 g, u32 b) { return (r << _DstShiftR) | (g << _DstShiftG) | (b << _DstShiftB); }
	static inline _PixelType dest_rgb_to_pixel(u32 r, u32 g, u32 b) { return dest_assemble_rgb(r >> _SrcShiftR, g >> _SrcShiftG, b >> _SrcShiftB); }

	// source 32-bit pixels are in MAME standardized format
	static inline u32 source32_r(u32 pixel) { return (pixel >> (16 + _SrcShiftR)) & (0xff >> _SrcShiftR); }
	static inline u32 source32_g(u32 pixel) { return (pixel >> ( 8 + _SrcShiftG)) & (0xff >> _SrcShiftG); }
	static inline u32 source32_b(u32 pixel) { return (pixel >> ( 0 + _SrcShiftB)) & (0xff >> _SrcShiftB); }

	// destination pixel masks are based on the template parameters as well
	static inline u32 dest_r(_PixelType pixel) { return (pixel >> _DstShiftR) & (0xff >> _SrcShiftR); }
	static inline u32 dest_g(_PixelType pixel) { return (pixel >> _DstShiftG) & (0xff >> _SrcShiftG); }
	static inline u32 dest_b(_PixelType pixel) { return (pixel >> _DstShiftB) & (0xff >> _SrcShiftB); }

	// generic conversion with special optimization for destinations in the standard format
	static inline _PixelType source32_to_dest(u32 pixel)
	{
		if (_SrcShiftR == 0 && _SrcShiftG == 0 && _SrcShiftB == 0 && _DstShiftR == 16 && _DstShiftG == 8 && _DstShiftB == 0)
			return pixel;
		else
			return dest_assemble_rgb(source32_r(pixel), source32_g(pixel), source32_b(pixel));
	}


	//-------------------------------------------------
	//  ycc_to_rgb - convert YCC to RGB; the YCC pixel
	//  contains Y in the LSB, Cb << 8, and Cr << 16
	//  This actually a YCbCr conversion,
	//  details my be found in chapter 6.4 ff of
	//  http://softwarecommunity.intel.com/isn/downloads/softwareproducts/pdfs/346495.pdf
	//  The document also contains the constants below as floats.
	//-------------------------------------------------

	static inline u32 clamp16_shift8(u32 x)
	{
		return ((s32(x) < 0) ? 0 : (x > 65535 ? 255: x >> 8));
	}

	static inline u32 ycc_to_rgb(u32 ycc)
	{
		// original equations:
		//
		//  C = Y - 16
		//  D = Cb - 128
		//  E = Cr - 128
		//
		//  R = clip(( 298 * C           + 409 * E + 128) >> 8)
		//  G = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8)
		//  B = clip(( 298 * C + 516 * D           + 128) >> 8)
		//
		//  R = clip(( 298 * (Y - 16)                    + 409 * (Cr - 128) + 128) >> 8)
		//  G = clip(( 298 * (Y - 16) - 100 * (Cb - 128) - 208 * (Cr - 128) + 128) >> 8)
		//  B = clip(( 298 * (Y - 16) + 516 * (Cb - 128)                    + 128) >> 8)
		//
		//  R = clip(( 298 * Y - 298 * 16                        + 409 * Cr - 409 * 128 + 128) >> 8)
		//  G = clip(( 298 * Y - 298 * 16 - 100 * Cb + 100 * 128 - 208 * Cr + 208 * 128 + 128) >> 8)
		//  B = clip(( 298 * Y - 298 * 16 + 516 * Cb - 516 * 128                        + 128) >> 8)
		//
		//  R = clip(( 298 * Y - 298 * 16                        + 409 * Cr - 409 * 128 + 128) >> 8)
		//  G = clip(( 298 * Y - 298 * 16 - 100 * Cb + 100 * 128 - 208 * Cr + 208 * 128 + 128) >> 8)
		//  B = clip(( 298 * Y - 298 * 16 + 516 * Cb - 516 * 128                        + 128) >> 8)
		//
		//  Now combine constants:
		//
		//  R = clip(( 298 * Y            + 409 * Cr - 56992) >> 8)
		//  G = clip(( 298 * Y - 100 * Cb - 208 * Cr + 34784) >> 8)
		//  B = clip(( 298 * Y + 516 * Cb            - 70688) >> 8)
		//
		//  Define common = 298 * y - 56992. This will save one addition
		//
		//  R = clip(( common            + 409 * Cr -     0) >> 8)
		//  G = clip(( common - 100 * Cb - 208 * Cr + 91776) >> 8)
		//  B = clip(( common + 516 * Cb            - 13696) >> 8)
		//

		u8 y = ycc;
		u8 cb = ycc >> 8;
		u8 cr = ycc >> 16;

		u32 common = 298 * y - 56992;
		u32 r = (common +            409 * cr);
		u32 g = (common - 100 * cb - 208 * cr + 91776);
		u32 b = (common + 516 * cb - 13696);

		// Now clamp and shift back
		return rgb_t(clamp16_shift8(r), clamp16_shift8(g), clamp16_shift8(b));
	}


	//-------------------------------------------------
	//  get_texel_palette16 - return a texel from a
	//  palettized 16bpp source
	//-------------------------------------------------

	static inline u32 get_texel_palette16(const render_texinfo &texture, s32 curu, s32 curv)
	{
		const rgb_t *palbase = texture.palette;
		if (_BilinearFilter)
		{
			s32 u0 = curu >> 16;
			s32 u1 = 1;
			if (u0 < 0) u0 = u1 = 0;
			else if (u0 + 1 >= texture.width) u0 = texture.width - 1, u1 = 0;
			s32 v0 = curv >> 16;
			s32 v1 = texture.rowpixels;
			if (v0 < 0) v0 = v1 = 0;
			else if (v0 + 1 >= texture.height) v0 = texture.height - 1, v1 = 0;

			const u16 *texbase = reinterpret_cast<const u16 *>(texture.base);
			texbase += v0 * texture.rowpixels + u0;

			u32 pix00 = palbase[texbase[0]];
			u32 pix01 = palbase[texbase[u1]];
			u32 pix10 = palbase[texbase[v1]];
			u32 pix11 = palbase[texbase[u1 + v1]];
			return rgbaint_t::bilinear_filter(pix00, pix01, pix10, pix11, curu >> 8, curv >> 8);
		}
		else
		{
			const u16 *texbase = reinterpret_cast<const u16 *>(texture.base) + (curv >> 16) * texture.rowpixels + (curu >> 16);
			return palbase[texbase[0]];
		}
	}


	//-------------------------------------------------
	//  get_texel_palette16a - return a texel from a
	//  palettized 16bpp source with alpha
	//-------------------------------------------------

	static inline u32 get_texel_palette16a(const render_texinfo &texture, s32 curu, s32 curv)
	{
		const rgb_t *palbase = texture.palette;
		if (_BilinearFilter)
		{
			s32 u0 = curu >> 16;
			s32 u1 = 1;
			if (u0 < 0) u0 = u1 = 0;
			else if (u0 + 1 >= texture.width) u0 = texture.width - 1, u1 = 0;
			s32 v0 = curv >> 16;
			s32 v1 = texture.rowpixels;
			if (v0 < 0) v0 = v1 = 0;
			else if (v0 + 1 >= texture.height) v0 = texture.height - 1, v1 = 0;

			const u16 *texbase = reinterpret_cast<const u16 *>(texture.base);
			texbase += v0 * texture.rowpixels + u0;

			return rgbaint_t::bilinear_filter(palbase[texbase[0]], palbase[texbase[u1]], palbase[texbase[v1]], palbase[texbase[u1 + v1]], curu >> 8, curv >> 8);
		}
		else
		{
			const u16 *texbase = reinterpret_cast<const u16 *>(texture.base) + (curv >> 16) * texture.rowpixels + (curu >> 16);
			return palbase[texbase[0]];
		}
	}


	//-------------------------------------------------
	//  get_texel_yuy16 - return a texel from a 16bpp
	//  YCbCr source (pixel is returned as Cr-Cb-Y)
	//-------------------------------------------------

	static inline u32 get_texel_yuy16(const render_texinfo &texture, s32 curu, s32 curv)
	{
		if (_BilinearFilter)
		{
			s32 u0 = curu >> 16;
			s32 u1 = 1;
			if (u0 < 0) u0 = u1 = 0;
			else if (u0 + 1 >= texture.width) u0 = texture.width - 1, u1 = 0;
			s32 v0 = curv >> 16;
			s32 v1 = texture.rowpixels;
			if (v0 < 0) v0 = v1 = 0;
			else if (v0 + 1 >= texture.height) v0 = texture.height - 1, v1 = 0;

			const u16 *texbase = reinterpret_cast<const u16 *>(texture.base);
			texbase += v0 * texture.rowpixels + (u0 & ~1);

			u32 pix00, pix01, pix10, pix11;
			if ((curu & 0x10000) == 0)
			{
				u32 cbcr = ((texbase[0] & 0xff) << 8) | ((texbase[1] & 0xff) << 16);
				pix00 = (texbase[0] >> 8) | cbcr;
				pix01 = (texbase[u1] >> 8) | cbcr;
				cbcr = ((texbase[v1 + 0] & 0xff) << 8) | ((texbase[v1 + 1] & 0xff) << 16);
				pix10 = (texbase[v1 + 0] >> 8) | cbcr;
				pix11 = (texbase[v1 + u1] >> 8) | cbcr;
			}
			else
			{
				u32 cbcr = ((texbase[0] & 0xff) << 8) | ((texbase[1] & 0xff) << 16);
				pix00 = (texbase[1] >> 8) | cbcr;
				if (u1 != 0)
				{
					cbcr = ((texbase[2] & 0xff) << 8) | ((texbase[3] & 0xff) << 16);
					pix01 = (texbase[2] >> 8) | cbcr;
				}
				else
					pix01 = pix00;
				cbcr = ((texbase[v1 + 0] & 0xff) << 8) | ((texbase[v1 + 1] & 0xff) << 16);
				pix10 = (texbase[v1 + 1] >> 8) | cbcr;
				if (u1 != 0)
				{
					cbcr = ((texbase[v1 + 2] & 0xff) << 8) | ((texbase[v1 + 3] & 0xff) << 16);
					pix11 = (texbase[v1 + 2] >> 8) | cbcr;
				}
				else
					pix11 = pix10;
			}
			return rgbaint_t::bilinear_filter(pix00, pix01, pix10, pix11, curu >> 8, curv >> 8);
		}
		else
		{
			const u16 *texbase = reinterpret_cast<const u16 *>(texture.base) + (curv >> 16) * texture.rowpixels + (curu >> 17) * 2;
			return (texbase[(curu >> 16) & 1] >> 8) | ((texbase[0] & 0xff) << 8) | ((texbase[1] & 0xff) << 16);
		}
	}


	//-------------------------------------------------
	//  get_texel_rgb32 - return a texel from a 32bpp
	//  RGB source
	//-------------------------------------------------

	static inline u32 get_texel_rgb32(const render_texinfo &texture, s32 curu, s32 curv)
	{
		if (_BilinearFilter)
		{
			s32 u0 = curu >> 16;
			s32 u1 = 1;
			if (u0 < 0) u0 = u1 = 0;
			else if (u0 + 1 >= texture.width) u0 = texture.width - 1, u1 = 0;
			s32 v0 = curv >> 16;
			s32 v1 = texture.rowpixels;
			if (v0 < 0) v0 = v1 = 0;
			else if (v0 + 1 >= texture.height) v0 = texture.height - 1, v1 = 0;

			const u32 *texbase = reinterpret_cast<const u32 *>(texture.base);
			texbase += v0 * texture.rowpixels + u0;

			return rgbaint_t::bilinear_filter(texbase[0], texbase[u1], texbase[v1], texbase[u1 + v1], curu >> 8, curv >> 8);
		}
		else
		{
			const u32 *texbase = reinterpret_cast<const u32 *>(texture.base) + (curv >> 16) * texture.rowpixels + (curu >> 16);
			return texbase[0];
		}
	}


	//-------------------------------------------------
	//  get_texel_argb32 - return a texel from a 32bpp
	//  ARGB source
	//-------------------------------------------------

	static inline u32 get_texel_argb32(const render_texinfo &texture, s32 curu, s32 curv)
	{
		if (_BilinearFilter)
		{
			s32 u0 = curu >> 16;
			s32 u1 = 1;
			if (u0 < 0) u0 = u1 = 0;
			else if (u0 + 1 >= texture.width) u0 = texture.width - 1, u1 = 0;
			s32 v0 = curv >> 16;
			s32 v1 = texture.rowpixels;
			if (v0 < 0) v0 = v1 = 0;
			else if (v0 + 1 >= texture.height) v0 = texture.height - 1, v1 = 0;

			const u32 *texbase = reinterpret_cast<const u32 *>(texture.base);
			texbase += v0 * texture.rowpixels + u0;

			return rgbaint_t::bilinear_filter(texbase[0], texbase[u1], texbase[v1], texbase[u1 + v1], curu >> 8, curv >> 8);
		}
		else
		{
			const u32 *texbase = reinterpret_cast<const u32 *>(texture.base) + (curv >> 16) * texture.rowpixels + (curu >> 16);
			return texbase[0];
		}
	}


	//-------------------------------------------------
	//  draw_aa_pixel - draw an antialiased pixel
	//-------------------------------------------------

	static inline void draw_aa_pixel(_PixelType *dstdata, u32 pitch, int x, int y, u32 col)
	{
		_PixelType *dest = dstdata + y * pitch + x;
		u32 dpix = _NoDestRead ? 0 : *dest;
		u32 dr = source32_r(col) + dest_r(dpix);
		u32 dg = source32_g(col) + dest_g(dpix);
		u32 db = source32_b(col) + dest_b(dpix);
		dr = (dr | -(dr >> (8 - _SrcShiftR))) & (0xff >> _SrcShiftR);
		dg = (dg | -(dg >> (8 - _SrcShiftG))) & (0xff >> _SrcShiftG);
		db = (db | -(db >> (8 - _SrcShiftB))) & (0xff >> _SrcShiftB);
		*dest = dest_assemble_rgb(dr, dg, db);
	}


	//-------------------------------------------------
	//  draw_line - draw a line or point
	//-------------------------------------------------

	static void draw_line(const render_primitive &prim, _PixelType *dstdata, s32 width, s32 height, u32 pitch)
	{
		// internal tables
		static u32 s_cosine_table[2049];

		// compute the start/end coordinates
		int x1 = int(prim.bounds.x0 * 65536.0f);
		int y1 = int(prim.bounds.y0 * 65536.0f);
		int x2 = int(prim.bounds.x1 * 65536.0f);
		int y2 = int(prim.bounds.y1 * 65536.0f);

		// handle color and intensity
		u32 col = rgb_t(int(255.0f * prim.color.r * prim.color.a), int(255.0f * prim.color.g * prim.color.a), int(255.0f * prim.color.b * prim.color.a));

		if (PRIMFLAG_GET_ANTIALIAS(prim.flags))
		{
			// build up the cosine table if we haven't yet
			if (s_cosine_table[0] == 0)
				for (int entry = 0; entry <= 2048; entry++)
					s_cosine_table[entry] = int(double(1.0 / cos(atan(double(entry) / 2048.0))) * 0x10000000 + 0.5);

			int beam = prim.width * 65536.0f;
			if (beam < 0x00010000)
				beam = 0x00010000;

			// draw an anti-aliased line
			int dx = abs(x1 - x2);
			int dy = abs(y1 - y2);

			if (dx >= dy)
			{
				int sx = ((x1 <= x2) ? 1 : -1);
				int sy = (dy == 0) ? 0 : div_32x32_shift(y2 - y1, dx, 16);
				if (sy < 0)
					dy--;
				x1 >>= 16;
				int xx = x2 >> 16;
				int bwidth = mul_32x32_hi(beam << 4, s_cosine_table[abs(sy) >> 5]);
				y1 -= bwidth >> 1; // start back half the diameter
				for (;;)
				{
					if (x1 >= 0 && x1 < width)
					{
						dx = bwidth;    // init diameter of beam
						dy = y1 >> 16;
						if (dy >= 0 && dy < height)
							draw_aa_pixel(dstdata, pitch, x1, dy, apply_intensity(0xff & (~y1 >> 8), col));
						dy++;
						dx -= 0x10000 - (0xffff & y1); // take off amount plotted
						u8 a1 = (dx >> 8) & 0xff;   // calc remainder pixel
						dx >>= 16;                   // adjust to pixel (solid) count
						while (dx--)                 // plot rest of pixels
						{
							if (dy >= 0 && dy < height)
								draw_aa_pixel(dstdata, pitch, x1, dy, col);
							dy++;
						}
						if (dy >= 0 && dy < height)
							draw_aa_pixel(dstdata, pitch, x1, dy, apply_intensity(a1,col));
					}
					if (x1 == xx) break;
					x1 += sx;
					y1 += sy;
				}
			}
			else
			{
				int sy = ((y1 <= y2) ? 1: -1);
				int sx = (dx == 0) ? 0 : div_32x32_shift(x2 - x1, dy, 16);
				if (sx < 0)
					dx--;
				y1 >>= 16;
				int yy = y2 >> 16;
				int bwidth = mul_32x32_hi(beam << 4,s_cosine_table[abs(sx) >> 5]);
				x1 -= bwidth >> 1; // start back half the width
				for (;;)
				{
					if (y1 >= 0 && y1 < height)
					{
						dy = bwidth;    // calc diameter of beam
						dx = x1 >> 16;
						if (dx >= 0 && dx < width)
							draw_aa_pixel(dstdata, pitch, dx, y1, apply_intensity(0xff & (~x1 >> 8), col));
						dx++;
						dy -= 0x10000 - (0xffff & x1); // take off amount plotted
						u8 a1 = (dy >> 8) & 0xff;   // remainder pixel
						dy >>= 16;                   // adjust to pixel (solid) count
						while (dy--)                 // plot rest of pixels
						{
							if (dx >= 0 && dx < width)
								draw_aa_pixel(dstdata, pitch, dx, y1, col);
							dx++;
						}
						if (dx >= 0 && dx < width)
							draw_aa_pixel(dstdata, pitch, dx, y1, apply_intensity(a1, col));
					}
					if (y1 == yy) break;
					y1 += sy;
					x1 += sx;
				}
			}
		}
		else // use good old Bresenham for non-antialiasing 980317 BW
		{
			x1 = (x1 + 0x8000) >> 16;
			y1 = (y1 + 0x8000) >> 16;
			x2 = (x2 + 0x8000) >> 16;
			y2 = (y2 + 0x8000) >> 16;

			int dx = abs(x1 - x2);
			int dy = abs(y1 - y2);
			int sx = (x1 <= x2) ? 1 : -1;
			int sy = (y1 <= y2) ? 1 : -1;
			int cx = dx / 2;
			int cy = dy / 2;

			if (dx >= dy)
			{
				for (;;)
				{
					if (x1 >= 0 && x1 < width && y1 >= 0 && y1 < height)
						draw_aa_pixel(dstdata, pitch, x1, y1, col);
					if (x1 == x2) break;
					x1 += sx;
					cx -= dy;
					if (cx < 0)
					{
						y1 += sy;
						cx += dx;
					}
				}
			}
			else
			{
				for (;;)
				{
					if (x1 >= 0 && x1 < width && y1 >= 0 && y1 < height)
						draw_aa_pixel(dstdata, pitch, x1, y1, col);
					if (y1 == y2) break;
					y1 += sy;
					cy -= dx;
					if (cy < 0)
					{
						x1 += sx;
						cy += dy;
					}
				}
			}
		}
	}



	//**************************************************************************
	//  RECT RASTERIZERS
	//**************************************************************************

	//-------------------------------------------------
	//  draw_rect - draw a solid rectangle
	//-------------------------------------------------

	static void draw_rect(const render_primitive &prim, _PixelType *dstdata, s32 width, s32 height, u32 pitch)
	{
		render_bounds fpos = prim.bounds;
		assert(fpos.x0 <= fpos.x1);
		assert(fpos.y0 <= fpos.y1);

		// clamp to integers
		s32 startx = round_nearest(fpos.x0);
		s32 starty = round_nearest(fpos.y0);
		s32 endx = round_nearest(fpos.x1);
		s32 endy = round_nearest(fpos.y1);

		// ensure we fit
		if (startx < 0) startx = 0;
		if (startx >= width) startx = width;
		if (endx < 0) endx = 0;
		if (endx >= width) endx = width;
		if (starty < 0) starty = 0;
		if (starty >= height) starty = height;
		if (endy < 0) endy = 0;
		if (endy >= height) endy = height;

		// bail if nothing left
		if (fpos.x0 > fpos.x1 || fpos.y0 > fpos.y1)
			return;

		// only support alpha and "none" blendmodes
		assert(PRIMFLAG_GET_BLENDMODE(prim.flags) == BLENDMODE_NONE ||
				PRIMFLAG_GET_BLENDMODE(prim.flags) == BLENDMODE_ALPHA);

		// fast case: no alpha
		if (PRIMFLAG_GET_BLENDMODE(prim.flags) == BLENDMODE_NONE || is_opaque(prim.color.a))
		{
			u32 r = u32(256.0f * prim.color.r);
			u32 g = u32(256.0f * prim.color.g);
			u32 b = u32(256.0f * prim.color.b);
			u32 pix;

			// clamp R,G,B to 0-256 range
			if (r > 0xff) { if (s32(r) < 0) r = 0; else r = 0xff; }
			if (g > 0xff) { if (s32(g) < 0) g = 0; else g = 0xff; }
			if (b > 0xff) { if (s32(b) < 0) b = 0; else b = 0xff; }
			pix = dest_rgb_to_pixel(r, g, b);

			// loop over rows
			for (s32 y = starty; y < endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + startx;

				// loop over cols
				for (s32 x = startx; x < endx; x++)
					*dest++ = pix;
			}
		}

		// alpha and/or coloring case
		else if (!is_transparent(prim.color.a))
		{
			u32 rmask = dest_rgb_to_pixel(0xff,0x00,0x00);
			u32 gmask = dest_rgb_to_pixel(0x00,0xff,0x00);
			u32 bmask = dest_rgb_to_pixel(0x00,0x00,0xff);
			u32 r = u32(256.0f * prim.color.r * prim.color.a);
			u32 g = u32(256.0f * prim.color.g * prim.color.a);
			u32 b = u32(256.0f * prim.color.b * prim.color.a);
			u32 inva = u32(256.0f * (1.0f - prim.color.a));

			// clamp R,G,B and inverse A to 0-256 range
			if (r > 0xff) { if (s32(r) < 0) r = 0; else r = 0xff; }
			if (g > 0xff) { if (s32(g) < 0) g = 0; else g = 0xff; }
			if (b > 0xff) { if (s32(b) < 0) b = 0; else b = 0xff; }
			if (inva > 0x100) { if (s32(inva) < 0) inva = 0; else inva = 0x100; }

			// pre-shift the RGBA pieces
			r = dest_rgb_to_pixel(r, 0, 0) << 8;
			g = dest_rgb_to_pixel(0, g, 0) << 8;
			b = dest_rgb_to_pixel(0, 0, b) << 8;

			// loop over rows
			for (s32 y = starty; y < endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + startx;

				// loop over cols
				for (s32 x = startx; x < endx; x++)
				{
					u32 dpix = _NoDestRead ? 0 : *dest;
					u32 dr = (r + ((dpix & rmask) * inva)) & (rmask << 8);
					u32 dg = (g + ((dpix & gmask) * inva)) & (gmask << 8);
					u32 db = (b + ((dpix & bmask) * inva)) & (bmask << 8);
					*dest++ = (dr | dg | db) >> 8;
				}
			}
		}
	}


	//**************************************************************************
	//  16-BIT PALETTE RASTERIZERS
	//**************************************************************************

	//-------------------------------------------------
	//  draw_quad_palette16_none - perform
	//  rasterization of a 16bpp palettized texture
	//-------------------------------------------------

	static void draw_quad_palette16_none(const render_primitive &prim, _PixelType *dstdata, u32 pitch, quad_setup_data &setup)
	{
		s32 dudx = setup.dudx;
		s32 dvdx = setup.dvdx;
		s32 endx = setup.endx;

		// ensure all parameters are valid
		assert(prim.texture.palette != nullptr);

		// fast case: no coloring, no alpha
		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < endx; x++)
				{
					u32 pix = get_texel_palette16(prim.texture, curu, curv);
					*dest++ = source32_to_dest(pix);
					curu += dudx;
					curv += dvdx;
				}
			}
		}

		// coloring-only case
		else if (is_opaque(prim.color.a))
		{
			u32 sr = u32(256.0f * prim.color.r);
			u32 sg = u32(256.0f * prim.color.g);
			u32 sb = u32(256.0f * prim.color.b);

			// clamp R,G,B to 0-256 range
			if (sr > 0x100) { if (s32(sr) < 0) sr = 0; else sr = 0x100; }
			if (sg > 0x100) { if (s32(sg) < 0) sg = 0; else sg = 0x100; }
			if (sb > 0x100) { if (s32(sb) < 0) sb = 0; else sb = 0x100; }

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < endx; x++)
				{
					u32 pix = get_texel_palette16(prim.texture, curu, curv);
					u32 r = (source32_r(pix) * sr) >> 8;
					u32 g = (source32_g(pix) * sg) >> 8;
					u32 b = (source32_b(pix) * sb) >> 8;

					*dest++ = dest_assemble_rgb(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}

		// alpha and/or coloring case
		else if (!is_transparent(prim.color.a))
		{
			u32 sr = u32(256.0f * prim.color.r * prim.color.a);
			u32 sg = u32(256.0f * prim.color.g * prim.color.a);
			u32 sb = u32(256.0f * prim.color.b * prim.color.a);
			u32 invsa = u32(256.0f * (1.0f - prim.color.a));

			// clamp R,G,B and inverse A to 0-256 range
			if (sr > 0x100) { if (s32(sr) < 0) sr = 0; else sr = 0x100; }
			if (sg > 0x100) { if (s32(sg) < 0) sg = 0; else sg = 0x100; }
			if (sb > 0x100) { if (s32(sb) < 0) sb = 0; else sb = 0x100; }
			if (invsa > 0x100) { if (s32(invsa) < 0) invsa = 0; else invsa = 0x100; }

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < endx; x++)
				{
					u32 pix = get_texel_palette16(prim.texture, curu, curv);
					u32 dpix = _NoDestRead ? 0 : *dest;
					u32 r = (source32_r(pix) * sr + dest_r(dpix) * invsa) >> 8;
					u32 g = (source32_g(pix) * sg + dest_g(dpix) * invsa) >> 8;
					u32 b = (source32_b(pix) * sb + dest_b(dpix) * invsa) >> 8;

					*dest++ = dest_assemble_rgb(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}


	//-------------------------------------------------
	//  draw_quad_palette16_add - perform
	//  rasterization of a 16bpp palettized texture
	//-------------------------------------------------

	static void draw_quad_palette16_add(const render_primitive &prim, _PixelType *dstdata, u32 pitch, quad_setup_data &setup)
	{
		s32 dudx = setup.dudx;
		s32 dvdx = setup.dvdx;
		s32 endx = setup.endx;

		// ensure all parameters are valid
		assert(prim.texture.palette != nullptr);

		// fast case: no coloring, no alpha
		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < endx; x++)
				{
					u32 pix = get_texel_palette16(prim.texture, curu, curv);
					if ((pix & 0xffffff) != 0)
					{
						u32 dpix = _NoDestRead ? 0 : *dest;
						u32 r = source32_r(pix) + dest_r(dpix);
						u32 g = source32_g(pix) + dest_g(dpix);
						u32 b = source32_b(pix) + dest_b(dpix);
						r = (r | -(r >> (8 - _SrcShiftR))) & (0xff >> _SrcShiftR);
						g = (g | -(g >> (8 - _SrcShiftG))) & (0xff >> _SrcShiftG);
						b = (b | -(b >> (8 - _SrcShiftB))) & (0xff >> _SrcShiftB);
						*dest = dest_assemble_rgb(r, g, b);
					}
					dest++;
					curu += dudx;
					curv += dvdx;
				}
			}
		}

		// alpha and/or coloring case
		else
		{
			u32 sr = u32(256.0f * prim.color.r * prim.color.a);
			u32 sg = u32(256.0f * prim.color.g * prim.color.a);
			u32 sb = u32(256.0f * prim.color.b * prim.color.a);

			// clamp R,G,B and inverse A to 0-256 range
			if (sr > 0x100) { if (s32(sr) < 0) sr = 0; else sr = 0x100; }
			if (sg > 0x100) { if (s32(sg) < 0) sg = 0; else sg = 0x100; }
			if (sb > 0x100) { if (s32(sb) < 0) sb = 0; else sb = 0x100; }

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < endx; x++)
				{
					u32 pix = get_texel_palette16(prim.texture, curu, curv);
					if ((pix & 0xffffff) != 0)
					{
						u32 dpix = _NoDestRead ? 0 : *dest;
						u32 r = ((source32_r(pix) * sr) >> 8) + dest_r(dpix);
						u32 g = ((source32_g(pix) * sg) >> 8) + dest_g(dpix);
						u32 b = ((source32_b(pix) * sb) >> 8) + dest_b(dpix);
						r = (r | -(r >> (8 - _SrcShiftR))) & (0xff >> _SrcShiftR);
						g = (g | -(g >> (8 - _SrcShiftG))) & (0xff >> _SrcShiftG);
						b = (b | -(b >> (8 - _SrcShiftB))) & (0xff >> _SrcShiftB);
						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}
			}
		}
	}



	//**************************************************************************
	//  16-BIT YUY RASTERIZERS
	//**************************************************************************

	//-------------------------------------------------
	//  draw_quad_yuy16_none - perform
	//  rasterization of a 16bpp YUY image
	//-------------------------------------------------

	static void draw_quad_yuy16_none(const render_primitive &prim, _PixelType *dstdata, u32 pitch, quad_setup_data &setup)
	{
		s32 dudx = setup.dudx;
		s32 dvdx = setup.dvdx;
		s32 endx = setup.endx;

		// fast case: no coloring, no alpha
		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < endx; x++)
				{
					u32 pix = ycc_to_rgb(get_texel_yuy16(prim.texture, curu, curv));
					*dest++ = source32_to_dest(pix);
					curu += dudx;
					curv += dvdx;
				}
			}
		}

		// coloring-only case
		else if (is_opaque(prim.color.a))
		{
			u32 sr = u32(256.0f * prim.color.r);
			u32 sg = u32(256.0f * prim.color.g);
			u32 sb = u32(256.0f * prim.color.b);

			// clamp R,G,B to 0-256 range
			if (sr > 0x100) { if (s32(sr) < 0) sr = 0; else sr = 0x100; }
			if (sg > 0x100) { if (s32(sg) < 0) sg = 0; else sg = 0x100; }
			if (sb > 0x100) { if (s32(sb) < 0) sb = 0; else sb = 0x100; }

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < endx; x++)
				{
					u32 pix = ycc_to_rgb(get_texel_yuy16(prim.texture, curu, curv));
					u32 r = (source32_r(pix) * sr) >> 8;
					u32 g = (source32_g(pix) * sg) >> 8;
					u32 b = (source32_b(pix) * sb) >> 8;

					*dest++ = dest_assemble_rgb(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}

		// alpha and/or coloring case
		else if (!is_transparent(prim.color.a))
		{
			u32 sr = u32(256.0f * prim.color.r * prim.color.a);
			u32 sg = u32(256.0f * prim.color.g * prim.color.a);
			u32 sb = u32(256.0f * prim.color.b * prim.color.a);
			u32 invsa = u32(256.0f * (1.0f - prim.color.a));

			// clamp R,G,B and inverse A to 0-256 range
			if (sr > 0x100) { if (s32(sr) < 0) sr = 0; else sr = 0x100; }
			if (sg > 0x100) { if (s32(sg) < 0) sg = 0; else sg = 0x100; }
			if (sb > 0x100) { if (s32(sb) < 0) sb = 0; else sb = 0x100; }
			if (invsa > 0x100) { if (s32(invsa) < 0) invsa = 0; else invsa = 0x100; }

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < endx; x++)
				{
					u32 pix = ycc_to_rgb(get_texel_yuy16(prim.texture, curu, curv));
					u32 dpix = _NoDestRead ? 0 : *dest;
					u32 r = (source32_r(pix) * sr + dest_r(dpix) * invsa) >> 8;
					u32 g = (source32_g(pix) * sg + dest_g(dpix) * invsa) >> 8;
					u32 b = (source32_b(pix) * sb + dest_b(dpix) * invsa) >> 8;

					*dest++ = dest_assemble_rgb(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}


	//-------------------------------------------------
	//  draw_quad_yuy16_add - perform
	//  rasterization by using RGB add after YUY
	//  conversion
	//-------------------------------------------------

	static void draw_quad_yuy16_add(const render_primitive &prim, _PixelType *dstdata, u32 pitch, quad_setup_data &setup)
	{
		s32 dudx = setup.dudx;
		s32 dvdx = setup.dvdx;
		s32 endx = setup.endx;

		// simply can't do this without reading from the dest
		if (_NoDestRead)
			return;

		// fast case: no coloring, no alpha
		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < endx; x++)
				{
					u32 pix = ycc_to_rgb(get_texel_yuy16(prim.texture, curu, curv));
					u32 dpix = _NoDestRead ? 0 : *dest;
					u32 r = source32_r(pix) + dest_r(dpix);
					u32 g = source32_g(pix) + dest_g(dpix);
					u32 b = source32_b(pix) + dest_b(dpix);
					r = (r | -(r >> (8 - _SrcShiftR))) & (0xff >> _SrcShiftR);
					g = (g | -(g >> (8 - _SrcShiftG))) & (0xff >> _SrcShiftG);
					b = (b | -(b >> (8 - _SrcShiftB))) & (0xff >> _SrcShiftB);
					*dest++ = dest_assemble_rgb(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}

		// alpha and/or coloring case
		else
		{
			u32 sr = u32(256.0f * prim.color.r);
			u32 sg = u32(256.0f * prim.color.g);
			u32 sb = u32(256.0f * prim.color.b);
			u32 sa = u32(256.0f * prim.color.a);

			// clamp R,G,B and inverse A to 0-256 range
			if (sr > 0x100) { if (s32(sr) < 0) sr = 0; else sr = 0x100; }
			if (sg > 0x100) { if (s32(sg) < 0) sg = 0; else sg = 0x100; }
			if (sb > 0x100) { if (s32(sb) < 0) sb = 0; else sb = 0x100; }
			if (sa > 0x100) { if (s32(sa) < 0) sa = 0; else sa = 0x100; }

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < endx; x++)
				{
					u32 pix = ycc_to_rgb(get_texel_yuy16(prim.texture, curu, curv));
					u32 dpix = _NoDestRead ? 0 : *dest;
					u32 r = ((source32_r(pix) * sr * sa) >> 16) + dest_r(dpix);
					u32 g = ((source32_g(pix) * sg * sa) >> 16) + dest_g(dpix);
					u32 b = ((source32_b(pix) * sb * sa) >> 16) + dest_b(dpix);
					r = (r | -(r >> (8 - _SrcShiftR))) & (0xff >> _SrcShiftR);
					g = (g | -(g >> (8 - _SrcShiftG))) & (0xff >> _SrcShiftG);
					b = (b | -(b >> (8 - _SrcShiftB))) & (0xff >> _SrcShiftB);
					*dest++ = dest_assemble_rgb(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}


	//**************************************************************************
	//  32-BIT RGB QUAD RASTERIZERS
	//**************************************************************************

	//-------------------------------------------------
	//  draw_quad_rgb32 - perform rasterization of
	//  a 32bpp RGB texture
	//-------------------------------------------------

	static void draw_quad_rgb32(const render_primitive &prim, _PixelType *dstdata, u32 pitch, quad_setup_data &setup)
	{
		const rgb_t *palbase = prim.texture.palette;
		s32 dudx = setup.dudx;
		s32 dvdx = setup.dvdx;
		s32 endx = setup.endx;

		// fast case: no coloring, no alpha
		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// no lookup case
				if (palbase == nullptr)
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_rgb32(prim.texture, curu, curv);
						*dest++ = source32_to_dest(pix);
						curu += dudx;
						curv += dvdx;
					}
				}

				// lookup case
				else
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_rgb32(prim.texture, curu, curv);
						u32 r = palbase[(pix >> 16) & 0xff] >> _SrcShiftR;
						u32 g = palbase[(pix >> 8) & 0xff] >> _SrcShiftG;
						u32 b = palbase[(pix >> 0) & 0xff] >> _SrcShiftB;

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}
			}
		}

		// coloring-only case
		else if (is_opaque(prim.color.a))
		{
			u32 sr = u32(256.0f * prim.color.r);
			u32 sg = u32(256.0f * prim.color.g);
			u32 sb = u32(256.0f * prim.color.b);

			// clamp R,G,B to 0-256 range
			if (sr > 0x100) { if (s32(sr) < 0) sr = 0; else sr = 0x100; }
			if (sg > 0x100) { if (s32(sg) < 0) sg = 0; else sg = 0x100; }
			if (sb > 0x100) { if (s32(sb) < 0) sb = 0; else sb = 0x100; }

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// no lookup case
				if (palbase == nullptr)
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_rgb32(prim.texture, curu, curv);
						u32 r = (source32_r(pix) * sr) >> 8;
						u32 g = (source32_g(pix) * sg) >> 8;
						u32 b = (source32_b(pix) * sb) >> 8;

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}

				// lookup case
				else
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_rgb32(prim.texture, curu, curv);
						u32 r = (palbase[(pix >> 16) & 0xff] * sr) >> (8 + _SrcShiftR);
						u32 g = (palbase[(pix >> 8) & 0xff] * sg) >> (8 + _SrcShiftG);
						u32 b = (palbase[(pix >> 0) & 0xff] * sb) >> (8 + _SrcShiftB);

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}
			}
		}

		// alpha and/or coloring case
		else if (!is_transparent(prim.color.a))
		{
			u32 sr = u32(256.0f * prim.color.r * prim.color.a);
			u32 sg = u32(256.0f * prim.color.g * prim.color.a);
			u32 sb = u32(256.0f * prim.color.b * prim.color.a);
			u32 invsa = u32(256.0f * (1.0f - prim.color.a));

			// clamp R,G,B and inverse A to 0-256 range
			if (sr > 0x100) { if (s32(sr) < 0) sr = 0; else sr = 0x100; }
			if (sg > 0x100) { if (s32(sg) < 0) sg = 0; else sg = 0x100; }
			if (sb > 0x100) { if (s32(sb) < 0) sb = 0; else sb = 0x100; }
			if (invsa > 0x100) { if (s32(invsa) < 0) invsa = 0; else invsa = 0x100; }

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// no lookup case
				if (palbase == nullptr)
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_rgb32(prim.texture, curu, curv);
						u32 dpix = _NoDestRead ? 0 : *dest;
						u32 r = (source32_r(pix) * sr + dest_r(dpix) * invsa) >> 8;
						u32 g = (source32_g(pix) * sg + dest_g(dpix) * invsa) >> 8;
						u32 b = (source32_b(pix) * sb + dest_b(dpix) * invsa) >> 8;

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}

				// lookup case
				else
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_rgb32(prim.texture, curu, curv);
						u32 dpix = _NoDestRead ? 0 : *dest;
						u32 r = ((palbase[(pix >> 16) & 0xff] >> _SrcShiftR) * sr + dest_r(dpix) * invsa) >> 8;
						u32 g = ((palbase[(pix >> 8) & 0xff] >> _SrcShiftG) * sg + dest_g(dpix) * invsa) >> 8;
						u32 b = ((palbase[(pix >> 0) & 0xff] >> _SrcShiftB) * sb + dest_b(dpix) * invsa) >> 8;

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}
			}
		}
	}


	//-------------------------------------------------
	//  draw_quad_rgb32_add - perform
	//  rasterization by using RGB add
	//-------------------------------------------------

	static void draw_quad_rgb32_add(const render_primitive &prim, _PixelType *dstdata, u32 pitch, quad_setup_data &setup)
	{
		const rgb_t *palbase = prim.texture.palette;
		s32 dudx = setup.dudx;
		s32 dvdx = setup.dvdx;
		s32 endx = setup.endx;

		// simply can't do this without reading from the dest
		if (_NoDestRead)
			return;

		// fast case: no coloring, no alpha
		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// no lookup case
				if (palbase == nullptr)
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 dpix = _NoDestRead ? 0 : *dest;
						u32 r = source32_r(pix) + dest_r(dpix);
						u32 g = source32_g(pix) + dest_g(dpix);
						u32 b = source32_b(pix) + dest_b(dpix);
						r = (r | -(r >> (8 - _SrcShiftR))) & (0xff >> _SrcShiftR);
						g = (g | -(g >> (8 - _SrcShiftG))) & (0xff >> _SrcShiftG);
						b = (b | -(b >> (8 - _SrcShiftB))) & (0xff >> _SrcShiftB);
						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}

				// lookup case
				else
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 dpix = _NoDestRead ? 0 : *dest;
						u32 r = (palbase[(pix >> 16) & 0xff] >> _SrcShiftR) + dest_r(dpix);
						u32 g = (palbase[(pix >> 8) & 0xff] >> _SrcShiftG) + dest_g(dpix);
						u32 b = (palbase[(pix >> 0) & 0xff] >> _SrcShiftB) + dest_b(dpix);
						r = (r | -(r >> (8 - _SrcShiftR))) & (0xff >> _SrcShiftR);
						g = (g | -(g >> (8 - _SrcShiftG))) & (0xff >> _SrcShiftG);
						b = (b | -(b >> (8 - _SrcShiftB))) & (0xff >> _SrcShiftB);
						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}
			}
		}

		// alpha and/or coloring case
		else
		{
			u32 sr = u32(256.0f * prim.color.r);
			u32 sg = u32(256.0f * prim.color.g);
			u32 sb = u32(256.0f * prim.color.b);
			u32 sa = u32(256.0f * prim.color.a);

			// clamp R,G,B and inverse A to 0-256 range
			if (sr > 0x100) { if (s32(sr) < 0) sr = 0; else sr = 0x100; }
			if (sg > 0x100) { if (s32(sg) < 0) sg = 0; else sg = 0x100; }
			if (sb > 0x100) { if (s32(sb) < 0) sb = 0; else sb = 0x100; }
			if (sa > 0x100) { if (s32(sa) < 0) sa = 0; else sa = 0x100; }

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// no lookup case
				if (palbase == nullptr)
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 dpix = _NoDestRead ? 0 : *dest;
						u32 r = ((source32_r(pix) * sr * sa) >> 16) + dest_r(dpix);
						u32 g = ((source32_g(pix) * sg * sa) >> 16) + dest_g(dpix);
						u32 b = ((source32_b(pix) * sb * sa) >> 16) + dest_b(dpix);
						r = (r | -(r >> (8 - _SrcShiftR))) & (0xff >> _SrcShiftR);
						g = (g | -(g >> (8 - _SrcShiftG))) & (0xff >> _SrcShiftG);
						b = (b | -(b >> (8 - _SrcShiftB))) & (0xff >> _SrcShiftB);
						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}

				// lookup case
				else
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 dpix = _NoDestRead ? 0 : *dest;
						u32 r = ((palbase[(pix >> 16) & 0xff] * sr * sa) >> (16 + _SrcShiftR)) + dest_r(dpix);
						u32 g = ((palbase[(pix >> 8) & 0xff] * sr * sa) >> (16 + _SrcShiftR)) + dest_g(dpix);
						u32 b = ((palbase[(pix >> 0) & 0xff] * sr * sa) >> (16 + _SrcShiftR)) + dest_b(dpix);
						r = (r | -(r >> (8 - _SrcShiftR))) & (0xff >> _SrcShiftR);
						g = (g | -(g >> (8 - _SrcShiftG))) & (0xff >> _SrcShiftG);
						b = (b | -(b >> (8 - _SrcShiftB))) & (0xff >> _SrcShiftB);
						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}
			}
		}
	}


	//-------------------------------------------------
	//  draw_quad_rgb32_multiply - perform
	//  rasterization using RGB multiply
	//-------------------------------------------------

	static void draw_quad_rgb32_multiply(const render_primitive &prim, _PixelType *dstdata, u32 pitch, quad_setup_data &setup)
	{
		const rgb_t *palbase = prim.texture.palette;
		s32 dudx = setup.dudx;
		s32 dvdx = setup.dvdx;
		s32 endx = setup.endx;

		// simply can't do this without reading from the dest
		if (_NoDestRead)
			return;

		// fast case: no coloring, no alpha
		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// no lookup case
				if (palbase == nullptr)
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 dpix = _NoDestRead ? 0 : *dest;
						u32 r = (source32_r(pix) * dest_r(dpix)) >> (8 - _SrcShiftR);
						u32 g = (source32_g(pix) * dest_g(dpix)) >> (8 - _SrcShiftG);
						u32 b = (source32_b(pix) * dest_b(dpix)) >> (8 - _SrcShiftB);

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}
				else
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 dpix = _NoDestRead ? 0 : *dest;
						u32 r = (palbase[(pix >> 16) & 0xff] * dest_r(dpix)) >> 8;
						u32 g = (palbase[(pix >> 8) & 0xff] * dest_g(dpix)) >> 8;
						u32 b = (palbase[(pix >> 0) & 0xff] * dest_b(dpix)) >> 8;

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}
			}
		}

		// alpha and/or coloring case
		else
		{
			u32 sr = u32(256.0f * prim.color.r * prim.color.a);
			u32 sg = u32(256.0f * prim.color.g * prim.color.a);
			u32 sb = u32(256.0f * prim.color.b * prim.color.a);

			// clamp R,G,B to 0-256 range
			if (sr > 0x100) { if (s32(sr) < 0) sr = 0; else sr = 0x100; }
			if (sg > 0x100) { if (s32(sg) < 0) sg = 0; else sg = 0x100; }
			if (sb > 0x100) { if (s32(sb) < 0) sb = 0; else sb = 0x100; }

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// no lookup case
				if (palbase == nullptr)
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 dpix = _NoDestRead ? 0 : *dest;
						u32 r = (source32_r(pix) * sr * dest_r(dpix)) >> (16 - _SrcShiftR);
						u32 g = (source32_g(pix) * sg * dest_g(dpix)) >> (16 - _SrcShiftG);
						u32 b = (source32_b(pix) * sb * dest_b(dpix)) >> (16 - _SrcShiftB);

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}
				else
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 dpix = _NoDestRead ? 0 : *dest;
						u32 r = (palbase[(pix >> 16) & 0xff] * sr * dest_r(dpix)) >> 16;
						u32 g = (palbase[(pix >> 8) & 0xff] * sg * dest_g(dpix)) >> 16;
						u32 b = (palbase[(pix >> 0) & 0xff] * sb * dest_b(dpix)) >> 16;

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += dudx;
						curv += dvdx;
					}
				}
			}
		}
	}


	//**************************************************************************
	//  32-BIT ARGB QUAD RASTERIZERS
	//**************************************************************************

	//-------------------------------------------------
	//  draw_quad_argb32_alpha - perform
	//  rasterization using standard alpha blending
	//-------------------------------------------------

	static void draw_quad_argb32_alpha(const render_primitive &prim, _PixelType *dstdata, u32 pitch, quad_setup_data &setup)
	{
		const rgb_t *palbase = prim.texture.palette;
		s32 dudx = setup.dudx;
		s32 dvdx = setup.dvdx;
		s32 endx = setup.endx;

		// fast case: no coloring, no alpha
		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// no lookup case
				if (palbase == nullptr)
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 ta = pix >> 24;
						if (ta != 0)
						{
							u32 dpix = _NoDestRead ? 0 : *dest;
							u32 invta = 0x100 - ta;
							u32 r = (source32_r(pix) * ta + dest_r(dpix) * invta) >> 8;
							u32 g = (source32_g(pix) * ta + dest_g(dpix) * invta) >> 8;
							u32 b = (source32_b(pix) * ta + dest_b(dpix) * invta) >> 8;

							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += dudx;
						curv += dvdx;
					}
				}

				// lookup case
				else
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 ta = pix >> 24;
						if (ta != 0)
						{
							u32 dpix = _NoDestRead ? 0 : *dest;
							u32 invta = 0x100 - ta;
							u32 r = ((palbase[(pix >> 16) & 0xff] >> _SrcShiftR) * ta + dest_r(dpix) * invta) >> 8;
							u32 g = ((palbase[(pix >> 8) & 0xff] >> _SrcShiftG) * ta + dest_g(dpix) * invta) >> 8;
							u32 b = ((palbase[(pix >> 0) & 0xff] >> _SrcShiftB) * ta + dest_b(dpix) * invta) >> 8;

							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += dudx;
						curv += dvdx;
					}
				}
			}
		}

		// alpha and/or coloring case
		else
		{
			u32 sr = u32(256.0f * prim.color.r);
			u32 sg = u32(256.0f * prim.color.g);
			u32 sb = u32(256.0f * prim.color.b);
			u32 sa = u32(256.0f * prim.color.a);

			// clamp R,G,B and inverse A to 0-256 range
			if (sr > 0x100) { if (s32(sr) < 0) sr = 0; else sr = 0x100; }
			if (sg > 0x100) { if (s32(sg) < 0) sg = 0; else sg = 0x100; }
			if (sb > 0x100) { if (s32(sb) < 0) sb = 0; else sb = 0x100; }
			if (sa > 0x100) { if (s32(sa) < 0) sa = 0; else sa = 0x100; }

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// no lookup case
				if (palbase == nullptr)
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 ta = (pix >> 24) * sa;
						if (ta != 0)
						{
							u32 dpix = _NoDestRead ? 0 : *dest;
							u32 invsta = (0x10000 - ta) << 8;
							u32 r = (source32_r(pix) * sr * ta + dest_r(dpix) * invsta) >> 24;
							u32 g = (source32_g(pix) * sg * ta + dest_g(dpix) * invsta) >> 24;
							u32 b = (source32_b(pix) * sb * ta + dest_b(dpix) * invsta) >> 24;

							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += dudx;
						curv += dvdx;
					}
				}


				// lookup case
				else
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 ta = (pix >> 24) * sa;
						if (ta != 0)
						{
							u32 dpix = _NoDestRead ? 0 : *dest;
							u32 invsta = (0x10000 - ta) << 8;
							u32 r = ((palbase[(pix >> 16) & 0xff] >> _SrcShiftR) * sr * ta + dest_r(dpix) * invsta) >> 24;
							u32 g = ((palbase[(pix >> 8) & 0xff] >> _SrcShiftG) * sg * ta + dest_g(dpix) * invsta) >> 24;
							u32 b = ((palbase[(pix >> 0) & 0xff] >> _SrcShiftB) * sb * ta + dest_b(dpix) * invsta) >> 24;

							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += dudx;
						curv += dvdx;
					}
				}
			}
		}
	}


	//-------------------------------------------------
	//  draw_quad_argb32_add - perform
	//  rasterization by using RGB add
	//-------------------------------------------------

	static void draw_quad_argb32_add(const render_primitive &prim, _PixelType *dstdata, u32 pitch, quad_setup_data &setup)
	{
		const rgb_t *palbase = prim.texture.palette;
		s32 dudx = setup.dudx;
		s32 dvdx = setup.dvdx;
		s32 endx = setup.endx;

		// simply can't do this without reading from the dest
		if (_NoDestRead)
			return;

		// fast case: no coloring, no alpha
		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// no lookup case
				if (palbase == nullptr)
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 ta = pix >> 24;
						if (ta != 0)
						{
							u32 dpix = _NoDestRead ? 0 : *dest;
							u32 r = ((source32_r(pix) * ta) >> 8) + dest_r(dpix);
							u32 g = ((source32_g(pix) * ta) >> 8) + dest_g(dpix);
							u32 b = ((source32_b(pix) * ta) >> 8) + dest_b(dpix);
							r = (r | -(r >> (8 - _SrcShiftR))) & (0xff >> _SrcShiftR);
							g = (g | -(g >> (8 - _SrcShiftG))) & (0xff >> _SrcShiftG);
							b = (b | -(b >> (8 - _SrcShiftB))) & (0xff >> _SrcShiftB);
							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += dudx;
						curv += dvdx;
					}
				}

				// lookup case
				else
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 ta = pix >> 24;
						if (ta != 0)
						{
							u32 dpix = _NoDestRead ? 0 : *dest;
							u32 r = ((palbase[(pix >> 16) & 0xff] * ta) >> (8 + _SrcShiftR)) + dest_r(dpix);
							u32 g = ((palbase[(pix >> 8) & 0xff] * ta) >> (8 + _SrcShiftG)) + dest_g(dpix);
							u32 b = ((palbase[(pix >> 0) & 0xff] * ta) >> (8 + _SrcShiftB)) + dest_b(dpix);
							r = (r | -(r >> (8 - _SrcShiftR))) & (0xff >> _SrcShiftR);
							g = (g | -(g >> (8 - _SrcShiftG))) & (0xff >> _SrcShiftG);
							b = (b | -(b >> (8 - _SrcShiftB))) & (0xff >> _SrcShiftB);
							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += dudx;
						curv += dvdx;
					}
				}
			}
		}

		// alpha and/or coloring case
		else
		{
			u32 sr = u32(256.0f * prim.color.r);
			u32 sg = u32(256.0f * prim.color.g);
			u32 sb = u32(256.0f * prim.color.b);
			u32 sa = u32(256.0f * prim.color.a);

			// clamp R,G,B and inverse A to 0-256 range
			if (sr > 0x100) { if (s32(sr) < 0) sr = 0; else sr = 0x100; }
			if (sg > 0x100) { if (s32(sg) < 0) sg = 0; else sg = 0x100; }
			if (sb > 0x100) { if (s32(sb) < 0) sb = 0; else sb = 0x100; }
			if (sa > 0x100) { if (s32(sa) < 0) sa = 0; else sa = 0x100; }

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				_PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// no lookup case
				if (palbase == nullptr)
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 ta = (pix >> 24) * sa;
						if (ta != 0)
						{
							u32 dpix = _NoDestRead ? 0 : *dest;
							u32 r = ((source32_r(pix) * sr * ta) >> 24) + dest_r(dpix);
							u32 g = ((source32_g(pix) * sg * ta) >> 24) + dest_g(dpix);
							u32 b = ((source32_b(pix) * sb * ta) >> 24) + dest_b(dpix);
							r = (r | -(r >> (8 - _SrcShiftR))) & (0xff >> _SrcShiftR);
							g = (g | -(g >> (8 - _SrcShiftG))) & (0xff >> _SrcShiftG);
							b = (b | -(b >> (8 - _SrcShiftB))) & (0xff >> _SrcShiftB);
							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += dudx;
						curv += dvdx;
					}
				}

				// lookup case
				else
				{
					// loop over cols
					for (s32 x = setup.startx; x < endx; x++)
					{
						u32 pix = get_texel_argb32(prim.texture, curu, curv);
						u32 ta = (pix >> 24) * sa;
						if (ta != 0)
						{
							u32 dpix = _NoDestRead ? 0 : *dest;
							u32 r = ((palbase[(pix >> 16) & 0xff] * sr * ta) >> (24 + _SrcShiftR)) + dest_r(dpix);
							u32 g = ((palbase[(pix >> 8) & 0xff] * sr * ta) >> (24 + _SrcShiftR)) + dest_g(dpix);
							u32 b = ((palbase[(pix >> 0) & 0xff] * sr * ta) >> (24 + _SrcShiftR)) + dest_b(dpix);
							r = (r | -(r >> (8 - _SrcShiftR))) & (0xff >> _SrcShiftR);
							g = (g | -(g >> (8 - _SrcShiftG))) & (0xff >> _SrcShiftG);
							b = (b | -(b >> (8 - _SrcShiftB))) & (0xff >> _SrcShiftB);
							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += dudx;
						curv += dvdx;
					}
				}
			}
		}
	}


	//**************************************************************************
	//  CORE QUAD RASTERIZERS
	//**************************************************************************

	//-------------------------------------------------
	//  setup_and_draw_textured_quad - perform setup
	//  and then dispatch to a texture-mode-specific
	//  drawing routine
	//-------------------------------------------------

	static void setup_and_draw_textured_quad(const render_primitive &prim, _PixelType *dstdata, s32 width, s32 height, u32 pitch)
	{
		assert(prim.bounds.x0 <= prim.bounds.x1);
		assert(prim.bounds.y0 <= prim.bounds.y1);

		// determine U/V deltas
		float fdudx = (prim.texcoords.tr.u - prim.texcoords.tl.u) / (prim.bounds.x1 - prim.bounds.x0);
		float fdvdx = (prim.texcoords.tr.v - prim.texcoords.tl.v) / (prim.bounds.x1 - prim.bounds.x0);
		float fdudy = (prim.texcoords.bl.u - prim.texcoords.tl.u) / (prim.bounds.y1 - prim.bounds.y0);
		float fdvdy = (prim.texcoords.bl.v - prim.texcoords.tl.v) / (prim.bounds.y1 - prim.bounds.y0);

		// clamp to integers
		quad_setup_data setup;
		setup.startx = round_nearest(prim.bounds.x0);
		setup.starty = round_nearest(prim.bounds.y0);
		setup.endx = round_nearest(prim.bounds.x1);
		setup.endy = round_nearest(prim.bounds.y1);

		// ensure we fit
		if (setup.startx < 0) setup.startx = 0;
		if (setup.startx >= width) setup.startx = width;
		if (setup.endx < 0) setup.endx = 0;
		if (setup.endx >= width) setup.endx = width;
		if (setup.starty < 0) setup.starty = 0;
		if (setup.starty >= height) setup.starty = height;
		if (setup.endy < 0) setup.endy = 0;
		if (setup.endy >= height) setup.endy = height;

		// compute start and delta U,V coordinates now
		setup.dudx = round_nearest(65536.0f * float(prim.texture.width) * fdudx);
		setup.dvdx = round_nearest(65536.0f * float(prim.texture.height) * fdvdx);
		setup.dudy = round_nearest(65536.0f * float(prim.texture.width) * fdudy);
		setup.dvdy = round_nearest(65536.0f * float(prim.texture.height) * fdvdy);
		setup.startu = round_nearest(65536.0f * float(prim.texture.width) * prim.texcoords.tl.u);
		setup.startv = round_nearest(65536.0f * float(prim.texture.height) * prim.texcoords.tl.v);

		// advance U/V to the middle of the first texel
		setup.startu += (setup.dudx + setup.dudy) / 2;
		setup.startv += (setup.dvdx + setup.dvdy) / 2;

		// if we're bilinear filtering, we need to offset u/v by half a texel
		if (_BilinearFilter)
		{
			setup.startu -= 0x8000;
			setup.startv -= 0x8000;
		}

		// render based on the texture coordinates
		switch (prim.flags & (PRIMFLAG_TEXFORMAT_MASK | PRIMFLAG_BLENDMODE_MASK))
		{
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE):
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16) | PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA):
				draw_quad_palette16_none(prim, dstdata, pitch, setup);
				break;

			case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16) | PRIMFLAG_BLENDMODE(BLENDMODE_ADD):
				draw_quad_palette16_add(prim, dstdata, pitch, setup);
				break;

			case PRIMFLAG_TEXFORMAT(TEXFORMAT_YUY16) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE):
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_YUY16) | PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA):
				draw_quad_yuy16_none(prim, dstdata, pitch, setup);
				break;

			case PRIMFLAG_TEXFORMAT(TEXFORMAT_YUY16) | PRIMFLAG_BLENDMODE(BLENDMODE_ADD):
				draw_quad_yuy16_add(prim, dstdata, pitch, setup);
				break;

			case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE):
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA):
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE):
				draw_quad_rgb32(prim, dstdata, pitch, setup);
				break;

			case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_RGB_MULTIPLY):
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_RGB_MULTIPLY):
				draw_quad_rgb32_multiply(prim, dstdata, pitch, setup);
				break;

			case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_ADD):
				draw_quad_rgb32_add(prim, dstdata, pitch, setup);
				break;

			case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA):
				draw_quad_argb32_alpha(prim, dstdata, pitch, setup);
				break;

			case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_ADD):
				draw_quad_argb32_add(prim, dstdata, pitch, setup);
				break;

			default:
				fatalerror("Unknown texformat(%d)/blendmode(%d) combo\n", PRIMFLAG_GET_TEXFORMAT(prim.flags), PRIMFLAG_GET_BLENDMODE(prim.flags));
				break;
		}
	}


	//**************************************************************************
	//  PRIMARY ENTRY POINT
	//**************************************************************************

	//-------------------------------------------------
	//  draw_primitives - draw a series of primitives
	//  using a software rasterizer
	//-------------------------------------------------

public:
	static void draw_primitives(const render_primitive_list &primlist, void *dstdata, u32 width, u32 height, u32 pitch)
	{
		// loop over the list and render each element
		for (const render_primitive *prim = primlist.first(); prim != nullptr; prim = prim->next())
			switch (prim->type)
			{
				case render_primitive::LINE:
					draw_line(*prim, reinterpret_cast<_PixelType *>(dstdata), width, height, pitch);
					break;

				case render_primitive::QUAD:
					if (!prim->texture.base)
						draw_rect(*prim, reinterpret_cast<_PixelType *>(dstdata), width, height, pitch);
					else
						setup_and_draw_textured_quad(*prim, reinterpret_cast<_PixelType *>(dstdata), width, height, pitch);
					break;

				default:
					throw emu_fatalerror("Unexpected render_primitive type");
			}
	}
};
