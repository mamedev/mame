// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont
/* The Legend of Silk Road - Unico 1999 */

/* Preliminary Driver by David Haywood */
/* Inputs, DIPs by Stephh & R. Belmont */
/* and preliminary sound hookup by R. Belmont + fixes by Pierpaolo Prazzoli */

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/silkroad.h"

/*

68020 interrupts
lev 1 : 0x64 : 0000 01d6 - just rte
lev 2 : 0x68 : 0000 01d6 - just rte
lev 3 : 0x6c : 0000 01d6 - just rte
lev 4 : 0x70 : 0000 012c - vblank
lev 5 : 0x74 : 0000 01d6 - just rte
lev 6 : 0x78 : 0000 01d6 - just rte
lev 7 : 0x7c : 0000 01d6 - just rte

*/

/*

The Legend of Silk Road
Unico, 1999

PCB No.: SR2001A
CPU    : MC68EC020FG16 (68020, 100 pin PQFP)
SND    : YM2151, YM3012, OKI M6295 (x2) (Note: No sound CPU)
OSC    : 32.000MHz, 3.579545MHz
RAM    : 62256 (x8), 6116 (x2), KM681000BLG-7L (x4, SOP32, Surface-mounted)
Other  : 2x Actel A40MX04 (84 pin PLCC, same video chips as Multi Champ...ESD16.c)
         15x PALs

DIPs   : 8 position (x2)

Typed from sheet supplied with PCB (* = Default)

DIP SWA
              1   2   3   4   5   6   7   8
--------------------------------------------------------------------------------------
Lives     1   OFF
          2*  ON

Special  OFF      OFF
Effect   ON*      ON

Not used              OFF OFF OFF

Difficulty 1                      OFF OFF ON
           2                      ON  OFF ON
           3                      OFF ON  ON
           4*                     ON  ON  ON
           5                      OFF OFF OFF
           6                      ON  OFF OFF
           7                      OFF ON  OFF
           8                      ON  ON  OFF
--------------------------------------------------------------------------------------


DIP SWB
            1   2   3   4   5   6   7   8
--------------------------------------------------------------------------------------
Not Used    OFF

Freeplay  No*   OFF
         Yes    ON

Not Used            OFF

Demo Sound  No          OFF
           Yes*         ON

Chute Type  Single*         OFF
            Multi           ON

Coin/Credit 1 Coin 1 Credit*    OFF OFF OFF
            1 Coin 2 Credit     ON  OFF OFF
            1 Coin 3 Credit     OFF ON  OFF
            1 Coin 4 Credit     ON  ON  OFF
            2 Coin 1 Credit     OFF OFF ON
            3 Coin 1 Credit     ON  OFF ON
            4 Coin 1 Credit     OFF ON  ON
            5 Coin 1 Credit     ON  ON  ON
--------------------------------------------------------------------------------------

ROMs:
ROM00.BIN   32 pin 4M Mask, read as 27C4001         OKI Samples
ROM01.BIN   MX27C2000                   Sound Program

ROM02.BIN   42 pin 8M Mask, read as uPD27C8000        \
ROM03.BIN   42 pin 8M Mask, read as uPD27C8000        / Main Program

ROM04.BIN       MX29F1610, SOP44 Surface Mounted Mask ROM \
ROM05.BIN       MX29F1610, SOP44 Surface Mounted Mask ROM  |
ROM06.BIN       MX29F1610, SOP44 Surface Mounted Mask ROM  |
ROM07.BIN       MX29F1610, SOP44 Surface Mounted Mask ROM  |
ROM08.BIN       MX29F1610, SOP44 Surface Mounted Mask ROM  |
ROM09.BIN       MX29F1610, SOP44 Surface Mounted Mask ROM  |
ROM10.BIN       MX29F1610, SOP44 Surface Mounted Mask ROM  |    GFX
ROM11.BIN       MX29F1610, SOP44 Surface Mounted Mask ROM  |
ROM12.BIN       MX29F1610, SOP44 Surface Mounted Mask ROM  |
ROM13.BIN       MX29F1610, SOP44 Surface Mounted Mask ROM  |
ROM14.BIN       MX29F1610, SOP44 Surface Mounted Mask ROM  |
ROM15.BIN       MX29F1610, SOP44 Surface Mounted Mask ROM /

*/

