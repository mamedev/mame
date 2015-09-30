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
            Remaped some Spectrum+ keys.
                Presing F3 to reset was seting 0xf7 on keyboard
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

*******************************************************************************/

#include "emu.h"
#include "includes/spectrum.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "sound/ay8910.h"
#include "sound/speaker.h"
#include "formats/tzx_cas.h"

/* +3 hardware */
#include "machine/ram.h"


/****************************************************************************************************/
/* Spectrum + 3 specific functions */
/* This driver uses some of the spectrum_128 functions. The +3 is similar to a spectrum 128
but with a disc drive */


static const int spectrum_plus3_memory_selections[]=
{
		0,1,2,3,
		4,5,6,7,
		4,5,6,3,
		4,7,6,3
};

WRITE8_MEMBER( spectrum_state::spectrum_plus3_port_3ffd_w )
{
	if (m_floppy==1)
		m_upd765->fifo_w(space, 0, data, 0xff);
}

READ8_MEMBER( spectrum_state::spectrum_plus3_port_3ffd_r )
{
	if (m_floppy==0)
		return 0xff;
	else
		return m_upd765->fifo_r(space, 0, 0xff);
}


READ8_MEMBER( spectrum_state::spectrum_plus3_port_2ffd_r )
{
	if (m_floppy==0)
		return 0xff;
	else
		return m_upd765->msr_r(space, 0, 0xff);
}


void spectrum_state::spectrum_plus3_update_memory()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 *messram = m_ram->pointer();

	if (m_port_7ffd_data & 8)
	{
		logerror("+3 SCREEN 1: BLOCK 7\n");
		m_screen_location = messram + (7 << 14);
	}
	else
	{
		logerror("+3 SCREEN 0: BLOCK 5\n");
		m_screen_location = messram + (5 << 14);
	}

	if ((m_port_1ffd_data & 0x01) == 0)
	{
		/* select ram at 0x0c000-0x0ffff */
		int ram_page = m_port_7ffd_data & 0x07;
		unsigned char *ram_data = messram + (ram_page<<14);
		membank("bank4")->set_base(ram_data);

		logerror("RAM at 0xc000: %02x\n", ram_page);

		/* Reset memory between 0x4000 - 0xbfff in case extended paging was being used */
		/* Bank 5 in 0x4000 - 0x7fff */
		membank("bank2")->set_base(messram + (5 << 14));

		/* Bank 2 in 0x8000 - 0xbfff */
		membank("bank3")->set_base(messram + (2 << 14));

		if (!m_cart->exists())
		{
			/* ROM switching */
			int ROMSelection = BIT(m_port_7ffd_data, 4) | ((m_port_1ffd_data >> 1) & 0x02);

			/* rom 0 is editor, rom 1 is syntax, rom 2 is DOS, rom 3 is 48 BASIC */
			unsigned char *ChosenROM = memregion("maincpu")->base() + 0x010000 + (ROMSelection << 14);

			membank("bank1")->set_base(ChosenROM);
			space.unmap_write(0x0000, 0x3fff);

			logerror("rom switch: %02x\n", ROMSelection);
		}
	}
	else
	{
		/* Extended memory paging */
		int MemorySelection = (m_port_1ffd_data >> 1) & 0x03;
		const int *memory_selection = &spectrum_plus3_memory_selections[(MemorySelection << 2)];
		unsigned char *ram_data = messram + (memory_selection[0] << 14);

		membank("bank1")->set_base(ram_data);
		/* allow writes to 0x0000-0x03fff */
		space.install_write_bank(0x0000, 0x3fff, "bank1");

		ram_data = messram + (memory_selection[1] << 14);
		membank("bank2")->set_base(ram_data);

		ram_data = messram + (memory_selection[2] << 14);
		membank("bank3")->set_base(ram_data);

		ram_data = messram + (memory_selection[3] << 14);
		membank("bank4")->set_base(ram_data);

		logerror("extended memory paging: %02x\n", MemorySelection);
	}
}



