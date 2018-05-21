// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * A low level emulation implementation of the Intergraph InterPro keyboard.
 *
 *   Ref   Part                     Function
 *     1   SN74LS244N               octal buffer and line driver with tri-state outputs
 *     2   M2732A-2F1               NMOS 32K (4Kx8) UV EPROM
 *     3   SN74LS374N               octal d-type flip-flop
 *     4   SN74LS373N               octal d-type latch
 *     5   Intel 8049AH             MCS-48 family microcontroller
 *     6   SN74LS368AN              hex bus driver with tri-state outputs
 *     7   SN74159N                 4-to-16 decoder/demultiplexer with open collector outputs
 *     8   SN74LS14N                hex Schmitt trigger inverter
 *     9   Motorola SC2 3181C       ?
 *    10   DS75452N                 dual peripheral NAND driver
 *    52   11.000MHz crystal
 *
 *    67                            membrane matrix connector?
 *    68                            reset and boot button connector
 *    69                            computer interface cable connector
 *    70, 71, 72                    keyswitch matrix connectors?
 *    5?-64                         status LEDs
 *
 * TODO
 *   - not functional, WIP
 *   - requires ~500us after start before ready to accept commands
 *   - keyboard matrix is not yet mapped
 */

/*
 * Work in progress notes
 *
 * 11MHz XTAL / 15 / 32 gives 22.916kHz clock (~43.6us/cycle)
 *   - prescaler loaded with 0xfa, gives ~262us/tick (~3.819kHz)
 *
 * P1
 *   0x01
 *   0x02
 *   0x04
 *   0x08
 *   0x10 - speaker out
 *   0x20
 *   0x40 - bus data valid?
 *   0x80
 *
 * P2
 *   0x0f - bank select
 *   0x20 - serial tx, inverted
 *
 * T0
 *   serial rx, inverted
 *
 * T1
 *
 * At runtime, R7 contains status flags:
 *   0x01
 *   0x02
 *   0x04
 *   0x08 - tx buffer empty?
 *   0x10 - rx double ESC?
 *   0x20 - timer 0x3a expired
 *   0x40 - rx complete
 *   0x80 - rx timer active
 */

#include "emu.h"
#include "lle.h"

#include "speaker.h"
#include "machine/keyboard.ipp"

#define LOG_GENERAL (1U << 0)
#define LOG_RXTX    (1U << 1)
#define LOG_PORT    (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE_NS(INTERPRO_LLE_EN_US_KEYBOARD, bus::interpro::keyboard, lle_en_us_device, "kbd_lle_en_us", "InterPro Keyboard (LLE, US English)")

namespace bus { namespace interpro { namespace keyboard {

namespace {

// TODO: actual matrix is not yet mapped
INPUT_PORTS_START(interpro_en_us)

	PORT_START("row_0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_NAME("Esc")       PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)                             PORT_CHAR('`')  PORT_CHAR('~')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)                                 PORT_CHAR('2')  PORT_CHAR('@')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)                                 PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)                                 PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)                                 PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)                                 PORT_CHAR('6')  PORT_CHAR('^')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)                                 PORT_CHAR('7')  PORT_CHAR('&')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)                                 PORT_CHAR('8')  PORT_CHAR('*')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)                                 PORT_CHAR('9')  PORT_CHAR('(')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)                                 PORT_CHAR('0')  PORT_CHAR(')')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)                             PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)                            PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_NAME("Backspace") PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_NAME("Delete")    PORT_CHAR(UCHAR_MAMEKEY(DEL))

	PORT_START("row_1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED) // "Alt Mode"
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_NAME("Tab")       PORT_CHAR(9)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)                                 PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)                                 PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)                                 PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)                                 PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)                                 PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)                                 PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)                                 PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)                                 PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)                                 PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)                         PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)                        PORT_CHAR(']')  PORT_CHAR('}')

	PORT_START("row_2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_NAME("Control")   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_NAME("Caps Lock") PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)                                 PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)                                 PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)                                 PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)                                 PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)                                 PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)                                 PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)                                 PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)                                 PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)                                 PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)                             PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)                             PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)                         PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_NAME("Return")    PORT_CHAR(UCHAR_MAMEKEY(ENTER))

	PORT_START("row_3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_NAME("LShift")    PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)                        PORT_CHAR('<')  PORT_CHAR('>')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)                                 PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)                                 PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)                                 PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)                                 PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)                                 PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)                                 PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)                                 PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)                             PORT_CHAR(',')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)                              PORT_CHAR('.')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)      PORT_NAME("RShift")   PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)          PORT_NAME("Up")       PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("row_4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED) // "Hold Screen"
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED) // "Superimpose"
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNUSED) // "Line Feed"
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)       PORT_NAME("Space")    PORT_CHAR(UCHAR_MAMEKEY(SPACE))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNUSED) // "Repeat"
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)        PORT_NAME("Left")     PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)        PORT_NAME("Down")     PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)       PORT_NAME("Right")    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

INPUT_PORTS_END

INPUT_PORTS_START(lle_en_us_device)
	PORT_INCLUDE(interpro_en_us)
INPUT_PORTS_END

ROM_START(lle_en_us)
	ROM_REGION(0x800, "mcu", 0)
	ROM_LOAD("i8049ah.5", 0x0, 0x800, CRC(7b74f43b) SHA1(c43d41ac52df4d1282edc06f891cf27ef9255faa))

	ROM_REGION(0x1000, "eprom", 0)
	ROM_LOAD("sd03595.37", 0x0, 0x1000, CRC(263ed215) SHA1(4de550ff1eec7996c7f26e92c5d268aa24024a7f))
