// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

SAPI-1 driver by Miodrag Milanovic

2008-09-09 Preliminary driver.

2010-12-07 Added some code to allow sapizps3 to read its rom.
With no available docs, the i/o ports are a guess. The ram
allocation is based on the actions of the various bios roms.
Port 25 is used as a jump vector. in a,(25); ld l,a; jp(hl).

2012-04-19 Connected sapizps3 to a terminal. It is trying to
load a 128-byte boot sector from a floppy disk.
Modernised driver.
Connected sapizps2 to ascii keyboard. System is now usable.
According to wikipedia, sapi1 & 2 have cassette facility,
while sapi3 uses 8 inch floppy disk.

Cassette:
2019-07-26 Cassette implemented @ 1200 baud.
- sapi1 -bios 1 and sapizps2 -bios 2 have a common system allowing
  files to be shared among them. (working)
- sapi1 -bios 0 uses its own system on ports 40-43, not compatible
  with the above. (working)
- sapizps2 -bios 0 and -bios 1: unknown how to save a file. They cannot
  load files from any of the above working systems. (not working)


ToDo:
- sapi3 is trying to read a disk, so there is no response after showing the logo

Unable to proceed due to no info available (& in English).

ZPS stands for "Základní Počítačová Sestava" (basic computer system).

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "bus/rs232/rs232.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "imagedev/cassette.h"
#include "machine/keyboard.h"
#include "machine/ram.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "machine/timer.h"
#include "speaker.h"


namespace {

class sapi_state : public driver_device
{
public:
	sapi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_bank1(*this, "bank1")
		, m_io_keyboard(*this, "LINE%u", 0U)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_uart(*this, "uart")
		, m_uart_clock(*this, "uart_clock")
		, m_v24(*this, "v24")
		, m_palette(*this, "palette")
	{ }

	void sapi3(machine_config &config);
	void sapi1(machine_config &config);
	void sapi2(machine_config &config);
	void sapi3a(machine_config &config);
	void sapi3b(machine_config &config);

	void init_sapizps3();
	void init_sapizps3a();
	void init_sapizps3b();

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	optional_shared_ptr<uint8_t> m_p_videoram;
	void kbd_put(u8 data);
	uint8_t sapi1_keyboard_r();
	void sapi1_keyboard_w(uint8_t data);
	uint8_t sapi2_keyboard_status_r();
	uint8_t sapi2_keyboard_data_r();
	uint8_t sapi3_0c_r();
	void sapi3_00_w(uint8_t data);
	uint8_t sapi3_25_r();
	void sapi3_25_w(uint8_t data);
	void port10_w(uint8_t data);
	void port11_w(uint8_t data);
	void port13_w(uint8_t data);
	void port43_w(uint8_t data);
	uint8_t port10_r();
	uint8_t port11_r();
	uint8_t port40_r();
	uint8_t port41_r();
	MC6845_UPDATE_ROW(crtc_update_row);
	int si();
	void so(int state);

	uint32_t screen_update_sapi1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sapi3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sapi1_mem(address_map &map) ATTR_COLD;
	void sapi1_io(address_map &map) ATTR_COLD;
	void sapi2_mem(address_map &map) ATTR_COLD;
	void sapi2_io(address_map &map) ATTR_COLD;
	void sapi3_io(address_map &map) ATTR_COLD;
	void sapi3_mem(address_map &map) ATTR_COLD;
	void sapi3a_io(address_map &map) ATTR_COLD;
	void sapi3a_mem(address_map &map) ATTR_COLD;
	void sapi3b_io(address_map &map) ATTR_COLD;
	void sapi3b_mem(address_map &map) ATTR_COLD;

