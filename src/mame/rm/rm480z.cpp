// license:BSD-3-Clause
// copyright-holders:Robin Sergeant
/*

Research Machines 480Z (aka "RML 480Z" or "Link 480Z")
Microcomputer produced by Research Machines Limited, Oxford, UK
1982-1985

===

From the Firmware Manual:

List of known firmware revisions by sign-on message:

RML 40-Character LINK 480Z V1.0
RML 80-Character LINK 480Z V1.0
RML 40-Character LINK 480Z V1.1 A
RML 40-Character LINK 480Z V1.1 B
RML 80-Character LINK 480Z V1.1 A
RML 80-Character LINK 480Z V1.1 B
RML 80-Character LINK 480Z V1.2 A
RML 80-Character LINK 480Z V1.2 B
RML 80-Character LINK 480Z V1.2 C
RML 80-Character LINK 480Z V1.2 D
RML 80-Character LINK 480Z V2.2 B

V1.0 refers to ROS 1.0 (Mk1 harwdare)
V1.1 refers to ROS 1.1
V1.2 refers to ROS 1.2
V2.2 refers to ROS 2.2 (Mk2 hardware)

The Mk1 hardware uses 4 x 8K EEPROMs, whereas Mk2 has 2 x 16K
(memory maps differ slighly due to these differences).

Monitor commands:
B - Boot CP/M  (ROS 1.2, ROS 2.2)
X - Boot CP/M from another drive (as above)
N - Boot network
T - Enter terminal mode
L - Load program from cassette
D - Dump memory to cassette
C - Continue program at restart address
J - Go to address
O - Select printer option and cassette speed
W - Select 40 or 80 characters per line (All 80-column machines)
R - Start ROM BASIC (ROS 1.1, ROS 1.2, ROS 2.2)
Ctrl+Shift+8 - Break and return to current OS
Ctrl+Shift+9 - Break and return to front panel
Ctrl+F - Enter Front Panel (=the debugger)
Ctrl+T - Enter Typewriter mode
Ctrl+S - Autopaging on
Ctrl+Q - Autopaging off
Ctrl+A - Toggle autopaging

Graphics characters: These are low-res (2x3 TRS80-style) from 80-BF, repeated at C0-FF.
80-BF will be low-intensity, except if using a colour monitor.
The characters 00-1F have one set for COS 3.4 and COS 4.0, and a different set for the others.
ROS 2.2 allows an alternate character set.

Sound:
RM480Z has the speaker fitted

===

4 different memory maps are used (see memory map functions for layout):

Page 0 is used for start-up and some ROS functions
Page 1 is designed for running CP/M
Page 2 is used to run BIR (Basic In Rom)
Page 3 contains only RAM (no known uses)

Up to 256K of RAM can be used with bank switching
The 64K address space is divided into 4 banks, each of which can be configured to
address a particular 16K chunk of physical RAM.

Page selection and bank switching is performed by writes to control ports.

Video resolution:
80x24 - 8 pixels wide, 10 pixels high = 640x240
Video input clock is 16MHz

Additional HRG video modes:

640 x 192 (1 bit per pixel, monochrome)
320 x 192 (2 bits per pixel, colour)
160 x 96 (4 bits per pixel, colour)

HRG occupies the top portion of the screen, with 4 lines of video (text) output below.
The video (text) display is drawn over any HRG output (text can overlay graphics).

===

Interrupt driven keyboard, with repeat key (non ASCII scan codes).

No built in FDC, but external floppy drives can be connected via RS232 port.

Could also network boot if connected to a CHAIN LAN, where a 380Z would act as a file server.

Programs could also be loaded from Cassette and rompacks.

===

TODO:

- Add cassette support
- Add rompack support (parallel port EEPROM cartridges)
- Add support for option hardware (if software supporting it is available).
- Save states

*/

#include "emu.h"
#include "rm480z.h"
#include "rm_mq2.h"
#include "machine/clock.h"
#include "speaker.h"
#include "screen.h"
#include "utf8.h"

