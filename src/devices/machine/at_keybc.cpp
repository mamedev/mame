// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    IBM PC/AT and PS/2 keyboard controllers

    IBM used Intel UPI-41 microcontrollers to implement bidirectional
    keyboard communication from the PC/AT and PS/2.  On the PS/2, the
    microcontroller also handles mouse communication.  The PS/2 host
    interface to the keyboard/mouse controller is backwards-compatible
    for documented commands.  However a number of undocumented commands
    are no longer implemented, the pin assignments are different, and
    interrupt outputs are latched externally using a 74ALS74 at ZM87.

    Many of the ICs that provide functional compatibility with the AT
    and/or PS/2 "8042" keyboard controllers do not incorporate actual
    UPI-41 microcontroller cores. This even applies to some directly
    equivalent 40-pin DIPs such as the JETkey "Keyboard BIOS" V3.0
    and V5.0, which were found to be Samsung Electronics ASICs when
    decapped.

    PC/AT I/O pin assignments:
    P10:    NC              no connection
    P11:    NC              no connection
    P12:    NC              no connection
    P13:    NC              no connection
    P14:    -RAM SEL        enable external RAM         low disables
    P15:                    manufacturing setting       low disables
    P16:    SW1             CRT adapter switch          low for CGA
    P17:    +KBD INH        inhibit keyboard            active low
    P20:    RC              reset CPU                   active low
    P21:    A20 GATE        enable A20 line             active high
    P22:    NC              no connection
    P23:    NC              no connection
    P24:    +OPT BUF FULL   output buffer full          active high
    P25:    NC              no connection
    P26:                    open collector KBD CLK      inverted
    P27:                    open collector KBD DATA     not inverted
    TEST0:  KBD CLK                                     not inverted
    TEST1:  KBD DATA                                    not inverted

    PS/2 I/O pin assignments
    P10:    KYBD DATA                                   not inverted
    P11:    MOUSE DATA                                  not inverted
    P12:                    keyboard/mouse power fuse   low if blown
    P13:                    no connection
    P14:                    no connection
    P15:                    no connection
    P16:                    no connection
    P17:                    no connection
    P20:    +HOT RES        reset CPU                   inverted
    P21:    A20 PASS        enable A20 line             not inverted
    P22:                    open collector MOUSE DATA   inverted
    P23:                    open collector MOUSE CLK    inverted
    P24:                    latched as BIRQ1            rising trigger
    P25:                    latched as IRQ12            rising trigger
    P26:                    open collector KYBD CLK     inverted
    P27:                    open collector KYBD DATA    inverted
    TEST0:  KYBD CLK                                    not inverted
    TEST1:  MOUSE CLK                                   not inverted

    Notes:
    * PS/2 BIRQ1 and IRQ12 are cleard by reading data.
    * PS/2 BIRQ1 and IRQ12 are active low, but we use ASSERT_LINE here.
    * PS/2 IRQ12 is open collector for sharing with cards.

    TODO:
    * Move display type and key lock controls to drivers.
    * Expose power fuse failure.

***************************************************************************/
/*
 * References:
 *
 *   http://bitsavers.org/pdf/ibm/pc/at/6183355_PC_AT_Technical_Reference_Mar86.pdf
 *   http://bitsavers.org/pdf/ibm/pc/ps2/84X1508_PS2_Model_80_Technical_Reference_Apr87.pdf
 *   http://halicery.com/Hardware/Intel%208042%20and%208048/index.html
 *   https://web.archive.org/web/20180124085559/http://computer-engineering.org/ps2protocol
 *
 * The Intel UPI-41AH/42AH datasheet identifies the following variants:
 *
 *   Device  Firmware              Application          Ports
 *   8242PC  Phoenix Technologies  AT, PS/2, EISA       kbd+aux
 *   8242BB  IBM                   AT, PS/2, EISA, PCI  kbd+aux
 *   8242WA  Award Software        AT                   kbd
 *   8242WB  Award Software        AT                   kbd+aux
 *
 */

