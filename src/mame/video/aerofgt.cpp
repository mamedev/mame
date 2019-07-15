// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/aerofgt.h"
#include "screen.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(aerofgt_state::get_pspikes_tile_info)
{
	uint16_t code = m_vram[0][tile_index];
	int bank = (code & 0x1000) >> 12;
	SET_TILE_INFO_MEMBER(0,
			(code & 0x0fff) | (m_gfxbank[bank] << 12),
			((code & 0xe000) >> 13) + 8 * m_charpalettebank,
			0);
}

/* also spinlbrk */
template<int Layer>
TILE_GET_INFO_MEMBER(aerofgt_state::karatblz_tile_info)
{
	uint16_t code = m_vram[Layer][tile_index];
	SET_TILE_INFO_MEMBER(Layer,
			(code & 0x1fff) | (m_gfxbank[Layer] << 13),
			(code & 0xe000) >> 13,
			0);
}

template<int Layer>
TILE_GET_INFO_MEMBER(aerofgt_state::spinlbrk_tile_info)
{
	uint16_t code = m_vram[Layer][tile_index];
	SET_TILE_INFO_MEMBER(Layer,
			(code & 0x0fff) | (m_gfxbank[Layer] << 12),
			(code & 0xf000) >> 12,
			0);
}

template<int Layer>
TILE_GET_INFO_MEMBER(aerofgt_state::get_tile_info)
{
	uint16_t code = m_vram[Layer][tile_index];
	int bank = (Layer << 2) | (code & 0x1800) >> 11;
	SET_TILE_INFO_MEMBER(Layer,
			(code & 0x07ff) | (m_gfxbank[bank] << 11),
			(code & 0xe000) >> 13,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/


void aerofgt_state::aerofgt_register_state_globals(  )
{
	save_item(NAME(m_gfxbank));
	save_item(NAME(m_bank));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_flip_screen));
	save_item(NAME(m_charpalettebank));
	save_item(NAME(m_spritepalettebank));
}

VIDEO_START_MEMBER(aerofgt_state,pspikes)
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(aerofgt_state::get_pspikes_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	/* no bg2 in this game */

	m_sprite_gfx = 1;

	aerofgt_register_state_globals();
	save_item(NAME(m_spikes91_lookup));
}

VIDEO_START_MEMBER(aerofgt_state,karatblz)
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(aerofgt_state::karatblz_tile_info<0>),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(aerofgt_state::karatblz_tile_info<1>),this),TILEMAP_SCAN_ROWS,8,8,64,64);

	m_tilemap[1]->set_transparent_pen(15);
	m_spritepalettebank = 0;
	m_sprite_gfx = 2;

	aerofgt_register_state_globals();
}

VIDEO_START_MEMBER(aerofgt_state,spinlbrk)
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(aerofgt_state::spinlbrk_tile_info<0>),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(aerofgt_state::karatblz_tile_info<1>),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap[1]->set_transparent_pen(15);

	m_spritepalettebank = 0;
	m_sprite_gfx = 2;

	/* sprite maps are hardcoded in this game */

	aerofgt_register_state_globals();
}

VIDEO_START_MEMBER(aerofgt_state,turbofrc)
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(aerofgt_state::get_tile_info<0>),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(aerofgt_state::get_tile_info<1>),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap[1]->set_transparent_pen(15);

	m_spritepalettebank = 0;
	m_sprite_gfx = 2;

	aerofgt_register_state_globals();
}


/* new hw type */
uint32_t aerofgt_state::aerofgt_tile_callback( uint32_t code )
{
	return m_sprlookupram[0][code&0x7fff];
}


/* old hw type */
uint32_t aerofgt_state::aerofgt_old_tile_callback( uint32_t code )
{
	return m_sprlookupram[0][code % (m_sprlookupram[0].bytes()/2)];
}

uint32_t aerofgt_state::aerofgt_ol2_tile_callback( uint32_t code )
{
	return m_sprlookupram[1][code % (m_sprlookupram[1].bytes()/2)];
}

