/**************************************************************************

                                Air Buster
                            (C) 1990  Kaneko

                    driver by Luca Elia (l.elia@tin.it)

[Screen]
    Size:               256 x 256
    Colors:             256 x 3
    Color Space:        32R x 32G x 32B

[Scrolling layers]
    Number:             2
    Size:               512 x 512
    Scrolling:          X,Y
    Tiles Size:         16 x 16
    Tiles Number:       0x1000
    Colors:             256 x 2 (0-511)
    Format:
                Offset:     0x400    0x000
                Bit:        fedc---- --------   Color
                            ----ba98 76543210   Code

[Sprites]
    On Screen:          256 x 2
    In ROM:             0x2000
    Colors:             256     (512-767)
    Format:             See Below


**************************************************************************/
#include "driver.h"
#include "kan_pand.h"

UINT8 *airbustr_videoram2, *airbustr_colorram2;
//int airbustr_clear_sprites;
static tilemap *bg_tilemap, *fg_tilemap;
static bitmap_t *sprites_bitmap;

WRITE8_HANDLER( airbustr_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( airbustr_colorram_w )
{
	space->machine->generic.colorram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( airbustr_videoram2_w )
{
	airbustr_videoram2[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( airbustr_colorram2_w )
{
	airbustr_colorram2[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

/*  Scroll Registers

    Port:
    4       Bg Y scroll, low 8 bits
    6       Bg X scroll, low 8 bits
    8       Fg Y scroll, low 8 bits
    A       Fg X scroll, low 8 bits

    C       3       2       1       0       <-Bit
            Bg Y    Bg X    Fg Y    Fg X    <-Scroll High Bits (complemented!)
*/

WRITE8_HANDLER( airbustr_scrollregs_w )
{
	static int bg_scrollx, bg_scrolly, fg_scrollx, fg_scrolly, highbits;

	switch (offset)		// offset 0 <-> port 4
	{
		case 0x00:	fg_scrolly =  data;	break;	// low 8 bits
		case 0x02:	fg_scrollx =  data;	break;
		case 0x04:	bg_scrolly =  data;	break;
		case 0x06:	bg_scrollx =  data;	break;
		case 0x08:	highbits   = ~data;	break;	// complemented high bits

		default:	logerror("CPU #2 - port %02X written with %02X - PC = %04X\n", offset, data, cpu_get_pc(space->cpu));
	}

	tilemap_set_scrolly(bg_tilemap, 0, ((highbits << 5) & 0x100) + bg_scrolly);
	tilemap_set_scrollx(bg_tilemap, 0, ((highbits << 6) & 0x100) + bg_scrollx);
	tilemap_set_scrolly(fg_tilemap, 0, ((highbits << 7) & 0x100) + fg_scrolly);
	tilemap_set_scrollx(fg_tilemap, 0, ((highbits << 8) & 0x100) + fg_scrollx);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int attr = airbustr_colorram2[tile_index];
	int code = airbustr_videoram2[tile_index] + ((attr & 0x0f) << 8);
	int color = attr >> 4;

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = machine->generic.colorram.u8[tile_index];
	int code = machine->generic.videoram.u8[tile_index] + ((attr & 0x0f) << 8);
	int color = (attr >> 4) + 16;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( airbustr )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	sprites_bitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);
	state_save_register_global_bitmap(machine, sprites_bitmap);
	tilemap_set_transparent_pen(fg_tilemap, 0);

	tilemap_set_scrolldx(bg_tilemap, 0x094, 0x06a);
	tilemap_set_scrolldy(bg_tilemap, 0x100, 0x1ff);
	tilemap_set_scrolldx(fg_tilemap, 0x094, 0x06a);
	tilemap_set_scrolldy(fg_tilemap, 0x100, 0x1ff);
}


VIDEO_UPDATE( airbustr )
{
	const device_config *pandora = devtag_get_device(screen->machine, "pandora");
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	// copy the sprite bitmap to the screen
	pandora_update(pandora, bitmap, cliprect);

	return 0;
}

VIDEO_EOF( airbustr )
{
	const device_config *pandora = devtag_get_device(machine, "pandora");
	// update the sprite bitmap
	pandora_eof(pandora);
}

