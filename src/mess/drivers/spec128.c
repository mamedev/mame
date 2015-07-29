// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/***************************************************************************

    NOTE: ****** Specbusy: press N, R, or E to boot *************


        Spectrum/Inves/TK90X etc. memory map:

    CPU:
        0000-3fff ROM
        4000-ffff RAM

        Spectrum 128/+2/+2a/+3 memory map:

        CPU:
                0000-3fff Banked ROM/RAM (banked rom only on 128/+2)
                4000-7fff Banked RAM
                8000-bfff Banked RAM
                c000-ffff Banked RAM

        TS2068 memory map: (Can't have both EXROM and DOCK active)
        The 8K EXROM can be loaded into multiple pages.

    CPU:
                0000-1fff     ROM / EXROM / DOCK (Cartridge)
                2000-3fff     ROM / EXROM / DOCK
                4000-5fff \
                6000-7fff  \
                8000-9fff  |- RAM / EXROM / DOCK
                a000-bfff  |
                c000-dfff  /
                e000-ffff /


Interrupts:

Changes:

29/1/2000   KT -    Implemented initial +3 emulation.
30/1/2000   KT -    Improved input port decoding for reading and therefore
            correct keyboard handling for Spectrum and +3.
31/1/2000   KT -    Implemented buzzer sound for Spectrum and +3.
            Implementation copied from Paul Daniel's Jupiter driver.
            Fixed screen display problems with dirty chars.
            Added support to load .Z80 snapshots. 48k support so far.
13/2/2000   KT -    Added Interface II, Kempston, Fuller and Mikrogen
            joystick support.
17/2/2000   DJR -   Added full key descriptions and Spectrum+ keys.
            Fixed Spectrum +3 keyboard problems.
17/2/2000   KT -    Added tape loading from WAV/Changed from DAC to generic
            speaker code.
18/2/2000   KT -    Added tape saving to WAV.
27/2/2000   KT -    Took DJR's changes and added my changes.
27/2/2000   KT -    Added disk image support to Spectrum +3 driver.
27/2/2000   KT -    Added joystick I/O code to the Spectrum +3 I/O handler.
14/3/2000   DJR -   Tape handling dipswitch.
26/3/2000   DJR -   Snapshot files are now classifed as snapshots not
            cartridges.
04/4/2000   DJR -   Spectrum 128 / +2 Support.
13/4/2000   DJR -   +4 Support (unofficial 48K hack).
13/4/2000   DJR -   +2a Support (rom also used in +3 models).
13/4/2000   DJR -   TK90X, TK95 and Inves support (48K clones).
21/4/2000   DJR -   TS2068 and TC2048 support (TC2048 Supports extra video
            modes but doesn't have bank switching or sound chip).
09/5/2000   DJR -   Spectrum +2 (France, Spain), +3 (Spain).
17/5/2000   DJR -   Dipswitch to enable/disable disk drives on +3 and clones.
27/6/2000   DJR -   Changed 128K/+3 port decoding (sound now works in Zub 128K).
06/8/2000   DJR -   Fixed +3 Floppy support
10/2/2001   KT  -   Re-arranged code and split into each model emulated.
            Code is split into 48k, 128k, +3, tc2048 and ts2048
            segments. 128k uses some of the functions in 48k, +3
            uses some functions in 128, and tc2048/ts2048 use some
            of the functions in 48k. The code has been arranged so
            these functions come in some kind of "override" order,
            read functions changed to use  READ8_HANDLER and write
            functions changed to use WRITE8_HANDLER.
            Added Scorpion256 preliminary.
18/6/2001   DJR -   Added support for Interface 2 cartridges.
xx/xx/2001  KS -    TS-2068 sound fixed.
            Added support for DOCK cartridges for TS-2068.
            Added Spectrum 48k Psycho modified rom driver.
            Added UK-2086 driver.
23/12/2001  KS -    48k machines are now able to run code in screen memory.
                Programs which keep their code in screen memory
                like monitors, tape copiers, decrunchers, etc.
                works now.
                Fixed problem with interrupt vector set to 0xffff (much
        more 128k games works now).
                A useful used trick on the Spectrum is to set
                interrupt vector to 0xffff (using the table
                which contain 0xff's) and put a byte 0x18 hex,
                the opcode for JR, at this address. The first
                byte of the ROM is a 0xf3 (DI), so the JR will
                jump to 0xfff4, where a long JP to the actual
                interrupt routine is put. Due to unideal
                bankswitching in MAME this JP were to 0001 what
                causes Spectrum to reset. Fixing this problem
                made much more software runing (i.e. Paperboy).
            Corrected frames per second value for 48k and 128k
            Sinclair machines.
                There are 50.08 frames per second for Spectrum
                48k what gives 69888 cycles for each frame and
                50.021 for Spectrum 128/+2/+2A/+3 what gives
                70908 cycles for each frame.
            Remapped some Spectrum+ keys.
                Pressing F3 to reset was seting 0xf7 on keyboard
                input port. Problem occurred for snapshots of
                some programms where it was readed as pressing
                key 4 (which is exit in Tapecopy by R. Dannhoefer
                for example).
            Added support to load .SP snapshots.
            Added .BLK tape images support.
                .BLK files are identical to .TAP ones, extension
                is an only difference.
08/03/2002  KS -    #FF port emulation added.
                Arkanoid works now, but is not playable due to
                completly messed timings.

Initialisation values used when determining which model is being emulated:
 48K        Spectrum doesn't use either port.
 128K/+2    Bank switches with port 7ffd only.
 +3/+2a     Bank switches with both ports.

Notes:
 1. No contented memory.
 2. No hi-res colour effects (need contended memory first for accurate timing).
 3. Multiface 1 and Interface 1 not supported.
 4. Horace and the Spiders cartridge doesn't run properly.
 5. Tape images not supported:
    .TZX, .SPC, .ITM, .PAN, .TAP(Warajevo), .VOC, .ZXS.
 6. Snapshot images not supported:
    .ACH, .PRG, .RAW, .SEM, .SIT, .SNX, .ZX, .ZXS, .ZX82.
 7. 128K emulation is not perfect - the 128K machines crash and hang while
    running quite a lot of games.
 8. Disk errors occur on some +3 games.
 9. Video hardware of all machines is timed incorrectly.
10. EXROM and HOME cartridges are not emulated.
11. The TK90X and TK95 roms output 0 to port #df on start up.
12. The purpose of this port is unknown (probably display mode as TS2068) and
    thus is not emulated.

Very detailed infos about the ZX Spectrum +3e can be found at

http://www.z88forever.org.uk/zxplus3e/


The hc2000 corrupts its memory, especially if you type something, and the
resulting mess can be seen in the F4 viewer display.

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/spectrum.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "sound/ay8910.h"
#include "sound/speaker.h"
#include "formats/tzx_cas.h"
#include "machine/ram.h"

/****************************************************************************************************/
/* Spectrum 128 specific functions */

WRITE8_MEMBER(spectrum_state::spectrum_128_port_7ffd_w)
{
	/* D0-D2: RAM page located at 0x0c000-0x0ffff */
	/* D3 - Screen select (screen 0 in ram page 5, screen 1 in ram page 7 */
	/* D4 - ROM select - which rom paged into 0x0000-0x03fff */
	/* D5 - Disable paging */

	/* disable paging? */
	if (m_port_7ffd_data & 0x20)
			return;

	if ((m_port_7ffd_data ^ data) & 0x08)
		spectrum_UpdateScreenBitmap();

	/* store new state */
	m_port_7ffd_data = data;

	/* update memory */
	spectrum_128_update_memory();
}

void spectrum_state::spectrum_128_update_memory()
{
	UINT8 *messram = m_ram->pointer();

	/* select ram at 0x0c000-0x0ffff */
	int ram_page = m_port_7ffd_data & 0x07;
	unsigned char *ram_data = messram + (ram_page<<14);
	membank("bank4")->set_base(ram_data);

	if (BIT(m_port_7ffd_data, 3))
		m_screen_location = messram + (7<<14);
	else
		m_screen_location = messram + (5<<14);

	if (!m_cart->exists())
	{
		/* ROM switching */
		int ROMSelection = BIT(m_port_7ffd_data, 4);

		/* rom 0 is 128K rom, rom 1 is 48 BASIC */
		unsigned char *ChosenROM = memregion("maincpu")->base() + 0x010000 + (ROMSelection << 14);

		membank("bank1")->set_base(ChosenROM);
	}
}

READ8_MEMBER( spectrum_state::spectrum_128_ula_r )
{
	int vpos = machine().first_screen()->vpos();

	return vpos<193 ? m_screen_location[0x1800|(vpos&0xf8)<<2]:0xff;
}

static ADDRESS_MAP_START (spectrum_128_io, AS_IO, 8, spectrum_state )
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(spectrum_port_fe_r,spectrum_port_fe_w) AM_MIRROR(0xfffe) AM_MASK(0xffff)
	AM_RANGE(0x001f, 0x001f) AM_READ(spectrum_port_1f_r) AM_MIRROR(0xff00)
	AM_RANGE(0x007f, 0x007f) AM_READ(spectrum_port_7f_r) AM_MIRROR(0xff00)
	AM_RANGE(0x00df, 0x00df) AM_READ(spectrum_port_df_r) AM_MIRROR(0xff00)
	AM_RANGE(0x0000, 0x0000) AM_WRITE(spectrum_128_port_7ffd_w) AM_MIRROR(0x7ffd)   // (A15 | A1) == 0, note: reading from this port does write to it by value from data bus
	AM_RANGE(0x8000, 0x8000) AM_DEVWRITE("ay8912", ay8910_device, data_w) AM_MIRROR(0x3ffd)
	AM_RANGE(0xc000, 0xc000) AM_DEVREADWRITE("ay8912", ay8910_device, data_r, address_w) AM_MIRROR(0x3ffd)
	AM_RANGE(0x0001, 0x0001) AM_READ(spectrum_128_ula_r) AM_MIRROR(0xfffe)
ADDRESS_MAP_END

static ADDRESS_MAP_START (spectrum_128_mem, AS_PROGRAM, 8, spectrum_state )
	AM_RANGE( 0x0000, 0x3fff) AM_ROMBANK("bank1") // don't use RAMBANK here otherwise programs can erase the ROM(!)
	AM_RANGE( 0x4000, 0x7fff) AM_RAMBANK("bank2")
	AM_RANGE( 0x8000, 0xbfff) AM_RAMBANK("bank3")
	AM_RANGE( 0xc000, 0xffff) AM_RAMBANK("bank4")
ADDRESS_MAP_END

MACHINE_RESET_MEMBER(spectrum_state,spectrum_128)
{
	UINT8 *messram = m_ram->pointer();

	memset(messram,0,128*1024);
	/* 0x0000-0x3fff always holds ROM */

	/* Bank 5 is always in 0x4000 - 0x7fff */
	membank("bank2")->set_base(messram + (5<<14));

	/* Bank 2 is always in 0x8000 - 0xbfff */
	membank("bank3")->set_base(messram + (2<<14));

	MACHINE_RESET_CALL_MEMBER(spectrum);

	/* set initial ram config */
	m_port_7ffd_data = 0;
	m_port_1ffd_data = -1;
	spectrum_128_update_memory();
}

/* F4 Character Displayer */
static const gfx_layout spectrum_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	96,                 /* 96 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( spec128 )
	GFXDECODE_ENTRY( "maincpu", 0x17d00, spectrum_charlayout, 0, 8 )
GFXDECODE_END


MACHINE_CONFIG_DERIVED( spectrum_128, spectrum )

	MCFG_DEVICE_REMOVE("maincpu")

	MCFG_CPU_ADD("maincpu", Z80, X1_128_SINCLAIR / 5)
	MCFG_CPU_PROGRAM_MAP(spectrum_128_mem)
	MCFG_CPU_IO_MAP(spectrum_128_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", spectrum_state,  spec_interrupt)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_RESET_OVERRIDE(spectrum_state, spectrum_128 )

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_RAW_PARAMS(X1_128_SINCLAIR / 2.5f, 456, 0, 352,  311, 0, 296)

	MCFG_VIDEO_START_OVERRIDE(spectrum_state, spectrum_128 )
	MCFG_GFXDECODE_MODIFY("gfxdecode", spec128)

	/* sound hardware */
	MCFG_SOUND_ADD("ay8912", AY8912, 1773400)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END




/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START(spec128)
	ROM_REGION(0x18000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English" )
	ROMX_LOAD("zx128_0.rom",0x10000,0x4000, CRC(e76799d2) SHA1(4f4b11ec22326280bdb96e3baf9db4b4cb1d02c5), ROM_BIOS(1))
	ROMX_LOAD("zx128_1.rom",0x14000,0x4000, CRC(b96a36be) SHA1(80080644289ed93d71a1103992a154cc9802b2fa), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "sp", "Spanish" )
	ROMX_LOAD("zx128s0.rom",0x10000,0x4000, CRC(453d86b2) SHA1(968937b1c750f0ef6205f01c6db4148da4cca4e3), ROM_BIOS(2))
	ROMX_LOAD("zx128s1.rom",0x14000,0x4000, CRC(6010e796) SHA1(bea3f397cc705eafee995ea629f4a82550562f90), ROM_BIOS(2))
ROM_END

ROM_START(specpls2)
	ROM_REGION(0x18000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English" )
	ROMX_LOAD("zxp2_0.rom",0x10000,0x4000, CRC(5d2e8c66) SHA1(72703f9a3e734f3c23ec34c0727aae4ccbef9a91), ROM_BIOS(1))
	ROMX_LOAD("zxp2_1.rom",0x14000,0x4000, CRC(98b1320b) SHA1(de8b0d2d0379cfe7c39322a086ca6da68c7f23cb), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "fr", "French" )
	ROMX_LOAD("plus2fr0.rom",0x10000,0x4000, CRC(c684c535) SHA1(56684c4c85a616e726a50707483b9a42d8e724ed), ROM_BIOS(2))
	ROMX_LOAD("plus2fr1.rom",0x14000,0x4000, CRC(f5e509c5) SHA1(7e398f62689c9d90a36d3a101351ec9987207308), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "sp", "Spanish" )
	ROMX_LOAD("plus2sp0.rom",0x10000,0x4000, CRC(e807d06e) SHA1(8259241b28ff85441f1bedc2bee53445767c51c5), ROM_BIOS(3))
	ROMX_LOAD("plus2sp1.rom",0x14000,0x4000, CRC(41981d4b) SHA1(ec0d5a158842d20601b4fbeaefc6668db979d0e1), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "ao", "ZX Spectrum +2c (Andrew Owen)" )
	ROMX_LOAD("plus2c-0.rom",0x10000,0x4000, CRC(bfddf748) SHA1(3eba870bcb2c5efa906f2ca3febe960fc35d66bb), ROM_BIOS(4))
	ROMX_LOAD("plus2c-1.rom",0x14000,0x4000, CRC(fd8552b6) SHA1(5ffcf79f2154ba2cf42cc1d9cb4be93cb5043e73), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 4, "namco", "ZX Spectrum +2c (Namco)" )
	ROMX_LOAD("pl2namco.rom",0x10000,0x8000, CRC(72a54e75) SHA1(311400157df689450dadc3620f4c4afa960b05ad), ROM_BIOS(5))
ROM_END

ROM_START(hc128)
	ROM_REGION(0x18000,"maincpu",0)
	ROM_LOAD("zx128_0.rom",0x10000,0x4000, CRC(e76799d2) SHA1(4f4b11ec22326280bdb96e3baf9db4b4cb1d02c5))
	ROM_LOAD("hc128.rom",  0x14000,0x4000, CRC(0241e960) SHA1(cea0d14391b9e571460a816088a1c00ecb24afa3))
ROM_END

ROM_START(hc2000)
	ROM_REGION(0x18000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "v1", "Version 1" )
	ROMX_LOAD("zx128_0.rom",0x10000,0x4000, CRC(e76799d2) SHA1(4f4b11ec22326280bdb96e3baf9db4b4cb1d02c5), ROM_BIOS(1))
	ROMX_LOAD("hc2000.v1",  0x14000,0x4000, CRC(453c1a5a) SHA1(f8139fc38478691cf44944dc83fd6e70b0f002fb), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v2", "Version 2" )
	ROMX_LOAD("zx128_0.rom",0x10000,0x4000, CRC(e76799d2) SHA1(4f4b11ec22326280bdb96e3baf9db4b4cb1d02c5), ROM_BIOS(2))
	ROMX_LOAD("hc2000.v2",  0x14000,0x4000, CRC(65d90464) SHA1(5e2096e6460ff2120c8ada97579fdf82c1199c09), ROM_BIOS(2))
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE     INPUT       INIT    COMPANY     FULLNAME */
COMP( 1986, spec128,  0,       0,       spectrum_128,   spec_plus, driver_device,   0,  "Sinclair Research Ltd", "ZX Spectrum 128" , 0 )
COMP( 1986, specpls2, spec128, 0,       spectrum_128,   spec_plus, driver_device,   0,  "Amstrad plc",           "ZX Spectrum +2" , 0 )
COMP( 1991, hc128,    spec128, 0,       spectrum_128,   spec_plus, driver_device,   0,  "ICE-Felix",             "HC-128" , 0 )
COMP( 1992, hc2000,   spec128, 0,       spectrum_128,   spec_plus, driver_device,   0,  "ICE-Felix",             "HC-2000" , MACHINE_NOT_WORKING )
