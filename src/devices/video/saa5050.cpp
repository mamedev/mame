// license:BSD-3-Clause
// copyright-holders:Curt Coder, Nigel Barnes
/**********************************************************************

    Mullard SAA5050 Teletext Character Generator emulation

    http://www.bighole.nl/pub/mirror/homepage.ntlworld.com/kryten_droid/teletext/spec/teletext_spec_1974.htm

**********************************************************************/

/*

    TODO:

    - interlace, use CRS to output odd/even fields
    - remote controller input
    - boxing

*/

#include "emu.h"
#include "saa5050.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAA5050, saa5050_device, "saa5050", "SAA5050 Teletext Character Generator")
DEFINE_DEVICE_TYPE(SAA5051, saa5051_device, "saa5051", "SAA5051 Teletext Character Generator")
DEFINE_DEVICE_TYPE(SAA5052, saa5052_device, "saa5052", "SAA5052 Teletext Character Generator")
DEFINE_DEVICE_TYPE(SAA5053, saa5053_device, "saa5053", "SAA5053 Teletext Character Generator")
DEFINE_DEVICE_TYPE(SAA5054, saa5054_device, "saa5054", "SAA5054 Teletext Character Generator")
DEFINE_DEVICE_TYPE(SAA5055, saa5055_device, "saa5055", "SAA5055 Teletext Character Generator")
DEFINE_DEVICE_TYPE(SAA5056, saa5056_device, "saa5056", "SAA5056 Teletext Character Generator")
DEFINE_DEVICE_TYPE(SAA5057, saa5057_device, "saa5057", "SAA5057 Teletext Character Generator")


//-------------------------------------------------
//  ROM( saa5050 )
//-------------------------------------------------

ROM_START( saa5050 )
	ROM_REGION( 0x500, "chargen", 0 )
	ROM_LOAD("saa5050", 0x0140, 0x03c0, CRC(201490f3) SHA1(6c8daba70374e5aa3a6402f24cdc5f8677d58a0f)) // verified both from datasheet listing and decap
ROM_END


//-------------------------------------------------
//  ROM( saa5051 )
//-------------------------------------------------

ROM_START( saa5051 )
	ROM_REGION( 0x500, "chargen", 0 )
	ROM_LOAD("saa5051", 0x0140, 0x03c0, BAD_DUMP CRC(0e55088b) SHA1(07cd9b7edbed6ef7b527622533d6957f5d56aa91)) // verified from datasheet listing
ROM_END


//-------------------------------------------------
//  ROM( saa5052 )
//-------------------------------------------------

ROM_START( saa5052 )
	ROM_REGION( 0x500, "chargen", 0 )
	ROM_LOAD("saa5052", 0x0140, 0x03c0, BAD_DUMP CRC(2eb76737) SHA1(ec4bc515e28e851a6433f7ca0a11ede0f1d21a68))
ROM_END


//-------------------------------------------------
//  ROM( saa5053 )
//-------------------------------------------------

ROM_START( saa5053 )
	ROM_REGION( 0x500, "chargen", 0 )
	ROM_LOAD("saa5053", 0x0140, 0x03c0, BAD_DUMP CRC(46288c33) SHA1(1e471a1b5670d7163e9f62d31be7cab0330a07cd))
ROM_END


//-------------------------------------------------
//  ROM( saa5054 )
//-------------------------------------------------

ROM_START( saa5054 )
	ROM_REGION( 0x500, "chargen", 0 )
	ROM_LOAD("saa5054", 0x0140, 0x03c0, BAD_DUMP CRC(56298472) SHA1(7a273ad7270507dca4ce621fc1e6b51a1ac25085))
ROM_END


//-------------------------------------------------
//  ROM( saa5055 )
//-------------------------------------------------

ROM_START( saa5055 )
	ROM_REGION( 0x500, "chargen", 0 )
	ROM_LOAD("saa5055", 0x0140, 0x03c0, BAD_DUMP CRC(f95b9c8c) SHA1(c5ce7fe84df6de6a317fa0e87bda413c82c04618))
ROM_END


//-------------------------------------------------
//  ROM( saa5056 )
//-------------------------------------------------

ROM_START( saa5056 )
	ROM_REGION( 0x500, "chargen", 0 )
	ROM_LOAD("saa5056", 0x0140, 0x03c0, BAD_DUMP CRC(86ab8b85) SHA1(2d1ff08b4dda15cf70832881750a962189455f41))
