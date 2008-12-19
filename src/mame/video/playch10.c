#include "driver.h"
#include "video/ppu2c0x.h"
#include "includes/playch10.h"

static int pc10_bios;

static tilemap *bg_tilemap;

WRITE8_HANDLER( playch10_videoram_w )
{
	if (pc10_sdcs)
	{
		videoram[offset] = data;
		tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
	}
}

PALETTE_INIT( playch10 )
{
	int i;

	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		/* red component */

		bit0 = ~(color_prom[0] >> 0) & 0x01;
		bit1 = ~(color_prom[0] >> 1) & 0x01;
		bit2 = ~(color_prom[0] >> 2) & 0x01;
		bit3 = ~(color_prom[0] >> 3) & 0x01;

		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = ~(color_prom[256] >> 0) & 0x01;
		bit1 = ~(color_prom[256] >> 1) & 0x01;
		bit2 = ~(color_prom[256] >> 2) & 0x01;
		bit3 = ~(color_prom[256] >> 3) & 0x01;

		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */

		bit0 = ~(color_prom[2*256] >> 0) & 0x01;
		bit1 = ~(color_prom[2*256] >> 1) & 0x01;
		bit2 = ~(color_prom[2*256] >> 2) & 0x01;
		bit3 = ~(color_prom[2*256] >> 3) & 0x01;

		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		color_prom++;
	}

	ppu2c0x_init_palette(machine, 256 );
}

static void ppu_irq( running_machine *machine, int num, int *ppu_regs )
{
	cpu_set_input_line(machine->cpu[1], INPUT_LINE_NMI, PULSE_LINE );
	pc10_int_detect = 1;
}

/* our ppu interface                                           */
/* things like mirroring and whether to use vrom or vram       */
/* can be set by calling 'ppu2c0x_override_hardware_options'   */

static const ppu2c0x_interface ppu_interface =
{
	PPU_2C03B,				/* type */
	1,						/* num */
	{ "gfx2" },				/* vrom gfx region */
	{ 1 },					/* gfxlayout num */
	{ 256 },				/* color base */
	{ PPU_MIRROR_NONE },	/* mirroring */
	{ ppu_irq },			/* irq */
	{ 0 }					/* vram */
};

static const ppu2c0x_interface ppu_interface_hboard =
{
	PPU_2C03B,				/* type */
	1,						/* num */
	{ "gfx2" },				/* vrom gfx region */
	{ 1 },					/* gfxlayout num */
	{ 256 },				/* color base */
	{ PPU_MIRROR_NONE },	/* mirroring */
	{ ppu_irq },			/* irq */
	{ 1 }					/* vram */
};

static TILE_GET_INFO( get_bg_tile_info )
{
	int offs = tile_index * 2;
	int code = videoram[offs] + ((videoram[offs + 1] & 0x07) << 8);
	int color = (videoram[offs + 1] >> 3) & 0x1f;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( playch10 )
{
	const UINT8 *bios = memory_region(machine, "main");
	pc10_bios = (bios[3] == 0x2a) ? 1 : 2;

	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	ppu2c0x_init(machine, &ppu_interface );
}

VIDEO_START( playch10_hboard )
{
	const UINT8 *bios = memory_region(machine, "main");
	pc10_bios = (bios[3] == 0x2a) ? 1 : 2;

	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	ppu2c0x_init(machine, &ppu_interface_hboard );
}

/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( playch10 )
{
	/* Dual monitor version */
	if (pc10_bios == 1)
	{
		const device_config *top_screen = device_list_find_by_tag(screen->machine->config->devicelist, VIDEO_SCREEN, "top");

		/* On Playchoice 10 single monitor, this bit toggles    */
		/* between PPU and BIOS display.                        */
		/* We support the multi-monitor layout. In this case,   */
		/* if the bit is not set, then we should display        */
		/* the PPU portion.                                     */

		if (screen == top_screen)
		{
			if ( !pc10_dispmask )
				/* render the ppu */
				ppu2c0x_render( 0, bitmap, 0, 0, 0, 0 );
			else
				bitmap_fill(bitmap, cliprect, 0);
		}
		else
		{
			/* When the bios is accessing vram, the video circuitry can't access it */

			if ( !pc10_sdcs )
				tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
			else
				bitmap_fill(bitmap, cliprect, 0);
		}
	}
	else	/* Single Monitor version */
	{
		rectangle top_monitor = *video_screen_get_visible_area(screen);

		top_monitor.max_y = ( top_monitor.max_y - top_monitor.min_y ) / 2;

		if(pc10_dispmask_old != pc10_dispmask)
		{
			pc10_dispmask_old = pc10_dispmask;

			if(pc10_dispmask)
				pc10_game_mode ^= 1;
		}

		if ( pc10_game_mode )
			/* render the ppu */
			ppu2c0x_render( 0, bitmap, 0, 0, 0, 0 );
		else
		{
			/* When the bios is accessing vram, the video circuitry can't access it */
			if ( !pc10_sdcs )
				tilemap_draw(bitmap, &top_monitor, bg_tilemap, 0, 0);
		}
	}
	return 0;
}
