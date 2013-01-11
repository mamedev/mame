#include "emu.h"
#include "includes/xyonix.h"

void xyonix_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;


	for (i = 0;i < machine().total_colors();i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 5) & 0x01;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color(machine(),i,MAKE_RGB(r,g,b));
	}
}


TILE_GET_INFO_MEMBER(xyonix_state::get_xyonix_tile_info)
{
	int tileno;
	int attr = m_vidram[tile_index+0x1000+1];

	tileno = (m_vidram[tile_index+1] << 0) | ((attr & 0x0f) << 8);

	SET_TILE_INFO_MEMBER(0,tileno,attr >> 4,0);
}

WRITE8_MEMBER(xyonix_state::xyonix_vidram_w)
{
	m_vidram[offset] = data;
	m_tilemap->mark_tile_dirty((offset-1)&0x0fff);
}

void xyonix_state::video_start()
{
	m_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(xyonix_state::get_xyonix_tile_info),this), TILEMAP_SCAN_ROWS, 4, 8, 80, 32);
}

UINT32 xyonix_state::screen_update_xyonix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
