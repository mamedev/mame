// license:BSD-3-Clause
// copyright-holders:David Haywood

// these contain the similar game selections to the games in unk6502_st2xxx.cpp but on updated hardware

// These use SPI ROMs and unSP2.0 instructions, so will be GeneralPlus branded parts, not SunPlus
// possibly the framebuffer based video ones rather than the ones with tile layers

/*

for pcp8728 long jumps are done indirect via a call to RAM

990c 20ec       r4 = 20ec
d9dd            [1d] = r4
990c 0007       r4 = 0007
d9de            [1e] = r4
fe80 28f7       goto 0028f7

the code to handle this is copied in at startup.

Almost all function calls in the game are handled via a call to RAM which copies data inline from SPI for execution
these calls manage their own stack, and copying back the caller function on return etc.

The largest function in RAM at any one time is ~0x600 bytes.

This appears to be incredibly inefficient but the system can't execute directly from SPI ROM, and doesn't have any
RAM outside of the small area internal to the Sunplus SoC

Graphics likewise appear to be loaded pixel by pixel from the SPI to framebuffer every single time there is a draw
call.  Sound is almost certainly handled in the same way.

There is a missing internal ROM that acts as bootstrap and provides some basic functions.  It is at least 0x1000
words in size, with the lowest call being to 0xf000.  It is potentially larger than this.

Calls:

0xf000 - copy dword from SPI using provided pointer

*/

#include "emu.h"

#include "cpu/unsp/unsp.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class pcp8718_state : public driver_device
{
public:
	pcp8718_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_spirom(*this, "spi")
	{ }

	void pcp8718(machine_config &config);

	void spi_init();

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<unsp_20_device> m_maincpu;
	required_shared_ptr<uint16_t> m_mainram;

	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	void map(address_map &map);

	uint16_t simulate_f000_r(offs_t offset);

	required_region_ptr<uint16_t> m_spirom;

	uint16_t unk_7abf_r();
};

uint32_t pcp8718_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}




void pcp8718_state::machine_start()
{
}

static INPUT_PORTS_START( pcp8718 )
INPUT_PORTS_END

uint16_t pcp8718_state::unk_7abf_r()
{
	return 0x0001;
}

void pcp8718_state::map(address_map &map)
{
	// there are calls to 01xxx and 02xxx regions
	// (RAM populated by internal ROM?, TODO: check to make sure code copied there isn't from SPI ROM like the GPL16250 bootstrap
	//  does from NAND, it doesn't seem to have a header in the same format at least)
	map(0x000000, 0x006fff).ram().share("mainram");
	map(0x007000, 0x0077ff).ram(); // might be registers, but the call stubs for RAM calls explicitly use addresses in here for private stack so that previous snippets can be restored?

	map(0x007abf, 0x007abf).r(FUNC(pcp8718_state::unk_7abf_r));
	

	// registers at 7xxx are similar to GPL16250, but not identical? (different video system?)

	// there are calls to 0x0f000 (internal ROM?)
	map(0x00f000, 0x00ffff).rom().region("maincpu", 0x00000);

	// seems to have same memory config registers etc. as GPL16250 so CS Space starts at 0x020000 and the 'bank' is likely at 0x200000 too
	map(0x020000, 0x3fffff).rom().region("spi", 0x00000);
}



uint16_t pcp8718_state::simulate_f000_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if ((offset+0xf000) == 0xf000)
		{

			uint16_t pc = m_maincpu->state_int(UNSP_PC);
			uint16_t sr = m_maincpu->state_int(UNSP_SR);

			int realpc = (pc | (sr << 16)) & 0x003fffff;
			if (realpc == 0xf000)
			{
				address_space& mem = m_maincpu->space(AS_PROGRAM);

				uint32_t source = (mem.read_word(0x001e) << 16) | mem.read_word(0x001d);

				if (source >= 0x20000)
				{
					uint16_t data = m_spirom[(source - 0x20000)];
					uint16_t data2 = m_spirom[(source - 0x20000) + 1];

					logerror("call to 0xf000 - copying from %08x to 04/05\n", source); // some code only uses 04, but other code copies pointers and expects results in 04 and 05

					mem.write_word(0x0004, data);
					mem.write_word(0x0005, data2);
				}
				else
				{
					logerror("call to 0xf000 - invalid source %08x\n", source);
				}
			}
		}
		else
		{
			fatalerror("simulate_f000_r unhandled BIOS simulation offset %04x\n", offset);
		}
	}
	return 0x9a90; // retf
}

