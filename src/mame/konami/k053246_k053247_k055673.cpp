// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************/
/*                                                                         */
/*                                 053246                                  */
/*                          with 053247 or 055673                          */
/*  is the 053247 / 055673 choice just a BPP change like the tilemaps?     */
/*                                                                         */
/*                                                                         */
/***************************************************************************/
/* later Konami GX board replaces the 053246 with a 058142 */

/*

053247/053246
-------------
Sprite generators. Nothing is known about their external interface.
The sprite RAM format is very similar to the 053245.

053246 memory map (but the 053247 sees and processes them too):
000-001 W  global X offset
002-003 W  global Y offset
004     W  low 8 bits of the ROM address to read
005     W  bit 0 = flip screen X
           bit 1 = flip screen Y
           bit 2 = unknown
           bit 4 = interrupt enable
           bit 5 = unknown
006-007 W  high 16 bits of the ROM address to read

???-??? R  reads data from the gfx ROMs (16 bits in total). The address of the
           data is determined by the registers above

*/

#include "emu.h"
#include "k053246_k053247_k055673.h"
#include "konami_helper.h"

#include <algorithm>

#define VERBOSE 0
#include "logmacro.h"


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k053247_device::clear_all()
{
	m_ram = nullptr;
	m_gfx = nullptr;

	std::fill(std::begin(m_kx46_regs), std::end(m_kx46_regs), 0);
	std::fill(std::begin(m_kx47_regs), std::end(m_kx47_regs), 0);

	m_objcha_line = 0;
	m_z_rejection = 0;
}

void k053247_device::k053247_get_ram(u16 **ram)
{
	*ram = m_ram.get();
}

int k053247_device::k053247_get_dx(void)
{
	return m_dx;
}

int k053247_device::k053247_get_dy(void)
{
	return m_dy;
}

u8 k053247_device::k053246_read_register(offs_t offset)
{
	return m_kx46_regs[offset];
}

u16 k053247_device::k053247_read_register(offs_t offset)
{
	return m_kx47_regs[offset];
}


void k053247_device::k055673_reg_word_w(offs_t offset, u16 data, u16 mem_mask) // write-only OBJSET2 registers (see p.43 table 6.1)
{
	COMBINE_DATA(m_kx47_regs + offset);
}

u16 k053247_device::k053247_word_r(offs_t offset)
{
	return m_ram[offset];
}

void k053247_device::k053247_word_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(m_ram.get() + offset);
}

u8 k053247_device::k053247_r(offs_t offset)
{
	int offs = offset >> 1;

	if (offset & 1)
		return(m_ram[offs] & 0xff);
	else
		return(m_ram[offs] >> 8);
}

void k053247_device::k053247_w(offs_t offset, u8 data)
{
	int offs = offset >> 1;

	if (offset & 1)
		m_ram[offs] = (m_ram[offs] & 0xff00) | data;
	else
		m_ram[offs] = (m_ram[offs] & 0x00ff) | (data << 8);
}

// The K055673 supports a non-objcha based ROM readback
// write the address to the 246 as usual, but there's a completely separate ROM
// window that works without needing an objcha line.
// in this window, +0 = 32 bits from one set of ROMs, and +8 = 32 bits from another set

// FIXME: rearrange ROM loading so this can be merged with the 4/6/8bpp version
u16 k053247_device::k055673_5bpp_rom_word_r(offs_t offset) // 5bpp
{
	u8 *ROM8 = (u8 *)&m_gfxrom[0];
	u16 *ROM = (u16 *)&m_gfxrom[0];
	int size4 = (m_gfxrom.length() / (1024 * 1024)) / 5;
	int romofs;

	size4 *= 4 * 1024 * 1024; // get offset to 5th bit
	ROM8 += size4;

	romofs = m_kx46_regs[6] << 16 | m_kx46_regs[7] << 8 | m_kx46_regs[4];

	switch (offset)
	{
		case 0: // 20k / 36u
			return ROM[romofs + 2];
		case 1: // 17k / 36y
			return ROM[romofs + 3];
		case 2: // 10k / 32y
		case 3:
			romofs /= 2;
			return ROM8[romofs + 1];
		case 4: // 22k / 34u
			return ROM[romofs];
		case 5: // 19k / 34y
			return ROM[romofs + 1];
		case 6: // 12k / 29y
		case 7:
			romofs /= 2;
			return ROM8[romofs];
		default:
			LOG("55673_rom_word_r: Unknown read offset %x\n", offset);
			break;
	}

	return 0;
}

