#include "emu.h"
#include "video/konicdev.h"
#include "includes/rockrage.h"

void rockrage_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	/* allocate the colortable */
	machine().colortable = colortable_alloc(machine(), 0x40);

	/* sprites */
	for (i = 0x20; i < 0x40; i++)
		colortable_entry_set_value(machine().colortable, i, i);

	/* characters */
	for (i = 0x40; i < 0x140; i++)
	{
		UINT8 ctabentry;

		ctabentry = (color_prom[(i - 0x40) + 0x000] & 0x0f) | 0x00;
		colortable_entry_set_value(machine().colortable, i + 0x000, ctabentry);

		ctabentry = (color_prom[(i - 0x40) + 0x100] & 0x0f) | 0x10;
		colortable_entry_set_value(machine().colortable, i + 0x100, ctabentry);
	}
}


static void set_pens( running_machine &machine )
{
	rockrage_state *state = machine.driver_data<rockrage_state>();
	int i;

	for (i = 0x00; i < 0x80; i += 2)
	{
		UINT16 data = state->m_paletteram[i] | (state->m_paletteram[i | 1] << 8);

		rgb_t color = MAKE_RGB(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));

		colortable_palette_set_color(machine.colortable, i >> 1, color);
	}
}


/***************************************************************************

  Callback for the K007342

***************************************************************************/

void rockrage_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags )
{
	rockrage_state *state = machine.driver_data<rockrage_state>();

	if (layer == 1)
		*code |= ((*color & 0x40) << 2) | ((bank & 0x01) << 9);
	else
		*code |= ((*color & 0x40) << 2) | ((bank & 0x03) << 10) | ((state->m_vreg & 0x04) << 7) | ((state->m_vreg & 0x08) << 9);
	*color = state->m_layer_colorbase[layer] + (*color & 0x0f);
}

/***************************************************************************

  Callback for the K007420

***************************************************************************/

void rockrage_sprite_callback( running_machine &machine, int *code, int *color )
{
	rockrage_state *state = machine.driver_data<rockrage_state>();

	*code |= ((*color & 0x40) << 2) | ((*color & 0x80) << 1) * ((state->m_vreg & 0x03) << 1);
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

	set_pens(screen.machine());

	k007342_tilemap_update(m_k007342);

	k007342_tilemap_draw(m_k007342, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);
	k007420_sprites_draw(m_k007420, bitmap, cliprect, screen.machine().gfx[1]);
	k007342_tilemap_draw(m_k007342, bitmap, cliprect, 0, 1 | TILEMAP_DRAW_OPAQUE, 0);
	k007342_tilemap_draw(m_k007342, bitmap, cliprect, 1, 0, 0);
	k007342_tilemap_draw(m_k007342, bitmap, cliprect, 1, 1, 0);
	return 0;
}
