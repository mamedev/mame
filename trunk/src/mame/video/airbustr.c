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

WRITE8_MEMBER(airbustr_state::airbustr_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(airbustr_state::airbustr_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(airbustr_state::airbustr_videoram2_w)
{
	m_videoram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(airbustr_state::airbustr_colorram2_w)
{
	m_colorram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
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

WRITE8_MEMBER(airbustr_state::airbustr_scrollregs_w)
{
	switch (offset)		// offset 0 <-> port 4
	{
		case 0x00:	m_fg_scrolly = data;	break;	// low 8 bits
		case 0x02:	m_fg_scrollx = data;	break;
		case 0x04:	m_bg_scrolly = data;	break;
		case 0x06:	m_bg_scrollx = data;	break;
		case 0x08:	m_highbits   = ~data;	break;	// complemented high bits

		default:	logerror("CPU #2 - port %02X written with %02X - PC = %04X\n", offset, data, cpu_get_pc(&space.device()));
	}

	m_bg_tilemap->set_scrolly(0, ((m_highbits << 5) & 0x100) + m_bg_scrolly);
	m_bg_tilemap->set_scrollx(0, ((m_highbits << 6) & 0x100) + m_bg_scrollx);
	m_fg_tilemap->set_scrolly(0, ((m_highbits << 7) & 0x100) + m_fg_scrolly);
	m_fg_tilemap->set_scrollx(0, ((m_highbits << 8) & 0x100) + m_fg_scrollx);
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

	machine.primary_screen->register_screen_bitmap(state->m_sprites_bitmap);
	state->m_fg_tilemap->set_transparent_pen(0);

	state->m_bg_tilemap->set_scrolldx(0x094, 0x06a);
	state->m_bg_tilemap->set_scrolldy(0x100, 0x1ff);
	state->m_fg_tilemap->set_scrolldx(0x094, 0x06a);
	state->m_fg_tilemap->set_scrolldy(0x100, 0x1ff);

	state->save_item(NAME(state->m_sprites_bitmap));
}


SCREEN_UPDATE_IND16( airbustr )
{
	airbustr_state *state = screen.machine().driver_data<airbustr_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

	// copy the sprite bitmap to the screen
	pandora_update(state->m_pandora, bitmap, cliprect);

	return 0;
}

SCREEN_VBLANK( airbustr )
{
	// rising edge
	if (vblank_on)
	{
		airbustr_state *state = screen.machine().driver_data<airbustr_state>();

		// update the sprite bitmap
		pandora_eof(state->m_pandora);
	}
}