#include "emu.h"
#include "at_keybc.h"

#define LOG_COMMAND (1U << 1)
#define LOG_STATUS  (1U << 2)

//#define VERBOSE (LOG_GENERAL|LOG_COMMAND)

#include "logmacro.h"

namespace {

ROM_START(at_kbc)
	ROM_REGION(0x0800, "mcu", 0)
	ROM_SYSTEM_BIOS(0, "ibm", "IBM 1983") // 1983 IBM controller BIOS
	ROMX_LOAD("1503033.bin", 0x0000, 0x0800, CRC(5a81c0d2) SHA1(0100f8789fb4de74706ae7f9473a12ec2b9bd729), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ptl", "PTL 1986") // unknown controller BIOS, (c) 1985, 1986 PTL
	ROMX_LOAD("yan25d05.bin", 0x0000, 0x0800, CRC(70c798f1) SHA1(ae9a79c7184a17331b70a50035ff63c757df094c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "award15", "Award 1985 V1.5") // MIPS Rx2030 i8742 keyboard controller
	ROMX_LOAD("keyboard_1.5.bin", 0x0000, 0x0800, CRC(f86ba0f7) SHA1(1ad475451db35a76d929824c035d582279fbe3a3), ROM_BIOS(2))
ROM_END

ROM_START(ps2_kbc)
	ROM_REGION(0x0800, "mcu", 0)
	ROM_SYSTEM_BIOS(0, "ibm", "IBM 1987") // IBM PS/2 Model 50
	ROMX_LOAD("72x8455.zm82", 0x0000, 0x0800, CRC(7da223d3) SHA1(54c52ff6c6a2310f79b2c7e6d1259be9de868f0e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "compaq", "Compaq 1987") // Compaq Portable 486
	ROMX_LOAD("128251-001.u3", 0x0000, 0x0800, CRC(da968f1e) SHA1(ac45a4d0f30b046ada300f1d105bcf0fb2db318a), ROM_BIOS(1))
ROM_END

INPUT_PORTS_START(at_kbc)
	PORT_START("P1")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_CONFNAME(0x40, 0x00, "CRT Adapter Switch")
	PORT_CONFSETTING(   0x40, "Monochrome Display Adapter")
	PORT_CONFSETTING(   0x00, "Color Graphics Adapter")
	PORT_CONFNAME(0x80, 0x80, "Key Lock")
	PORT_CONFSETTING(   0x80, DEF_STR(Off))
	PORT_CONFSETTING(   0x00, DEF_STR(On))
INPUT_PORTS_END

} // anonymous namespace



//**************************************************************************
//  DEVICE TYPES
//**************************************************************************

DEFINE_DEVICE_TYPE(AT_KEYBOARD_CONTROLLER, at_keyboard_controller_device, "at_keybc", "PC/AT Keyboard Controller")
DEFINE_DEVICE_TYPE(PS2_KEYBOARD_CONTROLLER, ps2_keyboard_controller_device, "ps2_keybc", "PS/2 Keyboard/Mouse Controller")


//**************************************************************************
//  KEYBOARD CONTROLLER DEVICE BASE
//**************************************************************************

uint8_t at_kbc_device_base::data_r()
{
	u8 const data = m_mcu->upi41_master_r(0U);
	LOG("data_r 0x%02x (%s)\n", data, machine().describe_context());
	return data;
}

uint8_t at_kbc_device_base::status_r()
{
	u8 const data = m_mcu->upi41_master_r(1U);
	LOGMASKED(LOG_STATUS, "status_r 0x%02x%s%s%s%s%s%s%s%s (%s)\n", data,
		BIT(data, 7) ? " PER" : "", BIT(data, 6) ? " RTO" : "",
		BIT(data, 5) ? " TTO" : "", BIT(data, 4) ? "" : " INH",
		BIT(data, 3) ? " CMD" : "", BIT(data, 2) ? " SYS" : "",
		BIT(data, 1) ? " IBF" : "", BIT(data, 0) ? " OBF" : "",
		machine().describe_context());
	return data;
}

void at_kbc_device_base::data_w(uint8_t data)
{
	LOG("data_w 0x%02x (%s)\n", data, machine().describe_context());
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(at_kbc_device_base::write_data), this), unsigned(data));
}

void at_kbc_device_base::command_w(uint8_t data)
{
	if (VERBOSE & LOG_COMMAND)
	{
		switch (data)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			LOGMASKED(LOG_COMMAND, "read controller ram indirect 0x%02x (%s)\n", data & 0x3f, machine().describe_context());
			break;

		case 0x20: LOGMASKED(LOG_COMMAND, "read command byte (%s)\n", machine().describe_context()); break;

				   case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b:
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			LOGMASKED(LOG_COMMAND, "read controller ram offset 0x%02x (%s)\n", data & 0x3f, machine().describe_context());
			break;

		case 0x40: case 0x41: case 0x42: case 0x43:
		case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b:
		case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b:
		case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			LOGMASKED(LOG_COMMAND, "write controller ram indirect 0x%02x (%s)\n", data & 0x3f, machine().describe_context());
			break;

		case 0x60: LOGMASKED(LOG_COMMAND, "write command byte (%s)\n", machine().describe_context()); break;

				   case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b:
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			LOGMASKED(LOG_COMMAND, "write controller ram offset 0x%02x (%s)\n", data & 0x3f, machine().describe_context());
			break;

		case 0x90: case 0x91: case 0x92: case 0x93:
		case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b:
		case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			LOGMASKED(LOG_COMMAND, "write output port 0x%1x (%s)\n", data & 0xf, machine().describe_context());
			break;

		case 0xa1: LOGMASKED(LOG_COMMAND, "get version number (%s)\n", machine().describe_context()); break;
		case 0xa4: LOGMASKED(LOG_COMMAND, "test password installed (%s)\n", machine().describe_context()); break;
		case 0xa5: LOGMASKED(LOG_COMMAND, "load security (%s)\n", machine().describe_context()); break;
		case 0xa6: LOGMASKED(LOG_COMMAND, "enable security (%s)\n", machine().describe_context()); break;
		case 0xa7: LOGMASKED(LOG_COMMAND, "disable auxiliary interface (%s)\n", machine().describe_context()); break;
		case 0xa8: LOGMASKED(LOG_COMMAND, "enable auxiliary interface (%s)\n", machine().describe_context()); break;
		case 0xa9: LOGMASKED(LOG_COMMAND, "auxiliary interface test (%s)\n", machine().describe_context()); break;
		case 0xaa: LOGMASKED(LOG_COMMAND, "controller self-test (%s)\n", machine().describe_context()); break;
		case 0xab: LOGMASKED(LOG_COMMAND, "keyboard interface test (%s)\n", machine().describe_context()); break;
		case 0xac: LOGMASKED(LOG_COMMAND, "diagnostic dump (%s)\n", machine().describe_context()); break;
		case 0xad: LOGMASKED(LOG_COMMAND, "disable keyboard interface (%s)\n", machine().describe_context()); break;
		case 0xae: LOGMASKED(LOG_COMMAND, "enable keyboard interface (%s)\n", machine().describe_context()); break;
		case 0xaf: LOGMASKED(LOG_COMMAND, "get version (%s)\n", machine().describe_context()); break;
		case 0xc0: LOGMASKED(LOG_COMMAND, "read input port (%s)\n", machine().describe_context()); break;
		case 0xc1: LOGMASKED(LOG_COMMAND, "poll input port low (%s)\n", machine().describe_context()); break;
		case 0xc2: LOGMASKED(LOG_COMMAND, "poll input port high (%s)\n", machine().describe_context()); break;
		case 0xd0: LOGMASKED(LOG_COMMAND, "read output port (%s)\n", machine().describe_context()); break;
		case 0xd1: LOGMASKED(LOG_COMMAND, "write output port (%s)\n", machine().describe_context()); break;
		case 0xd2: LOGMASKED(LOG_COMMAND, "write keyboard output buffer (%s)\n", machine().describe_context()); break;
		case 0xd3: LOGMASKED(LOG_COMMAND, "write auxiliary output buffer (%s)\n", machine().describe_context()); break;
		case 0xd4: LOGMASKED(LOG_COMMAND, "write auxiliary device (%s)\n", machine().describe_context()); break;
		case 0xe0: LOGMASKED(LOG_COMMAND, "read test inputs (%s)\n", machine().describe_context()); break;

		case 0xf0: case 0xf1: case 0xf2: case 0xf3:
		case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		case 0xf8: case 0xf9: case 0xfa: case 0xfb:
		case 0xfc: case 0xfd: case 0xfe: case 0xff:
			LOGMASKED(LOG_COMMAND, "pulse output port 0x%1x (%s)\n", data & 0xf, machine().describe_context());
			break;

		default:
			LOGMASKED(LOG_COMMAND, "unknown command 0x%02x (%s)\n", data, machine().describe_context());
			break;
		}
	}
	else
		LOG("command_w 0x%02x (%s)\n", data, machine().describe_context());

