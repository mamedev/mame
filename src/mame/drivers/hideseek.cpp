// license:BSD-3-Clause
// copyright-holders:Guru
/* Hide & Seek

the AG-2 AX51201 should be the follow-up to the AG-1 AX51101 in gunpey.c

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
#include "cpu/sh2/sh2.h"


class hideseek_state : public driver_device
{
public:
	hideseek_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
		{ }

	required_device<cpu_device> m_maincpu;


	DECLARE_DRIVER_INIT(hideseek);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(hideseek);
	UINT32 screen_update_hideseek(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

};


void hideseek_state::video_start()
{
}


UINT32 hideseek_state::screen_update_hideseek(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 32, hideseek_state )
	AM_RANGE(0x00000000, 0x0003ffff) AM_ROM // On-chip ROM
	AM_RANGE(0x00200000, 0x0023ffff) AM_RAM // CS0
	AM_RANGE(0x00400000, 0x005fffff) AM_ROM AM_REGION("prg", 0) // CS1
	// CS2 - CS3
	AM_RANGE(0x01000000, 0x01ffffff) AM_RAM // DRAM
	AM_RANGE(0xffff8000, 0xffff87ff) AM_RAM // HD64F7045F28 i/os
	AM_RANGE(0xfffff000, 0xffffffff) AM_RAM // on-chip RAM
//  AM_RANGE(0x06000000, 0x07ffffff) AM_ROM AM_REGION("blit_data", 0)
ADDRESS_MAP_END



static INPUT_PORTS_START( hideseek )
INPUT_PORTS_END



static GFXLAYOUT_RAW( hideseek, 2048, 1, 2048*8, 2048*8 )

static GFXDECODE_START( hideseek )
	GFXDECODE_ENTRY( "blit_data", 0, hideseek,     0x0000, 0x1 )
GFXDECODE_END


PALETTE_INIT_MEMBER(hideseek_state, hideseek)
{
	int i;

	for (i = 0; i < 0x8000; i++)
		palette.set_pen_color(i, rgb_t( pal5bit((i >> 10)&0x1f), pal5bit(((i >> 5))&0x1f), pal5bit((i >> 0)&0x1f)));
}



static MACHINE_CONFIG_START( hideseek, hideseek_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH2, 7372800 * 4 )
	MCFG_CPU_PROGRAM_MAP(mem_map)
//  MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", hideseek_state, hideseek_scanline, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(hideseek_state, screen_update_hideseek)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x10000)
	MCFG_PALETTE_INIT_OWNER(hideseek_state, hideseek)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hideseek)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker","rspeaker")

	/* sound : M9810 */
MACHINE_CONFIG_END


ROM_START( hideseek )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "hd64f7045f28_internal_rom.bin",  0x000000, 0x040000, NO_DUMP ) // on chip ROM
	ROM_FILL(         0, 1, nullptr )
	ROM_FILL(         1, 1, 0x40 )
	ROM_FILL(         2, 1, nullptr )
	ROM_FILL(         3, 1, nullptr )
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



DRIVER_INIT_MEMBER(hideseek_state,hideseek)
{
}


GAME( 200?, hideseek, 0, hideseek, hideseek, hideseek_state, hideseek,    ROT0, "<unknown>", "Hide & Seek",MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
