#include "emu.h"
#include "video/konicdev.h"
#include "includes/bladestl.h"


void bladestl_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	/* allocate the colortable */
	machine().colortable = colortable_alloc(machine(), 0x30);

	/* characters use pens 0x00-0x1f, no look-up table */
	for (i = 0; i < 0x20; i++)
		colortable_entry_set_value(machine().colortable, i, i);

	/* sprites use pens 0x20-0x2f */
	for (i = 0x20; i < 0x120; i++)
	{
		UINT8 ctabentry = (color_prom[i - 0x20] & 0x0f) | 0x20;
		colortable_entry_set_value(machine().colortable, i, ctabentry);
	}
}


static void set_pens( running_machine &machine )
{
	bladestl_state *state = machine.driver_data<bladestl_state>();
	int i;

	for (i = 0x00; i < 0x60; i += 2)
	{
		UINT16 data = state->m_paletteram[i | 1] | (state->m_paletteram[i] << 8);

		rgb_t color = MAKE_RGB(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));

		colortable_palette_set_color(machine.colortable, i >> 1, color);
	}
}



/***************************************************************************

  Callback for the K007342

***************************************************************************/

void bladestl_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags )
{
	bladestl_state *state = machine.driver_data<bladestl_state>();

	*code |= ((*color & 0x0f) << 8) | ((*color & 0x40) << 6);
	*color = state->m_layer_colorbase[layer];
}

/***************************************************************************

  Callback for the K007420

***************************************************************************/

void bladestl_sprite_callback( running_machine &machine, int *code,int *color )
{
	bladestl_state *state = machine.driver_data<bladestl_state>();

	*code |= ((*color & 0xc0) << 2) + state->m_spritebank;
	*code = (*code << 2) | ((*color & 0x30) >> 4);
	*color = 0 + (*color & 0x0f);
}


/***************************************************************************

  Screen Refresh

***************************************************************************/

SCREEN_UPDATE_IND16( bladestl )
{
	bladestl_state *state = screen.machine().driver_data<bladestl_state>();
	set_pens(screen.machine());

	k007342_tilemap_update(state->m_k007342);

	k007342_tilemap_draw(state->m_k007342, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE ,0);
	k007420_sprites_draw(state->m_k007420, bitmap, cliprect, screen.machine().gfx[1]);
	k007342_tilemap_draw(state->m_k007342, bitmap, cliprect, 1, 1 | TILEMAP_DRAW_OPAQUE ,0);
	k007342_tilemap_draw(state->m_k007342, bitmap, cliprect, 0, 0 ,0);
	k007342_tilemap_draw(state->m_k007342, bitmap, cliprect, 0, 1 ,0);
	return 0;
}
