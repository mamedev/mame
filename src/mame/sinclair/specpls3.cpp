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
26/3/2000   DJR -   Snapshot files are now classified as snapshots not
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
                made much more software running (i.e. Paperboy).
            Corrected frames per second value for 48k and 128k
            Sinclair machines.
                There are 50.08 frames per second for Spectrum
                48k what gives 69888 cycles for each frame and
                50.021 for Spectrum 128/+2/+2A/+3 what gives
                70908 cycles for each frame.
            Remapped some Spectrum+ keys.
                Pressing F3 to reset was setting 0xf7 on keyboard
                input port. Problem occurred for snapshots of
                some programs where it was read as pressing
                key 4 (which is exit in Tapecopy by R. Dannhoefer
                for example).
            Added support to load .SP snapshots.
            Added .BLK tape images support.
                .BLK files are identical to .TAP ones, extension
                is an only difference.
08/03/2002  KS -    #FF port emulation added.
                Arkanoid works now, but is not playable due to
                completely messed timings.

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
#include "specpls3.h"

#include "sound/ay8910.h"

#include "screen.h"
#include "softlist_dev.h"

#include "formats/ipf_dsk.h"
#include "formats/tzx_cas.h"

#define VERBOSE 0
#include "logmacro.h"

/****************************************************************************************************/
/* Spectrum + 3 specific functions */
/* This driver uses some of the spectrum_128 functions. The +3 is similar to a spectrum 128
but with a disc drive */


static const int spectrum_plus3_memory_selections[]=
{
	0, 1, 2, 3,
	4, 5, 6, 7,
	4, 5, 6, 3,
	4, 7, 6, 3
};

void specpls3_state::port_3ffd_w(offs_t offset, uint8_t data)
{
	if (m_upd765.found()) m_upd765->fifo_w(data);

	/* mface3 needs to see this port */
	if (m_exp) m_exp->iorq_w(offset | 0x3000, data);
}

uint8_t specpls3_state::port_3ffd_r()
{
	return m_upd765.found() ? m_upd765->fifo_r() : 0xff;
}


uint8_t specpls3_state::port_2ffd_r()
{
	return m_upd765.found() ? m_upd765->msr_r() : 0xff;
}


void specpls3_state::plus3_update_memory()
{
	m_screen->update_now();
	if (m_port_7ffd_data & 8)
	{
		LOG("+3 SCREEN 1: BLOCK 7\n");
		m_screen_location = m_ram->pointer() + (7 << 14);
	}
	else
	{
		LOG("+3 SCREEN 0: BLOCK 5\n");
		m_screen_location = m_ram->pointer() + (5 << 14);
	}

	if (m_port_1ffd_data & 0x01)
	{
		/* Extended memory paging */
		int MemorySelection = (m_port_1ffd_data >> 1) & 0x03;
		const int *memory_selection = &spectrum_plus3_memory_selections[(MemorySelection << 2)];
		m_bank_ram[0]->set_entry(memory_selection[0]);
		m_bank_ram[1]->set_entry(memory_selection[1]);
		m_bank_ram[2]->set_entry(memory_selection[2]);
		m_bank_ram[3]->set_entry(memory_selection[3]);
		LOG("extended memory paging: %02x\n", MemorySelection);
	}
	else
	{
		m_bank_rom[0]->set_entry(BIT(m_port_7ffd_data, 4) | ((m_port_1ffd_data >> 1) & 0x02));

		/* Reset memory between 0x4000 - 0xbfff in case extended paging was being used */
		/* Bank 5 in 0x4000 - 0x7fff */
		m_bank_ram[1]->set_entry(5);
		/* Bank 2 in 0x8000 - 0xbfff */
		m_bank_ram[2]->set_entry(2);
		/* select ram at 0x0c000-0x0ffff */
		int ram_page = m_port_7ffd_data & 0x07;
		m_bank_ram[3]->set_entry(ram_page);
		LOG("RAM at 0xc000: %02x\n", ram_page);
	}
}


void specpls3_state::rom_w(offs_t offset, uint8_t data)
{
	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
	else if (m_port_1ffd_data & 0x01)
		((u8*)m_bank_ram[0]->base())[offset] = data;
}