ROM_END


//-------------------------------------------------
//  ROM( saa5057 )
//-------------------------------------------------

ROM_START( saa5057 )
	ROM_REGION( 0x500, "chargen", 0 )
	ROM_LOAD("saa5057", 0x0140, 0x03c0, BAD_DUMP CRC(d6664fb3) SHA1(5a93445dde03066073e2909a935900e5f8439d81))
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *saa5050_device::device_rom_region() const
{
	return ROM_NAME( saa5050 );
}

const tiny_rom_entry *saa5051_device::device_rom_region() const
{
	return ROM_NAME( saa5051 );
}

const tiny_rom_entry *saa5052_device::device_rom_region() const
{
	return ROM_NAME( saa5052 );
}

const tiny_rom_entry *saa5053_device::device_rom_region() const
{
	return ROM_NAME( saa5053 );
}

const tiny_rom_entry *saa5054_device::device_rom_region() const
{
	return ROM_NAME( saa5054 );
}

const tiny_rom_entry *saa5055_device::device_rom_region() const
{
	return ROM_NAME( saa5055 );
}

const tiny_rom_entry *saa5056_device::device_rom_region() const
{
	return ROM_NAME( saa5056 );
}

const tiny_rom_entry *saa5057_device::device_rom_region() const
{
	return ROM_NAME( saa5057 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saa5050_device - constructor
//-------------------------------------------------

saa5050_device::saa5050_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_char_rom(*this, "chargen"),
	m_read_d(*this, 0),
	m_frame_count(0),
	m_cols(0),
	m_rows(0),
	m_size(0)
{
}

saa5050_device::saa5050_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	saa5050_device(mconfig, SAA5050, tag, owner, clock)
{
}

saa5051_device::saa5051_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saa5050_device(mconfig, SAA5051, tag, owner, clock)
{
}

saa5052_device::saa5052_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saa5050_device(mconfig, SAA5052, tag, owner, clock)
{
}

saa5053_device::saa5053_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saa5050_device(mconfig, SAA5053, tag, owner, clock)
{
}

saa5054_device::saa5054_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saa5050_device(mconfig, SAA5054, tag, owner, clock)
{
}

saa5055_device::saa5055_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saa5050_device(mconfig, SAA5055, tag, owner, clock)
{
}

saa5056_device::saa5056_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saa5050_device(mconfig, SAA5056, tag, owner, clock)
{
}

saa5057_device::saa5057_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saa5050_device(mconfig, SAA5057, tag, owner, clock)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saa5050_device::device_start()
{
	// register for state saving
	save_item(NAME(m_code));
	save_item(NAME(m_held_char));
	save_item(NAME(m_char_data));
	save_item(NAME(m_bit));
	save_item(NAME(m_color));
	save_item(NAME(m_ra));
	save_item(NAME(m_bg));
	save_item(NAME(m_fg));
	save_item(NAME(m_prev_col));
	save_item(NAME(m_graphics));
	save_item(NAME(m_separated));
	save_item(NAME(m_flash));
	save_item(NAME(m_boxed));
	save_item(NAME(m_dbl_height));
	save_item(NAME(m_dbl_height_bottom_row));
	save_item(NAME(m_dbl_height_prev_row));
	save_item(NAME(m_hold_char));
	save_item(NAME(m_frame_count));
}


//-------------------------------------------------
//  device_start - device-specific reset
//-------------------------------------------------

void saa5050_device::device_reset()
{
	m_ra = 0;
	m_dbl_height = false;
	m_dbl_height_bottom_row = false;
}


//-------------------------------------------------
//  process_control_character -
//-------------------------------------------------

void saa5050_device::process_control_character(uint8_t data)
{
	switch (data)
	{
		case ALPHA_RED:
		case ALPHA_GREEN:
		case ALPHA_YELLOW:
		case ALPHA_BLUE:
		case ALPHA_MAGENTA:
		case ALPHA_CYAN:
		case ALPHA_WHITE:
			m_graphics = false;
			m_fg = data & 0x07;
			set_next_chartype();
			break;

		case FLASH:
			m_flash = true;
			break;

		case STEADY:
			m_flash = false;
			break;

		case END_BOX:
		case START_BOX:
			// TODO
			break;

		case NORMAL_HEIGHT:
		case DOUBLE_HEIGHT:
			m_dbl_height = !!(data & 1);
			if (m_dbl_height) m_dbl_height_prev_row = true;
			break;

		case GRAPHICS_RED:
		case GRAPHICS_GREEN:
		case GRAPHICS_YELLOW:
		case GRAPHICS_BLUE:
		case GRAPHICS_MAGENTA:
		case GRAPHICS_CYAN:
		case GRAPHICS_WHITE:
			m_graphics = true;
			m_fg = data & 0x07;
			set_next_chartype();
			break;

		case CONCEAL_DISPLAY:
			m_fg = m_prev_col = m_bg;
			break;

		case CONTIGUOUS_GFX:
			m_separated = false;
			set_next_chartype();
			break;

		case SEPARATED_GFX:
			m_separated = true;
			set_next_chartype();
			break;

		case BLACK_BACKGROUND:
			m_bg = 0;
			break;

		case NEW_BACKGROUND:
			m_bg = m_fg;
			break;

		case HOLD_GRAPHICS:
			m_hold_char = true;
			break;

		case RELEASE_GRAPHICS:
			m_hold_char = false;
			break;
	}
}


void saa5050_device::set_next_chartype()
{
	if (m_graphics)
	{
		if (m_separated)
			m_next_chartype = SEPARATED;
		else
			m_next_chartype = CONTIGUOUS;
	}
	else
	{
		m_next_chartype = ALPHANUMERIC;
	}
};


//-------------------------------------------------
//  get_gfx_data - graphics generator
//-------------------------------------------------

uint16_t saa5050_device::get_gfx_data(uint8_t data, offs_t row, bool separated)
{
	uint16_t c = 0;
	switch (row >> 1)
	{
	case 0: case 1:
		c += (data & 0x01) ? 0xfc0 : 0; // bit 1 top left
		c += (data & 0x02) ? 0x03f : 0; // bit 2 top right
		if (separated) c &= 0x3cf;
		break;
	case 2:
		if (separated) break;
		c += (data & 0x01) ? 0xfc0 : 0; // bit 1 top left
		c += (data & 0x02) ? 0x03f : 0; // bit 2 top right
		break;
	case 3: case 4: case 5:
		c += (data & 0x04) ? 0xfc0 : 0; // bit 3 middle left
		c += (data & 0x08) ? 0x03f : 0; // bit 4 middle right
		if (separated) c &= 0x3cf;
		break;
	case 6:
		if (separated) break;
		c += (data & 0x04) ? 0xfc0 : 0; // bit 3 middle left
		c += (data & 0x08) ? 0x03f : 0; // bit 4 middle right
		break;
	case 7: case 8:
		c += (data & 0x10) ? 0xfc0 : 0; // bit 5 bottom left
		c += (data & 0x40) ? 0x03f : 0; // bit 7 bottom right
		if (separated) c &= 0x3cf;
		break;
	case 9:
		if (separated) break;
		c += (data & 0x10) ? 0xfc0 : 0; // bit 5 bottom left
		c += (data & 0x40) ? 0x03f : 0; // bit 7 bottom right
		break;
	}
	return c;
}


//-------------------------------------------------
//  get_rom_data - read rom
//-------------------------------------------------

uint16_t saa5050_device::get_rom_data(uint8_t data, offs_t row)
{
	uint16_t c;
	if (row < 0 || row >= 20)
	{
		c = 0;
	}
	else
	{
		c = m_char_rom[(data * 10) + (row >> 1)];
		c = ((c & 0x01) * 0x03) + ((c & 0x02) * 0x06) + ((c & 0x04) * 0x0c) + ((c & 0x08) * 0x18) + ((c & 0x10) * 0x30);
	}
	return c;
}


//-------------------------------------------------
//  character_rounding
//-------------------------------------------------

uint16_t saa5050_device::character_rounding(uint16_t a, uint16_t b)
{
	return a | ((a >> 1) & b & ~(b >> 1)) | ((a << 1) & b & ~(b << 1));
}


//-------------------------------------------------
//  get_character_data -
//-------------------------------------------------

void saa5050_device::get_character_data(uint8_t data)
{
	bool const dbl_height_prev = m_dbl_height;
	bool const flash_prev      = m_flash;
	bool const graphics_prev   = m_graphics;
	bool const hold_char_prev  = m_hold_char;

	m_prev_col = m_fg;
	m_curr_chartype = m_next_chartype;

	if (data < 0x20)
	{
		process_control_character(data);
		if (graphics_prev && (hold_char_prev || m_hold_char) && m_dbl_height == dbl_height_prev)
		{
			data = m_held_char;
			if (data >= 0x40 && data < 0x60) data = 0x20;
			m_curr_chartype = m_held_chartype;
		}
		else
		{
			m_held_char = 0x20;
			data = 0x20;
		}
	}
	else if (m_graphics)
	{
		if (data & 0x20)
		{
			m_held_char = data;
			m_held_chartype = m_curr_chartype;
		}
	}
	else
	{
		m_held_char = 0x20;
	}

	offs_t ra = m_ra;
	if (dbl_height_prev)
	{
		ra >>= 1;
		if (m_dbl_height_bottom_row) ra += 10;
	}

	if (flash_prev && !(m_frame_count & 0x30))
		data = 0x20;
	if (m_dbl_height_bottom_row && !m_dbl_height)
		data = 0x20;

	if (m_curr_chartype == ALPHANUMERIC || !BIT(data,5))
		m_char_data = character_rounding(get_rom_data(data, ra), get_rom_data(data, ra + ((ra & 1) ? 1 : -1)));
	else
		m_char_data = get_gfx_data(data, ra, (m_curr_chartype == SEPARATED));
}


//-------------------------------------------------
//  crs_w - character rounding select
//-------------------------------------------------

void saa5050_device::crs_w(int state)
{
	m_crs = !(state & 1);
}


//-------------------------------------------------
//  dew_w - data entry window
//-------------------------------------------------

void saa5050_device::dew_w(int state)
{
	if (state)
	{
		m_ra = 19;

		m_frame_count++;
		m_frame_count &= 0x3f;
	}
}


//-------------------------------------------------
//  lose_w - load output shift register enable
//-------------------------------------------------

void saa5050_device::lose_w(int state)
{
	if (state)
	{
		m_ra++;
		m_ra %= 20;

		if (!m_ra)
		{
			if (m_dbl_height_bottom_row)
				m_dbl_height_bottom_row = false;
			else
				m_dbl_height_bottom_row = m_dbl_height_prev_row;
		}

		m_fg = 7;
		m_bg = 0;
		m_graphics = false;
		m_separated = false;
		m_flash = false;
		m_boxed = false;
		m_hold_char = false;
		m_held_char = 0x20;
		m_next_chartype = ALPHANUMERIC;
		m_held_chartype = ALPHANUMERIC;
		m_dbl_height = false;
		m_dbl_height_prev_row = false;
		m_bit = 11;
	}
}


//-------------------------------------------------
//  tlc_r - transmitted large character
//-------------------------------------------------

int saa5050_device::tlc_r()
{
	return !m_dbl_height_bottom_row;
}


//-------------------------------------------------
//  write - character data write
//-------------------------------------------------

void saa5050_device::write(uint8_t data)
{
	m_code = data & 0x7f;
}


//-------------------------------------------------
//  f1_w - character clock
//-------------------------------------------------

void saa5050_device::f1_w(int state)
{
	if (state)
	{
		get_character_data(m_code);
	}
}


//-------------------------------------------------
//  tr6_w - pixel clock
//-------------------------------------------------

void saa5050_device::tr6_w(int state)
{
	if (state)
	{
		m_color = BIT(m_char_data, m_bit) ? m_prev_col : m_bg;

		m_bit--;
		if (m_bit < 0) m_bit = 11;
	}
}


//-------------------------------------------------
//  get_rgb - get output color
//-------------------------------------------------

int saa5050_device::get_rgb()
{
	return m_color;
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

uint32_t saa5050_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	dew_w(1);
	dew_w(0);

	for (int y = 0; y < m_rows * 20; y++)
	{
		int sy = y / 20;
		int x = 0;

		lose_w(1);
		lose_w(0);

		int ssy = tlc_r() ? sy : sy - 1;
		offs_t video_ram_addr = ssy * m_size;

		for (int sx = 0; sx < m_cols; sx++)
		{
			uint8_t code = m_read_d(video_ram_addr++);

			write(code & 0x7f);

			f1_w(1);
			f1_w(0);

			for (int bit = 0; bit < 12; bit++)
			{
				tr6_w(1);
				tr6_w(0);

				int color = get_rgb();

				if (BIT(code, 7)) color ^= 0x07;

				int r = BIT(color, 0) * 0xff;
				int g = BIT(color, 1) * 0xff;
				int b = BIT(color, 2) * 0xff;

				bitmap.pix(y, x++) = rgb_t(r, g, b);
			}
		}
	}

	return 0;
}
