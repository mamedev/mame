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

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/


void k053247_device::clear_all()
{
	m_ram = nullptr;
	m_gfx = nullptr;

	for (auto & elem : m_kx46_regs)
		elem = 0;

	for (auto & elem : m_kx47_regs)
		elem = 0;

	m_objcha_line = 0;
	m_z_rejection = 0;

	m_memory_region = nullptr;
}

void k053247_device::k053247_get_ram( UINT16 **ram )
{
	*ram = m_ram;
}

int k053247_device::k053247_get_dx( void )
{
	return m_dx;
}

int k053247_device::k053247_get_dy( void )
{
	return m_dy;
}

int k053247_device::k053246_read_register( int regnum )
{
	return(m_kx46_regs[regnum]);
}

int k053247_device::k053247_read_register( int regnum )
{
	return(m_kx47_regs[regnum]);
}


WRITE16_MEMBER( k053247_device::k055673_reg_word_w ) // write-only OBJSET2 registers (see p.43 table 6.1)
{
	COMBINE_DATA(m_kx47_regs + offset);
}

READ16_MEMBER( k053247_device::k053247_word_r )
{
	return m_ram[offset];
}

WRITE16_MEMBER( k053247_device::k053247_word_w )
{
	COMBINE_DATA(m_ram + offset);
}

READ8_MEMBER( k053247_device::k053247_r )
{
	int offs = offset >> 1;

	if (offset & 1)
		return(m_ram[offs] & 0xff);
	else
		return(m_ram[offs] >> 8);
}

WRITE8_MEMBER( k053247_device::k053247_w )
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
READ16_MEMBER( k053247_device::k055673_5bpp_rom_word_r ) // 5bpp
{
	UINT8 *ROM8 = (UINT8 *)space.machine().root_device().memregion(m_memory_region)->base();
	UINT16 *ROM = (UINT16 *)space.machine().root_device().memregion(m_memory_region)->base();
	int size4 = (space.machine().root_device().memregion(m_memory_region)->bytes() / (1024 * 1024)) / 5;
	int romofs;

	size4 *= 4 * 1024 * 1024;   // get offset to 5th bit
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
			LOG(("55673_rom_word_r: Unknown read offset %x\n", offset));
			break;
	}

	return 0;
}

READ16_MEMBER( k053247_device::k055673_rom_word_r )
{
	if (m_bpp == 5)
		return k055673_5bpp_rom_word_r(space, offset, mem_mask);

	UINT16 *ROM = (UINT16 *)space.machine().root_device().memregion(m_memory_region)->base();
	int romofs;

	romofs = m_kx46_regs[6] << 16 | m_kx46_regs[7] << 8 | m_kx46_regs[4];

	romofs = (romofs >> 2) * m_bpp;

	if ((offset & 0x4) == 0) romofs += m_bpp >> 1;

	return ROM[romofs + (offset & 0x3)];
}

READ8_MEMBER( k053247_device::k053246_r )
{
	if (m_objcha_line == ASSERT_LINE)
	{
		int addr;

		addr = (m_kx46_regs[6] << 17) | (m_kx46_regs[7] << 9) | (m_kx46_regs[4] << 1) | ((offset & 1) ^ 1);
		addr &= space.machine().root_device().memregion(m_memory_region)->bytes() - 1;
//      if (VERBOSE)
//          popmessage("%04x: offset %02x addr %06x", space.device().safe_pc(), offset, addr);
		return space.machine().root_device().memregion(m_memory_region)->base()[addr];
	}
	else
	{
//      LOG(("%04x: read from unknown 053246 address %x\n", space.device().safe_pc(), offset));
		return 0;
	}
}

WRITE8_MEMBER( k053247_device::k053246_w )
{
	m_kx46_regs[offset] = data;
}

READ16_MEMBER( k053247_device::k053246_word_r )
{
	offset <<= 1;
	return k053246_r( space, offset + 1) | (k053246_r( space, offset) << 8);
}

