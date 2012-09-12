/*****************************************************************************
 *
 * video/abc806.c
 *
 ****************************************************************************/

/*

    TODO:

    - flashing
    - double height
    - underline

*/

#include "includes/abc80x.h"



// these are needed because the MC6845 emulation does
// not position the active display area correctly
#define HORIZONTAL_PORCH_HACK	109
#define VERTICAL_PORCH_HACK		27


static const rgb_t PALETTE[] =
{
	RGB_BLACK, // black
	MAKE_RGB(0xff, 0x00, 0x00), // red
	MAKE_RGB(0x00, 0xff, 0x00), // green
	MAKE_RGB(0xff, 0xff, 0x00), // yellow
	MAKE_RGB(0x00, 0x00, 0xff), // blue
	MAKE_RGB(0xff, 0x00, 0xff), // magenta
	MAKE_RGB(0x00, 0xff, 0xff), // cyan
	RGB_WHITE // white
};


//-------------------------------------------------
//  hrs_w - high resolution memory banking
//-------------------------------------------------

WRITE8_MEMBER( abc806_state::hrs_w )
{
	/*

        bit     signal  description

        0       VM14    visible screen memory area bit 0
        1       VM15    visible screen memory area bit 1
        2       VM16    visible screen memory area bit 2
        3       VM17    visible screen memory area bit 3
        4       F14     cpu accessible screen memory area bit 0
        5       F15     cpu accessible screen memory area bit 1
        6       F16     cpu accessible screen memory area bit 2
        7       F17     cpu accessible screen memory area bit 3

    */

	m_hrs = data;
}


//-------------------------------------------------
//  hrc_w - high resolution color write
//-------------------------------------------------

WRITE8_MEMBER( abc806_state::hrc_w )
{
	int reg = (offset >> 8) & 0x0f;

	m_hrc[reg] = data;
}


//-------------------------------------------------
//  charram_r - character RAM read
//-------------------------------------------------

READ8_MEMBER( abc806_state::charram_r )
{
	m_attr_data = m_color_ram[offset];

	return m_char_ram[offset];
}


//-------------------------------------------------
//  charram_w - character RAM write
//-------------------------------------------------

WRITE8_MEMBER( abc806_state::charram_w )
{
	m_color_ram[offset] = m_attr_data;

	m_char_ram[offset] = data;
}


//-------------------------------------------------
//  ami_r - attribute memory read
//-------------------------------------------------

READ8_MEMBER( abc806_state::ami_r )
{
	return m_attr_data;
}


//-------------------------------------------------
//  amo_w - attribute memory write
//-------------------------------------------------

WRITE8_MEMBER( abc806_state::amo_w )
{
	m_attr_data = data;
}


//-------------------------------------------------
//  cli_r - palette PROM read
//-------------------------------------------------

READ8_MEMBER( abc806_state::cli_r )
{
	/*

        bit     description

        0       HRU II data bit 0
        1       HRU II data bit 1
        2       HRU II data bit 2
        3       HRU II data bit 3
        4
        5
        6
        7       RTC data output

    */

	UINT16 hru2_addr = (m_hru2_a8 << 8) | (offset >> 8);
	UINT8 data = m_hru2_prom[hru2_addr] & 0x0f;

	logerror("HRU II %03x : %01x\n", hru2_addr, data);

	data |= m_rtc->dio_r() << 7;

	return data;
}


//-------------------------------------------------
//  sti_r - protection device read
//-------------------------------------------------

READ8_MEMBER( abc806_state::sti_r )
{
	/*

        bit     description

        0
        1
        2
        3
        4
        5
        6
        7       PROT DOUT

    */

	return 0x7f;
}


//-------------------------------------------------
//  sto_w -
//-------------------------------------------------

WRITE8_MEMBER( abc806_state::sto_w )
{
	int level = BIT(data, 7);

	switch (data & 0x07)
	{
	case 0:
		// external memory enable
		m_eme = level;
		break;
	case 1:
		// 40/80 column display
		m_40 = level;
		break;
	case 2:
		// HRU II address line 8, PROT A0
		m_hru2_a8 = level;
		break;
	case 3:
		// PROT INI
		break;
	case 4:
		// text display enable
		m_txoff = level;
		break;
	case 5:
		// RTC chip select
		m_rtc->cs_w(!level);
		break;
	case 6:
		// RTC clock
		m_rtc->clk_w(level);
		break;
	case 7:
		// RTC data in, PROT DIN
		m_rtc->dio_w(level);
		break;
	}
}