u16 k053247_device::k055673_rom_word_r(offs_t offset)
{
	if (m_bpp == 5)
		return k055673_5bpp_rom_word_r(offset);

	u16 *ROM = (u16 *)&m_gfxrom[0];
	int romofs;

	romofs = m_kx46_regs[6] << 16 | m_kx46_regs[7] << 8 | m_kx46_regs[4];

	romofs = (romofs >> 2) * m_bpp;

	if ((offset & 0x4) == 0) romofs += m_bpp >> 1;

	return ROM[romofs + (offset & 0x3)];
}

u16 k053247_device::k055673_ps_rom_word_r(offs_t offset)
{
	u8 *ROM = (u8 *)&m_gfxrom[0];
	int romofs;
	int magic = (offset & 1);

	romofs = m_kx46_regs[6] << 16 | m_kx46_regs[7] << 8 | m_kx46_regs[4];
	offset = ((offset & 4) >> 1);

	int finoffs = (romofs * 2) + (offset * 2) + magic;

	return ROM[finoffs+2] | (ROM[finoffs]<<8);
}

u16 k053247_device::k055673_gr_rom_word_r(offs_t offset)
{
	const u8 *ROM = (u8 *)&m_gfxrom[0];
	const int romofs = m_kx46_regs[6] << 16 | m_kx46_regs[7] << 8 | m_kx46_regs[4];

	offset = ((offset & 4) >> 1);

	const int finoffs = (romofs * 2) + (offset * 2);

	return ROM[finoffs + 1] | (ROM[finoffs] << 8);
}

u8 k053247_device::k053246_r(offs_t offset)
{
	if (m_objcha_line == ASSERT_LINE)
	{
		int addr = (m_kx46_regs[6] << 17) | (m_kx46_regs[7] << 9) | (m_kx46_regs[4] << 1) | ((offset & 1) ^ 1);

		// assumes it can make an address mask with m_gfxrom.length() - 1
		assert(!(m_gfxrom.length() & (m_gfxrom.length() - 1)));
		addr &= m_gfxrom.length() - 1;

		return m_gfxrom[addr];
	}
	else
	{
		return 0;
	}
}

void k053247_device::k053246_w(offs_t offset, u8 data)
{
	m_kx46_regs[offset] = data;
}

void k053247_device::k053246_set_objcha_line(int state)
{
	m_objcha_line = state;
}

