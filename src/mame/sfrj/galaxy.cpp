// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha, Miodrag Milanovic
/***************************************************************************
Galaksija driver by Krzysztof Strzecha and Miodrag Milanovic

22/05/2008 Tape support added (Miodrag Milanovic)
21/05/2008 Galaksija plus initial support (Miodrag Milanovic)
20/05/2008 Added real video implementation (Miodrag Milanovic)
18/04/2005 Possibilty to disable ROM 2. 2k, 22k, 38k and 54k memory
       configurations added.
13/03/2005 Memory mapping improved. Palette corrected. Supprort for newer
           version of snapshots added. Lot of cleanups. Keyboard mapping
           corrected.
19/09/2002 malloc() replaced by image_malloc().
15/09/2002 Snapshot loading fixed. Code cleanup.
31/01/2001 Snapshot loading corrected.
09/01/2001 Fast mode implemented (many thanks to Kevin Thacker).
07/01/2001 Keyboard corrected (still some keys unknown).
           Horizontal screen positioning in video subsystem added.
05/01/2001 Keyboard implemented (some keys unknown).
03/01/2001 Snapshot loading added.
01/01/2001 Preliminary driver.

ToDo:
- pacmanp not showing its hi-res graphics - get black screen
- is the hack in the video still needed? commenting it out made no difference.

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "formats/gtp_cas.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

namespace {

class galaxy_state : public driver_device
{
public:
	galaxy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_cassette(*this, "cassette")
		, m_ram(*this, RAM_TAG)
		, m_p_chargen(*this, "chargen")
		, m_io_keyboard(*this, "LINE%u", 0U)
	{ }

	void galaxy(machine_config &config);
	void galaxyp(machine_config &config);

	void init_galaxy();

private:
	uint8_t keyboard_r(offs_t offset);
	void latch_w(uint8_t data);
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(gal_video);
	IRQ_CALLBACK_MEMBER(irq_callback);
	void set_timer();
	void setup_snapshot (const uint8_t * data, uint32_t size);
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);
	void galaxy_mem(address_map &map) ATTR_COLD;
	void galaxyp_io(address_map &map) ATTR_COLD;
	void galaxyp_mem(address_map &map) ATTR_COLD;

	int m_interrupts_enabled = 0;
	uint8_t m_latch_value = 0U;
	uint32_t m_gal_cnt = 0U;
	uint8_t m_code = 0U;
	uint8_t m_first = 0U;
	uint32_t m_start_addr = 0U;
	emu_timer *m_gal_video_timer = nullptr;
	bitmap_ind16 m_bitmap{};

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<cassette_image_device> m_cassette;
	optional_device<ram_device> m_ram;
	required_region_ptr<u8> m_p_chargen;
	required_ioport_array<8> m_io_keyboard;
};

/***************************************************************************
  I/O devices
***************************************************************************/

uint8_t galaxy_state::keyboard_r(offs_t offset)
{
	if (offset == 0)
	{
		double level = m_cassette->input();
		return (level >  -0.1) ? 0xff : 0xfe;
	}
	else
		return m_io_keyboard[(offset>>3) & 0x07]->read() & (0x01<<(offset & 0x07)) ? 0xfe : 0xff;
}

void galaxy_state::latch_w(uint8_t data)
{
	double val = (BIT(data,6) ^ BIT(data,2)) ? 0 : BIT(data,6) ? -1.0f : +1.0f;
	m_latch_value = data;
	m_cassette->output(val);
}

/***************************************************************************
  Interrupts
***************************************************************************/

IRQ_CALLBACK_MEMBER(galaxy_state::irq_callback)
{
	set_timer();
	m_interrupts_enabled = true;
	return 0xff;
}

/***************************************************************************
  Snapshot files (GAL)
***************************************************************************/

#define GALAXY_SNAPSHOT_V1_SIZE 8268
#define GALAXY_SNAPSHOT_V2_SIZE 8244