/* Stephh's notes :

     - Bit 7 of "system inputs" seems to freeze the inputs (and skip parts of code)
       when it is active :
         * at start, all inputs are read
         * when pressed for the 1st time, the inputs are NOT read and some code is skipped
         * when pressed for the 2nd time, the inputs are read and NO code is skipped
       Here are the routines where this bit is involved :
         * "initialization" at 0x000320
         * test at 0x00014e
     - Bit 7 of "misc inputs" freezes the game (code at 0x001570). VBLANK ?

     - 0xc00025 is read and written (code at 0x001724) but its effect is unknown
     - 0xc00031 is read and written (code at 0x001a58) but its effect is unknown

*/

WRITE32_MEMBER(silkroad_state::paletteram32_xRRRRRGGGGGBBBBB_dword_w)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	m_palette->set_pen_color(offset,pal5bit(m_generic_paletteram_32[offset] >> (10+16)),pal5bit(m_generic_paletteram_32[offset] >> (5+16)),pal5bit(m_generic_paletteram_32[offset] >> (0+16)));
}

WRITE32_MEMBER(silkroad_state::silk_6295_bank_w)
{
	if (ACCESSING_BITS_24_31)
	{
		int bank = (data & 0x3000000) >> 24;
		if(bank < 3)
			m_oki1->set_bank_base(0x40000 * (bank));
	}
}

WRITE32_MEMBER(silkroad_state::silk_coin_counter_w)
{
	if (ACCESSING_BITS_16_23)
	{
		coin_counter_w(machine(), 0, data & 0x10000);
		coin_counter_w(machine(), 1, data & 0x80000);
	}
}