	machine().scheduler().synchronize(timer_expired_delegate(FUNC(at_kbc_device_base::write_command), this), unsigned(data));
}

void at_kbc_device_base::kbd_clk_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(at_kbc_device_base::set_kbd_clk_in), this), state);
}

void at_kbc_device_base::kbd_data_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(at_kbc_device_base::set_kbd_data_in), this), state);
}

at_kbc_device_base::at_kbc_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_mcu(*this, "mcu")
	, m_hot_res_cb(*this), m_gate_a20_cb(*this), m_kbd_irq_cb(*this)
	, m_kbd_clk_cb(*this), m_kbd_data_cb(*this)
	, m_hot_res(0U), m_gate_a20(0U), m_kbd_irq(0U)
	, m_kbd_clk_in(1U), m_kbd_clk_out(1U), m_kbd_data_in(1U), m_kbd_data_out(1U)
{
}

at_kbc_device_base::~at_kbc_device_base()
{
}

void at_kbc_device_base::device_start()
{
	save_item(NAME(m_hot_res));
	save_item(NAME(m_gate_a20));
	save_item(NAME(m_kbd_irq));
	save_item(NAME(m_kbd_clk_in));
	save_item(NAME(m_kbd_clk_out));
	save_item(NAME(m_kbd_data_in));
	save_item(NAME(m_kbd_data_out));

	m_hot_res = m_gate_a20 = m_kbd_irq = 0U;
	m_kbd_clk_in = m_kbd_clk_out = 1U;
	m_kbd_data_in = m_kbd_data_out = 1U;
}

