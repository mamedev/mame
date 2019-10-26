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
#include "includes/airbustr.h"

/*  Scroll Registers

    Port:
    4       Bg Y scroll, low 8 bits
    6       Bg X scroll, low 8 bits
    8       Fg Y scroll, low 8 bits
    A       Fg X scroll, low 8 bits

    C       3       2       1       0       <-Bit
            Bg Y    Bg X    Fg Y    Fg X    <-Scroll High Bits (complemented!)
*/

WRITE8_MEMBER(airbustr_state::scrollregs_w)
{
	switch (offset)     // offset 0 <-> port 4
	{
		case 0x00:
		case 0x04:  m_scrolly[((offset & 4) >> 2) ^ 1] = data;    break;  // low 8 bits
		case 0x02:
		case 0x06:  m_scrollx[((offset & 4) >> 2) ^ 1] = data;    break;
		case 0x08:  m_highbits   = ~data;   break;  // complemented high bits

		default:    logerror("CPU #2 - port %02X written with %02X - PC = %04X\n", offset, data, m_slave->pc());
	}

	for (int layer = 0; layer < 2; layer++)
	{
		m_tilemap[layer]->set_scrolly(0, ((m_highbits << (5+(layer<<1))) & 0x100) + m_scrolly[layer]);
		m_tilemap[layer]->set_scrollx(0, ((m_highbits << (6+(layer<<1))) & 0x100) + m_scrollx[layer]);
	}
}

template<int Layer>
TILE_GET_INFO_MEMBER(airbustr_state::get_tile_info)
{
	int attr = m_colorram[Layer][tile_index];
	int code = m_videoram[Layer][tile_index] + ((attr & 0x0f) << 8);
	int color = (attr >> 4) + ((Layer ^ 1) << 4);

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void airbustr_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(airbustr_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(airbustr_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tilemap[1]->set_transparent_pen(0);

	for (int layer = 0; layer < 2; layer++)
	{
		m_tilemap[layer]->set_scrolldx(0x094, 0x06a);
		m_tilemap[layer]->set_scrolldy(0x100, 0x1ff);
	}
}


uint32_t airbustr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	// copy the sprite bitmap to the screen
	m_pandora->update(bitmap, cliprect);

	return 0;
}

WRITE_LINE_MEMBER(airbustr_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		// update the sprite bitmap
		m_pandora->eof();
	}
}