void galaxy_state::setup_snapshot(const uint8_t * data, uint32_t size)
{
	switch (size)
	{
		case GALAXY_SNAPSHOT_V1_SIZE:
			m_maincpu->set_state_int(Z80_AF,   data[0x00] | data[0x01] << 8);
			m_maincpu->set_state_int(Z80_BC,   data[0x04] | data[0x05] << 8);
			m_maincpu->set_state_int(Z80_DE,   data[0x08] | data[0x09] << 8);
			m_maincpu->set_state_int(Z80_HL,   data[0x0c] | data[0x0d] << 8);
			m_maincpu->set_state_int(Z80_IX,   data[0x10] | data[0x11] << 8);
			m_maincpu->set_state_int(Z80_IY,   data[0x14] | data[0x15] << 8);
			m_maincpu->set_state_int(Z80_PC,   data[0x18] | data[0x19] << 8);
			m_maincpu->set_state_int(Z80_SP,   data[0x1c] | data[0x1d] << 8);
			m_maincpu->set_state_int(Z80_AF2,  data[0x20] | data[0x21] << 8);
			m_maincpu->set_state_int(Z80_BC2,  data[0x24] | data[0x25] << 8);
			m_maincpu->set_state_int(Z80_DE2,  data[0x28] | data[0x29] << 8);
			m_maincpu->set_state_int(Z80_HL2,  data[0x2c] | data[0x2d] << 8);
			m_maincpu->set_state_int(Z80_IFF1, data[0x30]);
			m_maincpu->set_state_int(Z80_IFF2, data[0x34]);
			m_maincpu->set_state_int(Z80_HALT, data[0x38]);
			m_maincpu->set_state_int(Z80_IM,   data[0x3c]);
			m_maincpu->set_state_int(Z80_I,    data[0x40]);
			m_maincpu->set_state_int(Z80_R,    (data[0x44] & 0x7f) | (data[0x48] & 0x80));

			memcpy (m_ram->pointer(), data + 0x084c, (m_ram->size() < 0x1800) ? m_ram->size() : 0x1800);

			break;
		case GALAXY_SNAPSHOT_V2_SIZE:
			m_maincpu->set_state_int(Z80_AF,   data[0x00] | data[0x01] << 8);
			m_maincpu->set_state_int(Z80_BC,   data[0x02] | data[0x03] << 8);
			m_maincpu->set_state_int(Z80_DE,   data[0x04] | data[0x05] << 8);
			m_maincpu->set_state_int(Z80_HL,   data[0x06] | data[0x07] << 8);
			m_maincpu->set_state_int(Z80_IX,   data[0x08] | data[0x09] << 8);
			m_maincpu->set_state_int(Z80_IY,   data[0x0a] | data[0x0b] << 8);
			m_maincpu->set_state_int(Z80_PC,   data[0x0c] | data[0x0d] << 8);
			m_maincpu->set_state_int(Z80_SP,   data[0x0e] | data[0x0f] << 8);
			m_maincpu->set_state_int(Z80_AF2,  data[0x10] | data[0x11] << 8);
			m_maincpu->set_state_int(Z80_BC2,  data[0x12] | data[0x13] << 8);
			m_maincpu->set_state_int(Z80_DE2,  data[0x14] | data[0x15] << 8);
			m_maincpu->set_state_int(Z80_HL2,  data[0x16] | data[0x17] << 8);

			m_maincpu->set_state_int(Z80_IFF1, data[0x18] & 0x01);
			m_maincpu->set_state_int(Z80_IFF2, (uint64_t)0);

			m_maincpu->set_state_int(Z80_HALT, (uint64_t)0);

			m_maincpu->set_state_int(Z80_IM,   (data[0x18] >> 1) & 0x03);

			m_maincpu->set_state_int(Z80_I,    data[0x19]);
			m_maincpu->set_state_int(Z80_R,    data[0x1a]);

			memcpy (m_ram->pointer(), data + 0x0834, (m_ram->size() < 0x1800) ? m_ram->size() : 0x1800);

			break;
	}

	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

SNAPSHOT_LOAD_MEMBER(galaxy_state::snapshot_cb)
{
	uint32_t const snapshot_size = image.length();
	switch (snapshot_size)
	{
		case GALAXY_SNAPSHOT_V1_SIZE:
		case GALAXY_SNAPSHOT_V2_SIZE:
			break;
		default:
			return std::make_pair(
					image_error::INVALIDLENGTH,
					util::string_format("Unsupported image size (must be %u or %u bytes)", GALAXY_SNAPSHOT_V1_SIZE, GALAXY_SNAPSHOT_V2_SIZE));
	}

	std::vector<uint8_t> snapshot_data(snapshot_size);
	image.fread(&snapshot_data[0], snapshot_size);

	setup_snapshot(&snapshot_data[0], snapshot_size);

	return std::make_pair(std::error_condition(), std::string());
}

/***************************************************************************
  Driver Initialization
***************************************************************************/

void galaxy_state::init_galaxy()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_ram( 0x2800, 0x2800 + m_ram->size() - 1, m_ram->pointer());

	if (m_ram->size() < (6 + 48) * 1024)
		space.nop_readwrite( 0x2800 + m_ram->size(), 0xffff);
}

