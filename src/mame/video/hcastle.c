/***************************************************************************

    Haunted Castle video emulation

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/hcastle.h"


void hcastle_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int chip;

	/* allocate the colortable */
	machine().colortable = colortable_alloc(machine(), 0x80);

	for (chip = 0; chip < 2; chip++)
	{
		int pal;

		for (pal = 0; pal < 8; pal++)
		{
			int i;
			int clut = (chip << 1) | (pal & 1);

			for (i = 0; i < 0x100; i++)
			{
				UINT8 ctabentry;

				if (((pal & 0x01) == 0) && (color_prom[(clut << 8) | i] == 0))
					ctabentry = 0;
				else
					ctabentry = (pal << 4) | (color_prom[(clut << 8) | i] & 0x0f);

				colortable_entry_set_value(machine().colortable, (chip << 11) | (pal << 8) | i, ctabentry);
			}
		}
	}
}


static void set_pens(running_machine &machine)
{
	hcastle_state *state = machine.driver_data<hcastle_state>();
	int i;

	for (i = 0x00; i < 0x100; i += 2)
	{
		UINT16 data = state->m_paletteram[i | 1] | (state->m_paletteram[i] << 8);

		rgb_t color = MAKE_RGB(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));

		colortable_palette_set_color(machine.colortable, i >> 1, color);
	}
}



/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(hcastle_state::tilemap_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 6);	/* skip 0x400 */
}

TILE_GET_INFO_MEMBER(hcastle_state::get_fg_tile_info)
{
	UINT8 ctrl_5 = k007121_ctrlram_r(m_k007121_1, generic_space(), 5);
	UINT8 ctrl_6 = k007121_ctrlram_r(m_k007121_1, generic_space(), 6);
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int attr = m_pf1_videoram[tile_index];
	int tile = m_pf1_videoram[tile_index + 0x400];
	int color = attr & 0x7;
	int bank =  ((attr & 0x80) >> 7) |
				((attr >> (bit0 + 2)) & 0x02) |
				((attr >> (bit1 + 1)) & 0x04) |
				((attr >> (bit2    )) & 0x08) |
				((attr >> (bit3 - 1)) & 0x10);

	SET_TILE_INFO_MEMBER(
			0,
			tile + bank * 0x100 + m_pf1_bankbase,
			((ctrl_6 & 0x30) * 2 + 16) + color,
			0);
}

TILE_GET_INFO_MEMBER(hcastle_state::get_bg_tile_info)
{
	UINT8 ctrl_5 = k007121_ctrlram_r(m_k007121_2, generic_space(), 5);
	UINT8 ctrl_6 = k007121_ctrlram_r(m_k007121_2, generic_space(), 6);
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int attr = m_pf2_videoram[tile_index];
	int tile = m_pf2_videoram[tile_index + 0x400];
	int color = attr & 0x7;
	int bank =  ((attr & 0x80) >> 7) |
				((attr >> (bit0 + 2)) & 0x02) |
				((attr >> (bit1 + 1)) & 0x04) |
				((attr >> (bit2    )) & 0x08) |
				((attr >> (bit3 - 1)) & 0x10);

	SET_TILE_INFO_MEMBER(
			1,
			tile + bank * 0x100 + m_pf2_bankbase,
			((ctrl_6 & 0x30) * 2 + 16) + color,
			0);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void hcastle_state::video_start()
{

	m_fg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(hcastle_state::get_fg_tile_info),this), tilemap_mapper_delegate(FUNC(hcastle_state::tilemap_scan),this), 8, 8, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(hcastle_state::get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(hcastle_state::tilemap_scan),this), 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);
}



/***************************************************************************

    Memory handlers

***************************************************************************/

WRITE8_MEMBER(hcastle_state::hcastle_pf1_video_w)
{
	m_pf1_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0xbff);
}

WRITE8_MEMBER(hcastle_state::hcastle_pf2_video_w)
{
	m_pf2_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0xbff);
}

WRITE8_MEMBER(hcastle_state::hcastle_gfxbank_w)
{
	m_gfx_bank = data;
}

READ8_MEMBER(hcastle_state::hcastle_gfxbank_r)
{
	return m_gfx_bank;
}

