// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu, Robbbert
/***************************************************************************

        Intel MCS-86 System Design Kit (SDK-86)

        12/05/2009 Skeleton driver by Micko
        29/11/2009 Some fleshing out by Lord Nightmare
        22/06/2011 Working [Robbbert]

    TODO:
    Add optional 2x 8255A port read/write logging



This is an evaluation kit for the 8086 cpu.

There is no speaker or storage facility in the standard kit.

Download the User Manual to get the operating procedures.
The user manual is available from: http://www.bitsavers.org/pdf/intel/8086/9800698A_SDK-86_Users_Man_Apr79.pdf

ToDo:
- Artwork
- Add INTR and RESET keys

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "sdk86.lh"

#define I8251_TAG       "i8251"
#define RS232_TAG       "rs232"


class sdk86_state : public driver_device
{
public:
	sdk86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void sdk86(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(scanlines_w);
	DECLARE_WRITE8_MEMBER(digit_w);
	DECLARE_READ8_MEMBER(kbd_r);

	void sdk86_io(address_map &map);
	void sdk86_mem(address_map &map);

	uint8_t m_digit;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<cpu_device> m_maincpu;
	output_finder<8> m_digits;
};

void sdk86_state::sdk86_mem(address_map &map)
{
	map(0x00000, 0x00fff).ram(); //2K standard, or 4k (board fully populated)
	map(0xfe000, 0xfffff).rom();
}

void sdk86_state::sdk86_io(address_map &map)
{
	map.unmap_value_high();
	map(0xfff0, 0xfff3).mirror(4).rw(I8251_TAG, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xffe8, 0xffeb).mirror(4).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write)).umask16(0x00ff);
	map(0xfff8, 0xffff).rw("port1", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff00);
	map(0xfff8, 0xffff).rw("port2", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
}

/* Input ports */
static INPUT_PORTS_START( sdk86 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 EB/AX") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 ER/BX") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 GO/CX") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 ST/DX") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 IB/SP") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 OB/BP") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 MV/SI") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 EW/DI") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 IW/CS") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 OW/DS") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A SS") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B ES") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C IP") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D FL") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REG") PORT_CODE(KEYCODE_R)
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


WRITE8_MEMBER( sdk86_state::scanlines_w )
{
	m_digit = data;
}

WRITE8_MEMBER( sdk86_state::digit_w )
{
	if (m_digit < 8)
		m_digits[m_digit] = data;
}

