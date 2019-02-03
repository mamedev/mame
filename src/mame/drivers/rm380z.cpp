// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Gabriele D'Antona
/*

Research Machines 380Z (aka "RML 380Z" or "RM 380Z")
Microcomputer produced by Research Machines Limited, Oxford, UK
1978-1985
MESS driver by Wilbert Pol and friol (dantonag (at) gmail.com)
Driver started on 22/12/2011

Stefano Bodrato, 09/12/2016 - skeleton for cassette support
True tape samples are needed to continue !

===

From the Firmware Manual:

List of known firmware revisions by sign-on message:
*** RM380Z ***
COS 3.0/C
COS 3.0/F
COS 3.0/M
COS 3.4C/C
COS 3.4C/F
COS 3.4C/M
COS 3.4D/C
COS 3.4D/M
COS 3.4E/F
COS 4.0/F
COS 4.0A/F
COS 4.0A/M
COS 4.0B/M
COS 4.2 A

/C = Cassette
/F = 8 inch single density floppy
/M = 5.25 inch single density floppy
Ver 4.2 can have either floppy type

*** RM480Z ***
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

V1.0 refers to ROS 1.0
V1.1 refers to ROS 1.1
V1.2 refers to ROS 1.2
V2.2 refers to ROS 2.2

Monitor commands:
B - Boot CP/M  (COS /F, COS /M, ROS 1.2, ROS 2.2)
X - Boot CP/M from another drive (as above)
N - Boot network (ROS)
T - Enter terminal mode (ROS)
L - Load program from cassette (COS /C, ROS)
D - Dump memory to cassette (as above)
C - Continue program at restart address (as above)
J - Go to address
O - Select printer option (and cassette speed for COS /C, ROS)
M - Enable HRG board as memory (COS 3.4, COS 4.0)
W - Select 40 or 80 characters per line (All 80-column machines)
R - Start ROM BASIC (ROS 1.1, ROS 1.2, ROS 2.2)
Ctrl+Shift+8 - Break and return to current OS (ROS)
Ctrl+Shift+9 - Break and return to front panel (ROS)
Ctrl+F - Enter Front Panel (=the debugger)
Ctrl+T - Enter Typewriter mode
Ctrl+S - Autopaging on
Ctrl+Q - Autopaging off
Ctrl+A - Toggle autopaging

Graphics characters: These are low-res (2x3 TRS80-style) from 80-BF, repeated at C0-FF.
80-BF will be low-intensity, except for a RM480Z if using a colour monitor.
The characters 00-1F have one set for COS 3.4 and COS 4.0, and a different set for the others.
COS 4.0 and 4.2 allow one to redefine the 80-FF character range, and to have attributes.
ROS 2.2 allows an alternate character set.

Sound:
RM380Z has a connector for a speaker
RM480Z has the speaker fitted

===

Memory map from sevice manual:

PAGE SEL bit in PORT0 set to 0:

  0000-3BFF - CPU RAM row 1 (15KB!)
  3C00-7BFF - CPU RAM row 2 (16KB)
  7C00-BBFF - Add-on RAM row 1 or HRG RAM (16KB)
  BC00-DFFF - Add-on RAM row 2 (9KB!)
  E000-EFFF - ROM (COS)
  F000-F5FF - VDU and HRG Video RAM
  F600-F9FF - ROM (monitor extension)
  FA00-FAFF - Reserved, RAM?
  FB00-FBFF - Memory-mapped ports (FBFC-FBFF)
  FC00-FFFF - RAM

PAGE SEL bit in PORT0 set to 1:
  0000-0FFF - ROM (COS mirror from E000)
  1B00-1BFF - Memory-mapped ports (1BFC-1BFF)
  1C00-1DFF - ROM
  4000-7FFF - CPU RAM row 1 (16KB!, this RAM normally appears at 0000)
  8000-BFFF - ???
  C000-DFFF - ???
  E000-EFFF - ROM (COS)
  F000-F5FF - VDU and HRG Video RAM
  F600-F9FF - ROM (monitor extension)
  FA00-FAFF - Reserved, RAM?
  FB00-FBFF - Memory-mapped ports (FBFC-FBFF)
  FC00-FFFF - RAM

Video resolution (not confirmed):
80x24 - 6 pixels wide (5 + spacing), 10 pixels high (9 + spacing) = 480x240
Video input clock is 16MHz

According to the manuals, VDU-1 chargen is Texas 74LS262.

===

Notes on COS 4.0 disassembly:

- routine at 0xe438 is called at startup in COS 4.0 and it sets the RST vectors in RAM
- routine at 0xe487 finds "top" of system RAM and stores it in 0x0006 and 0x000E
- 0xeca0 - outputs a string (null terminated) to screen (?)
- 0xff18 - does char output to screen (char in A?)

===

TODO:

- Properly implement "backwards" or "last 4 lines" scrolling
- Properly implement dimming and graphic chars (>0x80)
- Understand why any write to disk command fails with "bad sector"
- Understand why ctrl-U (blinking cursor) in COS 4.0 stops keyboard input from working
- Get a reliable ROM dump and charset ROM dump


Attempt to register save state entry after state registration is closed!
Module timer tag static_vblank_timer name m_param
Attempt to register save state entry after state registration is closed!
Module timer tag static_vblank_timer name m_enabled
Attempt to register save state entry after state registration is closed!
Module timer tag static_vblank_timer name m_period.attoseconds
Attempt to register save state entry after state registration is closed!
Module timer tag static_vblank_timer name m_period.seconds
Attempt to register save state entry after state registration is closed!
Module timer tag static_vblank_timer name m_start.attoseconds
Attempt to register save state entry after state registration is closed!
Module timer tag static_vblank_timer name m_start.seconds
Attempt to register save state entry after state registration is closed!
Module timer tag static_vblank_timer name m_expire.attoseconds
Attempt to register save state entry after state registration is closed!
Module timer tag static_vblank_timer name m_expire.seconds
':maincpu' (E48B): unmapped program memory write to E000 = C1 & FF
':maincpu' (E48E): unmapped program memory write to E000 = 3E & FF

*/