uint32_t aerofgt_state::spinbrk_tile_callback( uint32_t code )
{
	/* enemy sprites use ROM instead of RAM */
	return m_sprlookuprom[code % m_sprlookuprom.length()];
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void aerofgt_state::setbank( int layer, int num, int bank )
{
	if (m_gfxbank[num] != bank)
	{
		m_gfxbank[num] = bank;
		m_tilemap[layer]->mark_all_dirty();
	}
}

WRITE8_MEMBER(aerofgt_state::pspikes_gfxbank_w)
{
	setbank(0, 0, (data & 0xf0) >> 4);
	setbank(0, 1, data & 0x0f);
}

WRITE8_MEMBER(aerofgt_state::karatblz_gfxbank_w)
{
	setbank(0, 0, (data & 0x01));
	setbank(1, 1, (data & 0x08) >> 3);
}

WRITE8_MEMBER(aerofgt_state::spinlbrk_gfxbank_w)
{
	setbank(0, 0, (data & 0x07));
	setbank(1, 1, (data & 0x38) >> 3);
}

WRITE16_MEMBER(aerofgt_state::turbofrc_gfxbank_w)
{
	data = COMBINE_DATA(&m_bank[offset]);

	setbank(offset, 4 * offset + 0, (data >> 0) & 0x0f);
	setbank(offset, 4 * offset + 1, (data >> 4) & 0x0f);
	setbank(offset, 4 * offset + 2, (data >> 8) & 0x0f);
	setbank(offset, 4 * offset + 3, (data >> 12) & 0x0f);
}

WRITE16_MEMBER(aerofgt_state::aerofgt_gfxbank_w)
{
	data = COMBINE_DATA(&m_bank[offset]);

	setbank(offset >> 1, 2 * offset + 0, (data >> 8) & 0xff);
	setbank(offset >> 1, 2 * offset + 1, (data >> 0) & 0xff);
}

WRITE8_MEMBER(aerofgt_state::kickball_gfxbank_w)
{
	// I strongly doubt this logic is correct
	setbank(0, 0, (data & 0x0f)&~0x01);
	setbank(0, 1, (data & 0x0f));
}

WRITE8_MEMBER(aerofgt_state::pspikes_palette_bank_w)
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

WRITE8_MEMBER(aerofgt_state::spinlbrk_flip_screen_w)
{
	m_flip_screen = BIT(data, 7);
	m_tilemap[0]->set_flip(m_flip_screen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tilemap[1]->set_flip(m_flip_screen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
}

WRITE8_MEMBER(aerofgt_state::turbofrc_flip_screen_w)
{
	m_flip_screen = BIT(data, 7);
	m_tilemap[0]->set_flip(m_flip_screen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tilemap[1]->set_flip(m_flip_screen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	// bit 6 = ?
}


/***************************************************************************

  Display refresh

***************************************************************************/


uint32_t aerofgt_state::screen_update_pspikes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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


uint32_t aerofgt_state::screen_update_karatblz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->set_scrollx(0, m_scrollx[0] - 8);
	m_tilemap[0]->set_scrolly(0, m_scrolly[0]);
	m_tilemap[1]->set_scrollx(0, m_scrollx[1] - 4);
	m_tilemap[1]->set_scrolly(0, m_scrolly[1]);

	screen.priority().fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	/* we use the priority buffer so sprites are drawn front to back */
	m_spr_old[1]->turbofrc_draw_sprites(m_spriteram+0x200,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen);
	m_spr_old[1]->turbofrc_draw_sprites(m_spriteram+0x200,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen);

	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram+0x000,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen);
	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram+0x000,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen);
	return 0;
}

uint32_t aerofgt_state::screen_update_spinlbrk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, scrolly;

	m_tilemap[0]->set_scroll_rows(512);
	scrolly = 0;
	for (i = 0; i < 256; i++)
		m_tilemap[0]->set_scrollx((i + scrolly) & 0x1ff, m_rasterram[i] - 8);
//  m_tilemap[0]->set_scrolly(0, m_scrolly[0]);
	m_tilemap[1]->set_scrollx(0, m_scrollx[1] - 4);
//  m_tilemap[1]->set_scrolly(0, m_scrolly[1]);

	screen.priority().fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 1);

	/* we use the priority buffer so sprites are drawn front to back */
	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram+0x000,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen);
	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram+0x000,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen);

	m_spr_old[1]->turbofrc_draw_sprites(m_spriteram+0x200,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen);
	m_spr_old[1]->turbofrc_draw_sprites(m_spriteram+0x200,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen);
	return 0;
}

