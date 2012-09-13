/* Speed Spin video, see driver file for notes */

#include "emu.h"
#include "includes/speedspn.h"


TILE_GET_INFO_MEMBER(speedspn_state::get_speedspn_tile_info)
{
	int code = m_vidram[tile_index*2+1] | (m_vidram[tile_index*2] << 8);
	int attr = m_attram[tile_index^0x400];

	SET_TILE_INFO_MEMBER(0,code,attr & 0x3f,(attr & 0x80) ? TILE_FLIPX : 0);
}

void speedspn_state::video_start()
{
	m_vidram = auto_alloc_array(machine(), UINT8, 0x1000 * 2);
	m_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(speedspn_state::get_speedspn_tile_info),this),TILEMAP_SCAN_COLS, 8, 8,64,32);
}

WRITE8_MEMBER(speedspn_state::speedspn_vidram_w)
{
	m_vidram[offset + m_bank_vidram] = data;

	if (m_bank_vidram == 0)
		m_tilemap->mark_tile_dirty(offset/2);
}

WRITE8_MEMBER(speedspn_state::speedspn_attram_w)
{
	m_attram[offset] = data;

	m_tilemap->mark_tile_dirty(offset^0x400);
}

READ8_MEMBER(speedspn_state::speedspn_vidram_r)
{
	return m_vidram[offset + m_bank_vidram];
}

WRITE8_MEMBER(speedspn_state::speedspn_banked_vidram_change)
{
//  logerror("VidRam Bank: %04x\n", data);
	m_bank_vidram = data & 1;
	m_bank_vidram *= 0x1000;
}

WRITE8_MEMBER(speedspn_state::speedspn_global_display_w)
{
//  logerror("Global display: %u\n", data);
	m_display_disable = data & 1;
}


static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	speedspn_state *state = machine.driver_data<speedspn_state>();
	gfx_element *gfx = machine.gfx[1];
	UINT8 *source = state->m_vidram+ 0x1000;
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

		drawgfx_transpen(bitmap,cliprect,gfx,
				tileno,
				color,
				0,0,
				xpos,ypos,15);

		source +=4;
	}
}


SCREEN_UPDATE_IND16(speedspn)
{
	speedspn_state *state = screen.machine().driver_data<speedspn_state>();
	if (state->m_display_disable)
	{
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
		return 0;
	}

#if 0
	{
		FILE* f;
		f = fopen("vidram.bin","wb");
		fwrite(state->m_vidram, 1, 0x1000 * 2, f);
		fclose(f);
	}
#endif
	state->m_tilemap->set_scrollx(0, 0x100); // verify
	state->m_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap,cliprect);
	return 0;
}
