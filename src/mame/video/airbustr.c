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
#include "emu.h"
#include "video/kan_pand.h"
#include "includes/airbustr.h"

WRITE8_HANDLER( airbustr_videoram_w )
{
	airbustr_state *state = (airbustr_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( airbustr_colorram_w )
{
	airbustr_state *state = (airbustr_state *)space->machine->driver_data;
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( airbustr_videoram2_w )
{
	airbustr_state *state = (airbustr_state *)space->machine->driver_data;
	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( airbustr_colorram2_w )
{
	airbustr_state *state = (airbustr_state *)space->machine->driver_data;
	state->colorram2[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
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
	airbustr_state *state = (airbustr_state *)space->machine->driver_data;
	switch (offset)		// offset 0 <-> port 4
	{
		case 0x00:	state->fg_scrolly = data;	break;	// low 8 bits
		case 0x02:	state->fg_scrollx = data;	break;
		case 0x04:	state->bg_scrolly = data;	break;
		case 0x06:	state->bg_scrollx = data;	break;
		case 0x08:	state->highbits   = ~data;	break;	// complemented high bits

		default:	logerror("CPU #2 - port %02X written with %02X - PC = %04X\n", offset, data, cpu_get_pc(space->cpu));
	}

	tilemap_set_scrolly(state->bg_tilemap, 0, ((state->highbits << 5) & 0x100) + state->bg_scrolly);
	tilemap_set_scrollx(state->bg_tilemap, 0, ((state->highbits << 6) & 0x100) + state->bg_scrollx);
	tilemap_set_scrolly(state->fg_tilemap, 0, ((state->highbits << 7) & 0x100) + state->fg_scrolly);
	tilemap_set_scrollx(state->fg_tilemap, 0, ((state->highbits << 8) & 0x100) + state->fg_scrollx);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	airbustr_state *state = (airbustr_state *)machine->driver_data;
	int attr = state->colorram2[tile_index];
	int code = state->videoram2[tile_index] + ((attr & 0x0f) << 8);
	int color = attr >> 4;

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	airbustr_state *state = (airbustr_state *)machine->driver_data;
	int attr = state->colorram[tile_index];
	int code = state->videoram[tile_index] + ((attr & 0x0f) << 8);
	int color = (attr >> 4) + 16;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( airbustr )
{
	airbustr_state *state = (airbustr_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	state->sprites_bitmap = machine->primary_screen->alloc_compatible_bitmap();
	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	tilemap_set_scrolldx(state->bg_tilemap, 0x094, 0x06a);
	tilemap_set_scrolldy(state->bg_tilemap, 0x100, 0x1ff);
	tilemap_set_scrolldx(state->fg_tilemap, 0x094, 0x06a);
	tilemap_set_scrolldy(state->fg_tilemap, 0x100, 0x1ff);

	state_save_register_global_bitmap(machine, state->sprites_bitmap);
}


VIDEO_UPDATE( airbustr )
{
	airbustr_state *state = (airbustr_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);

	// copy the sprite bitmap to the screen
	pandora_update(state->pandora, bitmap, cliprect);

	return 0;
}

VIDEO_EOF( airbustr )
{
	airbustr_state *state = (airbustr_state *)machine->driver_data;

	// update the sprite bitmap
	pandora_eof(state->pandora);
}