WRITE16_MEMBER( k053247_device::k053246_word_w )
{
	if (ACCESSING_BITS_8_15)
		k053246_w( space, offset << 1,(data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		k053246_w( space, (offset << 1) + 1,data & 0xff);
}

void k053247_device::k053246_set_objcha_line( int state )
{
	m_objcha_line = state;
}

int k053247_device::k053246_is_irq_enabled( void )
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

template<class _BitmapClass>
void k053247_device::k053247_sprites_draw_common( _BitmapClass &bitmap, const rectangle &cliprect )
{
#define NUM_SPRITES 256


	int code, color, x, y, shadow, shdmask, count, temp, primask;

	int sortedlist[NUM_SPRITES];
	int offs,zcode;




	UINT8 drawmode_table[256];
	UINT8 shadowmode_table[256];


	memset(drawmode_table, DRAWMODE_SOURCE, sizeof(drawmode_table));
	drawmode_table[0] = DRAWMODE_NONE;
	memset(shadowmode_table, DRAWMODE_SHADOW, sizeof(shadowmode_table));
	shadowmode_table[0] = DRAWMODE_NONE;

	/*
	    safeguard older drivers missing any of the following video attributes:

	    VIDEO_HAS_SHADOWS | VIDEO_HAS_HIGHLIGHTS
	*/
	if (m_palette->shadows_enabled())
	{
		if (sizeof(typename _BitmapClass::pixel_t) == 4 && (m_palette->hilights_enabled()))
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

		k053247_draw_single_sprite_gxcore( bitmap, cliprect,
				nullptr, nullptr,
				code, m_ram, offs,
				color,
				/* gx only */
				0, 0, 0, 0,
				/* non-gx only */
				primask,shadow,drawmode_table,shadowmode_table,shdmask
				);



	} // end of sprite-list loop
#undef NUM_SPRITES
}

void k053247_device::k053247_sprites_draw( bitmap_ind16 &bitmap, const rectangle &cliprect )
{ k053247_sprites_draw_common( bitmap, cliprect); }

void k053247_device::k053247_sprites_draw( bitmap_rgb32 &bitmap, const rectangle &cliprect )
{ k053247_sprites_draw_common( bitmap, cliprect); }



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
		UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy,
		int scalex, int scaley, int alpha, int drawmode, int zcode, int pri, UINT8* gx_objzbuf, UINT8* gx_shdzbuf)
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
	granularity = shdpen = m_gfx->granularity();
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
	src_base  = m_gfx->get_data(code % m_gfx->elements());

	pal_base  = m_palette->pens() + m_gfx->colorbase() + (color % m_gfx->colors()) * granularity;
	shd_base  = m_palette->shadow_table();

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
				case 0: // all pens solid
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
							rgb_t pix = dst_ptr[ecx];
							szbuf_ptr[ecx*2] = z8;
							szbuf_ptr[ecx*2+1] = p8;

							// the shadow tables are 15-bit lookup tables which accept RGB15... lossy, nasty, yuck!
							dst_ptr[ecx] = shd_base[pix.as_rgb15()];
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
			}   // switch (drawmode)
		}   // if (zcode < 0)
	}   // if (!nozoom)
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
							rgb_t pix = dst_ptr[ecx];
							szbuf_ptr[ecx*2] = z8;
							szbuf_ptr[ecx*2+1] = p8;

							// the shadow tables are 15-bit lookup tables which accept RGB15... lossy, nasty, yuck!
							dst_ptr[ecx] = shd_base[pix.as_rgb15()];
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