uint32_t aerofgt_state::screen_update_turbofrc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, scrolly;

	m_tilemap[0]->set_scroll_rows(512);
	scrolly = m_scrolly[0] + 2;
	for (i = 0; i < 256; i++)
//      m_tilemap[0]->set_scrollx((i + scrolly) & 0x1ff, m_rasterram[i] - 11);
		m_tilemap[0]->set_scrollx((i + scrolly) & 0x1ff, m_rasterram[7] - 11 - (m_flip_screen ? 188 : 0));
	m_tilemap[0]->set_scrolly(0, scrolly - (m_flip_screen ? 2 : 0));
	m_tilemap[1]->set_scrollx(0, m_scrollx[1] - (m_flip_screen ? 185 : 7));
	m_tilemap[1]->set_scrolly(0, m_scrolly[1] + (m_flip_screen ? 0 : 2));

	screen.priority().fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 1);

	/* we use the priority buffer so sprites are drawn front to back */
	m_spr_old[1]->turbofrc_draw_sprites(m_spriteram+0x200,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen); //ship
	m_spr_old[1]->turbofrc_draw_sprites(m_spriteram+0x200,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen); //intro

	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram+0x000,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 1, m_flip_screen); //enemy
	m_spr_old[0]->turbofrc_draw_sprites(m_spriteram+0x000,m_spriteram.bytes()/2,m_spritepalettebank, bitmap, cliprect, screen.priority(), 0, m_flip_screen); //enemy
	return 0;
}

uint32_t aerofgt_state::screen_update_aerofgt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
VIDEO_START_MEMBER(aerofgt_state,wbbc97)
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(aerofgt_state::get_pspikes_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	/* no bg2 in this game */

	m_tilemap[0]->set_transparent_pen(15);

	m_sprite_gfx = 1;

	aerofgt_register_state_globals();

	save_item(NAME(m_wbbc97_bitmap_enable));
}

// BOOTLEG
WRITE16_MEMBER(aerofgt_state::pspikesb_gfxbank_w)
{
	COMBINE_DATA(&m_rasterram[0x200 / 2]);

	setbank(0, 0, (data & 0xf000) >> 12);
	setbank(0, 1, (data & 0x0f00) >> 8);
}

// BOOTLEG
WRITE16_MEMBER(aerofgt_state::spikes91_lookup_w)
{
	m_spikes91_lookup = data & 1;
}

// BOOTLEG
WRITE16_MEMBER(aerofgt_state::wbbc97_bitmap_enable_w)
{
	COMBINE_DATA(&m_wbbc97_bitmap_enable);
}

// BOOTLEG
void aerofgt_state::aerfboo2_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int chip, int chip_disabled_pri )
{
	int attr_start, base, first;

	base = chip * 0x0200;
//  first = 4 * m_spriteram[0x1fe + base];
	first = 0;

	for (attr_start = base + 0x0200 - 4; attr_start >= first + base; attr_start -= 4)
	{
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color, pri;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(m_spriteram[attr_start + 2] & 0x0080))
			continue;

		pri = m_spriteram[attr_start + 2] & 0x0010;

		if ( chip_disabled_pri && !pri)
			continue;
		if ((!chip_disabled_pri) && (pri >> 4))
			continue;
		ox = m_spriteram[attr_start + 1] & 0x01ff;
		xsize = (m_spriteram[attr_start + 2] & 0x0700) >> 8;
		zoomx = (m_spriteram[attr_start + 1] & 0xf000) >> 12;
		oy = m_spriteram[attr_start + 0] & 0x01ff;
		ysize = (m_spriteram[attr_start + 2] & 0x7000) >> 12;
		zoomy = (m_spriteram[attr_start + 0] & 0xf000) >> 12;
		flipx = m_spriteram[attr_start + 2] & 0x0800;
		flipy = m_spriteram[attr_start + 2] & 0x8000;
		color = (m_spriteram[attr_start + 2] & 0x000f) + 16 * m_spritepalettebank;

		map_start = m_spriteram[attr_start + 3];

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		for (y = 0; y <= ysize; y++)
		{
			int sx, sy;

			if (flipy)
				sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx)
					sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				code = m_sprlookupram[chip][map_start % (m_sprlookupram[chip].bytes()/2)];

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
void aerofgt_state::pspikesb_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int i;

	for (i = 4; i < m_spriteram.bytes() / 2; i += 4)
	{
		int xpos, ypos, color, flipx, flipy, code;

		if (m_spriteram[i + 3 - 4] & 0x8000)
			break;

		xpos = (m_spriteram[i + 2] & 0x1ff) - 34;
		ypos = 256 - (m_spriteram[i + 3 - 4] & 0x1ff) - 33;
		code = m_spriteram[i + 0] & 0x1fff;
		flipy = 0;
		flipx = m_spriteram[i + 1] & 0x0800;
		color = m_spriteram[i + 1] & 0x000f;

		m_gfxdecode->gfx(m_sprite_gfx)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				xpos,ypos,15);

		/* wrap around y */
		m_gfxdecode->gfx(m_sprite_gfx)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				xpos,ypos + 512,15);

	}
}