	uint8_t m_term_data = 0U;
	uint8_t m_keyboard_mask = 0U;
	uint8_t m_refresh_counter = 0U;
	uint8_t m_zps3_25 = 0U;
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	void kansas_w(int state);
	u8 m_cass_data[4]{};
	bool m_cassinbit = 0, m_cassoutbit = 0, m_cassold = 0;
	bool m_ier = 0, m_iet = 0;
	optional_memory_bank m_bank1;   // Only for sapi3
	required_ioport_array<5> m_io_keyboard;
	required_device<cpu_device> m_maincpu;
	optional_device<cassette_image_device> m_cass;
	optional_device<ay31015_device> m_uart;
	optional_device<clock_device> m_uart_clock;
	optional_device<rs232_port_device> m_v24;
	optional_device<palette_device> m_palette;
};

static const uint8_t MHB2501[] = {
	0x0c,0x11,0x13,0x15,0x17,0x10,0x0e,0x00, // @
	0x04,0x0a,0x11,0x11,0x1f,0x11,0x11,0x00, // A
	0x1e,0x11,0x11,0x1e,0x11,0x11,0x1e,0x00, // B
	0x0e,0x11,0x10,0x10,0x10,0x11,0x0e,0x00, // C
	0x1e,0x09,0x09,0x09,0x09,0x09,0x1e,0x00, // D
	0x1f,0x10,0x10,0x1e,0x10,0x10,0x1f,0x00, // E
	0x1f,0x10,0x10,0x1e,0x10,0x10,0x10,0x00, // F
	0x0e,0x11,0x10,0x10,0x13,0x11,0x0f,0x00, // G

	0x11,0x11,0x11,0x1f,0x11,0x11,0x11,0x00, // H
	0x0e,0x04,0x04,0x04,0x04,0x04,0x0e,0x00, // I
	0x01,0x01,0x01,0x01,0x11,0x11,0x0e,0x00, // J
	0x11,0x12,0x14,0x18,0x14,0x12,0x11,0x00, // K
	0x10,0x10,0x10,0x10,0x10,0x10,0x1f,0x00, // L
	0x11,0x1b,0x15,0x15,0x11,0x11,0x11,0x00, // M
	0x11,0x11,0x19,0x15,0x13,0x11,0x11,0x00, // N
	0x0e,0x11,0x11,0x11,0x11,0x11,0x0e,0x00, // O

	0x1e,0x11,0x11,0x1e,0x10,0x10,0x10,0x00, // P
	0x0e,0x11,0x11,0x11,0x15,0x12,0x0d,0x00, // Q
	0x1e,0x11,0x11,0x1e,0x14,0x12,0x11,0x00, // R
	0x0e,0x11,0x10,0x0e,0x01,0x11,0x0e,0x00, // S
	0x1f,0x04,0x04,0x04,0x04,0x04,0x04,0x00, // T
	0x11,0x11,0x11,0x11,0x11,0x11,0x0e,0x00, // U
	0x11,0x11,0x11,0x0a,0x0a,0x04,0x04,0x00, // V
	0x11,0x11,0x11,0x15,0x15,0x15,0x0a,0x00, // W

	0x11,0x11,0x0a,0x04,0x0a,0x11,0x11,0x00, // X
	0x11,0x11,0x0a,0x04,0x04,0x04,0x04,0x00, // Y
	0x1f,0x01,0x02,0x04,0x08,0x10,0x1f,0x00, // Z
	0x1c,0x10,0x10,0x10,0x10,0x10,0x1c,0x00, // [
	0x00,0x10,0x08,0x04,0x02,0x01,0x00,0x00, // backslash
	0x07,0x01,0x01,0x01,0x01,0x01,0x07,0x00, // ]
	0x0e,0x11,0x00,0x00,0x00,0x00,0x00,0x00, // ^
	0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x00, // _

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //
	0x04,0x04,0x04,0x04,0x04,0x00,0x04,0x00, // !
	0x0a,0x0a,0x0a,0x00,0x00,0x00,0x00,0x00, // "
	0x0a,0x0a,0x1f,0x0a,0x1f,0x0a,0x0a,0x00, // #
	0x00,0x11,0x0e,0x0a,0x0e,0x11,0x00,0x00, //
	0x18,0x19,0x02,0x04,0x08,0x13,0x03,0x00, // %
	0x04,0x0a,0x0a,0x0c,0x15,0x12,0x0d,0x00, // &
	0x04,0x04,0x08,0x00,0x00,0x00,0x00,0x00, // '

	0x02,0x04,0x08,0x08,0x08,0x04,0x02,0x00, // (
	0x08,0x04,0x02,0x02,0x02,0x04,0x08,0x00, // )
	0x00,0x04,0x15,0x0e,0x15,0x04,0x00,0x00, // *
	0x00,0x04,0x04,0x1f,0x04,0x04,0x00,0x00, // +
	0x00,0x00,0x00,0x00,0x08,0x08,0x10,0x00, // ,
	0x00,0x00,0x00,0x1f,0x00,0x00,0x00,0x00, // -
	0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00, // .
	0x00,0x01,0x02,0x04,0x08,0x10,0x00,0x00, // /

	0x0e,0x11,0x13,0x15,0x19,0x11,0x0e,0x00, // 0
	0x04,0x0c,0x04,0x04,0x04,0x04,0x0e,0x00, // 1
	0x0e,0x11,0x01,0x06,0x08,0x10,0x1f,0x00, // 2
	0x1f,0x01,0x02,0x06,0x01,0x11,0x0e,0x00, // 3
	0x02,0x06,0x0a,0x12,0x1f,0x02,0x02,0x00, // 4
	0x1f,0x10,0x1e,0x01,0x01,0x11,0x0e,0x00, // 5
	0x07,0x08,0x10,0x1e,0x11,0x11,0x0e,0x00, // 6
	0x1f,0x01,0x02,0x04,0x08,0x08,0x08,0x00, // 7

	0x0e,0x11,0x11,0x0e,0x11,0x11,0x0e,0x00, // 8
	0x0e,0x11,0x11,0x0f,0x01,0x02,0x1c,0x00, // 9
	0x00,0x00,0x00,0x00,0x08,0x00,0x08,0x00, // :
	0x00,0x00,0x04,0x00,0x04,0x04,0x08,0x00, // ;
	0x02,0x04,0x08,0x10,0x08,0x04,0x02,0x00, // <
	0x00,0x00,0x1f,0x00,0x1f,0x00,0x00,0x00, // =
	0x08,0x04,0x02,0x01,0x02,0x04,0x08,0x00, // >
	0x0e,0x11,0x01,0x02,0x04,0x00,0x04,0x00  // ?
};


/* Address maps */
void sapi_state::sapi1_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x1fff).rom(); // Extension ROM
	map(0x2000, 0x23ff).ram();
	map(0x2400, 0x27ff).rw(FUNC(sapi_state::sapi1_keyboard_r), FUNC(sapi_state::sapi1_keyboard_w)); // PORT 0 - keyboard
	//map(0x2800, 0x2bff).noprw(); // PORT 1
	//map(0x2c00, 0x2fff).noprw(); // PORT 2
	//map(0x3000, 0x33ff).noprw(); // 3214
	map(0x3800, 0x3fff).ram().share("videoram"); // AND-1 (video RAM)
	map(0x4000, 0x7fff).ram(); // REM-1
}