void rm480z_state::rm480z_MK1_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankrw(m_bank[0]);
	map(0x4000, 0x7fff).bankrw(m_bank[1]);
	map(0x8000, 0xbfff).bankrw(m_bank[2]);
	map(0xc000, 0xffff).bankrw(m_bank[3]);

	map(0x0000, 0xf7ff).view(m_view);
	// page 0 (for start-up)
	m_view[0](0x0000, 0x07ff).rom().region("ros", 0x0000);
	m_view[0](0x0800, 0x17ff).rom().region("bir0", 0x0800);
	m_view[0](0x1800, 0x1fff).rom().region("bir1", 0x1800);
	m_view[0](0x3800, 0x3fff).rom().region("ros", 0x1800);
	m_view[0](0xe800, 0xf7ff).rom().region("ros", 0x0800);
	// page 1 (for running CP/M)
	m_view[1](0xe800, 0xf7ff).rom().region("ros", 0x0800);
	//page 2 (for running BIR)
	m_view[2](0x9800, 0x9fff).rom().region("bir1", 0x1800);
	m_view[2](0xa000, 0xbfff).rom().region("bir2", 0x0000);	
	m_view[2](0xc000, 0xd7ff).rom().region("bir1", 0x0000);
	m_view[2](0xd800, 0xdfff).rom().region("bir0", 0x1800);
	m_view[2](0xe000, 0xe7ff).rom().region("bir0", 0x0000);
	m_view[2](0xe800, 0xf7ff).rom().region("ros", 0x0800);
}

void rm480z_state::rm480z_MK2_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankrw(m_bank[0]);
	map(0x4000, 0x7fff).bankrw(m_bank[1]);
	map(0x8000, 0xbfff).bankrw(m_bank[2]);
	map(0xc000, 0xffff).bankrw(m_bank[3]);

	map(0x0000, 0xf7ff).view(m_view);
	// page 0 (for start-up)
	m_view[0](0x0000, 0x07ff).rom().region("rom0", 0x0000);
	m_view[0](0x0800, 0x1fff).rom().region("rom1", 0x0800);
	m_view[0](0x7800, 0x7fff).rom().region("rom0", 0x3800);
	m_view[0](0xe800, 0xf7ff).rom().region("rom0", 0x2800);
	// page 1 (for running CP/M)
	m_view[1](0xe800, 0xf7ff).rom().region("rom0", 0x2800);
	//page 2 (for running BIR)
	m_view[2](0x9800, 0xbfff).rom().region("rom1", 0x1800);
	m_view[2](0xc000, 0xc7ff).rom().region("rom1", 0x0000);
	m_view[2](0xc800, 0xe7ff).rom().region("rom0", 0x0800);
	m_view[2](0xe800, 0xf7ff).rom().region("rom0", 0x2800);
}

void rm480z_state::rm480z_io(address_map &map)
{
	map(0x00, 0x17).select(0x7f00).rw(FUNC(rm480z_state::videoram_read), FUNC(rm480z_state::videoram_write));
	map(0x18, 0x1d).select(0xff00).rw(FUNC(rm480z_state::status_port_read), FUNC(rm480z_state::control_port_write));
	map(0x20, 0x23).mirror(0xff00).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x24, 0x27).mirror(0xff00).rw(m_sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x38, 0x3b).mirror(0xff00).rw(FUNC(rm480z_state::hrg_port_read), FUNC(rm480z_state::hrg_port_write));
	//map(0x28, 0x29).mirror(0xff02); // am9511/am9512 maths chip // option
	//map(0x2c, 0x2f).mirror(0xff00); // z80ctc IEEE int, Maths int, RTC, RTC // option
	//map(0x30, 0x37).mirror(0xff00); // IEEE chip // option
}

