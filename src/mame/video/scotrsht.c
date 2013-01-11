#include "emu.h"
#include "includes/scotrsht.h"


/* Similar as Iron Horse */
void scotrsht_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	/* allocate the colortable */
	machine().colortable = colortable_alloc(machine(), 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine().colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* characters use colors 0x80-0xff, sprites use colors 0-0x7f */
	for (i = 0; i < 0x200; i++)
	{
		int j;

		for (j = 0; j < 8; j++)
		{
			UINT8 ctabentry = ((~i & 0x100) >> 1) | (j << 4) | (color_prom[i] & 0x0f);
			colortable_entry_set_value(machine().colortable, ((i & 0x100) << 3) | (j << 8) | (i & 0xff), ctabentry);
		}
	}
}

WRITE8_MEMBER(scotrsht_state::scotrsht_videoram_w)
{

	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(scotrsht_state::scotrsht_colorram_w)
{

	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(scotrsht_state::scotrsht_charbank_w)
{

	if (m_charbank != (data & 0x01))
	{
		m_charbank = data & 0x01;
		m_bg_tilemap->mark_all_dirty();
	}

	/* other bits unknown */
}

WRITE8_MEMBER(scotrsht_state::scotrsht_palettebank_w)
{

	if (m_palette_bank != ((data & 0x70) >> 4))
	{
		m_palette_bank = ((data & 0x70) >> 4);
		m_bg_tilemap->mark_all_dirty();
	}

	coin_counter_w(machine(), 0, data & 1);
	coin_counter_w(machine(), 1, data & 2);

	// data & 4 unknown
}


TILE_GET_INFO_MEMBER(scotrsht_state::scotrsht_get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + (m_charbank << 9) + ((attr & 0x40) << 2);
	int color = (attr & 0x0f) + m_palette_bank * 16;
	int flag = 0;

	if(attr & 0x10) flag |= TILE_FLIPX;
	if(attr & 0x20) flag |= TILE_FLIPY;

	// data & 0x80 -> tile priority?

	SET_TILE_INFO_MEMBER(0, code, color, flag);
}

/* Same as Jailbreak + palette bank */
static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	scotrsht_state *state = machine.driver_data<scotrsht_state>();
	UINT8 *spriteram = state->m_spriteram;
	int i;

	for (i = 0; i < state->m_spriteram.bytes(); i += 4)
	{
		int attr = spriteram[i + 1];    // attributes = ?tyxcccc
		int code = spriteram[i] + ((attr & 0x40) << 2);
		int color = (attr & 0x0f) + state->m_palette_bank * 16;
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = spriteram[i + 2] - ((attr & 0x80) << 1);
		int sy = spriteram[i + 3];

		if (state->flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transmask(bitmap, cliprect, machine.gfx[1], code, color, flipx, flipy,
			sx, sy,
			colortable_get_transpen_mask(machine.colortable, machine.gfx[1], color, state->m_palette_bank * 16));
	}
}

void scotrsht_state::video_start()
{

	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(scotrsht_state::scotrsht_get_bg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);

	m_bg_tilemap->set_scroll_cols(64);
}

UINT32 scotrsht_state::screen_update_scotrsht(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int col;

	for (col = 0; col < 32; col++)
		m_bg_tilemap->set_scrolly(col, m_scroll[col]);

	m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(machine(), bitmap, cliprect);
	return 0;
}