//-------------------------------------------------
//  sso_w - sync offset write
//-------------------------------------------------

WRITE8_MEMBER( abc806_state::sso_w )
{
	m_sync = data & 0x3f;
}


//-------------------------------------------------
//  MC6845_UPDATE_ROW( abc806_update_row )
//-------------------------------------------------

static MC6845_UPDATE_ROW( abc806_update_row )
{
	abc806_state *state = device->machine().driver_data<abc806_state>();

//  UINT8 old_data = 0xff;
	int fg_color = 7;
	int bg_color = 0;
	int underline = 0;
	int flash = 0;
	int e5 = state->m_40;
	int e6 = state->m_40;
	int th = 0;

	// prevent wraparound
	if (y >= 240) return;

	y += state->m_sync + VERTICAL_PORCH_HACK;

	for (int column = 0; column < x_count; column++)
	{
		UINT8 data = state->m_char_ram[(ma + column) & 0x7ff];
		UINT8 attr = state->m_color_ram[(ma + column) & 0x7ff];
		UINT16 rad_addr;
		UINT8 rad_data;

		if ((attr & 0x07) == ((attr >> 3) & 0x07))
		{
			// special case

			switch (attr >> 6)
			{
			case 0:
				// use previously selected attributes
				break;
			case 1:
				// reserved for future use
				break;
			case 2:
				// blank
				fg_color = 0;
				bg_color = 0;
				underline = 0;
				flash = 0;
				break;
			case 3:
				// double width
				e5 = BIT(attr, 0);
				e6 = BIT(attr, 1);

				// read attributes from next byte
				attr = state->m_color_ram[(ma + column + 1) & 0x7ff];

				if (attr != 0x00)
				{
					fg_color = attr & 0x07;
					bg_color = (attr >> 3) & 0x07;
					underline = BIT(attr, 6);
					flash = BIT(attr, 7);
				}
				break;
			}
		}
		else
		{
			// normal case
			fg_color = attr & 0x07;
			bg_color = (attr >> 3) & 0x07;
			underline = BIT(attr, 6);
			flash = BIT(attr, 7);
			e5 = state->m_40;
			e6 = state->m_40;
		}

		if (column == cursor_x)
		{
			rad_data = 0x0f;
		}
		else
		{
			rad_addr = (e6 << 8) | (e5 << 7) | (flash << 6) | (underline << 5) | (state->m_flshclk << 4) | ra;
			rad_data = state->m_rad_prom[rad_addr] & 0x0f;

			rad_data = ra; // HACK because the RAD prom is not dumped yet
		}

		UINT16 chargen_addr = (th << 12) | (data << 4) | rad_data;
		UINT8 chargen_data = state->m_char_rom[chargen_addr & 0xfff] << 2;
		int x = HORIZONTAL_PORCH_HACK + (column + 4) * ABC800_CHAR_WIDTH;

		for (int bit = 0; bit < ABC800_CHAR_WIDTH; bit++)
		{
			int color = BIT(chargen_data, 7) ? fg_color : bg_color;

			bitmap.pix32(y, x++) = PALETTE[color];

			if (e5 || e6)
			{
				bitmap.pix32(y, x++) = PALETTE[color];
			}

			chargen_data <<= 1;
		}

		if (e5 || e6)
		{
			column++;
		}

//      old_data = data;
	}
}


//-------------------------------------------------
//  hs_w - horizontal sync write
//-------------------------------------------------

WRITE_LINE_MEMBER( abc806_state::hs_w )
{
	int vsync;

	if (!state)
	{
		m_v50_addr++;

		// clock current vsync value into the shift register
		m_vsync_shift <<= 1;
		m_vsync_shift |= m_vsync;

		vsync = BIT(m_vsync_shift, m_sync);

		if (!m_d_vsync && vsync)
		{
			// clear V50 address
			m_v50_addr = 0;
		}
		else if (m_d_vsync && !vsync)
		{
			// flash clock
			if (m_flshclk_ctr & 0x20)
			{
				m_flshclk = !m_flshclk;
				m_flshclk_ctr = 0;
			}
			else
			{
				m_flshclk_ctr++;
			}
		}

		if (m_d_vsync != vsync)
		{
			// signal _DEW to DART
			m_dart->ri_w(1, !vsync);
		}

		m_d_vsync = vsync;
	}
}


