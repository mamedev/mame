// license:BSD-3-Clause
// copyright-holders:AJR
/********************************************************************************

    Data RD100

    Little is known about this system except for a few PCB pictures. No
    manuals, schematic or circuit description have been found.

    The RD100 was apparently sold in France under the "Superkit" brand. There
    appear to have been several versions. Earlier models had 7-segment LEDs
    and rudimentary keyboards. The model dumped here is apparently the K32K,
    which had a 16x2 character LCD display, a QWERTY keyboard and non-numeric
    keypad, Centronics and RS-232 ports, and an extension board for prototyping.

*********************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"


namespace {

class rd100_state : public driver_device
{
public:
	rd100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_keys(*this, "KEY%u", 0U)
		, m_pia1(*this, "pia1")
	{ }

	void rd100(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(pixel_update);

	uint8_t keys_r();
	void key_scan_w(uint8_t data);
	int shift_r();
	int ctrl_r();

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_ioport_array<9> m_keys;
	required_device<pia6821_device> m_pia1;

	uint8_t m_key_scan = 0;
	bool m_shift = false;
	bool m_ctrl = false;
};


void rd100_state::machine_start()
{
	save_item(NAME(m_key_scan));
}

void rd100_state::machine_reset()
{
	m_key_scan = 0;
	m_shift = 0;
	m_ctrl = 0;
}

HD44780_PIXEL_UPDATE(rd100_state::pixel_update)
{
	if (pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

uint8_t rd100_state::keys_r()
{
	uint8_t result = 0xff;
	for (int i = 0; i < 8; i++)
		if (!BIT(m_key_scan, i))
			result &= m_keys[i]->read();

	return result;
}

void rd100_state::key_scan_w(uint8_t data)
{
	m_key_scan = data;
}

int rd100_state::shift_r()
{
	if (m_shift)
	{
		m_shift = 0;
		m_pia1->ca1_w(1);
	}
	bool ky = BIT(m_keys[8]->read(), 0);
	if (!ky)
		m_shift = 1;
	return ky;
}

int rd100_state::ctrl_r()
{
	if (m_ctrl)
	{
		m_ctrl = 0;
		m_pia1->cb1_w(1);
	}
	bool ky = BIT(m_keys[8]->read(), 1);
	if (!ky)
		m_ctrl = 1;
	return ky;
}

void rd100_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();
	map(0x8404, 0x8407).rw("piax", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8408, 0x840b).rw("piay", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8608, 0x860f).rw("timer", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0x8610, 0x8611).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x8640, 0x8643).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8680, 0x8683).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8700, 0x8701).rw("hd44780", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x8800, 0xffff).rom().region("roms", 0x800);
}

/* Input ports */
static INPUT_PORTS_START( rd100 )
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8')   PORT_CHAR('(')                 PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CHAR(0x1b)                PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('I')   PORT_CHAR('i') PORT_CHAR(0x09) PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ')                                  PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('K')   PORT_CHAR('k') PORT_CHAR(0x0b) PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('@')   PORT_CHAR('`')                 PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',')   PORT_CHAR('<')                 PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\')  PORT_CHAR('|') PORT_CHAR(0x1c) PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9')   PORT_CHAR(')')                 PORT_CODE(KEYCODE_9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1')   PORT_CHAR('!')                 PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('O')   PORT_CHAR('o') PORT_CHAR(0x0f) PORT_CODE(KEYCODE_O)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('Q')   PORT_CHAR('q') PORT_CHAR(0x11) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('L')   PORT_CHAR('l') PORT_CHAR(0x0c) PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('A')   PORT_CHAR('a') PORT_CHAR(0x01) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.')   PORT_CHAR('>')                 PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('Z')   PORT_CHAR('z') PORT_CHAR(0x1a) PORT_CODE(KEYCODE_Z)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0')   PORT_CHAR('_') PORT_CHAR(0x1f) PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2')   PORT_CHAR('"')                 PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('P')   PORT_CHAR('p') PORT_CHAR(0x10) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('W')   PORT_CHAR('w') PORT_CHAR(0x17) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';')   PORT_CHAR('+')                 PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('S')   PORT_CHAR('s') PORT_CHAR(0x13) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[')   PORT_CHAR('{')                 PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('X')   PORT_CHAR('x') PORT_CHAR(0x18) PORT_CODE(KEYCODE_X)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(':')   PORT_CHAR('*')                 PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3')   PORT_CHAR('#')                 PORT_CODE(KEYCODE_3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('^')   PORT_CHAR('~') PORT_CHAR(0x1e) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('E')   PORT_CHAR('e') PORT_CHAR(0x05) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/')   PORT_CHAR('?')                 PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('D')   PORT_CHAR('d') PORT_CHAR(0x04) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']')   PORT_CHAR('}') PORT_CHAR(0x1d) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('C')   PORT_CHAR('c') PORT_CHAR(0x03) PORT_CODE(KEYCODE_C)

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-')   PORT_CHAR('=')                 PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4')   PORT_CHAR('$')                 PORT_CODE(KEYCODE_4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BS")  PORT_CHAR(0x08)                PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('R')   PORT_CHAR('r') PORT_CHAR(0x12) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CR")  PORT_CHAR(0x0d)                PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('F')   PORT_CHAR('f') PORT_CHAR(0x06) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('V')   PORT_CHAR('v') PORT_CHAR(0x16) PORT_CODE(KEYCODE_V)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5')   PORT_CHAR('%')                 PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('T')   PORT_CHAR('t') PORT_CHAR(0x14) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('G')   PORT_CHAR('g') PORT_CHAR(0x07) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('B')   PORT_CHAR('b') PORT_CHAR(0x02) PORT_CODE(KEYCODE_B)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6')   PORT_CHAR('&')                 PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('Y')   PORT_CHAR('y') PORT_CHAR(0x19) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('H')   PORT_CHAR('h')                 PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('N')   PORT_CHAR('n') PORT_CHAR(0x0e) PORT_CODE(KEYCODE_N)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7')   PORT_CHAR('\'')                PORT_CODE(KEYCODE_7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('U')   PORT_CHAR('u') PORT_CHAR(0x15) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('J')   PORT_CHAR('j') PORT_CHAR(0x0a) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('M')   PORT_CHAR('m')                 PORT_CODE(KEYCODE_M)

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CHAR(UCHAR_SHIFT_1)     PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")  PORT_CHAR(UCHAR_SHIFT_2)     PORT_CODE(KEYCODE_TAB)
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void rd100_state::rd100(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, 4_MHz_XTAL); // MC6809P???
	m_maincpu->set_addrmap(AS_PROGRAM, &rd100_state::mem_map);

	PIA6821(config, m_pia1);
	m_pia1->readpa_handler().set(FUNC(rd100_state::keys_r));
	m_pia1->writepb_handler().set(FUNC(rd100_state::key_scan_w));
	m_pia1->readca1_handler().set(FUNC(rd100_state::shift_r));
	m_pia1->readcb1_handler().set(FUNC(rd100_state::ctrl_r));

	PIA6821(config, "pia2");
	PIA6821(config, "piax");
	PIA6821(config, "piay");

	ptm6840_device &timer(PTM6840(config, "timer", 4_MHz_XTAL / 4));
	timer.o3_callback().set("acia", FUNC(acia6850_device::write_txc));
	timer.o3_callback().append("acia", FUNC(acia6850_device::write_rxc));

	acia6850_device &acia(ACIA6850(config, "acia"));
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_size(16*6, 16);
	screen.set_visarea(0, 16*6-1, 0, 16-1);
	screen.set_palette("palette");

	hd44780_device &hd44780(HD44780(config, "hd44780", 270'000)); // TODO: clock not measured, datasheet typical clock used
	hd44780.set_lcd_size(2, 16);
	hd44780.set_pixel_update_cb(FUNC(rd100_state::pixel_update));

	PALETTE(config, "palette").set_entries(2);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set("acia", FUNC(acia6850_device::write_cts));
	rs232.dsr_handler().set("acia", FUNC(acia6850_device::write_dcd));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}

ROM_START( rd100 )
	ROM_REGION( 0x8000, "roms", 0 )
	ROM_LOAD( "pak3-01.bin",  0x0000, 0x8000, CRC(cf5bbf01) SHA1(0673f4048d700b84c30781af23fbeabe0b994306) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME  FLAGS
COMP( 1989, rd100, 0,      0,      rd100,   rd100, rd100_state, empty_init, "Data R.D.", "RD100",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
