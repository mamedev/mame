// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench
/***************************************************************************

Toaplan 'FCU' sprite generator

Sprite layer:
Up to 256 sprites, variable size with size LUT RAM
8x8 to 128x128 pixel (1 to 16 tile per width/height)

 used at
 - toaplan/toaplan1.cpp
 - toaplan/fireshrk.cpp

Sprite RAM format: (4 16 bit word per sprite)

Word Bit                 Description
     0000 0000 0000 0000
     fedc ba98 7654 3210
0    x--- ---- ---- ---- Invisible flag
     -xxx xxxx xxxx xxxx Tile index
1    xxxx ---- ---- ---- Priority*
     ---- xxxx xx-- ---- Sprite size LUT index
     ---- ---- --xx xxxx Color index**
2    xxxx xxxx x--- ---- X position
3    xxxx xxxx x--- ---- Y position

* 0...15 = backmost to frontmost
** 16 color unit
*** Unmarked bits are unused/unknown

Sprite size LUT RAM format: (2 nibbles per entry, Only LSB is used)

Bit       Description
7654 3210
xxxx ---- Sprite height (1 tile unit)
---- xxxx Sprite width (1 tile unit)

***************************************************************************/


#include "emu.h"
#include "toaplan_fcu.h"
#include "screen.h"

#include <algorithm>

#define LOG_RAM  (1 << 1)
#define LOG_REGS (1 << 2)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGRAM(...)  LOGMASKED(LOG_RAM,  __VA_ARGS__)
#define LOGREGS(...) LOGMASKED(LOG_REGS, __VA_ARGS__)

// host interface
void toaplan_fcu_device::host_map(address_map &map)
{
	map(0x0, 0x1).r(FUNC(toaplan_fcu_device::frame_done_r));
//  map(0x0, 0x1).w(?? disable sprite refresh ??)
	map(0x2, 0x3).rw(FUNC(toaplan_fcu_device::spriteram_offs_r), FUNC(toaplan_fcu_device::spriteram_offs_w));
	map(0x4, 0x5).rw(FUNC(toaplan_fcu_device::spriteram_r), FUNC(toaplan_fcu_device::spriteram_w));
	map(0x6, 0x7).rw(FUNC(toaplan_fcu_device::spritesizeram_r), FUNC(toaplan_fcu_device::spritesizeram_w));
}

DEFINE_DEVICE_TYPE(TOAPLAN_FCU, toaplan_fcu_device, "toaplan_fcu", "Toaplan FCU")

toaplan_fcu_device::toaplan_fcu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TOAPLAN_FCU, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_spriteram(*this, finder_base::DUMMY_TAG)
	, m_spritesizeram(*this, finder_base::DUMMY_TAG)
	, m_pri_cb(*this)
	, m_frame_done_cb(*this, 0)
	, m_colbase(0)
	, m_flipscreen(false)
	, m_spriteram_offs(0)
{
}

void toaplan_fcu_device::device_start()
{
	assert(!(m_spriteram.length() & (m_spriteram.length() - 1)));
	assert(!(m_spritesizeram.length() & (m_spritesizeram.length() - 1)));

	m_pri_cb.resolve();

	const u32 spriteram_size = m_spriteram.length();
	const u32 spritesizeram_size = m_spritesizeram.length();
	m_buffered_spriteram = make_unique_clear<u16[]>(spriteram_size);
	m_buffered_spritesizeram = make_unique_clear<u16[]>(spritesizeram_size);

	save_pointer(NAME(m_buffered_spriteram), spriteram_size);
	save_pointer(NAME(m_buffered_spritesizeram), spritesizeram_size);

	save_item(NAME(m_flipscreen));
	save_item(NAME(m_spriteram_offs));
}

void toaplan_fcu_device::device_reset()
{
}

/***************************************************************************
    Host interfaces
***************************************************************************/

u16 toaplan_fcu_device::frame_done_r()
{
	return m_frame_done_cb();
}

void toaplan_fcu_device::flipscreen_w(u8 data)
{
	LOGREGS("%s: Setting FCU controller flipscreen port to %02x\n", machine().describe_context(), data);
	m_flipscreen = BIT(data, 7);   /* 0x80 = flip, 0x00 = no flip */
}

u16 toaplan_fcu_device::spriteram_offs_r() // this aint really needed ?
{
	return m_spriteram_offs;
}

void toaplan_fcu_device::spriteram_offs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_spriteram_offs);
}


