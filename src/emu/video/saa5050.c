/**********************************************************************

    Mullard SAA5050 Teletext Character Generator emulation

    http://www.bighole.nl/pub/mirror/homepage.ntlworld.com/kryten_droid/teletext/spec/teletext_spec_1974.htm

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - character rounding
    - remote controller input
    - boxing

*/

#include "saa5050.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SAA5050 = &device_creator<saa5050_device>;
const device_type SAA5052 = &device_creator<saa5052_device>;


//-------------------------------------------------
//  ROM( saa5050 )
//-------------------------------------------------

ROM_START( saa5050 )
	ROM_REGION( 0x10000, "chargen", 0 )
	ROM_LOAD( "saa5050", 0x0140, 0x08c0, BAD_DUMP CRC(78c17e3e) SHA1(4e1c59dc484505de1dc0b1ba7e5f70a54b0d4ccc) )
ROM_END


//-------------------------------------------------
//  ROM( saa5052 )
//-------------------------------------------------

ROM_START( saa5052 )
	ROM_REGION( 0x10000, "chargen", 0 )
	ROM_LOAD( "saa5052", 0x0140, 0x08c0, BAD_DUMP CRC(cda3bf79) SHA1(cf5ea94459c09001d422dadc212bc970b4b4aa20) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *saa5050_device::device_rom_region() const
{
	switch (m_variant)
	{
		default:		return ROM_NAME( saa5050 );
		case TYPE_5052:	return ROM_NAME( saa5052 );
	}
}



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	NUL = 0,
	ALPHA_RED,
	ALPHA_GREEN,
	ALPHA_YELLOW,
	ALPHA_BLUE,
	ALPHA_MAGENTA,
	ALPHA_CYAN,
	ALPHA_WHITE,
	FLASH,
	STEADY,
	END_BOX,
	START_BOX,
	NORMAL_HEIGHT,
	DOUBLE_HEIGHT,
	S0,
	S1,
	DLE,
	GRAPHICS_RED,
	GRAPHICS_GREEN,
	GRAPHICS_YELLOW,
	GRAPHICS_BLUE,
	GRAPHICS_MAGENTA,
	GRAPHICS_CYAN,
	GRAPHICS_WHITE,
	CONCEAL_DISPLAY,
	CONTIGUOUS_GFX,
	SEPARATED_GFX,
	ESC,
	BLACK_BACKGROUND,
	NEW_BACKGROUND,
	HOLD_GRAPHICS,
	RELEASE_GRAPHICS
};


static const rgb_t PALETTE[] =
{
	RGB_BLACK,
	MAKE_RGB(0xff, 0x00, 0x00),
	MAKE_RGB(0x00, 0xff, 0x00),
	MAKE_RGB(0xff, 0xff, 0x00),
	MAKE_RGB(0x00, 0x00, 0xff),
	MAKE_RGB(0xff, 0x00, 0xff),
	MAKE_RGB(0x00, 0xff, 0xff),
	RGB_WHITE
};



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saa5050_device - constructor
//-------------------------------------------------

saa5050_device::saa5050_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant)
    : device_t(mconfig, type, name, tag, owner, clock),
      m_frame_count(0),
      m_variant(variant)
{
}

saa5050_device::saa5050_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, SAA5050, "SAA5050", tag, owner, clock),
      m_frame_count(0),
      m_variant(TYPE_5050)
{
}

