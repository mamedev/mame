// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Philips SAA5240 European Controlled Teletext Circuit (CCT)

    TODO:
    - implement data acquisition.
    - output interlaced video.

*********************************************************************/

#include "emu.h"
#include "saa5240.h"

#include "screen.h"

#define LOG_DATA (1U << 1)
#define LOG_LINE (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(SAA5240A, saa5240a_device, "saa5240a", "SAA5240A EURO CCT")
DEFINE_DEVICE_TYPE(SAA5240B, saa5240b_device, "saa5240b", "SAA5240B EURO CCT")
DEFINE_DEVICE_TYPE(SAA5243E, saa5243e_device, "saa5243e", "SAA5243E EURO CCT")
//DEFINE_DEVICE_TYPE(SAA5243H, saa5243h_device, "saa5243h", "SAA5243H EURO CCT")


//-------------------------------------------------
//  ROM( saa5240 )
//-------------------------------------------------

ROM_START(saa5240a)
	ROM_REGION16_BE(0xc80, "chargen", ROMREGION_ERASE00)
	ROM_LOAD("saa5240a", 0x0280, 0x0a00, BAD_DUMP CRC(e205d1fc) SHA1(cb6872260c91f0665f8c7d691c9becc327e0ecc3)) // hand made from datasheet
ROM_END

ROM_START(saa5240b)
	ROM_REGION16_BE(0xc80, "chargen", ROMREGION_ERASE00)
	ROM_LOAD("saa5240b", 0x0280, 0x0a00, BAD_DUMP CRC(7b196f2c) SHA1(61d4460ed0956ff470cc1362ea6bb1f9abe4bc03)) // hand made from datasheet
ROM_END

ROM_START(saa5243e)
	ROM_REGION16_BE(0x1180, "chargen", ROMREGION_ERASE00)
	ROM_LOAD("saa5243e", 0x0280, 0x0f00, BAD_DUMP CRC(a74402e7) SHA1(48123c9bb0377e417a147f5a68088c1ed7bee12d)) // hand made from datasheet
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *saa5240a_device::device_rom_region() const
{
	return ROM_NAME(saa5240a);
}

const tiny_rom_entry *saa5240b_device::device_rom_region() const
{
	return ROM_NAME(saa5240b);
}

const tiny_rom_entry *saa5243e_device::device_rom_region() const
{
	return ROM_NAME(saa5243e);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saa5240_device - constructor
//-------------------------------------------------

saa5240_device::saa5240_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_char_rom(*this, "chargen")
	, m_ram_size(0x2000)
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

saa5240a_device::saa5240a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saa5240_device(mconfig, SAA5240A, tag, owner, clock)
{
}

saa5240b_device::saa5240b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saa5240_device(mconfig, SAA5240B, tag, owner, clock)
{
}

saa5243e_device::saa5243e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saa5240_device(mconfig, SAA5243E, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saa5240_device::device_start()
{
	m_ram = make_unique_clear<uint8_t[]>(m_ram_size);

	// memory is cleared to 'space' on power-up
	std::memset(m_ram.get(), 0x20, m_ram_size);

	// row 0 column 7 chapter 0 is alpha white
	m_ram[0x07] = 0x07;

	// registers cleared on power-up, except R5 and R6
	std::fill(std::begin(m_register), std::end(m_register), 0x00);
	m_register[5] = 0x03;
	m_register[6] = 0x03;

	// state saving
	save_pointer(NAME(m_ram), m_ram_size);
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


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void saa5240_device::write_scl(int state)
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
					m_i2c_bits++;
				}
				else
				{
					if (m_i2c_bits == 8)
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
								m_i2c_state = STATE_READSELACK;
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
							case 1: // Mode
							case 2: // Page request address
							case 3: // Page request data
							case 4: // Display chapter
							case 5: // Display control (normal)
							case 6: // Display control (newsflash/subtitle)
								m_i2c_address++; // auto-increment register
								break;

							case 7: // Display mode
								break;

							case 8: // Active chapter
								if (BIT(m_register[8], 3))
								{
									// clear memory
									for (int i = 0; i < 0x400; i++)
										m_ram[ram_addr(m_register[8], 0, 0) + i] = 0x20;
								}
								[[fallthrough]];

							case 9:  // Active row
							case 10: // Active column
								update_active_data();
								m_i2c_address++; // auto-increment register
								break;

							case 11: // Active data
								m_ram[ram_addr(m_register[8], m_register[9], m_register[10])] = m_i2c_shift;
								increment_active_data();
								break;
							}
							break;
						}

						if (m_i2c_state != STATE_IDLE)
						{
							m_i2c_sdar = 0;
						}
					}
					else
					{
						m_i2c_bits = 0;
						m_i2c_sdar = 1;
					}
				}
			}
			break;

		case STATE_READSELACK:
			m_i2c_bits = 0;
			m_i2c_state = STATE_DATAOUT;
			break;

		case STATE_DATAOUT:
			if (m_i2c_bits < 8)
			{
				if (m_i2c_scl)
				{
					m_i2c_bits++;
				}
				else
				{
					if (m_i2c_bits == 0)
					{
						m_i2c_shift = m_register[m_i2c_address];
						LOGMASKED(LOG_DATA, "data[ %02x ] -> %02x\n", m_i2c_address, m_i2c_shift);

						if (m_i2c_address == 11)
							increment_active_data();

						if (!BIT(m_register[1], 6))
						{
							// 7 bits with parity checking
							m_i2c_shift &= 0x7f;
							m_i2c_shift = (m_i2c_shift << 1) | calc_parity(m_i2c_shift);
						}
					}

					m_i2c_sdar = (m_i2c_shift >> 7) & 1;

					m_i2c_shift = (m_i2c_shift << 1) & 0xff;
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

					m_i2c_bits = 0;
				}
				else
				{

					m_i2c_sdar = 1;
				}
			}
			break;
		}
	}
}