INPUT_PORTS_START( rm480z )
	PORT_START("kbrow.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("m M")             PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <")             PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("i I")             PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 )")             PORT_CODE(KEYCODE_9)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("k K")             PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("l L")             PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 _")             PORT_CODE(KEYCODE_0)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("o O")             PORT_CODE(KEYCODE_O)

	PORT_START("kbrow.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("x X")             PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("c C")             PORT_CODE(KEYCODE_C)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("w W")             PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #")             PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("s S")             PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("d D")             PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $")             PORT_CODE(KEYCODE_4)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("e E")             PORT_CODE(KEYCODE_E)

	PORT_START("kbrow.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("v V")             PORT_CODE(KEYCODE_V)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("b B")             PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("r R")             PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %")             PORT_CODE(KEYCODE_5)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("f F")             PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("g G")             PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 &")             PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("t T")             PORT_CODE(KEYCODE_T)

	PORT_START("kbrow.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift")           PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Repeat")          PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {")             PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |")            PORT_CODE(KEYCODE_BACKSLASH) PORT_CODE(KEYCODE_BACKSLASH2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }")             PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Line Feed")       PORT_CODE(KEYCODE_MENU)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")          PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Carriage Return") PORT_CODE(KEYCODE_ENTER)

	PORT_START("kbrow.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space")           PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("n N")             PORT_CODE(KEYCODE_N)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("y Y")             PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 '")             PORT_CODE(KEYCODE_7)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("h H")             PORT_CODE(KEYCODE_H)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("j J")             PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 (")             PORT_CODE(KEYCODE_8)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("u U")             PORT_CODE(KEYCODE_U)

	PORT_START("kbrow.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")         PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("z Z")             PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("q Q")             PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !")             PORT_CODE(KEYCODE_1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock")       PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("a A")             PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 \"")            PORT_CODE(KEYCODE_2)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Escape")          PORT_CODE(KEYCODE_ESC)

	PORT_START("kbrow.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3")              PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)         PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)        PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)           PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4")              PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)         PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2")              PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1")              PORT_CODE(KEYCODE_F1)

	PORT_START("kbrow.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >")             PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?")             PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("p P")             PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- =")             PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; +")             PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(": *")             PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("^ ~")             PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("@ `")             PORT_CODE(KEYCODE_ASTERISK)

	PORT_START("display_type")
	PORT_CONFNAME( 0x01, 0x00, "Monitor" ) PORT_CHANGED_MEMBER(DEVICE_SELF, rm480z_state, monitor_changed, 0)
	PORT_CONFSETTING( 0x00, "TTL RGB Colour Monitor" )
	PORT_CONFSETTING( 0x01, "Monochrome b/w Monitor" )
INPUT_PORTS_END

uint32_t rm480z_state::screen_update_rm480z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// blank screen
	bitmap.fill(0);

	update_screen(bitmap);

	return 0;
}

static void rm480z_default_rs232_devices(device_slot_interface &device)
{
	device.option_add("rm_mq2", RM_MQ2);
}

static const z80_daisy_config daisy_chain[] =
{
	{ "sio" },
	{ "ctc" },
	{ nullptr }
};

void rm480z_state::rm480z(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &rm480z_state::rm480z_MK2_mem);
	m_maincpu->set_addrmap(AS_IO, &rm480z_state::rm480z_io);
	m_maincpu->set_daisy_config(daisy_chain);

	Z80SIO(config, m_sio, 16_MHz_XTAL / 4);
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_sio->out_txdb_callback().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(m_rs232, FUNC(rs232_port_device::write_rts));

	// rs232 port for floppy drive
	RS232_PORT(config, m_rs232, rm480z_default_rs232_devices, "rm_mq2");
	m_rs232->rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	m_rs232->dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));
	m_rs232->dsr_handler().set(m_sio, FUNC(z80sio_device::syncb_w));
	m_rs232->cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));

	Z80CTC(config, m_ctc, 16_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	CLOCK(config, "ctc_clock", 16_MHz_XTAL / 8).signal_handler().set(m_ctc, FUNC(z80ctc_device::trg0));

	/* video hardware */
	PALETTE(config, m_palette).set_init(FUNC(rm480z_state::palette_init)).set_entries(19);
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(rm480z_state::screen_update_rm480z));
	m_screen->set_palette(m_palette);
	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 240);
	m_screen->register_vblank_callback(vblank_state_delegate(&rm480z_state::vblank_callback, this));

	// keyboard is clocked by 500 kHz oscillator divided to give a 15625 Hz scan rate
	TIMER(config, "kbd_scan").configure_periodic(FUNC(rm480z_state::kbd_scan), attotime::from_hz(500_kHz_XTAL / 32));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.80);

	/* RAM configurations */
	RAM(config, RAM_TAG).set_default_size("256K");
}

void rm480z_state::rm480za(machine_config &config)
{
	rm480z(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &rm480z_state::rm480z_MK1_mem);
}

/* ROM definitions */

ROM_START( rm480z )
	ROM_REGION( 0x4000, "rom0", 0 )
	ROM_LOAD( "fv2.0_0_12099_19.2.86.ic83", 0x0000, 0x4000, CRC(a0f02d8a) SHA1(1c063b842699dc0ad85a5a5f337f2864497f9c0f) )
	ROM_REGION( 0x4000, "rom1", 0 )
	ROM_LOAD( "fv2.0_1_12100_27.2.86.ic93", 0x0000, 0x4000, CRC(2a93ca6e) SHA1(7fdd772d4251dbf951a687d184ed787cfe21212b) )
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "cg06_12098_28.2.86.ic98",    0x0000, 0x2000, CRC(15d40f7e) SHA1(a7266357eb9be849f77a97ff3013b236c0af8289) )
ROM_END

ROM_START( rm480za )
	ROM_REGION( 0x2000, "ros", 0 )
	ROM_DEFAULT_BIOS("1.2d")
	ROM_SYSTEM_BIOS(0, "1.2b", "ROS 1.2B")
	ROMX_LOAD( "ros_1.2b.ls",   0x0000, 0x2000, CRC(37e93287) SHA1(c96d4b7eedadb0fb8e3732b6ba3e898e123c393f), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "1.2d", "ROS 1.2D")
	ROMX_LOAD( "ros-1-2d.bin",  0x0000, 0x2000, CRC(3fe61618) SHA1(ee4d70694489ab7f123e59d73b304d6d5fcd8a81), ROM_BIOS(1) )

	ROM_REGION( 0x2000, "bir0", 0 )
	ROM_LOAD( "bir5-4-0.bin",  0x0000, 0x2000, CRC(51875c95) SHA1(96bb058512a0f21634e629229effc6b36d0f0a7a) )
	ROM_REGION( 0x2000, "bir1", 0 )
	ROM_LOAD( "bir5-4-1.bin",  0x0000, 0x2000, CRC(63959245) SHA1(2e42453ce281fd6cc2de176ff98f0a326d3ae8a8) )
	ROM_REGION( 0x2000, "bir2", 0 )
	ROM_LOAD( "bir5-4-2.bin",  0x0000, 0x2000, CRC(d3eb07cf) SHA1(9e576e8d2ae571319dc6c1cb035f13cf56abf690) )	

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "cg06.lq",        0x0000, 0x2000, BAD_DUMP CRC(15d40f7e) SHA1(a7266357eb9be849f77a97ff3013b236c0af8289) ) // chip is marked CG05, might not be the same, so marked as bad
ROM_END

/* Driver */
//   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT                       COMPANY              FULLNAME                FLAGS
COMP(1981, rm480z,  0,      0,      rm480z,  rm480z, rm480z_state, driver_device::empty_init, "Research Machines", "LINK RM-480Z (set 1)", 0)
COMP(1981, rm480za, rm480z, 0,      rm480za, rm480z, rm480z_state, driver_device::empty_init, "Research Machines", "LINK RM-480Z (set 2)", 0)
