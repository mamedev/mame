// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for BraiLab Plus talking computer.

    (from a document translated from Hungarian)
In addition to the Z80 microprocessor on the two-sided printed circuit board,
there are 256 Kbytes of dynamic RAM, 16 Kbytes of EPROM, 2 Z80PIOs, 4 Kbytes
of static RAM, a character generator EPROM, a WD2793 floppy disk controller,
a I8251 PUSART and a MEA-8000 speech synthesizer.

180 Kbytes of 256 Kbytes of RAM is handled as a fixed RAM disk under the
CP/M operating system. Memory management supports the use of a dynamic RAM
fixed disk, provides the appropriate address space for the operating system,
and flips between them and between EPROMs. The two Z80PIOs perform centronics
interface, system clock interrupt handling, and serial programming of the
baud rate with serial interface I8251 and HD-4702.

The increased size of 4 Kbytes memory enabled 25x80 character screen.
Floppy disk holds 792kb (formatted).

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/f4702.h"
#include "machine/i8251.h"
#include "machine/wd_fdc.h"
#include "machine/z80pio.h"


namespace {

class braiplus_state : public driver_device
{
public:
	braiplus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio(*this, "pio%u", 1U)
		, m_keyboard(*this, "K%u", 0U)
		, m_baud_rate(0)
	{
	}

	void braiplus(machine_config &config);

private:
	void baud_rate_w(u8 data);
	u8 baud_rate_r();
	void bank_w(u8 data);
	void unknown_w(u8 data);
	u8 keyboard_r(offs_t offset);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device_array<z80pio_device, 2> m_pio;
	required_ioport_array<10> m_keyboard;

	u8 m_baud_rate;
};


void braiplus_state::baud_rate_w(u8 data)
{
	m_baud_rate = data >> 4;
}

u8 braiplus_state::baud_rate_r()
{
	return m_baud_rate;
}

void braiplus_state::bank_w(u8 data)
{
	// TODO: bankswitching
}

void braiplus_state::unknown_w(u8 data)
{
	// TODO: unknown (only high nibble used)
}

u8 braiplus_state::keyboard_r(offs_t offset)
{
	u8 ret = offset < 0x14 ? m_keyboard[offset >> 1]->read() : 0xff;
	if (BIT(offset, 0))
		ret >>= 4;
	else
		ret &= 0x0f;

	// TODO: high nibble appears to be used by an optional PC/XT keyboard adapter
	return ret | 0xf0;
}

void braiplus_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x8000, 0x801f).r(FUNC(braiplus_state::keyboard_r));
	map(0xa000, 0xbfff).ram();
	map(0xf000, 0xffff).ram();
}

void braiplus_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x04, 0x07).rw(m_pio[0], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x08, 0x0b).rw(m_pio[1], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x0c, 0x0d).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x10, 0x13).rw("fdc", FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0x14, 0x14).w(FUNC(braiplus_state::bank_w));
	map(0x18, 0x18).w(FUNC(braiplus_state::unknown_w));
}


static INPUT_PORTS_START(braiplus)
	PORT_START("K0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Ctrl") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Ctrl") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key A0")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key A1")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key A2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key A3")

	PORT_START("K1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('"') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('&') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('\'') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('(') PORT_CODE(KEYCODE_8)

	PORT_START("K2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)

	PORT_START("K3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)

	PORT_START("K4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N') PORT_CODE(KEYCODE_N)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)

	PORT_START("K5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x00e9, 0x7b) PORT_CHAR(0x00c9, 0x5b) PORT_NAME(u8"É") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x00e1, 0x60) PORT_CHAR(0x00c1, 0x40) PORT_NAME(u8"Á") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR('_') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR(')') PORT_CODE(KEYCODE_9)

	PORT_START("K6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('?') PORT_CHAR('/') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('-')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x0d) PORT_NAME("CR") PORT_CODE(KEYCODE_ENTER)

	PORT_START("K7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x00f3, 0x7e) PORT_CHAR(0x00d3, 0x5e) PORT_NAME(u8"Ó") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x00f6, 0x7c) PORT_CHAR(0x00d6, 0x5c) PORT_NAME(u8"Ö") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 5F")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x00fc, 0x7d) PORT_CHAR(0x00dc, 0x5d) PORT_NAME(u8"Ü") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 05")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 08")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 18")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 04")

	PORT_START("K8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key E0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key E1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key E2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key E3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key E4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key E5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key E6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key E7")

	PORT_START("K9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key E8")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key E9")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key EA")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key EB")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key EC")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key ED")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key EE")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key EF")
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "pio1" },
	{ "pio2" },
	{ nullptr }
};

void braiplus_state::braiplus(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &braiplus_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &braiplus_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	Z80PIO(config, m_pio[0], 4'000'000);
	m_pio[0]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80PIO(config, m_pio[1], 4'000'000);
	m_pio[1]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio[1]->out_pa_callback().set(FUNC(braiplus_state::baud_rate_w));

	f4702_device &brg(F4702(config, "brg", 2.4576_MHz_XTAL));
	brg.s_callback().set(FUNC(braiplus_state::baud_rate_r));
	brg.z_callback().set("usart", FUNC(i8251_device::write_txc));
	brg.z_callback().append("usart", FUNC(i8251_device::write_rxc));

	i8251_device &usart(I8251(config, "usart", 2'000'000));
	usart.rxrdy_handler().set(m_pio[1], FUNC(z80pio_device::pa3_w));

	WD2793(config, "fdc", 2'000'000);

	// HACK: what should be driving this?
	CLOCK(config, "clock_hack", 100).signal_handler().set(m_pio[0], FUNC(z80pio_device::pa7_w));
}

ROM_START( braiplus )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "brailabplus.bin", 0x0000, 0x4000, CRC(521d6952) SHA1(f7405520d86fc7abd2dec51d1d016658472f6fe8) )

	ROM_REGION( 0x0800, "chargen", 0 ) // no idea what chargen it uses, using the one from homelab4 for now
	ROM_LOAD( "hl4.chr",   0x0000, 0x0800, BAD_DUMP CRC(f58ee39b) SHA1(49399c42d60a11b218a225856da86a9f3975a78a))
ROM_END

} // anonymous namespace


COMP( 1988, braiplus, 0, 0, braiplus, braiplus, braiplus_state, empty_init, "Jozsef and Endre Lukacs", "BraiLab Plus", MACHINE_IS_SKELETON )
