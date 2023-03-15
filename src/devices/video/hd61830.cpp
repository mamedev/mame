// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Hitachi HD61830 LCD Timing Controller emulation

**********************************************************************/

#include "emu.h"
#include "hd61830.h"

#include "screen.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(HD61830, hd61830_device, "hd61830", "Hitachi HD61830B LCD Controller")


// default address map
void hd61830_device::hd61830(address_map &map)
{
	if (!has_configured_map(0))
		map(0x0000, 0xffff).ram();
}


// internal character generator ROM
ROM_START( hd61830 )
	ROM_REGION( 0x5c0, "hd61830", 0 ) // internal 7360-bit chargen ROM
	ROM_LOAD( "hd61830.bin", 0x000, 0x5c0, BAD_DUMP CRC(06a934da) SHA1(bf3f074db5dc92e6f530cb18d6c013563099a87d) ) // typed in from manual
ROM_END


//-------------------------------------------------
//  device_rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *hd61830_device::device_rom_region() const
{
	return ROM_NAME(hd61830);
}



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

static constexpr int CYCLES[] =
{
	4, 4, 4, 4, 4, -1, -1, -1, 4, 4, 4, 4, 6, 6, 36, 36
};

static constexpr int MODE_EXTERNAL_CG      = 0x01;
static constexpr int MODE_GRAPHIC          = 0x02;
static constexpr int MODE_CURSOR           = 0x04;
static constexpr int MODE_BLINK            = 0x08;
static constexpr int MODE_MASTER           = 0x10;
static constexpr int MODE_DISPLAY_ON       = 0x20;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hd61830_device - constructor
//-------------------------------------------------