saa5052_device::saa5052_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : saa5050_device(mconfig, SAA5052, "SAA5052", tag, owner, clock, TYPE_5052)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void saa5050_device::device_config_complete()
{
	// inherit a copy of the static code
	const saa5050_interface *intf = reinterpret_cast<const saa5050_interface *>(static_config());
	if (intf != NULL)
		*static_cast<saa5050_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_d_cb, 0, sizeof(m_in_d_cb));
	}

	switch (m_variant)
	{
		default:		m_shortname = "saa5050"; break;
		case TYPE_5052:	m_shortname = "saa5052"; break;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saa5050_device::device_start()
{
	// resolve callbacks
	m_in_d_func.resolve(m_in_d_cb, *this);

	// find memory regions
	m_char_rom = memregion("chargen")->base();

	// register for state saving
	save_item(NAME(m_code));
	save_item(NAME(m_last_code));
	save_item(NAME(m_char_data));
	save_item(NAME(m_bit));
	save_item(NAME(m_color));
	save_item(NAME(m_ra));
	save_item(NAME(m_bg));
	save_item(NAME(m_fg));
	save_item(NAME(m_graphics));
	save_item(NAME(m_separated));
	save_item(NAME(m_conceal));
	save_item(NAME(m_flash));
	save_item(NAME(m_boxed));
	save_item(NAME(m_double_height));
	save_item(NAME(m_double_height_top_row));
	save_item(NAME(m_double_height_bottom_row));
	save_item(NAME(m_hold));
	save_item(NAME(m_frame_count));
	save_item(NAME(m_variant));
}


//-------------------------------------------------
//  process_control_character -
//-------------------------------------------------

void saa5050_device::process_control_character(UINT8 data)
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
		m_conceal = false;
		m_fg = data & 0x07;
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
		m_double_height = 0;
		break;

	case DOUBLE_HEIGHT:
		if (!m_double_height_bottom_row)
		{
			m_double_height_top_row = true;
		}

		m_double_height = 1;
		break;

	case GRAPHICS_RED:
	case GRAPHICS_GREEN:
	case GRAPHICS_YELLOW:
	case GRAPHICS_BLUE:
	case GRAPHICS_MAGENTA:
	case GRAPHICS_CYAN:
	case GRAPHICS_WHITE:
		m_graphics = true;
		m_conceal = false;
		m_fg = data & 0x07;
		break;

	case CONCEAL_DISPLAY:
		m_conceal = true;
		break;

	case CONTIGUOUS_GFX:
		m_separated = false;
		break;

	case SEPARATED_GFX:
		m_separated = true;
		break;

	case BLACK_BACKGROUND:
		m_bg = 0;
		break;

	case NEW_BACKGROUND:
		m_bg = m_fg;
		break;

	case HOLD_GRAPHICS:
		m_hold = true;
		break;

	case RELEASE_GRAPHICS:
		m_hold = false;
		break;
	}
}


//-------------------------------------------------
//  get_character_data -
//-------------------------------------------------

void saa5050_device::get_character_data(UINT8 data)
{
	if (m_graphics && (data & 0x20))
	{
		data += (data & 0x40) ? 64 : 96;
		if (m_separated) data += 64;
	}

	if ((data < 0x20) && m_hold) data = m_last_code;
	if (m_conceal) data = 0x20;
	if (m_flash && (m_frame_count > 38)) data = 0x20;
	if (m_double_height_bottom_row && !m_double_height) data = 0x20;
	m_last_code = data;

	offs_t ra = m_ra >> 1;
	if (m_double_height) ra >>= 1;
	if (m_double_height && m_double_height_bottom_row) ra += 5;

	offs_t char_rom_addr = (data * 10) + ra;
	m_char_data = m_char_rom[char_rom_addr];
}


//-------------------------------------------------
//  dew_w - data entry window
//-------------------------------------------------

WRITE_LINE_MEMBER( saa5050_device::dew_w )
{
	if (state)
	{
		m_ra = 19;
		m_double_height_top_row = false;

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

		m_fg = 7;
		m_bg = 0;
		m_graphics = false;
		m_separated = false;
		m_conceal = false;
		m_flash = false;
		m_boxed = false;
		m_hold = false;
		m_double_height = 0;
		m_bit = 5;
		m_last_code = 0x20;

		if (!m_ra)
		{
			m_double_height_bottom_row = m_double_height_top_row;
			m_double_height_top_row = false;
		}
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
		process_control_character(m_code);
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
		m_color = BIT(m_char_data, m_bit) ? m_fg : m_bg;

		m_bit--;
		if (m_bit < 0) m_bit = 5;
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
			UINT8 code = m_in_d_func(video_ram_addr++);

			write(code & 0x7f);

			f1_w(1);
			f1_w(0);

			for (int bit = 0; bit < 6; bit++)
			{
				tr6_w(1);
				tr6_w(0);

				int color = get_rgb();

				if (BIT(code, 7)) color ^= 0x07;

				int r = BIT(color, 0) * 0xff;
				int g = BIT(color, 1) * 0xff;
				int b = BIT(color, 2) * 0xff;

				rgb_t rgb = MAKE_RGB(r, g, b);

				bitmap.pix32(y, x++) = rgb;
				bitmap.pix32(y, x++) = rgb;
			}
		}
	}

	return 0;
}
