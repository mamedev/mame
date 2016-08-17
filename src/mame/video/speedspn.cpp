// license:BSD-3-Clause
// copyright-holders:David Haywood, Farfetch'd
/* Speed Spin video, see driver file for notes */

#include "emu.h"
#include "includes/speedspn.h"


TILE_GET_INFO_MEMBER(speedspn_state::get_tile_info)
{
	int code = m_vidram[tile_index*2+1] | (m_vidram[tile_index*2] << 8);
	int attr = m_attram[tile_index^0x400];

	SET_TILE_INFO_MEMBER(0,code,attr & 0x3f,(attr & 0x80) ? TILE_FLIPX : 0);
}

void speedspn_state::video_start()
{
	m_display_disable = false;
	m_bank_vidram = 0;
	m_vidram.resize(0x1000 * 2);
	memset(&m_vidram[0], 0, 0x1000*2);
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(speedspn_state::get_tile_info),this),TILEMAP_SCAN_COLS, 8, 8,64,32);

	save_item(NAME(m_display_disable));
	save_item(NAME(m_bank_vidram));
	save_item(NAME(m_vidram));
}

WRITE8_MEMBER(speedspn_state::vidram_w)
{
	m_vidram[offset + m_bank_vidram] = data;

	if (m_bank_vidram == 0)
		m_tilemap->mark_tile_dirty(offset/2);
}

WRITE8_MEMBER(speedspn_state::attram_w)
{
	m_attram[offset] = data;

	m_tilemap->mark_tile_dirty(offset^0x400);
}

READ8_MEMBER(speedspn_state::vidram_r)
{
	return m_vidram[offset + m_bank_vidram];
}

WRITE8_MEMBER(speedspn_state::vidram_bank_w)
{
//  logerror("VidRam Bank: %04x\n", data);
	m_bank_vidram = data & 1;
	m_bank_vidram *= 0x1000;
}

WRITE8_MEMBER(speedspn_state::display_disable_w)
{
//  logerror("Global display: %u\n", data);
	m_display_disable = data & 1;
}


void speedspn_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(1);
	UINT8 *source = &m_vidram[0x1000];
	UINT8 *finish = source + 0x1000;

	while( source<finish )
	{
		int xpos = source[0];
		int tileno = source[1];
		int attr = source[2];
		int ypos = source[3];
		int color;

		if (!attr && xpos) break; // end of sprite list marker?

		if (attr&0x10) xpos +=0x100;

		xpos = 0x1f8-xpos;
		tileno += ((attr & 0xe0) >> 5) * 0x100;
		color = attr & 0x0f;

		gfx->transpen(bitmap,cliprect,
				tileno,
				color,
				0,0,
				xpos,ypos,15);

		source +=4;
	}
}


UINT32 speedspn_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_display_disable)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

#if 0
	{
		FILE* f;
		f = fopen("vidram.bin","wb");
		fwrite(m_vidram, 1, 0x1000 * 2, f);
		fclose(f);
	}
#endif
	m_tilemap->set_scrollx(0, 0x100); // verify
	m_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	return 0;
}
