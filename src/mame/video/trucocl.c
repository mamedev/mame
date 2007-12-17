/***************************************************************************

Truco Clemente (c) 1991 Miky SRL

driver by Ernesto Corvi

Notes:
- Audio is almost there.
- After one game you can't play anymore.
- I think this runs on a heavily modified PacMan type of board.

----------------------------------
Additional Notes (Roberto Fresca):
----------------------------------
Mainboard: Pacman bootleg jamma board.
Daughterboard: Custom made, plugged in the 2 roms and Z80 mainboard sockets.

  - 01 x Z80
  - 03 x 27c010
  - 02 x am27s19
  - 03 x GAL 16v8b      (All of them have the same contents... Maybe read protected.)
  - 01 x PAL CE 20v8h   (The fuse map is suspect too)
  - 01 x lm324n

  To not overload the driver, I put the rest of technical info in
  http://www.mameworld.net/robbie/trucocl.htm

- Added 2 "hidden" color proms (am27s19)
- One GAL is connected to the color proms inputs.
- The name of the company is "Miky SRL" instead of "Caloi Miky SRL".
  Caloi (Carlos Loiseau), is the Clemente's creator.

***************************************************************************/

#include "driver.h"

static tilemap *bg_tilemap;

PALETTE_INIT( trucocl )
{
	int i;

	for (i = 0;i < 32;i++)
		palette_set_color_rgb(machine,i,pal4bit(color_prom[i] >> 0),pal4bit(color_prom[i+32] >> 0),pal4bit(color_prom[i+32] >> 4));
}

WRITE8_HANDLER( trucocl_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( trucocl_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int gfxsel = colorram[tile_index] & 1;
	int bank = ( ( colorram[tile_index] >> 2 ) & 0x07 );
	int code = videoram[tile_index];
	int colour = (colorram[tile_index] & 2) >> 1;

	code |= ( bank & 1 ) << 10;
	code |= ( bank & 2 ) << 8;
	code += ( bank & 4 ) << 6;

	SET_TILE_INFO(gfxsel,code,colour,0);
}

VIDEO_START( trucocl )
{
	bg_tilemap = tilemap_create( get_bg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32 );
}

VIDEO_UPDATE( trucocl )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}