ROM_END

} // anonymous namespace

lle_device_base::lle_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_interpro_keyboard_port_interface(mconfig, *this)
	, device_matrix_keyboard_interface(mconfig, *this, "row_0", "row_1", "row_2", "row_3", "row_4")
	, m_mcu(*this, "mcu")
	, m_bell(*this, "bell")
	, m_ext(*this, "ext")
	, m_txd(0)
{
}
MACHINE_CONFIG_START(lle_device_base::device_add_mconfig)
	MCFG_DEVICE_ADD("mcu", I8049, 11_MHz_XTAL)
	MCFG_DEVICE_ADDRESS_MAP(AS_IO, io_map)
	MCFG_MCS48_PORT_T0_IN_CB(READLINE(*this, lle_device_base, t0_r))
	MCFG_MCS48_PORT_T1_IN_CB(READLINE(*this, lle_device_base, t1_r))
	MCFG_MCS48_PORT_P1_IN_CB(READ8(*this, lle_device_base, p1_r))
	MCFG_MCS48_PORT_P1_OUT_CB(WRITE8(*this, lle_device_base, p1_w))
	MCFG_MCS48_PORT_P2_IN_CB(READ8(*this, lle_device_base, p2_r))
	MCFG_MCS48_PORT_P2_OUT_CB(WRITE8(*this, lle_device_base, p2_w))
	MCFG_MCS48_PORT_BUS_IN_CB(READ8(*this, lle_device_base, bus_r))
	MCFG_MCS48_PORT_BUS_OUT_CB(WRITE8(*this, lle_device_base, bus_w))

	MCFG_DEVICE_ADD(m_ext, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(ext_map)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDR_WIDTH(12)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x100)

	SPEAKER(config, "keyboard").front_center();
	MCFG_DEVICE_ADD("bell", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "keyboard", 0.25)
MACHINE_CONFIG_END

void lle_device_base::device_start()
{
}

void lle_device_base::device_reset()
{
}

void lle_device_base::io_map(address_map &map)
{
	map(0, 0xff).m(m_ext, FUNC(address_map_bank_device::amap8));
}

void lle_device_base::ext_map(address_map &map)
{
	map(0, 0xfff).rom().region("eprom", 0);

	map(0x7fe, 0x7fe).w(this, FUNC(lle_device_base::latch_w));
}

READ_LINE_MEMBER(lle_device_base::t0_r)
{
	if ((VERBOSE & LOG_RXTX) && (m_mcu->pc() == 0x8e) && m_txd == 0)
	{
		auto const suppressor(machine().disable_side_effects());

		address_space &mcu_ram(m_mcu->space(AS_DATA));
		const u8 input(mcu_ram.read_byte(0x42));

		LOGMASKED(LOG_RXTX, "received byte 0x%02x\n", input);
	}

	return m_txd;
}

READ_LINE_MEMBER(lle_device_base::t1_r)
{
	// TODO: possibly one of the modifiers?

	return CLEAR_LINE;
}

READ8_MEMBER(lle_device_base::p1_r)
{
	LOGMASKED(LOG_PORT, "p1_r (%s)\n", machine().describe_context());

	return 0xff;
}

WRITE8_MEMBER(lle_device_base::p1_w)
{
	LOGMASKED(LOG_PORT, "p1_w 0x%02x (%s)\n", data, machine().describe_context());

	m_bell->level_w(BIT(data, 4));
}

READ8_MEMBER(lle_device_base::p2_r)
{
	LOGMASKED(LOG_PORT, "p2_r (%s)\n", machine().describe_context());

	return 0xff;
}

WRITE8_MEMBER(lle_device_base::p2_w)
{
	LOGMASKED(LOG_PORT, "p2_w 0x%02x (%s)\n", data, machine().describe_context());

	if ((VERBOSE & LOG_RXTX) && (m_mcu->pc() == 0x1d || m_mcu->pc() == 0x21))
	{
		auto const suppressor(machine().disable_side_effects());

		address_space &mcu_ram(m_mcu->space(AS_DATA));
		const u8 txd_state(mcu_ram.read_byte(0x1d));
		const u8 output(mcu_ram.read_byte(0x2d));

		if (txd_state == 0x20)
			LOGMASKED(LOG_RXTX, "transmitting byte 0x%02x\n", output);
	}

	m_ext->set_bank(data & 0x0f);

	// serial transmit
	output_rxd((data & 0x20) ? CLEAR_LINE : ASSERT_LINE);
}

READ8_MEMBER(lle_device_base::bus_r)
{
	LOGMASKED(LOG_PORT, "bus_r (%s)\n", machine().describe_context());

	return 0xff;
}

WRITE8_MEMBER(lle_device_base::bus_w)
{
	LOGMASKED(LOG_PORT, "bus_w 0x%02x (%s)\n", data, machine().describe_context());

	m_bus = data;
}

WRITE8_MEMBER(lle_device_base::latch_w)
{
	LOGMASKED(LOG_PORT, "latch_w 0x%02x (%s)\n", data, machine().describe_context());
}

lle_en_us_device::lle_en_us_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: lle_device_base(mconfig, INTERPRO_LLE_EN_US_KEYBOARD, tag, owner, clock)
{
}

tiny_rom_entry const *lle_en_us_device::device_rom_region() const
{
	return ROM_NAME(lle_en_us);
}

ioport_constructor lle_en_us_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(lle_en_us_device);
}

} } } // namespace bus::interpro::keyboard
