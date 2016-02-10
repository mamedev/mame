// license:BSD-3-Clause
// copyright-holders:Luca Elia
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
	switch (offset)     // offset 0 <-> port 4
	{
		case 0x00:  m_fg_scrolly = data;    break;  // low 8 bits
		case 0x02:  m_fg_scrollx = data;    break;
		case 0x04:  m_bg_scrolly = data;    break;
		case 0x06:  m_bg_scrollx = data;    break;
		case 0x08:  m_highbits   = ~data;   break;  // complemented high bits

		default:    logerror("CPU #2 - port %02X written with %02X - PC = %04X\n", offset, data, space.device().safe_pc());
	}

	m_bg_tilemap->set_scrolly(0, ((m_highbits << 5) & 0x100) + m_bg_scrolly);
	m_bg_tilemap->set_scrollx(0, ((m_highbits << 6) & 0x100) + m_bg_scrollx);
	m_fg_tilemap->set_scrolly(0, ((m_highbits << 7) & 0x100) + m_fg_scrolly);
	m_fg_tilemap->set_scrollx(0, ((m_highbits << 8) & 0x100) + m_fg_scrollx);
}

TILE_GET_INFO_MEMBER(airbustr_state::get_fg_tile_info)
{
	int attr = m_colorram2[tile_index];
	int code = m_videoram2[tile_index] + ((attr & 0x0f) << 8);
	int color = attr >> 4;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(airbustr_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0x0f) << 8);
	int color = (attr >> 4) + 16;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void airbustr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(airbustr_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(airbustr_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_screen->register_screen_bitmap(m_sprites_bitmap);
	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrolldx(0x094, 0x06a);
	m_bg_tilemap->set_scrolldy(0x100, 0x1ff);
	m_fg_tilemap->set_scrolldx(0x094, 0x06a);
	m_fg_tilemap->set_scrolldy(0x100, 0x1ff);

	save_item(NAME(m_sprites_bitmap));
}


UINT32 airbustr_state::screen_update_airbustr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// copy the sprite bitmap to the screen
	m_pandora->update(bitmap, cliprect);

	return 0;
}

void airbustr_state::screen_eof_airbustr(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		// update the sprite bitmap
		m_pandora->eof();
	}
}