int k053247_device::k053246_is_irq_enabled(void)
{
	// This bit enables obj DMA rather than obj IRQ even though the two functions usually coincide.
	return m_kx46_regs[5] & 0x10;
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

template<class BitmapClass>
void k053247_device::k053247_sprites_draw_common(BitmapClass &bitmap, const rectangle &cliprect)
{
	static constexpr int NUM_SPRITES = 256;

	int code, color, x, y, shadow, shdmask, count, temp, primask;

	int sortedlist[NUM_SPRITES];
	int offs,zcode;

	u8 drawmode_table[256];
	u8 shadowmode_table[256];

	memset(drawmode_table, DRAWMODE_SOURCE, sizeof(drawmode_table));
	drawmode_table[0] = DRAWMODE_NONE;
	memset(shadowmode_table, DRAWMODE_SHADOW, sizeof(shadowmode_table));
	shadowmode_table[0] = DRAWMODE_NONE;

	/*
	    safeguard older drivers missing any of the following video attributes:
	    VIDEO_HAS_SHADOWS | VIDEO_HAS_HIGHLIGHTS
	*/
	if (palette().shadows_enabled())
	{
		if (sizeof(typename BitmapClass::pixel_t) == 4 && (palette().highlights_enabled()))
			shdmask = 3; // enable all shadows and highlights
		else
			shdmask = 0; // enable default shadows
	}
	else
		shdmask = -1; // disable everything

	/*
	    The k053247 does not draw pixels on top of those with equal or smaller Z-values
	    regardless of priority. Embedded shadows inherit Z-values from their host sprites
	    but do not assume host priorities unless explicitly told. In other words shadows
	    can have priorities different from that of normal pens in the same sprite,
	    in addition to the ability of masking themselves from specific layers or pixels
	    on the other sprites.

	    In front-to-back rendering, sprites cannot sandwich between alpha blended layers
	    or the draw code will have to figure out the percentage opacities of what is on
	    top and beneath each sprite pixel and blend the target accordingly. The process
	    is overly demanding for realtime software and is thus another shortcoming of
	    pdrawgfx and pixel based mixers. Even mahjong games with straight forward video
	    subsystems are feeling the impact by which the girls cannot appear under
	    translucent dialogue boxes.

	    These are a small part of the k053247's feature set but many games expect them
	    to be the minimum compliances. The specification will undoubtedly require
	    redesigning the priority system from the ground up. Drawgfx.c and tilemap.c must
	    also undergo heavy facelifts but in the end the changes could hurt simpler games
	    more than they help complex systems; therefore the new engine should remain
	    completely stand alone and self-contained. Implementation details are being
	    hammered down but too early to make propositions.
	*/

	// Prebuild a sorted table by descending Z-order.
	zcode = m_z_rejection;
	offs = count = 0;

	if (zcode == -1)
	{
		for (; offs < 0x800; offs += 8)
			if (m_ram[offs] & 0x8000)
				sortedlist[count++] = offs;
	}
	else
	{
		for (; offs < 0x800; offs += 8)
			if ((m_ram[offs] & 0x8000) && ((m_ram[offs] & 0xff) != zcode))
				sortedlist[count++] = offs;
	}

	int w = count;
	count--;
	int h = count;

	if (!(m_kx47_regs[0xc / 2] & 0x10))
	{
		// sort objects in decending order(smaller z closer) when OPSET PRI is clear
		for (y = 0; y < h; y++)
		{
			offs = sortedlist[y];
			zcode = m_ram[offs] & 0xff;
			for (x = y + 1; x < w; x++)
			{
				temp = sortedlist[x];
				code = m_ram[temp] & 0xff;
				if (zcode <= code)
				{
					zcode = code;
					sortedlist[x] = offs;
					sortedlist[y] = offs = temp;
				}
			}
		}
	}
	else
	{
		// sort objects in ascending order(bigger z closer) when OPSET PRI is set
		for (y = 0; y < h; y++)
		{
			offs = sortedlist[y];
			zcode = m_ram[offs] & 0xff;
			for (x = y + 1; x < w; x++)
			{
				temp = sortedlist[x];
				code = m_ram[temp] & 0xff;
				if (zcode >= code)
				{
					zcode = code;
					sortedlist[x] = offs;
					sortedlist[y] = offs = temp;
				}
			}
		}
	}

	for (; count >= 0; count--)
	{
		offs = sortedlist[count];

		code = m_ram[offs + 1];
		shadow = color = m_ram[offs + 6];
		primask = 0;

		m_k053247_cb(&code, &color, &primask);

		k053247_draw_single_sprite_gxcore(bitmap, cliprect,
				nullptr, nullptr,
				code, m_ram.get(), offs,
				color,
				/* gx only */
				0, 0, 0, 0,
				/* non-gx only */
				primask,shadow,drawmode_table,shadowmode_table,shdmask);
	} // end of sprite-list loop
}

void k053247_device::k053247_sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{ k053247_sprites_draw_common(bitmap, cliprect); }

void k053247_device::k053247_sprites_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{ k053247_sprites_draw_common(bitmap, cliprect); }


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

void k053247_device::zdrawgfxzoom32GP(
		bitmap_rgb32 &bitmap, const rectangle &cliprect,
		u32 code, u32 color, bool flipx, bool flipy, int sx, int sy,
		int scalex, int scaley, int alpha, int drawmode, int zcode, u8 pri, u8* gx_objzbuf, u8* gx_shdzbuf)
{
	constexpr u8 FP = 19; // 13.19 fixed point

	// cull objects with invalid scale
	if (!scalex || !scaley) return;

	// find shadow pens and cull invisible shadows
	const u16 granularity = m_gfx->granularity();
	int shdpen = granularity - 1;

	if (zcode >= 0)
	{
		if (drawmode == 5)
		{
			drawmode = 4;
			shdpen = 1;
		}
	}
	else if (drawmode >= 4) return;

	// alpha blend check: cull if 0% opaque, or skip alpha blending if 100% opaque
	if (drawmode & 2)
	{
		if (!alpha) return;
		if (alpha == 255) drawmode &= ~2;
	}

	rectangle dst_rect = rectangle {sx, 0, sy, 0};
	dst_rect.set_size(((scalex << 4) + 0x8000) >> 16, ((scaley << 4) + 0x8000) >> 16);

	// cull off-screen and zero height/width objects
	if (dst_rect.left() > cliprect.right() || dst_rect.right() < cliprect.left()) return;
	if (dst_rect.top() > cliprect.bottom() || dst_rect.bottom() < cliprect.top()) return;
	if (!dst_rect.width() || !dst_rect.height()) return;

	constexpr u8 src_size = 16;
	const int src_stride_x = (src_size << FP) / dst_rect.width();
	const int src_stride_y = (src_size << FP) / dst_rect.height();

	const int src_offset_x = std::max(cliprect.left() - dst_rect.left(), 0);
	const int src_offset_y = std::max(cliprect.top() - dst_rect.top(), 0);

	const int src_base_x = src_offset_x * src_stride_x;
	const int src_base_y = src_offset_y * src_stride_y;

	// exclude dst_rect area outside cliprect
	dst_rect &= cliprect;

	// invert src x/y offsets if flip enabled
	u8 flip_mask = 0;
	if (flipx) flip_mask |= src_size - 1;
	if (flipy) flip_mask |= (src_size - 1) << 4;

	const int dst_pitch = bitmap.rowpixels();
	u32 *dst_ptr = &bitmap.pix(0) + dst_rect.left() + dst_rect.top() * dst_pitch;

	const u8 *src_base = m_gfx->get_data(code % m_gfx->elements());
	const pen_t *pal_base = palette().pens() + m_gfx->colorbase() + (color % m_gfx->colors()) * granularity;

	const int z_buffer_offset = (dst_rect.top() - cliprect.top()) * GX_ZBUFW + (dst_rect.left() - cliprect.left());
	u8 *ozbuf_ptr = gx_objzbuf + z_buffer_offset;

	const u8 z8 = (u8)zcode;

	if (zcode < 0)
	{
		// no shadow and z-buffering

		for (int y = 0; y < dst_rect.height(); ++y)
		{
			for (int x = 0; x < dst_rect.width(); ++x)
			{
				const int x_off = (src_base_x + x * src_stride_x) >> FP;
				const int y_off = (src_base_y + y * src_stride_y) >> FP;
				const u8 pal_idx = src_base[(x_off + y_off * 16) ^ flip_mask];
				if (!pal_idx || pal_idx >= shdpen) continue;
				ozbuf_ptr[x + y * GX_ZBUFW] = z8;
				dst_ptr[x + y * dst_pitch] = pal_base[pal_idx];
			}
		}
	}
	else if (drawmode < 4)
	{
		// 0: all pens solid
		// 1: solid pens only
		// 2: all pens solid with alpha blending
		// 3: solid pens only with alpha blending

		for (int y = 0; y < dst_rect.height(); ++y)
		{
			for (int x = 0; x < dst_rect.width(); ++x)
			{
				const int x_off = (src_base_x + x * src_stride_x) >> FP;
				const int y_off = (src_base_y + y * src_stride_y) >> FP;
				const u8 pal_idx = src_base[(x_off + y_off * 16) ^ flip_mask];
				if (!pal_idx || (drawmode & 0b01 && pal_idx >= shdpen) || ozbuf_ptr[x + y * GX_ZBUFW] < z8) continue;
				ozbuf_ptr[x + y * GX_ZBUFW] = z8;

				if ((drawmode & 0b10) == 0) // solid sprite
				{
					dst_ptr[x + y * dst_pitch] = pal_base[pal_idx];
				}
				else // alpha blended sprite
				{
					const u8 alpha_level = alpha;
					const bool additive_mode = alpha & (1 << 8);
					// mix_pri flips src & dst
					// todo: find a game that exhibits this behavior
					// const bool mix_pri = alpha & (1 << 9);

					const u32 src = pal_base[pal_idx];
					const u32 dst = dst_ptr[x + y * dst_pitch];

					if (additive_mode)
					{
						// todo: improve additive blend calculation
						const u32 temp = alpha_blend_r32(src, 0, alpha_level);
						dst_ptr[x + y * dst_pitch] = add_blend_r32(dst, temp);
					}
					else
					{
						dst_ptr[x + y * dst_pitch] = alpha_blend_r32(dst, src, alpha_level);
					}
				}
			}
		}
	}
	else
	{
		// 4: shadow pens only

		const pen_t *shd_base = palette().shadow_table();
		u8 *szbuf_ptr = gx_shdzbuf + z_buffer_offset * 2;

		for (int y = 0; y < dst_rect.height(); ++y)
		{
			for (int x = 0; x < dst_rect.width(); ++x)
			{
				const int x_off = (src_base_x + x * src_stride_x) >> FP;
				const int y_off = (src_base_y + y * src_stride_y) >> FP;
				const u8 pal_idx = src_base[(x_off + y_off * 16) ^ flip_mask];
				const int szbuf_offset = x * 2 + y * GX_ZBUFW * 2;
				if (pal_idx < shdpen || szbuf_ptr[szbuf_offset] < z8 || szbuf_ptr[szbuf_offset + 1] <= pri) continue;
				szbuf_ptr[szbuf_offset] = z8;
				szbuf_ptr[szbuf_offset + 1] = pri;

				rgb_t pix = dst_ptr[x + y * dst_pitch];
				// the shadow tables are 15-bit lookup tables which accept RGB15... lossy, nasty, yuck!
				dst_ptr[x + y * dst_pitch] = shd_base[pix.as_rgb15()];
				//dst_ptr[ecx] =(eax>>3&0x001f);lend_r32(eax, 0x00000000, 128);
			}
		}
	}
}


void k053247_device::zdrawgfxzoom32GP(
		bitmap_ind16 &bitmap, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, int sx, int sy,
		int scalex, int scaley, int alpha, int drawmode, int zcode, int pri, u8* gx_objzbuf, u8* gx_shdzbuf)
{
	fatalerror("no zdrawgfxzoom32GP for bitmap_ind16\n");
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/


DEFINE_DEVICE_TYPE(K055673, k055673_device, "k055673", "K055673 Sprite Generator")

k055673_device::k055673_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: k053247_device(mconfig, K055673, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k055673_device::device_start()
{
	if (!palette().device().started())
		throw device_missing_dependencies();

	// resolve callbacks
	m_k053247_cb.resolve();

	int gfx_index = 0;
	u32 total;

	static const gfx_layout spritelayout = /* System GX sprite layout */
	{
		16,16,
		0,
		5,
		{ 32, 24, 16, 8, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7, 40, 41, 42, 43, 44, 45, 46, 47 },
		{ 0, 10*8, 10*8*2, 10*8*3, 10*8*4, 10*8*5, 10*8*6, 10*8*7, 10*8*8,
			10*8*9, 10*8*10, 10*8*11, 10*8*12, 10*8*13, 10*8*14, 10*8*15 },
		16*16*5
	};
	static const gfx_layout spritelayout2 = /* Run and Gun sprite layout */
	{
		16,16,
		0,
		4,
		{ 24, 16, 8, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7, 32, 33, 34, 35, 36, 37, 38, 39 },
		{ 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 },
		16*16*4
	};
	static const gfx_layout spritelayout3 = /* Lethal Enforcers II sprite layout */
	{
		16,16,
		0,
		8,
		{ 56, 48, 40, 32, 24, 16, 8, 0 },
		{  0,1,2,3,4,5,6,7,64+0,64+1,64+2,64+3,64+4,64+5,64+6,64+7 },
		{ 128*0, 128*1, 128*2,  128*3,  128*4,  128*5,  128*6,  128*7,
			128*8, 128*9, 128*10, 128*11, 128*12, 128*13, 128*14, 128*15 },
		16*16*8
	};
	static const gfx_layout spritelayout4 = /* System GX 6bpp sprite layout */
	{
		16,16,
		0,
		6,
		{ 40, 32, 24, 16, 8, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7, 48, 49, 50, 51, 52, 53, 54, 55 },
		{ 0, 12*8, 12*8*2, 12*8*3, 12*8*4, 12*8*5, 12*8*6, 12*8*7, 12*8*8,
			12*8*9, 12*8*10, 12*8*11, 12*8*12, 12*8*13, 12*8*14, 12*8*15 },
		16*16*6
	};
	static const gfx_layout spritelayout5 = /* Pirate Ship layout */
	{
		16,16,
		0,
		4,
		{ 24, 8, 16, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7, 32, 33, 34, 35, 36, 37, 38, 39 },
		{ 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 },
		16*16*4
	};
	u8 *s1, *s2, *d;
	long i;
	u16 *alt_k055673_rom;
	int size4;

	alt_k055673_rom = (u16 *)&m_gfxrom[0];

	/* decode the graphics */
	switch (m_bpp)
	{
		case K055673_LAYOUT_GX:
			size4 = (m_gfxrom.length() / 0x100000) / 5;
			size4 *= 0x400000;
			/* set the # of tiles based on the 4bpp section */
			m_combined_gfx = std::make_unique<u16[]>(size4 * 5 / 2);
			alt_k055673_rom = m_combined_gfx.get();
			d = (u8 *)alt_k055673_rom;
			// now combine the graphics together to form 5bpp
			s1 = (u8 *)&m_gfxrom[0]; // 4bpp area
			s2 = s1 + (size4); // 1bpp area
			for (i = 0; i < size4; i+= 4)
			{
				*d++ = *s1++;
				*d++ = *s1++;
				*d++ = *s1++;
				*d++ = *s1++;
				*d++ = *s2++;
			}

			total = size4 / 128;
			konami_decode_gfx(*this, gfx_index, (u8 *)alt_k055673_rom, total, &spritelayout, 5);
			break;

		case K055673_LAYOUT_RNG:
			total = m_gfxrom.length() / (16*16/2);
			konami_decode_gfx(*this, gfx_index, (u8 *)alt_k055673_rom, total, &spritelayout2, 4);
			break;

		case K055673_LAYOUT_PS:
			total = m_gfxrom.length() / (16*16/2);
			konami_decode_gfx(*this, gfx_index, (u8 *)alt_k055673_rom, total, &spritelayout5, 4);
			break;

		case K055673_LAYOUT_LE2:
			total = m_gfxrom.length() / (16*16);
			konami_decode_gfx(*this, gfx_index, (u8 *)alt_k055673_rom, total, &spritelayout3, 8);
			break;

		case K055673_LAYOUT_GX6:
			total = m_gfxrom.length() / (16*16*6/8);
			konami_decode_gfx(*this, gfx_index, (u8 *)alt_k055673_rom, total, &spritelayout4, 6);
			break;

		default:
			fatalerror("Unsupported layout\n");
	}

	if (VERBOSE && !(palette().shadows_enabled()))
		popmessage("driver should use VIDEO_HAS_SHADOWS");

	m_z_rejection = -1;
	m_gfx = gfx(gfx_index);
	m_objcha_line = CLEAR_LINE;
	m_ram = std::make_unique<u16[]>(0x4000/2);

	memset(m_ram.get(), 0, 0x4000);
	std::fill(std::begin(m_kx46_regs), std::end(m_kx46_regs), 0);
	std::fill(std::begin(m_kx47_regs), std::end(m_kx47_regs), 0);

	save_pointer(NAME(m_ram), 0x2000);
	save_item(NAME(m_kx46_regs));
	save_item(NAME(m_kx47_regs));
	save_item(NAME(m_objcha_line));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------


DEFINE_DEVICE_TYPE(K053247, k053247_device, "k053247", "K053246/K053247 Sprite Generator")

k053247_device::k053247_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: k053247_device(mconfig, K053247, tag, owner, clock)
{
}

k053247_device::k053247_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_gfx_interface(mconfig, *this, nullptr)
	, m_k053247_cb(*this)
	, m_gfxrom(*this, DEVICE_SELF)
	, m_gfx_num(0)
{
	clear_all();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k053247_device::device_start()
{
	if (!palette().device().started())
		throw device_missing_dependencies();

	// resolve callbacks
	m_k053247_cb.resolve();

	u32 total;
	static const gfx_layout spritelayout =
	{
		16,16,
		0,
		4,
		{ 0, 1, 2, 3 },
		{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4,
				10*4, 11*4, 8*4, 9*4, 14*4, 15*4, 12*4, 13*4 },
		{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
				8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
		128*8
	};

	/* decode the graphics */
	switch (m_bpp)
	{
	case NORMAL_PLANE_ORDER:
		total = m_gfxrom.length() / 128;
		konami_decode_gfx(*this, m_gfx_num, (u8 *)&m_gfxrom[0], total, &spritelayout, 4);
		break;

	default:
		fatalerror("Unsupported plane_order\n");
	}

	if (VERBOSE)
	{
		if (screen().format() == BITMAP_FORMAT_RGB32)
		{
			if (!palette().shadows_enabled() || !palette().highlights_enabled())
				popmessage("driver missing SHADOWS or HIGHLIGHTS flag");
		}
		else
		{
			if (!(palette().shadows_enabled()))
				popmessage("driver should use VIDEO_HAS_SHADOWS");
		}
	}

	m_gfx = gfx(m_gfx_num);

	m_ram = make_unique_clear<u16[]>(0x1000 / 2);

	save_pointer(NAME(m_ram), 0x1000 / 2);
	save_item(NAME(m_kx46_regs));
	save_item(NAME(m_kx47_regs));
	save_item(NAME(m_objcha_line));
	save_item(NAME(m_z_rejection));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k053247_device::device_reset()
{
	m_z_rejection = -1;
	m_objcha_line = CLEAR_LINE;

	std::fill(std::begin(m_kx46_regs), std::end(m_kx46_regs), 0);
	std::fill(std::begin(m_kx47_regs), std::end(m_kx47_regs), 0);
}


/*
    In a K053247+K055555 setup objects with Z-code 0x00 should be ignored
    when PRFLIP is cleared, while objects with Z-code 0xff should be
    ignored when PRFLIP is set.

    These behaviors can also be seen in older K053245(6)+K053251 setups.
    Bucky'O Hare, The Simpsons and Sunset Riders rely on their implications
    to prepare and retire sprites. They probably apply to many other Konami
    games but it's hard to tell because most artifacts have been filtered
    by exclusion sort.

    A driver may call K05324x_set_z_rejection() to set which zcode to ignore.
    Parameter:
               -1 = accept all(default)
        0x00-0xff = zcode to ignore
*/

void k053247_device::k053247_set_z_rejection(int zcode)
{
	m_z_rejection = zcode;
}