/***************************************************************************
  Machine Initialization
***************************************************************************/

void galaxy_state::machine_reset()
{
	m_interrupts_enabled = true;
}

TIMER_CALLBACK_MEMBER(galaxy_state::gal_video)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	if (m_interrupts_enabled == true)
	{
		uint8_t dat = BIT(m_latch_value, 2, 4);
		if ((m_gal_cnt >= 48 * 2) && (m_gal_cnt < 48 * 210))  // display on screen just m_first 208 lines
		{
			uint16_t addr = (m_maincpu->state_int(Z80_I) << 8) | m_maincpu->state_int(Z80_R) | (~m_latch_value & 0x80);
			if (!BIT(m_latch_value, 1)) // bit 2 latch represents mode
			{
				// Text mode
				if (m_first == 0 && (m_maincpu->state_int(Z80_R) & 0x1f) == 0)
				{
					// Due to a fact that on real processor latch value is set at
					// the end of last cycle we need to skip display of double
					// m_first char in each row
					m_code = 0x00;
					m_first = 1;
				}
				else
				{
					m_code = space.read_byte(addr) & 0xbf;
					m_code += (m_code & 0x80) >> 1;
					m_code = m_p_chargen[(m_code & 0x7f) +(dat << 7 )] ^ 0xff;
					m_first = 0;
				}
				int y = m_gal_cnt / 48 - 2;
				int x = (m_gal_cnt % 48) * 8;

				m_bitmap.pix(y, x++ ) = BIT(m_code, 0);
				m_bitmap.pix(y, x++ ) = BIT(m_code, 1);
				m_bitmap.pix(y, x++ ) = BIT(m_code, 2);
				m_bitmap.pix(y, x++ ) = BIT(m_code, 3);
				m_bitmap.pix(y, x++ ) = BIT(m_code, 4);
				m_bitmap.pix(y, x++ ) = BIT(m_code, 5);
				m_bitmap.pix(y, x++ ) = BIT(m_code, 6);
				m_bitmap.pix(y, x   ) = BIT(m_code, 7);
			}
			else
			{ // Graphics mode
				if (m_first < 4 && (m_maincpu->state_int(Z80_R) & 0x1f) == 0)
				{
					// Due to a fact that on real processor latch value is set at
					// the end of last cycle we need to skip display of 4 times
					// m_first char in each row
					m_code = 0x00;
					m_first++;
				}
				else
				{
					m_code = space.read_byte(addr) ^ 0xff;
					m_first = 0;
				}
				int y = m_gal_cnt / 48 - 2;
				int x = (m_gal_cnt % 48) * 8;

				/* hack - until calc of R is fixed in Z80 */
				if (x == 11 * 8 && y == 0)
					m_start_addr = addr;

				if ((x / 8 >= 11) && (x / 8 < 44))
					m_code = space.read_byte(m_start_addr + y * 32 + (m_gal_cnt % 48) - 11) ^ 0xff;
				else
					m_code = 0x00;
				/* end of hack */

				m_bitmap.pix(y, x++ ) = BIT(m_code, 0);
				m_bitmap.pix(y, x++ ) = BIT(m_code, 1);
				m_bitmap.pix(y, x++ ) = BIT(m_code, 2);
				m_bitmap.pix(y, x++ ) = BIT(m_code, 3);
				m_bitmap.pix(y, x++ ) = BIT(m_code, 4);
				m_bitmap.pix(y, x++ ) = BIT(m_code, 5);
				m_bitmap.pix(y, x++ ) = BIT(m_code, 6);
				m_bitmap.pix(y, x   ) = BIT(m_code, 7);
			}
		}
		m_gal_cnt++;
	}
}