// BOOTLEG
void aerofgt_state::spikes91_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int i;
	m_spritepalettebank = 1;

	for (i = m_spriteram.bytes() / 2 - 4; i >= 4; i -= 4)
	{
		int xpos, ypos, color, flipx, flipy, code;

		code = m_spriteram[i + 0] & 0x1fff;

		if (!code)
			continue;

		xpos = (m_spriteram[i + 2] & 0x01ff) - 16;
		ypos = 256 - (m_spriteram[i + 1] & 0x00ff) - 26;
		flipy = 0;
		flipx = m_spriteram[i + 3] & 0x8000;
		color = ((m_spriteram[i + 3] & 0x00f0) >> 4);

		code |= m_spikes91_lookup * 0x2000;

		m_gfxdecode->gfx(m_sprite_gfx)->transpen(bitmap,cliprect,
				m_sprlookuprom[code],
				color,
				flipx,flipy,
				xpos,ypos,15);

		/* wrap around y */
		m_gfxdecode->gfx(m_sprite_gfx)->transpen(bitmap,cliprect,
				m_sprlookuprom[code],
				color,
				flipx,flipy,
				xpos,ypos + 512,15);
	}
}

// BOOTLEG
void aerofgt_state::aerfboot_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int attr_start, last;

	last = ((m_rasterram[0x404 / 2] << 5) - 0x8000) / 2;

	for (attr_start = m_spriteram.bytes() / 2 - 4; attr_start >= last; attr_start -= 4)
	{
		int code;
		int ox, oy, sx, sy, zoomx, zoomy, flipx, flipy, color, pri;

		ox = m_spriteram[attr_start + 1] & 0x01ff;
		oy = m_spriteram[attr_start + 0] & 0x01ff;
		flipx = m_spriteram[attr_start + 2] & 0x0800;
		flipy = m_spriteram[attr_start + 2] & 0x8000;
		color = m_spriteram[attr_start + 2] & 0x000f;

		zoomx = (m_spriteram[attr_start + 1] & 0xf000) >> 12;
		zoomy = (m_spriteram[attr_start + 0] & 0xf000) >> 12;
		pri = m_spriteram[attr_start + 2] & 0x0010;
		code = m_spriteram[attr_start + 3] & 0x1fff;

		if (!(m_spriteram[attr_start + 2] & 0x0040))
			code |= 0x2000;

		zoomx = 32 + zoomx;
		zoomy = 32 + zoomy;

		sy = ((oy + 16 - 1) & 0x1ff) - 16;

		sx = ((ox + 16 + 3) & 0x1ff) - 16;

		m_gfxdecode->gfx(m_sprite_gfx + (code >= 0x1000 ? 0 : 1))->prio_zoom_transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,
				zoomx << 11,zoomy << 11,
				screen.priority(),pri ? 0 : 2,15);

	}

	last = ((m_rasterram[0x402 / 2] << 5) - 0x8000) / 2;

	for (attr_start = ((m_spriteram.bytes() / 2) / 2) - 4; attr_start >= last; attr_start -= 4)
	{
		int code;
		int ox, oy, sx, sy, zoomx, zoomy, flipx, flipy, color, pri;

		ox = m_spriteram[attr_start + 1] & 0x01ff;
		oy = m_spriteram[attr_start + 0] & 0x01ff;
		flipx = m_spriteram[attr_start + 2] & 0x0800;
		flipy = m_spriteram[attr_start + 2] & 0x8000;
		color = m_spriteram[attr_start + 2] & 0x000f;

		zoomx = (m_spriteram[attr_start + 1] & 0xf000) >> 12;
		zoomy = (m_spriteram[attr_start + 0] & 0xf000) >> 12;
		pri = m_spriteram[attr_start + 2] & 0x0010;
		code = m_spriteram[attr_start + 3] & 0x1fff;

		if (!(m_spriteram[attr_start + 2] & 0x0040))
			code |= 0x2000;

		zoomx = 32 + zoomx;
		zoomy = 32 + zoomy;

		sy = ((oy + 16 - 1) & 0x1ff) - 16;

		sx = ((ox + 16 + 3) & 0x1ff) - 16;

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
void aerofgt_state::wbbc97_draw_bitmap( bitmap_rgb32 &bitmap )
{
	int x, y, count;

	count = 16; // weird, the bitmap doesn't start at 0?
	for (y = 0; y < 256; y++)
		for (x = 0; x < 512; x++)
		{
			int color = m_bitmapram[count] >> 1;

			/* data is GRB; convert to RGB */
			rgb_t pen = rgb_t(pal5bit((color & 0x3e0) >> 5), pal5bit((color & 0x7c00) >> 10), pal5bit(color & 0x1f));
			bitmap.pix32(y, (10 + x - m_rasterram[(y & 0x7f)]) & 0x1ff) = pen;

			count++;
			count &= 0x1ffff;
		}
}

// BOOTLEG
uint32_t aerofgt_state::screen_update_pspikesb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, scrolly;

	m_tilemap[0]->set_scroll_rows(256);
	scrolly = m_scrolly[0];
	for (i = 0; i < 256; i++)
		m_tilemap[0]->set_scrollx((i + scrolly) & 0xff, m_rasterram[i] + 22);
	m_tilemap[0]->set_scrolly(0, scrolly);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	pspikesb_draw_sprites(screen, bitmap, cliprect);
	return 0;
}

