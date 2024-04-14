// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Standard Microsystems CRT9028/9128 Video Terminal Logic Controller

    The CRT 9028 and CRT 9128 are single-chip terminal-oriented video
    processors. The two differ from each other in their bus protocol
    for accessing the address, data and status registers: the 9028 has
    separate RD and WR strobes for an 8051 or similar microcontroller,
    while the 9128 replaces these with DS and R/W inputs that are more
    Z8-oriented. A separate address/data bus connects to a 2Kx8 static
    RAM or similar as display memory.

    The internal 128-character set is mask-programmed, as are all
    screen timings, with alternate vertical timings to allow operation
    at either 60 Hz or 50 Hz. Mask parameters also determine the
    polarity of the sync signals, the positioning of the underline
    attribute and cursor and the dot patterns used for 6-segment wide
    (block) graphics and 4-segment thin (line) graphics.

    TODO: implement timing and display functions

**********************************************************************/

#include "emu.h"
#include "crt9028.h"

#include "screen.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(CRT9028_000, crt9028_000_device, "crt9028_000", "CRT9028-000 VTLC")

//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  crt9028_device - constructor
//-------------------------------------------------

crt9028_device::crt9028_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock,
							int dots_per_char, int chars_per_row, int horiz_blanking, int hsync_delay, int hsync_width, bool hsync_active,
							int char_rows, int scans_per_char, bool vsync_active,
							int vert_blanking, int vsync_delay, int vsync_width,
							int alt_vert_blanking, int alt_vsync_delay, int alt_vsync_width,
							int csync_delay, int csync_width, int underline,
							u16 wide_gfx_seg1_4, u16 wide_gfx_seg2_5, u16 wide_gfx_seg3_6, u8 wide_gfx_pattern,
							u16 thin_gfx_seg1, u16 thin_gfx_seg2, u16 thin_gfx_seg3, u16 thin_gfx_seg4,
							u8 thin_gfx_dots1, u8 thin_gfx_dots2, u8 thin_gfx_dots3, u8 thin_gfx_dots4)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_space_config("charram", ENDIANNESS_LITTLE, 8, 11, 0)
	, m_charset(*this, "charset")
	, m_hsync_callback(*this)
	, m_vsync_callback(*this)
	, m_dots_per_char(dots_per_char)
	, m_chars_per_row(chars_per_row)
	, m_horiz_blanking(horiz_blanking)
	, m_hsync_delay(hsync_delay)
	, m_hsync_width(hsync_width)
	, m_hsync_active(hsync_active)
	, m_char_rows(char_rows)
	, m_scans_per_char(scans_per_char)
	, m_vsync_active(vsync_active)
	, m_vert_blanking{vert_blanking, alt_vert_blanking}
	, m_vsync_delay{vsync_delay, alt_vsync_delay}
	, m_vsync_width{vsync_width, alt_vsync_width}
	, m_csync_delay(csync_delay)
	, m_csync_width(csync_width)
	, m_underline(underline)
	, m_wide_gfx_seg{wide_gfx_seg1_4, wide_gfx_seg2_5, wide_gfx_seg3_6}
	, m_wide_gfx_pattern(wide_gfx_pattern)
	, m_thin_gfx_seg{thin_gfx_seg1, thin_gfx_seg2, thin_gfx_seg3, thin_gfx_seg4}
	, m_thin_gfx_dots{thin_gfx_dots1, thin_gfx_dots2, thin_gfx_dots3, thin_gfx_dots4}
	, m_address_register(0)
{
	// Mostly unused now
	(void)m_hsync_delay;
	(void)m_hsync_width;
	(void)m_hsync_active;
	(void)m_vsync_active;
	(void)m_vsync_delay;
	(void)m_vsync_width;
	(void)m_csync_delay;
	(void)m_csync_width;
	(void)m_underline;
	(void)m_wide_gfx_seg;
	(void)m_wide_gfx_pattern;
	(void)m_thin_gfx_seg;
	(void)m_thin_gfx_dots;
}


//-------------------------------------------------
//  crt9028_000_device - constructor
//-------------------------------------------------

crt9028_000_device::crt9028_000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: crt9028_device(mconfig, CRT9028_000, tag, owner, clock,
						7, 80, 20, 4, 8, false,
						24, 10, false,
						20, 4, 8,
						72, 30, 10,
						2, 8, 9,
						0x3c0, 0x038, 0x007, 0x0f,
						0x3e0, 0x020, 0x03f, 0x020,
						0x10, 0xff, 0x10, 0xff)
{
}


//-------------------------------------------------
//  device_config_complete - finalise device
//  configuration
//-------------------------------------------------