uint8_t specpls3_state::rom_r(offs_t offset)
{
	return m_exp->romcs()
		? m_exp->mreq_r(offset)
		: (m_port_1ffd_data & 0x01)
		  ? ((u8*)m_bank_ram[0]->base())[offset]
		  : ((u8*)m_bank_rom[0]->base())[offset];
}

void specpls3_state::port_7ffd_w(offs_t offset, uint8_t data)
{
	/* D0-D2 - RAM page located at 0x0c000-0x0ffff */
	/* D3    - Screen select (screen 0 in ram page 5, screen 1 in ram page 7 */
	/* D4    - ROM select low bit - which rom paged into 0x0000-0x03fff */
	/* D5    - Disable paging (permanent until reset) */

	/* mface3 needs to see this port */
	if (m_exp) m_exp->iorq_w(offset | 0x4000, data);

	/* paging disabled? */
	if (m_port_7ffd_data & 0x20) return;

	/* store new state */
	m_port_7ffd_data = data;

	/* update memory */
	plus3_update_memory();
}

void specpls3_state::port_1ffd_w(offs_t offset, uint8_t data)
{
	/* D0=0 - Normal ROM/RAM paging mode */
	/*   D1 - Not used */
	/*   D2 - Rom select high bit */
	/* D0=1 - Special RAM paging mode (all-RAM CP/M modes) */
	/*  D1-D2 - Special paging mode 0-3 */
	/* D3   - Disk motor on/off */
	/* D4   - Parallel port strobe */

	if (m_upd765.found())
	{
		for (auto &flop : m_flop)
			if (flop->get_device()) flop->get_device()->mon_w(!BIT(data, 3));
	}

	/* mface3 needs to see this port */
	if (m_exp) m_exp->iorq_w(offset | 0x1000, data);

	/* paging disabled? */
	if ((m_port_7ffd_data & 0x20)==0)
	{
		/* no */
		m_port_1ffd_data = data;
		plus3_update_memory();
	}
	else
	{
		/* yes, update only non-memory related */
		m_port_1ffd_data &= 0x7;
		m_port_1ffd_data |= data & 0xf8;
	}
}

void specpls3_state::video_start()
{
	spectrum_128_state::video_start();
	m_contention_pattern = {1, 0, 7, 6, 5, 4, 3, 2};
	m_contention_offset = 1;
	m_border4t_render_at = 5;
}

/* ports are not decoded full.
The function decodes the ports appropriately */
void specpls3_state::plus3_io(address_map &map)
{
	map(0x0000, 0xffff).rw(m_exp, FUNC(spectrum_expansion_slot_device::iorq_r), FUNC(spectrum_expansion_slot_device::iorq_w));
	map(0x0000, 0x0000).rw(FUNC(specpls3_state::spectrum_ula_r), FUNC(specpls3_state::spectrum_ula_w)).select(0xfffe);
	map(0x4000, 0x4000).w(FUNC(specpls3_state::port_7ffd_w)).select(0x3ffd);
	map(0x8000, 0x8000).w("ay8912", FUNC(ay8910_device::data_w)).mirror(0x3ffd);
	map(0xc000, 0xc000).rw("ay8912", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w)).mirror(0x3ffd);
	map(0x1000, 0x1000).w(FUNC(specpls3_state::port_1ffd_w)).select(0x0ffd);
	map(0x2000, 0x2000).r(FUNC(specpls3_state::port_2ffd_r)).mirror(0x0ffd);
	map(0x3000, 0x3000).rw(FUNC(specpls3_state::port_3ffd_r), FUNC(specpls3_state::port_3ffd_w)).select(0x0ffd);
}

void specpls3_state::plus3_mem(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(specpls3_state::rom_r), FUNC(specpls3_state::rom_w));
	map(0x4000, 0x7fff).rw(FUNC(specpls3_state::spectrum_128_ram_r<1>), FUNC(specpls3_state::spectrum_128_ram_w<1>));
	map(0x8000, 0xbfff).rw(FUNC(specpls3_state::spectrum_128_ram_r<2>), FUNC(specpls3_state::spectrum_128_ram_w<2>));
	map(0xc000, 0xffff).rw(FUNC(specpls3_state::spectrum_128_ram_r<3>), FUNC(specpls3_state::spectrum_128_ram_w<3>));
}

