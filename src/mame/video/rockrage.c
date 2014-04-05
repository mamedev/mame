#include "emu.h"
#include "includes/rockrage.h"

PALETTE_INIT_MEMBER(rockrage_state, rockrage)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* sprites */
	for (i = 0x20; i < 0x40; i++)
		palette.set_pen_indirect(i, i);

	/* characters */
	for (i = 0x40; i < 0x140; i++)
	{
		UINT8 ctabentry;

		ctabentry = (color_prom[(i - 0x40) + 0x000] & 0x0f) | 0x00;
		palette.set_pen_indirect(i + 0x000, ctabentry);

		ctabentry = (color_prom[(i - 0x40) + 0x100] & 0x0f) | 0x10;
		palette.set_pen_indirect(i + 0x100, ctabentry);
	}
}


void rockrage_state::set_pens()
{
	int i;

	for (i = 0x00; i < 0x80; i += 2)
	{
		UINT16 data = m_paletteram[i] | (m_paletteram[i | 1] << 8);

		rgb_t color = rgb_t(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));

		m_palette->set_indirect_color(i >> 1, color);
	}
}


/***************************************************************************

  Callback for the K007342

***************************************************************************/

K007342_CALLBACK_MEMBER(rockrage_state::rockrage_tile_callback)
{
	if (layer == 1)
		*code |= ((*color & 0x40) << 2) | ((bank & 0x01) << 9);
	else
		*code |= ((*color & 0x40) << 2) | ((bank & 0x03) << 10) | ((m_vreg & 0x04) << 7) | ((m_vreg & 0x08) << 9);
	*color = m_layer_colorbase[layer] + (*color & 0x0f);
}

/***************************************************************************

  Callback for the K007420

***************************************************************************/

K007420_CALLBACK_MEMBER(rockrage_state::rockrage_sprite_callback)
{
	*code |= ((*color & 0x40) << 2) | ((*color & 0x80) << 1) * ((m_vreg & 0x03) << 1);
	*code = (*code << 2) | ((*color & 0x30) >> 4);
	*color = 0;
}


WRITE8_MEMBER(rockrage_state::rockrage_vreg_w)
{
	/* bits 4-7: unused */
	/* bit 3: bit 4 of bank # (layer 0) */
	/* bit 2: bit 1 of bank # (layer 0) */
	/* bits 0-1: sprite bank select */

	if ((data & 0x0c) != (m_vreg & 0x0c))
		machine().tilemap().mark_all_dirty();

	m_vreg = data;
}

/***************************************************************************

  Screen Refresh

***************************************************************************/

UINT32 rockrage_state::screen_update_rockrage(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();

	m_k007342->tilemap_update();

	m_k007342->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);
	m_k007420->sprites_draw(bitmap, cliprect, m_gfxdecode->gfx(1));
	m_k007342->tilemap_draw(screen, bitmap, cliprect, 0, 1 | TILEMAP_DRAW_OPAQUE, 0);
	m_k007342->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	m_k007342->tilemap_draw(screen, bitmap, cliprect, 1, 1, 0);
	return 0;
}
