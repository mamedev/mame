// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "aerofgt.h"
#include "screen.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(aerofgt_base_state::get_pspikes_tile_info)
{
	const uint16_t code = m_vram[0][tile_index];
	const int bank = (code & 0x1000) >> 12;
	tileinfo.set(0,
			(code & 0x0fff) | (m_gfxbank[bank] << 12),
			((code & 0xe000) >> 13) + 8 * m_charpalettebank,
			0);
}

// also spinlbrk
template<int Layer>
TILE_GET_INFO_MEMBER(aerofgt_base_state::karatblz_tile_info)
{
	const uint16_t code = m_vram[Layer][tile_index];
	tileinfo.set(Layer,
			(code & 0x1fff) | (m_gfxbank[Layer] << 13),
			(code & 0xe000) >> 13,
			0);
}

template<int Layer>
TILE_GET_INFO_MEMBER(aerofgt_base_state::spinlbrk_tile_info)
{
	const uint16_t code = m_vram[Layer][tile_index];
	tileinfo.set(Layer,
			(code & 0x0fff) | (m_gfxbank[Layer] << 12),
			(code & 0xf000) >> 12,
			0);
}

template<int Layer>
TILE_GET_INFO_MEMBER(aerofgt_base_state::get_tile_info)
{
	const uint16_t code = m_vram[Layer][tile_index];
	const int bank = (Layer << 2) | (code & 0x1800) >> 11;
	tileinfo.set(Layer,
			(code & 0x07ff) | (m_gfxbank[bank] << 11),
			(code & 0xe000) >> 13,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/


void aerofgt_base_state::aerofgt_register_state_globals()
{
	save_item(NAME(m_gfxbank));
	save_item(NAME(m_bank));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_flip_screen));
	save_item(NAME(m_charpalettebank));
	save_item(NAME(m_spritepalettebank));
}

VIDEO_START_MEMBER(aerofgt_base_state,pspikes)
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aerofgt_base_state::get_pspikes_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 64,32);
	// no bg2 in this game

	m_sprite_gfx = 1;
	m_charpalettebank = 0;

	aerofgt_register_state_globals();
}

void spikes91_state::video_start()
{
	VIDEO_START_CALL_MEMBER(pspikes);

	m_spikes91_lookup = 0;

	save_item(NAME(m_spikes91_lookup));
}

VIDEO_START_MEMBER(aerofgt_base_state,karatblz)
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aerofgt_base_state::karatblz_tile_info<0>)), TILEMAP_SCAN_ROWS, 8,8, 64,64);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aerofgt_base_state::karatblz_tile_info<1>)), TILEMAP_SCAN_ROWS, 8,8, 64,64);

	m_tilemap[1]->set_transparent_pen(15);
	m_spritepalettebank = 0;
	m_sprite_gfx = 2;

	aerofgt_register_state_globals();
}

VIDEO_START_MEMBER(aerofgt_banked_sound_state,spinlbrk)
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aerofgt_banked_sound_state::spinlbrk_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aerofgt_banked_sound_state::karatblz_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap[1]->set_transparent_pen(15);

	m_spritepalettebank = 0;
	m_sprite_gfx = 2;

	// sprite maps are hardcoded in this game

	aerofgt_register_state_globals();
}

VIDEO_START_MEMBER(aerofgt_base_state,turbofrc)
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aerofgt_base_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aerofgt_base_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap[1]->set_transparent_pen(15);

	m_spritepalettebank = 0;
	m_sprite_gfx = 2;

	aerofgt_register_state_globals();
}

VIDEO_START_MEMBER(aerofgt_base_state,aerofgtb)
{
	VIDEO_START_CALL_MEMBER(turbofrc);

	m_tilemap[0]->set_scrolldx(1, 1);
	m_tilemap[1]->set_scrolldx(1, 1);
}


// new hw type
uint32_t aerofgt_state::tile_callback(uint32_t code)
{
	return m_sprlookupram[0][code&0x7fff];
}


// old hw type
uint32_t aerofgt_base_state::aerofgt_old_tile_callback(uint32_t code)
{
	return m_sprlookupram[0][code % (m_sprlookupram[0].bytes()/2)];
}