WRITE8_MEMBER( spectrum_state::spectrum_plus3_port_7ffd_w )
{
		/* D0-D2: RAM page located at 0x0c000-0x0ffff */
		/* D3 - Screen select (screen 0 in ram page 5, screen 1 in ram page 7 */
		/* D4 - ROM select - which rom paged into 0x0000-0x03fff */
		/* D5 - Disable paging */

	/* disable paging? */
	if (m_port_7ffd_data & 0x20)
		return;

	/* store new state */
	m_port_7ffd_data = data;

	/* update memory */
	spectrum_plus3_update_memory();
}

WRITE8_MEMBER( spectrum_state::spectrum_plus3_port_1ffd_w )
{
	/* D0-D1: ROM/RAM paging */
	/* D2: Affects if d0-d1 work on ram/rom */
	/* D3 - Disk motor on/off */
	/* D4 - parallel port strobe */

	m_upd765_0->get_device()->mon_w(!BIT(data, 3));
	m_upd765_1->get_device()->mon_w(!BIT(data, 3));

	m_port_1ffd_data = data;

	/* disable paging? */
	if ((m_port_7ffd_data & 0x20)==0)
	{
		/* no */
		spectrum_plus3_update_memory();
	}
}

/* ports are not decoded full.
The function decodes the ports appropriately */
static ADDRESS_MAP_START (spectrum_plus3_io, AS_IO, 8, spectrum_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(spectrum_port_fe_r,spectrum_port_fe_w) AM_MIRROR(0xfffe) AM_MASK(0xffff)
	AM_RANGE(0x001f, 0x001f) AM_READ(spectrum_port_1f_r) AM_MIRROR(0xff00)
	AM_RANGE(0x4000, 0x4000) AM_WRITE(spectrum_plus3_port_7ffd_w) AM_MIRROR(0x3ffd)
	AM_RANGE(0x8000, 0x8000) AM_DEVWRITE("ay8912", ay8910_device, data_w) AM_MIRROR(0x3ffd)
	AM_RANGE(0xc000, 0xc000) AM_DEVREADWRITE("ay8912", ay8910_device, data_r, address_w) AM_MIRROR(0x3ffd)
	AM_RANGE(0x1000, 0x1000) AM_WRITE(spectrum_plus3_port_1ffd_w) AM_MIRROR(0x0ffd)
	AM_RANGE(0x2000, 0x2000) AM_READ(spectrum_plus3_port_2ffd_r) AM_MIRROR(0x0ffd)
	AM_RANGE(0x3000, 0x3000) AM_READWRITE(spectrum_plus3_port_3ffd_r,spectrum_plus3_port_3ffd_w) AM_MIRROR(0x0ffd)
ADDRESS_MAP_END

MACHINE_RESET_MEMBER(spectrum_state,spectrum_plus3)
{
	UINT8 *messram = m_ram->pointer();
	memset(messram,0,128*1024);

	MACHINE_RESET_CALL_MEMBER(spectrum);

	/* Initial configuration */
	m_port_7ffd_data = 0;
	m_port_1ffd_data = 0;
	spectrum_plus3_update_memory();
}

DRIVER_INIT_MEMBER(spectrum_state,plus3)
{
	m_floppy = 1;
}

DRIVER_INIT_MEMBER(spectrum_state,plus2)
{
	m_floppy = 0;
}

static SLOT_INTERFACE_START( specpls3_floppies )
	SLOT_INTERFACE( "3ssdd", FLOPPY_3_SSDD )
SLOT_INTERFACE_END

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

static GFXDECODE_START( specpls3 )
	GFXDECODE_ENTRY( "maincpu", 0x1fd00, spectrum_charlayout, 0, 8 )
GFXDECODE_END


static MACHINE_CONFIG_DERIVED( spectrum_plus3, spectrum_128 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(spectrum_plus3_io)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50.01)
	MCFG_GFXDECODE_MODIFY("gfxdecode", specpls3)

	MCFG_MACHINE_RESET_OVERRIDE(spectrum_state, spectrum_plus3 )

	MCFG_UPD765A_ADD("upd765", true, true)
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", specpls3_floppies, "3ssdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", specpls3_floppies, "3ssdd", floppy_image_device::default_floppy_formats)

	MCFG_SOFTWARE_LIST_ADD("flop_list", "specpls3_flop")
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(specpl2a)
	ROM_REGION(0x20000,"maincpu",0)
	ROM_LOAD("p2a41_0.rom",0x10000,0x4000, CRC(30c9f490) SHA1(62ec15a4af56cd1d206d0bd7011eac7c889a595d))
	ROM_LOAD("p2a41_1.rom",0x14000,0x4000, CRC(a7916b3f) SHA1(1a7812c383a3701e90e88d1da086efb0c033ac72))
	ROM_LOAD("p2a41_2.rom",0x18000,0x4000, CRC(c9a0b748) SHA1(8df145d10ff78f98138682ea15ebccb2874bf759))
	ROM_LOAD("p2a41_3.rom",0x1c000,0x4000, CRC(b88fd6e3) SHA1(be365f331942ec7ec35456b641dac56a0dbfe1f0))
ROM_END

ROM_START(specpls3)
	ROM_REGION(0x20000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English v4.0" )
	ROMX_LOAD("pl3-0.rom",0x10000,0x4000, CRC(17373da2) SHA1(e319ed08b4d53a5e421a75ea00ea02039ba6555b), ROM_BIOS(1))
	ROMX_LOAD("pl3-1.rom",0x14000,0x4000, CRC(f1d1d99e) SHA1(c9969fc36095a59787554026a9adc3b87678c794), ROM_BIOS(1))
	ROMX_LOAD("pl3-2.rom",0x18000,0x4000, CRC(3dbf351d) SHA1(22e50c6ba4157a3f6a821bd9937cd26e292775c6), ROM_BIOS(1))
	ROMX_LOAD("pl3-3.rom",0x1c000,0x4000, CRC(04448eaa) SHA1(65f031caa8148a5493afe42c41f4929deab26b4e), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "sp", "Spanish v.40" )
	ROMX_LOAD("plus3sp0.rom",0x10000,0x4000, CRC(1f86147a) SHA1(e9b0a60a1a8def511d59090b945d175bdc646346), ROM_BIOS(2))
	ROMX_LOAD("plus3sp1.rom",0x14000,0x4000, CRC(a8ac4966) SHA1(4e48f196427596c7990c175d135c15a039c274a4), ROM_BIOS(2))
	ROMX_LOAD("plus3sp2.rom",0x18000,0x4000, CRC(f6bb0296) SHA1(09fc005625589ef5992515957ce7a3167dec24b2), ROM_BIOS(2))
	ROMX_LOAD("plus3sp3.rom",0x1c000,0x4000, CRC(f6d25389) SHA1(ec8f644a81e2e9bcb58ace974103ea960361bad2), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "en41", "English v4.1" )
	ROMX_LOAD("plus341.rom",0x10000,0x10000, CRC(be0d9ec4) SHA1(500c0945760abeefcbd08bc22c0d07b14b336cf0), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "4ms", "Customize 3.5\" 4ms" )
	ROMX_LOAD("p3_01_4m.rom",0x10000,0x8000, CRC(ad99380a) SHA1(4e5d114b72d464cefdde0566457f52a3c0c1cae2), ROM_BIOS(4))
	ROMX_LOAD("p3_23_4m.rom",0x18000,0x8000, CRC(07727895) SHA1(752cdd6a083ab9910348995e483541d60bb6372b), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 4, "12ms", "Customize 3.5\" 12ms" )
	ROMX_LOAD("p3_01_cm.rom",0x10000,0x8000, CRC(ad99380a) SHA1(4e5d114b72d464cefdde0566457f52a3c0c1cae2), ROM_BIOS(5))
	ROMX_LOAD("p3_23_cm.rom",0x18000,0x8000, CRC(61f2b50c) SHA1(d062765ceb1f3cd2c94ea51cb737cac7ad6151b4), ROM_BIOS(5))
ROM_END

ROM_START(specpl3e)
	ROM_REGION(0x20000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English" )
	ROMX_LOAD("roma-en.rom",0x10000,0x8000, CRC(2d533344) SHA1(5ff2dae32eb745d87e0b54c595d1d20a866f316f), ROM_BIOS(1))
	ROMX_LOAD("romb-en.rom",0x18000,0x8000, CRC(ef8d5d92) SHA1(983aa53aa76e25a3af123c896016bacf6829b72b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "sp", "Spanish" )
	ROMX_LOAD("roma-es.rom",0x10000,0x8000, CRC(ba694b4b) SHA1(d15d9e43950483cffc79f1cfa89ecb114a88f6c2), ROM_BIOS(2))
	ROMX_LOAD("romb-es.rom",0x18000,0x8000, CRC(61ed94db) SHA1(935b14c13db75d872de8ad0d591aade0adbbc355), ROM_BIOS(2))
ROM_END

ROM_START(sp3e8bit)
	ROM_REGION(0x20000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English" )
	ROMX_LOAD("3e8biten.rom",0x10000,0x10000, CRC(beee3bf6) SHA1(364ec903916282d5401901c5fb0cb93a142038b3), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "sp", "Spanish" )
	ROMX_LOAD("3e8bites.rom",0x10000,0x10000, CRC(cafe4c35) SHA1(8331d273d29d3e37ec1324053bb050874d2c1434), ROM_BIOS(2))
ROM_END

ROM_START(sp3ezcf)
	ROM_REGION(0x20000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English" )
	ROMX_LOAD("3ezcfen.rom",0x10000,0x10000, CRC(43993f11) SHA1(27cbfbe8b5ef9eec6056026fa0b84fe158ba2f45), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "sp", "Spanish" )
	ROMX_LOAD("3ezcfes.rom",0x10000,0x10000, CRC(1325a0d7) SHA1(521cf47e10f46c8a621c8889ef1f008454c7e10b), ROM_BIOS(2))
ROM_END

ROM_START(sp3eata)
	ROM_REGION(0x20000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English" )
	ROMX_LOAD("3ezxaen.rom",0x10000,0x10000, CRC(dfb676dc) SHA1(37618bc66ae33dbf686be8a92867e4a9144b65dc), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "sp", "Spanish" )
	ROMX_LOAD("3ezxaes.rom",0x10000,0x10000, CRC(8f0ae91a) SHA1(71693e18b30c90914be58cba26682ca025c924ea), ROM_BIOS(2))
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE         INPUT       INIT    COMPANY     FULLNAME */
COMP( 1987, specpl2a, spec128,  0,      spectrum_plus3, spec_plus, spectrum_state,  plus2,  "Amstrad plc",          "ZX Spectrum +2a" , 0 )
COMP( 1987, specpls3, spec128,  0,      spectrum_plus3, spec_plus, spectrum_state,  plus3,  "Amstrad plc",          "ZX Spectrum +3" , 0 )
COMP( 2000, specpl3e, spec128,  0,      spectrum_plus3, spec_plus, spectrum_state,  plus3,  "Amstrad plc",          "ZX Spectrum +3e" , MACHINE_UNOFFICIAL )
COMP( 2002, sp3e8bit, spec128,  0,      spectrum_plus3, spec_plus, spectrum_state,  plus3,  "Amstrad plc",          "ZX Spectrum +3e 8bit IDE" , MACHINE_UNOFFICIAL )
COMP( 2002, sp3eata,  spec128,  0,      spectrum_plus3, spec_plus, spectrum_state,  plus3,  "Amstrad plc",          "ZX Spectrum +3e 8bit ZXATASP" , MACHINE_UNOFFICIAL )
COMP( 2002, sp3ezcf,  spec128,  0,      spectrum_plus3, spec_plus, spectrum_state,  plus3,  "Amstrad plc",          "ZX Spectrum +3e 8bit ZXCF" , MACHINE_UNOFFICIAL )