hd61830_device::hd61830_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HD61830, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_read_rd(*this),
	m_bf(false),
	m_cac(0),
	m_blink(0),
	m_cursor(0),
	m_space_config("videoram", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(hd61830_device::hd61830), this)),
	m_char_rom(*this, "hd61830")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd61830_device::device_start()
{
	// allocate timers
	m_busy_timer = timer_alloc(FUNC(hd61830_device::clear_busy_flag), this);

	// resolve callbacks
	m_read_rd.resolve_safe(0);

	// register for state saving
	save_item(NAME(m_bf));
	save_item(NAME(m_ir));
	save_item(NAME(m_mcr));
	save_item(NAME(m_dor));
	save_item(NAME(m_cac));
	save_item(NAME(m_dsa));
	save_item(NAME(m_vp));
	save_item(NAME(m_hp));
	save_item(NAME(m_hn));
	save_item(NAME(m_nx));
	save_item(NAME(m_cp));
	save_item(NAME(m_blink));
	save_item(NAME(m_cursor));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hd61830_device::device_reset()
{
	// display off, slave mode
	m_mcr &= ~(MODE_MASTER | MODE_DISPLAY_ON);

	// default horizontal pitch
	m_hp = 6;
}


//-------------------------------------------------
//  clear_busy_flag -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(hd61830_device::clear_busy_flag)
{
	m_bf = false;
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector hd61830_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


/*-------------------------------------------------
    set_busy_flag - set busy flag and arm timer
                    to clear it later
-------------------------------------------------*/

void hd61830_device::set_busy_flag()
{
	// set busy flag
	//m_bf = true; TODO figure out correct timing

	// adjust busy timer
	m_busy_timer->adjust(clocks_to_attotime(CYCLES[m_ir]));
}


//-------------------------------------------------
//  status_r - status register read
//-------------------------------------------------

uint8_t hd61830_device::status_r()
{
	LOG("HD61830 Status Read: %s\n", m_bf ? "busy" : "ready");

	return m_bf ? 0x80 : 0;
}


//-------------------------------------------------
//  control_w - instruction register write
//-------------------------------------------------

void hd61830_device::control_w(uint8_t data)
{
	m_ir = data;
}


//-------------------------------------------------
//  data_r - data register read
//-------------------------------------------------

uint8_t hd61830_device::data_r()
{
	uint8_t data = m_dor;

	LOG("HD61830 Display Data Read %02x\n", m_dor);

	m_dor = readbyte(m_cac);

	m_cac++;

	return data;
}


//-------------------------------------------------
//  data_w - data register write
//-------------------------------------------------

void hd61830_device::data_w(uint8_t data)
{
	if (m_bf)
	{
		logerror("HD61830 Ignoring data write %02x due to business\n", data);
		return;
	}

	switch (m_ir)
	{
	case INSTRUCTION_MODE_CONTROL:
		m_mcr = data;

		LOG("HD61830 %s CG\n", (data & MODE_EXTERNAL_CG) ? "External" : "Internal");
		LOG("HD61830 %s Display Mode\n", (data & MODE_GRAPHIC) ? "Graphic" : "Character");
		LOG("HD61830 %s Mode\n", (data & MODE_MASTER) ? "Master" : "Slave");
		LOG("HD61830 Cursor %s\n", (data & MODE_CURSOR) ? "On" : "Off");
		LOG("HD61830 Blink %s\n", (data & MODE_BLINK) ? "On" : "Off");
		LOG("HD61830 Display %s\n", (data & MODE_DISPLAY_ON) ? "On" : "Off");
		break;

	case INSTRUCTION_CHARACTER_PITCH:
		m_hp = (data & 0x07) + 1;
		m_vp = (data >> 4) + 1;

		LOG("HD61830 Horizontal Character Pitch: %u\n", m_hp);
		LOG("HD61830 Vertical Character Pitch: %u\n", m_vp);
		break;

	case INSTRUCTION_NUMBER_OF_CHARACTERS:
		m_hn = (data & 0x7f) + 1;
		m_hn = (m_hn % 2 == 0) ? m_hn : (m_hn + 1);

		LOG("HD61830 Number of Characters: %u\n", m_hn);
		break;

	case INSTRUCTION_NUMBER_OF_TIME_DIVISIONS:
		m_nx = (data & 0x7f) + 1;

		LOG("HD61830 Number of Time Divisions: %u\n", m_nx);
		break;

	case INSTRUCTION_CURSOR_POSITION:
		m_cp = (data & 0x0f) + 1;

		LOG("HD61830 Cursor Position: %u\n", m_cp);
		break;

	case INSTRUCTION_DISPLAY_START_LOW:
		m_dsa = (m_dsa & 0xff00) | data;

		LOG("HD61830 Display Start Address Low %04x\n", m_dsa);
		break;

	case INSTRUCTION_DISPLAY_START_HIGH:
		m_dsa = (data << 8) | (m_dsa & 0xff);

		LOG("HD61830 Display Start Address High %04x\n", m_dsa);
		break;

	case INSTRUCTION_CURSOR_ADDRESS_LOW:
		if (BIT(m_cac, 7) && !BIT(data, 7))
		{
			m_cac = (((m_cac >> 8) + 1) << 8) | data;
		}
		else
		{
			m_cac = (m_cac & 0xff00) | data;
		}

		LOG("HD61830 Cursor Address Low %02x: %04x\n", data, m_cac);
		break;

	case INSTRUCTION_CURSOR_ADDRESS_HIGH:
		m_cac = (data << 8) | (m_cac & 0xff);

		LOG("HD61830 Cursor Address High %02x: %04x\n", data, m_cac);
		break;

	case INSTRUCTION_DISPLAY_DATA_WRITE:
		writebyte(m_cac, data);

		LOG("HD61830 Display Data Write %02x -> %04x row %u col %u\n", data, m_cac, m_cac / 40, m_cac % 40);

		m_cac++;
		break;

	case INSTRUCTION_CLEAR_BIT:
		{
		int bit = data & 0x07;
		uint8_t md = readbyte(m_cac);

		md &= ~(1 << bit);

		LOG("HD61830 Clear Bit %u at %04x\n", bit + 1, m_cac);

		writebyte(m_cac, md);

		m_cac++;
		}
		break;

	case INSTRUCTION_SET_BIT:
		{
		int bit = data & 0x07;
		uint8_t md = readbyte(m_cac);

		md |= 1 << bit;

		LOG("HD61830 Set Bit %u at %04x\n", bit + 1, m_cac);

		writebyte(m_cac, md);

		m_cac++;
		}
		break;

	default:
		logerror("HD61830 Illegal Instruction %02x!\n", m_ir);
		return;
	}

	// burn cycles
	set_busy_flag();
}


//-------------------------------------------------
//  draw_scanline - draw one graphics scanline
//-------------------------------------------------

uint16_t hd61830_device::draw_scanline(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, uint16_t ra)
{
	for (int sx = 0; sx < m_hn; sx+=2)
	{
		uint8_t data1 = readbyte(ra++);
		uint8_t data2 = readbyte(ra++);

		for (int x = 0; x < m_hp; x++)
		{
			if(y >= 0 && y < bitmap.height())
			{
				if(((sx * m_hp) + x) >= 0 && ((sx * m_hp) + x) < bitmap.width())
					bitmap.pix(y, (sx * m_hp) + x) = BIT(data1, x);
				if(((sx * m_hp) + x + m_hp) >= 0 && ((sx * m_hp) + x + m_hp) < bitmap.width())
					bitmap.pix(y, (sx * m_hp) + x + m_hp) = BIT(data2, x);
			}
		}
	}
	return ra;
}


//-------------------------------------------------
//  update_graphics - draw graphics mode screen
//-------------------------------------------------

void hd61830_device::update_graphics(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t rac1 = m_dsa;
	uint16_t rac2 = rac1 + (m_nx * m_hn);
	for (int y = 0; y < m_nx; y++)
	{
		/* draw upper half scanline */
		rac1 = draw_scanline(bitmap, cliprect, y, rac1);

		/* draw lower half scanline */
		rac2 = draw_scanline(bitmap, cliprect, y + m_nx, rac2);
	}
}


//-------------------------------------------------
//  draw_char - draw a char
//-------------------------------------------------

void hd61830_device::draw_char(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t ma, int x, int y, uint8_t md)
{
	for (int cl = 0; cl < m_vp; cl++)
	{
		for (int cr = 0; cr < m_hp; cr++)
		{
			int sy = y * m_vp + cl;
			int sx = x * m_hp + cr;
			uint8_t data;

			if (m_mcr & MODE_EXTERNAL_CG)
			{
				data = m_read_rd((cl << 12) | md);
			}
			else
			{
				uint16_t addr = 0;

				if (md >= 0x20 && md < 0x80 && cl < 7)
				{
					// 5x7 characters 0x20..0x7f
					addr = (md - 0x20) * 7 + cl;
				}
				else if (md >= 0xa0 && md < 0xe0 && cl < 7)
				{
					// 5x7 characters 0xa0..0xdf
					addr = 96*7 + (md - 0xa0) * 7 + cl;
				}
				else if (md >= 0xe0 && cl < 11)
				{
					// 5x11 characters 0xe0..0xff
					addr = 160*7 + (md - 0xe0) * 11 + cl;
				}

				data = m_char_rom[addr];
			}

			int cursor = m_mcr & MODE_CURSOR;
			int blink = m_mcr & MODE_BLINK;

			// cursor off
			int pixel = BIT(data, cr);

			if (blink && (ma == m_cac))
			{
				// cursor off, character blink
				if (!cursor)
					pixel = m_cursor ? 1 : pixel;

				// cursor blink
				if (cursor && (cl == m_cp))
					pixel = m_cursor ? 1 : 0;
			}
			else
			{
				// cursor on
				if (cursor && (cl == m_cp))
					pixel = m_cursor ? 1 : 0;
			}

			if (sy < screen().height() && sx < screen().width())
				bitmap.pix(sy, sx) = pixel;
		}
	}
}


//-------------------------------------------------
//  update_text - draw text mode screen
//-------------------------------------------------

void hd61830_device::update_text(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t ma = 0;
	int rows = m_nx / m_vp;
	uint16_t rac1 = m_dsa & 0xfff;
	uint16_t rac2 = rac1 + (rows * m_hn);
	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < m_hn; x+=2)
		{
			uint8_t md1 = readbyte(rac1);
			uint8_t md2 = readbyte(rac1+1);

			draw_char(bitmap, cliprect, ma, x, y, md1);
			draw_char(bitmap, cliprect, ma+1, x+1, y, md2);

			md1 = readbyte(rac2);
			md2 = readbyte(rac2+1);

			draw_char(bitmap, cliprect, ma + (rows * m_hn), x, y + rows, md1);
			draw_char(bitmap, cliprect, ma+1 + (rows * m_hn), x+1, y + rows, md2);

			ma+=2;
			rac1+=2;
			rac2+=2;
		}
	}
}


//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

uint32_t hd61830_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_mcr & MODE_DISPLAY_ON)
	{
		if (m_mcr & MODE_GRAPHIC)
		{
			update_graphics(bitmap, cliprect);
		}
		else
		{
			update_text(bitmap, cliprect);
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}

	m_blink++;

	if (m_blink == 0x20)
	{
		m_blink = 0;
		m_cursor = !m_cursor;
	}
	return 0;
}
