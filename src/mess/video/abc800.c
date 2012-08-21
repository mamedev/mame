/*****************************************************************************
 *
 * video/abc800.c
 *
 ****************************************************************************/

#include "includes/abc80x.h"



// these are needed because the MC6845 emulation does
// not position the active display area correctly
#define HORIZONTAL_PORCH_HACK	115
#define VERTICAL_PORCH_HACK		29



//**************************************************************************
//  HIGH RESOLUTION GRAPHICS
//**************************************************************************

//-------------------------------------------------
//  hrs_w - high resolution scanline write
//-------------------------------------------------

WRITE8_MEMBER( abc800_state::hrs_w )
{
	m_hrs = data;
}


//-------------------------------------------------
//  hrc_w - high resolution color write
//-------------------------------------------------

WRITE8_MEMBER( abc800_state::hrc_w )
{
	m_fgctl = data;
}



//**************************************************************************
//  ABC 800 COLOR
//**************************************************************************

//-------------------------------------------------
//  translate_trom_offset -
//-------------------------------------------------

offs_t abc800c_state::translate_trom_offset(offs_t offset)
{
	int row = offset >> 7;
	int col = offset & 0x7f;

	if (col >= 80) row += 16;
	else if (col >= 40) row += 8;

	return (row * 40) + (col % 40);
}


//-------------------------------------------------
//  char_ram_r - character RAM read
//-------------------------------------------------

READ8_MEMBER( abc800c_state::char_ram_r )
{
	return saa5050_videoram_r(m_trom, translate_trom_offset(offset));
}


//-------------------------------------------------
//  char_ram_w - character RAM write
//-------------------------------------------------

WRITE8_MEMBER( abc800c_state::char_ram_w )
{
	saa5050_videoram_w(m_trom, translate_trom_offset(offset), data);
}


//-------------------------------------------------
//  hr_update - high resolution screen update
//-------------------------------------------------

void abc800c_state::hr_update(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 addr = 0;

	for (int y = m_hrs; y < MIN(cliprect.max_y + 1, m_hrs + 480); y += 2)
	{
		int x = 0;

		for (int sx = 0; sx < 64; sx++)
		{
			UINT8 data = m_video_ram[addr++];

			for (int dot = 0; dot < 4; dot++)
			{
				UINT16 fgctl_addr = ((m_fgctl & 0x7f) << 2) | ((data >> 6) & 0x03);
				UINT8 fgctl = m_fgctl_prom[fgctl_addr];
				int color = fgctl & 0x07;

				if (color)
				{
					rgb_t rgb = palette_entry_get_color(machine().palette, bitmap.pix16(y, x));
					bool black = !RGB_RED(rgb) && !RGB_GREEN(rgb) && !RGB_BLUE(rgb);
					bool opaque = !BIT(fgctl, 3);

					if (black || opaque)
					{
						color += 128;

						bitmap.pix16(y, x) = color;
						bitmap.pix16(y, x + 1) = color;

						bitmap.pix16(y + 1, x) = color;
						bitmap.pix16(y + 1, x + 1) = color;
					}
				}

				data <<= 2;
				x += 2;
			}
		}
	}
}


//-------------------------------------------------
//  VIDEO_START( abc800 )
//-------------------------------------------------

void abc800_state::video_start()
{
	// find memory regions
	m_char_rom = memregion(MC6845_TAG)->base();
	m_fgctl_prom = memregion("hru2")->base();

	// register for state saving
	save_item(NAME(m_hrs));
	save_item(NAME(m_fgctl));
}


//-------------------------------------------------
//  VIDEO_START( abc800c )
//-------------------------------------------------

void abc800c_state::video_start()
{
	abc800_state::video_start();

	// initialize palette
	palette_set_color_rgb(machine(), 128+0, 0x00, 0x00, 0x00); // black
	palette_set_color_rgb(machine(), 128+1, 0xff, 0x00, 0x00); // red
	palette_set_color_rgb(machine(), 128+2, 0x00, 0xff, 0x00); // green
	palette_set_color_rgb(machine(), 128+3, 0xff, 0xff, 0x00); // yellow
	palette_set_color_rgb(machine(), 128+4, 0x00, 0x00, 0xff); // blue
	palette_set_color_rgb(machine(), 128+5, 0xff, 0x00, 0xff); // magenta
	palette_set_color_rgb(machine(), 128+6, 0x00, 0xff, 0xff); // cyan
	palette_set_color_rgb(machine(), 128+7, 0xff, 0xff, 0xff); // white
}


//-------------------------------------------------
//  SCREEN_UPDATE( abc800c )
//-------------------------------------------------