READ8_MEMBER( sdk86_state::kbd_r )
{
	uint8_t data = 0xff;

	if (m_digit < 3)
	{
		char kbdrow[6];
		sprintf(kbdrow,"X%X",m_digit);
		data = ioport(kbdrow)->read();
	}
	return data;
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_4800 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_4800 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void sdk86_state::sdk86(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, XTAL(14'745'600)/3); /* divided down by i8284 clock generator; jumper selection allows it to be slowed to 2.5MHz, hence changing divider from 3 to 6 */
	m_maincpu->set_addrmap(AS_PROGRAM, &sdk86_state::sdk86_mem);
	m_maincpu->set_addrmap(AS_IO, &sdk86_state::sdk86_io);

	/* video hardware */
	config.set_default_layout(layout_sdk86);

	/* Devices */
	i8251_device &i8251(I8251(config, I8251_TAG, 0));
	i8251.txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	i8251.dtr_handler().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	i8251.rts_handler().set(I8251_TAG, FUNC(i8251_device::write_cts));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(I8251_TAG, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(I8251_TAG, FUNC(i8251_device::write_dsr));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	clock_device &usart_clock(CLOCK(config, "usart_clock", XTAL(14'745'600)/3/16));
	usart_clock.signal_handler().set(I8251_TAG, FUNC(i8251_device::write_txc));
	usart_clock.signal_handler().append(I8251_TAG, FUNC(i8251_device::write_rxc));

	i8279_device &kbdc(I8279(config, "i8279", 2500000));        // based on divider
	kbdc.out_sl_callback().set(FUNC(sdk86_state::scanlines_w)); // scan SL lines
	kbdc.out_disp_callback().set(FUNC(sdk86_state::digit_w));   // display A&B
	kbdc.in_rl_callback().set(FUNC(sdk86_state::kbd_r));        // kbd RL lines
	kbdc.in_shift_callback().set_constant(0);                   // Shift key
	kbdc.in_ctrl_callback().set_constant(0);

	I8255A(config, "port1");
	I8255A(config, "port2");
}

/* ROM definition */
ROM_START( sdk86 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF ) // all are Intel D2616 ?eproms with the windows painted over? (factory programmed eproms? this would match the 'i8642' marking on the factory programmed eprom version of the AT keyboard mcu...)
	/* Note that the rom pairs at FE000-FEFFF and FF000-FFFFF are
	   interchangeable; the ones at FF000-FFFFF are the ones which start on
	   bootup, and the other ones live at FE000-FEFFF and can be switched in by
	   the user. One pair is the Serial RS232 Monitor and the other is the
	   Keypad/front panel monitor. On the SDK-86 I (LN) dumped, the Keypad
	   monitor was primary, but the other SDK-86 I know of has the roms in
	   the opposite arrangement (Serial primary). */
	// Keypad Monitor Version 1.1 (says "- 86   1.1" on LED display at startup)
	ROM_SYSTEM_BIOS( 0, "keypad", "Keypad Monitor" )
	ROMX_LOAD( "0456_104531-001.a36", 0xfe000, 0x0800, CRC(f9c4a809) SHA1(aea324c3f52dd393f1eed2b856ba11f050a35b93), ROM_SKIP(1) | ROM_BIOS(0) ) /* Label: "iD2616 // T142099WS // (C)INTEL '77 // 0456 // 104531-001" */
	ROMX_LOAD( "0457_104532-001.a37", 0xfe001, 0x0800, CRC(a245ba5c) SHA1(7f67277f866fca5377cb123e9cc405b5fdfe61d3), ROM_SKIP(1) | ROM_BIOS(0) ) /* Label: "iD2616 // T145054WS // (C)INTEL '77 // 0457 // 104532-001" */
	ROMX_LOAD( "0169_102042-001.a27", 0xff000, 0x0800, CRC(3f46311a) SHA1(a97e6861b736f26230b9adbf5cd2576a9f60d626), ROM_SKIP(1) | ROM_BIOS(0) ) /* Label: "iD2616 // T142094WS // (C)INTEL '77 // 0169 // 102042-001" */
	ROMX_LOAD( "0170_102043-001.a30", 0xff001, 0x0800, CRC(65924471) SHA1(5d258695bf585f89179dfa0a113a0eeeabd5ee2b), ROM_SKIP(1) | ROM_BIOS(0) ) /* Label: "iD2616 // T145056WS // (C)INTEL '77 // 0170 // 102043-001" */
	// Serial Monitor Version 1.2 (says "  86   1.2" on LED display at startup, and sends a data prompt over serial)
	ROM_SYSTEM_BIOS( 1, "serial", "Serial Monitor" )
	ROMX_LOAD( "0169_102042-001.a36", 0xfe000, 0x0800, CRC(3f46311a) SHA1(a97e6861b736f26230b9adbf5cd2576a9f60d626), ROM_SKIP(1) | ROM_BIOS(1) ) /* Label: "iD2616 // T142094WS // (C)INTEL '77 // 0169 // 102042-001" */
	ROMX_LOAD( "0170_102043-001.a37", 0xfe001, 0x0800, CRC(65924471) SHA1(5d258695bf585f89179dfa0a113a0eeeabd5ee2b), ROM_SKIP(1) | ROM_BIOS(1) ) /* Label: "iD2616 // T145056WS // (C)INTEL '77 // 0170 // 102043-001" */
	ROMX_LOAD( "0456_104531-001.a27", 0xff000, 0x0800, CRC(f9c4a809) SHA1(aea324c3f52dd393f1eed2b856ba11f050a35b93), ROM_SKIP(1) | ROM_BIOS(1) ) /* Label: "iD2616 // T142099WS // (C)INTEL '77 // 0456 // 104531-001" */
	ROMX_LOAD( "0457_104532-001.a30", 0xff001, 0x0800, CRC(a245ba5c) SHA1(7f67277f866fca5377cb123e9cc405b5fdfe61d3), ROM_SKIP(1) | ROM_BIOS(1) ) /* Label: "iD2616 // T145054WS // (C)INTEL '77 // 0457 // 104532-001" */

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

/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY   FULLNAME  FLAGS */
COMP( 1979, sdk86, 0,      0,      sdk86,   sdk86, sdk86_state, empty_init, "Intel",  "MCS-86 System Design Kit", MACHINE_NO_SOUND_HW)