void k053247_device::zdrawgfxzoom32GP(
		bitmap_ind16 &bitmap, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy,
		int scalex, int scaley, int alpha, int drawmode, int zcode, int pri, UINT8* gx_objzbuf, UINT8* gx_shdzbuf)
{
	fatalerror("no zdrawgfxzoom32GP for bitmap_ind16\n");
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/




const device_type K055673 = &device_creator<k055673_device>;

k055673_device::k055673_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: k053247_device(mconfig, K055673, "K053246 & K055673 Sprite Generator", tag, owner, clock, "k055673", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k055673_device::device_start()
{
	int gfx_index;
	UINT32 total;

	static const gfx_layout spritelayout =  /* System GX sprite layout */
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
	UINT8 *s1, *s2, *d;
	long i;
	UINT16 *alt_k055673_rom;
	int size4;

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (m_gfxdecode->gfx(gfx_index) == nullptr)
			break;
	assert(gfx_index != MAX_GFX_ELEMENTS);

	alt_k055673_rom = (UINT16 *)machine().root_device().memregion(m_memory_region)->base();

	/* decode the graphics */
	switch (m_bpp)
	{
		case K055673_LAYOUT_GX:
			size4 = (machine().root_device().memregion(m_memory_region)->bytes()/(1024*1024))/5;
			size4 *= 4*1024*1024;
			/* set the # of tiles based on the 4bpp section */
			alt_k055673_rom = auto_alloc_array(machine(), UINT16, size4 * 5 / 2);
			d = (UINT8 *)alt_k055673_rom;
			// now combine the graphics together to form 5bpp
			s1 = machine().root_device().memregion(m_memory_region)->base(); // 4bpp area
			s2 = s1 + (size4);   // 1bpp area
			for (i = 0; i < size4; i+= 4)
			{
				*d++ = *s1++;
				*d++ = *s1++;
				*d++ = *s1++;
				*d++ = *s1++;
				*d++ = *s2++;
			}

			total = size4 / 128;
			konami_decode_gfx(machine(), m_gfxdecode, m_palette, gfx_index, (UINT8 *)alt_k055673_rom, total, &spritelayout, 5);
			break;

		case K055673_LAYOUT_RNG:
			total = machine().root_device().memregion(m_memory_region)->bytes() / (16*16/2);
			konami_decode_gfx(machine(), m_gfxdecode, m_palette, gfx_index, (UINT8 *)alt_k055673_rom, total, &spritelayout2, 4);
			break;

		case K055673_LAYOUT_LE2:
			total = machine().root_device().memregion(m_memory_region)->bytes() / (16*16);
			konami_decode_gfx(machine(), m_gfxdecode, m_palette, gfx_index, (UINT8 *)alt_k055673_rom, total, &spritelayout3, 8);
			break;

		case K055673_LAYOUT_GX6:
			total = machine().root_device().memregion(m_memory_region)->bytes() / (16*16*6/8);
			konami_decode_gfx(machine(), m_gfxdecode, m_palette, gfx_index, (UINT8 *)alt_k055673_rom, total, &spritelayout4, 6);
			break;

		default:
			fatalerror("Unsupported layout\n");
	}

	if (VERBOSE && !(m_palette->shadows_enabled()))
		popmessage("driver should use VIDEO_HAS_SHADOWS");

	m_z_rejection = -1;
	m_gfx = m_gfxdecode->gfx(gfx_index);
	m_objcha_line = CLEAR_LINE;
	m_ram = auto_alloc_array(machine(), UINT16, 0x1000/2);

	memset(m_ram,  0, 0x1000);
	memset(m_kx46_regs, 0, 8);
	memset(m_kx47_regs, 0, 32);

	machine().save().save_pointer(NAME(m_ram), 0x800);
	machine().save().save_item(NAME(m_kx46_regs));
	machine().save().save_item(NAME(m_kx47_regs));
	machine().save().save_item(NAME(m_objcha_line));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------



const device_type K053246 = &device_creator<k053247_device>;

k053247_device::k053247_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K053246, "K053246 & K053247 Sprite Generator", tag, owner, clock, "k053247", __FILE__),
		device_video_interface(mconfig, *this),
		m_gfxdecode(*this),
		m_palette(*this)
{
	clear_all();
}

k053247_device::k053247_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_video_interface(mconfig, *this),
		m_gfxdecode(*this),
		m_palette(*this)
{
	clear_all();
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void k053247_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<k053247_device &>(device).m_gfxdecode.set_tag(tag);
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void k053247_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<k053247_device &>(device).m_palette.set_tag(tag);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k053247_device::device_start()
{
//  bitmap_rgb32* blah;
//  if (0)
//      k053247_draw_single_sprite_gxcore(blah,

	UINT32 total;
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
		total = machine().root_device().memregion(m_memory_region)->bytes() / 128;
		konami_decode_gfx(machine(), m_gfxdecode, m_palette, m_gfx_num, machine().root_device().memregion(m_memory_region)->base(), total, &spritelayout, 4);
		break;

	default:
		fatalerror("Unsupported plane_order\n");
	}

	if (VERBOSE)
	{
		if (m_screen->format() == BITMAP_FORMAT_RGB32)
		{
			if (!m_palette->shadows_enabled() || !m_palette->hilights_enabled())
				popmessage("driver missing SHADOWS or HIGHLIGHTS flag");
		}
		else
		{
			if (!(m_palette->shadows_enabled()))
				popmessage("driver should use VIDEO_HAS_SHADOWS");
		}
	}

	m_gfx = m_gfxdecode->gfx(m_gfx_num);

	m_ram = auto_alloc_array_clear(machine(), UINT16, 0x1000 / 2);

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

	memset(m_kx46_regs, 0, 8);
	memset(m_kx47_regs, 0, 32);
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

void k053247_device::k053247_set_z_rejection( int zcode )
{
	m_z_rejection = zcode;
}


READ16_MEMBER( k053247_device::k053246_reg_word_r )
{
	return(m_kx46_regs[offset * 2] << 8 | m_kx46_regs[offset * 2 + 1]);
}   // OBJSET1