u16 toaplan_fcu_device::spriteram_r()
{
	return m_spriteram[m_spriteram_offs & (m_spriteram.length() - 1)];
}

void toaplan_fcu_device::spriteram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_spriteram[m_spriteram_offs & (m_spriteram.length() - 1)]);

	if (m_spriteram_offs >= m_spriteram.length())
	{
		LOGRAM("%s: Sprite_RAM_word_w, %08x out of range !\n", machine().describe_context(), m_spriteram_offs);
		return;
	}

	m_spriteram_offs++;
}

u16 toaplan_fcu_device::spritesizeram_r()
{
	return m_spritesizeram[m_spriteram_offs & (m_spritesizeram.length() - 1)];
}

void toaplan_fcu_device::spritesizeram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_spritesizeram[m_spriteram_offs & (m_spritesizeram.length() - 1)]);

	if (m_spriteram_offs >= m_spritesizeram.length())
	{
		LOGRAM("%s: Sprite_Size_RAM_word_w, %08x out of range !\n", machine().describe_context(), m_spriteram_offs);
		return;
	}

	m_spriteram_offs++; /// really ? shouldn't happen on the sizeram
}

/***************************************************************************
    Sprite Handlers
***************************************************************************/

template<class BitmapClass>
void toaplan_fcu_device::draw_sprites_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect)
{
	u16 const *const source = m_buffered_spriteram.get();
	u16 const *const size   = m_buffered_spritesizeram.get();
	const rectangle &visarea = screen.visible_area();

	for (int offs = m_spriteram.bytes() / 2 - 4; offs >= 0; offs -= 4)
	{
		if (BIT(~source[offs], 15))
		{
			const u16 attrib = source[offs + 1];
			const u8 priority = (attrib & 0xf000) >> 12;
			u32 primask = 0;
			if (!m_pri_cb.isnull())
				m_pri_cb(priority, primask);

			u32 sprite = source[offs] & 0x7fff;
			const u32 color = attrib & 0x3f;

			/****** find sprite dimension ******/
			const u32 sizeram_ptr = (attrib >> 6) & 0x3f;
			const u32 sprite_sizex = ( size[sizeram_ptr]       & 0x0f) * 8;
			const u32 sprite_sizey = ((size[sizeram_ptr] >> 4) & 0x0f) * 8;

			/****** find position to display sprite ******/
			int sx_base = (source[offs + 2] >> 7) & 0x1ff;
			int sy_base = (source[offs + 3] >> 7) & 0x1ff;

			if (sx_base >= 0x180) sx_base -= 0x200;
			if (sy_base >= 0x180) sy_base -= 0x200;

			/****** flip the sprite layer ******/
			if (m_flipscreen)
			{
				sx_base = visarea.width() - (sx_base + 8);  /* visarea.x = 320 */
				sy_base = visarea.height() - (sy_base + 8); /* visarea.y = 240 */
				sy_base += ((visarea.max_y + 1) - visarea.height()) * 2;    /* Horizontal games are offset so adjust by  + 0x20 */
			}

			for (int dim_y = 0; dim_y < sprite_sizey; dim_y += 8)
			{
				int sy;
				if (m_flipscreen) sy = sy_base - dim_y;
				else              sy = sy_base + dim_y;

				for (int dim_x = 0; dim_x < sprite_sizex; dim_x += 8)
				{
					int sx;
					if (m_flipscreen) sx = sx_base - dim_x;
					else              sx = sx_base + dim_x;

					gfx(0)->prio_transpen(bitmap, cliprect,
							sprite, color,
							m_flipscreen, m_flipscreen,
							sx, sy,
							screen.priority(), primask, 0);
					sprite++;
				}
			}
		}
	}
}


void toaplan_fcu_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_sprites_common(screen, bitmap, cliprect);
}


void toaplan_fcu_device::draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	draw_sprites_common(screen, bitmap, cliprect);
}


/****************************************************************************
    Spriteram is always 1 frame ahead, suggesting spriteram buffering.
    There are no CPU output registers that control this so we
    assume it happens automatically every frame, at the end of vblank
****************************************************************************/

void toaplan_fcu_device::vblank(int state)
{
	if (state)
	{
		std::copy_n(&m_spriteram[0], m_spriteram.length(), m_buffered_spriteram.get());
		std::copy_n(&m_spritesizeram[0], m_spritesizeram.length(), m_buffered_spritesizeram.get());
	}
}
