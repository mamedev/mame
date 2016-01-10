// license:???
// copyright-holders:Paul Leaman
#include "emu.h"
#include "includes/blktiger.h"


#define BGRAM_BANK_SIZE 0x1000
#define BGRAM_BANKS 4


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(blktiger_state::bg8x4_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x70) << 4) + ((row & 0x30) << 7);
}

TILEMAP_MAPPER_MEMBER(blktiger_state::bg4x8_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x30) << 4) + ((row & 0x70) << 6);
}

TILE_GET_INFO_MEMBER(blktiger_state::get_bg_tile_info)
{
	/* the tile priority table is a guess compiled by looking at the game. It
	   was not derived from a PROM so it could be wrong. */
	static const UINT8 split_table[16] =
	{
		3,3,2,2,
		1,1,0,0,
		0,0,0,0,
		0,0,0,0
	};
	UINT8 attr = m_scroll_ram[2 * tile_index + 1];
	int color = (attr & 0x78) >> 3;
	SET_TILE_INFO_MEMBER(1,
			m_scroll_ram[2 * tile_index] + ((attr & 0x07) << 8),
			color,
			(attr & 0x80) ? TILE_FLIPX : 0);
	tileinfo.group = split_table[color];
}

TILE_GET_INFO_MEMBER(blktiger_state::get_tx_tile_info)
{
	UINT8 attr = m_txvideoram[tile_index + 0x400];
	SET_TILE_INFO_MEMBER(0,
			m_txvideoram[tile_index] + ((attr & 0xe0) << 3),
			attr & 0x1f,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void blktiger_state::video_start()
{
	m_chon = 1;
	m_bgon = 1;
	m_objon = 1;
	m_screen_layout = 0;

	m_scroll_ram = std::make_unique<UINT8[]>(BGRAM_BANK_SIZE * BGRAM_BANKS);

	m_tx_tilemap =    &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(blktiger_state::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap8x4 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(blktiger_state::get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(blktiger_state::bg8x4_scan),this), 16, 16, 128, 64);
	m_bg_tilemap4x8 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(blktiger_state::get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(blktiger_state::bg4x8_scan),this), 16, 16, 64, 128);

	m_tx_tilemap->set_transparent_pen(3);

	m_bg_tilemap8x4->set_transmask(0, 0xffff, 0x8000);  /* split type 0 is totally transparent in front half */
	m_bg_tilemap8x4->set_transmask(1, 0xfff0, 0x800f);  /* split type 1 has pens 4-15 transparent in front half */
	m_bg_tilemap8x4->set_transmask(2, 0xff00, 0x80ff);  /* split type 1 has pens 8-15 transparent in front half */
	m_bg_tilemap8x4->set_transmask(3, 0xf000, 0x8fff);  /* split type 1 has pens 12-15 transparent in front half */
	m_bg_tilemap4x8->set_transmask(0, 0xffff, 0x8000);
	m_bg_tilemap4x8->set_transmask(1, 0xfff0, 0x800f);
	m_bg_tilemap4x8->set_transmask(2, 0xff00, 0x80ff);
	m_bg_tilemap4x8->set_transmask(3, 0xf000, 0x8fff);

	save_pointer(NAME(m_scroll_ram.get()), BGRAM_BANK_SIZE * BGRAM_BANKS);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(blktiger_state::blktiger_txvideoram_w)
{
	m_txvideoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
}

READ8_MEMBER(blktiger_state::blktiger_bgvideoram_r)
{
	return m_scroll_ram[offset + m_scroll_bank];
}

WRITE8_MEMBER(blktiger_state::blktiger_bgvideoram_w)
{
	offset += m_scroll_bank;

	m_scroll_ram[offset] = data;
	m_bg_tilemap8x4->mark_tile_dirty(offset / 2);
	m_bg_tilemap4x8->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(blktiger_state::blktiger_bgvideoram_bank_w)
{
	m_scroll_bank = (data % BGRAM_BANKS) * BGRAM_BANK_SIZE;
}


WRITE8_MEMBER(blktiger_state::blktiger_scrolly_w)
{
	int scrolly;

	m_scroll_y[offset] = data;
	scrolly = m_scroll_y[0] | (m_scroll_y[1] << 8);
	m_bg_tilemap8x4->set_scrolly(0, scrolly);
	m_bg_tilemap4x8->set_scrolly(0, scrolly);
}

WRITE8_MEMBER(blktiger_state::blktiger_scrollx_w)
{
	int scrollx;

	m_scroll_x[offset] = data;
	scrollx = m_scroll_x[0] | (m_scroll_x[1] << 8);
	m_bg_tilemap8x4->set_scrollx(0, scrollx);
	m_bg_tilemap4x8->set_scrollx(0, scrollx);
}


WRITE8_MEMBER(blktiger_state::blktiger_video_control_w)
{
	/* bits 0 and 1 are coin counters */
	machine().bookkeeping().coin_counter_w(0,data & 1);
	machine().bookkeeping().coin_counter_w(1,data & 2);

	/* bit 5 resets the sound CPU */
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x20) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 6 flips screen */
	flip_screen_set(data & 0x40);

	/* bit 7 enables characters? Just a guess */
	m_chon = ~data & 0x80;
}

WRITE8_MEMBER(blktiger_state::blktiger_video_enable_w)
{
	/* not sure which is which, but I think that bit 1 and 2 enable background and sprites */
	/* bit 1 enables bg ? */
	m_bgon = ~data & 0x02;

	/* bit 2 enables sprites ? */
	m_objon = ~data & 0x04;
}

WRITE8_MEMBER(blktiger_state::blktiger_screen_layout_w)
{
	m_screen_layout = data;
	m_bg_tilemap8x4->enable(m_screen_layout);
	m_bg_tilemap4x8->enable(!m_screen_layout);
}



/***************************************************************************

  Display refresh

***************************************************************************/

void blktiger_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *buffered_spriteram = m_spriteram->buffer();
	int offs;

	/* Draw the sprites. */
	for (offs = m_spriteram->bytes() - 4;offs >= 0;offs -= 4)
	{
		int attr = buffered_spriteram[offs+1];
		int sx = buffered_spriteram[offs + 3] - ((attr & 0x10) << 4);
		int sy = buffered_spriteram[offs + 2];
		int code = buffered_spriteram[offs] | ((attr & 0xe0) << 3);
		int color = attr & 0x07;
		int flipx = attr & 0x08;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flip_screen(),
				sx,sy,15);
	}
}

UINT32 blktiger_state::screen_update_blktiger(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(1023, cliprect);

	if (m_bgon)
		(m_screen_layout ? m_bg_tilemap8x4 : m_bg_tilemap4x8)->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);

	if (m_objon)
		draw_sprites(bitmap, cliprect);

	if (m_bgon)
		(m_screen_layout ? m_bg_tilemap8x4 : m_bg_tilemap4x8)->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);

	if (m_chon)
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
