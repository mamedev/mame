// license:BSD-3-Clause
// copyright-holders:Curt Coder, Nigel Barnes
/**********************************************************************

    Mullard SAA5050 Teletext Character Generator emulation

    http://www.bighole.nl/pub/mirror/homepage.ntlworld.com/kryten_droid/teletext/spec/teletext_spec_1974.htm

**********************************************************************/

/*

    TODO:

    - remote controller input
    - boxing

*/

#include "saa5050.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SAA5050 = &device_creator<saa5050_device>;
const device_type SAA5051 = &device_creator<saa5051_device>;
const device_type SAA5052 = &device_creator<saa5052_device>;
const device_type SAA5053 = &device_creator<saa5053_device>;
const device_type SAA5054 = &device_creator<saa5054_device>;
const device_type SAA5055 = &device_creator<saa5055_device>;
const device_type SAA5056 = &device_creator<saa5056_device>;
const device_type SAA5057 = &device_creator<saa5057_device>;


//-------------------------------------------------
//  ROM( saa5050 )
//-------------------------------------------------

ROM_START( saa5050 )
	ROM_REGION( 0x500, "chargen", 0 )
	ROM_LOAD("saa5050", 0x0140, 0x03c0, BAD_DUMP CRC(6298fc0b) SHA1(ae38e7f51dd33733bacfa896425ca105682b31d6))
ROM_END


//-------------------------------------------------
//  ROM( saa5051 )
//-------------------------------------------------

ROM_START( saa5051 )
	ROM_REGION( 0x500, "chargen", 0 )
	ROM_LOAD("saa5051", 0x0140, 0x03c0, BAD_DUMP CRC(a770611c) SHA1(9ab9d24b845fe2964fba2f4770d54025d2c8026a))
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
	ROM_LOAD("saa5057", 0x0140, 0x08c0, BAD_DUMP CRC(d6664fb3) SHA1(5a93445dde03066073e2909a935900e5f8439d81))
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *saa5050_device::device_rom_region() const
{
	return ROM_NAME( saa5050 );
}

const rom_entry *saa5051_device::device_rom_region() const
{
	return ROM_NAME( saa5051 );
}

const rom_entry *saa5052_device::device_rom_region() const
{
	return ROM_NAME( saa5052 );
}

const rom_entry *saa5053_device::device_rom_region() const
{
	return ROM_NAME( saa5053 );
}

const rom_entry *saa5054_device::device_rom_region() const
{
	return ROM_NAME( saa5054 );
}

const rom_entry *saa5055_device::device_rom_region() const
{
	return ROM_NAME( saa5055 );
}

const rom_entry *saa5056_device::device_rom_region() const
{
	return ROM_NAME( saa5056 );
}

const rom_entry *saa5057_device::device_rom_region() const
{
	return ROM_NAME( saa5057 );
}


#define ALPHANUMERIC    0x01
#define CONTIGUOUS      0x02
#define SEPARATED       0x03


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saa5050_device - constructor
//-------------------------------------------------

saa5050_device::saa5050_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_char_rom(*this, "chargen"),
	m_read_d(*this),
	m_frame_count(0),
	m_cols(0),
	m_rows(0),
	m_size(0)
{
}

saa5050_device::saa5050_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SAA5050, "SAA5050 Video", tag, owner, clock, "saa5050", __FILE__),
	m_char_rom(*this, "chargen"),
	m_read_d(*this),
	m_frame_count(0),
	m_cols(0),
	m_rows(0),
	m_size(0)
{
}

saa5051_device::saa5051_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: saa5050_device(mconfig, SAA5051, "SAA5051", tag, owner, clock, "saa5051", __FILE__) { }

saa5052_device::saa5052_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: saa5050_device(mconfig, SAA5052, "SAA5052", tag, owner, clock, "saa5052", __FILE__) { }

saa5053_device::saa5053_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: saa5050_device(mconfig, SAA5053, "SAA5053", tag, owner, clock, "saa5053", __FILE__) { }

saa5054_device::saa5054_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: saa5050_device(mconfig, SAA5054, "SAA5054", tag, owner, clock, "saa5054", __FILE__) { }

saa5055_device::saa5055_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: saa5050_device(mconfig, SAA5055, "SAA5055", tag, owner, clock, "saa5055", __FILE__) { }

saa5056_device::saa5056_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: saa5050_device(mconfig, SAA5056, "SAA5056", tag, owner, clock, "saa5056", __FILE__) { }

saa5057_device::saa5057_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: saa5050_device(mconfig, SAA5057, "SAA5057", tag, owner, clock, "saa5057", __FILE__) { }



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saa5050_device::device_start()
{
	// resolve callbacks
	m_read_d.resolve_safe(0);

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
	save_item(NAME(m_double_height));
	save_item(NAME(m_double_height_old));
	save_item(NAME(m_double_height_bottom_row));
	save_item(NAME(m_double_height_prev_row));
	save_item(NAME(m_hold_char));
	save_item(NAME(m_hold_clear));
	save_item(NAME(m_hold_off));
	save_item(NAME(m_frame_count));
}