inline void at_kbc_device_base::set_hot_res(u8 state)
{
	if (state != m_hot_res)
		m_hot_res_cb((m_hot_res = state) ? ASSERT_LINE : CLEAR_LINE);
}

inline void at_kbc_device_base::set_gate_a20(u8 state)
{
	if (state != m_gate_a20)
		m_gate_a20_cb((m_gate_a20 = state) ? ASSERT_LINE : CLEAR_LINE);
}

inline void at_kbc_device_base::set_kbd_irq(u8 state)
{
	if (state != m_kbd_irq)
		m_kbd_irq_cb((m_kbd_irq = state) ? ASSERT_LINE : CLEAR_LINE);
}

inline void at_kbc_device_base::set_kbd_clk_out(u8 state)
{
	if (state != m_kbd_clk_out)
		m_kbd_clk_cb(m_kbd_clk_out = state);
}

inline void at_kbc_device_base::set_kbd_data_out(u8 state)
{
	if (state != m_kbd_data_out)
		m_kbd_data_cb(m_kbd_data_out = state);
}

inline u8 at_kbc_device_base::kbd_clk_r() const
{
	return m_kbd_clk_in & m_kbd_clk_out;
}

inline u8 at_kbc_device_base::kbd_data_r() const
{
	return m_kbd_data_in & m_kbd_data_out;
}