#include "emu.h"
#include "includes/rm380z.h"

#include "emupal.h"
#include "screen.h"


void rm380z_state::rm380z_mem(address_map &map)
{
	map(0xe000, 0xefff).rom().region(RM380Z_MAINCPU_TAG, 0);
	map(0xf000, 0xf5ff).rw(FUNC(rm380z_state::videoram_read), FUNC(rm380z_state::videoram_write));
	map(0xf600, 0xf9ff).rom().region(RM380Z_MAINCPU_TAG, 0x1000);     /* Extra ROM space for COS4.0 */
	map(0xfa00, 0xfaff).ram();
	map(0xfb00, 0xfbff).rw(FUNC(rm380z_state::port_read), FUNC(rm380z_state::port_write));
	map(0xfc00, 0xffff).rw(FUNC(rm380z_state::hiram_read), FUNC(rm380z_state::hiram_write));
}

void rm380z_state::rm380z_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xbf).rw(FUNC(rm380z_state::rm380z_portlow_r), FUNC(rm380z_state::rm380z_portlow_w));
	map(0xc0, 0xc3).rw(m_fdc, FUNC(fd1771_device::read), FUNC(fd1771_device::write));
	map(0xc4, 0xc4).w(FUNC(rm380z_state::disk_0_control));
	map(0xc5, 0xff).rw(FUNC(rm380z_state::rm380z_porthi_r), FUNC(rm380z_state::rm380z_porthi_w));
}

void rm380z_state::rm480z_mem(address_map &map)
{
	map(0x0000, 0xe7ff).ram();
	map(0xe800, 0xf7ff).rom().region(RM380Z_MAINCPU_TAG, 0);
	map(0xf800, 0xffff).ram();
}

void rm380z_state::rm480z_io(address_map &map)
{
	//AM_RANGE(0x00, 0x17) AM_RAM // videoram
	//AM_RANGE(0x18, 0x18) AM_MIRROR(0xff00) // control port 0
	//AM_RANGE(0x19, 0x19) AM_MIRROR(0xff00) // control port 1
	//AM_RANGE(0x1a, 0x1a) AM_MIRROR(0xff00) // control port 2
	//AM_RANGE(0x1b, 0x1b) AM_MIRROR(0xff00) // control port 3 (DAC) // option
	//AM_RANGE(0x1d, 0x1d) AM_MIRROR(0xff00) // control port 5 (USERIO) // option
	//AM_RANGE(0x20, 0x23) AM_MIRROR(0xff00) // system CTC - 0=SIO4&cassin, 1=SIO2&cassio, 2=keybd int, 3=50hz int for repeat key
	//AM_RANGE(0x24, 0x27) AM_MIRROR(0xff00) // system SIO - 0=chA network data, 1=chB SIO4 data, 2=ChA control, 3=ChB control
	//AM_RANGE(0x28, 0x29) AM_MIRROR(0xff02) // am9511/am9512 maths chip // option
	//AM_RANGE(0x2c, 0x2f) AM_MIRROR(0xff00) // z80ctc IEEE int, Maths int, RTC, RTC // option
	//AM_RANGE(0x30, 0x37) AM_MIRROR(0xff00) // IEEE chip // option
	//AM_RANGE(0x38, 0x3b) AM_MIRROR(0xff00) // Hi-res graphics option
}

INPUT_PORTS_START( rm380z )
//  PORT_START("additional_chars")
//  PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Escape") PORT_CODE(KEYCODE_ESC) PORT_CODE(KEYCODE_ESC)
INPUT_PORTS_END