void specpls3_state::machine_start()
{
	spectrum_128_state::machine_start();

	save_item(NAME(m_port_1ffd_data));

	// reconfigure ROMs
	memory_region *rom = memregion("maincpu");
	m_bank_rom[0]->configure_entries(0, 4, rom->base() + 0x10000, 0x4000);

	m_bank_ram[0]->configure_entries(0, m_ram->size() / 0x4000, m_ram->pointer(), 0x4000);
}

void specpls3_state::machine_reset()
{
	/* Initial configuration */
	m_port_fe_data = -1;
	m_port_7ffd_data = 0;
	m_port_1ffd_data = 0;
	plus3_update_memory();
}

void specpls3_state::plus3_us_w(uint8_t data)
{
	// US1 is not connected, so US0 alone selects either drive
	floppy_image_device *flop = m_flop[data & 1]->get_device();
	m_upd765->set_floppy(flop);
	if (flop) flop->ds_w(data & 1);
}

static void specpls3_floppies(device_slot_interface &device)
{
	device.option_add("3ssdd", FLOPPY_3_SSDD);
	device.option_add("35ssdd", FLOPPY_35_SSDD);
}

void specpls3_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_IPF_FORMAT);
}

bool specpls3_state::is_contended(offs_t offset)
{
	u8 bank = m_bank_ram[3]->entry();
	return spectrum_state::is_contended(offset)
		|| ((offset >= 0xc000 && offset <= 0xffff) && (bank & 4)); // Memory banks 4, 5, 6 and 7 are contended
}

/* F4 Character Displayer */
static const gfx_layout spectrum_charlayout =
{
	8, 8,          /* 8 x 8 characters */
	96,            /* 96 characters */
	1,             /* 1 bits per pixel */
	{ 0 },         /* no bitplanes */
	{STEP8(0, 1)}, /* x offsets */
	{STEP8(0, 8)}, /* y offsets */
	8*8            /* every char takes 8 bytes */
};

static GFXDECODE_START( specpls3 )
	GFXDECODE_ENTRY( "maincpu", 0x1fd00, spectrum_charlayout, 7, 8 )
GFXDECODE_END


void specpls3_state::spectrum_plus2(machine_config &config)
{
	spectrum_128(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &specpls3_state::plus3_mem);
	m_maincpu->set_addrmap(AS_IO, &specpls3_state::plus3_io);
	m_maincpu->nomreq_cb().set_nop();

	subdevice<gfxdecode_device>("gfxdecode")->set_info(specpls3);

	SPECTRUM_EXPANSION_SLOT(config.replace(), m_exp, specpls3_expansion_devices, nullptr);
	m_exp->irq_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_exp->nmi_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	// these models don't have floating bus
	m_exp->fb_r_handler().set([]() { return 0xff; });
}