static ADDRESS_MAP_START( cpu_map, AS_PROGRAM, 32, silkroad_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x40c000, 0x40cfff) AM_RAM AM_SHARE("sprram") // sprites
	AM_RANGE(0x600000, 0x603fff) AM_RAM_WRITE(paletteram32_xRRRRRGGGGGBBBBB_dword_w) AM_SHARE("paletteram") // palette
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(silkroad_fgram_w) AM_SHARE("vidram")  // lower Layer
	AM_RANGE(0x804000, 0x807fff) AM_RAM_WRITE(silkroad_fgram2_w) AM_SHARE("vidram2")  // mid layer
	AM_RANGE(0x808000, 0x80bfff) AM_RAM_WRITE(silkroad_fgram3_w) AM_SHARE("vidram3") // higher layer
	AM_RANGE(0xc00000, 0xc00003) AM_READ_PORT("INPUTS")
	AM_RANGE(0xc00004, 0xc00007) AM_READ_PORT("DSW")
	AM_RANGE(0xc00024, 0xc00027) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x00ff0000)
	AM_RANGE(0xc00028, 0xc0002f) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x00ff0000)
	AM_RANGE(0xc00030, 0xc00033) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x00ff0000)
	AM_RANGE(0xc00034, 0xc00037) AM_WRITE(silk_6295_bank_w)
	AM_RANGE(0xc00038, 0xc0003b) AM_WRITE(silk_coin_counter_w)
	AM_RANGE(0xc0010c, 0xc00123) AM_WRITEONLY AM_SHARE("regs")
	AM_RANGE(0xfe0000, 0xffffff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( silkroad )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_SERVICE2 ) /* Not mentioned in the "test mode" */
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_SPECIAL )  /* See notes - Stephh*/
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )  // this input makes the 020 lock up...- RB
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("DSW")
	PORT_DIPNAME( 0x00010000, 0x00000000, DEF_STR( Lives ) )    PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(          0x00010000, "1" )
	PORT_DIPSETTING(          0x00000000, "2" )
	PORT_DIPNAME( 0x00020000, 0x00000000, "Special Effect" )    PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(          0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x00040000, IP_ACTIVE_LOW, "SWA:3" )
	PORT_DIPUNUSED_DIPLOC( 0x00080000, IP_ACTIVE_LOW, "SWA:4" )
	PORT_DIPUNUSED_DIPLOC( 0x00100000, IP_ACTIVE_LOW, "SWA:5" )
	PORT_DIPNAME( 0x00e00000, 0x00000000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWA:6,7,8")
	PORT_DIPSETTING(          0x00600000, DEF_STR( Easiest ) )          // "1"
	PORT_DIPSETTING(          0x00400000, DEF_STR( Easier ) )           // "2"
	PORT_DIPSETTING(          0x00200000, DEF_STR( Easy ) )             // "3"
	PORT_DIPSETTING(          0x00000000, DEF_STR( Normal ) )           // "4"
	PORT_DIPSETTING(          0x00e00000, DEF_STR( Medium ) )           // "5"
	PORT_DIPSETTING(          0x00c00000, DEF_STR( Hard ) )             // "6"
	PORT_DIPSETTING(          0x00a00000, DEF_STR( Harder ) )           // "7"
	PORT_DIPSETTING(          0x00800000, DEF_STR( Hardest ) )          // "8"
	PORT_DIPUNUSED_DIPLOC( 0x01000000, IP_ACTIVE_LOW, "SWB:1" )
	PORT_DIPNAME( 0x02000000, 0x02000000, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(          0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x04000000, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPNAME( 0x08000000, 0x00000000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(          0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x10000000, "Chute Type" )        PORT_DIPLOCATION("SWB:5")   // "Coin Box"
	PORT_DIPSETTING(          0x10000000, DEF_STR( Single ) )                   // "1"
	PORT_DIPSETTING(          0x00000000, "Multi" )                         // "2"
	PORT_DIPNAME( 0xe0000000, 0xe0000000, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SWB:6,7,8")
	PORT_DIPSETTING(          0x00000000, DEF_STR(5C_1C))
	PORT_DIPSETTING(          0x20000000, DEF_STR(4C_1C))
	PORT_DIPSETTING(          0x40000000, DEF_STR(3C_1C))
	PORT_DIPSETTING(          0x60000000, DEF_STR(2C_1C))
	PORT_DIPSETTING(          0xe0000000, DEF_STR(1C_1C))
	PORT_DIPSETTING(          0xc0000000, DEF_STR(1C_2C))
	PORT_DIPSETTING(          0xa0000000, DEF_STR(1C_3C))
	PORT_DIPSETTING(          0x80000000, DEF_STR(1C_4C))

//  PORT_START("MISC")  /* Misc inputs */
//  PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* VBLANK ? */
//  PORT_BIT( 0xff7f, IP_ACTIVE_LOW, IPT_UNUSED ) /* unknown / unused */
INPUT_PORTS_END


/* BACKGROUNDS */
static const gfx_layout tiles16x16x6_layout =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{ 0x0000000*8+8,0x0000000*8+0,  0x0800000*8+8, 0x0800000*8+0, 0x1000000*8+8,0x1000000*8+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	16*32
};

static GFXDECODE_START( silkroad )
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16x6_layout,  0x0000, 256 )
GFXDECODE_END

static MACHINE_CONFIG_START( silkroad, silkroad_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, XTAL_32MHz/2) /* 16MHz */
	MCFG_CPU_PROGRAM_MAP(cpu_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", silkroad_state,  irq4_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(6*8+2, 64*8-1-(10*8)-2, 2*8, 32*8-1-(2*8))
	MCFG_SCREEN_UPDATE_DRIVER(silkroad_state, screen_update_silkroad)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", silkroad)
	MCFG_PALETTE_ADD("palette", 0x2000)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_OKIM6295_ADD("oki1", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.45)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.45)

	MCFG_OKIM6295_ADD("oki2", 2112000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.45)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.45)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( silkroad )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "rom02.bin", 0x000000, 0x100000, CRC(4e5200fc) SHA1(4d4cab03a6ec4ad825001e1e92193940646141e5) )
	ROM_LOAD32_WORD_SWAP( "rom03.bin", 0x000002, 0x100000, CRC(73ccc78c) SHA1(2ac17aa8d7dac8636d29a4e4228a556334b51f1a) )

	ROM_REGION( 0x1800000, "gfx1", ROMREGION_INVERT )
	/* Sprites */
	ROM_LOAD( "rom12.rom12", 0x0000000, 0x0200000, CRC(96393d04) SHA1(f512bb8603510d39e649f4ec1c5e2d0e4bf3a2cc) ) // 0
	ROM_LOAD( "rom08.rom08", 0x0800000, 0x0200000, CRC(23f1d462) SHA1(6ca8052b16ccc1fe59716e03f66bd33af5145b37) ) // 0
	ROM_LOAD( "rom04.rom04", 0x1000000, 0x0200000, CRC(d9f0bbd7) SHA1(32c055ad5497c0bec5db40b528e589d7724e354f) ) // 0

	ROM_LOAD( "rom13.rom13", 0x0200000, 0x0200000, CRC(4ca1698e) SHA1(4fffc2f2a5fb434c42463ce904fd811866c53f81) ) // 1
	ROM_LOAD( "rom09.rom09", 0x0a00000, 0x0200000, CRC(ef0b5bf4) SHA1(acd3bc5070de84608c5da0d091094382853cb048) ) // 1
	ROM_LOAD( "rom05.rom05", 0x1200000, 0x0200000, CRC(512d6e25) SHA1(fc0a56663d77bbdfbd4242e14a55563073634582) ) // 1

	ROM_LOAD( "rom14.rom14", 0x0400000, 0x0200000, CRC(d00b19c4) SHA1(d5b955dca5d0d251166a7f35a0bbbda6a91ecbd0) ) // 2
	ROM_LOAD( "rom10.rom10", 0x0c00000, 0x0200000, CRC(7d324280) SHA1(cdf6d9342292f693cc5ec1b72816f2788963fcec) ) // 2
	ROM_LOAD( "rom06.rom06", 0x1400000, 0x0200000, CRC(3ac26060) SHA1(98ad8efbbf8020daf7469db3e0fda02af6c4c767) ) // 2
	/* Backgrounds */
	ROM_LOAD( "rom07.rom07", 0x0600000, 0x0200000, CRC(9fc6ff9d) SHA1(51c3ca9709a01e0ad6bc76c0d674ed03f9822598) ) // 3
	ROM_LOAD( "rom11.rom11", 0x0e00000, 0x0200000, CRC(11abaf1c) SHA1(19e86f3ebfec518a96c0520f36cfc1b525e7e55c) ) // 3
	ROM_LOAD( "rom15.rom15", 0x1600000, 0x0200000, CRC(26a3b168) SHA1(a4b7955cc4d4fbec7c975a9456f2219ef33f1166) ) // 3

	ROM_REGION( 0x080000, "user1", 0 )
	ROM_LOAD( "rom00.bin", 0x000000, 0x080000, CRC(b10ba7ab) SHA1(a6a3ae71b803af9c31d7e97dc86cfcc123ee9a40) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0xc0000, "oki1", 0 ) /* Samples */
	ROM_COPY( "user1", nullptr, 0x000000, 0x020000)
	ROM_COPY( "user1", 0x020000, 0x020000, 0x020000)
	ROM_COPY( "user1", nullptr, 0x040000, 0x020000)
	ROM_COPY( "user1", 0x040000, 0x060000, 0x020000)
	ROM_COPY( "user1", nullptr, 0x080000, 0x020000)
	ROM_COPY( "user1", 0x060000, 0x0a0000, 0x020000)

	ROM_REGION( 0x080000, "oki2", 0 )
	ROM_LOAD( "rom01.bin", 0x000000, 0x040000, CRC(db8cb455) SHA1(6723b4018208d554bd1bf1e0640b72d2f4f47302) )
ROM_END

ROM_START( silkroada )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "rom02.bin", 0x000000, 0x100000, CRC(4e5200fc) SHA1(4d4cab03a6ec4ad825001e1e92193940646141e5) )
	ROM_LOAD32_WORD_SWAP( "rom03.bin", 0x000002, 0x100000, CRC(73ccc78c) SHA1(2ac17aa8d7dac8636d29a4e4228a556334b51f1a) )

	ROM_REGION( 0x1800000, "gfx1", ROMREGION_INVERT )
	/* Sprites */ // this board has these 3 ROMs instead of the 6 on the set above
	ROM_LOAD( "unico_sr13.rom13", 0x0000000, 0x0400000, CRC(d001c3df) SHA1(ef1b1510f33401b0983093e2d8db48d3886c4fe1) ) // 0 + 1
	ROM_LOAD( "unico_sr09.rom09", 0x0800000, 0x0400000, CRC(696d908d) SHA1(abe3ec8a53875a136f78bbed723bb89d04196427) ) // 0 + 1
	ROM_LOAD( "unico_sr05.rom05", 0x1000000, 0x0400000, CRC(00f638c1) SHA1(cc6da13f8e82b08f8098c7636fbd1d40ee5ab132) ) // 0 + 1

	ROM_LOAD( "rom14.rom14",      0x0400000, 0x0200000, CRC(d00b19c4) SHA1(d5b955dca5d0d251166a7f35a0bbbda6a91ecbd0) ) // 2
	ROM_LOAD( "rom10.rom10",      0x0c00000, 0x0200000, CRC(7d324280) SHA1(cdf6d9342292f693cc5ec1b72816f2788963fcec) ) // 2
	ROM_LOAD( "rom06.rom06",      0x1400000, 0x0200000, CRC(3ac26060) SHA1(98ad8efbbf8020daf7469db3e0fda02af6c4c767) ) // 2
	/* Backgrounds */
	ROM_LOAD( "rom07.rom07",      0x0600000, 0x0200000, CRC(9fc6ff9d) SHA1(51c3ca9709a01e0ad6bc76c0d674ed03f9822598) ) // 3
	ROM_LOAD( "rom11.rom11",      0x0e00000, 0x0200000, CRC(11abaf1c) SHA1(19e86f3ebfec518a96c0520f36cfc1b525e7e55c) ) // 3
	ROM_LOAD( "rom15.rom15",      0x1600000, 0x0200000, CRC(26a3b168) SHA1(a4b7955cc4d4fbec7c975a9456f2219ef33f1166) ) // 3

	ROM_REGION( 0x080000, "user1", 0 )
	ROM_LOAD( "rom00.bin", 0x000000, 0x080000, CRC(b10ba7ab) SHA1(a6a3ae71b803af9c31d7e97dc86cfcc123ee9a40) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0xc0000, "oki1", 0 ) /* Samples */
	ROM_COPY( "user1", nullptr, 0x000000, 0x020000)
	ROM_COPY( "user1", 0x020000, 0x020000, 0x020000)
	ROM_COPY( "user1", nullptr, 0x040000, 0x020000)
	ROM_COPY( "user1", 0x040000, 0x060000, 0x020000)
	ROM_COPY( "user1", nullptr, 0x080000, 0x020000)
	ROM_COPY( "user1", 0x060000, 0x0a0000, 0x020000)

	ROM_REGION( 0x080000, "oki2", 0 )
	ROM_LOAD( "rom01.bin", 0x000000, 0x040000, CRC(db8cb455) SHA1(6723b4018208d554bd1bf1e0640b72d2f4f47302) )
ROM_END


GAME( 1999, silkroad,         0, silkroad, silkroad, driver_device, 0, ROT0, "Unico", "The Legend of Silkroad", MACHINE_SUPPORTS_SAVE )
GAME( 1999, silkroada, silkroad, silkroad, silkroad, driver_device, 0, ROT0, "Unico", "The Legend of Silkroad (larger ROMs)", MACHINE_SUPPORTS_SAVE ) // same content but fewer GFX roms of a larger size
