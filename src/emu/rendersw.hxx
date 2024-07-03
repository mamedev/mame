// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    rendersw.hxx

    Software-only rasterization system.

***************************************************************************/


#include "emucore.h"
#include "eminline.h"
#include "video/rgbutil.h"
#include "render.h"

#include <array>

template <typename PixelType, int SrcShiftR, int SrcShiftG, int SrcShiftB, int DstShiftR, int DstShiftG, int DstShiftB, bool NoDestRead = false, bool BilinearFilter = false>
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
	template <int... Values>
	static auto make_cosine_table(std::integer_sequence<int, Values...>)
	{
		return std::array<u32, sizeof...(Values)>{ u32((1.0 / cos(atan(double(Values) / double(sizeof...(Values) - 1)))) * 0x10000000 + 0.5)... };
	}
	static constexpr bool is_opaque(float alpha) { return (alpha >= (NoDestRead ? 0.5f : 1.0f)); }
	static constexpr bool is_transparent(float alpha) { return (alpha < (NoDestRead ? 0.5f : 0.0001f)); }
	static rgb_t apply_intensity(int intensity, rgb_t color) { return color.scale8(intensity); }
	static float round_nearest(float f) { return floor(f + 0.5f); }

	// destination pixels are written based on the values of the template parameters
	static constexpr PixelType dest_assemble_rgb(u32 r, u32 g, u32 b) { return (r << DstShiftR) | (g << DstShiftG) | (b << DstShiftB); }
	static constexpr PixelType dest_rgb_to_pixel(u32 r, u32 g, u32 b) { return dest_assemble_rgb(r >> SrcShiftR, g >> SrcShiftG, b >> SrcShiftB); }

	// source 32-bit pixels are in MAME standardized format
	static constexpr u32 source32_r(u32 pixel) { return (pixel >> (16 + SrcShiftR)) & (0xff >> SrcShiftR); }
	static constexpr u32 source32_g(u32 pixel) { return (pixel >> ( 8 + SrcShiftG)) & (0xff >> SrcShiftG); }
	static constexpr u32 source32_b(u32 pixel) { return (pixel >> ( 0 + SrcShiftB)) & (0xff >> SrcShiftB); }

	// destination pixel masks are based on the template parameters as well
	static constexpr u32 dest_r(PixelType pixel) { return (pixel >> DstShiftR) & (0xff >> SrcShiftR); }
	static constexpr u32 dest_g(PixelType pixel) { return (pixel >> DstShiftG) & (0xff >> SrcShiftG); }
	static constexpr u32 dest_b(PixelType pixel) { return (pixel >> DstShiftB) & (0xff >> SrcShiftB); }

	// generic conversion with special optimization for destinations in the standard format
	static constexpr PixelType source32_to_dest(u32 pixel)
	{
		if (SrcShiftR == 0 && SrcShiftG == 0 && SrcShiftB == 0 && DstShiftR == 16 && DstShiftG == 8 && DstShiftB == 0)
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

	static constexpr u32 clamp16_shift8(u32 x)
	{
		return (s32(x) < 0) ? 0 : (x > 65535) ? 255 : (x >> 8);
	}

	static constexpr u32 ycc_to_rgb(u32 ycc)
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
		rgb_t const *const palbase = texture.palette;
		if constexpr (BilinearFilter)
		{
			s32 u0 = curu >> 16;
			s32 u1 = 1;
			if (u0 < 0) u0 = u1 = 0;
			else if (u0 + 1 >= texture.width) u0 = texture.width - 1, u1 = 0;
			s32 v0 = curv >> 16;
			s32 v1 = texture.rowpixels;
			if (v0 < 0) v0 = v1 = 0;
			else if (v0 + 1 >= texture.height) v0 = texture.height - 1, v1 = 0;

			u16 const *texbase = reinterpret_cast<u16 const *>(texture.base);
			texbase += v0 * texture.rowpixels + u0;

			u32 pix00 = palbase[texbase[0]];
			u32 pix01 = palbase[texbase[u1]];
			u32 pix10 = palbase[texbase[v1]];
			u32 pix11 = palbase[texbase[u1 + v1]];
			return rgbaint_t::bilinear_filter(pix00, pix01, pix10, pix11, curu >> 8, curv >> 8);
		}
		else
		{
			s32 u = std::clamp<s32>(curu >> 16, 0, texture.width - 1);
			s32 v = std::clamp<s32>(curv >> 16, 0, texture.height - 1);

			u16 const *const texbase = reinterpret_cast<u16 const *>(texture.base) + v * texture.rowpixels + u;
			return palbase[texbase[0]];
		}
	}


	//-------------------------------------------------
	//  get_texel_palette16a - return a texel from a
	//  palettized 16bpp source with alpha
	//-------------------------------------------------

	static inline u32 get_texel_palette16a(const render_texinfo &texture, s32 curu, s32 curv)
	{
		rgb_t const *const palbase = texture.palette;
		if constexpr (BilinearFilter)
		{
			s32 u0 = curu >> 16;
			s32 u1 = 1;
			if (u0 < 0) u0 = u1 = 0;
			else if (u0 + 1 >= texture.width) u0 = texture.width - 1, u1 = 0;
			s32 v0 = curv >> 16;
			s32 v1 = texture.rowpixels;
			if (v0 < 0) v0 = v1 = 0;
			else if (v0 + 1 >= texture.height) v0 = texture.height - 1, v1 = 0;

			u16 const *texbase = reinterpret_cast<u16 const *>(texture.base);
			texbase += v0 * texture.rowpixels + u0;

			return rgbaint_t::bilinear_filter(palbase[texbase[0]], palbase[texbase[u1]], palbase[texbase[v1]], palbase[texbase[u1 + v1]], curu >> 8, curv >> 8);
		}
		else
		{
			s32 u = std::clamp<s32>(curu >> 16, 0, texture.width - 1);
			s32 v = std::clamp<s32>(curv >> 16, 0, texture.height - 1);

			u16 const *const texbase = reinterpret_cast<u16 const *>(texture.base) + v * texture.rowpixels + u;
			return palbase[texbase[0]];
		}
	}


	//-------------------------------------------------
	//  get_texel_yuy16 - return a texel from a 16bpp
	//  YCbCr source (pixel is returned as Cr-Cb-Y)
	//-------------------------------------------------

	static inline u32 get_texel_yuy16(const render_texinfo &texture, s32 curu, s32 curv)
	{
		if constexpr (BilinearFilter)
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
			s32 u = std::clamp<s32>(curu >> 16, 0, texture.width - 1);
			s32 v = std::clamp<s32>(curv >> 16, 0, texture.height - 1);

			const u16 *texbase = reinterpret_cast<const u16 *>(texture.base) + v * texture.rowpixels + (u >> 1) * 2;
			return (texbase[u & 1] >> 8) | ((texbase[0] & 0xff) << 8) | ((texbase[1] & 0xff) << 16);
		}
	}


	//-------------------------------------------------
	//  get_texel_rgb32 - return a texel from a 32bpp
	//  RGB source
	//-------------------------------------------------

	template <bool Wrap>
	static inline u32 get_texel_rgb32(const render_texinfo &texture, s32 curu, s32 curv)
	{
		if constexpr (BilinearFilter)
		{
			s32 u0, u1, v0, v1;
			if constexpr (Wrap)
			{
				u0 = (curu >> 16) % texture.width;
				if (0 > u0)
					u0 += texture.width;
				u1 = (u0 + 1) % texture.width;

				v0 = (curv >> 16) % texture.height;
				if (0 > v0)
					v0 += texture.height;
				v1 = (v0 + 1) % texture.height;
			}
			else
			{
				u0 = curu >> 16;
				if (u0 < 0)
					u0 = u1 = 0;
				else if (texture.width <= (u0 + 1))
					u0 = u1 = texture.width - 1;
				else
					u1 = u0 + 1;

				v0 = curv >> 16;
				if (v0 < 0)
					v0 = v1 = 0;
				else if (texture.height <= (v0 + 1))
					v0 = v1 = texture.height - 1;
				else
					v1 = v0 + 1;
			}
			u32 const *const texbase = reinterpret_cast<u32 const *>(texture.base);
			u32 const *const row0base = texbase + (v0 * texture.rowpixels);
			u32 const *const row1base = texbase + (v1 * texture.rowpixels);
			return rgbaint_t::bilinear_filter(row0base[u0], row0base[u1], row1base[u0], row1base[u1], curu >> 8, curv >> 8);
		}
		else
		{
			s32 u, v;
			if constexpr (Wrap)
			{
				u = (curu >> 16) % texture.width;
				if (0 > u)
					u += texture.width;

				v = (curv >> 16) % texture.height;
				if (0 > v)
					v += texture.height;
			}
			else
			{
				u = std::clamp<s32>(curu >> 16, 0, texture.width - 1);
				v = std::clamp<s32>(curv >> 16, 0, texture.height - 1);
			}
			u32 const *const rowbase = reinterpret_cast<u32 const *>(texture.base) + (v * texture.rowpixels);
			return rowbase[u];
		}
	}


	//-------------------------------------------------
	//  get_texel_argb32 - return a texel from a 32bpp
	//  ARGB source
	//-------------------------------------------------

	template <bool Wrap>
	static inline u32 get_texel_argb32(render_texinfo const &texture, s32 curu, s32 curv)
	{
		if constexpr (BilinearFilter)
		{
			s32 u0, u1, v0, v1;
			if constexpr (Wrap)
			{
				u0 = (curu >> 16) % texture.width;
				if (0 > u0)
					u0 += texture.width;
				u1 = (u0 + 1) % texture.width;

				v0 = (curv >> 16) % texture.height;
				if (0 > v0)
					v0 += texture.height;
				v1 = (v0 + 1) % texture.height;
			}
			else
			{
				u0 = curu >> 16;
				if (u0 < 0)
					u0 = u1 = 0;
				else if (texture.width <= (u0 + 1))
					u0 = u1 = texture.width - 1;
				else
					u1 = u0 + 1;

				v0 = curv >> 16;
				if (v0 < 0)
					v0 = v1 = 0;
				else if (texture.height <= (v0 + 1))
					v0 = v1 = texture.height - 1;
				else
					v1 = v0 + 1;
			}
			u32 const *const texbase = reinterpret_cast<u32 const *>(texture.base);
			u32 const *const row0base = texbase + (v0 * texture.rowpixels);
			u32 const *const row1base = texbase + (v1 * texture.rowpixels);
			return rgbaint_t::bilinear_filter(row0base[u0], row0base[u1], row1base[u0], row1base[u1], curu >> 8, curv >> 8);
		}
		else
		{
			s32 u, v;
			if constexpr (Wrap)
			{
				u = (curu >> 16) % texture.width;
				if (0 > u)
					u += texture.width;

				v = (curv >> 16) % texture.height;
				if (0 > v)
					v += texture.height;
			}
			else
			{
				u = std::clamp<s32>(curu >> 16, 0, texture.width - 1);
				v = std::clamp<s32>(curv >> 16, 0, texture.height - 1);
			}
			u32 const *const rowbase = reinterpret_cast<u32 const *>(texture.base) + (v * texture.rowpixels);
			return rowbase[u];
		}
	}


	//-------------------------------------------------
	//  draw_aa_pixel - draw an antialiased pixel
	//-------------------------------------------------

	static inline void draw_aa_pixel(PixelType *dstdata, u32 pitch, int x, int y, u32 col)
	{
		PixelType *dest = dstdata + y * pitch + x;
		u32 dpix = NoDestRead ? 0 : *dest;
		u32 dr = source32_r(col) + dest_r(dpix);
		u32 dg = source32_g(col) + dest_g(dpix);
		u32 db = source32_b(col) + dest_b(dpix);
		dr = (dr | -(dr >> (8 - SrcShiftR))) & (0xff >> SrcShiftR);
		dg = (dg | -(dg >> (8 - SrcShiftG))) & (0xff >> SrcShiftG);
		db = (db | -(db >> (8 - SrcShiftB))) & (0xff >> SrcShiftB);
		*dest = dest_assemble_rgb(dr, dg, db);
	}


	//-------------------------------------------------
	//  draw_line - draw a line or point
	//-------------------------------------------------

	static void draw_line(render_primitive const &prim, PixelType *dstdata, s32 width, s32 height, u32 pitch)
	{
		// internal cosine table generated at compile time
		static auto const s_cosine_table = make_cosine_table(std::make_integer_sequence<int, 2049>());

		// compute the start/end coordinates
		int x1 = int(prim.bounds.x0 * 65536.0f);
		int y1 = int(prim.bounds.y0 * 65536.0f);
		int x2 = int(prim.bounds.x1 * 65536.0f);
		int y2 = int(prim.bounds.y1 * 65536.0f);

		// handle color and intensity
		u32 col = rgb_t(int(255.0f * prim.color.r * prim.color.a), int(255.0f * prim.color.g * prim.color.a), int(255.0f * prim.color.b * prim.color.a));

		if (PRIMFLAG_GET_ANTIALIAS(prim.flags))
		{
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

	static void draw_rect(render_primitive const &prim, PixelType *dstdata, s32 width, s32 height, u32 pitch)
	{
		render_bounds const fpos = prim.bounds;
		assert(fpos.x0 <= fpos.x1);
		assert(fpos.y0 <= fpos.y1);

		// clamp to integers and ensure we fit
		s32 const startx = std::clamp<s32>(round_nearest(fpos.x0), 0, width);
		s32 const starty = std::clamp<s32>(round_nearest(fpos.y0), 0, height);
		s32 const endx = std::clamp<s32>(round_nearest(fpos.x1), 0, width);
		s32 const endy = std::clamp<s32>(round_nearest(fpos.y1), 0, height);

		// bail if nothing left
		if ((startx > endx) || (starty > endy))
			return;

		// only support alpha and "none" blendmodes
		assert(PRIMFLAG_GET_BLENDMODE(prim.flags) == BLENDMODE_NONE ||
				PRIMFLAG_GET_BLENDMODE(prim.flags) == BLENDMODE_ALPHA);

		if ((PRIMFLAG_GET_BLENDMODE(prim.flags) == BLENDMODE_NONE) || is_opaque(prim.color.a))
		{
			// fast case: no alpha

			// clamp R,G,B to 0-256 range
			u32 const r = u32(std::clamp(256.0f * prim.color.r, 0.0f, 255.0f));
			u32 const g = u32(std::clamp(256.0f * prim.color.g, 0.0f, 255.0f));
			u32 const b = u32(std::clamp(256.0f * prim.color.b, 0.0f, 255.0f));
			u32 const pix = dest_rgb_to_pixel(r, g, b);

			// loop over rows
			for (s32 y = starty; y < endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + startx;

				// loop over cols
				for (s32 x = startx; x < endx; x++)
					*dest++ = pix;
			}
		}
		else if (!is_transparent(prim.color.a))
		{
			// alpha and/or coloring case
			u32 const rmask = dest_rgb_to_pixel(0xff,0x00,0x00);
			u32 const gmask = dest_rgb_to_pixel(0x00,0xff,0x00);
			u32 const bmask = dest_rgb_to_pixel(0x00,0x00,0xff);

			// clamp R,G,B and inverse A to 0-256 range
			u32 r = u32(std::clamp(256.0f * prim.color.r * prim.color.a, 0.0f, 255.0f));
			u32 g = u32(std::clamp(256.0f * prim.color.g * prim.color.a, 0.0f, 255.0f));
			u32 b = u32(std::clamp(256.0f * prim.color.b * prim.color.a, 0.0f, 255.0f));
			u32 const inva = u32(std::clamp(256.0f * (1.0f - prim.color.a), 0.0f, 256.0f));

			// pre-shift the RGBA pieces
			r = dest_rgb_to_pixel(r, 0, 0) << 8;
			g = dest_rgb_to_pixel(0, g, 0) << 8;
			b = dest_rgb_to_pixel(0, 0, b) << 8;

			// loop over rows
			for (s32 y = starty; y < endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + startx;

				// loop over cols
				for (s32 x = startx; x < endx; x++)
				{
					u32 dpix = NoDestRead ? 0 : *dest;
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

	static void draw_quad_palette16_none(render_primitive const &prim, PixelType *dstdata, u32 pitch, quad_setup_data const &setup)
	{
		// ensure all parameters are valid
		assert(prim.texture.palette != nullptr);

		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// fast case: no coloring, no alpha

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < setup.endx; x++)
				{
					u32 const pix = get_texel_palette16(prim.texture, curu, curv);
					*dest++ = source32_to_dest(pix);
					curu += setup.dudx;
					curv += setup.dvdx;
				}
			}
		}
		else if (is_opaque(prim.color.a))
		{
			// coloring-only case

			// clamp R,G,B to 0-256 range
			u32 const sr = u32(std::clamp(256.0f * prim.color.r, 0.0f, 256.0f));
			u32 const sg = u32(std::clamp(256.0f * prim.color.g, 0.0f, 256.0f));
			u32 const sb = u32(std::clamp(256.0f * prim.color.b, 0.0f, 256.0f));

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < setup.endx; x++)
				{
					u32 const pix = get_texel_palette16(prim.texture, curu, curv);
					u32 const r = (source32_r(pix) * sr) >> 8;
					u32 const g = (source32_g(pix) * sg) >> 8;
					u32 const b = (source32_b(pix) * sb) >> 8;

					*dest++ = dest_assemble_rgb(r, g, b);
					curu += setup.dudx;
					curv += setup.dvdx;
				}
			}
		}
		else if (!is_transparent(prim.color.a))
		{
			// alpha and/or coloring case

			// clamp R,G,B and inverse A to 0-256 range
			u32 const sr = u32(std::clamp(256.0f * prim.color.r * prim.color.a, 0.0f, 256.0f));
			u32 const sg = u32(std::clamp(256.0f * prim.color.g * prim.color.a, 0.0f, 256.0f));
			u32 const sb = u32(std::clamp(256.0f * prim.color.b * prim.color.a, 0.0f, 256.0f));
			u32 const invsa = u32(std::clamp(256.0f * (1.0f - prim.color.a), 0.0f, 256.0f));

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < setup.endx; x++)
				{
					u32 const pix = get_texel_palette16(prim.texture, curu, curv);
					u32 const dpix = NoDestRead ? 0 : *dest;
					u32 const r = (source32_r(pix) * sr + dest_r(dpix) * invsa) >> 8;
					u32 const g = (source32_g(pix) * sg + dest_g(dpix) * invsa) >> 8;
					u32 const b = (source32_b(pix) * sb + dest_b(dpix) * invsa) >> 8;

					*dest++ = dest_assemble_rgb(r, g, b);
					curu += setup.dudx;
					curv += setup.dvdx;
				}
			}
		}
	}


	//-------------------------------------------------
	//  draw_quad_palette16_add - perform
	//  rasterization of a 16bpp palettized texture
	//-------------------------------------------------

	static void draw_quad_palette16_add(render_primitive const &prim, PixelType *dstdata, u32 pitch, quad_setup_data const &setup)
	{
		// ensure all parameters are valid
		assert(prim.texture.palette != nullptr);

		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// fast case: no coloring, no alpha

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < setup.endx; x++)
				{
					const u32 pix = get_texel_palette16(prim.texture, curu, curv);
					if ((pix & 0xffffff) != 0)
					{
						u32 const dpix = NoDestRead ? 0 : *dest;
						u32 r = source32_r(pix) + dest_r(dpix);
						u32 g = source32_g(pix) + dest_g(dpix);
						u32 b = source32_b(pix) + dest_b(dpix);
						r = (r | -(r >> (8 - SrcShiftR))) & (0xff >> SrcShiftR);
						g = (g | -(g >> (8 - SrcShiftG))) & (0xff >> SrcShiftG);
						b = (b | -(b >> (8 - SrcShiftB))) & (0xff >> SrcShiftB);
						*dest = dest_assemble_rgb(r, g, b);
					}
					dest++;
					curu += setup.dudx;
					curv += setup.dvdx;
				}
			}
		}
		else
		{
			// alpha and/or coloring case

			// clamp R,G,B and inverse A to 0-256 range
			u32 const sr = u32(std::clamp(256.0f * prim.color.r * prim.color.a, 0.0f, 256.0f));
			u32 const sg = u32(std::clamp(256.0f * prim.color.g * prim.color.a, 0.0f, 256.0f));
			u32 const sb = u32(std::clamp(256.0f * prim.color.b * prim.color.a, 0.0f, 256.0f));

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < setup.endx; x++)
				{
					u32 const pix = get_texel_palette16(prim.texture, curu, curv);
					if ((pix & 0xffffff) != 0)
					{
						u32 const dpix = NoDestRead ? 0 : *dest;
						u32 r = ((source32_r(pix) * sr) >> 8) + dest_r(dpix);
						u32 g = ((source32_g(pix) * sg) >> 8) + dest_g(dpix);
						u32 b = ((source32_b(pix) * sb) >> 8) + dest_b(dpix);
						r = (r | -(r >> (8 - SrcShiftR))) & (0xff >> SrcShiftR);
						g = (g | -(g >> (8 - SrcShiftG))) & (0xff >> SrcShiftG);
						b = (b | -(b >> (8 - SrcShiftB))) & (0xff >> SrcShiftB);
						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
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

	static void draw_quad_yuy16_none(render_primitive const &prim, PixelType *dstdata, u32 pitch, quad_setup_data const &setup)
	{
		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// fast case: no coloring, no alpha

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < setup.endx; x++)
				{
					u32 const pix = ycc_to_rgb(get_texel_yuy16(prim.texture, curu, curv));
					*dest++ = source32_to_dest(pix);
					curu += setup.dudx;
					curv += setup.dvdx;
				}
			}
		}
		else if (is_opaque(prim.color.a))
		{
			// coloring-only case

			// clamp R,G,B to 0-256 range
			u32 const sr = u32(std::clamp(256.0f * prim.color.r, 0.0f, 256.0f));
			u32 const sg = u32(std::clamp(256.0f * prim.color.g, 0.0f, 256.0f));
			u32 const sb = u32(std::clamp(256.0f * prim.color.b, 0.0f, 256.0f));

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < setup.endx; x++)
				{
					u32 const pix = ycc_to_rgb(get_texel_yuy16(prim.texture, curu, curv));
					u32 const r = (source32_r(pix) * sr) >> 8;
					u32 const g = (source32_g(pix) * sg) >> 8;
					u32 const b = (source32_b(pix) * sb) >> 8;

					*dest++ = dest_assemble_rgb(r, g, b);
					curu += setup.dudx;
					curv += setup.dvdx;
				}
			}
		}
		else if (!is_transparent(prim.color.a))
		{
			// alpha and/or coloring case

			// clamp R,G,B and inverse A to 0-256 range
			u32 const sr = u32(std::clamp(256.0f * prim.color.r * prim.color.a, 0.0f, 256.0f));
			u32 const sg = u32(std::clamp(256.0f * prim.color.g * prim.color.a, 0.0f, 256.0f));
			u32 const sb = u32(std::clamp(256.0f * prim.color.b * prim.color.a, 0.0f, 256.0f));
			u32 const invsa = u32(std::clamp(256.0f * (1.0f - prim.color.a), 0.0f, 256.0f));

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < setup.endx; x++)
				{
					u32 const pix = ycc_to_rgb(get_texel_yuy16(prim.texture, curu, curv));
					u32 const dpix = NoDestRead ? 0 : *dest;
					u32 const r = (source32_r(pix) * sr + dest_r(dpix) * invsa) >> 8;
					u32 const g = (source32_g(pix) * sg + dest_g(dpix) * invsa) >> 8;
					u32 const b = (source32_b(pix) * sb + dest_b(dpix) * invsa) >> 8;

					*dest++ = dest_assemble_rgb(r, g, b);
					curu += setup.dudx;
					curv += setup.dvdx;
				}
			}
		}
	}


	//-------------------------------------------------
	//  draw_quad_yuy16_add - perform
	//  rasterization by using RGB add after YUY
	//  conversion
	//-------------------------------------------------

	static void draw_quad_yuy16_add(render_primitive const &prim, PixelType *dstdata, u32 pitch, quad_setup_data const &setup)
	{
		// simply can't do this without reading from the dest
		if constexpr (NoDestRead)
			return;

		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// fast case: no coloring, no alpha

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < setup.endx; x++)
				{
					u32 const pix = ycc_to_rgb(get_texel_yuy16(prim.texture, curu, curv));
					u32 const dpix = NoDestRead ? 0 : *dest;
					u32 r = source32_r(pix) + dest_r(dpix);
					u32 g = source32_g(pix) + dest_g(dpix);
					u32 b = source32_b(pix) + dest_b(dpix);
					r = (r | -(r >> (8 - SrcShiftR))) & (0xff >> SrcShiftR);
					g = (g | -(g >> (8 - SrcShiftG))) & (0xff >> SrcShiftG);
					b = (b | -(b >> (8 - SrcShiftB))) & (0xff >> SrcShiftB);
					*dest++ = dest_assemble_rgb(r, g, b);
					curu += setup.dudx;
					curv += setup.dvdx;
				}
			}
		}
		else
		{
			// alpha and/or coloring case

			// clamp R,G,B and inverse A to 0-256 range
			u32 const sr = u32(std::clamp(256.0f * prim.color.r, 0.0f, 256.0f));
			u32 const sg = u32(std::clamp(256.0f * prim.color.g, 0.0f, 256.0f));
			u32 const sb = u32(std::clamp(256.0f * prim.color.b, 0.0f, 256.0f));
			u32 const sa = u32(std::clamp(256.0f * prim.color.a, 0.0f, 256.0f));

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				// loop over cols
				for (s32 x = setup.startx; x < setup.endx; x++)
				{
					const u32 pix = ycc_to_rgb(get_texel_yuy16(prim.texture, curu, curv));
					const u32 dpix = NoDestRead ? 0 : *dest;
					u32 r = ((source32_r(pix) * sr * sa) >> 16) + dest_r(dpix);
					u32 g = ((source32_g(pix) * sg * sa) >> 16) + dest_g(dpix);
					u32 b = ((source32_b(pix) * sb * sa) >> 16) + dest_b(dpix);
					r = (r | -(r >> (8 - SrcShiftR))) & (0xff >> SrcShiftR);
					g = (g | -(g >> (8 - SrcShiftG))) & (0xff >> SrcShiftG);
					b = (b | -(b >> (8 - SrcShiftB))) & (0xff >> SrcShiftB);
					*dest++ = dest_assemble_rgb(r, g, b);
					curu += setup.dudx;
					curv += setup.dvdx;
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

	template <bool Wrap>
	static void draw_quad_rgb32(render_primitive const &prim, PixelType *dstdata, u32 pitch, quad_setup_data const &setup)
	{
		rgb_t const *const palbase = prim.texture.palette;

		// fast case: no coloring, no alpha
		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				if (!palbase)
				{
					// no lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_rgb32<Wrap>(prim.texture, curu, curv);
						*dest++ = source32_to_dest(pix);
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
				else
				{
					// lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_rgb32<Wrap>(prim.texture, curu, curv);
						u32 const r = palbase[(pix >> 16) & 0xff] >> SrcShiftR;
						u32 const g = palbase[(pix >> 8) & 0xff] >> SrcShiftG;
						u32 const b = palbase[(pix >> 0) & 0xff] >> SrcShiftB;

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
			}
		}
		else if (is_opaque(prim.color.a))
		{
			// coloring-only case

			// clamp R,G,B to 0-256 range
			u32 const sr = u32(std::clamp(256.0f * prim.color.r, 0.0f, 256.0f));
			u32 const sg = u32(std::clamp(256.0f * prim.color.g, 0.0f, 256.0f));
			u32 const sb = u32(std::clamp(256.0f * prim.color.b, 0.0f, 256.0f));

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				if (!palbase)
				{
					// no lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_rgb32<Wrap>(prim.texture, curu, curv);
						u32 const r = (source32_r(pix) * sr) >> 8;
						u32 const g = (source32_g(pix) * sg) >> 8;
						u32 const b = (source32_b(pix) * sb) >> 8;

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
				else
				{
					// lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_rgb32<Wrap>(prim.texture, curu, curv);
						u32 const r = (palbase[(pix >> 16) & 0xff] * sr) >> (8 + SrcShiftR);
						u32 const g = (palbase[(pix >> 8) & 0xff] * sg) >> (8 + SrcShiftG);
						u32 const b = (palbase[(pix >> 0) & 0xff] * sb) >> (8 + SrcShiftB);

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
			}
		}
		else if (!is_transparent(prim.color.a))
		{
			// alpha and/or coloring case

			// clamp R,G,B and inverse A to 0-256 range
			u32 const sr = u32(std::clamp(256.0f * prim.color.r * prim.color.a, 0.0f, 256.0f));
			u32 const sg = u32(std::clamp(256.0f * prim.color.g * prim.color.a, 0.0f, 256.0f));
			u32 const sb = u32(std::clamp(256.0f * prim.color.b * prim.color.a, 0.0f, 256.0f));
			u32 const invsa = u32(std::clamp(256.0f * (1.0f - prim.color.a), 0.0f, 256.0f));

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				if (!palbase)
				{
					// no lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_rgb32<Wrap>(prim.texture, curu, curv);
						u32 const dpix = NoDestRead ? 0 : *dest;
						u32 const r = (source32_r(pix) * sr + dest_r(dpix) * invsa) >> 8;
						u32 const g = (source32_g(pix) * sg + dest_g(dpix) * invsa) >> 8;
						u32 const b = (source32_b(pix) * sb + dest_b(dpix) * invsa) >> 8;

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
				else
				{
					// lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_rgb32<Wrap>(prim.texture, curu, curv);
						u32 const dpix = NoDestRead ? 0 : *dest;
						u32 const r = ((palbase[(pix >> 16) & 0xff] >> SrcShiftR) * sr + dest_r(dpix) * invsa) >> 8;
						u32 const g = ((palbase[(pix >> 8) & 0xff] >> SrcShiftG) * sg + dest_g(dpix) * invsa) >> 8;
						u32 const b = ((palbase[(pix >> 0) & 0xff] >> SrcShiftB) * sb + dest_b(dpix) * invsa) >> 8;

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
			}
		}
	}


	//-------------------------------------------------
	//  draw_quad_rgb32_add - perform
	//  rasterization by using RGB add
	//-------------------------------------------------

	template <bool Wrap>
	static void draw_quad_rgb32_add(render_primitive const &prim, PixelType *dstdata, u32 pitch, quad_setup_data const &setup)
	{
		// simply can't do this without reading from the dest
		if (NoDestRead)
			return;

		rgb_t const *const palbase = prim.texture.palette;

		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// fast case: no coloring, no alpha

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				if (!palbase)
				{
					// no lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const dpix = NoDestRead ? 0 : *dest;
						u32 r = source32_r(pix) + dest_r(dpix);
						u32 g = source32_g(pix) + dest_g(dpix);
						u32 b = source32_b(pix) + dest_b(dpix);
						r = (r | -(r >> (8 - SrcShiftR))) & (0xff >> SrcShiftR);
						g = (g | -(g >> (8 - SrcShiftG))) & (0xff >> SrcShiftG);
						b = (b | -(b >> (8 - SrcShiftB))) & (0xff >> SrcShiftB);
						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
				else
				{
					// lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const dpix = NoDestRead ? 0 : *dest;
						u32 r = (palbase[(pix >> 16) & 0xff] >> SrcShiftR) + dest_r(dpix);
						u32 g = (palbase[(pix >> 8) & 0xff] >> SrcShiftG) + dest_g(dpix);
						u32 b = (palbase[(pix >> 0) & 0xff] >> SrcShiftB) + dest_b(dpix);
						r = (r | -(r >> (8 - SrcShiftR))) & (0xff >> SrcShiftR);
						g = (g | -(g >> (8 - SrcShiftG))) & (0xff >> SrcShiftG);
						b = (b | -(b >> (8 - SrcShiftB))) & (0xff >> SrcShiftB);
						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
			}
		}
		else
		{
			// alpha and/or coloring case

			// clamp R,G,B and inverse A to 0-256 range
			u32 const sr = u32(std::clamp(256.0f * prim.color.r, 0.0f, 256.0f));
			u32 const sg = u32(std::clamp(256.0f * prim.color.g, 0.0f, 256.0f));
			u32 const sb = u32(std::clamp(256.0f * prim.color.b, 0.0f, 256.0f));
			u32 const sa = u32(std::clamp(256.0f * prim.color.a, 0.0f, 256.0f));

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				if (!palbase)
				{
					// no lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const dpix = NoDestRead ? 0 : *dest;
						u32 r = ((source32_r(pix) * sr * sa) >> 16) + dest_r(dpix);
						u32 g = ((source32_g(pix) * sg * sa) >> 16) + dest_g(dpix);
						u32 b = ((source32_b(pix) * sb * sa) >> 16) + dest_b(dpix);
						r = (r | -(r >> (8 - SrcShiftR))) & (0xff >> SrcShiftR);
						g = (g | -(g >> (8 - SrcShiftG))) & (0xff >> SrcShiftG);
						b = (b | -(b >> (8 - SrcShiftB))) & (0xff >> SrcShiftB);
						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
				else
				{
					// lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const dpix = NoDestRead ? 0 : *dest;
						u32 r = ((palbase[(pix >> 16) & 0xff] * sr * sa) >> (16 + SrcShiftR)) + dest_r(dpix);
						u32 g = ((palbase[(pix >> 8) & 0xff] * sr * sa) >> (16 + SrcShiftR)) + dest_g(dpix);
						u32 b = ((palbase[(pix >> 0) & 0xff] * sr * sa) >> (16 + SrcShiftR)) + dest_b(dpix);
						r = (r | -(r >> (8 - SrcShiftR))) & (0xff >> SrcShiftR);
						g = (g | -(g >> (8 - SrcShiftG))) & (0xff >> SrcShiftG);
						b = (b | -(b >> (8 - SrcShiftB))) & (0xff >> SrcShiftB);
						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
			}
		}
	}


	//-------------------------------------------------
	//  draw_quad_rgb32_multiply - perform
	//  rasterization using RGB multiply
	//-------------------------------------------------

	template <bool Wrap>
	static void draw_quad_rgb32_multiply(render_primitive const &prim, PixelType *dstdata, u32 pitch, quad_setup_data const &setup)
	{
		// simply can't do this without reading from the dest
		if (NoDestRead)
			return;

		rgb_t const *const palbase = prim.texture.palette;

		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// fast case: no coloring, no alpha

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				if (!palbase)
				{
					// no lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const dpix = NoDestRead ? 0 : *dest;
						u32 const r = (source32_r(pix) * dest_r(dpix)) >> (8 - SrcShiftR);
						u32 const g = (source32_g(pix) * dest_g(dpix)) >> (8 - SrcShiftG);
						u32 const b = (source32_b(pix) * dest_b(dpix)) >> (8 - SrcShiftB);

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
				else
				{
					// lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const dpix = NoDestRead ? 0 : *dest;
						u32 const r = (palbase[(pix >> 16) & 0xff] * dest_r(dpix)) >> 8;
						u32 const g = (palbase[(pix >> 8) & 0xff] * dest_g(dpix)) >> 8;
						u32 const b = (palbase[(pix >> 0) & 0xff] * dest_b(dpix)) >> 8;

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
			}
		}
		else
		{
			// alpha and/or coloring case

			// clamp R,G,B to 0-256 range
			u32 const sr = u32(std::clamp(256.0f * prim.color.r * prim.color.a, 0.0f, 256.0f));
			u32 const sg = u32(std::clamp(256.0f * prim.color.g * prim.color.a, 0.0f, 256.0f));
			u32 const sb = u32(std::clamp(256.0f * prim.color.b * prim.color.a, 0.0f, 256.0f));

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				if (!palbase)
				{
					// no lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const dpix = NoDestRead ? 0 : *dest;
						u32 const r = (source32_r(pix) * sr * dest_r(dpix)) >> (16 - SrcShiftR);
						u32 const g = (source32_g(pix) * sg * dest_g(dpix)) >> (16 - SrcShiftG);
						u32 const b = (source32_b(pix) * sb * dest_b(dpix)) >> (16 - SrcShiftB);

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
				else
				{
					// lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const dpix = NoDestRead ? 0 : *dest;
						u32 const r = (palbase[(pix >> 16) & 0xff] * sr * dest_r(dpix)) >> 16;
						u32 const g = (palbase[(pix >> 8) & 0xff] * sg * dest_g(dpix)) >> 16;
						u32 const b = (palbase[(pix >> 0) & 0xff] * sb * dest_b(dpix)) >> 16;

						*dest++ = dest_assemble_rgb(r, g, b);
						curu += setup.dudx;
						curv += setup.dvdx;
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

	template <bool Wrap>
	static void draw_quad_argb32_alpha(render_primitive const &prim, PixelType *dstdata, u32 pitch, quad_setup_data const &setup)
	{
		rgb_t const *const palbase = prim.texture.palette;

		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// fast case: no coloring, no alpha

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				if (!palbase)
				{
					// no lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const ta = pix >> 24;
						if (ta != 0)
						{
							u32 const dpix = NoDestRead ? 0 : *dest;
							u32 const invta = 0x100 - ta;
							u32 const r = (source32_r(pix) * ta + dest_r(dpix) * invta) >> 8;
							u32 const g = (source32_g(pix) * ta + dest_g(dpix) * invta) >> 8;
							u32 const b = (source32_b(pix) * ta + dest_b(dpix) * invta) >> 8;

							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
				else
				{
					// lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const ta = pix >> 24;
						if (ta != 0)
						{
							u32 const dpix = NoDestRead ? 0 : *dest;
							u32 const invta = 0x100 - ta;
							u32 const r = ((palbase[(pix >> 16) & 0xff] >> SrcShiftR) * ta + dest_r(dpix) * invta) >> 8;
							u32 const g = ((palbase[(pix >> 8) & 0xff] >> SrcShiftG) * ta + dest_g(dpix) * invta) >> 8;
							u32 const b = ((palbase[(pix >> 0) & 0xff] >> SrcShiftB) * ta + dest_b(dpix) * invta) >> 8;

							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
			}
		}
		else
		{
			// alpha and/or coloring case

			// clamp R,G,B and inverse A to 0-256 range
			u32 const sr = u32(std::clamp(256.0f * prim.color.r, 0.0f, 256.0f));
			u32 const sg = u32(std::clamp(256.0f * prim.color.g, 0.0f, 256.0f));
			u32 const sb = u32(std::clamp(256.0f * prim.color.b, 0.0f, 256.0f));
			u32 const sa = u32(std::clamp(256.0f * prim.color.a, 0.0f, 256.0f));

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				if (!palbase)
				{
					// no lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const ta = (pix >> 24) * sa;
						if (ta != 0)
						{
							u32 const dpix = NoDestRead ? 0 : *dest;
							u32 const invsta = (0x10000 - ta) << 8;
							u32 const r = (source32_r(pix) * sr * ta + dest_r(dpix) * invsta) >> 24;
							u32 const g = (source32_g(pix) * sg * ta + dest_g(dpix) * invsta) >> 24;
							u32 const b = (source32_b(pix) * sb * ta + dest_b(dpix) * invsta) >> 24;

							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
				else
				{
					// lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const ta = (pix >> 24) * sa;
						if (ta != 0)
						{
							u32 const dpix = NoDestRead ? 0 : *dest;
							u32 const invsta = (0x10000 - ta) << 8;
							u32 const r = ((palbase[(pix >> 16) & 0xff] >> SrcShiftR) * sr * ta + dest_r(dpix) * invsta) >> 24;
							u32 const g = ((palbase[(pix >> 8) & 0xff] >> SrcShiftG) * sg * ta + dest_g(dpix) * invsta) >> 24;
							u32 const b = ((palbase[(pix >> 0) & 0xff] >> SrcShiftB) * sb * ta + dest_b(dpix) * invsta) >> 24;

							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
			}
		}
	}


	//-------------------------------------------------
	//  draw_quad_argb32_add - perform
	//  rasterization by using RGB add
	//-------------------------------------------------

	template <bool Wrap>
	static void draw_quad_argb32_add(render_primitive const &prim, PixelType *dstdata, u32 pitch, quad_setup_data const &setup)
	{
		// simply can't do this without reading from the dest
		if (NoDestRead)
			return;

		rgb_t const *const palbase = prim.texture.palette;

		// fast case: no coloring, no alpha
		if (prim.color.r >= 1.0f && prim.color.g >= 1.0f && prim.color.b >= 1.0f && is_opaque(prim.color.a))
		{
			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				if (!palbase)
				{
					// no lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const ta = pix >> 24;
						if (ta != 0)
						{
							u32 const dpix = NoDestRead ? 0 : *dest;
							u32 r = ((source32_r(pix) * ta) >> 8) + dest_r(dpix);
							u32 g = ((source32_g(pix) * ta) >> 8) + dest_g(dpix);
							u32 b = ((source32_b(pix) * ta) >> 8) + dest_b(dpix);
							r = (r | -(r >> (8 - SrcShiftR))) & (0xff >> SrcShiftR);
							g = (g | -(g >> (8 - SrcShiftG))) & (0xff >> SrcShiftG);
							b = (b | -(b >> (8 - SrcShiftB))) & (0xff >> SrcShiftB);
							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
				else
				{
					// lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const ta = pix >> 24;
						if (ta != 0)
						{
							u32 const dpix = NoDestRead ? 0 : *dest;
							u32 r = ((palbase[(pix >> 16) & 0xff] * ta) >> (8 + SrcShiftR)) + dest_r(dpix);
							u32 g = ((palbase[(pix >> 8) & 0xff] * ta) >> (8 + SrcShiftG)) + dest_g(dpix);
							u32 b = ((palbase[(pix >> 0) & 0xff] * ta) >> (8 + SrcShiftB)) + dest_b(dpix);
							r = (r | -(r >> (8 - SrcShiftR))) & (0xff >> SrcShiftR);
							g = (g | -(g >> (8 - SrcShiftG))) & (0xff >> SrcShiftG);
							b = (b | -(b >> (8 - SrcShiftB))) & (0xff >> SrcShiftB);
							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
			}
		}
		else
		{
			// alpha and/or coloring case

			// clamp R,G,B and inverse A to 0-256 range
			u32 const sr = u32(std::clamp(256.0f * prim.color.r, 0.0f, 256.0f));
			u32 const sg = u32(std::clamp(256.0f * prim.color.g, 0.0f, 256.0f));
			u32 const sb = u32(std::clamp(256.0f * prim.color.b, 0.0f, 256.0f));
			u32 const sa = u32(std::clamp(256.0f * prim.color.a, 0.0f, 256.0f));

			// loop over rows
			for (s32 y = setup.starty; y < setup.endy; y++)
			{
				PixelType *dest = dstdata + y * pitch + setup.startx;
				s32 curu = setup.startu + (y - setup.starty) * setup.dudy;
				s32 curv = setup.startv + (y - setup.starty) * setup.dvdy;

				if (!palbase)
				{
					// no lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const ta = (pix >> 24) * sa;
						if (ta != 0)
						{
							u32 const dpix = NoDestRead ? 0 : *dest;
							u32 r = ((source32_r(pix) * sr * ta) >> 24) + dest_r(dpix);
							u32 g = ((source32_g(pix) * sg * ta) >> 24) + dest_g(dpix);
							u32 b = ((source32_b(pix) * sb * ta) >> 24) + dest_b(dpix);
							r = (r | -(r >> (8 - SrcShiftR))) & (0xff >> SrcShiftR);
							g = (g | -(g >> (8 - SrcShiftG))) & (0xff >> SrcShiftG);
							b = (b | -(b >> (8 - SrcShiftB))) & (0xff >> SrcShiftB);
							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += setup.dudx;
						curv += setup.dvdx;
					}
				}
				else
				{
					// lookup case

					// loop over cols
					for (s32 x = setup.startx; x < setup.endx; x++)
					{
						u32 const pix = get_texel_argb32<Wrap>(prim.texture, curu, curv);
						u32 const ta = (pix >> 24) * sa;
						if (ta != 0)
						{
							u32 const dpix = NoDestRead ? 0 : *dest;
							u32 r = ((palbase[(pix >> 16) & 0xff] * sr * ta) >> (24 + SrcShiftR)) + dest_r(dpix);
							u32 g = ((palbase[(pix >> 8) & 0xff] * sr * ta) >> (24 + SrcShiftR)) + dest_g(dpix);
							u32 b = ((palbase[(pix >> 0) & 0xff] * sr * ta) >> (24 + SrcShiftR)) + dest_b(dpix);
							r = (r | -(r >> (8 - SrcShiftR))) & (0xff >> SrcShiftR);
							g = (g | -(g >> (8 - SrcShiftG))) & (0xff >> SrcShiftG);
							b = (b | -(b >> (8 - SrcShiftB))) & (0xff >> SrcShiftB);
							*dest = dest_assemble_rgb(r, g, b);
						}
						dest++;
						curu += setup.dudx;
						curv += setup.dvdx;
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

	static void setup_and_draw_textured_quad(render_primitive const &prim, PixelType *dstdata, s32 width, s32 height, u32 pitch)
	{
		assert(prim.bounds.x0 <= prim.bounds.x1);
		assert(prim.bounds.y0 <= prim.bounds.y1);

		// determine U/V deltas
		float const fdudx = (prim.texcoords.tr.u - prim.texcoords.tl.u) / (prim.bounds.x1 - prim.bounds.x0);
		float const fdvdx = (prim.texcoords.tr.v - prim.texcoords.tl.v) / (prim.bounds.x1 - prim.bounds.x0);
		float const fdudy = (prim.texcoords.bl.u - prim.texcoords.tl.u) / (prim.bounds.y1 - prim.bounds.y0);
		float const fdvdy = (prim.texcoords.bl.v - prim.texcoords.tl.v) / (prim.bounds.y1 - prim.bounds.y0);

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
		if constexpr (BilinearFilter)
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
				if (PRIMFLAG_GET_TEXWRAP(prim.flags))
					draw_quad_rgb32<true>(prim, dstdata, pitch, setup);
				else
					draw_quad_rgb32<false>(prim, dstdata, pitch, setup);
				break;

			case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_RGB_MULTIPLY):
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_RGB_MULTIPLY):
				if (PRIMFLAG_GET_TEXWRAP(prim.flags))
					draw_quad_rgb32_multiply<true>(prim, dstdata, pitch, setup);
				else
					draw_quad_rgb32_multiply<false>(prim, dstdata, pitch, setup);
				break;

			case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_ADD):
				if (PRIMFLAG_GET_TEXWRAP(prim.flags))
					draw_quad_rgb32_add<true>(prim, dstdata, pitch, setup);
				else
					draw_quad_rgb32_add<false>(prim, dstdata, pitch, setup);
				break;

			case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA):
				if (PRIMFLAG_GET_TEXWRAP(prim.flags))
					draw_quad_argb32_alpha<true>(prim, dstdata, pitch, setup);
				else
					draw_quad_argb32_alpha<false>(prim, dstdata, pitch, setup);
				break;

			case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_ADD):
				if (PRIMFLAG_GET_TEXWRAP(prim.flags))
					draw_quad_argb32_add<true>(prim, dstdata, pitch, setup);
				else
					draw_quad_argb32_add<false>(prim, dstdata, pitch, setup);
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
	static void draw_primitives(render_primitive_list const &primlist, void *dstdata, u32 width, u32 height, u32 pitch)
	{
		// loop over the list and render each element
		for (render_primitive const *prim = primlist.first(); prim != nullptr; prim = prim->next())
			switch (prim->type)
			{
				case render_primitive::LINE:
					draw_line(*prim, reinterpret_cast<PixelType *>(dstdata), width, height, pitch);
					break;

				case render_primitive::QUAD:
					if (!prim->texture.base)
						draw_rect(*prim, reinterpret_cast<PixelType *>(dstdata), width, height, pitch);
					else
						setup_and_draw_textured_quad(*prim, reinterpret_cast<PixelType *>(dstdata), width, height, pitch);
					break;

				default:
					throw emu_fatalerror("Unexpected render_primitive type");
			}
	}
};
