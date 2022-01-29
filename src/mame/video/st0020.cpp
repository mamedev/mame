// license:BSD-3-Clause
// copyright-holders:Luca Elia,David Haywood
/************************************************************************************************************

  ST0020 - Seta Zooming Sprites + 4 Tilemaps + Blitter

  ST0032 seems very similar, used by the newer Jockey Club II boards

  The tilemaps are used by jclub2, while gdfs uses its own tilemap.

  To do:

  - fix visible area in jclub2 under non-wide monitor setting.

************************************************************************************************************/

#include "emu.h"
#include "st0020.h"

#include "screen.h"


DEFINE_DEVICE_TYPE(ST0020_SPRITES, st0020_device, "st0020", "Seta ST0020 Sprites")


#define ST0020_ST0032_BYTESWAP_DATA() \
		do { if (m_is_st0032) data = ((data & 0x00ff)<<8) | ((data & 0xff00)>>8); } while (false)
#define ST0020_ST0032_BYTESWAP_MEM_MASK() \
		do { if (m_is_st0032) mem_mask = ((mem_mask & 0x00ff)<<8) | ((mem_mask & 0xff00)>>8); } while (false)


st0020_device::st0020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ST0020_SPRITES, tag, owner, clock),
	device_gfx_interface(mconfig, *this),
	m_rom_ptr(*this, DEVICE_SELF)
{
	m_is_st0032 = 0;
	m_is_jclub2 = 0;
}

void st0020_device::device_reset()
{
	m_gfxram_bank = 0;
}

void st0020_device::device_start()
{
	if (!palette().device().started())
		throw device_missing_dependencies();

	// Allocate RAM
	m_gfxram    =   make_unique_clear<uint16_t[]>(4 * 0x100000/2);
	m_spriteram =   make_unique_clear<uint16_t[]>(0x80000/2);
	m_regs      =   make_unique_clear<uint16_t[]>(0x100/2);

	// Gfx element
	const int granularity = 16;
	const gfx_layout layout_16x8x8 =
	{
		16,8,
		0x400000/(16*8),
		8,
		{ STEP8(0,1)    },
		{ STEP16(0,8)   },
		{ STEP8(0,16*8) },
		16*8*8
	};
	set_gfx(0, std::make_unique<gfx_element>(&palette(), layout_16x8x8,  (uint8_t *)m_gfxram.get(), 0, palette().entries() / granularity, 0));
	gfx(0)->set_granularity(granularity); /* 256 colour sprites with palette selectable on 64 colour boundaries */

	// Tilemaps
	m_tmap[0] = &machine().tilemap().create(
				*this, tilemap_get_info_delegate(*this, FUNC(st0020_device::get_tile_info<0>)), tilemap_mapper_delegate(*this, FUNC(st0020_device::scan_16x16)), 16,8, 0x40,0x40*2);
	m_tmap[1] = &machine().tilemap().create(
				*this, tilemap_get_info_delegate(*this, FUNC(st0020_device::get_tile_info<1>)), tilemap_mapper_delegate(*this, FUNC(st0020_device::scan_16x16)), 16,8, 0x40,0x40*2);
	m_tmap[2] = &machine().tilemap().create(
				*this, tilemap_get_info_delegate(*this, FUNC(st0020_device::get_tile_info<2>)), tilemap_mapper_delegate(*this, FUNC(st0020_device::scan_16x16)), 16,8, 0x40,0x40*2);
	m_tmap[3] = &machine().tilemap().create(
				*this, tilemap_get_info_delegate(*this, FUNC(st0020_device::get_tile_info<3>)), tilemap_mapper_delegate(*this, FUNC(st0020_device::scan_16x16)), 16,8, 0x40,0x40*2);
	for (int i = 0; i < 4; ++i)
	{
		m_tmap[i]->set_transparent_pen(0);
//      m_tmap[i]->set_scrolldy(-0x301, 0);
	}

	// Save state
	save_pointer(NAME(m_gfxram),      4 * 0x100000/2);
	save_pointer(NAME(m_spriteram),   0x80000/2);
	save_pointer(NAME(m_regs),        0x100/2);
	save_item(NAME(m_gfxram_bank));
}


// Gfx ram
uint16_t st0020_device::gfxram_r(offs_t offset, uint16_t mem_mask)
{
	ST0020_ST0032_BYTESWAP_MEM_MASK();

	uint16_t data = m_gfxram[offset + m_gfxram_bank * 0x100000/2];

	ST0020_ST0032_BYTESWAP_DATA();
	return data;
}