void sapi_state::sapi1_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	// -bios 1
	map(0x10, 0x10).rw(FUNC(sapi_state::port10_r),FUNC(sapi_state::port10_w));
	map(0x11, 0x11).rw(FUNC(sapi_state::port11_r),FUNC(sapi_state::port11_w));
	map(0x12, 0x12).rw(m_uart, FUNC(ay51013_device::receive), FUNC(ay51013_device::transmit));
	map(0x13, 0x13).w(FUNC(sapi_state::port13_w));
	// -bios 0
	map(0x40, 0x40).r(FUNC(sapi_state::port40_r)); // get inverted byte
	map(0x41, 0x41).r(FUNC(sapi_state::port41_r)); // get status
	map(0x42, 0x42).nopw();   // strobe DS
	map(0x43, 0x43).w(FUNC(sapi_state::port43_w)); // send inverted byte
}

void sapi_state::sapi2_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x1fff).rom(); // Extension ROM
	map(0x2000, 0x23ff).ram();
	map(0x2400, 0x27ff).r(FUNC(sapi_state::sapi2_keyboard_status_r));
	map(0x2800, 0x28ff).r(FUNC(sapi_state::sapi2_keyboard_data_r));
	map(0x3800, 0x3fff).ram().share("videoram"); // AND-1 (video RAM)
	map(0x4000, 0x7fff).ram(); // REM-1
}

void sapi_state::sapi2_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x10, 0x10).rw(FUNC(sapi_state::port10_r),FUNC(sapi_state::port10_w));
	map(0x11, 0x11).rw(FUNC(sapi_state::port11_r),FUNC(sapi_state::port11_w));
	map(0x12, 0x12).rw(m_uart, FUNC(ay51013_device::receive), FUNC(ay51013_device::transmit));
	map(0x13, 0x13).w(FUNC(sapi_state::port13_w));
}