void specpls3_state::spectrum_plus3(machine_config &config)
{
	spectrum_plus2(config);

	UPD765A(config, m_upd765, 16_MHz_XTAL / 4, true, false); // clocked through SED9420
	m_upd765->us_wr_callback().set(FUNC(specpls3_state::plus3_us_w));
	FLOPPY_CONNECTOR(config, "upd765:0", specpls3_floppies, "3ssdd", specpls3_state::floppy_formats).enable_sound(true); // internal drive
	FLOPPY_CONNECTOR(config, "upd765:1", specpls3_floppies, nullptr, specpls3_state::floppy_formats).enable_sound(true); // external drive

	SOFTWARE_LIST(config, "flop_list").set_original("specpls3_flop");
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

/* Amstrad built +2A/+2B/+3/+3B models:

    +2A/B has built-in tape "datacorder", +3/B has built-in 3" fdd
    +2A/+3 use common z70830 pcb with v4.0 rom  (fdc etc. unpopulated on +2A)
    +2B/+3B use unique z70833/z70835 pcbs but use same v4.1 rom

   Note, +2 (non-A/B, aka "grey case") although Amstrad built is essentially a re-cased Sinclair 128K, see spec128.cpp
*/
ROM_START(specpl2a)
	ROM_REGION(0x20000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English v4.0" )  // +2A
	ROMX_LOAD("40092.ic7",0x10000,0x8000, CRC(9bc85686) SHA1(5992daf925f6e225fc0d01f7640282954d092ef4), ROM_BIOS(0))
	ROMX_LOAD("40093.ic8",0x18000,0x8000, CRC(db551783) SHA1(a0432adcca03f849fb39b6dce6414740cf4aecd2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "sp", "Spanish v4.0" )
	ROMX_LOAD("40094.ic7",0x10000,0x8000, CRC(392242fb) SHA1(976ae88951f8d1beb5d107f048950118a7133823), ROM_BIOS(1))
	ROMX_LOAD("40101.ic8",0x18000,0x8000, CRC(5daaae01) SHA1(09ca25b4dbec064a4964ab7a41d48404199afd77), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "enb", "English v4.1" )  // +2B
	ROMX_LOAD("40092u.ic7",0x10000,0x8000, CRC(80808d82) SHA1(b9e88ec18f844ce42ecb7802d82c2bda65f9c4f2), ROM_BIOS(2))
	ROMX_LOAD("40093u.ic8",0x18000,0x8000, CRC(61f2b50c) SHA1(d062765ceb1f3cd2c94ea51cb737cac7ad6151b4), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "spb", "Spanish v4.1" )
	ROMX_LOAD("40094s.ic7",0x10000,0x8000, CRC(9d102acf) SHA1(c525bd23f79ca968d34a0efdcc47b2eb342007f5), ROM_BIOS(3))
	ROMX_LOAD("40101s.ic8",0x18000,0x8000, CRC(1408ddce) SHA1(56eb124d44ee8c8daef130be4d7e735ec412c4ba), ROM_BIOS(3))
ROM_END

ROM_START(specpls3)
	ROM_REGION(0x20000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English v4.0" )  // +3
	ROMX_LOAD("40092.ic7",0x10000,0x8000, CRC(9bc85686) SHA1(5992daf925f6e225fc0d01f7640282954d092ef4), ROM_BIOS(0))
	ROMX_LOAD("40093.ic8",0x18000,0x8000, CRC(db551783) SHA1(a0432adcca03f849fb39b6dce6414740cf4aecd2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "sp", "Spanish v4.0" )
	ROMX_LOAD("40094.ic7",0x10000,0x8000, CRC(392242fb) SHA1(976ae88951f8d1beb5d107f048950118a7133823), ROM_BIOS(1))
	ROMX_LOAD("40101.ic8",0x18000,0x8000, CRC(5daaae01) SHA1(09ca25b4dbec064a4964ab7a41d48404199afd77), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "enb", "English v4.1" )  // +3B
	ROMX_LOAD("40092u.ic7",0x10000,0x8000, CRC(80808d82) SHA1(b9e88ec18f844ce42ecb7802d82c2bda65f9c4f2), ROM_BIOS(2))
	ROMX_LOAD("40093u.ic8",0x18000,0x8000, CRC(61f2b50c) SHA1(d062765ceb1f3cd2c94ea51cb737cac7ad6151b4), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "spb", "Spanish v4.1" )
	ROMX_LOAD("40094s.ic7",0x10000,0x8000, CRC(9d102acf) SHA1(c525bd23f79ca968d34a0efdcc47b2eb342007f5), ROM_BIOS(3))
	ROMX_LOAD("40101s.ic8",0x18000,0x8000, CRC(1408ddce) SHA1(56eb124d44ee8c8daef130be4d7e735ec412c4ba), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 4, "4ms", "Customize 3.5\" 4ms" )  // unofficial 3.5" fdd hacks by Cristian Secară (v4.1 english)
	ROMX_LOAD("p3_01_4m.rom",0x10000,0x8000, CRC(ad99380a) SHA1(4e5d114b72d464cefdde0566457f52a3c0c1cae2), ROM_BIOS(4))
	ROMX_LOAD("p3_23_4m.rom",0x18000,0x8000, CRC(07727895) SHA1(752cdd6a083ab9910348995e483541d60bb6372b), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 5, "12ms", "Customize 3.5\" 12ms" )
	ROMX_LOAD("p3_01_cm.rom",0x10000,0x8000, CRC(ad99380a) SHA1(4e5d114b72d464cefdde0566457f52a3c0c1cae2), ROM_BIOS(5))
	ROMX_LOAD("p3_23_cm.rom",0x18000,0x8000, CRC(61f2b50c) SHA1(d062765ceb1f3cd2c94ea51cb737cac7ad6151b4), ROM_BIOS(5))
ROM_END

ROM_START(specpl3e)
	ROM_REGION(0x20000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English" )
	ROMX_LOAD("roma-en.rom",0x10000,0x8000, CRC(2d533344) SHA1(5ff2dae32eb745d87e0b54c595d1d20a866f316f), ROM_BIOS(0))
	ROMX_LOAD("romb-en.rom",0x18000,0x8000, CRC(ef8d5d92) SHA1(983aa53aa76e25a3af123c896016bacf6829b72b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "sp", "Spanish" )
	ROMX_LOAD("roma-es.rom",0x10000,0x8000, CRC(ba694b4b) SHA1(d15d9e43950483cffc79f1cfa89ecb114a88f6c2), ROM_BIOS(1))
	ROMX_LOAD("romb-es.rom",0x18000,0x8000, CRC(61ed94db) SHA1(935b14c13db75d872de8ad0d591aade0adbbc355), ROM_BIOS(1))
ROM_END

ROM_START(sp3e8bit)
	ROM_REGION(0x20000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English" )
	ROMX_LOAD("3e8biten.rom",0x10000,0x10000, CRC(beee3bf6) SHA1(364ec903916282d5401901c5fb0cb93a142038b3), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "sp", "Spanish" )
	ROMX_LOAD("3e8bites.rom",0x10000,0x10000, CRC(cafe4c35) SHA1(8331d273d29d3e37ec1324053bb050874d2c1434), ROM_BIOS(1))
ROM_END

ROM_START(sp3ezcf)
	ROM_REGION(0x20000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English" )
	ROMX_LOAD("3ezcfen.rom",0x10000,0x10000, CRC(43993f11) SHA1(27cbfbe8b5ef9eec6056026fa0b84fe158ba2f45), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "sp", "Spanish" )
	ROMX_LOAD("3ezcfes.rom",0x10000,0x10000, CRC(1325a0d7) SHA1(521cf47e10f46c8a621c8889ef1f008454c7e10b), ROM_BIOS(1))
ROM_END

ROM_START(sp3eata)
	ROM_REGION(0x20000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English" )
	ROMX_LOAD("3ezxaen.rom",0x10000,0x10000, CRC(dfb676dc) SHA1(37618bc66ae33dbf686be8a92867e4a9144b65dc), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "sp", "Spanish" )
	ROMX_LOAD("3ezxaes.rom",0x10000,0x10000, CRC(8f0ae91a) SHA1(71693e18b30c90914be58cba26682ca025c924ea), ROM_BIOS(1))
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE         INPUT      CLASS           INIT        COMPANY        FULLNAME                         FLAGS */
COMP( 1987, specpl2a, 0,        0,     spectrum_plus2, spec_plus, specpls3_state, empty_init, "Amstrad plc", "ZX Spectrum +2a",              0 )
COMP( 1987, specpls3, specpl2a, 0,     spectrum_plus3, spec_plus, specpls3_state, empty_init, "Amstrad plc", "ZX Spectrum +3",               0 )
COMP( 2000, specpl3e, 0,        0,     spectrum_plus3, spec_plus, specpls3_state, empty_init, "Amstrad plc", "ZX Spectrum +3e",              MACHINE_UNOFFICIAL )
COMP( 2002, sp3e8bit, 0,        0,     spectrum_plus3, spec_plus, specpls3_state, empty_init, "Amstrad plc", "ZX Spectrum +3e 8bit IDE",     MACHINE_UNOFFICIAL )
COMP( 2002, sp3eata,  0,        0,     spectrum_plus3, spec_plus, specpls3_state, empty_init, "Amstrad plc", "ZX Spectrum +3e 8bit ZXATASP", MACHINE_UNOFFICIAL )
COMP( 2002, sp3ezcf,  0,        0,     spectrum_plus3, spec_plus, specpls3_state, empty_init, "Amstrad plc", "ZX Spectrum +3e 8bit ZXCF",    MACHINE_UNOFFICIAL )
