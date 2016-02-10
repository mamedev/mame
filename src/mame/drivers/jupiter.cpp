// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Robbbert
/**************************************************************************

Wave Mate Jupiter



Jupiter 3
*********

Status: Preliminary
Hangs if your input line starts with 'k'.

ToDo:
- Connect all devices
- Everything!

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/keyboard.h"
#include "machine/terminal.h"
#include "machine/wd_fdc.h"
#include "includes/jupiter.h"

#define TERMINAL_TAG "terminal"
#define KEYBOARD_TAG "keyboard"


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( jupiter_m6800_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( jupiter_m6800_mem, AS_PROGRAM, 8, jupiter2_state )
	AM_RANGE(0x0000, 0x7fff) AM_RAM
//  AM_RANGE(0xc000, 0xcfff) Video RAM
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION(MCM6571AP_TAG, 0)
//  AM_RANGE(0xff58, 0xff5c) Cartridge Disk Controller PIA
//  AM_RANGE(0xff60, 0xff76) DMA Controller
//  AM_RANGE(0xff80, 0xff83) Floppy PIA
	AM_RANGE(0xff84, 0xff87) AM_DEVREADWRITE(INS1771N1_TAG, wd_fdc_t, read, write)
//  AM_RANGE(0xff90, 0xff93) Hytype Parallel Printer PIA
//  AM_RANGE(0xffa0, 0xffa7) Persci Floppy Disk Controller
//  AM_RANGE(0xffb0, 0xffb3) Video PIA
//  AM_RANGE(0xffc0, 0xffc1) Serial Port 0 ACIA
//  AM_RANGE(0xffc4, 0xffc5) Serial Port 1 ACIA
//  AM_RANGE(0xffc8, 0xffc9) Serial Port 2 ACIA
//  AM_RANGE(0xffcc, 0xffcd) Serial Port 3 ACIA
//  AM_RANGE(0xffd0, 0xffd1) Serial Port 4 ACIA / Cassette
//  AM_RANGE(0xffd4, 0xffd5) Serial Port 5 ACIA / EPROM Programmer (2704/2708)
//  AM_RANGE(0xffd8, 0xffd9) Serial Port 6 ACIA / Hardware Breakpoint Registers
//  AM_RANGE(0xffdc, 0xffdd) Serial Port 7 ACIA
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( jupiter_m6800_io )
//-------------------------------------------------

static ADDRESS_MAP_START( jupiter_m6800_io, AS_IO, 8, jupiter2_state )
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( jupiter3_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( jupiter3_mem, AS_PROGRAM, 8, jupiter3_state )
	AM_RANGE(0x0000, 0xbfff) AM_RAM AM_SHARE("p_ram")
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_SHARE("p_videoram")
	AM_RANGE(0xe000, 0xefff) AM_ROM AM_REGION(Z80_TAG, 0)
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( jupiter3_io )
//-------------------------------------------------

static ADDRESS_MAP_START( jupiter3_io, AS_IO, 8, jupiter3_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xa1, 0xa4) AM_READ(ff_r)
	AM_RANGE(0xb0, 0xb0) AM_READ(status_r)
	AM_RANGE(0xb2, 0xb2) AM_READ(key_r)
ADDRESS_MAP_END

READ8_MEMBER( jupiter3_state::ff_r )
{
	return 0xfd;
}

//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( jupiter )
//-------------------------------------------------

static INPUT_PORTS_START( jupiter )
INPUT_PORTS_END

READ8_MEMBER( jupiter3_state::key_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( jupiter3_state::status_r )
{
	return (m_term_data) ? 0x80 : 0x00;
}

WRITE8_MEMBER( jupiter3_state::kbd_put )
{
	if (data)
		m_term_data = data ^ 0x80;
}


//**************************************************************************
//  VIDEO
//**************************************************************************

void jupiter3_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

UINT32 jupiter3_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 32; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x++)
			{
				gfx = 0;
				if (ra < 9)
				{
					chr = m_p_videoram[x];
					gfx = m_p_chargen[(chr<<4) | ra ];
				}

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=64;
	}
	return 0;
}




//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

static SLOT_INTERFACE_START( jupiter_floppies )
	SLOT_INTERFACE( "525ssdd", FLOPPY_525_SSDD )
SLOT_INTERFACE_END


//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( jupiter )
//-------------------------------------------------

void jupiter2_state::machine_start()
{
}


//-------------------------------------------------
//  MACHINE_START( jupiter3 )
//-------------------------------------------------

void jupiter3_state::machine_reset()
{
	UINT8* ROM = memregion(Z80_TAG)->base();
	memcpy(m_p_ram, ROM, 0x1000);
	m_maincpu->set_pc(0xe000);
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( jupiter )
//-------------------------------------------------

static MACHINE_CONFIG_START( jupiter, jupiter2_state )
	// basic machine hardware
	MCFG_CPU_ADD(MCM6571AP_TAG, M6800, 2000000)
	MCFG_CPU_PROGRAM_MAP(jupiter_m6800_mem)
	MCFG_CPU_IO_MAP(jupiter_m6800_io)

	// devices
	MCFG_DEVICE_ADD(INS1771N1_TAG, FD1771, 1000000)
	MCFG_FLOPPY_DRIVE_ADD(INS1771N1_TAG":0", jupiter_floppies, "525ssdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(INS1771N1_TAG":1", jupiter_floppies, nullptr, floppy_image_device::default_floppy_formats)

	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( jupiter3 )
//-------------------------------------------------

static MACHINE_CONFIG_START( jupiter3, jupiter3_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(jupiter3_mem)
	MCFG_CPU_IO_MAP(jupiter3_io)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(jupiter3_state, screen_update)
	MCFG_SCREEN_SIZE(512, 320)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 320-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	// devices
	MCFG_DEVICE_ADD(INS1771N1_TAG, FD1771, 1000000)
	MCFG_FLOPPY_DRIVE_ADD(INS1771N1_TAG":0", jupiter_floppies, "525ssdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(INS1771N1_TAG":1", jupiter_floppies, nullptr, floppy_image_device::default_floppy_formats)

	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(jupiter3_state, kbd_put))

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( jupiter2 )
//-------------------------------------------------

ROM_START( jupiter2 )
	ROM_REGION( 0x1000, MCM6571AP_TAG, ROMREGION_INVERT ) // address and data lines are inverted
	ROM_LOAD( "idb v1.1 for 60k jii f000.1c", 0x0000, 0x0400, CRC(50893aae) SHA1(da0222c4cb6188f6cfc657fc33558d0a6a41cd1a) )
	ROM_LOAD( "idb v1.1 for 60k jii f400.6c", 0x0400, 0x0400, CRC(a435344a) SHA1(bc4f4143301b10ec762ecc0cb69e5a9d4c4bef7b) )
	ROM_LOAD( "idb v1.1 for 60k jii f800.1d", 0x0800, 0x0400, CRC(ab82df45) SHA1(be7ea5347ff0582401e26c2fa10e13463cbe57c6) )
	ROM_LOAD( "boot_v2.6_sn5d00000000000003_fc00.6d", 0x0c00, 0x0400, CRC(8f33e4ed) SHA1(fb206e5019c166583ff516de3608ae86d2636d2a) )
	ROM_LOAD( "jupiter ii boot rom v2.6 12_18_82 s_n 5d000...0015.6d", 0x0c00, 0x0400, CRC(f87cefdf) SHA1(229ea961e6036ec39e0ae33abc7f554bf9d8361b) )
ROM_END


//-------------------------------------------------
//  ROM( jupiter3 )
//-------------------------------------------------

ROM_START( jupiter3 )
	ROM_REGION( 0x1000, Z80_TAG, ROMREGION_INVERT ) // address and data lines are inverted
	ROM_LOAD( "jove 2.0 78_034 4v2d000 1.1c", 0x0000, 0x0400, CRC(be92a76c) SHA1(9c7d9b37c2bbf0c2e9465421e3e1bcf3dd9e66a6) )
	ROM_LOAD( "jove 2.0 78_034 4v2d000 2.6c", 0x0400, 0x0400, CRC(ee98dd32) SHA1(0513261c7c0d911225ea957ee67394871a36ada4) )
	ROM_LOAD( "jove 2.0 78_034 4v2d000 3.1d", 0x0800, 0x0400, CRC(51476b1d) SHA1(ab6f4eb244bcf9718aafdae67da086ec81f33fa6) )
	ROM_LOAD( "jove 2.0 78_034 4v2d000 4.6d", 0x0c00, 0x0400, CRC(16a9595d) SHA1(06150278650590497732e1f3f42356de56737921) )

	// character generator is missing, using one from c10 for now
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END



//**************************************************************************
//  DRIVER INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  DRIVER_INIT( jupiter )
//-------------------------------------------------

DRIVER_INIT_MEMBER(jupiter2_state,jupiter)
{
	UINT8 *rom = memregion(MCM6571AP_TAG)->base();
	UINT8 inverted[0x1000];

	memcpy(inverted, rom, 0x1000);

	for (offs_t addr = 0; addr < 0x400; addr++)
	{
		// invert address lines
		rom[0x3ff - addr] = inverted[addr];
		rom[0x7ff - addr] = inverted[addr + 0x400];
		rom[0xbff - addr] = inverted[addr + 0x800];
		rom[0xfff - addr] = inverted[addr + 0xc00];
	}
}


//-------------------------------------------------
//  DRIVER_INIT( jupiter3 )
//-------------------------------------------------

DRIVER_INIT_MEMBER(jupiter3_state,jupiter3)
{
	UINT8 *rom = memregion(Z80_TAG)->base();
	UINT8 inverted[0x1000];

	memcpy(inverted, rom, 0x1000);

	for (offs_t addr = 0; addr < 0x400; addr++)
	{
		// invert address lines
		rom[0x3ff - addr] = inverted[addr];
		rom[0x7ff - addr] = inverted[addr + 0x400];
		rom[0xbff - addr] = inverted[addr + 0x800];
		rom[0xfff - addr] = inverted[addr + 0xc00];
	}
}

//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    INIT      COMPANY          FULLNAME       FLAGS
COMP( 1976, jupiter2, 0,      0,       jupiter,   jupiter, jupiter2_state, jupiter, "Wave Mate",   "Jupiter II",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1976, jupiter3, 0,      0,       jupiter3,  jupiter, jupiter3_state, jupiter3,"Wave Mate",   "Jupiter III", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
