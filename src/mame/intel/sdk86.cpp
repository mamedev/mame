// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu
/***************************************************************************

Intel MCS-86 System Design Kit (SDK-86)
This is an evaluation kit for the 8086 cpu.

There is no speaker or storage facility in the standard kit.

Download the User Manual to get the operating procedures.
The user manual is available from: http://www.bitsavers.org/pdf/intel/8086/9800698A_SDK-86_Users_Man_Apr79.pdf

2009-05-12 Skeleton driver by Micko
2009-11-29 Some fleshing out by Lord Nightmare
2011-06-22 Working [Robbbert]


Paste Test:
       N0100^11^22^33^44^55^66^77^88^99^X0100^
       Press UP to verify the data.

ToDo:
- Add optional 2x 8255A port read/write logging

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "sdk86.lh"


namespace {

class sdk86_state : public driver_device
{
public:
	sdk86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
		, m_kdc(*this, "i8279")
		, m_ppi1(*this, "ppi1")
		, m_ppi2(*this, "ppi2")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void sdk86(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(nmi_button);
	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

private:
	void scanlines_w(u8 data);
	void digit_w(u8 data);
	u8 kbd_r();

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_digit = 0U;
	void machine_start() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_uart;
	required_device<i8279_device> m_kdc;
	required_device<i8255_device> m_ppi1;
	required_device<i8255_device> m_ppi2;
	required_ioport_array<3> m_io_keyboard;
	output_finder<8> m_digits;
};

void sdk86_state::mem_map(address_map &map)
{
	map(0x00000, 0x00fff).ram(); //2K standard, or 4k (board fully populated)
	map(0xfe000, 0xfffff).rom().region("maincpu", 0);
}

void sdk86_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0xfff0, 0xfff3).mirror(4).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xffe8, 0xffeb).mirror(4).rw(m_kdc, FUNC(i8279_device::read), FUNC(i8279_device::write)).umask16(0x00ff);
	map(0xfff8, 0xffff).rw(m_ppi1, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff00);
	map(0xfff8, 0xffff).rw(m_ppi2, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
}

/* Input ports */
static INPUT_PORTS_START( sdk86 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 EB/AX") PORT_CODE(KEYCODE_0) PORT_CHAR('0')  // examine/modify bytes
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 ER/BX") PORT_CODE(KEYCODE_1) PORT_CHAR('1')  // examine/modify register
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 GO/CX") PORT_CODE(KEYCODE_2) PORT_CHAR('2')  // go
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 ST/DX") PORT_CODE(KEYCODE_3) PORT_CHAR('3')  // step
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 IB/SP") PORT_CODE(KEYCODE_4) PORT_CHAR('4')  // read a byte from port
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 OB/BP") PORT_CODE(KEYCODE_5) PORT_CHAR('5')  // output byte to port
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 MV/SI") PORT_CODE(KEYCODE_6) PORT_CHAR('6')  // copy memory block
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 EW/DI") PORT_CODE(KEYCODE_7) PORT_CHAR('7')  // examine/modify words

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 IW/CS") PORT_CODE(KEYCODE_8) PORT_CHAR('8')  // read a word from port
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 OW/DS") PORT_CODE(KEYCODE_9) PORT_CHAR('9')  // output a word to port
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A SS") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B ES") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C IP") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D FL") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_ENTER) PORT_CHAR('X')  // end current command
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CODE(KEYCODE_UP) PORT_CHAR('^')  // enter data and increment address
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('M')   // subtract one number from another
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('P')   // add two numbers
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_COLON) PORT_CHAR('S')   // separator between segment and offset
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')     // registers
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INTR") PORT_CODE(KEYCODE_ESC) PORT_CHANGED_MEMBER(DEVICE_SELF, sdk86_state, nmi_button, 0) PORT_CHAR('I')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Systm Reset") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, sdk86_state, reset_button, 0) PORT_CHAR('N')
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(sdk86_state::nmi_button)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

// Reset button connects to i8284, output of this resets all major chips
INPUT_CHANGED_MEMBER(sdk86_state::reset_button)
{
	if (newval)
	{
		m_ppi1->reset();
		m_ppi2->reset();
		m_uart->reset();
		m_kdc->reset();
		m_maincpu->reset();
	}
}