void saa5240_device::write_sda(int state)
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

int saa5240_device::read_sda()
{
	int res = m_i2c_sdar & 1;

	LOGMASKED(LOG_LINE, "read sda %d\n", res);

	return res;
}


uint16_t saa5240_device::ram_addr(int chapter, int row, int column)
{
	uint16_t addr = (chapter & 0x07) * 0x400 + (row & 0x1f) * 40 + (column & 0x3f);
	return addr & (m_ram_size - 1);
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
	m_register[11] = m_ram[ram_addr(m_register[8], m_register[9], m_register[10])];
}

//-------------------------------------------------
//  calculate the byte parity
//-------------------------------------------------

int saa5240_device::calc_parity(uint8_t data)
{
	uint8_t count = 0;

	while (data != 0)
	{
		count++;
		data &= (data - 1);
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
	static const uint8_t nc_char_matrix[6][128] =
	{
		// SAA5240A - German / SAA5240B - Italian / SAA5243E - German
		{
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
			0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
			0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
			0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
			0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
			0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
			0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f
		},
		// SAA5240A - English / SAA5240B - German / SAA5243E - English
		{
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
			0x20, 0x21, 0x22, 0x83, 0x84, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
			0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
			0x80, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
			0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
			0x81, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
			0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x88, 0x89, 0x8a, 0x7f
		},
		// SAA5240A - Swedish / SAA5240B - French / SAA5243E - Swedish
		{
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
			0x20, 0x21, 0x22, 0x93, 0x94, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
			0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
			0x90, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
			0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
			0x91, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
			0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x92, 0x98, 0x99, 0x9a, 0x7f
		},
		// SAA5243E - Italian
		{
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
			0x20, 0x21, 0x22, 0xa3, 0xa4, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
			0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
			0xa0, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
			0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0xab, 0xac, 0xad, 0xae, 0xaf,
			0xa1, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
			0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0xa2, 0xa8, 0xa9, 0xaa, 0x7f
		},
		// SAA5243E - Spanish
		{
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
			0x20, 0x21, 0x22, 0xb3, 0xb4, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
			0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
			0xb0, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
			0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
			0xb1, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
			0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0xb2, 0xb8, 0xb9, 0xba, 0x7f
		},
		// SAA5243E - Swedish
		{
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
			0x20, 0x21, 0x22, 0xc3, 0xc4, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
			0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
			0xc0, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
			0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
			0xc1, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
			0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0xc2, 0xc8, 0xc9, 0xca, 0x7f
		}
	};

	// decode national option characters
	if (!BIT(data, 7))
		data = nc_char_matrix[1][data]; // TODO: select character set from page header control bits
	else
		data &= 0x7f;

	return m_char_rom[(data * 10) + row];
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

	if (m_curr_chartype == ALPHANUMERIC || !BIT(data, 5))
		m_char_data = get_rom_data(data, ra);
	else
		m_char_data = get_gfx_data(data, ra, (m_curr_chartype == SEPARATED));
}


//-------------------------------------------------
//  vcs_w - video composite sync
//-------------------------------------------------

void saa5240_device::vcs_w(int state)
{
	if (state)
	{
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

void saa5240_device::f6_w(int state)
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
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int sy = y / 10;
		int x = screen.visible_area().left();

		m_ra = y % 10;

		vcs_w(1);
		vcs_w(0);

		for (int sx = 0; sx < 40; sx++)
		{
			// get display data
			get_character_data(m_ram[ram_addr(m_register[4], sy, sx)]);

			for (int bit = 0; bit < 12; bit++)
			{
				f6_w(1);
				f6_w(0);

				// cursor on at active row/column
				if (BIT(m_register[7], 6) && (sy == m_register[9]) && (sx == m_register[10]))
					bitmap.pix(y, x++) = rgb_t::white();
				else
					bitmap.pix(y, x++) = rgb_t(pal1bit(BIT(get_rgb(), 0)), pal1bit(BIT(get_rgb(), 1)), pal1bit(BIT(get_rgb(), 2)));
			}
		}
	}

	return 0;
}
