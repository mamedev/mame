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
	airbustr_state *state = space->machine().driver_data<airbustr_state>();
	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

WRITE8_HANDLER( airbustr_colorram_w )
{
	airbustr_state *state = space->machine().driver_data<airbustr_state>();
	state->m_colorram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

WRITE8_HANDLER( airbustr_videoram2_w )
{
	airbustr_state *state = space->machine().driver_data<airbustr_state>();
	state->m_videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap, offset);
}

WRITE8_HANDLER( airbustr_colorram2_w )
{
	airbustr_state *state = space->machine().driver_data<airbustr_state>();
	state->m_colorram2[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap, offset);
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
	airbustr_state *state = space->machine().driver_data<airbustr_state>();
	switch (offset)		// offset 0 <-> port 4
	{
		case 0x00:	state->m_fg_scrolly = data;	break;	// low 8 bits
		case 0x02:	state->m_fg_scrollx = data;	break;
		case 0x04:	state->m_bg_scrolly = data;	break;
		case 0x06:	state->m_bg_scrollx = data;	break;
		case 0x08:	state->m_highbits   = ~data;	break;	// complemented high bits

		default:	logerror("CPU #2 - port %02X written with %02X - PC = %04X\n", offset, data, cpu_get_pc(&space->device()));
	}

	tilemap_set_scrolly(state->m_bg_tilemap, 0, ((state->m_highbits << 5) & 0x100) + state->m_bg_scrolly);
	tilemap_set_scrollx(state->m_bg_tilemap, 0, ((state->m_highbits << 6) & 0x100) + state->m_bg_scrollx);
	tilemap_set_scrolly(state->m_fg_tilemap, 0, ((state->m_highbits << 7) & 0x100) + state->m_fg_scrolly);
	tilemap_set_scrollx(state->m_fg_tilemap, 0, ((state->m_highbits << 8) & 0x100) + state->m_fg_scrollx);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	airbustr_state *state = machine.driver_data<airbustr_state>();
	int attr = state->m_colorram2[tile_index];
	int code = state->m_videoram2[tile_index] + ((attr & 0x0f) << 8);
	int color = attr >> 4;

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	airbustr_state *state = machine.driver_data<airbustr_state>();
	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index] + ((attr & 0x0f) << 8);
	int color = (attr >> 4) + 16;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( airbustr )
{
	airbustr_state *state = machine.driver_data<airbustr_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	state->m_sprites_bitmap = machine.primary_screen->alloc_compatible_bitmap();
	tilemap_set_transparent_pen(state->m_fg_tilemap, 0);

	tilemap_set_scrolldx(state->m_bg_tilemap, 0x094, 0x06a);
	tilemap_set_scrolldy(state->m_bg_tilemap, 0x100, 0x1ff);
	tilemap_set_scrolldx(state->m_fg_tilemap, 0x094, 0x06a);
	tilemap_set_scrolldy(state->m_fg_tilemap, 0x100, 0x1ff);

	state->save_item(NAME(*state->m_sprites_bitmap));
}


SCREEN_UPDATE( airbustr )
{
	airbustr_state *state = screen->machine().driver_data<airbustr_state>();

	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);

	// copy the sprite bitmap to the screen
	pandora_update(state->m_pandora, bitmap, cliprect);

	return 0;
}

SCREEN_EOF( airbustr )
{
	airbustr_state *state = machine.driver_data<airbustr_state>();

	// update the sprite bitmap
	pandora_eof(state->m_pandora);
}