void galaxy_state::set_timer()
{
	m_gal_cnt = 0;
	m_gal_video_timer->adjust(attotime::zero, 0, attotime::from_hz(6144000 / 8));
}

void galaxy_state::machine_start()
{
	m_gal_cnt = 0;

	m_gal_video_timer = timer_alloc(FUNC(galaxy_state::gal_video), this);
	m_gal_video_timer->adjust(attotime::zero, 0, attotime::never);

	m_screen->register_screen_bitmap(m_bitmap);

	save_item(NAME(m_interrupts_enabled));
	save_item(NAME(m_latch_value));
	save_item(NAME(m_gal_cnt));
	save_item(NAME(m_code));
	save_item(NAME(m_first));
	save_item(NAME(m_start_addr));
}

uint32_t galaxy_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_gal_video_timer->adjust(attotime::zero, 0, attotime::never);
	if (m_interrupts_enabled == false)
	{
		const rectangle black_area(0, 384 - 1, 0, 208 - 1);
		m_bitmap.fill(0, black_area);
	}
	m_interrupts_enabled = false;
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

void galaxy_state::galaxyp_io(address_map &map)
{
	map.global_mask(0x01);
	map.unmap_value_high();
	map(0x00, 0x00).w("ay8910", FUNC(ay8910_device::address_w));
	map(0x01, 0x01).w("ay8910", FUNC(ay8910_device::data_w));
}


void galaxy_state::galaxy_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x2037).mirror(0x07c0).r(FUNC(galaxy_state::keyboard_r));
	map(0x2038, 0x203f).mirror(0x07c0).w(FUNC(galaxy_state::latch_w));
	// see init_galaxy for ram placement
}

void galaxy_state::galaxyp_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom(); // ROM A
	map(0x1000, 0x1fff).rom(); // ROM B
	map(0x2000, 0x2037).mirror(0x07c0).r(FUNC(galaxy_state::keyboard_r));
	map(0x2038, 0x203f).mirror(0x07c0).w(FUNC(galaxy_state::latch_w));
	map(0x2800, 0xdfff).ram();
	map(0xe000, 0xefff).rom().region("maincpu",0x2000); // ROM C
	map(0xf000, 0xffff).rom().region("maincpu",0x3000); // ROM D
}

/* 2008-05 FP:
Small note about natural keyboard support. Currently:
- "List" is mapped to 'ESC'
- "Break" is mapped to 'F1'
- "Repeat" is mapped to 'F2'                           */

static INPUT_PORTS_START (galaxy)
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)       PORT_CHAR('A')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)       PORT_CHAR('B')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)       PORT_CHAR('C')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)       PORT_CHAR('D')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)       PORT_CHAR('E')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)       PORT_CHAR('F')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)       PORT_CHAR('G')

	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)       PORT_CHAR('H')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)       PORT_CHAR('I')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)       PORT_CHAR('J')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)       PORT_CHAR('K')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)       PORT_CHAR('L')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)       PORT_CHAR('M')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)       PORT_CHAR('N')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)       PORT_CHAR('O')

	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)       PORT_CHAR('P')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)       PORT_CHAR('Q')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)       PORT_CHAR('R')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)       PORT_CHAR('S')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)       PORT_CHAR('T')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)       PORT_CHAR('U')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)       PORT_CHAR('V')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)       PORT_CHAR('W')

	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)       PORT_CHAR('X')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)       PORT_CHAR('Y')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)       PORT_CHAR('Z')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)      PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)    PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)    PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)   PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)   PORT_CHAR(' ')

	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)       PORT_CHAR('0') PORT_CHAR('_')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)       PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)       PORT_CHAR('2') PORT_CHAR('"')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)       PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)       PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)       PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)       PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)       PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)       PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)       PORT_CHAR('9') PORT_CHAR(')')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)   PORT_CHAR(';') PORT_CHAR('+')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)   PORT_CHAR(':') PORT_CHAR('*')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)   PORT_CHAR(',') PORT_CHAR('<')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)  PORT_CHAR('=') PORT_CHAR('-')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)    PORT_CHAR('.') PORT_CHAR('>')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)   PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)   PORT_CHAR(13)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_PAUSE) PORT_CHAR(UCHAR_MAMEKEY(F1))
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Repeat") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(F2))
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("List") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE7")
INPUT_PORTS_END

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0, 1*128*8, 2*128*8, 3*128*8, 4*128*8, 5*128*8, 6*128*8, 7*128*8, 8*128*8, 9*128*8, 10*128*8, 11*128*8, 12*128*8, 13*128*8, 14*128*8, 15*128*8 },
	8                   /* every char takes 1 x 16 bytes */
};