// BOOTLEG
uint32_t aerofgt_state::screen_update_spikes91(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, scrolly;
	int y, x;
	int count;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	m_tilemap[0]->set_scroll_rows(256);
	scrolly = m_scrolly[0];

	for (i = 0; i < 256; i++)
		m_tilemap[0]->set_scrollx((i + scrolly) & 0xff, m_rasterram[i + 0x01f0 / 2] + 0x96 + 0x16);
	m_tilemap[0]->set_scrolly(0, scrolly);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	spikes91_draw_sprites(screen, bitmap, cliprect);

	/* we could use a tilemap, but it's easier to just do it here */
	count = 0;
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 64; x++)
		{
			uint16_t tileno = m_tx_tilemap_ram[count] & 0x1fff;
			uint16_t colour = m_tx_tilemap_ram[count] & 0xe000;
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
uint32_t aerofgt_state::screen_update_aerfboot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

	/* we use the priority buffer so sprites are drawn front to back */
	aerfboot_draw_sprites(screen, bitmap, cliprect);
	return 0;
}

// BOOTLEG
uint32_t aerofgt_state::screen_update_aerfboo2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

	/* we use the priority buffer so sprites are drawn front to back */
	aerfboo2_draw_sprites(screen, bitmap, cliprect, 1, -1); //ship
	aerfboo2_draw_sprites(screen, bitmap, cliprect, 1, 0); //intro
	aerfboo2_draw_sprites(screen, bitmap, cliprect, 0, -1); //enemy
	aerfboo2_draw_sprites(screen, bitmap, cliprect, 0, 0); //enemy
	return 0;
}

// BOOTLEG (still uses original sprite type)
uint32_t aerofgt_state::screen_update_wbbc97(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i, scrolly;

	m_tilemap[0]->set_scroll_rows(256);
	scrolly = m_scrolly[0];
	for (i = 0; i < 256; i++)
		m_tilemap[0]->set_scrollx((i + scrolly) & 0xff, m_rasterram[i]);
	m_tilemap[0]->set_scrolly(0, scrolly);

	screen.priority().fill(0, cliprect);

	if (m_wbbc97_bitmap_enable)
	{
		wbbc97_draw_bitmap(bitmap);
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