void crt9028_device::device_config_complete()
{
	if (!has_screen())
		return;

	if (screen().refresh_attoseconds() == 0)
	{
		int visible_scan_lines = m_char_rows * m_scans_per_char;
		screen().set_raw(clock(), m_dots_per_char * (m_chars_per_row + m_horiz_blanking), 0, m_dots_per_char * m_chars_per_row,
			visible_scan_lines + m_vert_blanking[0], 0, visible_scan_lines);
	}

	if (!screen().has_screen_update())
		screen().set_screen_update(*this, FUNC(crt9028_device::screen_update));
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  for the address spaces
//-------------------------------------------------

device_memory_interface::space_config_vector crt9028_device::memory_space_config() const
{
	return space_config_vector{std::make_pair(0, &m_space_config)};
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void crt9028_device::device_start()
{
	m_space = &space(0);

	save_item(NAME(m_address_register));
}


//-------------------------------------------------
//  screen_update - screen update method
//-------------------------------------------------

u32 crt9028_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


//-------------------------------------------------
//  chip_reset - software reset command
//-------------------------------------------------

void crt9028_device::chip_reset()
{
	logerror("%s: Chip reset\n", machine().describe_context());
}


//-------------------------------------------------
//  status_r - read status register
//-------------------------------------------------

u8 crt9028_device::status_r()
{
	return 0x80;
}


//-------------------------------------------------
//  filadd_w - set fill address register
//-------------------------------------------------

void crt9028_device::filadd_w(u8 data)
{
	logerror("%s: Fill address = %03X\n", machine().describe_context(), data << 4);
}


//-------------------------------------------------
//  tosadd_w - set top of screen address register
//-------------------------------------------------

void crt9028_device::tosadd_w(u8 data)
{
	logerror("%s: Top of screen address = %03X\n", machine().describe_context(), data << 4);
}


//-------------------------------------------------
//  curlo_w - set cursor low register
//-------------------------------------------------

void crt9028_device::curlo_w(u8 data)
{
	logerror("%s: Cursor low = %02X\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  curhi_w - set cursor high/scroll register
//-------------------------------------------------

void crt9028_device::curhi_w(u8 data)
{
	logerror("%s: Cursor high = %02X\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  attdat_w - set attribute data register
//-------------------------------------------------

void crt9028_device::attdat_w(u8 data)
{
	logerror("%s: Attribute data = %02X\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  mode_w - set mode register
//-------------------------------------------------

void crt9028_device::mode_w(u8 data)
{
	logerror("%s: Auto increment %sabled\n", machine().describe_context(), BIT(data, 7) ? "en" : "dis");
}


//-------------------------------------------------
//  character_r - read character from memory
//-------------------------------------------------

u8 crt9028_device::character_r()
{
	return 0;
}


//-------------------------------------------------
//  character_w - write character to memory
//-------------------------------------------------

void crt9028_device::character_w(u8 data)
{
	logerror("%s: Character = %02X\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  read - read from data or status register
//-------------------------------------------------

u8 crt9028_device::read(offs_t offset)
{
	if (BIT(offset, 0))
		return status_r();
	else if (m_address_register == 0xe)
		return character_r();
	else
	{
		if (!machine().side_effects_disabled())
			logerror("Read from unknown or write-only register %X\n", m_address_register);
		return 0;
	}
}


//-------------------------------------------------
//  write - write to data or address register
//-------------------------------------------------

void crt9028_device::write(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
		m_address_register = data & 0x0f;
	else switch (m_address_register)
	{
	case 0x6:
		chip_reset();
		break;

	case 0x8:
		tosadd_w(data);
		break;

	case 0x9:
		curlo_w(data);
		break;

	case 0xa:
		curhi_w(data);
		break;

	case 0xb:
		filadd_w(data);
		break;

	case 0xc:
		attdat_w(data);
		break;

	case 0xd:
		character_w(data);
		break;

	case 0xe:
		mode_w(data);
		break;

	default:
		logerror("%s: Unknown register %X = %02X\n", machine().describe_context(), m_address_register, data);
		break;
	}
}


//**************************************************************************
//  INTERNAL ROM
//**************************************************************************

ROM_START(crt9028_000)
	ROM_REGION(0x400, "charset", 0)
	ROM_LOAD("crt9028_000.bin", 0x000, 0x400, NO_DUMP)
ROM_END


//-------------------------------------------------
//  rom_region - return a pointer to the implicit
//  rom region description for this device
//-------------------------------------------------

const tiny_rom_entry *crt9028_000_device::device_rom_region() const
{
	return ROM_NAME(crt9028_000);
}
