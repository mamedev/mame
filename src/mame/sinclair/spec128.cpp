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


The hc2000 corrupts its memory, especially if you type something, and the
resulting mess can be seen in the F4 viewer display.

*******************************************************************************/

#include "emu.h"
#include "spec128.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#include "screen.h"

#include "formats/tzx_cas.h"


/****************************************************************************************************/
/* Spectrum 128 specific functions */

void spectrum_128_state::video_start()
{
	spectrum_state::video_start();
	m_screen_location = m_ram->pointer() + (5 << 14);
	m_border4t_render_at = 3;
}

uint8_t spectrum_128_state::spectrum_128_pre_opcode_fetch_r(offs_t offset)
{
	if (is_contended(offset)) content_early();

	/* this allows expansion devices to act upon opcode fetches from MEM addresses
	   for example, interface1 detection fetches requires fetches at 0008 / 0708 to
	   enable paged ROM and then fetches at 0700 to disable it
	*/
	m_exp->pre_opcode_fetch(offset);
	uint8_t retval = m_maincpu->space(AS_PROGRAM).read_byte(offset);
	m_exp->post_opcode_fetch(offset);
	return retval;
}

void spectrum_128_state::spectrum_128_rom_w(offs_t offset, uint8_t data)
{
	m_exp->mreq_w(offset, data);
}

u8 spectrum_128_state::spectrum_128_rom_r(offs_t offset)
{
	return m_exp->romcs()
		? m_exp->mreq_r(offset)
		: ((u8*)m_bank_rom[0]->base())[offset];
}

template <u8 Bank> void spectrum_128_state::spectrum_128_ram_w(offs_t offset, u8 data)
{
	u16 addr = 0x4000 * Bank + offset;
	if (is_contended(addr)) content_early();
	if (is_vram_write(addr)) m_screen->update_now();

	((u8*)m_bank_ram[Bank]->base())[offset] = data;
}
// Base 128 models typically don't share RAM in bank0. Reserved for extension in 256+.
template void spectrum_128_state::spectrum_128_ram_w<0>(offs_t offset, u8 data);

template <u8 Bank> u8 spectrum_128_state::spectrum_128_ram_r(offs_t offset)
{
	u16 addr = 0x4000 * Bank + offset;
	if (is_contended(addr)) content_early();

	return ((u8*)m_bank_ram[Bank]->base())[offset];
}

void spectrum_128_state::spectrum_128_port_7ffd_w(offs_t offset, uint8_t data)
{
	if (is_contended(offset)) content_early();
	content_early(1);

	/* D0-D2: RAM page located at 0x0c000-0x0ffff */
	/* D3 - Screen select (screen 0 in ram page 5, screen 1 in ram page 7 */
	/* D4 - ROM select - which rom paged into 0x0000-0x03fff */
	/* D5 - Disable paging */

	/* disable paging? */
	if (m_port_7ffd_data & 0x20) return;

	/* store new state */
	m_port_7ffd_data = data;

	/* update memory */
	spectrum_128_update_memory();

	m_exp->iorq_w(offset | 1, data);
}

void spectrum_128_state::spectrum_128_update_memory()
{
	m_bank_rom[0]->set_entry(BIT(m_port_7ffd_data, 4));
	/* select ram at 0x0c000-0x0ffff */
	m_bank_ram[3]->set_entry(m_port_7ffd_data & 0x07);

	m_screen->update_now();
	if (BIT(m_port_7ffd_data, 3))
		m_screen_location = m_ram->pointer() + (7<<14);
	else
		m_screen_location = m_ram->pointer() + (5<<14);
}

uint8_t spectrum_128_state::spectrum_port_r(offs_t offset)
{
	if (is_contended(offset))
	{
		content_early();
		content_late();
	}

	// Pass through to expansion device if present
	if (m_exp->get_card_device())
		return m_exp->iorq_r(offset | 1);

	return floating_bus_r();
}

void spectrum_128_state::spectrum_128_io(address_map &map)
{
	map(0x0000, 0x0000).select(0xfffe).rw(FUNC(spectrum_128_state::spectrum_ula_r), FUNC(spectrum_128_state::spectrum_ula_w));
	map(0x0001, 0x0001).select(0xfffe).rw(FUNC(spectrum_128_state::spectrum_port_r), FUNC(spectrum_128_state::spectrum_port_w));
	map(0x0001, 0x0001).select(0x7ffc).w(FUNC(spectrum_128_state::spectrum_128_port_7ffd_w));   // (A15 | A1) == 0, note: reading from this port does write to it by value from data bus
	map(0x8000, 0x8000).mirror(0x3ffd).w("ay8912", FUNC(ay8910_device::data_w));
	map(0xc000, 0xc000).mirror(0x3ffd).rw("ay8912", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
}

void spectrum_128_state::spectrum_128_mem(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(spectrum_128_state::spectrum_128_rom_r), FUNC(spectrum_128_state::spectrum_128_rom_w));
	map(0x4000, 0x7fff).rw(FUNC(spectrum_128_state::spectrum_128_ram_r<1>), FUNC(spectrum_128_state::spectrum_128_ram_w<1>));
	map(0x8000, 0xbfff).rw(FUNC(spectrum_128_state::spectrum_128_ram_r<2>), FUNC(spectrum_128_state::spectrum_128_ram_w<2>));
	map(0xc000, 0xffff).rw(FUNC(spectrum_128_state::spectrum_128_ram_r<3>), FUNC(spectrum_128_state::spectrum_128_ram_w<3>));
}