void sapi_state::sapi3_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).ram().bankrw("bank1");
	map(0x0800, 0xf7ff).ram();
	map(0xf800, 0xffff).ram().share("videoram");
}

void sapi_state::sapi3a_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).ram().bankrw("bank1");
	map(0x0800, 0xf7ff).ram();
	map(0xf800, 0xfdff).rom();
	map(0xfe00, 0xffff).ram();
}

void sapi_state::sapi3b_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).ram().bankrw("bank1");
	map(0x0800, 0xafff).ram();
	map(0xb000, 0xbfff).ram().share("videoram");
	map(0xc000, 0xffff).ram();
}

void sapi_state::sapi3_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(sapi_state::sapi3_00_w));
	map(0x25, 0x25).rw(FUNC(sapi_state::sapi3_25_r), FUNC(sapi_state::sapi3_25_w));
}

void sapi_state::sapi3a_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(sapi_state::sapi3_00_w));
	map(0x10, 0x10).rw(FUNC(sapi_state::port10_r), FUNC(sapi_state::port10_w));
	map(0x11, 0x11).rw(FUNC(sapi_state::port11_r), FUNC(sapi_state::port11_w));
	map(0x12, 0x12).rw(m_uart, FUNC(ay51013_device::receive), FUNC(ay51013_device::transmit));
	map(0x13, 0x13).w(FUNC(sapi_state::port13_w));
	map(0x25, 0x25).rw(FUNC(sapi_state::sapi3_25_r), FUNC(sapi_state::sapi3_25_w));
}

void sapi_state::sapi3b_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(sapi_state::sapi3_00_w));
	map(0x0c, 0x0c).r(FUNC(sapi_state::sapi3_0c_r));
	map(0x25, 0x25).rw(FUNC(sapi_state::sapi3_25_r), FUNC(sapi_state::sapi3_25_w));
	map(0xe0, 0xe0).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xe1, 0xe1).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}

/* Input ports */
static INPUT_PORTS_START( sapi1 )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR(';')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('/')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('\'')

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR(0xA4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR(',')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('-')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('<')

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('"')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('.')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('@')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('>')

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('#')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
INPUT_PORTS_END



/**************************************

    Video

**************************************/

uint32_t sapi_state::screen_update_sapi1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for(uint8_t y = 0; y < 24; y++ )
	{
		uint16_t addr = y*64;
		uint16_t xpos = 0;
		for(uint8_t x = 0; x < 40; x++ )
		{
			uint8_t chr = m_p_videoram[addr + x];
			uint8_t attr = (chr >> 6) & 3;
			chr &= 0x3f;
			for(uint8_t ra = 0; ra < 9; ra++ )
			{
				for(uint8_t b = 0; b < 6; b++ )
				{
					bool val = 0;

					if (ra==8)
					{
						if (attr==2)
							val = BIT(m_refresh_counter, 5);
					}
					else
					{
						val = BIT(MHB2501[(chr<<3) | ra], 5-b);
						if (attr==1)
							val = BIT(m_refresh_counter, 5) ? val : 0;
					}

					if(attr==3)
					{
						bitmap.pix(y*9+ra, xpos+2*b   ) = val;
						bitmap.pix(y*9+ra, xpos+2*b+1 ) = val;
					}
					else
					{
						bitmap.pix(y*9+ra, xpos+b ) = val;
					}
				}
			}
			xpos+= (attr==3) ? 12 : 6;
			if (xpos>=6*40) break;
		}
	}
	m_refresh_counter++;
	return 0;
}