//-------------------------------------------------
//  device_start - device-specific reset
//-------------------------------------------------

void saa5050_device::device_reset()
{
	m_ra = 0;
	m_double_height = false;
	m_double_height_bottom_row = false;
}


//-------------------------------------------------
//  process_control_character -
//-------------------------------------------------

void saa5050_device::process_control_character(UINT8 data)
{
	m_hold_clear = false;
	m_hold_off = false;

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
			m_hold_clear = true;
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
			m_double_height = !!(data & 1);
			if (m_double_height) m_double_height_prev_row = true;
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
			m_hold_off = true;
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

UINT16 saa5050_device::get_gfx_data(UINT8 data, offs_t row, bool separated)
{
	UINT16 c = 0;
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

UINT16 saa5050_device::get_rom_data(UINT8 data, offs_t row)
{
	UINT16 c;
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

UINT16 saa5050_device::character_rounding(UINT16 a, UINT16 b)
{
	return a | ((a >> 1) & b & ~(b >> 1)) | ((a << 1) & b & ~(b << 1));
}


//-------------------------------------------------
//  get_character_data -
//-------------------------------------------------

void saa5050_device::get_character_data(UINT8 data)
{
	m_double_height_old = m_double_height;
	m_prev_col = m_fg;
	m_curr_chartype = m_next_chartype;

	if (data < 0x20)
	{
		process_control_character(data);
		if (m_hold_char && m_double_height == m_double_height_old)
		{
			data = m_held_char;
			if (data >= 0x40 && data < 0x60) data = 0x20;
			m_curr_chartype = m_held_chartype;
		}
		else
		{
			data = 0x20;
		}
	}
	else if (m_graphics)
	{
		m_held_char = data;
		m_held_chartype = m_curr_chartype;
	}

	offs_t ra = m_ra;
	if (m_double_height_old)
	{
		ra >>= 1;
		if (m_double_height_bottom_row) ra += 10;
	}

	if (m_flash && (m_frame_count > 38)) data = 0x20;
	if (m_double_height_bottom_row && !m_double_height) data = 0x20;

	if (m_hold_off)
	{
		m_hold_char = false;
		m_held_char = 32;
	}
	if (m_hold_clear)
	{
		m_held_char = 32;
	}

	if (m_curr_chartype == ALPHANUMERIC || !BIT(data,5))
		m_char_data = character_rounding(get_rom_data(data, ra), get_rom_data(data, ra + ((ra & 1) ? 1 : -1)));
	else
		m_char_data = get_gfx_data(data, ra, (m_curr_chartype == SEPARATED));
}


//-------------------------------------------------
//  crs_w - character rounding select
//-------------------------------------------------

WRITE_LINE_MEMBER( saa5050_device::crs_w )
{
	m_crs = !(state & 1);
}


//-------------------------------------------------
//  dew_w - data entry window
//-------------------------------------------------

WRITE_LINE_MEMBER( saa5050_device::dew_w )
{
	if (state)
	{
		m_ra = 19;

		m_frame_count++;
		if (m_frame_count > 50) m_frame_count = 0;
	}
}


//-------------------------------------------------
//  lose_w - load output shift register enable
//-------------------------------------------------

WRITE_LINE_MEMBER( saa5050_device::lose_w )
{
	if (state)
	{
		m_ra++;
		m_ra %= 20;

		if (m_ra == 19)
		{
			if (m_double_height_bottom_row)
				m_double_height_bottom_row = false;
			else
				m_double_height_bottom_row = m_double_height_prev_row;
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
		m_double_height = false;
		m_double_height_prev_row = false;
		m_bit = 11;
	}
}


//-------------------------------------------------
//  write - character data write
//-------------------------------------------------

void saa5050_device::write(UINT8 data)
{
	m_code = data & 0x7f;
}


//-------------------------------------------------
//  f1_w - character clock
//-------------------------------------------------

WRITE_LINE_MEMBER( saa5050_device::f1_w )
{
	if (state)
	{
		get_character_data(m_code);
	}
}


//-------------------------------------------------
//  tr6_w - pixel clock
//-------------------------------------------------

WRITE_LINE_MEMBER( saa5050_device::tr6_w )
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

UINT32 saa5050_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	dew_w(1);
	dew_w(0);

	for (int y = 0; y < m_rows * 20; y++)
	{
		int sy = y / 20;
		int x = 0;

		lose_w(1);
		lose_w(0);

		int ssy = m_double_height_bottom_row ? sy - 1 : sy;
		offs_t video_ram_addr = ssy * m_size;

		for (int sx = 0; sx < m_cols; sx++)
		{
			UINT8 code = m_read_d(video_ram_addr++);

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

				rgb_t rgb = rgb_t(r, g, b);

				bitmap.pix32(y, x++) = rgb;
			}
		}
	}

	return 0;
}
