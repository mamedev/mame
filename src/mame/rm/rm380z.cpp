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

Robin Sergeant, 2024 - h/w scrolling, HRG, sound, 8" disk support
and various bug fixes.

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

Monitor commands:
B - Boot CP/M  (COS /F, COS /M)
X - Boot CP/M from another drive (as above)
L - Load program from cassette (COS /C)
D - Dump memory to cassette (as above)
C - Continue program at restart address (as above)
J - Go to address
O - Select printer option (and cassette speed for COS /C)
M - Enable HRG board as memory (COS 3.4, COS 4.0)
W - Select 40 or 80 characters per line (All 80-column machines)
Ctrl+F - Enter Front Panel (=the debugger)
Ctrl+T - Enter Typewriter mode
Ctrl+S - Autopaging on
Ctrl+Q - Autopaging off
Ctrl+A - Toggle autopaging

Graphics characters: These are low-res (2x3 TRS80-style) from 80-BF, repeated at C0-FF.
80-BF will be low-intensity.
The characters 00-1F have one set for COS 3.4 and COS 4.0, and a different set for the others.
COS 4.0 and 4.2 allow one to redefine the 80-FF character range, and to have attributes.
ROS 2.2 allows an alternate character set.

Sound:
RM380Z has a connector for a speaker

===

Memory map from service manual:

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

Video resolution:
80x24 - 8 pixels wide, 10 pixels high = 640x240
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

- Make cassette interface work.


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
#include "rm380z.h"
#include "speaker.h"

#include "screen.h"


void rm380z_state::rm380z_mem(address_map &map)
{
	map(0xe000, 0xefff).rom().region(RM380Z_MAINCPU_TAG, 0);
	map(0xf000, 0xf5ff).rw(FUNC(rm380z_state::videoram_read), FUNC(rm380z_state::videoram_write));
	map(0xf600, 0xf9ff).rom().region(RM380Z_MAINCPU_TAG, 0x1000);     /* Extra ROM space for COS4.0 */
	map(0xfa00, 0xfaff).ram();
	map(0xfb00, 0xfbff).rw(FUNC(rm380z_state::port_read), FUNC(rm380z_state::port_write));
	map(0xfc00, 0xffff).ram().share("hiram");
}

void rm380z_state::rm380z_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xbf).rw(FUNC(rm380z_state::rm380z_portlow_r), FUNC(rm380z_state::rm380z_portlow_w));
	map(0xc0, 0xc3).mirror(0x20).rw(m_fdc, FUNC(fd1771_device::read), FUNC(fd1771_device::write));
	map(0xc4, 0xc7).mirror(0x20).w(FUNC(rm380z_state::disk_0_control));
	map(0xe8, 0xff).rw(FUNC(rm380z_state::rm380z_porthi_r), FUNC(rm380z_state::rm380z_porthi_w));
}

INPUT_PORTS_START( rm380z )

//  PORT_START("additional_chars")
//  PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Escape") PORT_CODE(KEYCODE_ESC) PORT_CODE(KEYCODE_ESC)

INPUT_PORTS_END

INPUT_PORTS_START( rm380zhrg )

	PORT_START("display_type")
	PORT_CONFNAME( 0x01, 0x00, "Monitor" ) PORT_CHANGED_MEMBER(DEVICE_SELF, rm380z_state_cos40_hrg, monitor_changed, 0)
	PORT_CONFSETTING( 0x00, "Colour Monitor" )
	PORT_CONFSETTING( 0x01, "Monochrome b/w Monitor" )

INPUT_PORTS_END

//
//
//

static void rm380z_floppies(device_slot_interface &device)
{
	device.option_add("mds", FLOPPY_525_SD);
	device.option_add("fds", FLOPPY_8_DSSD);
}

uint32_t rm380z_state::screen_update_rm380z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// blank screen
	bitmap.fill(0);

	update_screen(bitmap);

	return 0;
}