UINT32 abc800c_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// clear screen
	bitmap.fill(get_black_pen(machine()), cliprect);

	// draw text
	if (!BIT(m_fgctl, 7))
	{
		saa5050_update(m_trom, bitmap, cliprect);
	}

	saa5050_frame_advance(m_trom);

	// draw HR graphics
	hr_update(bitmap, cliprect);

	return 0;
}


//-------------------------------------------------
//  saa5050_interface trom_intf
//-------------------------------------------------

static const saa5050_interface trom_intf =
{
	SCREEN_TAG,
	0,	// starting gfxnum
	40, 24, 40,  // x, y, size
	0	// rev y order
};


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( abc800c_video )
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( abc800c_video )
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(abc800c_state, screen_update)

	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(480, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 480-1, 0, 480-1)

	MCFG_PALETTE_LENGTH(128+8)
	MCFG_PALETTE_INIT(saa5050)

	MCFG_GFXDECODE(saa5050)

	MCFG_SAA5050_ADD(SAA5052_TAG, trom_intf)
MACHINE_CONFIG_END



//**************************************************************************
//  ABC 800 MONOCHROME
//**************************************************************************

//-------------------------------------------------
//  PALETTE_INIT( abc800m )
//-------------------------------------------------

static PALETTE_INIT( abc800m )
{
	palette_set_color_rgb(machine, 0, 0x00, 0x00, 0x00); // black
	palette_set_color_rgb(machine, 1, 0xff, 0xff, 0x00); // yellow
}


//-------------------------------------------------
//  hr_update - high resolution screen update
//-------------------------------------------------

void abc800m_state::hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT16 addr = 0;

	for (int y = m_hrs + VERTICAL_PORCH_HACK; y < MIN(cliprect.max_y + 1, m_hrs + VERTICAL_PORCH_HACK + 240); y++)
	{
		int x = HORIZONTAL_PORCH_HACK;

		for (int sx = 0; sx < 64; sx++)
		{
			UINT8 data = m_video_ram[addr++];

			for (int dot = 0; dot < 4; dot++)
			{
				UINT16 fgctl_addr = ((m_fgctl & 0x7f) << 2) | ((data >> 6) & 0x03);
				int color = (m_fgctl_prom[fgctl_addr] & 0x07) ? 1 : 0;

				bitmap.pix32(y, x++) = palette[color];
				bitmap.pix32(y, x++) = palette[color];

				data <<= 2;
			}
		}
	}
}


//-------------------------------------------------
//  MC6845_UPDATE_ROW( abc800m_update_row )
//-------------------------------------------------

static MC6845_UPDATE_ROW( abc800m_update_row )
{
	abc800m_state *state = device->machine().driver_data<abc800m_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());

	int column;

	// prevent wraparound
	if (y >= 240) return;

	y += VERTICAL_PORCH_HACK;

	for (column = 0; column < x_count; column++)
	{
		int bit;

		UINT16 address = (state->m_char_ram[(ma + column) & 0x7ff] << 4) | (ra & 0x0f);
		UINT8 data = (state->m_char_rom[address & 0x7ff] & 0x3f);

		if (column == cursor_x)
		{
			data = 0x3f;
		}

		data <<= 2;

		for (bit = 0; bit < ABC800_CHAR_WIDTH; bit++)
		{
			int x = HORIZONTAL_PORCH_HACK + (column * ABC800_CHAR_WIDTH) + bit;

			if (BIT(data, 7))
			{
				bitmap.pix32(y, x) = palette[1];
			}

			data <<= 1;
		}
	}
}


//-------------------------------------------------
//  mc6845_interface crtc_intf
//-------------------------------------------------

static const mc6845_interface crtc_intf =
{
	SCREEN_TAG,
	ABC800_CHAR_WIDTH,
	NULL,
	abc800m_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE(Z80DART_TAG, z80dart_rib_w),
	NULL
};


//-------------------------------------------------
//  SCREEN_UPDATE( abc800m )
//-------------------------------------------------

UINT32 abc800m_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// HACK expand visible area to workaround MC6845
	screen.set_visible_area(0, 767, 0, 311);

	// clear screen
	bitmap.fill(get_black_pen(machine()), cliprect);

	// draw HR graphics
	hr_update(bitmap, cliprect);

	// draw text
	if (!BIT(m_fgctl, 7))
	{
		m_crtc->screen_update(screen, bitmap, cliprect);
	}

	return 0;
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( abc800m_video )
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( abc800m_video )
	MCFG_MC6845_ADD(MC6845_TAG, MC6845, ABC800_CCLK, crtc_intf)

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(abc800m_state, screen_update)

	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(768, 312)
	MCFG_SCREEN_VISIBLE_AREA(0,768-1, 0, 312-1)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(abc800m)
MACHINE_CONFIG_END