void st0020_device::gfxram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	ST0020_ST0032_BYTESWAP_MEM_MASK();
	ST0020_ST0032_BYTESWAP_DATA();

	offset += m_gfxram_bank * 0x100000/2;
	COMBINE_DATA(&m_gfxram[offset]);
	gfx(0)->mark_dirty(offset / (16*8/2));
}

void st0020_device::gfxram_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data = COMBINE_DATA(&m_regs[offset]);

	if (data & ~0x43)
		logerror("%s: Unknown gfxram_bank bit written %04X\n", machine().describe_context(), data);

	if (ACCESSING_BITS_0_7)
		m_gfxram_bank = data & 3;
}


// Tilemaps
int st0020_device::tmap_offset(int i)
{
	return m_is_st0032  ?   (m_regs[i * 16/2 + 0x28/2] & 0x007c) * 0x1000/2 :
							(m_regs[i *  8/2         ] & 0x7c00) * 0x10/2   ;
}

int st0020_device::tmap_priority(int i)
{
	return m_is_st0032  ?   (m_regs[i * 16/2 + 0x24/2] & 0x0fc0) >> 6   :
							(m_regs[i *  8/2 + 0x04/2] & 0x0fc0) >> 6   ;
}

int st0020_device::tmap_is_enabled(int i)
{
	// jclub2 uses 0x19/0x00 for used/unused tilemaps
	return m_is_st0032  ?   m_regs[i * 16/2 + 0x24/2] & 0x0001  :
							m_regs[i *  8/2 + 0x04/2] & 0x0001  ;
}

/***************************************************************************

    Tile format

    Offset:     Bits:                   Value:

        0.w                             Tile Code

        2.w     fedc ba-- ---- ----
                ---- --9- ---- ----     0/1 = 256/64-Color Granularity *
                ---- ---8 7654 3210     Color

    * ST-0032 only?
      ST-0020 uses a tilemap flag to switch between 32/128-color granularity

***************************************************************************/

template<int Layer>
TILE_GET_INFO_MEMBER(st0020_device::get_tile_info)
{
	int offset = tmap_offset(Layer) + (tile_index & ~1);
	uint16_t tile   =   m_spriteram[offset + 0] + (tile_index & 1);
	uint16_t color  =   m_spriteram[offset + 1];

	if (m_is_st0032)    color = (color & 0x1ff) * ((color & 0x200) ? 4 : 16);
	else                color = color * ((m_regs[Layer * 4 + 3] & 0x0100) ? 2 : 8);

	tileinfo.set(0, tile, color, 0);
}

TILEMAP_MAPPER_MEMBER(st0020_device::scan_16x16)
{
	return (row & 1) | ((col & 0x3f) << 1) | ((row & ~1) << 6);
}

// Sprite RAM
uint16_t st0020_device::sprram_r(offs_t offset)
{
	return m_spriteram[offset];
}

void st0020_device::sprram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_spriteram[offset]);

	for (int i = 0; i < 4; ++i)
	{
		int tmap_offs = tmap_offset(i);
		if ((offset >= tmap_offs) && (offset < tmap_offs + 0x4000/2))
		{
			int tile_index = (offset - tmap_offs) & ~1;
			m_tmap[i]->mark_tile_dirty(tile_index);
			m_tmap[i]->mark_tile_dirty(tile_index + 1);
			// the same offset can be used by multiple tilemaps, so do not break the loop here
		}
	}
}

// Blitter
void st0020_device::do_blit_w(uint16_t data)
{
	uint32_t src  =   (m_regs[0xc0/2] + (m_regs[0xc2/2] << 16)) << 1;
	uint32_t dst  =   (m_regs[0xc4/2] + (m_regs[0xc6/2] << 16)) << 4;
	uint32_t len  =   (m_regs[0xc8/2]) << 4;

	if ( m_rom_ptr && (src+len <= m_rom_ptr.bytes()) && (dst+len <= 4 * 0x100000) )
	{
		memcpy( &m_gfxram[dst/2], &m_rom_ptr[src], len );

		if (len % (16*8))   len = len / (16*8) + 1;
		else                len = len / (16*8);

		dst /= 16*8;
		while (len--)
		{
			gfx(0)->mark_dirty(dst);
			dst++;
		}
	}
	else
	{
		logerror("%s: Blit out of range: src %x, dst %x, len %x\n", machine().describe_context(), src, dst, len);
	}
}

// Blitter / Tilemaps / CRTC registers