void spectrum_128_state::spectrum_128_fetch(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(spectrum_128_state::spectrum_128_pre_opcode_fetch_r));
}

void spectrum_128_state::machine_start()
{
	spectrum_state::machine_start();

	save_item(NAME(m_port_7ffd_data));

	/* rom 0 is 128K rom, rom 1 is 48 BASIC */
	memory_region *rom = memregion("maincpu");
	m_bank_rom[0]->configure_entries(0, 2, rom->base() + 0x10000, 0x4000);

	auto ram_entries = m_ram->size() / 0x4000;
	for (auto i = 1; i < 4; i++)
		m_bank_ram[i]->configure_entries(0, ram_entries, m_ram->pointer(), 0x4000);

	m_bank_ram[1]->set_entry(ram_entries > 5 ? 5 : (ram_entries - 1)); /* Bank 5 is always in 0x4000 - 0x7fff */
	m_bank_ram[2]->set_entry(2); /* Bank 2 is always in 0x8000 - 0xbfff */
}

void spectrum_128_state::machine_reset()
{
	spectrum_state::machine_reset();

	/* set initial ram config */
	m_port_7ffd_data = 0;
	spectrum_128_update_memory();
}

bool spectrum_128_state::is_vram_write(offs_t offset) {
	return (BIT(m_port_7ffd_data, 3) && m_bank_ram[3]->entry() == 7)
		? offset >= 0xc000 && offset < 0xdb00
		: spectrum_state::is_vram_write(offset);
}

bool spectrum_128_state::is_contended(offs_t offset) {
	u8 bank = m_bank_ram[3]->entry();
	return spectrum_state::is_contended(offset)
		|| ((offset >= 0xc000 && offset <= 0xffff) && (bank & 1)); // Memory banks 1,3,5 and 7 are contended
}

static const gfx_layout spectrum_charlayout =
{
	8, 8,           /* 8 x 8 characters */
	96,             /* 96 characters */
	1,              /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	{STEP8(0, 1)},  /* x offsets */
	{STEP8(0, 8)},  /* y offsets */
	8*8             /* every char takes 8 bytes */
};

static GFXDECODE_START( spec128 )
	GFXDECODE_ENTRY( "maincpu", 0x17d00, spectrum_charlayout, 7, 8 )
GFXDECODE_END

rectangle spectrum_128_state::get_screen_area()
{
	return { SPEC_LEFT_BORDER, SPEC_LEFT_BORDER + SPEC_DISPLAY_XSIZE - 1,
			SPEC128_UNSEEN_LINES + SPEC_TOP_BORDER, SPEC128_UNSEEN_LINES + SPEC_TOP_BORDER + SPEC_DISPLAY_YSIZE - 1 };
}