// The attributes seem to be different on this one, they need to be understood, so disabled for now
uint32_t sapi_state::screen_update_sapi3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for(uint8_t y = 0; y < 20; y++ )
	{
		uint16_t addr = y*64;
		uint16_t xpos = 0;
		for(uint8_t x = 0; x < 40; x++ )
		{
			uint8_t chr = m_p_videoram[addr + x];
			uint8_t attr = 0;//(chr >> 6) & 3;
			if (chr > 0x3f)
				chr &= 0x1f;

			for(uint8_t ra = 0; ra < 9; ra++ )
			{
				for(uint8_t b = 0; b < 6; b++ )
				{
					bool val = 0;

					if (ra==8)
					{
						if (attr==2)
							val = BIT(m_refresh_counter, 5);
					}
					else
					{
						val = BIT(MHB2501[(chr<<3) | ra], 5-b);
						if (attr==1)
							val = BIT(m_refresh_counter, 5) ? val : 0;
					}

					if(attr==3)
					{
						bitmap.pix(y*9+ra, xpos+2*b   ) = val;
						bitmap.pix(y*9+ra, xpos+2*b+1 ) = val;
					}
					else
					{
						bitmap.pix(y*9+ra, xpos+b ) = val;
					}
				}
			}
			xpos+= (attr==3) ? 12 : 6;
			if (xpos>=6*40) break;
		}
	}
	m_refresh_counter++;
	return 0;
}

MC6845_UPDATE_ROW( sapi_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);

	for (uint16_t x = 0; x < x_count; x++)
	{
		uint8_t inv = 0, gfx = 0;
		if (x == cursor_x) inv ^= 0xff;
		uint16_t mem = (2*(ma + x)) & 0xfff;
		uint8_t chr = m_p_videoram[mem] & 0x3f;

		if (ra < 8)
			gfx = MHB2501[(chr<<3) | ra] ^ inv;

		/* Display a scanline of a character */
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}

/**************************************

    Keyboard

**************************************/

uint8_t sapi_state::sapi1_keyboard_r()
{
	uint8_t key = 0xff;
	if (BIT(m_keyboard_mask, 0)) key &= m_io_keyboard[0]->read();
	if (BIT(m_keyboard_mask, 1)) key &= m_io_keyboard[1]->read();
	if (BIT(m_keyboard_mask, 2)) key &= m_io_keyboard[2]->read();
	if (BIT(m_keyboard_mask, 3)) key &= m_io_keyboard[3]->read();
	if (BIT(m_keyboard_mask, 4)) key &= m_io_keyboard[4]->read();
	return key;
}

void sapi_state::sapi1_keyboard_w(uint8_t data)
{
	m_keyboard_mask = (data ^ 0xff ) & 0x1f;
}

uint8_t sapi_state::sapi2_keyboard_status_r()
{
	return (m_term_data) ? 0 : 1;
}

uint8_t sapi_state::sapi2_keyboard_data_r()
{
	uint8_t ret = ~m_term_data;
	m_term_data = 0;
	return ret;
}

uint8_t sapi_state::port10_r()
{
	uint8_t result = 0;
	result |= m_uart->tbmt_r() || m_uart->dav_r();
	result |= m_uart->or_r() << 1;
	result |= m_uart->fe_r() << 2;
	result |= m_uart->pe_r() << 3;
	result |= m_cassinbit    << 4;
	if (m_v24)
	{
		result |= m_v24->dcd_r() << 5;
		result |= m_v24->dsr_r() << 6;
		result |= m_v24->cts_r() << 7;
	}
	return result;
}

void sapi_state::port10_w(uint8_t data)
{
	if (m_v24)
	{
		m_v24->write_rts(BIT(data, 0));
		m_v24->write_dtr(BIT(data, 1));
	}
	// WD2 = BRK
	// WD3 = S1 - allow cassin?
	// WD4 = KAZ - cass or rs232
	// WD5 = START - motor
	// WD6 = IET - allow tmbt to cause irq
	// WD7 = IER - allow dav to cause irq
	m_iet = BIT(data, 6);
	m_ier = BIT(data, 7);
}

uint8_t sapi_state::port11_r()
{
	u8 data = 0x3f;
	data |= m_uart->dav_r() ? 0x80 : 0;
	data |= m_uart->tbmt_r() ? 0x40 : 0;
	return data;
}

void sapi_state::port11_w(uint8_t data)
{
	m_uart->write_np(BIT(data, 0));
	m_uart->write_tsb(BIT(data, 1));
	m_uart->write_nb1(BIT(data, 3));
	m_uart->write_nb2(BIT(data, 2));
	m_uart->write_eps(BIT(data, 4));
	m_uart->write_cs(1);
	m_uart->write_cs(0);
}