WRITE8_MEMBER(hcastle_state::hcastle_pf1_control_w)
{

	if (offset == 3)
	{
		if ((data & 0x8) == 0)
			m_spriteram->copy(0x800, 0x800);
		else
			m_spriteram->copy(0x000, 0x800);
	}
	else if (offset == 7)
	{
		m_fg_tilemap->set_flip((data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	}
	k007121_ctrl_w(m_k007121_1, space, offset, data);
}

WRITE8_MEMBER(hcastle_state::hcastle_pf2_control_w)
{

	if (offset == 3)
	{
		if ((data & 0x8) == 0)
			m_spriteram2->copy(0x800, 0x800);
		else
			m_spriteram2->copy(0x000, 0x800);
	}
	else if (offset == 7)
	{
		m_bg_tilemap->set_flip((data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	}
	k007121_ctrl_w(m_k007121_2, space, offset, data);
}

/*****************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *sbank, int bank )
{
	hcastle_state *state = machine.driver_data<hcastle_state>();
	device_t *k007121 = bank ? state->m_k007121_2 : state->m_k007121_1;
	address_space &space = machine.driver_data()->generic_space();
	int base_color = (k007121_ctrlram_r(k007121, space, 6) & 0x30) * 2;
	int bank_base = (bank == 0) ? 0x4000 * (state->m_gfx_bank & 1) : 0;

	k007121_sprites_draw(k007121, bitmap, cliprect, machine.gfx[bank], machine.colortable, sbank, base_color, 0, bank_base, (UINT32)-1);
}

/*****************************************************************************/

UINT32 hcastle_state::screen_update_hcastle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();

	UINT8 ctrl_1_0 = k007121_ctrlram_r(m_k007121_1, space, 0);
	UINT8 ctrl_1_1 = k007121_ctrlram_r(m_k007121_1, space, 1);
	UINT8 ctrl_1_2 = k007121_ctrlram_r(m_k007121_1, space, 2);
	UINT8 ctrl_1_3 = k007121_ctrlram_r(m_k007121_1, space, 3);
	UINT8 ctrl_2_0 = k007121_ctrlram_r(m_k007121_2, space, 0);
	UINT8 ctrl_2_1 = k007121_ctrlram_r(m_k007121_2, space, 1);
	UINT8 ctrl_2_2 = k007121_ctrlram_r(m_k007121_2, space, 2);
	UINT8 ctrl_2_3 = k007121_ctrlram_r(m_k007121_2, space, 3);

	set_pens(machine());

	m_pf1_bankbase = 0x0000;
	m_pf2_bankbase = 0x4000 * ((m_gfx_bank & 2) >> 1);

	if (ctrl_1_3 & 0x01)
		m_pf1_bankbase += 0x2000;
	if (ctrl_2_3 & 0x01)
		m_pf2_bankbase += 0x2000;

	if (m_pf1_bankbase != m_old_pf1)
		m_fg_tilemap->mark_all_dirty();

	if (m_pf2_bankbase != m_old_pf2)
		m_bg_tilemap->mark_all_dirty();

	m_old_pf1 = m_pf1_bankbase;
	m_old_pf2 = m_pf2_bankbase;

	m_bg_tilemap->set_scrolly(0, ctrl_2_2);
	m_bg_tilemap->set_scrollx(0, ((ctrl_2_1 << 8) + ctrl_2_0));
	m_fg_tilemap->set_scrolly(0, ctrl_1_2);
	m_fg_tilemap->set_scrollx(0, ((ctrl_1_1 << 8) + ctrl_1_0));

//  /* Sprite priority */
//  if (ctrl_1_3 & 0x20)
	if ((m_gfx_bank & 0x04) == 0)
	{
		m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
		draw_sprites(machine(), bitmap, cliprect, m_spriteram->buffer(), 0);
		draw_sprites(machine(), bitmap, cliprect, m_spriteram2->buffer(), 1);
		m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	}
	else
	{
		m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
		m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
		draw_sprites(machine(), bitmap, cliprect, m_spriteram->buffer(), 0);
		draw_sprites(machine(), bitmap, cliprect, m_spriteram2->buffer(), 1);
	}
	return 0;
}