void spectrum_128_state::spectrum_128(machine_config &config)
{
	spectrum(config);

	Z80(config.replace(), m_maincpu, X1_128_SINCLAIR / 10);
	m_maincpu->set_memory_map(&spectrum_128_state::spectrum_128_mem);
	m_maincpu->set_io_map(&spectrum_128_state::spectrum_128_io);
	m_maincpu->set_m1_map(&spectrum_128_state::spectrum_128_fetch);
	m_maincpu->set_vblank_int("screen", FUNC(spectrum_128_state::spec_interrupt));
	m_maincpu->nomreq_cb().set(FUNC(spectrum_128_state::spectrum_nomreq));

	config.set_maximum_quantum(attotime::from_hz(60));

	/* video hardware */
	rectangle visarea = { get_screen_area().left() - SPEC_LEFT_BORDER, get_screen_area().right() + SPEC_RIGHT_BORDER,
		get_screen_area().top() - SPEC_TOP_BORDER, get_screen_area().bottom() + SPEC_BOTTOM_BORDER };
	m_screen->set_raw(X1_128_SINCLAIR / 5, SPEC128_CYCLES_PER_LINE * 2, SPEC128_UNSEEN_LINES + SPEC_SCREEN_HEIGHT, visarea);

	subdevice<gfxdecode_device>("gfxdecode")->set_info(spec128);

	/* sound hardware */
	AY8912(config, "ay8912", X1_128_SINCLAIR / 20).add_route(ALL_OUTPUTS, "mono", 0.25);

	/* expansion port */
	SPECTRUM_EXPANSION_SLOT(config.replace(), m_exp, spec128_expansion_devices, nullptr);
	m_exp->irq_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_exp->nmi_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_exp->fb_r_handler().set(FUNC(spectrum_128_state::floating_bus_r));

	/* internal ram */
	m_ram->set_default_size("128K");
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(spec128)
	ROM_REGION(0x18000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English" )
	ROMX_LOAD("zx128_0.rom",0x10000,0x4000, CRC(e76799d2) SHA1(4f4b11ec22326280bdb96e3baf9db4b4cb1d02c5), ROM_BIOS(0))
	ROMX_LOAD("zx128_1.rom",0x14000,0x4000, CRC(b96a36be) SHA1(80080644289ed93d71a1103992a154cc9802b2fa), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "sp", "Spanish" )
	ROMX_LOAD("zx128s0.rom",0x10000,0x4000, CRC(453d86b2) SHA1(968937b1c750f0ef6205f01c6db4148da4cca4e3), ROM_BIOS(1))
	ROMX_LOAD("zx128s1.rom",0x14000,0x4000, CRC(6010e796) SHA1(bea3f397cc705eafee995ea629f4a82550562f90), ROM_BIOS(1))
ROM_END

ROM_START(specpls2)
	ROM_REGION(0x18000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "en", "English" )
	ROMX_LOAD("zxp2_0.rom",0x10000,0x4000, CRC(5d2e8c66) SHA1(72703f9a3e734f3c23ec34c0727aae4ccbef9a91), ROM_BIOS(0))
	ROMX_LOAD("zxp2_1.rom",0x14000,0x4000, CRC(98b1320b) SHA1(de8b0d2d0379cfe7c39322a086ca6da68c7f23cb), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "fr", "French" )
	ROMX_LOAD("plus2fr0.rom",0x10000,0x4000, CRC(c684c535) SHA1(56684c4c85a616e726a50707483b9a42d8e724ed), ROM_BIOS(1))
	ROMX_LOAD("plus2fr1.rom",0x14000,0x4000, CRC(f5e509c5) SHA1(7e398f62689c9d90a36d3a101351ec9987207308), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "sp", "Spanish" )
	ROMX_LOAD("plus2sp0.rom",0x10000,0x4000, CRC(e807d06e) SHA1(8259241b28ff85441f1bedc2bee53445767c51c5), ROM_BIOS(2))
	ROMX_LOAD("plus2sp1.rom",0x14000,0x4000, CRC(41981d4b) SHA1(ec0d5a158842d20601b4fbeaefc6668db979d0e1), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "ao", "ZX Spectrum +2c (Andrew Owen)" )
	ROMX_LOAD("plus2c-0.rom",0x10000,0x4000, CRC(bfddf748) SHA1(3eba870bcb2c5efa906f2ca3febe960fc35d66bb), ROM_BIOS(3))
	ROMX_LOAD("plus2c-1.rom",0x14000,0x4000, CRC(fd8552b6) SHA1(5ffcf79f2154ba2cf42cc1d9cb4be93cb5043e73), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 4, "namco", "ZX Spectrum +2c (Namco)" )
	ROMX_LOAD("pl2namco.rom",0x10000,0x8000, CRC(72a54e75) SHA1(311400157df689450dadc3620f4c4afa960b05ad), ROM_BIOS(4))
ROM_END

ROM_START(hc128)  // Romanian clone, "ICE Felix HC91+" (aka HC-128), AY-8910 was optional
	ROM_REGION(0x18000,"maincpu",0)
	ROM_LOAD("zx128_0.rom",0x10000,0x4000, CRC(e76799d2) SHA1(4f4b11ec22326280bdb96e3baf9db4b4cb1d02c5))
	ROM_LOAD("hc128.rom",  0x14000,0x4000, CRC(0241e960) SHA1(cea0d14391b9e571460a816088a1c00ecb24afa3))
ROM_END


//    YEAR  NAME      PARENT   COMPAT  MACHINE       INPUT      STATE               INIT        COMPANY                  FULLNAME           FLAGS
COMP( 1986, spec128,  0,       0,      spectrum_128, spec128,   spectrum_128_state, empty_init, "Sinclair Research Ltd", "ZX Spectrum 128", 0 )
COMP( 1986, specpls2, spec128, 0,      spectrum_128, spec_plus, spectrum_128_state, empty_init, "Amstrad plc",           "ZX Spectrum +2",  0 )
COMP( 1991, hc128,    spec128, 0,      spectrum_128, spec_plus, spectrum_128_state, empty_init, "ICE-Felix",             "HC-91+ (HC-128)", 0 )