uint32_t aerofgt_sound_cpu_state::aerofgt_ol2_tile_callback(uint32_t code)
{
	return m_sprlookupram[1][code % (m_sprlookupram[1].bytes()/2)];
}

uint32_t aerofgt_banked_sound_state::spinbrk_tile_callback(uint32_t code)
{
	// enemy sprites use ROM instead of RAM
	return m_sprlookuprom[code % m_sprlookuprom.length()];
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void aerofgt_base_state::setbank(int layer, int num, int bank)
{
	if (m_gfxbank[num] != bank)
	{
		m_gfxbank[num] = bank;
		m_tilemap[layer]->mark_all_dirty();
	}
}

void aerofgt_base_state::pspikes_gfxbank_w(uint8_t data)
{
	setbank(0, 0, (data & 0xf0) >> 4);
	setbank(0, 1, data & 0x0f);
}

void aerofgt_sound_cpu_state::karatblz_gfxbank_w(uint8_t data)
{
	setbank(0, 0, (data & 0x01));
	setbank(1, 1, (data & 0x08) >> 3);
}

void aerofgt_banked_sound_state::spinlbrk_gfxbank_w(uint8_t data)
{
	setbank(0, 0, (data & 0x07));
	setbank(1, 1, (data & 0x38) >> 3);
}

void aerofgt_base_state::turbofrc_gfxbank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data = COMBINE_DATA(&m_bank[offset]);

	setbank(offset, 4 * offset + 0, (data >> 0) & 0x0f);
	setbank(offset, 4 * offset + 1, (data >> 4) & 0x0f);
	setbank(offset, 4 * offset + 2, (data >> 8) & 0x0f);
	setbank(offset, 4 * offset + 3, (data >> 12) & 0x0f);
}

void aerofgt_state::gfxbank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data = COMBINE_DATA(&m_bank[offset]);

	setbank(offset >> 1, 2 * offset + 0, (data >> 8) & 0xff);
	setbank(offset >> 1, 2 * offset + 1, (data >> 0) & 0xff);
}

void aerofgt_sound_cpu_state::kickball_gfxbank_w(uint8_t data)
{
	// I strongly doubt this logic is correct
	setbank(0, 0, (data & 0x0f) & ~0x01);
	setbank(0, 1, (data & 0x0f));
}

void aerofgt_base_state::pspikes_palette_bank_w(uint8_t data)
{
	m_spritepalettebank = data & 0x03;
	if (m_charpalettebank != (data & 0x1c) >> 2)
	{
		m_charpalettebank = (data & 0x1c) >> 2;
		m_tilemap[0]->mark_all_dirty();
	}

	m_flip_screen = BIT(data, 7);
	m_tilemap[0]->set_flip(m_flip_screen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
}

void aerofgt_sound_cpu_state::spinlbrk_flip_screen_w(uint8_t data)
{
	m_flip_screen = BIT(data, 7);
	m_tilemap[0]->set_flip(m_flip_screen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tilemap[1]->set_flip(m_flip_screen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
}

void aerofgt_banked_sound_state::turbofrc_flip_screen_w(uint8_t data)
{
	m_flip_screen = BIT(data, 7);
	m_tilemap[0]->set_flip(m_flip_screen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tilemap[1]->set_flip(m_flip_screen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	// bit 6 = ?
}


/***************************************************************************

  Display refresh

***************************************************************************/


uint32_t aerofgt_base_state::screen_update_pspikes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, scrolly;

	m_tilemap[0]->set_scroll_rows(256);
	scrolly = m_scrolly[0];
	for (i = 0; i < 256; i++)
		m_tilemap[0]->set_scrollx((i + scrolly) & 0xff, m_rasterram[i]);
	m_tilemap[0]->set_scrolly(0, scrolly);

	screen.priority().fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram,m_spriteram.bytes(),m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen);
	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram,m_spriteram.bytes(),m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen);
	return 0;
}


uint32_t aerofgt_sound_cpu_state::screen_update_karatblz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->set_scrollx(0, m_scrollx[0] - 8);
	m_tilemap[0]->set_scrolly(0, m_scrolly[0]);
	m_tilemap[1]->set_scrollx(0, m_scrollx[1] - 4);
	m_tilemap[1]->set_scrolly(0, m_scrolly[1]);

	screen.priority().fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	// we use the priority buffer so sprites are drawn front to back
	m_spr_old[1]->turbofrc_draw_sprites(m_spriteram+0x200,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen);
	m_spr_old[1]->turbofrc_draw_sprites(m_spriteram+0x200,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen);

	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram+0x000,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen);
	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram+0x000,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen);

	return 0;
}