void sdk86_state::scanlines_w(u8 data)
{
	m_digit = data;
}

void sdk86_state::digit_w(u8 data)
{
	if (m_digit < 8)
		m_digits[m_digit] = data;
}

u8 sdk86_state::kbd_r()
{
	u8 data = 0xff;

	if ((m_digit & 7) < 3)
		data = m_io_keyboard[m_digit & 7]->read();

	return data;
}

void sdk86_state::machine_start()
{
	m_digits.resolve();
	save_item(NAME(m_digit));
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_4800 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_4800 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void sdk86_state::sdk86(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, XTAL(14'745'600)/3); /* divided down by i8284 clock generator; jumper selection allows it to be slowed to 2.5MHz, hence changing divider from 3 to 6 */
	m_maincpu->set_addrmap(AS_PROGRAM, &sdk86_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &sdk86_state::io_map);

	/* video hardware */
	config.set_default_layout(layout_sdk86);

	/* Devices */
	I8251(config, m_uart, 0);
	m_uart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_uart->rts_handler().set(m_uart, FUNC(i8251_device::write_cts));

	// it's meant to interface with an intellec unit, and you need floppy disks & drives for that
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(m_uart, FUNC(i8251_device::write_dsr));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	clock_device &usart_clock(CLOCK(config, "usart_clock", XTAL(14'745'600)/3/16));
	usart_clock.signal_handler().set(m_uart, FUNC(i8251_device::write_txc));
	usart_clock.signal_handler().append(m_uart, FUNC(i8251_device::write_rxc));

	I8279(config, m_kdc, 2500000);        // based on divider
	m_kdc->out_sl_callback().set(FUNC(sdk86_state::scanlines_w)); // scan SL lines
	m_kdc->out_disp_callback().set(FUNC(sdk86_state::digit_w));   // display A&B
	m_kdc->in_rl_callback().set(FUNC(sdk86_state::kbd_r));        // kbd RL lines
	m_kdc->in_shift_callback().set_constant(0);                   // Shift key
	m_kdc->in_ctrl_callback().set_constant(0);

	I8255A(config, m_ppi1);
	I8255A(config, m_ppi2);
}

