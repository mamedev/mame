// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Philips SAA5240 European Controlled Teletext Circuit

*********************************************************************/

#include "emu.h"
#include "saa5240.h"

#define LOG_DATA (1 << 1)
#define LOG_LINE (1 << 2)

#define VERBOSE (LOG_DATA)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(SAA5240A, saa5240a_device, "saa5240a", "SAA5240A EURO CCT")
DEFINE_DEVICE_TYPE(SAA5240B, saa5240b_device, "saa5240b", "SAA5240B EURO CCT")
DEFINE_DEVICE_TYPE(SAA5243E, saa5243e_device, "saa5243e", "SAA5243E EURO CCT")
//DEFINE_DEVICE_TYPE(SAA5243H, saa5243h_device, "saa5243h", "SAA5243H EURO CCT")


//-------------------------------------------------
//  default address map
//-------------------------------------------------

void saa5240_device::saa5240_vram(address_map &map)
{
	if (!has_configured_map(0))
		map(0x0000, 0x1fff).ram();
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector saa5240_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

//-------------------------------------------------
//  ROM( saa5240 )
//-------------------------------------------------

ROM_START( saa5240a )
	ROM_REGION16_BE( 0xc80, "chargen", ROMREGION_ERASE00 )
	ROM_LOAD("saa5240a", 0x0280, 0x0a00, BAD_DUMP CRC(e205d1fc) SHA1(cb6872260c91f0665f8c7d691c9becc327e0ecc3)) // hand made from datasheet
ROM_END

ROM_START( saa5240b )
	ROM_REGION16_BE( 0xc80, "chargen", ROMREGION_ERASE00 )
	ROM_LOAD("saa5240b", 0x0280, 0x0a00, BAD_DUMP CRC(7b196f2c) SHA1(61d4460ed0956ff470cc1362ea6bb1f9abe4bc03)) // hand made from datasheet
ROM_END

ROM_START( saa5243e )
	ROM_REGION16_BE( 0x1180, "chargen", ROMREGION_ERASE00 )
	ROM_LOAD("saa5243e", 0x0280, 0x0f00, BAD_DUMP CRC(a74402e7) SHA1(48123c9bb0377e417a147f5a68088c1ed7bee12d)) // hand made from datasheet
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *saa5240a_device::device_rom_region() const
{
	return ROM_NAME( saa5240a );
}

const tiny_rom_entry *saa5240b_device::device_rom_region() const
{
	return ROM_NAME( saa5240b );
}

const tiny_rom_entry *saa5243e_device::device_rom_region() const
{
	return ROM_NAME( saa5243e );
}



#define ALPHANUMERIC    0x01
#define CONTIGUOUS      0x02
#define SEPARATED       0x03


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saa5240_device - constructor
//-------------------------------------------------

saa5240_device::saa5240_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_char_rom(*this, "chargen")
	, m_space_config("videoram", ENDIANNESS_LITTLE, 8, 13, 0, address_map_constructor(FUNC(saa5240a_device::saa5240_vram), this))
	, m_slave_address(SAA5240_SLAVE_ADDRESS)
	, m_i2c_scl(0)
	, m_i2c_sdaw(0)
	, m_i2c_sdar(1)
	, m_i2c_state(STATE_IDLE)
	, m_i2c_bits(0)
	, m_i2c_shift(0)
	, m_i2c_devsel(0)
	, m_i2c_address(0)
{
}

saa5240a_device::saa5240a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	saa5240_device(mconfig, SAA5240A, tag, owner, clock)
{
}

saa5240b_device::saa5240b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	saa5240_device(mconfig, SAA5240B, tag, owner, clock)
{
}

saa5243e_device::saa5243e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	saa5240_device(mconfig, SAA5243E, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saa5240_device::device_start()
{
	m_videoram = &space(0);

	// state saving
	save_item(NAME(m_i2c_scl));
	save_item(NAME(m_i2c_sdaw));
	save_item(NAME(m_i2c_sdar));
	save_item(NAME(m_i2c_state));
	save_item(NAME(m_i2c_bits));
	save_item(NAME(m_i2c_shift));
	save_item(NAME(m_i2c_devsel));
	save_item(NAME(m_i2c_address));
	save_item(NAME(m_register));
	save_item(NAME(m_slave_address));
}

void saa5240_device::device_reset()
{
	// memmory is cleared to 'space' on power-up
	for (int i = 0; i < 0x1fff; i++)
		m_videoram->write_byte(i, 0x20);

	// row 0 column 7 chapter 0 is alpha white
	m_videoram->write_byte(7, 0x07);

	// reset registers
	std::fill(std::begin(m_register), std::end(m_register), 0x00);
	m_register[5] = 0x03;
	m_register[6] = 0x03;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_MEMBER(saa5240_device::write_scl)
{
	if (m_i2c_scl != state)
	{
		m_i2c_scl = state;
		LOGMASKED(LOG_LINE, "set_scl_line %d\n", m_i2c_scl);

		switch (m_i2c_state)
		{
		case STATE_DEVSEL:
		case STATE_ADDRESS:
		case STATE_DATAIN:
			if (m_i2c_bits < 8)
			{
				if (m_i2c_scl)
				{
					m_i2c_shift = ((m_i2c_shift << 1) | m_i2c_sdaw) & 0xff;
					m_i2c_bits++;
				}
			}
			else
			{
				if (m_i2c_scl)
				{
					switch (m_i2c_state)
					{
					case STATE_DEVSEL:
						m_i2c_devsel = m_i2c_shift;

						if ((m_i2c_devsel & 0xfe) != m_slave_address)
						{
							LOGMASKED(LOG_DATA, "devsel %02x: not this device\n", m_i2c_devsel);
							m_i2c_state = STATE_IDLE;
						}
						else if ((m_i2c_devsel & 1) == 0)
						{
							LOGMASKED(LOG_DATA, "devsel %02x: write\n", m_i2c_devsel);
							m_i2c_state = STATE_ADDRESS;
						}
						else
						{
							LOGMASKED(LOG_DATA, "devsel %02x: read\n", m_i2c_devsel);
							m_i2c_state = STATE_DATAOUT;
						}
						break;

					case STATE_ADDRESS:
						m_i2c_address = m_i2c_shift;
						LOGMASKED(LOG_DATA, "address %02x\n", m_i2c_shift);

						m_i2c_state = STATE_DATAIN;
						break;

					case STATE_DATAIN:
						m_register[m_i2c_address] = m_i2c_shift;
						LOGMASKED(LOG_DATA, "data[ %02x ] <- %02x\n", m_i2c_address, m_i2c_shift);

						switch (m_i2c_address)
						{
						case 1: case 2: case 3: case 4: case 5: case 6:
							// auto-increment register
							m_i2c_address++;
							break;

						case 8:
							// active chapter - clear memory
							//if (BIT(m_register[8], 3))
							//{
							//  for (int i = 0; i < 0x3ff; i++)
							//      m_videoram->write_byte((m_register[8] & 0x07) * 0x400 + i, 0x20);
							//}
							[[fallthrough]];
						case 9: case 10:
							update_active_data();

							// auto-increment register
							m_i2c_address++;
							break;
						case 11:
							// active data
							offs_t offset = (m_register[8] & 0x07) * 0x400 + (m_register[9] & 0x1f) * 40 + (m_register[10] & 0x3f);
							m_videoram->write_byte(offset, m_i2c_shift);

							increment_active_data();
							break;
						}
						break;
					}

					m_i2c_bits++;
				}
				else
				{
					if (m_i2c_bits == 8)
					{
						m_i2c_sdar = 0;
					}
					else
					{
						m_i2c_bits = 0;
						m_i2c_sdar = 1;
					}
				}
			}
			break;

		case STATE_DATAOUT:
			if (m_i2c_bits < 8)
			{
				if (m_i2c_scl)
				{
					if (m_i2c_bits == 0)
					{
						m_i2c_shift = m_register[m_i2c_address];
						LOGMASKED(LOG_DATA, "data[ %02x ] -> %02x\n", m_i2c_address, m_i2c_shift);

						if (m_i2c_address == 11)
							increment_active_data();

						// FIXME: is this conditional?
						if (BIT(m_register[1], 6))
						{
							// 7 bits with parity checking
							m_i2c_shift &= 0x7f;
							m_i2c_shift = (m_i2c_shift << 1) | calc_parity(m_i2c_shift);
						}
					}

					m_i2c_sdar = (m_i2c_shift >> 7) & 1;

					m_i2c_shift = (m_i2c_shift << 1) & 0xff;
					m_i2c_bits++;
				}
			}
			else
			{
				if (m_i2c_scl)
				{
					if (m_i2c_sdaw)
					{
						LOGMASKED(LOG_DATA, "nack\n");
						m_i2c_state = STATE_IDLE;
					}

					m_i2c_bits++;
				}
				else
				{
					if (m_i2c_bits == 8)
					{
						m_i2c_sdar = 1;
					}
					else
					{
						m_i2c_bits = 0;
					}
				}
			}
			break;
		}
	}
}

WRITE_LINE_MEMBER(saa5240_device::write_sda)
{
	state &= 1;
	if (m_i2c_sdaw != state)
	{
		LOGMASKED(LOG_LINE, "set sda %d\n", state);
		m_i2c_sdaw = state;

		if (m_i2c_scl)
		{
			if (m_i2c_sdaw)
			{
				LOGMASKED(LOG_DATA, "stop\n");
				m_i2c_state = STATE_IDLE;
			}
			else
			{
				LOGMASKED(LOG_DATA, "start\n");
				m_i2c_state = STATE_DEVSEL;
				m_i2c_bits = 0;
			}

			m_i2c_sdar = 1;
		}
	}
}

READ_LINE_MEMBER(saa5240_device::read_sda)
{
	int res = m_i2c_sdar & 1;

	LOGMASKED(LOG_LINE, "read sda %d\n", res);

	return res;
}


void saa5240_device::increment_active_data()
{
	// auto-increment column
	m_register[10]++;

	if (m_register[10] >= 40)
	{
		m_register[10] = 0;
		m_register[9]++;
	}

	update_active_data();
}

void saa5240_device::update_active_data()
{
	offs_t offset = (m_register[8] & 0x07) * 0x400 + (m_register[9] & 0x1f) * 40 + (m_register[10] & 0x3f);
	m_register[11] = m_videoram->read_byte(offset);
}

//-------------------------------------------------
//  calculate the byte parity
//-------------------------------------------------

int saa5240_device::calc_parity(uint8_t data)
{
	uint8_t count = 0;

	while(data != 0)
	{
		count++;
		data &= (data-1);
	}

	return (count ^ 1) & 1;
}



//-------------------------------------------------
//  process_control_character
//-------------------------------------------------

void saa5240_device::process_control_character(uint8_t data)
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


void saa5240_device::set_next_chartype()
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

uint16_t saa5240_device::get_gfx_data(uint8_t data, offs_t row, bool separated)
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

uint16_t saa5240_device::get_rom_data(uint8_t data, offs_t row)
{
	uint16_t c;
	if (row < 0 || row >= 10)
	{
		c = 0;
	}
	else
	{
		c = m_char_rom[(data * 10) + row];
	}
	return c;
}


//-------------------------------------------------
//  get_character_data
//-------------------------------------------------

void saa5240_device::get_character_data(uint8_t data)
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
		m_char_data = get_rom_data(data, ra);
	else
		m_char_data = get_gfx_data(data, ra, (m_curr_chartype == SEPARATED));
}


//-------------------------------------------------
//  vcs_w - video composite sync
//-------------------------------------------------

WRITE_LINE_MEMBER(saa5240_device::vcs_w)
{
	if (state)
	{
		m_ra++;
		m_ra %= 10;

		if (!m_ra)
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
//  f6_w - character display clock
//-------------------------------------------------

WRITE_LINE_MEMBER(saa5240_device::f6_w)
{
	if (state)
	{
		m_color = BIT(m_char_data, m_bit) ? m_prev_col : m_bg;

		m_bit--;
		if (m_bit < 0)
		{
			m_bit = 11;
		}
	}
}


//-------------------------------------------------
//  get_rgb - get output color
//-------------------------------------------------

int saa5240_device::get_rgb()
{
	return m_color;
}


//-------------------------------------------------
//  screen_update
//-------------------------------------------------

uint32_t saa5240_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 25 * 10; y++)
	{
		int sy = y / 10;
		int x = 0;

		vcs_w(1);
		vcs_w(0);

		for (int sx = 0; sx < 40; sx++)
		{
			uint8_t code = m_videoram->read_byte((sy * 40) + sx) & 0x7f;
			get_character_data(code);

			for (int bit = 0; bit < 12; bit++)
			{
				f6_w(1);
				f6_w(0);

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