/***************************************************************************

    Tilemap Registers (8 bytes each for ST-0020, 16 bytes for ST-0032):

    Offset:     Bits:                   Value:

        0.w     f--- ---- ---- ----
                -edc ba-- ---- ----     Tile RAM offset / 4000 (ST-0032: low bits of offset 8)
                ---- --98 7654 3210     X scroll

        2.w     fedc ba-- ---- ----
                ---- --98 7654 3210     Y scroll

        4.w     fedc ---- ---- ----
                ---- ba98 76-- ----     (jclub2o:        switches seemingly at random between 0 and 3f)
                ---- ---- --54 3210     (jclub2o/jclub2: 19 for tilemaps to display / 0 for unused ones)

        6.w     fe-- ---- ---- ----
                --d- ---- ---- ----     (jclub2o: usually on for used tilemaps, but it's not enable: see test mode)
                ---c ba9- ---- ----
                ---- ---8 ---- ----     0/1 = Color granularity 128/32 (ST-0020 only?)
                ---- ---- 7654 3210

   Note: gdfs, which does not use the ST-0020 tilemaps, keeps all these registers to 0

***************************************************************************/

void st0020_device::tmap_st0020_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t old = m_regs[offset];
	data = COMBINE_DATA(&m_regs[offset]);

	int i   = offset / (8/2);
	int reg = offset % (8/2);

	switch (reg)
	{
		case 0x00/2:
			if ((old ^ data) & 0x7c00)
				m_tmap[i]->mark_all_dirty();
			m_tmap[i]->set_scrollx(0, data);
			break;
		case 0x02/2:
			m_tmap[i]->set_scrolly(0, data + m_regs[0x7a/2] + 1);   // fixme update when writing offset
			break;
		case 0x04/2:
			// Priority/Enable?
			break;
		case 0x06/2:
			// Color Granularity + ?
			if ((old ^ data) & 0x0100)
				m_tmap[i]->mark_all_dirty();
			break;
	}
}

void st0020_device::tmap_st0032_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t old = m_regs[offset];
	data = COMBINE_DATA(&m_regs[offset]);

	int i   = (offset - 0x20/2) / (16/2);
	int reg = (offset - 0x20/2) % (16/2);

	switch (reg)
	{
		case 0x00/2:
			m_tmap[i]->set_scrollx(0, data);
			break;
		case 0x02/2:
			m_tmap[i]->set_scrolly(0, data + m_regs[0x78/2] - 1);   // fixme update when writing offset
			break;
		case 0x08/2:
			if ((old ^ data) & 0x007c)
				m_tmap[i]->mark_all_dirty();
			break;
	}
}


uint16_t st0020_device::regs_r(offs_t offset)
{
	if (m_is_st0032)
	{
		// bits 0, 1, 2
		// jclub2v200: waits for bit 0 == 0 (vblank?) before writing sprite ram
		if (offset == 0x0c/2)
			return 0;
	}
	else
	{
		// bits A, B, C
		// gdfs: waits for bit A == 0 (vblank?) before flipping reg 0x86 between 8/9 (double buffering?)
		//       tests bit C before changing reg 0x86 and then doing a blit
		if (offset == 0x00/2)
			return 0;
	}

	logerror("%s: Reg read: %02X\n", machine().describe_context(), offset*2);
	return 0;
}


void st0020_device::regs_st0020_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset < 0x20/2)
	{
		tmap_st0020_w(offset, data, mem_mask);
		return;
	}

	data = COMBINE_DATA(&m_regs[offset]);
	switch (offset)
	{
		// crtc
//                      jclub2v203:
//      case 0x20/2:    // 0000
//      case 0x22/2:    // 0000
//      case 0x62/2:    // 004C (normal monitor), 0050 (wide monitor)
//      case 0x64/2:    // 01AC (normal monitor), 01E0 (wide monitor)
//      case 0x7c/2:    // 0301 (normal monitor), 0302 (wide monitor)
//      case 0x84/2:    // 0082 (normal monitor), 0081 (wide monitor)

//                      gdfs:
//      case 0x20/2:    // 000C
//      case 0x22/2:    // 03F0
//      case 0x68/2:    // 0000 (normal), 0298 (flip screen)
//      case 0x6a/2:    // 0000 (normal), 0298 (flip screen)
//      case 0x84/2:    // 0002 (normal), 001A (flip screen) <- mask 0018 might be flip_xy

//      case 0x86/2:    double buffering?

		case 0x8a/2:
			gfxram_bank_w(offset, data, mem_mask);
			break;

		// blitter
		case 0xc0/2:    // source address
		case 0xc2/2:
		case 0xc4/2:    // destination address
		case 0xc6/2:
		case 0xc8/2:    // length
			break;

		case 0xca/2:    // start
			do_blit_w(data);
			break;

		default:
			logerror("%s: Reg written: %02X <- %04X\n", machine().describe_context(), offset*2, data);
	}
}