uint32_t aerofgt_banked_sound_state::screen_update_spinlbrk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->set_scroll_rows(512);
	int scrolly = 0;
	for (int i = 0; i < 256; i++)
		m_tilemap[0]->set_scrollx((i + scrolly) & 0x1ff, m_rasterram[i] - 8);
//  m_tilemap[0]->set_scrolly(0, m_scrolly[0]);
	m_tilemap[1]->set_scrollx(0, m_scrollx[1] - 4);
//  m_tilemap[1]->set_scrolly(0, m_scrolly[1]);

	screen.priority().fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 1);

	// we use the priority buffer so sprites are drawn front to back
	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram+0x000,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen);
	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram+0x000,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen);

	m_spr_old[1]->turbofrc_draw_sprites(m_spriteram+0x200,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen);
	m_spr_old[1]->turbofrc_draw_sprites(m_spriteram+0x200,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen);

	return 0;
}

uint32_t aerofgt_banked_sound_state::screen_update_turbofrc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->set_scroll_rows(512);
	int scrolly = m_scrolly[0] + 2;
	for (int i = 0; i < 256; i++)
//      m_tilemap[0]->set_scrollx((i + scrolly) & 0x1ff, m_rasterram[i] - 11);
		m_tilemap[0]->set_scrollx((i + scrolly) & 0x1ff, m_rasterram[7] - 11 - (m_flip_screen ? 188 : 0));
	m_tilemap[0]->set_scrolly(0, scrolly - (m_flip_screen ? 2 : 0));
	m_tilemap[1]->set_scrollx(0, m_scrollx[1] - (m_flip_screen ? 185 : 7));
	m_tilemap[1]->set_scrolly(0, m_scrolly[1] + (m_flip_screen ? 0 : 2));

	screen.priority().fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 1);

	// we use the priority buffer so sprites are drawn front to back
	m_spr_old[1]->turbofrc_draw_sprites(m_spriteram+0x200,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen); //ship
	m_spr_old[1]->turbofrc_draw_sprites(m_spriteram+0x200,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen); //intro

	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram+0x000,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen); //enemy
	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram+0x000,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen); //enemy

	return 0;
}

uint32_t aerofgt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->set_scrollx(0, m_rasterram[0x0000] - 18);
	m_tilemap[0]->set_scrolly(0, m_scrolly[0]);
	m_tilemap[1]->set_scrollx(0, m_rasterram[0x0200] - 20);
	m_tilemap[1]->set_scrolly(0, m_scrolly[1]);

	screen.priority().fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);

	m_spr->draw_sprites(m_spriteram, m_spriteram.bytes(), screen, bitmap, cliprect, 0x03, 0x00);
	m_spr->draw_sprites(m_spriteram, m_spriteram.bytes(), screen, bitmap, cliprect, 0x03, 0x01);

	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	m_spr->draw_sprites(m_spriteram, m_spriteram.bytes(), screen, bitmap, cliprect, 0x03, 0x02);
	m_spr->draw_sprites(m_spriteram, m_spriteram.bytes(), screen, bitmap, cliprect, 0x03, 0x03);

	return 0;
}


/***************************************************************************

  BOOTLEG SUPPORT

***************************************************************************/

// BOOTLEG
void wbbc97_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wbbc97_state::get_pspikes_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	// no bg2 in this game

	m_tilemap[0]->set_transparent_pen(15);

	m_sprite_gfx = 1;

	aerofgt_register_state_globals();

	save_item(NAME(m_bitmap_enable));
}

// BOOTLEG
void aerofgt_base_state::pspikesb_gfxbank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rasterram[0x200 / 2]);

	setbank(0, 0, (data & 0xf000) >> 12);
	setbank(0, 1, (data & 0x0f00) >> 8);
}

