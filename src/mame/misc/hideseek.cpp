// license:BSD-3-Clause
// copyright-holders:Guru
/* Hide & Seek

the AG-2 AX51201 should be the follow-up to the AG-1 AX51101 in gunpey.cpp

AS:
Current ROM code barely contains some valid SH-2 opcodes but not enough for a HD64F7045F28. i.e. It doesn't contain VBR set-up, valid irq routines,
a valid irq table and no internal SH-2 i/o set-up. sub-routine at 0xe0 also points to 0x4b0, which is full of illegal opcodes with current ROM.
The HD64F7045F28 manual says that there's an on-chip ROM at 0-0x3ffff (page 168), meaning that this one definitely needs a trojan/decap (and also an SH-2 core
that supports the very different i/o map ;-)

Guru:
It's called Hide and Seek. I don't know the manufacturer.
I dumped the ROMs and with a byte swap on U10 I see plain text with words like BET, PAYOUT, HIT etc so it looks like some kind of gambling game.
Main program ROMs are 16M flashROMs. The rest are 128M flashROMs. ROMs U8, U9 and U11 are empty.
PCB number is BO-023C
The CPU is a SH2 HD64F7045F28. Connected xtal is 7.3728MHz.
Main RAM is 2x NEC D431000 (128k x8-bit SRAM).
There are 3 chips NEC DX-102 (used on seta2/ssv).
The main gfx chip is a new type not previously used in MAME I think. Number is AG-2 AX51201. Connected OSC is 25MHz
Video RAM is 2x Etrontech EM639165TS-7G (8M x16-bit SDRAM)
Sound is OKI M9810. Connected xtal unknown speed. Written on it is 409G649.
Other stuff: NEC D4992 (RTC?) and xtal possibly 32.768kHz, 3V coin battery, 93L46 EEPROM, 1x 8-position DIPSW, 2x Lattice iM4A3-64 CPLDs; one on the main board and one on the ROM board.
*/



#include "emu.h"
#include "cpu/sh/sh7604.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class hideseek_state : public driver_device
{
public:
	hideseek_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void hideseek(machine_config &config);

	void init_hideseek();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void hideseek_palette(palette_device &palette) const;
	uint32_t screen_update_hideseek(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


void hideseek_state::video_start()
{
}


uint32_t hideseek_state::screen_update_hideseek(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void hideseek_state::mem_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom(); // On-chip ROM
	map(0x00200000, 0x0023ffff).ram(); // CS0
	map(0x00400000, 0x005fffff).rom().region("prg", 0); // CS1
	// CS2 - CS3
	map(0x01000000, 0x01ffffff).ram(); // DRAM
	map(0xffff8000, 0xffff87ff).ram(); // HD64F7045F28 i/os
	map(0xfffff000, 0xffffffff).ram(); // on-chip RAM
//  map(0x06000000, 0x07ffffff).rom().region("blit_data", 0);
}



static INPUT_PORTS_START( hideseek )
INPUT_PORTS_END



static GFXLAYOUT_RAW( hideseek, 2048, 1, 2048*8, 2048*8 )

static GFXDECODE_START( gfx_hideseek )
	GFXDECODE_ENTRY( "blit_data", 0, hideseek,     0x0000, 0x1 )
GFXDECODE_END


void hideseek_state::hideseek_palette(palette_device &palette) const
{
	for (int i = 0; i < 0x8000; i++)
		palette.set_pen_color(i, rgb_t(pal5bit((i >> 10)&0x1f), pal5bit((i >> 5)&0x1f), pal5bit((i >> 0)&0x1f)));
}



void hideseek_state::hideseek(machine_config &config)
{
	/* basic machine hardware */
	SH7604(config, m_maincpu, 7372800 * 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &hideseek_state::mem_map);
//  TIMER(config, "scantimer").configure_scanline(FUNC(hideseek_state::hideseek_scanline), "screen", 0, 1);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(hideseek_state::screen_update_hideseek));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(hideseek_state::hideseek_palette), 0x10000);
	GFXDECODE(config, "gfxdecode", "palette", gfx_hideseek);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	/* sound : M9810 */
}


ROM_START( hideseek )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "hd64f7045f28_internal_rom.bin",  0x000000, 0x040000, NO_DUMP ) // on chip ROM
	ROM_FILL(         0, 1, 0x00 )
	ROM_FILL(         1, 1, 0x40 )
	ROM_FILL(         2, 1, 0x00 )
	ROM_FILL(         3, 1, 0x00 )
	ROM_FILL(         4, 1, 0x01 )
	ROM_FILL(         5, 1, 0xff )
	ROM_FILL(         6, 1, 0xff )
	ROM_FILL(         7, 1, 0xfc )

	ROM_REGION32_BE( 0x400000, "prg", 0 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "29f160te.u10",  0x000000, 0x200000, CRC(44539f9b) SHA1(2e531455e5445e09e99494e47d96866e8ee07135) )
	ROM_LOAD16_WORD_SWAP( "29f160te.u11",  0x200000, 0x200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) ) /* empty! */

	ROM_REGION( 0x4000000, "blit_data", 0 )
	ROM_LOAD16_WORD_SWAP( "s29gl128n.u6",  0x0000000, 0x1000000,  CRC(2d6632f3) SHA1(e8d91bcc6975f5c07b438d29e8a23d403c7e52aa) )
	ROM_LOAD16_WORD_SWAP( "s29gl128n.u7",  0x1000000, 0x1000000,  CRC(9af057bf) SHA1(e905d0dfa5b866d7317adafe03fcdfadd1e44d78) )
	ROM_LOAD16_WORD_SWAP( "s29gl128n.u8",  0x2000000, 0x1000000,  CRC(a47ca14a) SHA1(3b4417fc421cee30a9ad0fd9319220a8dae32da2) ) /* empty! */
	ROM_LOAD16_WORD_SWAP( "s29gl128n.u9",  0x3000000, 0x1000000,  CRC(a47ca14a) SHA1(3b4417fc421cee30a9ad0fd9319220a8dae32da2) ) /* empty! */


	ROM_REGION( 0x1000000, "ymz", 0 )
	ROM_LOAD( "s29gl128n.u4",  0x000000, 0x1000000, CRC(b8eb7cdb) SHA1(a90c70058e50f3369a6e517e0a534b9ef1e0087c) )
ROM_END



void hideseek_state::init_hideseek()
{
}

} // anonymous namespace


GAME( 200?, hideseek, 0, hideseek, hideseek, hideseek_state, init_hideseek, ROT0, "<unknown>", "Hide & Seek", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