void st0020_device::regs_st0032_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset >= 0x20/2 && offset < 0x60/2)
	{
		tmap_st0032_w(offset, data, mem_mask);
		return;
	}

	data = COMBINE_DATA(&m_regs[offset]);
	switch (offset)
	{
		// crtc
//                      jclub2v200:
//      case 0x62/2:    // 004C (normal monitor), 0050 (wide monitor)
//      case 0x64/2:    // 01AC (normal monitor), 01E0 (wide monitor)

//      case 0x82/2:    double buffering?

		case 0x86/2:
			gfxram_bank_w(offset, data, mem_mask);
			break;

		// blitter
		case 0xc0/2:    // source address
		case 0xc2/2:
		case 0xc4/2:    // destination address
		case 0xc6/2:
		case 0xc8/2:    // length
			break;

		case 0xca/2:    // start
			do_blit_w(data);
			break;

		default:
			logerror("%s: Reg written: %02X <- %04X\n", machine().describe_context(), offset*2, data);
	}
}

void st0020_device::regs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_is_st0032)
		regs_st0032_w(offset, data, mem_mask);
	else
		regs_st0020_w(offset, data, mem_mask);
}

/***************************************************************************

    Sprites RAM is 0x80000 bytes long. The first 0x2000? bytes hold a list
    of sprites to display (the list can be made shorter using an end-of-list
    marker).

    Each entry in the list uses 8 bytes (padded to 16 for the ST-0032) and is
    a multi-sprite: it tells the hardware to display several single-sprites.

    The list of multi-sprites looks like this:

    Offset:     Bits:                   Value:

        0.w     fedc ba-- ---- ----
                ---- --98 7654 3210     X displacement

        2.w     fedc ba-- ---- ----
                ---- --98 7654 3210     Y displacement

        4.w     f--- ---- ---- ----     List end
                -edc ba98 7654 3210     Offset of the single-sprite(s) data (16-byte units)

        6.w                             Number of single-sprites (how many bits?)

    A single-sprite uses 16 bytes:

    Offset:     Bits:                   Value:

        0.w                             Code

        2.w     f--- ---- ---- ----     Flip X
                -e-- ---- ---- ----     Flip Y
                --dc b--- ---- ----
                ---- -a-- ---- ----     0 = 256 color steps, 1 = 64 color steps
                ---- --98 7654 3210     Color code

        4.w     fedc ba-- ---- ----
                ---- --98 7654 3210     X displacement

        6.w     fedc ba-- ---- ----
                ---- --98 7654 3210     Y displacement

        8.w     fedc ba98 ---- ----     Zoomed Y Size - 1
                ---- ---- 7654 3210     Zoomed X Size - 1

        A.w     fedc ba98 ---- ----
                ---- ---- 7654 ----     Priority
                ---- ---- ---- 32--     Y Tiles (1,2,4,8)
                ---- ---- ---- --10     X Tiles (1,2,4,8)

        C.w                             Unused

        E.w                             Unused

***************************************************************************/