// BOOTLEG
void spikes91_state::spikes91_lookup_w(uint16_t data)
{
	m_spikes91_lookup = BIT(data, 0);
}

// BOOTLEG
void wbbc97_state::bitmap_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bitmap_enable);
}

// BOOTLEG
void aerofgt_base_state::aerfboo2_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int chip, int chip_disabled_pri)
{
	int base, first;

	base = chip * 0x0200;
//  first = 4 * m_spriteram[0x1fe + base];
	first = 0;

	for (int attr_start = base + 0x0200 - 4; attr_start >= first + base; attr_start -= 4)
	{
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (BIT(~m_spriteram[attr_start + 2], 7))
			continue;

		const int pri = BIT(m_spriteram[attr_start + 2], 4);

		if (chip_disabled_pri && !pri)
			continue;
		if ((!chip_disabled_pri) && pri)
			continue;
		const int ox = m_spriteram[attr_start + 1] & 0x01ff;
		const int xsize = (m_spriteram[attr_start + 2] & 0x0700) >> 8;
		const int zoomx = 32 - ((m_spriteram[attr_start + 1] & 0xf000) >> 12);
		const int oy = m_spriteram[attr_start + 0] & 0x01ff;
		const int ysize = (m_spriteram[attr_start + 2] & 0x7000) >> 12;
		const int zoomy = 32 - ((m_spriteram[attr_start + 0] & 0xf000) >> 12);
		const bool flipx = BIT(m_spriteram[attr_start + 2], 11);
		const bool flipy = BIT(m_spriteram[attr_start + 2], 15);
		const int color = (m_spriteram[attr_start + 2] & 0x000f) + 16 * m_spritepalettebank;

		int map_start = m_spriteram[attr_start + 3];

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      ox += (xsize*(32 - zoomx)+2)/4;
//      oy += (ysize*(32 - zoomy)+2)/4;

		for (int y = 0; y <= ysize; y++)
		{
			int sy;
			if (flipy)
				sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (int x = 0; x <= xsize; x++)
			{
				int sx;
				if (flipx)
					sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				const int code = m_sprlookupram[chip][map_start % (m_sprlookupram[chip].bytes()/2)];

				m_gfxdecode->gfx(m_sprite_gfx + chip)->prio_zoom_transpen(bitmap,cliprect,
								code,
								color,
								flipx,flipy,
								sx,sy,
								zoomx << 11, zoomy << 11,
								screen.priority(),pri ? 0 : 2,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

// BOOTLEG
void aerofgt_base_state::pspikesb_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 4; i < m_spriteram.bytes() / 2; i += 4)
	{
		if (BIT(m_spriteram[i + 3 - 4], 15))
			break;

		const int xpos = (m_spriteram[i + 2] & 0x1ff) - 34;
		const int ypos = 256 - (m_spriteram[i + 3 - 4] & 0x1ff) - 33;
		const int code = m_spriteram[i + 0] & 0x1fff;
		const bool flipy = 0;
		const bool flipx = BIT(m_spriteram[i + 1], 11);
		const int color = m_spriteram[i + 1] & 0x000f;

		m_gfxdecode->gfx(m_sprite_gfx)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				xpos,ypos,15);

		// wrap around y
		m_gfxdecode->gfx(m_sprite_gfx)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				xpos,ypos + 512,15);

	}
}

// BOOTLEG
void spikes91_state::spikes91_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_spritepalettebank = 1;

	for (int i = m_spriteram.bytes() / 2 - 4; i >= 4; i -= 4)
	{
		int code = m_spriteram[i + 0] & 0x1fff;

		if (!code)
			continue;

		const int xpos = (m_spriteram[i + 2] & 0x01ff) - 16;
		const int ypos = 256 - (m_spriteram[i + 1] & 0x00ff) - 26;
		const bool flipy = 0;
		const bool flipx = BIT(m_spriteram[i + 3], 15);
		const int color = ((m_spriteram[i + 3] & 0x00f0) >> 4);

		code |= m_spikes91_lookup << 13;

		m_gfxdecode->gfx(m_sprite_gfx)->transpen(bitmap,cliprect,
				m_sprlookuprom[code],
				color,
				flipx,flipy,
				xpos,ypos,15);

		// wrap around y
		m_gfxdecode->gfx(m_sprite_gfx)->transpen(bitmap,cliprect,
				m_sprlookuprom[code],
				color,
				flipx,flipy,
				xpos,ypos + 512,15);
	}
}