//-------------------------------------------------
//  vs_w - vertical sync write
//-------------------------------------------------

WRITE_LINE_MEMBER( abc806_state::vs_w )
{
	m_vsync = state;
}


//-------------------------------------------------
//  mc6845_interface crtc_intf
//-------------------------------------------------

static const mc6845_interface crtc_intf =
{
	SCREEN_TAG,
	ABC800_CHAR_WIDTH,
	NULL,
	abc806_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(abc806_state, hs_w),
	DEVCB_DRIVER_LINE_MEMBER(abc806_state, vs_w),
	NULL
};


//-------------------------------------------------
//  hr_update - high resolution screen update
//-------------------------------------------------

void abc806_state::hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 addr = (m_hrs & 0x0f) << 15;

	for (int y = m_sync + VERTICAL_PORCH_HACK; y < MIN(cliprect.max_y + 1, m_sync + VERTICAL_PORCH_HACK + 240); y++)
	{
		for (int sx = 0; sx < 128; sx++)
		{
			UINT8 data = m_video_ram[addr++];
			UINT16 dot = (m_hrc[data >> 4] << 8) | m_hrc[data & 0x0f];

			for (int pixel = 0; pixel < 4; pixel++)
			{
				int x = HORIZONTAL_PORCH_HACK + (ABC800_CHAR_WIDTH * 4) - 16 + (sx * 4) + pixel;

				if (BIT(dot, 15) || (bitmap.pix32(y, x) == RGB_BLACK))
				{
					bitmap.pix32(y, x) = PALETTE[(dot >> 12) & 0x07];
				}

				dot <<= 4;
			}
		}
	}
}


//-------------------------------------------------
//  VIDEO_START( abc806 )
//-------------------------------------------------

void abc806_state::video_start()
{
	// initialize variables
	for (int i = 0; i < 16; i++)
	{
		m_hrc[i] = 0;
	}

	m_sync = 10;
	m_d_vsync = 1;
	m_vsync = 1;
	m_40 = 1;

	// find memory regions
	m_char_rom = memregion(MC6845_TAG)->base();
	m_rad_prom = memregion("rad")->base();
	m_hru2_prom = memregion("hru2")->base();

	// allocate memory
	m_char_ram.allocate(ABC806_CHAR_RAM_SIZE);
	m_color_ram = auto_alloc_array(machine(), UINT8, ABC806_ATTR_RAM_SIZE);

	// register for state saving
	save_pointer(NAME(m_char_ram.target()), ABC806_CHAR_RAM_SIZE);
	save_pointer(NAME(m_color_ram), ABC806_ATTR_RAM_SIZE);
	save_pointer(NAME(m_video_ram.target()), ABC806_VIDEO_RAM_SIZE);
	save_item(NAME(m_txoff));
	save_item(NAME(m_40));
	save_item(NAME(m_flshclk_ctr));
	save_item(NAME(m_flshclk));
	save_item(NAME(m_attr_data));
	save_item(NAME(m_hrs));
	save_item(NAME(m_hrc));
	save_item(NAME(m_sync));
	save_item(NAME(m_v50_addr));
	save_item(NAME(m_hru2_a8));
	save_item(NAME(m_vsync_shift));
	save_item(NAME(m_vsync));
	save_item(NAME(m_d_vsync));
}


//-------------------------------------------------
//  SCREEN_UPDATE( abc806 )
//-------------------------------------------------

UINT32 abc806_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// HACK expand visible area to workaround MC6845
	screen.set_visible_area(0, 767, 0, 311);

	// clear screen
	bitmap.fill(get_black_pen(machine()), cliprect);

	if (!m_txoff)
	{
		// draw text
		m_crtc->screen_update(screen, bitmap, cliprect);
	}

	// draw HR graphics
	hr_update(bitmap, cliprect);

	return 0;
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( abc806_video )
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( abc806_video )
	MCFG_MC6845_ADD(MC6845_TAG, MC6845, ABC800_CCLK, crtc_intf)

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(abc806_state, screen_update)

	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(768, 312)
	MCFG_SCREEN_VISIBLE_AREA(0, 768-1, 0, 312-1)
MACHINE_CONFIG_END