void sapi_state::port13_w(uint8_t data)
{
	// really pulsed by K155AG3 (=74123N): R29=22k, C16=220 (output combined with master reset)
	m_uart->write_xr(0);
	m_uart->write_xr(1);
}

uint8_t sapi_state::port40_r()
{
	return ~m_uart->receive();
}

uint8_t sapi_state::port41_r()
{
	u8 data = 0x7e;
	data |= m_uart->dav_r() ? 0 : 1;
	data |= m_uart->tbmt_r() ? 0x80 : 0;
	return data;
}

void sapi_state::port43_w(uint8_t data)
{
	m_uart->transmit(~data);
}

int sapi_state::si()
{
	return m_cassinbit;
}

void sapi_state::so(int state)
{
	m_cassoutbit = state;
}

void sapi_state::kansas_w(int state)
{
	if ((m_cass->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD)
	{
		// incoming @19200Hz
		u8 twobit = m_cass_data[3] & 15;

		if (state)
		{
			if (twobit == 0)
				m_cassold = m_cassoutbit;

			if (m_cassold)
				m_cass->output(BIT(m_cass_data[3], 2) ? -1.0 : +1.0); // 2400Hz
			else
				m_cass->output(BIT(m_cass_data[3], 3) ? -1.0 : +1.0); // 1200Hz

			m_cass_data[3]++;
		}
	}

	m_uart->write_tcp(state);
	m_uart->write_rcp(state);
}

TIMER_DEVICE_CALLBACK_MEMBER( sapi_state::kansas_r )
{
	// no tape - set to idle
	m_cass_data[1]++;
	if (m_cass_data[1] > 48)
	{
		m_cass_data[1] = 48;
		m_cassinbit = 1;
	}

	// check for interrupts  (not needed)
//  if ((m_uart->tbmt_r() && m_iet) || (m_uart->dav_r() && m_ier))
//      m_maincpu->set_input_line(I8085_INTR_LINE, HOLD_LINE);

	if ((m_cass->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_PLAY)
		return;

	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	uint8_t cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_cassinbit = (m_cass_data[1] < 24) ? 1 : 0;
		m_cass_data[1] = 0;
	}
}

/**************************************

    Machine

**************************************/

void sapi_state::kbd_put(u8 data)
{
	m_term_data = data;
}

uint8_t sapi_state::sapi3_0c_r()
{
	return 0xc0;
}

/* switch out the rom shadow */
void sapi_state::sapi3_00_w(uint8_t data)
{
	m_bank1->set_entry(0);
}

/* to stop execution in random ram */
uint8_t sapi_state::sapi3_25_r()
{
	return m_zps3_25;
}

void sapi_state::sapi3_25_w(uint8_t data)
{
	m_zps3_25 = data & 0xfc; //??
}

void sapi_state::machine_reset()
{
	m_keyboard_mask = 0;
	m_refresh_counter = 0x20;

	if (m_uart)
	{
		// setup uart to 8N2 for sapi1 -bios 0
		m_uart->write_np(1);
		m_uart->write_tsb(1);
		m_uart->write_nb1(1);
		m_uart->write_nb2(1);
		m_uart->write_eps(1);
		m_uart->write_cs(1);
		m_uart->write_cs(0);
	}

	if (m_bank1)
		m_bank1->set_entry(1);
}

void sapi_state::machine_start()
{
	save_item(NAME(m_term_data));
	save_item(NAME(m_keyboard_mask));
	save_item(NAME(m_refresh_counter));
	save_item(NAME(m_zps3_25));
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassinbit));
	save_item(NAME(m_cassoutbit));
	save_item(NAME(m_cassold));
	save_item(NAME(m_ier));
	save_item(NAME(m_iet));

	m_term_data = 0;
}

void sapi_state::init_sapizps3()
{
	uint8_t *RAM = memregion("maincpu")->base();
	m_bank1->configure_entries(0, 2, &RAM[0x0000], 0x10000);
}

void sapi_state::init_sapizps3a()
{
	uint8_t *RAM = memregion("maincpu")->base();
	m_bank1->configure_entries(0, 2, &RAM[0x0000], 0xf800);
	m_uart->write_swe(0);
}