// BOOTLEG
void aerofgt_base_state::aerfboot_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int last = ((m_rasterram[0x404 / 2] << 5) - 0x8000) / 2;
	if (last < 0) last = 0;

	for (int attr_start = m_spriteram.bytes() / 2 - 4; attr_start >= last; attr_start -= 4)
	{
		const int ox = m_spriteram[attr_start + 1] & 0x01ff;
		const int oy = m_spriteram[attr_start + 0] & 0x01ff;
		const bool flipx = BIT(m_spriteram[attr_start + 2], 11);
		const bool flipy = BIT(m_spriteram[attr_start + 2], 15);
		const int color = m_spriteram[attr_start + 2] & 0x000f;

		int zoomx = (m_spriteram[attr_start + 1] & 0xf000) >> 12;
		int zoomy = (m_spriteram[attr_start + 0] & 0xf000) >> 12;
		const int pri = BIT(m_spriteram[attr_start + 2], 4);
		int code = m_spriteram[attr_start + 3] & 0x1fff;

		if (BIT(~m_spriteram[attr_start + 2], 6))
			code |= 0x2000;

		zoomx = 32 + zoomx;
		zoomy = 32 + zoomy;

		int sy = ((oy + 16 - 1) & 0x1ff) - 16;

		int sx = ((ox + 16 + 3) & 0x1ff) - 16;

		m_gfxdecode->gfx(m_sprite_gfx + (code >= 0x1000 ? 0 : 1))->prio_zoom_transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,
				zoomx << 11,zoomy << 11,
				screen.priority(),pri ? 0 : 2,15);

	}

	last = ((m_rasterram[0x402 / 2] << 5) - 0x8000) / 2;
	if (last < 0) last = 0;

	for (int attr_start = ((m_spriteram.bytes() / 2) / 2) - 4; attr_start >= last; attr_start -= 4)
	{
		const int ox = m_spriteram[attr_start + 1] & 0x01ff;
		const int oy = m_spriteram[attr_start + 0] & 0x01ff;
		const bool flipx = BIT(m_spriteram[attr_start + 2], 11);
		const bool flipy = BIT(m_spriteram[attr_start + 2], 15);
		const int color = m_spriteram[attr_start + 2] & 0x000f;

		int zoomx = (m_spriteram[attr_start + 1] & 0xf000) >> 12;
		int zoomy = (m_spriteram[attr_start + 0] & 0xf000) >> 12;
		const int pri = BIT(m_spriteram[attr_start + 2], 4);
		int code = m_spriteram[attr_start + 3] & 0x1fff;

		if (BIT(~m_spriteram[attr_start + 2], 6))
			code |= 0x2000;

		zoomx = 32 + zoomx;
		zoomy = 32 + zoomy;

		int sy = ((oy + 16 - 1) & 0x1ff) - 16;

		int sx = ((ox + 16 + 3) & 0x1ff) - 16;

		m_gfxdecode->gfx(m_sprite_gfx + (code >= 0x1000 ? 0 : 1))->prio_zoom_transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,
				zoomx << 11,zoomy << 11,
				screen.priority(),pri ? 0 : 2,15);

	}
}

// BOOTLEG
void wbbc97_state::draw_bitmap(bitmap_rgb32 &bitmap)
{
	int count = 16; // weird, the bitmap doesn't start at 0?
	for (int y = 0; y < 256; y++)
		for (int x = 0; x < 512; x++)
		{
			const int color = m_bitmapram[count] >> 1;

			// data is GRB; convert to RGB
			const rgb_t pen = rgb_t(pal5bit((color & 0x3e0) >> 5), pal5bit((color & 0x7c00) >> 10), pal5bit(color & 0x1f));
			bitmap.pix(y, (10 + x - m_rasterram[(y & 0x7f)]) & 0x1ff) = pen;

			count++;
			count &= 0x1ffff;
		}
}

