#include "driver.h"
#include "deprecat.h"
#include "cpu/m6502/m6502.h"


UINT8 *spdodgeb_videoram;

static int tile_palbank;
static int sprite_palbank;

static tilemap *bg_tilemap;



PALETTE_INIT( spdodgeb )
{
	int i;


	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		/* red component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[machine->config->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[machine->config->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[machine->config->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[machine->config->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( background_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 code = spdodgeb_videoram[tile_index];
	UINT8 attr = spdodgeb_videoram[tile_index + 0x800];
	SET_TILE_INFO(
			0,
			code + ((attr & 0x1f) << 8),
			((attr & 0xe0) >> 5) + 8 * tile_palbank,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( spdodgeb )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info,background_scan,8,8,64,32);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

static int lastscroll;

INTERRUPT_GEN( spdodgeb_interrupt )
{
	int iloop = cpu_getiloops(device);
	int scanline = (32-iloop) * 8;

	if (iloop > 1 && iloop < 32)
	{
		cpu_set_input_line(device, M6502_IRQ_LINE, HOLD_LINE);
		video_screen_update_partial(device->machine->primary_screen, scanline+7);
	}
	else if (!iloop)
	{
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
		video_screen_update_partial(device->machine->primary_screen, 256);
	}
}

WRITE8_HANDLER( spdodgeb_scrollx_lo_w )
{
	lastscroll = (lastscroll & 0x100) | data;
}

WRITE8_HANDLER( spdodgeb_ctrl_w )
{
	UINT8 *rom = memory_region(space->machine, "maincpu");

	/* bit 0 = flip screen */
	flip_screen_set(space->machine, data & 0x01);

	/* bit 1 = ROM bank switch */
	memory_set_bankptr(space->machine, 1,rom + 0x10000 + 0x4000 * ((~data & 0x02) >> 1));

	/* bit 2 = scroll high bit */
	lastscroll = (lastscroll & 0x0ff) | ((data & 0x04) << 6);

	/* bit 3 = to mcu?? */

	/* bits 4-7 = palette bank select */
	if (tile_palbank != ((data & 0x30) >> 4))
	{
		tile_palbank = ((data & 0x30) >> 4);
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
	sprite_palbank = (data & 0xc0) >> 6;
}

WRITE8_HANDLER( spdodgeb_videoram_w )
{
	spdodgeb_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x7ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

#define DRAW_SPRITE( order, sx, sy ) drawgfx_transpen( bitmap, \
					cliprect,gfx, \
					(which+order),color+ 8 * sprite_palbank,flipx,flipy,sx,sy,0);

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	const gfx_element *gfx = machine->gfx[1];
	UINT8 *src;
	int i;

	src = spriteram;

/*  240-SY   Z|F|CLR|WCH WHICH    SX
    xxxxxxxx x|x|xxx|xxx xxxxxxxx xxxxxxxx
*/
	for (i = 0;i < spriteram_size;i += 4)
	{
		int attr = src[i+1];
		int which = src[i+2]+((attr & 0x07)<<8);
		int sx = src[i+3];
		int sy = 240 - src[i];
		int size = (attr & 0x80) >> 7;
		int color = (attr & 0x38) >> 3;
		int flipx = ~attr & 0x40;
		int flipy = 0;
		int dy = -16;
		int cy;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
			dy = -dy;
		}

		if (sx < -8) sx += 256; else if (sx > 248) sx -= 256;

		switch (size)
		{
			case 0: /* normal */
			if (sy < -8) sy += 256; else if (sy > 248) sy -= 256;
			DRAW_SPRITE(0,sx,sy);
			break;

			case 1: /* double y */
			if (flip_screen_get(machine)) { if (sy > 240) sy -= 256; } else { if (sy < 0) sy += 256; }
			cy = sy + dy;
			which &= ~1;
			DRAW_SPRITE(0,sx,cy);
			DRAW_SPRITE(1,sx,sy);
			break;
		}
	}
}

#undef DRAW_SPRITE


VIDEO_UPDATE( spdodgeb )
{
	tilemap_set_scrollx(bg_tilemap,0,lastscroll+5);
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(screen->machine, bitmap,cliprect);
	return 0;
}