void rm380z_state::base_configure(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &rm380z_state::rm380z_mem);
	m_maincpu->set_addrmap(AS_IO, &rm380z_state::rm380z_io);

	/* video hardware */
	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(rm380z_state::screen_update_rm380z));
	m_screen->set_palette(m_palette);

	SPEAKER(config, "mono").front_center();

	/* RAM configurations */
	RAM(config, RAM_TAG).set_default_size("56K");

	/* floppy disk */
	FD1771(config, m_fdc, 16_MHz_XTAL / 16);
	FLOPPY_CONNECTOR(config, m_floppy0, rm380z_floppies, "mds", floppy_image_device::default_mfm_floppy_formats).set_fixed(true);
	FLOPPY_CONNECTOR(config, m_floppy1, rm380z_floppies, "mds", floppy_image_device::default_mfm_floppy_formats).set_fixed(true);

	/* keyboard */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(rm380z_state::keyboard_put));
}

void rm380z_state::fds_configure()
{
	// FDS drives require a 2 MHz square wave clock frequency
	m_fdc->set_unscaled_clock(16_MHz_XTAL / 8);
	// change media type for floppy connectors
	m_floppy0->set_default_option("fds");
	m_floppy1->set_default_option("fds");
}

void rm380z_state_cos34::rm380z34e(machine_config &config)
{
	base_configure(config);

	/* cassette */
	CASSETTE(config, m_cassette);
//  m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	m_screen->set_raw(8_MHz_XTAL, 512, 0, 320, 312, 0, 240);

	SN74S262(config, m_rocg, 0);
	m_rocg->set_palette(m_palette);
}

void rm380z_state_cos40::rm380z(machine_config &config)
{
	base_configure(config);

	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.80);

	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 240);
}

void rm380z_state_cos40_hrg::rm380zhrg(machine_config &config)
{
	rm380z(config);

	m_palette->set_init(FUNC(rm380z_state_cos40_hrg::palette_init)).set_entries(19);
}

/* ROM definitions */

ROM_START( rm380z34d ) // COS 3.4D/F
	ROM_REGION( 0x10000, RM380Z_MAINCPU_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "cos34d-f.bin", 0x0000, 0x1000, CRC(eb128b40) SHA1(c46f358fb76459987e41750d052995563f2f7d53))
ROM_END

ROM_START( rm380z34e ) // COS 3.4E/M
	ROM_REGION( 0x10000, RM380Z_MAINCPU_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "cos34e-m.bin", 0x0000, 0x1000, CRC(20e2ddf4) SHA1(3177b28793d5a348c94fd0ae6393d74e2e9a8662))
ROM_END

ROM_START( rm380z ) // COS 4.0B/M
	ROM_REGION( 0x10000, RM380Z_MAINCPU_TAG, 0 )
	ROM_LOAD( "cos40b-m.bin",           0x0000, 0x1000, CRC(1f0b3a5c) SHA1(0b29cb2a3b7eaa3770b34f08c4fd42844f42700f) )
	ROM_LOAD( "cos40b-m_f600-f9ff.bin", 0x1000, 0x0400, CRC(e3397d9d) SHA1(490a0c834b0da392daf782edc7d51ca8f0668b1a) )
	ROM_LOAD( "cos40b-m_1c00-1dff.bin", 0x1400, 0x0200, CRC(0f759f44) SHA1(9689c1c1faa62c56def999cbedbbb0c8d928dcff) )
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "c-gen-22.bin",           0x0000, 0x0800, CRC(1b67127f) SHA1(289a919871d30c5e832d22244bcac1dcfd544baa) )
ROM_END

ROM_START( rm380zhrg ) // COS 4.0B/M
	ROM_REGION( 0x10000, RM380Z_MAINCPU_TAG, 0 )
	ROM_LOAD( "cos40b-m.bin",           0x0000, 0x1000, CRC(1f0b3a5c) SHA1(0b29cb2a3b7eaa3770b34f08c4fd42844f42700f) )
	ROM_LOAD( "cos40b-m_f600-f9ff.bin", 0x1000, 0x0400, CRC(e3397d9d) SHA1(490a0c834b0da392daf782edc7d51ca8f0668b1a) )
	ROM_LOAD( "cos40b-m_1c00-1dff.bin", 0x1400, 0x0200, CRC(0f759f44) SHA1(9689c1c1faa62c56def999cbedbbb0c8d928dcff) )
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "c-gen-22.bin",           0x0000, 0x0800, CRC(1b67127f) SHA1(289a919871d30c5e832d22244bcac1dcfd544baa) )
ROM_END