static GFXDECODE_START( gfx_galaxy )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END


void galaxy_state::galaxy(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 6'144'000 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxy_state::galaxy_mem);
	m_maincpu->set_vblank_int("screen", FUNC(galaxy_state::irq0_line_hold));
	m_maincpu->set_irq_acknowledge_callback(FUNC(galaxy_state::irq_callback));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_palette("palette");
	m_screen->set_size(384, 212);
	m_screen->set_visarea(0, 384-1, 0, 208-1);
	m_screen->set_screen_update(FUNC(galaxy_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_galaxy);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* snapshot */
	SNAPSHOT(config, "snapshot", "gal").set_load_callback(FUNC(galaxy_state::snapshot_cb));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(gtp_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("galaxy_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("galaxy");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("6K").set_extra_options("2K,22K,38K,54K");
}

void galaxy_state::galaxyp(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 6'144'000 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxy_state::galaxyp_mem);
	m_maincpu->set_addrmap(AS_IO, &galaxy_state::galaxyp_io);
	m_maincpu->set_vblank_int("screen", FUNC(galaxy_state::irq0_line_hold));
	m_maincpu->set_irq_acknowledge_callback(FUNC(galaxy_state::irq_callback));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_palette("palette");
	m_screen->set_size(384, 208);
	m_screen->set_visarea(0, 384-1, 0, 208-1);
	m_screen->set_screen_update(FUNC(galaxy_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_galaxy);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* snapshot */
	SNAPSHOT(config, "snapshot", "gal").set_load_callback(FUNC(galaxy_state::snapshot_cb));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay(AY8910(config, "ay8910", 6'144'000 / 4));
	ay.add_route(ALL_OUTPUTS, "mono", 0.50);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(gtp_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("galaxy_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("galaxy");
}

// Original Galaksija kit came with v28 version of ROM A
// at end of 1984 ROM B appeared and people patched their ROM A v28
// to make it auto boot ROM B
// later official v29 was made to auto boot ROM B
// chargen also include prompt char with logo of Mipro, Voja Antonic company
// Elektronika inzinjering have different chargen, modified to include logo char
ROM_START (galaxy)
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("v29")
	ROM_SYSTEM_BIOS( 0, "v29",  "ROM A v29 + ROM B" )
	ROMX_LOAD( "rom_a_v29.dd8", 0x0000, 0x1000, CRC(e6853bc1) SHA1(aea7a4c0c7ffe1f212f7b9faecfd728862ac6904), ROM_BIOS(0) )
	ROMX_LOAD( "rom_b_v5.dd9",  0x1000, 0x1000, CRC(5dc5a100) SHA1(5d5ab4313a2d0effe7572bb129193b64cab002c1), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v29ei","ROM A v29 + ROM B Elektronika inzinjering" )
	ROMX_LOAD( "rom_a_v29.dd8", 0x0000, 0x1000, CRC(e6853bc1) SHA1(aea7a4c0c7ffe1f212f7b9faecfd728862ac6904), ROM_BIOS(1) )
	ROMX_LOAD( "rom_b_v5.dd9",  0x1000, 0x1000, CRC(5dc5a100) SHA1(5d5ab4313a2d0effe7572bb129193b64cab002c1), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v28p", "ROM A v28 + ROM B auto" )
	ROMX_LOAD( "rom_a_v28p.dd8",0x0000, 0x1000, CRC(dc970a32) SHA1(dfc92163654a756b70f5a446daf49d7534f4c739), ROM_BIOS(2) )
	ROMX_LOAD( "rom_b_v5.dd9",  0x1000, 0x1000, CRC(5dc5a100) SHA1(5d5ab4313a2d0effe7572bb129193b64cab002c1), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v28b", "ROM A v28 + ROM B" )
	ROMX_LOAD( "rom_a_v28.dd8", 0x0000, 0x1000, CRC(365f3e24) SHA1(ffc6bf2ec09eabdad76604a63f5dd697c30c4358), ROM_BIOS(3) )
	ROMX_LOAD( "rom_b_v5.dd9",  0x1000, 0x1000, CRC(5dc5a100) SHA1(5d5ab4313a2d0effe7572bb129193b64cab002c1), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v28a", "ROM A v28 only" )
	ROMX_LOAD( "rom_a_v28.dd8", 0x0000, 0x1000, CRC(365f3e24) SHA1(ffc6bf2ec09eabdad76604a63f5dd697c30c4358), ROM_BIOS(4) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROMX_LOAD( "chr_mipro.dd3", 0x0000, 0x0800, CRC(fd77b6d2) SHA1(cc73b0386b84383b4841e58a1c328cb67b0121d8), ROM_BIOS(0) )
	ROMX_LOAD( "chr_eling.dd3", 0x0000, 0x0800, CRC(5c3b5bb5) SHA1(19429a61dc5e55ddec3242a8f695e06dd7961f88), ROM_BIOS(1) )
	ROMX_LOAD( "chr_mipro.dd3", 0x0000, 0x0800, CRC(fd77b6d2) SHA1(cc73b0386b84383b4841e58a1c328cb67b0121d8), ROM_BIOS(2) )
	ROMX_LOAD( "chr_mipro.dd3", 0x0000, 0x0800, CRC(fd77b6d2) SHA1(cc73b0386b84383b4841e58a1c328cb67b0121d8), ROM_BIOS(3) )
	ROMX_LOAD( "chr_mipro.dd3", 0x0000, 0x0800, CRC(fd77b6d2) SHA1(cc73b0386b84383b4841e58a1c328cb67b0121d8), ROM_BIOS(4) )
ROM_END

// Galaksija plus was hardware modification of original, and could not be considered extension board since it
// was not using expansion port and was also requiring changes on main computer board in order to work.
// It was on separate board and it also included RAM expansion and AY sound generator.
// Instuctions to build also included how to patch ROM A to make it auto boot ROM C.
ROM_START (galaxyp)
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("v29c")
	ROM_SYSTEM_BIOS( 0, "v29c", "ROM A v29 boot ROM C" )
	ROMX_LOAD( "rom_a_v29c.dd8", 0x0000, 0x1000, CRC(5cb8fb2a) SHA1(fdddae2b08d0dc81eb6191a92e60ac411d8150e9), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v29",  "ROM A v29" )
	ROMX_LOAD( "rom_a_v29.dd8",  0x0000, 0x1000, CRC(e6853bc1) SHA1(aea7a4c0c7ffe1f212f7b9faecfd728862ac6904), ROM_BIOS(1) )

	ROM_LOAD( "rom_b_v5.dd9",    0x1000, 0x1000, CRC(5dc5a100) SHA1(5d5ab4313a2d0effe7572bb129193b64cab002c1) )
	ROM_LOAD( "rom_c.bin",       0x2000, 0x1000, CRC(d4cfab14) SHA1(b507b9026844eeb757547679907394aa42055eee) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "chr_mipro.dd3", 0x0000, 0x0800, CRC(fd77b6d2) SHA1(cc73b0386b84383b4841e58a1c328cb67b0121d8) )
ROM_END

} // Anonymous namespace

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT          COMPANY                                   FULLNAME */
COMP( 1983, galaxy,  0,      0,      galaxy,  galaxy,  galaxy_state, init_galaxy,  "Voja Antonic / Elektronika inzenjering", "Galaksija",      MACHINE_SUPPORTS_SAVE )
COMP( 1985, galaxyp, galaxy, 0,      galaxyp, galaxy,  galaxy_state, empty_init,   "Nenad Dunjic",                           "Galaksija plus", MACHINE_SUPPORTS_SAVE )