void st0020_device::draw_zooming_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	// Sprites list
	uint16_t *spriteram = m_spriteram.get();

	uint16_t *s1    =   spriteram;
	uint16_t *end1  =   spriteram + 0x02000/2;

	priority <<= 4;

	const int s1_inc = m_is_st0032 ? 16/2 : 8/2;

	for ( ; s1 < end1; s1+=s1_inc )
	{
		int attr, code, color, num, sprite, zoom, size;
		int sx, x, xoffs, flipx, xnum, xstart, xend, xinc, xdim, xscale;
		int sy, y, yoffs, flipy, ynum, ystart, yend, yinc, ydim, yscale;

		if (m_is_st0032)
		{
			num     =   s1[ 0 ];
			sprite  =   s1[ 1 ];
			xoffs   =   s1[ 2 ];
			yoffs   =   s1[ 3 ];

			// List end
			if (num & 0x8000)
				break;
		}
		else
		{
			xoffs   =   s1[ 0 ];
			yoffs   =   s1[ 1 ];
			sprite  =   s1[ 2 ];
			num     =   s1[ 3 ];

			// List end
			if (sprite & 0x8000)
				break;
		}

		num = num % 0x101; // how many?

		int s2 = 0;
		int spritebase = (sprite & 0x7fff) * 16/2;

		for( ; num > 0; num--,s2+=16/2 )
		{
			code    =   spriteram[(spritebase + s2 + 0 ) & 0x3ffff];
			attr    =   spriteram[(spritebase + s2 + 1 ) & 0x3ffff];
			sx      =   spriteram[(spritebase + s2 + 2 ) & 0x3ffff];
			sy      =   spriteram[(spritebase + s2 + 3 ) & 0x3ffff];
			zoom    =   spriteram[(spritebase + s2 + 4 ) & 0x3ffff];
			size    =   spriteram[(spritebase + s2 + 5 ) & 0x3ffff];

			if (priority != (size & 0xf0))
				break;

			flipx   =   (attr & 0x8000);
			flipy   =   (attr & 0x4000);

			if (m_is_st0032)
				color = (attr & 0x200) ? (attr & 0x1ff) : (attr & 0x1ff) * 4;
			else
				color = (attr & 0x400) ? (attr & 0x3ff) : (attr & 0x3ff) * 4;

			// Single-sprite tile size
			xnum = 1 << ((size >> 0) & 3);
			ynum = 1 << ((size >> 2) & 3);

			xnum = (xnum + 1) / 2;

			if (flipx)  { xstart = xnum-1;  xend = -1;    xinc = -1; }
			else        { xstart = 0;       xend = xnum;  xinc = +1; }

			if (flipy)  { ystart = ynum-1;  yend = -1;    yinc = -1; }
			else        { ystart = 0;       yend = ynum;  yinc = +1; }

			// Apply global offsets
			sx  +=  xoffs;
			sy  +=  yoffs;

			// Sign extend the position
			sx  =   (sx & 0x1ff) - (sx & 0x200);
			sy  =   (sy & 0x1ff) - (sy & 0x200);

			// Y is inverted
			sy  =   -sy;

			// otherwise everything is off-screen
			if (m_is_jclub2)
				sy += 0x100;

			// Use fixed point values (16.16), for accuracy
			sx <<= 16;
			sy <<= 16;

			xdim    =   ( ( ((zoom >> 0) & 0xff) + 1) << 16 ) / xnum;
			ydim    =   ( ( ((zoom >> 8) & 0xff) + 1) << 16 ) / ynum;

			xscale  =   xdim / 16;
			yscale  =   ydim / 8;

			/* Let's approximate to the nearest greater integer value
			   to avoid holes in between tiles */
			if (xscale & 0xffff)    xscale += (1<<16) / 16;
			if (yscale & 0xffff)    yscale += (1<<16) / 8;

			// Draw the tiles

			for (x = xstart; x != xend; x += xinc)
			{
				for (y = ystart; y != yend; y += yinc)
				{
					gfx(0)->zoom_transpen(bitmap, cliprect,
							code++,
							color * 4,
							flipx, flipy,
							(sx + x * xdim) / 0x10000, (sy + y * ydim) / 0x10000,
							xscale, yscale, 0
					);
				}
			}
		}   // single-sprites
	}   // sprites list
}


void st0020_device::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bool update_visible_area)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int mask = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  mask |= 1;
		if (machine().input().code_pressed(KEYCODE_W))  mask |= 2;
		if (machine().input().code_pressed(KEYCODE_E))  mask |= 4;
		if (machine().input().code_pressed(KEYCODE_R))  mask |= 8;
		if (machine().input().code_pressed(KEYCODE_A))  mask |= 16;
		if (mask != 0) layers_ctrl &= mask;
	}
#endif

	// crtc
	if (update_visible_area)
	{
		int x0 = m_regs[0x62/2];
		int x1 = m_regs[0x64/2];
		int y0 = m_regs[m_is_st0032 ? 0x72/2 : 0x74/2];
		int y1 = m_regs[m_is_st0032 ? 0x74/2 : 0x76/2];
		if ((x1 > x0) && (y1 > y0))
			screen.set_visible_area(0, (x1 - x0) - 1, y0, y1 - 1);
	}

	// tilemaps
	for (int pri = 0x3f; pri >= 0; --pri)
		for (int i = 0; i < 4; ++i)
			if (    (layers_ctrl & (1 << i))
					&& (tmap_priority(i) == pri)
					&& tmap_is_enabled(i)           )
				m_tmap[i]->draw(screen, bitmap, cliprect, 0, 0);

	// sprites
	if (layers_ctrl & 16)
		for (int pri = 0; pri <= 0xf; ++pri)
			draw_zooming_sprites(bitmap, cliprect, pri);

#ifdef MAME_DEBUG
#if 0
	popmessage("1: %04x,%04x (%04x %04x) 2: %04x,%04x (%04x %04x)",
			m_regs[0x08/2], m_regs[0x0a/2], m_regs[0x0c/2], m_regs[0x0e/2],
			m_regs[0x10/2], m_regs[0x12/2], m_regs[0x14/2], m_regs[0x16/2] );
#endif
#endif
}
