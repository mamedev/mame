#include "driver.h"
#include "video/konamiic.h"

static int layer_colorbase[2];
extern int bladestl_spritebank;


PALETTE_INIT( bladestl )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x30);

	/* characters use pens 0x00-0x1f, no look-up table */
	for (i = 0; i < 0x20; i++)
		colortable_entry_set_value(machine->colortable, i, i);

	/* sprites use pens 0x20-0x2f */
	for (i = 0x20; i < 0x120; i++)
	{
		UINT8 ctabentry = (color_prom[i - 0x20] & 0x0f) | 0x20;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}


static void set_pens(running_machine *machine)
{
	int i;

	for (i = 0x00; i < 0x60; i += 2)
	{
		UINT16 data = machine->generic.paletteram.u8[i | 1] | (machine->generic.paletteram.u8[i] << 8);

		rgb_t color = MAKE_RGB(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));

		colortable_palette_set_color(machine->colortable, i >> 1, color);
	}
}



/***************************************************************************

  Callback for the K007342

***************************************************************************/

static void tile_callback(int layer, int bank, int *code, int *color, int *flags)
{
	*code |= ((*color & 0x0f) << 8) | ((*color & 0x40) << 6);
	*color = layer_colorbase[layer];
}

/***************************************************************************

  Callback for the K007420

***************************************************************************/

static void sprite_callback(int *code,int *color)
{
	*code |= ((*color & 0xc0) << 2) + bladestl_spritebank;
	*code = (*code << 2) | ((*color & 0x30) >> 4);
	*color = 0 + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( bladestl )
{
	layer_colorbase[0] = 0;
	layer_colorbase[1] = 1;

	K007342_vh_start(machine,0,tile_callback);
	K007420_vh_start(machine,1,sprite_callback);
}

/***************************************************************************

  Screen Refresh

***************************************************************************/

VIDEO_UPDATE( bladestl )
{
	set_pens(screen->machine);

	K007342_tilemap_update();

	K007342_tilemap_draw( bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE ,0);
	K007420_sprites_draw( bitmap, cliprect );
	K007342_tilemap_draw( bitmap, cliprect, 1, 1 | TILEMAP_DRAW_OPAQUE ,0);
	K007342_tilemap_draw( bitmap, cliprect, 0, 0 ,0);
	K007342_tilemap_draw( bitmap, cliprect, 0, 1 ,0);
	return 0;
}