//
//
//

static void rm380z_floppies(device_slot_interface &device)
{
	device.option_add("sssd", FLOPPY_525_SSSD);
}

uint32_t rm380z_state::screen_update_rm380z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	update_screen(bitmap);
	return 0;
}

MACHINE_CONFIG_START(rm380z_state::rm380z)
	/* basic machine hardware */
	MCFG_DEVICE_ADD(RM380Z_MAINCPU_TAG, Z80, 16_MHz_XTAL / 4)
	MCFG_DEVICE_PROGRAM_MAP(rm380z_mem)
	MCFG_DEVICE_IO_MAP(rm380z_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	// according to videos and pictures of the real hardware, chars are spaced of at least 1 pixel
	// and there is at least 1 pixel between each row of characters
	MCFG_SCREEN_SIZE((RM380Z_SCREENCOLS*(RM380Z_CHDIMX+1)), (RM380Z_SCREENROWS*(RM380Z_CHDIMY+1)))
	MCFG_SCREEN_VISIBLE_AREA(0, (RM380Z_SCREENCOLS*(RM380Z_CHDIMX+1))-1, 0, (RM380Z_SCREENROWS*(RM380Z_CHDIMY+1))-1)
	MCFG_SCREEN_UPDATE_DRIVER(rm380z_state, screen_update_rm380z)
	MCFG_SCREEN_PALETTE("palette")

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* cassette */
	CASSETTE(config, m_cassette);
//  m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED);
	//m_cassette->change_state((BIT(data,x)) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);

	/* RAM configurations */
	RAM(config, RAM_TAG).set_default_size("56K");

	/* floppy disk */
	FD1771(config, m_fdc, 1_MHz_XTAL);

	FLOPPY_CONNECTOR(config, "wd1771:0", rm380z_floppies, "sssd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, "wd1771:1", rm380z_floppies, "sssd", floppy_image_device::default_floppy_formats);

	/* keyboard */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(rm380z_state::keyboard_put));
MACHINE_CONFIG_END

MACHINE_CONFIG_START(rm380z_state::rm480z)
	/* basic machine hardware */
	MCFG_DEVICE_ADD(RM380Z_MAINCPU_TAG, Z80, 16_MHz_XTAL / 4)
	MCFG_DEVICE_PROGRAM_MAP(rm480z_mem)
	MCFG_DEVICE_IO_MAP(rm480z_io)

	MCFG_MACHINE_RESET_OVERRIDE(rm380z_state, rm480z)
	/* video hardware */
//  MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(50)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
//  MCFG_SCREEN_SIZE((RM380Z_SCREENCOLS*(RM380Z_CHDIMX+1)), (RM380Z_SCREENROWS*(RM380Z_CHDIMY+1)))
//  MCFG_SCREEN_VISIBLE_AREA(0, (RM380Z_SCREENCOLS*(RM380Z_CHDIMX+1))-1, 0, (RM380Z_SCREENROWS*(RM380Z_CHDIMY+1))-1)
//  MCFG_SCREEN_UPDATE_DRIVER(rm380z_state, screen_update_rm480z)
//  MCFG_SCREEN_PALETTE("palette")

//  MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* keyboard */
//  generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
//  keyboard.set_keyboard_callback(FUNC(rm380z_state::keyboard_put));
MACHINE_CONFIG_END


/* ROM definitions */

ROM_START( rm380z34d ) // COS 3.4D/F
	ROM_REGION( 0x10000, RM380Z_MAINCPU_TAG, 0 )
	ROM_LOAD( "cos34d-f.bin", 0x0000, 0x1000, CRC(eb128b40) SHA1(c46f358fb76459987e41750d052995563f2f7d53))
	// chargen ROM is undumped, afaik
	ROM_REGION( 0x1680, "chargen", 0 )
	ROM_LOAD( "ch3.raw", 0x0000, 0x1680, BAD_DUMP CRC(c223622b) SHA1(185ef24896419d7ff46f71a760ac217de3811684))
ROM_END

ROM_START( rm380z34e ) // COS 3.4E/M
	ROM_REGION( 0x10000, RM380Z_MAINCPU_TAG, 0 )
	ROM_LOAD( "cos34e-m.bin", 0x0000, 0x1000, CRC(20e2ddf4) SHA1(3177b28793d5a348c94fd0ae6393d74e2e9a8662))
	// chargen ROM is undumped, afaik
	ROM_REGION( 0x1680, "chargen", 0 )
	ROM_LOAD( "ch3.raw", 0x0000, 0x1680, BAD_DUMP CRC(c223622b) SHA1(185ef24896419d7ff46f71a760ac217de3811684))
ROM_END

ROM_START( rm380z ) // COS 4.0B/M
	ROM_REGION( 0x10000, RM380Z_MAINCPU_TAG, 0 )
	// I'm not sure of how those roms have been dumped. I don't know if those are good dumps or not.
	ROM_LOAD( "cos40b-m.bin", 0x0000, 0x1000, BAD_DUMP CRC(1f0b3a5c) SHA1(0b29cb2a3b7eaa3770b34f08c4fd42844f42700f))
	ROM_LOAD( "cos40b-m_f600-f9ff.bin", 0x1000, 0x400, BAD_DUMP CRC(e3397d9d) SHA1(490a0c834b0da392daf782edc7d51ca8f0668b1a))
	ROM_LOAD( "cos40b-m_1c00-1dff.bin", 0x1400, 0x200, BAD_DUMP CRC(0f759f44) SHA1(9689c1c1faa62c56def999cbedbbb0c8d928dcff))
	// chargen ROM is undumped, afaik
	ROM_REGION( 0x1680, "chargen", 0 )
	ROM_LOAD( "ch3.raw", 0x0000, 0x1680, BAD_DUMP CRC(c223622b) SHA1(185ef24896419d7ff46f71a760ac217de3811684))
ROM_END

// RM480Z is quite different, might be better off in its own driver
ROM_START( rm480z )
	ROM_REGION( 0x10000, RM380Z_MAINCPU_TAG, 0 )
	ROM_LOAD( "fv2.0_0_12099_19.2.86.ic83", 0x0000, 0x4000, CRC(a0f02d8a) SHA1(1c063b842699dc0ad85a5a5f337f2864497f9c0f) )
	ROM_LOAD( "fv2.0_1_12100_27.2.86.ic93", 0x4000, 0x4000, CRC(2a93ca6e) SHA1(7fdd772d4251dbf951a687d184ed787cfe21212b) )
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "cg06_12098_28.2.86.ic98", 0x0000, 0x2000, CRC(15d40f7e) SHA1(a7266357eb9be849f77a97ff3013b236c0af8289) )
ROM_END