// BOOTLEG
uint32_t aerofgt_base_state::screen_update_pspikesb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->set_scroll_rows(256);
	int scrolly = m_scrolly[0];
	for (int i = 0; i < 256; i++)
		m_tilemap[0]->set_scrollx((i + scrolly) & 0xff, m_rasterram[i] + 22);
	m_tilemap[0]->set_scrolly(0, scrolly);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	pspikesb_draw_sprites(screen, bitmap, cliprect);
	return 0;
}

// BOOTLEG
uint32_t spikes91_state::screen_update_spikes91(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *const gfx = m_gfxdecode->gfx(0);

	m_tilemap[0]->set_scroll_rows(256);
	const int scrolly = m_scrolly[0];

	for (int i = 0; i < 256; i++)
		m_tilemap[0]->set_scrollx((i + scrolly) & 0xff, m_rasterram[i + 0x01f0 / 2] + 0x96 + 0x16);
	m_tilemap[0]->set_scrolly(0, scrolly);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	spikes91_draw_sprites(screen, bitmap, cliprect);

	// we could use a tilemap, but it's easier to just do it here
	int count = 0;
	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			const uint16_t tileno = m_tx_tilemap_ram[count] & 0x1fff;
			const uint16_t colour = m_tx_tilemap_ram[count] & 0xe000;
			gfx->transpen(bitmap,cliprect,
					tileno,
					colour>>13,
					0,0,
					(x*8)+24,(y*8)+8,15);

			count++;
		}

	}

	return 0;
}

// BOOTLEG
uint32_t aerofgt_base_state::screen_update_aerfboot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, scrolly;

	m_tilemap[0]->set_scroll_rows(512);
	scrolly = m_scrolly[0] + 2;
	for (i = 0; i < 256; i++)
		m_tilemap[0]->set_scrollx((i + scrolly) & 0x1ff, m_rasterram[7] + 174);
	m_tilemap[0]->set_scrolly(0, scrolly);
	m_tilemap[1]->set_scrollx(0, m_scrollx[1] + 172);
	m_tilemap[1]->set_scrolly(0, m_scrolly[1] + 2);

	screen.priority().fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 1);

	// we use the priority buffer so sprites are drawn front to back
	aerfboot_draw_sprites(screen, bitmap, cliprect);
	return 0;
}

// BOOTLEG
uint32_t aerofgt_base_state::screen_update_aerfboo2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, scrolly;

	m_tilemap[0]->set_scroll_rows(512);
	scrolly = m_scrolly[0] + 2;
	for (i = 0; i < 256; i++)
//      m_tilemap[0]->set_scrollx((i + scrolly) & 0x1ff, m_rasterram[i] - 11);
		m_tilemap[0]->set_scrollx((i + scrolly) & 0x1ff, m_rasterram[7] - 11);
	m_tilemap[0]->set_scrolly(0, scrolly);
	m_tilemap[1]->set_scrollx(0, m_scrollx[1] - 7);
	m_tilemap[1]->set_scrolly(0, m_scrolly[1] + 2);

	screen.priority().fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 1);

	// we use the priority buffer so sprites are drawn front to back
	aerfboo2_draw_sprites(screen, bitmap, cliprect, 1, -1); //ship
	aerfboo2_draw_sprites(screen, bitmap, cliprect, 1, 0); //intro
	aerfboo2_draw_sprites(screen, bitmap, cliprect, 0, -1); //enemy
	aerfboo2_draw_sprites(screen, bitmap, cliprect, 0, 0); //enemy
	return 0;
}

// BOOTLEG (still uses original sprite type)
uint32_t wbbc97_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->set_scroll_rows(256);
	const int scrolly = m_scrolly[0];
	for (int i = 0; i < 256; i++)
		m_tilemap[0]->set_scrollx((i + scrolly) & 0xff, m_rasterram[i]);
	m_tilemap[0]->set_scrolly(0, scrolly);

	screen.priority().fill(0, cliprect);

	if (m_bitmap_enable)
	{
		draw_bitmap(bitmap);
		m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	}

	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram,m_spriteram.bytes(),m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen);
	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram,m_spriteram.bytes(),m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen);
	return 0;
}
