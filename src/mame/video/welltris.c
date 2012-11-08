#include "emu.h"
#include "includes/welltris.h"




#ifdef UNUSED_FUNCTION
READ16_MEMBER(welltris_state::welltris_spriteram_r)
{
	return welltris_spriteram[offset];
}
#endif

WRITE16_MEMBER(welltris_state::welltris_spriteram_w)
{
	int offs;

	COMBINE_DATA(&m_spriteram[offset]);

	/* hack... sprite doesn't work otherwise (quiz18kn) */
	if ((offset == 0x1fe) &&
		(m_spriteram[0x01fc] == 0x0000) &&
		(m_spriteram[0x01fd] == 0x0000) &&
		(m_spriteram[0x01ff] == 0x0000)) {
		for (offs = 0; offs < 0x1fc; offs++) m_spriteram[offs] = 0x0000;
	}
}


void welltris_state::setbank(int num, int bank)
{
	if (m_gfxbank[num] != bank)
	{
		m_gfxbank[num] = bank;
		m_char_tilemap->mark_all_dirty();
	}
}


/* Not really enough evidence here */

WRITE16_MEMBER(welltris_state::welltris_palette_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{

		if (m_charpalettebank != (data & 0x03))
		{
			m_charpalettebank = (data & 0x03);
			m_char_tilemap->mark_all_dirty();
		}

		flip_screen_set(data & 0x80);

		m_spritepalettebank = (data & 0x20) >> 5;
		m_pixelpalettebank = (data & 0x08) >> 3;
	}
}

WRITE16_MEMBER(welltris_state::welltris_gfxbank_w)
{
	if (ACCESSING_BITS_0_7)
	{

		setbank(0, (data & 0xf0) >> 4);
		setbank(1, data & 0x0f);
	}
}

WRITE16_MEMBER(welltris_state::welltris_scrollreg_w)
{

	switch (offset) {
		case 0: m_scrollx = data - 14; break;
		case 1: m_scrolly = data +  0; break;
	}
}

TILE_GET_INFO_MEMBER(welltris_state::get_welltris_tile_info)
{
	UINT16 code = m_charvideoram[tile_index];
	int bank = (code & 0x1000) >> 12;

	SET_TILE_INFO_MEMBER(
			0,
			(code & 0x0fff) + (m_gfxbank[bank] << 12),
			((code & 0xe000) >> 13) + (8 * m_charpalettebank),
			0);
}

WRITE16_MEMBER(welltris_state::welltris_charvideoram_w)
{

	COMBINE_DATA(&m_charvideoram[offset]);
	m_char_tilemap->mark_tile_dirty(offset);
}

void welltris_state::video_start()
{
	m_char_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(welltris_state::get_welltris_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);

	m_char_tilemap->set_transparent_pen(15);
}

static void draw_background(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	welltris_state *state = machine.driver_data<welltris_state>();
	int x, y;
	int pixdata;

	for (y = 0; y < 256; y++) {
		for (x = 0; x < 512 / 2; x++) {
			pixdata = state->m_pixelram[(x & 0xff) + (y & 0xff) * 256];

			bitmap.pix16(y, (x * 2) + 0) = (pixdata >> 8) + (0x100 * state->m_pixelpalettebank) + 0x400;
			bitmap.pix16(y, (x * 2) + 1) = (pixdata & 0xff) + (0x100 * state->m_pixelpalettebank) + 0x400;
		}
	}
}

UINT32 welltris_state::screen_update_welltris(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_char_tilemap->set_scrollx(0, m_scrollx);
	m_char_tilemap->set_scrolly(0, m_scrolly);

	draw_background(machine(), bitmap, cliprect);
	m_char_tilemap->draw(bitmap, cliprect, 0, 0);
	m_spr_old->turbofrc_draw_sprites(m_spriteram, m_spriteram.bytes(), m_spritepalettebank, machine(), bitmap, cliprect, 0);
	return 0;
}