ROM_START( rm480za )
	ROM_REGION( 0x10000, RM380Z_MAINCPU_TAG, 0 )
	ROM_LOAD( "ros_1.2b.ls",   0x0000, 0x2000, CRC(37e93287) SHA1(c96d4b7eedadb0fb8e3732b6ba3e898e123c393f) )
	ROM_LOAD( "bir_5.4b_0.lt", 0x2000, 0x4000, CRC(64759792) SHA1(ffce5e59788cd492937e85b368b71e8a5a2aaa9c) )
	ROM_LOAD( "bir_5.4b_1.lu", 0x6000, 0x4000, CRC(adce35df) SHA1(dc20df0fcfefc5e2682b47e7ce454043dc593cd8) )
	ROM_LOAD( "bir_5.4b_2.lv", 0xa000, 0x4000, CRC(5a455fc8) SHA1(04c5f17120ee085c013e600b77ca63cbb138aa4d) )
	ROM_REGION( 0x10000, "user1", 0 ) // bioses for fdc board versions
	ROM_LOAD( "idc3-1i.rom",   0x0000, 0x2000, CRC(39e2cdf0) SHA1(ba523af357b61bbe6192727139850f36597d79f1) )
	ROM_LOAD( "idc5-1j.rom",   0x2000, 0x2000, CRC(d2ac27e2) SHA1(12d3966e0096c9bfb98135e15c3ddb37920cce15) )
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "cg06.lq", 0x0000, 0x2000, BAD_DUMP CRC(15d40f7e) SHA1(a7266357eb9be849f77a97ff3013b236c0af8289) ) // chip is marked CG05, might not be the same, so marked as bad
ROM_END


/* Driver */
//   YEAR  NAME       PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT            COMPANY              FULLNAME                FLAGS
COMP(1978, rm380z,    0,      0,      rm380z,  rm380z, rm380z_state, init_rm380z,    "Research Machines", "RM-380Z, COS 4.0B",    MACHINE_NO_SOUND_HW)
COMP(1978, rm380z34d, rm380z, 0,      rm380z,  rm380z, rm380z_state, init_rm380z34d, "Research Machines", "RM-380Z, COS 3.4D",    MACHINE_NO_SOUND_HW)
COMP(1978, rm380z34e, rm380z, 0,      rm380z,  rm380z, rm380z_state, init_rm380z34e, "Research Machines", "RM-380Z, COS 3.4E",    MACHINE_NO_SOUND_HW)
COMP(1981, rm480z,    rm380z, 0,      rm480z,  rm380z, rm380z_state, init_rm380z34e, "Research Machines", "LINK RM-480Z (set 1)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP(1981, rm480za,   rm380z, 0,      rm480z,  rm380z, rm380z_state, init_rm380z34e, "Research Machines", "LINK RM-480Z (set 2)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