ROM_START( rm380zf ) // COS 4.0B/F
	ROM_REGION( 0x10000, RM380Z_MAINCPU_TAG, 0 )
	ROM_LOAD( "cos40b-f.bin",           0x0000, 0x1000, CRC(c4110957) SHA1(08d924c7a152ca102585520a987051bcad3fca3f) )
	ROM_LOAD( "cos40b-f_f600-f9ff.bin", 0x1000, 0x0400, CRC(3b983326) SHA1(4a5273ca196cb98f9bb262f3e8f13bf22c9ca11c) )
	ROM_LOAD( "cos40b-f_1c00-1dff.bin", 0x1400, 0x0200, CRC(0f759f44) SHA1(9689c1c1faa62c56def999cbedbbb0c8d928dcff) )
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "c-gen-22.bin",           0x0000, 0x0800, CRC(1b67127f) SHA1(289a919871d30c5e832d22244bcac1dcfd544baa) )
ROM_END

ROM_START( rm380zfhrg ) // COS 4.0B/F
	ROM_REGION( 0x10000, RM380Z_MAINCPU_TAG, 0 )
	ROM_LOAD( "cos40b-f.bin",           0x0000, 0x1000, CRC(c4110957) SHA1(08d924c7a152ca102585520a987051bcad3fca3f) )
	ROM_LOAD( "cos40b-f_f600-f9ff.bin", 0x1000, 0x0400, CRC(3b983326) SHA1(4a5273ca196cb98f9bb262f3e8f13bf22c9ca11c) )
	ROM_LOAD( "cos40b-f_1c00-1dff.bin", 0x1400, 0x0200, CRC(0f759f44) SHA1(9689c1c1faa62c56def999cbedbbb0c8d928dcff) )
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "c-gen-22.bin",           0x0000, 0x0800, CRC(1b67127f) SHA1(289a919871d30c5e832d22244bcac1dcfd544baa) )
ROM_END


/* Driver */
//   YEAR  NAME        PARENT   COMPAT  MACHINE     INPUT      CLASS                   INIT                       COMPANY              FULLNAME                        FLAGS
COMP(1978, rm380z,     0,       0,      rm380z,     rm380z,    rm380z_state_cos40,     driver_device::empty_init, "Research Machines", "RM-380Z, COS 4.0B/M",          0)
COMP(1978, rm380zhrg,  rm380z,  0,      rm380zhrg,  rm380zhrg, rm380z_state_cos40_hrg, driver_device::empty_init, "Research Machines", "RM-380Z, COS 4.0B/M with HRG", 0)
COMP(1978, rm380zf,    0,       0,      rm380zf,    rm380z,    rm380z_state_cos40,     driver_device::empty_init, "Research Machines", "RM-380Z, COS 4.0B/F",          0)
COMP(1978, rm380zfhrg, rm380zf, 0,      rm380zfhrg, rm380zhrg, rm380z_state_cos40_hrg, driver_device::empty_init, "Research Machines", "RM-380Z, COS 4.0B/F with HRG", 0)
COMP(1978, rm380z34d,  rm380z,  0,      rm380z34d,  rm380z,    rm380z_state_cos34,     driver_device::empty_init, "Research Machines", "RM-380Z, COS 3.4D/F",          MACHINE_NO_SOUND_HW)
COMP(1978, rm380z34e,  rm380z,  0,      rm380z34e,  rm380z,    rm380z_state_cos34,     driver_device::empty_init, "Research Machines", "RM-380Z, COS 3.4E/M",          MACHINE_NO_SOUND_HW)