TIMER_CALLBACK_MEMBER(at_kbc_device_base::write_data)
{
	m_mcu->upi41_master_w(0U, u8(u32(param)));
}

TIMER_CALLBACK_MEMBER(at_kbc_device_base::write_command)
{
	m_mcu->upi41_master_w(1U, u8(u32(param)));
}

TIMER_CALLBACK_MEMBER(at_kbc_device_base::set_kbd_clk_in)
{
	m_kbd_clk_in = param ? 1U : 0U;
}

TIMER_CALLBACK_MEMBER(at_kbc_device_base::set_kbd_data_in)
{
	m_kbd_data_in = param ? 1U : 0U;
}


//**************************************************************************
//  PC/AT KEYBOARD CONTROLLER DEVICE
//**************************************************************************

at_keyboard_controller_device::at_keyboard_controller_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: at_kbc_device_base(mconfig, AT_KEYBOARD_CONTROLLER, tag, owner, clock)
{
}

at_keyboard_controller_device::~at_keyboard_controller_device()
{
}

tiny_rom_entry const *at_keyboard_controller_device::device_rom_region() const
{
	return ROM_NAME(at_kbc);
}

void at_keyboard_controller_device::device_add_mconfig(machine_config &config)
{
	I8042AH(config, m_mcu, DERIVED_CLOCK(1, 1));
	m_mcu->p1_in_cb().set_ioport("P1");
	m_mcu->p1_out_cb().set_nop();
	m_mcu->p2_in_cb().set_constant(0xffU);
	m_mcu->p2_out_cb().set(FUNC(at_keyboard_controller_device::p2_w));
	m_mcu->t0_in_cb().set([this] () { return kbd_clk_r(); });
	m_mcu->t1_in_cb().set([this] () { return kbd_data_r(); });
}

ioport_constructor at_keyboard_controller_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(at_kbc);
}

void at_keyboard_controller_device::p2_w(uint8_t data)
{
	set_hot_res(BIT(~data, 0));
	set_gate_a20(BIT(data, 1));
	set_kbd_irq(BIT(data, 4));
	set_kbd_clk_out(BIT(~data, 6));
	set_kbd_data_out(BIT(data, 7));
}


//**************************************************************************
//  PS/2 KEYBOARD/MOUSE CONTROLLER DEVICE
//**************************************************************************

uint8_t ps2_keyboard_controller_device::data_r()
{
	set_kbd_irq(0U);
	set_aux_irq(0U);
	u8 const data = m_mcu->upi41_master_r(0U);
	LOG("data_r 0x%02x (%s)\n", data, machine().describe_context());
	return data;
}

uint8_t ps2_keyboard_controller_device::status_r()
{
	u8 const data = m_mcu->upi41_master_r(1U);
	LOGMASKED(LOG_STATUS, "status_r 0x%02x%s%s%s%s%s%s%s%s (%s)\n", data,
		BIT(data, 7) ? " PER" : "",     BIT(data, 6) ? " GTO" : "",
		BIT(data, 5) ? " AUX_OBF" : "", BIT(data, 4) ? "" : " INH",
		BIT(data, 3) ? " CMD" : "",     BIT(data, 2) ? " SYS" : "",
		BIT(data, 1) ? " IBF" : "",     BIT(data, 0) ? " OBF" : "",
		machine().describe_context());
	return data;
}

void ps2_keyboard_controller_device::aux_clk_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(ps2_keyboard_controller_device::set_aux_clk_in), this), state);
}

void ps2_keyboard_controller_device::aux_data_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(ps2_keyboard_controller_device::set_aux_data_in), this), state);
}

