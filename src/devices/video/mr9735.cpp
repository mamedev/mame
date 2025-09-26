// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    MR9735 Teletext/Viewdata 625-line Video Generator

**********************************************************************/

#include "emu.h"
#include "mr9735.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MR9735_002, mr9735_002_device, "mr9735_002", "MR9735-002 Teletext/Viewdata Video Generator")


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( mr9735_002 )
	ROM_REGION( 0x500, "chargen", 0 )
	ROM_LOAD("mr9735-002", 0x0140, 0x03c0, BAD_DUMP CRC(201490f3) SHA1(6c8daba70374e5aa3a6402f24cdc5f8677d58a0f)) // assumed to be the same as SAA5050
ROM_END

const tiny_rom_entry *mr9735_002_device::device_rom_region() const
{
	return ROM_NAME( mr9735_002 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mr9735_002_device - constructor
//-------------------------------------------------

mr9735_002_device::mr9735_002_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_char_rom(*this, "chargen")
	, m_read_data(*this, 0)
	, m_frame_count(0)
	, m_interlace(false)
{
}

mr9735_002_device::mr9735_002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mr9735_002_device(mconfig, MR9735_002, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mr9735_002_device::device_start()
{
	save_item(NAME(m_held_char));
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
//  device_reset - device-specific reset
//-------------------------------------------------

void mr9735_002_device::device_reset()
{
	m_ra = 0;
	m_dbl_height = false;
	m_dbl_height_bottom_row = false;

	if (m_interlace)
		m_char_rows = 20;
	else
		m_char_rows = 10;
}


//-------------------------------------------------
//  process_control_character -
//-------------------------------------------------

void mr9735_002_device::process_control_character(uint8_t data)
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


void mr9735_002_device::set_next_chartype()
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

uint16_t mr9735_002_device::get_gfx_data(uint8_t data, offs_t row, bool separated)
{
	uint16_t c = 0;

	if (m_interlace)
		row >>= 1;

	switch (row)
	{
	case 0: case 1:
		c += (data & 0x01) ? 0x38 : 0; // bit 1 top left
		c += (data & 0x02) ? 0x07 : 0; // bit 2 top right
		if (separated) c &= 0x36;
		break;
	case 2:
		if (separated) break;
		c += (data & 0x01) ? 0x38 : 0; // bit 1 top left
		c += (data & 0x02) ? 0x07 : 0; // bit 2 top right
		break;
	case 3: case 4: case 5:
		c += (data & 0x04) ? 0x38 : 0; // bit 3 middle left
		c += (data & 0x08) ? 0x07 : 0; // bit 4 middle right
		if (separated) c &= 0x36;
		break;
	case 6:
		if (separated) break;
		c += (data & 0x04) ? 0x38 : 0; // bit 3 middle left
		c += (data & 0x08) ? 0x07 : 0; // bit 4 middle right
		break;
	case 7: case 8:
		c += (data & 0x10) ? 0x38 : 0; // bit 5 bottom left
		c += (data & 0x40) ? 0x07 : 0; // bit 7 bottom right
		if (separated) c &= 0x36;
		break;
	case 9:
		if (separated) break;
		c += (data & 0x10) ? 0x38 : 0; // bit 5 bottom left
		c += (data & 0x40) ? 0x07 : 0; // bit 7 bottom right
		break;
	}

	if (m_interlace)
		c = ((c & 0x01) * 0x03) + ((c & 0x02) * 0x06) + ((c & 0x04) * 0x0c) + ((c & 0x08) * 0x18) + ((c & 0x10) * 0x30) + ((c & 0x20) * 0x60);

	return c;
}


//-------------------------------------------------
//  get_rom_data - read rom
//-------------------------------------------------

uint16_t mr9735_002_device::get_rom_data(uint8_t data, offs_t row)
{
	uint16_t c;

	if (row < 0 || row >= m_char_rows)
	{
		c = 0;
	}
	else
	{
		if (m_interlace)
			row >>= 1;

		c = m_char_rom[(data * 10) + row];

		if (m_interlace)
			c = ((c & 0x01) * 0x03) + ((c & 0x02) * 0x06) + ((c & 0x04) * 0x0c) + ((c & 0x08) * 0x18) + ((c & 0x10) * 0x30);
	}

	return c;
}


//-------------------------------------------------
//  character_rounding
//-------------------------------------------------

uint16_t mr9735_002_device::character_rounding(uint16_t a, uint16_t b)
{
	return a | ((a >> 1) & b & ~(b >> 1)) | ((a << 1) & b & ~(b << 1));
}


//-------------------------------------------------
//  get_character_data -
//-------------------------------------------------

uint16_t mr9735_002_device::get_character_data(uint8_t data)
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
		if (m_dbl_height_bottom_row)
		{
			ra += m_interlace ? 10 : 5;
		}
	}

	if (flash_prev && !(m_frame_count & 0x30))
		data = 0x20;

	if (m_dbl_height_bottom_row && !m_dbl_height)
		data = 0x20;

	uint16_t char_data;

	if (m_curr_chartype == ALPHANUMERIC || !BIT(data, 5))
	{
		if (m_interlace)
			char_data = character_rounding(get_rom_data(data, ra), get_rom_data(data, ra + ((ra & 1) ? 1 : -1)));
		else
			char_data = get_rom_data(data, ra);
	}
	else
	{
		char_data = get_gfx_data(data, ra, (m_curr_chartype == SEPARATED));
	}
	return char_data;
}


//-------------------------------------------------
//  lfb_w - line flyback
//-------------------------------------------------

void mr9735_002_device::lfb_w(int state)
{
	if (state)
	{
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
	}
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

uint32_t mr9735_002_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_frame_count++ > 50)
		m_frame_count = 0;

	for (int y = 0; y < 24 * m_char_rows; y++)
	{
		int sy = y / m_char_rows;
		int x = 0;

		m_ra = y % m_char_rows;

		lfb_w(1);
		lfb_w(0);

		int ssy = m_dbl_height_bottom_row ? sy - 1 : sy;
		offs_t video_ram_addr = ssy * NUM_COLS;

		for (int sx = 0; sx < NUM_COLS; sx++)
		{
			uint8_t code = ~m_read_data(video_ram_addr++);

			uint16_t char_data = get_character_data(code & 0x7f);

			for (int bit = (m_interlace ? 11 : 5); bit >= 0; bit--)
			{
				int color = BIT(char_data, bit) ? m_prev_col : m_bg;

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
