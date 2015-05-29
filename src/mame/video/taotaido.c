// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Tao Taido Video Hardware */

/*

its similar to other video system games

zooming might be wrong (only used on title logo?)

*/

#include "emu.h"
#include "vsystem_spr.h"
#include "includes/taotaido.h"

/* sprite tile codes 0x4000 - 0x7fff get remapped according to the content of these registers */
WRITE16_MEMBER(taotaido_state::sprite_character_bank_select_w)
{
	if(ACCESSING_BITS_8_15)
		m_sprite_character_bank_select[offset*2] = data >> 8;
	if(ACCESSING_BITS_0_7)
		m_sprite_character_bank_select[offset*2+1] = data &0xff;
}

/* sprites are like the other video system / psikyo games, we can merge this with aerofgt and plenty of other
   things eventually */


/* the tilemap */

WRITE16_MEMBER(taotaido_state::tileregs_w)
{
	switch (offset)
	{
		case 0: // would normally be x scroll?
		case 1: // would normally be y scroll?
		case 2:
		case 3:
			logerror ("unhanded tilemap register write offset %02x data %04x \n",offset,data);
			break;

		/* tile banks */
		case 4:
		case 5:
		case 6:
		case 7:
			if(ACCESSING_BITS_8_15)
				m_video_bank_select[(offset-4)*2] = data >> 8;
			if(ACCESSING_BITS_0_7)
				m_video_bank_select[(offset-4)*2+1] = data &0xff;
				m_bg_tilemap->mark_all_dirty();
			break;
	}
}

WRITE16_MEMBER(taotaido_state::bgvideoram_w)
{
	COMBINE_DATA(&m_bgram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(taotaido_state::bg_tile_info)
{
	int code = m_bgram[tile_index]&0x01ff;
	int bank = (m_bgram[tile_index]&0x0e00)>>9;
	int col  = (m_bgram[tile_index]&0xf000)>>12;

	code |= m_video_bank_select[bank]*0x200;

	SET_TILE_INFO_MEMBER(1,
			code,
			col,
			0);
}

TILEMAP_MAPPER_MEMBER(taotaido_state::tilemap_scan_rows)
{
	/* logical (col,row) -> memory offset */
	return row*0x40 + (col&0x3f) + ((col&0x40)<<6);
}


UINT32 taotaido_state::tile_callback( UINT32 code )
{
	code = m_spriteram2_older[code&0x7fff];

	if (code > 0x3fff)
	{
		int block = (code & 0x3800)>>11;
		code &= 0x07ff;
		code |= m_sprite_character_bank_select[block] * 0x800;
	}

	return code;
}


void taotaido_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(taotaido_state::bg_tile_info),this),tilemap_mapper_delegate(FUNC(taotaido_state::tilemap_scan_rows),this),16,16,128,64);

	m_spriteram_old = auto_alloc_array(machine(), UINT16, 0x2000/2);
	m_spriteram_older = auto_alloc_array(machine(), UINT16, 0x2000/2);

	m_spriteram2_old = auto_alloc_array(machine(), UINT16, 0x10000/2);
	m_spriteram2_older = auto_alloc_array(machine(), UINT16, 0x10000/2);

	save_item(NAME(m_sprite_character_bank_select));
	save_item(NAME(m_video_bank_select));
}


UINT32 taotaido_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  m_bg_tilemap->set_scrollx(0,(m_scrollram[0x380/2]>>4)); // the values put here end up being wrong every other frame
//  m_bg_tilemap->set_scrolly(0,(m_scrollram[0x382/2]>>4)); // the values put here end up being wrong every other frame

	/* not amazingly efficient however it should be functional for row select and linescroll */
	int line;
	rectangle clip;

	const rectangle &visarea = screen.visible_area();
	clip = visarea;

	for (line = 0; line < 224;line++)
	{
		clip.min_y = clip.max_y = line;

		m_bg_tilemap->set_scrollx(0,((m_scrollram[(0x00+4*line)/2])>>4)+30);
		m_bg_tilemap->set_scrolly(0,((m_scrollram[(0x02+4*line)/2])>>4)-line);

		m_bg_tilemap->draw(screen, bitmap, clip, 0,0);
	}

	m_spr->draw_sprites(m_spriteram_older, m_spriteram.bytes(), screen, bitmap,cliprect);
	return 0;
}

void taotaido_state::screen_eof(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		/* sprites need to be delayed by 2 frames? */

		memcpy(m_spriteram2_older,m_spriteram2_old,0x10000);
		memcpy(m_spriteram2_old,m_spriteram2,0x10000);

		memcpy(m_spriteram_older,m_spriteram_old,0x2000);
		memcpy(m_spriteram_old,m_spriteram,0x2000);
	}
}