void sapi_state::init_sapizps3b()
{
	uint8_t *RAM = memregion("maincpu")->base();
	m_bank1->configure_entries(0, 2, &RAM[0x0000], 0x10000);
}


/* Machine driver */
void sapi_state::sapi1(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, 18_MHz_XTAL / 9); // Tesla MHB8080A + MHB8224 + MHB8228
	m_maincpu->set_addrmap(AS_PROGRAM, &sapi_state::sapi1_mem);
	m_maincpu->set_addrmap(AS_IO, &sapi_state::sapi1_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(40*6, 24*9);
	screen.set_visarea(0, 40*6-1, 0, 24*9-1);
	screen.set_screen_update(FUNC(sapi_state::screen_update_sapi1));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");

	// cassette is connected to the uart
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	SPEAKER(config, "mono").front_center();
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	// uart
	AY31015(config, m_uart);   // MHB1012
	m_uart->read_si_callback().set(FUNC(sapi_state::si));
	m_uart->write_so_callback().set(FUNC(sapi_state::so));
	m_uart->set_auto_rdav(true);

	CLOCK(config, m_uart_clock, 12.288_MHz_XTAL / 640);   // 19200
	m_uart_clock->signal_handler().set(FUNC(sapi_state::kansas_w));
	TIMER(config, "kansas_r").configure_periodic(FUNC(sapi_state::kansas_r), attotime::from_hz(40000));
}

void sapi_state::sapi2(machine_config &config)
{
	sapi1(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &sapi_state::sapi2_mem);
	m_maincpu->set_addrmap(AS_IO, &sapi_state::sapi2_io);

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(sapi_state::kbd_put));
}

void sapi_state::sapi3(machine_config &config)
{
	sapi2(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &sapi_state::sapi3_mem);
	m_maincpu->set_addrmap(AS_IO, &sapi_state::sapi3_io);

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_size(40*6, 20*9);
	screen.set_visarea(0, 40*6-1, 0, 20*9-1);
	screen.set_screen_update(FUNC(sapi_state::screen_update_sapi3));

	config.device_remove("cassette");
	config.device_remove("uart_clock");
	config.device_remove("kansas_r");
	config.device_remove("mono");
	config.device_remove("uart");
}

void sapi_state::sapi3b(machine_config &config)
{
	sapi3(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &sapi_state::sapi3b_mem);
	m_maincpu->set_addrmap(AS_IO, &sapi_state::sapi3b_io);

	mc6845_device &crtc(MC6845(config, "crtc", 1008000)); // guess
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(6);
	crtc.set_update_row_callback(FUNC(sapi_state::crtc_update_row));

	subdevice<screen_device>("screen")->set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	subdevice<screen_device>("screen")->set_no_palette();
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 ) // high bit stripped off in software
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void sapi_state::sapi3a(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, 18_MHz_XTAL / 9); // Tesla MHB8080A + MHB8224 + MHB8228
	m_maincpu->set_addrmap(AS_PROGRAM, &sapi_state::sapi3a_mem);
	m_maincpu->set_addrmap(AS_IO, &sapi_state::sapi3a_io);

	/* video hardware */
	AY51013(config, m_uart); // Tesla MHB1012
	m_uart->read_si_callback().set(m_v24, FUNC(rs232_port_device::rxd_r));
	m_uart->write_so_callback().set(m_v24, FUNC(rs232_port_device::write_txd));
	m_uart->set_auto_rdav(true); // RDAV not actually tied to RDE, but pulsed by K155AG3 (=74123N): R25=22k, C14=220

	CLOCK(config, m_uart_clock, 12.288_MHz_XTAL / 80); // TODO: divider selectable by jumpers
	m_uart_clock->signal_handler().set(m_uart, FUNC(ay51013_device::write_tcp));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay51013_device::write_rcp));

	RS232_PORT(config, m_v24, default_rs232_devices, "terminal").set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");
}


/**************************************

    Roms

**************************************/

ROM_START( sapi1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "mb1", "MB1" )
	ROMX_LOAD("sapi1.rom", 0x0000, 0x1000, CRC(c6e85b01) SHA1(2a26668249c6161aef7215a1e2b92bfdf6fe3671), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "mb2", "MB2 (ANK-1)" )
	ROMX_LOAD("mb2_4.bin", 0x0000, 0x1000, CRC(a040b3e0) SHA1(586990a07a96323741679a11ff54ad0023da87bc), ROM_BIOS(1))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("sapi1.chr",  0x0000, 0x1000, CRC(9edafa2c) SHA1(a903db0e8923cca91646274d010dc19b6b377e3e))