/* ROM definition */
ROM_START( sdk86 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF ) // all are Intel D2616 ?eproms with the windows painted over? (factory programmed eproms? this would match the 'i8642' marking on the factory programmed eprom version of the AT keyboard mcu...)
	/* Note that the rom pairs at FE000-FEFFF and FF000-FFFFF are
	   interchangeable; the ones at FF000-FFFFF are the ones which start on
	   bootup, and the other ones live at FE000-FEFFF and can be switched in by
	   the user. One pair is the Serial RS232 Monitor and the other is the
	   Keypad/front panel monitor. On the SDK-86 I (LN) dumped, the Keypad
	   monitor was primary, but the other SDK-86 I know of has the roms in
	   the opposite arrangement (Serial primary). */
	// Keypad Monitor Version 1.1 (says "- 86   1.1" on LED display at startup)
	ROM_SYSTEM_BIOS( 0, "keypad", "Keypad Monitor" )
	ROMX_LOAD( "0456_104531-001.a36", 0x0000, 0x0800, CRC(f9c4a809) SHA1(aea324c3f52dd393f1eed2b856ba11f050a35b93), ROM_SKIP(1) | ROM_BIOS(0) ) /* Label: "iD2616 // T142099WS // (C)INTEL '77 // 0456 // 104531-001" */
	ROMX_LOAD( "0457_104532-001.a37", 0x0001, 0x0800, CRC(a245ba5c) SHA1(7f67277f866fca5377cb123e9cc405b5fdfe61d3), ROM_SKIP(1) | ROM_BIOS(0) ) /* Label: "iD2616 // T145054WS // (C)INTEL '77 // 0457 // 104532-001" */
	ROMX_LOAD( "0169_102042-001.a27", 0x1000, 0x0800, CRC(3f46311a) SHA1(a97e6861b736f26230b9adbf5cd2576a9f60d626), ROM_SKIP(1) | ROM_BIOS(0) ) /* Label: "iD2616 // T142094WS // (C)INTEL '77 // 0169 // 102042-001" */
	ROMX_LOAD( "0170_102043-001.a30", 0x1001, 0x0800, CRC(65924471) SHA1(5d258695bf585f89179dfa0a113a0eeeabd5ee2b), ROM_SKIP(1) | ROM_BIOS(0) ) /* Label: "iD2616 // T145056WS // (C)INTEL '77 // 0170 // 102043-001" */
	// Serial Monitor Version 1.2 (says "  86   1.2" on LED display at startup, and sends a data prompt over serial)
	ROM_SYSTEM_BIOS( 1, "serial", "Serial Monitor" )
	ROMX_LOAD( "0169_102042-001.a36", 0x0000, 0x0800, CRC(3f46311a) SHA1(a97e6861b736f26230b9adbf5cd2576a9f60d626), ROM_SKIP(1) | ROM_BIOS(1) ) /* Label: "iD2616 // T142094WS // (C)INTEL '77 // 0169 // 102042-001" */
	ROMX_LOAD( "0170_102043-001.a37", 0x0001, 0x0800, CRC(65924471) SHA1(5d258695bf585f89179dfa0a113a0eeeabd5ee2b), ROM_SKIP(1) | ROM_BIOS(1) ) /* Label: "iD2616 // T145056WS // (C)INTEL '77 // 0170 // 102043-001" */
	ROMX_LOAD( "0456_104531-001.a27", 0x1000, 0x0800, CRC(f9c4a809) SHA1(aea324c3f52dd393f1eed2b856ba11f050a35b93), ROM_SKIP(1) | ROM_BIOS(1) ) /* Label: "iD2616 // T142099WS // (C)INTEL '77 // 0456 // 104531-001" */
	ROMX_LOAD( "0457_104532-001.a30", 0x1001, 0x0800, CRC(a245ba5c) SHA1(7f67277f866fca5377cb123e9cc405b5fdfe61d3), ROM_SKIP(1) | ROM_BIOS(1) ) /* Label: "iD2616 // T145054WS // (C)INTEL '77 // 0457 // 104532-001" */

	/* proms:
	 * dumped 11/21/09 through 11/29/09 by LN
	 * purposes: (according to sdk-86 user manual from http://www.bitsavers.org/pdf/intel/8086/9800698A_SDK-86_Users_Man_Apr79.pdf)
	 * A12: main address decoding (selects ram or rom or open bus/offboard, see page 2-7)
	 * A22: I/O decoding for 8251, 8279 and optional pair of 8255 chips (in the FFE8-FFFF I/O area; see page 2-6)
	 * A26: ROM address decoding for selecting which of the 4 pairs of roms is active (note that to use the FCxxx and FDxxx pairs requires wiring them into the prototype area, they are not standard; see page 2-5)
	 * A29: RAM address decoding (see page 2-4)
	 */
	ROM_REGION(0x1000, "proms", 0 ) // all are Intel D3625A 1kx4 (82s137A equivalent)
	ROM_LOAD( "0036_101993-001.a12", 0x0000, 0x0400, CRC(bb7edbfd) SHA1(8847f9815c7cb8695986743199673920a7d4390d)) /* Label: "iD3625A 0036 // 8142 // 101993-001" */
	ROM_LOAD( "0035_101992-001.a22", 0x0400, 0x0400, CRC(76aced0c) SHA1(89fa39473e19d8cb6b65d6430d3d683ae2398fb3)) /* Label: "iD3625A 0035 // 8142 // 101992-001" */
	ROM_LOAD( "0037_101994-001.a26", 0x0800, 0x0400, CRC(d6f33d30) SHA1(41e794bf202266fa57516403e6a80ebbf6c95fdc)) /* Label: "iD3625A 0037 // 8142 // 101994-001" */
	ROM_LOAD( "0038_101995-001.a29", 0x0C00, 0x0400, CRC(3d2c18bc) SHA1(5e1935cd07fef26b2cf3d8fa7612fe0d8e678c06)) /* Label: "iD3625A 0038 // 8142 // 101995-001" */
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY   FULLNAME  FLAGS */
COMP( 1979, sdk86, 0,      0,      sdk86,   sdk86, sdk86_state, empty_init, "Intel",  "MCS-86 System Design Kit", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
