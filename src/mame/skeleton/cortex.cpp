// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Powertran Cortex

2012-04-20 Skeleton driver.

ftp://ftp.whtech.com/Powertran Cortex/
http://www.powertrancortex.com/index.html

Uses Texas Instruments parts and similar to other TI computers.
It was designed by TI engineers, so it may perhaps be a clone
of another TI or the Geneve.

Chips:
TMS9995   - CPU
TMS9929   - Video
TMS9911   - DMA to floppy (unemulated device)
TMS9909   - Floppy Disk Controller (unemulated device)
TMS9902   - UART (x2) (device not usable with rs232.h)
AY-5-2376 - Keyboard controller

All input to be in uppercase. Note that "lowercase" is just smaller uppercase,
and is not acceptable as input.

There's no option in BASIC to produce sound. It will beep if an invalid key
(usually a control key) is pressed.

To clear the screen press Ctrl L.

TODO:
- Unemulated devices
- Cassette
- Keyboard REPEAT circuit
- Memory mapping unit (74LS610)
- Various CRU I/O

****************************************************************************/

#include "emu.h"

#include "cpu/tms9900/tms9995.h"
#include "machine/74259.h"
#include "machine/kr2376.h"
//#include "machine/tms9902.h"
#include "video/tms9928a.h"
#include "sound/beep.h"

#include "speaker.h"


namespace {

class cortex_state : public driver_device
{
public:
	cortex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_beep(*this, "beeper")
		, m_ay2376(*this, "ay2376")
		, m_pb(*this, "PB")
		, m_io_dsw(*this, "DSW")
	{ }

	void cortex(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void kbd_strobe(int state);
	void kbd_ack_w(int state);
	void romsw_w(int state);
	void vdp_int_w(int state);
	u8 pio_r(offs_t offset);
	u8 kbd_r(offs_t offset);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	bool m_kbd_ack = 0;
	bool m_vdp_int = 0;
	u8 m_kbd_data  = 0U;

	required_device<tms9995_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
	required_device<beep_device> m_beep;
	required_device<kr2376_device> m_ay2376;
	required_ioport m_pb;
	required_ioport m_io_dsw;
};

void cortex_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share(m_ram).bankr(m_bank1);
	map(0x8000, 0xefff).ram();
	map(0xf100, 0xf11f).ram(); // memory mapping unit
	map(0xf120, 0xf121).rw("crtc", FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
	//map(0xf140, 0xf147) // fdc tms9909
}

void cortex_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x000f).mirror(0x30).w("control", FUNC(ls259_device::write_d0));
	map(0x0000, 0x000f).r(FUNC(cortex_state::pio_r));
	map(0x0010, 0x001f).r(FUNC(cortex_state::kbd_r));
	//map(0x0080, 0x00bf).rw("uart1", FUNC(tms9902_device::cruread), FUNC(tms9902_device::cruwrite)); // RS232 (r12 = 80-bf)
	//map(0x0180, 0x01bf).rw("uart2", FUNC(tms9902_device::cruread), FUNC(tms9902_device::cruwrite)); // Cassette (r12 = 180-1bf)
	//map(0x01c0, 0x01ff).rw("dma", FUNC(tms9911_device::read), FUNC(tms9911_device::write)); // r12 = 1c0-1fe
	//map(0x0800, 0x080f).w(cortex_state::cent_data_w)); // r12 = 800-80e
	//map(0x0810, 0x0811).w(FUNC(cortex_state::cent_strobe_w)); // r12 = 810
	//map(0x0812, 0x0813).r(FUNC(cortex_state::cent_stat_r)); // CRU 409 (r12 = 812)
}

/* Input ports */
static INPUT_PORTS_START( cortex )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))     PORT_NAME("Edit")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))  PORT_NAME("Insert")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))     PORT_NAME("Delete")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))   PORT_NAME("Home")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD)                               PORT_CHAR('_')

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))  PORT_NAME(u8"\u2190") // U+2190 = ←
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                   PORT_NAME("Return")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))  PORT_NAME(u8"\u2193") // U+2193 = ↓
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)                    PORT_NAME("Rub Out")

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('c') PORT_CHAR('G')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(PGUP))   PORT_NAME("Clear")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))    PORT_NAME("Escape")

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))  PORT_NAME(u8"\u2192") // U+2192 = →
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))     PORT_NAME(u8"\u2191") // U+2191 = ↑

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')

	PORT_START("PB")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift")      PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl")       PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)  PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock")  PORT_CODE(KEYCODE_CAPSLOCK)                 PORT_TOGGLE  PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Graph")      PORT_CODE(KEYCODE_PGDN)                                  PORT_CHAR(UCHAR_MAMEKEY(PGDN))

	PORT_START("DSW")
	PORT_DIPNAME(0x04, 0x00, "DISK SIZE")
	PORT_DIPSETTING(   0x04, "20cm")
	PORT_DIPSETTING(   0x00, "13cm")
	PORT_DIPNAME(0x08, 0x08, "DISK DENSITY")
	PORT_DIPSETTING(   0x08, "Double")
	PORT_DIPSETTING(   0x00, "Single")
INPUT_PORTS_END

u8 cortex_state::pio_r(offs_t offset)
{
	switch (offset)
	{
	case 5:
		return m_kbd_ack;

	case 6:
		return m_vdp_int;

	case 2:
	case 3:
		return BIT(m_io_dsw->read(), offset);

	default:
		return 1;
	}
}

u8 cortex_state::kbd_r(offs_t offset)
{
	return BIT(m_kbd_data, offset);
}

void cortex_state::kbd_ack_w(int state)
{
	if (!state)
	{
		m_maincpu->set_input_line(INT_9995_INT4, CLEAR_LINE);
		m_kbd_ack = 1;
	}
}

void cortex_state::romsw_w(int state)
{
	m_bank1->set_entry(state ? 0 : 1);
}

void cortex_state::vdp_int_w(int state)
{
	m_vdp_int = state ? 0 : 1;  // change polarity to match mame
}

void cortex_state::kbd_strobe(int state)
{
	if (state)
	{
		m_kbd_data = m_ay2376->data_r() & 0x5f;

		// Caps Lock
		if (BIT(m_pb->read(), 2))
			m_kbd_data |= BIT(m_ay2376->data_r(), 7) << 5;
		else
			m_kbd_data |= BIT(m_ay2376->data_r(), 5) << 5;

		// Graphics
		m_kbd_data |= BIT(m_pb->read(), 3) << 7;

		m_kbd_ack = 0;
		m_maincpu->set_input_line(INT_9995_INT4, ASSERT_LINE);
	}
}

void cortex_state::machine_start()
{
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);

	save_item(NAME(m_kbd_ack));
	save_item(NAME(m_vdp_int));
	save_item(NAME(m_kbd_data));
}

void cortex_state::machine_reset()
{
	m_kbd_ack = 1;
	m_vdp_int = 0;
	m_beep->set_state(0);
	m_bank1->set_entry(1);
	m_maincpu->ready_line(ASSERT_LINE);
	m_maincpu->reset_line(ASSERT_LINE);
}

void cortex_state::cortex(machine_config &config)
{
	/* basic machine hardware */
	/* TMS9995 CPU @ 12.0 MHz */
	// Standard variant, no overflow int
	// No lines connected yet
	TMS9995(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cortex_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &cortex_state::io_map);

	ls259_device &control(LS259(config, "control")); // IC64
	//control.q_out_cb<0>().set(FUNC(cortex_state::basic_led_w));
	control.q_out_cb<1>().set(FUNC(cortex_state::kbd_ack_w));
	//control.q_out_cb<2>().set(FUNC(cortex_state::ebus_int_ack_w));
	//control.q_out_cb<3>().set(FUNC(cortex_state::ebus_to_en_w));
	//control.q_out_cb<4>().set(FUNC(cortex_state::disk_size_w));
	control.q_out_cb<5>().set(FUNC(cortex_state::romsw_w));
	control.q_out_cb<6>().set("beeper", FUNC(beep_device::set_state));

	/* video hardware */
	tms9929a_device &crtc(TMS9929A(config, "crtc", 10.738635_MHz_XTAL));
	crtc.set_screen("screen");
	crtc.int_callback().set_inputline(m_maincpu, INT_9995_INT1);
	crtc.int_callback().append(FUNC(cortex_state::vdp_int_w));
	crtc.set_vram_size(0x4000);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	KR2376_ST(config, m_ay2376, 50000);
	m_ay2376->x<0>().set_ioport("X0");
	m_ay2376->x<1>().set_ioport("X1");
	m_ay2376->x<2>().set_ioport("X2");
	m_ay2376->x<3>().set_ioport("X3");
	m_ay2376->x<4>().set_ioport("X4");
	m_ay2376->x<5>().set_ioport("X5");
	m_ay2376->x<6>().set_ioport("X6");
	m_ay2376->x<7>().set_ioport("X7");
	m_ay2376->shift().set_ioport("PB").bit(0);
	m_ay2376->control().set_ioport("PB").bit(1);
	m_ay2376->strobe().set(FUNC(cortex_state::kbd_strobe));

	//tms9902_device &uart1(TMS9902(config, "uart1", 12_MHz_XTAL / 4));
	//uart1.int_cb().set_inputline(m_maincpu, INT_9995_INT4);
	//tms9902_device &uart2(TMS9902(config, "uart2", 12_MHz_XTAL / 4));
	//uart2.int_cb().set_inputline(m_maincpu, INT_9995_INT4);

	/* Sound */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 950); // guess
	m_beep->add_route(ALL_OUTPUTS, "mono", 0.05);
}

/* ROM definition */
ROM_START( cortex )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "basic", "Cortex BIOS")
	ROMX_LOAD( "cortex.ic47", 0x0000, 0x2000, CRC(bdb8c7bd) SHA1(340829dcb7a65f2e830fd5aff82a312e3ed7918f), ROM_BIOS(0))
	ROMX_LOAD( "cortex.ic46", 0x2000, 0x2000, CRC(4de459ea) SHA1(00a42fe556d4ffe1f85b2ce369f544b07fbd06d9), ROM_BIOS(0))
	ROMX_LOAD( "cortex.ic45", 0x4000, 0x2000, CRC(b0c9b6e8) SHA1(4e20c3f0b7546b803da4805cd3b8616f96c3d923), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "forth", "FIG-Forth")
	ROMX_LOAD( "forth.ic47",  0x0000, 0x2000, CRC(999034be) SHA1(0dcc7404c38aa0ae913101eb0aa98da82104b5d4), ROM_BIOS(1))
	ROMX_LOAD( "forth.ic46",  0x2000, 0x2000, CRC(8eca54cc) SHA1(0f1680e941ef60bb9bde9a4b843b78f30dff3202), ROM_BIOS(1))
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY                  FULLNAME  FLAGS
COMP( 1982, cortex, 0,      0,      cortex,  cortex, cortex_state, empty_init, "Powertran Cybernetics", "Cortex", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