ROM_END

ROM_START( sapizps2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "v4", "MIKOS 4" )
	ROMX_LOAD("36.bin", 0x0000, 0x0800, CRC(a27f340a) SHA1(d07d208fcbe428897336c17197d3e8fb52181f38), ROM_BIOS(0))
	ROMX_LOAD("37.bin", 0x0800, 0x0800, CRC(30daa708) SHA1(66e990c40788ee25cf6cabd4842a78daf4fcdddd), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v5", "MIKOS 5" )
	ROMX_LOAD("mikos5_1.bin", 0x0000, 0x0800, CRC(c2a83ca3) SHA1(a3678253d7690c89945e791ea0f8e15b081c9126), ROM_BIOS(1))
	ROMX_LOAD("mikos5_2.bin", 0x0800, 0x0800, CRC(c4458a04) SHA1(0cc909323f0e6507d95e57ea39e1deb8bd57bf89), ROM_BIOS(1))
	ROMX_LOAD("mikos5_3.bin", 0x1000, 0x0800, CRC(efb499f3) SHA1(78f0ca3ff10d7af4ae94ab820723296beb035f8f), ROM_BIOS(1))
	ROMX_LOAD("mikos5_4.bin", 0x1800, 0x0800, CRC(4d90e9be) SHA1(8ec554198697550a49432e8210d43700ef1d6a32), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "mb3", "MB3 (Consul)" )
	ROMX_LOAD("mb3_1.bin", 0x0000, 0x1000, CRC(be895f88) SHA1(7fc2a92f41d978a9f0ccd0e235ea3c6146adfb6f), ROM_BIOS(2))
ROM_END

ROM_START( sapizps3 )
	ROM_REGION( 0x10800, "maincpu", 0 )
	// These 2 bioses use videoram at F800
	ROM_SYSTEM_BIOS( 0, "per", "Perina" )
	ROMX_LOAD("perina_1988.bin",0x10000, 0x0800, CRC(d71e8d3a) SHA1(9b3a26ea7c2f2c8a1fb10b51c1c880acc9fd806d), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "1zmod", "JPR-1Zmod" )
	ROMX_LOAD("jpr1zmod.bin",   0x10000, 0x0800, CRC(69a29b07) SHA1(1cd31032954fcd7d10b1586be62db6f7597eb4f2), ROM_BIOS(1))
ROM_END

ROM_START( sapizps3a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// This bios uses a terminal
	ROM_LOAD("jpr1a.bin",      0xf800, 0x0800, CRC(3ed89786) SHA1(dcc8657b4884bfe58d114c539b733b73d038ee30))
ROM_END

ROM_START( sapizps3b )
	ROM_REGION( 0x10800, "maincpu", 0 )
	// This BIOS uses a 6845
	ROM_LOAD("pkt1.bin",       0x10000, 0x0800, CRC(ed5a2725) SHA1(3383c15f87f976400b8d0f31829e2a95236c4b6c))
ROM_END

} // Anonymous namespace


/* Driver */

//    YEAR  NAME       PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT            COMPANY  FULLNAME                   FLAGS
COMP( 1985, sapi1,     0,      0,      sapi1,   sapi1, sapi_state, empty_init,     "Tesla", "SAPI-1 ZPS 1",            MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1985, sapizps2,  sapi1,  0,      sapi2,   sapi1, sapi_state, empty_init,     "Tesla", "SAPI-1 ZPS 2",            MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1985, sapizps3,  sapi1,  0,      sapi3,   sapi1, sapi_state, init_sapizps3,  "Tesla", "SAPI-1 ZPS 3",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1985, sapizps3a, sapi1,  0,      sapi3a,  sapi1, sapi_state, init_sapizps3a, "Tesla", "SAPI-1 ZPS 3 (terminal)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1985, sapizps3b, sapi1,  0,      sapi3b,  sapi1, sapi_state, init_sapizps3b, "Tesla", "SAPI-1 ZPS 3 (6845)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )

