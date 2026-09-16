// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Nicola Salmoria, Ernesto Corvi

/*
    Namco System 1/86 Sprites - found on Namco System 1, System 86,
    Baraduke/Alien Sector and Metro-Cross hardware

    based on docs in namcos86.cpp and namcos1.cpp,
    they configured with following chips:
    - CUS39 ULA sprite generator
    - CUS35/CUS48 ULA sprite address generator

    "Shadow" sprites are used in namcos1.cpp

    Device used in the following drivers:
    - namco/namcos86.cpp
    - namco/namcos1.cpp

    Using same sprite hardware but not using this implementation currently:
    - namco/baraduke.cpp

*/

#include "emu.h"
#include "namcos1_sprite.h"

DEFINE_DEVICE_TYPE(NAMCOS1_SPRITE, namcos1_sprite_device, "namcos1_sprite", "Namco System 1 Sprites (CUS39, CUS35/CUS48)")

namcos1_sprite_device::namcos1_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NAMCOS1_SPRITE, tag, owner, clock),
	device_gfx_interface(mconfig, *this),
	m_spriteram(*this, "spriteram", 0x800, ENDIANNESS_BIG), // actually used area only
	m_shadow_cb(*this),
	m_pri_cb(*this),
	m_gfxbank_cb(*this),
	m_flip_cb(*this),
	m_copy_sprites(false),
	m_flip_screen(false)
{
}


void namcos1_sprite_device::device_start()
{
	m_shadow_cb.resolve_safe(std::make_pair(false, nullptr));
	m_pri_cb.resolve_safe(0);
	m_gfxbank_cb.resolve();

	save_item(NAME(m_copy_sprites));
	save_item(NAME(m_flip_screen));
}

/**************************************************************************************/

void namcos1_sprite_device::spriteram_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().w(FUNC(namcos1_sprite_device::spriteram_w)).share(m_spriteram);
}


u8 namcos1_sprite_device::spriteram_r(offs_t offset)
{
	return m_spriteram[offset];
}


void namcos1_sprite_device::spriteram_w(offs_t offset, u8 data)
{
	m_spriteram[offset] = data;

	// a write to this offset tells the sprite chip to buffer the sprite list
	if (offset == 0x7f2)
		m_copy_sprites = true;

	// flip screen is embedded in the sprite control registers
	if (offset == 0x7f6)
	{
		const bool flip = BIT(data, 0);
		if (flip != m_flip_screen)
			m_flip_cb(m_flip_screen = data);
	}
}


/**************************************************************************************/

/*
sprite format (original docs from namcos1.cpp):

0-3  scratchpad RAM
4-9  CPU writes here, hardware copies from here to 10-15
10   xx------  X size (16, 8, 32, 4)
10   --x-----  X flip
10   ---xx---  X offset inside 32x32 tile
10   -----xxx  depends on hardware (priority (baraduke) or tile bank (namcos86 and namcos1))
11   xxxxxxxx  tile number
12   xxxxxxx-  color
12   -------x  X position MSB
13   xxxxxxxx  X position
14   xxx-----  depends on hardware (priority (namcos86 and namcos1))
14   ---xx---  Y offset inside 32x32 tile
14   -----xx-  Y size (16, 8, 32, 4)
14   -------x  Y flip
15   xxxxxxxx  Y position
*/

void namcos1_sprite_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	constexpr int sprite_size[4] = { 16, 8, 32, 4 };
	u8 const *source = &m_spriteram[0x800-0x20]; // the last is NOT a sprite
	u8 const *const finish = &m_spriteram[0];
	gfx_element *gfx = this->gfx(0);

	const int sprite_xoffs = m_spriteram[0x07f5] + ((m_spriteram[0x07f4] & 1) << 8);
	const int sprite_yoffs = m_spriteram[0x07f7];

	while (source >= finish)
	{
		const u8 attr1 = source[10];
		const u8 attr2 = source[14];
		u32 color = source[12];
		bool flipx = BIT(attr1, 5);
		bool flipy = BIT(attr2, 0);
		const u16 sizex = sprite_size[(attr1 & 0xc0) >> 6];
		const u16 sizey = sprite_size[(attr2 & 0x06) >> 1];
		const u16 tx = (attr1 & 0x18) & (~(sizex - 1));
		const u16 ty = (attr2 & 0x18) & (~(sizey - 1));
		int sx = source[13] + ((color & 0x01) << 8);
		int sy = -source[15] - sizey;
		u32 sprite = source[11];
		const u32 sprite_bank = attr1 & 7;
		const u32 pri_mask = m_pri_cb(attr1, attr2);

		if (!m_gfxbank_cb.isnull())
			sprite = m_gfxbank_cb(sprite, sprite_bank);
		color = color >> 1;

		sx += sprite_xoffs;
		sy -= sprite_yoffs;

		if (m_flip_screen)
		{
			sx = -sx - sizex;
			sy = -sy - sizey;
			flipx = !flipx;
			flipy = !flipy;
		}

		sy++; // sprites are buffered and delayed by one scanline

		const auto [shadow_on, shadow_table] = m_shadow_cb(color);

		gfx->set_source_clip(tx, sizex, ty, sizey);
		if (shadow_on && shadow_table)
		{
			gfx->prio_transtable(bitmap,cliprect,
					sprite,
					color,
					flipx,flipy,
					sx & 0x1ff,
					((sy + 16) & 0xff) - 16,
					screen.priority(), pri_mask,
					shadow_table);
		}
		else
		{
			gfx->prio_transpen(bitmap,cliprect,
					sprite,
					color,
					flipx,flipy,
					sx & 0x1ff,
					((sy + 16) & 0xff) - 16,
					screen.priority(), pri_mask,
					0xf);
		}

		source -= 0x10;
	}
}

void namcos1_sprite_device::copy_sprites()
{
	if (m_copy_sprites)
	{
		for (int i = 0; i < 0x800; i += 16)
		{
			for (int j = 10; j < 16; j++)
				m_spriteram[i + j] = m_spriteram[i + j - 6];
		}

		m_copy_sprites = false;
	}
}