ps2_keyboard_controller_device::ps2_keyboard_controller_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: at_kbc_device_base(mconfig, PS2_KEYBOARD_CONTROLLER, tag, owner, clock)
	, m_aux_irq_cb(*this)
	, m_aux_clk_cb(*this), m_aux_data_cb(*this)
	, m_aux_irq(0U)
	, m_aux_clk_in(1U), m_aux_clk_out(1U), m_aux_data_in(1U), m_aux_data_out(1U)
	, m_p2_data(0xffU)
{
}

ps2_keyboard_controller_device::~ps2_keyboard_controller_device()
{
}

tiny_rom_entry const *ps2_keyboard_controller_device::device_rom_region() const
{
	return ROM_NAME(ps2_kbc);
}

void ps2_keyboard_controller_device::device_add_mconfig(machine_config &config)
{
	I8042AH(config, m_mcu, DERIVED_CLOCK(1, 1));
	m_mcu->p1_in_cb().set(FUNC(ps2_keyboard_controller_device::p1_r));
	m_mcu->p1_out_cb().set_nop();
	m_mcu->p2_in_cb().set_constant(0xffU);
	m_mcu->p2_out_cb().set(FUNC(ps2_keyboard_controller_device::p2_w));
	m_mcu->t0_in_cb().set([this] () { return kbd_clk_r(); });
	m_mcu->t1_in_cb().set([this] () { return aux_clk_r(); });
}

void ps2_keyboard_controller_device::device_start()
{
	at_kbc_device_base::device_start();

	save_item(NAME(m_aux_irq));
	save_item(NAME(m_aux_clk_in));
	save_item(NAME(m_aux_clk_out));
	save_item(NAME(m_aux_data_in));
	save_item(NAME(m_aux_data_out));
	save_item(NAME(m_p2_data));

	m_aux_irq = 0U;
	m_aux_clk_in = m_aux_clk_out = 1U;
	m_aux_data_in = m_aux_data_out = 1U;
	m_p2_data = 0xffU;
}

inline void ps2_keyboard_controller_device::set_aux_irq(u8 state)
{
	if (state != m_aux_irq)
		m_aux_irq_cb((m_aux_irq = state) ? ASSERT_LINE : CLEAR_LINE);
}

inline void ps2_keyboard_controller_device::set_aux_clk_out(u8 state)
{
	if (state != m_aux_clk_out)
		m_aux_clk_cb(m_aux_clk_out = state);
}

inline void ps2_keyboard_controller_device::set_aux_data_out(u8 state)
{
	if (state != m_aux_data_out)
		m_aux_data_cb(m_aux_data_out = state);
}

inline u8 ps2_keyboard_controller_device::aux_clk_r() const
{
	return m_aux_clk_in & m_aux_clk_out;
}

inline u8 ps2_keyboard_controller_device::aux_data_r() const
{
	return m_aux_data_in & m_aux_data_out;
}

TIMER_CALLBACK_MEMBER(ps2_keyboard_controller_device::set_aux_clk_in)
{
	m_aux_clk_in = param ? 1U : 0U;
}

TIMER_CALLBACK_MEMBER(ps2_keyboard_controller_device::set_aux_data_in)
{
	m_aux_data_in = param ? 1U : 0U;
}

uint8_t ps2_keyboard_controller_device::p1_r()
{
	return kbd_data_r() | (aux_data_r() << 1) | 0xfcU;
}

void ps2_keyboard_controller_device::p2_w(uint8_t data)
{
	set_hot_res(BIT(~data, 0));
	set_gate_a20(BIT(data, 1));
	set_aux_data_out(BIT(~data, 2));
	set_aux_clk_out(BIT(~data, 3));
	set_kbd_clk_out(BIT(~data, 6));
	set_kbd_data_out(BIT(~data, 7));

	if (BIT(data & ~m_p2_data, 4))
		set_kbd_irq(1U);
	if (BIT(data & ~m_p2_data, 5))
		set_aux_irq(1U);
	m_p2_data = data;
}