void pcp8718_state::machine_reset()
{
	// this looks like it might actually be part of the IRQ handler (increase counter at 00 at the very start) rather than where we should end up after startup
	// it also looks like (from the pc = 2xxx opcodes) that maybe this code should be being executed in RAM as those don't give correct offsets in the data segment.
	m_maincpu->set_state_int(UNSP_PC, 0x4000);
	m_maincpu->set_state_int(UNSP_SR, 0x0000);

	//uint16_t* ROM = (uint16_t*)memregion("maincpu")->base();
	//ROM[0x0000] = 0x9a90; // retf from internal ROM call to 0xf000 (unknown purpose)	

	// there doesn't appear to be any code to set the SP, so it must be done by the internal ROM
	m_maincpu->set_state_int(UNSP_SP, 0x5fff);

	m_maincpu->space(AS_PROGRAM).install_read_handler(0xf000, 0xffff, read16sm_delegate(*this, FUNC(pcp8718_state::simulate_f000_r)));

}

void pcp8718_state::pcp8718(machine_config &config)
{

	UNSP_20(config, m_maincpu, 20000000); // unknown CPU, unsp20 based
	m_maincpu->set_addrmap(AS_PROGRAM, &pcp8718_state::map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(pcp8718_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200);
}

// pcp8718 and pcp8728 both contain user data (player name?) and will need to be factory defaulted once they work
// the ROM code is slightly different between them

ROM_START( pcp8718 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x2000, NO_DUMP ) // exact size unknown

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	//ROM_LOAD16_WORD_SWAP( "8718_en25f32.bin", 0x000000, 0x400000, CRC(cc138db4) SHA1(379af3d94ae840f52c06416d6cf32e25923af5ae) ) // bad dump, some blocks are corrupt
	ROM_LOAD16_WORD_SWAP( "eyecare_25q32av1g_ef4016.bin", 0x000000, 0x400000, CRC(58415e10) SHA1(b1adcc03f2ad8d741544204671677740e904ce1a) )
ROM_END

ROM_START( pcp8728 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x2000, NO_DUMP ) // exact size unknown

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "pcp 8728 788 in 1.bin", 0x000000, 0x400000, CRC(60115f21) SHA1(e15c39f11e442a76fae3823b6d510178f6166926) )
ROM_END

ROM_START( unkunsp )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x2000, NO_DUMP ) // exact size unknown

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "fm25q16a.bin", 0x000000, 0x200000, CRC(aeb472ac) SHA1(500c24b725f6d3308ef8cbdf4259f5be556c7c92) )
ROM_END


void pcp8718_state::spi_init()
{
	uint16_t* rom = (uint16_t*)memregion("spi")->base();

	uint32_t start = rom[0] | rom[1] << 16;
	uint32_t end =   rom[2] | rom[1] << 16;


	logerror("start: %08x\n", start);
	logerror("end: %08x\n", end);

	int writebase = 0x4000;

	if (start > end)
	{
		logerror("invalid initial copy?\n");
		return;
	}
	else
	{
		start -= 0x20000;
		end -= 0x20000;

		for (int i = start; i <= end; i++)
		{
			uint16_t dat = rom[i];
			m_mainram[writebase++] = dat;
		}
	}

}


CONS( 200?, pcp8718,      0,       0,      pcp8718,   pcp8718, pcp8718_state, spi_init, "PCP", "PCP 8718 - HD 360 Degrees Rocker Palm Eyecare Console - 788 in 1", MACHINE_IS_SKELETON )
CONS( 200?, pcp8728,      0,       0,      pcp8718,   pcp8718, pcp8718_state, spi_init, "PCP", "PCP 8728 - 788 in 1", MACHINE_IS_SKELETON ) // what name was this sold under?

// maybe different hardware, first 0x2000 bytes in ROM is blank, so bootstrap pointers aren't there at least
CONS( 200?, unkunsp,      0,       0,      pcp8718,   pcp8718, pcp8718_state, empty_init, "<unknown>", "unknown unSP based handheld", MACHINE_IS_SKELETON )
