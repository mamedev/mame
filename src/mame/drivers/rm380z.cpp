// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Gabriele D'Antona
/*

Research Machines 380Z (aka "RML 380Z" or "RM 380Z")
Microcomputer produced by Research Machines Limited, Oxford, UK
1978-1985
MESS driver by Wilbert Pol and friol (dantonag (at) gmail.com)
Driver started on 22/12/2011

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


#include "includes/rm380z.h"

#define KEYBOARD_TAG "keyboard"

static ADDRESS_MAP_START(rm380z_mem, AS_PROGRAM, 8, rm380z_state)
	AM_RANGE( 0xe000, 0xefff ) AM_ROM AM_REGION(RM380Z_MAINCPU_TAG, 0)
	AM_RANGE( 0xf000, 0xf5ff ) AM_READWRITE(videoram_read,videoram_write)
	AM_RANGE( 0xf600, 0xf9ff ) AM_ROM AM_REGION(RM380Z_MAINCPU_TAG, 0x1000)     /* Extra ROM space for COS4.0 */
	AM_RANGE( 0xfa00, 0xfaff ) AM_RAM
	AM_RANGE( 0xfb00, 0xfbff ) AM_READWRITE( port_read, port_write )
	AM_RANGE( 0xfc00, 0xffff ) AM_READWRITE(hiram_read,hiram_write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rm380z_io , AS_IO, 8, rm380z_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xbf) AM_READWRITE(rm380z_portlow_r, rm380z_portlow_w)
	AM_RANGE(0xc0, 0xc3) AM_DEVREADWRITE("wd1771", fd1771_t, read, write)
	AM_RANGE(0xc4, 0xc4) AM_WRITE(disk_0_control)
	AM_RANGE(0xc5, 0xff) AM_READWRITE(rm380z_porthi_r, rm380z_porthi_w)
ADDRESS_MAP_END

INPUT_PORTS_START( rm380z )
//  PORT_START("additional_chars")
//  PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Escape") PORT_CODE(KEYCODE_ESC) PORT_CODE(KEYCODE_ESC)
INPUT_PORTS_END

//
//
//

static SLOT_INTERFACE_START( rm380z_floppies )
	SLOT_INTERFACE("sssd", FLOPPY_525_SSSD)
SLOT_INTERFACE_END

UINT32 rm380z_state::screen_update_rm380z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	update_screen(bitmap);
	return 0;
}

static MACHINE_CONFIG_START( rm380z, rm380z_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(RM380Z_MAINCPU_TAG, Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(rm380z_mem)
	MCFG_CPU_IO_MAP(rm380z_io)

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

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* RAM configurations */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("56K")

	/* floppy disk */
	MCFG_FD1771_ADD("wd1771", XTAL_1MHz)

	MCFG_FLOPPY_DRIVE_ADD("wd1771:0", rm380z_floppies, "sssd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd1771:1", rm380z_floppies, "sssd", floppy_image_device::default_floppy_formats)

	/* keyboard */
	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(rm380z_state, keyboard_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( rm380z )
	ROM_REGION( 0x10000, RM380Z_MAINCPU_TAG, 0 )
//  ROM_LOAD( "cos34e-m.bin", 0x0000, 0x1000, CRC(20e2ddf4) SHA1(3177b28793d5a348c94fd0ae6393d74e2e9a8662))
	// I'm not sure of how those roms have been dumped. I don't know if those are good dumps or not.
	ROM_LOAD( "cos40b-m.bin", 0x0000, 0x1000, BAD_DUMP CRC(1f0b3a5c) SHA1(0b29cb2a3b7eaa3770b34f08c4fd42844f42700f))
	ROM_LOAD( "cos40b-m_f600-f9ff.bin", 0x1000, 0x400, BAD_DUMP CRC(e3397d9d) SHA1(490a0c834b0da392daf782edc7d51ca8f0668b1a))
	ROM_LOAD( "cos40b-m_1c00-1dff.bin", 0x1400, 0x200, BAD_DUMP CRC(0f759f44) SHA1(9689c1c1faa62c56def999cbedbbb0c8d928dcff))
	// chargen ROM is undumped, afaik
	ROM_REGION( 0x1680, "chargen", 0 )
	ROM_LOAD( "ch3.raw", 0x0000, 0x1680, BAD_DUMP CRC(c223622b) SHA1(185ef24896419d7ff46f71a760ac217de3811684))
ROM_END

/* Driver */

COMP(1978, rm380z, 0,      0,       rm380z,    rm380z, driver_device,  0,    "Research Machines", "RM-380Z", MACHINE_NO_SOUND_HW)
