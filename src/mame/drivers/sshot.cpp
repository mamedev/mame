// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*

* Model Racing's Super Shot
* Model Racing's Gun Champ on Super Shot hardware

  Driver by Mariusz Wojcieszek

  Todo:
  - Discrete sound

SUPER SHOT
Mainboard:
 __________________________________________________________________________________________________________________________
|      1      2      3      4      5      6      7      8      9      10      11      12     13     14     15              |
|      _      _      _      _      _     ____     ____     ____     ____     ____     ____     ____     ____               |
|     | |    | |    | |    | |    | |   |    |   |    |   |    |   |    |   |    |   |    |   |    |   |    |              |
|A    |R|    | |    | |    |C|    |C|   |    |   |    |   |    |   |    |   |    |   |    |   |    |   |    |              |
|     | |    |B|    |B|    | |    | |   |    |   |    |   |    |   |    |   |    |   |    |   |    |   |    |              |
|     |_|    | |    | |    |_|    |_|   |TMS |   |TMS |   |TMS |   |TMS |   |TMS |   |TMS |   |TMS |   |TMS |              |
|            |_|    |_|                 |2708|   |2708|   |2708|   |2708|   |2708|   |2708|   |2708|   |2708|              |
|                                       |____|   |____|   |____|   |____|   |____|   |____|   |____|   |____|              |
|                                                                                                                          |
|                                                                                                                          |
|      _        _____                                                                                                     _|
|     | |      |/853 |                                                                                                   |_
|     |S|      |     |      _      _                                                                                      _|
|     | |      |INS80|     | |    | |     _      _      _      ____     ____     _      _      _      _      _            -|
|     |_|      |60N  |     | |    | |    | |    | |    | |    |SS  |   |SS  |   | |    | |    | |    | |    | |           -|
|B             |     |     |B|    |B|    |D|    | |    | |    |  A |   |  B |   |E|    | |    | |    |A|    |A|           -|
|              |ISP-8|     | |    | |    | |    |B|    |B|    |    |   |    |   | |    |B|    |B|    | |    | |           -|
|              |A/600|     |_|    |_|    |_|    | |    | |    |TMS |   |TMS |   |_|    | |    | |    |_|    |_|           -|
|              |N    |                          |_|    |_|    |2708|   |2708|          |_|    |_|                         -|
|              |     |                                        |____|   |____|                                             -|
|              |     |                                                                                                    -|
|              |_____|                                                                                                    -|
|                                                                                                                         -|
|  __________________                                                                                                     -|
| |                  |                                                               _      _      _      _               -|
| |                  |      _             _      _      _      _      _       _     | |    | |    | |    | |              -|
| |    EPOXY BLOCK   |     | |           | |    | |    | |    | |    | |     | |    | |    | |    | |    | |              -|
| |                  |     |Q|           |J|    |H|    |K|    |I|    |H|     |H|    |F|    |F|    |F|    |F|              -|
|C|                  |     | |           | |    | |    | |    | |    | |     | |    | |    | |    | |    | |              -|
| |__________________|     |_|           |_|    |_|    |_|    |_|    |_|     |_|    |_|    |_|    |_|    |_|             |_
|                                                                                                                          |
|                                                                                                                          |
|                                                                                                                          |
|                                                                                    _      _                              |
|      _      _      _      _      _      _                           _      __     | |    | |     _      _                |
|     | |    | |    | |    | |    | |    | |    _              _     | |    |  |    | |    | |    | |    | |               |
|     |M|    |H|    |N|    |O|    |P|    |G|   |L|            |L|    |D|    |SW|    |B|    |B|    |S|    |S|               |
|D    | |    | |    | |    | |    | |    | |   |_|            |_|    | |    | 1|    | |    | |    | |    | |               |
|     |_|    |_|    |_|    |_|    |_|    |_|                         |_|    |__|    |_|    |_|    |_|    |_|               |
|                                                                                                                          |
|                                                                                                                          |
|                                                                                                                          |
|E            CS 233                                                                                                       |
|                                                                                                                          |
|                                                                                                                          |
|__________________________________________________________________________________________________________________________|


A = MM2114N-3                 L = NE555P
B = DM81LS97N                 M = DM7404N
C = 7921 2111A-2              N = DM7400N
D = DM74LS138N / NS82LS05N    O = DM74LS32N
E = DM74166N                  P = DM80LS97N / DM74LS367N
F = SN74LS374N                Q = DM74LS02N
G = DM74LS20N                 R = DM74LS175N
H = SN74LS161AN               S = DM74LS14N
I = DM74LS04N
J = F7474PC
K = 74LS112PC



EPOXY-BLOCK:
Component-Side:             Solder Side:
 _____________________        _____________________
|  _   _   _       _  |      |            D3918    |
| | | | | | |     | | |      |            5686     |
| | | | | | |     | | |      |                     |
| | | | | | |     | | |      |                     |
| |_| |_| |_|     |_| |      |                     |
|_____________________|      |_____________________|


SW1:
 _________________
| DST08        ON |
|-----------------|
|| |#|#|#|#|#| | ||
|-----------------|
||#| | | | | |#|#||
|-----------------|
| 1 2 3 4 5 6 7 8 |
|_________________|


Soundboard:
 ____________________________________________________________
|                                                            |
|       7                6     5     4     3     2     1     |
|                        _     _     _     _           _     |     A = SN7442AN
|                       | |   | |   | |   | |         | |    |     B = F-7404PC
|                       |F|   |P|   |A|   |C|         |Q|  A |     C = SN7405N
|                       | |   | |   | |   | |         | |    |     D = F 74132PC
|                       |_|   |_|   |_|   |_|         |_|    |     E = SN74393N
|                                                            |     F = LM3900N
|                        _     _     _     _     _           |     G = LM324N
|                       | |   | |   | |   | |   | |          |     H = SN7432N
|                       |G|   |E|   |B|   |D|   |O|        B |     I = NE 556N
|                       | |   | |   | |   | |   | |          |     J = SN7474N
|                       |_|   |_|   |_|   |_|   |_|          |     K = SN7400N
|                                                            |     L = UA339PC
|                              _     _     _     _           |     M = SN7410N
|                             | |   | |   | |   | |          |     N = CA810Q
|                             |J|   |I|   |H|   |E|        C |     O = 6331
|                             | |   | |   | |   | |          |     P = 6301
|                             |_|   |_|   |_|   |_|          |     Q = S50240
|                                                            |
|                              _     _     _     _           |
|   ______                    | |   | |   | |   | |          |
|  |  N   |                   |C|   |B|   |K|   |I|        D |
|  |______|                   | |   | |   | |   | |          |
|                             |_|   |_|   |_|   |_|          |
|                                                            |
|                              _     _     _                 |
|                             | |   | |   | |                |
|                             |L|   |M|   |K|              E |
|                             | |   | |   | |                |
|MR model                     |_|   |_|   |_|                |
|   racing CS229                                             |
|               __                           __              |
|______________|  |_|_|_|_|_|_|_|_|_|_|_|_|_|  |_____________|
                  1                         22


 __________________I N S T R U C T I O N S _________________
|                                                           |
| INSERT COIN(S) AND PRESS START BUTTON                     |
|                                                           |
| 1) THREE SPIRALLING BALLS SCORE FROM ... 100 TO 10 POINTS |
| 2) THE SEQUENCE OF GLASSES ................... 420 POINTS |
| 3) THE SEQUENCE OF LAMPS ..................... 440 POINTS |
|    THE LAMPS AT THE CENTRE SCORE HIGHEST                  |
| 4) THE THREE FAST SPIRALLING BALLS SCORE ...... 80 POINTS |
|    HITTING THE FIRST BALL SCORES 500 POINTS AND           |
|    GIVES THE GLASSES AND BOTTLE SEQUENCE.                 |
|    HITTING THE SECOND BALL SCORES 240 POINTS AND          |
|    GIVES THE GLASS SEQUENCE AT INCREASED SPEED.           |
|___________________________________________________________|


GUN CHAMP
Same pcb as Super Shot, but with gun hardware as 8080bw Gun Champ, no xy pots,
 seems it uses $4202 and $4206 to read gun pos
Mainboard:  CS249
Soundboard: CS240
Given CS numbers this is released after the other GunChamp

*/

#include "emu.h"
#include "cpu/scmp/scmp.h"

#include "gunchamps.lh"


class supershot_state : public driver_device
{
public:
	supershot_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT8> m_videoram;
	tilemap_t   *m_tilemap;
	DECLARE_WRITE8_MEMBER(supershot_vidram_w);
	DECLARE_WRITE8_MEMBER(supershot_output0_w);
	DECLARE_WRITE8_MEMBER(supershot_output1_w);
	TILE_GET_INFO_MEMBER(get_supershot_text_tile_info);
	virtual void video_start() override;
	UINT32 screen_update_supershot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};


/*************************************
 *
 *  Video
 *
 *************************************/

TILE_GET_INFO_MEMBER(supershot_state::get_supershot_text_tile_info)
{
	UINT8 code = m_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

void supershot_state::video_start()
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supershot_state::get_supershot_text_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

UINT32 supershot_state::screen_update_supershot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

WRITE8_MEMBER(supershot_state::supershot_vidram_w)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}


/*************************************
 *
 *  Outputs
 *
 *************************************/

WRITE8_MEMBER(supershot_state::supershot_output0_w)
{
	/*
	    bit     signal      description

	    0       12          EXPLOSIONE LAMP. DX
	    1       13          RIMBALZO PALLINA (BALL BOUNCE)
	    2       14          CONSENSO GIOCO (CONSENT GAME)
	    3       16          VINCITA EXT. PLAY (WIN EXT. PLAY)
	    4       n.c.
	    5       n.c.
	    6       n.c.
	    7       H           n.c.
	*/
}

WRITE8_MEMBER(supershot_state::supershot_output1_w)
{
	/*
	    bit     signal      description

	    0       5           ESPL. BICCHIERI E BOTTIGLIE (EXPLOSION GLASSES AND BOTTLES)
	    1       F           n.c.
	    2       E           APPARIZIONE BOTTIGLIE (APPEARANCE BOTTLES)
	    3       4           MUSICA FINE GIOCO (END OF GAME MUSIC)
	    4       S           EXPANSIONE CERCHIO (EXPANSION CIRCLE)
	    5       n.c.
	    6       N           EXPLOSIONE CERCHIO (CIRCLE EXPLOSION)
	    7       11          SIBILO SPIRALE (PATHY SPIRAL)
	*/
}


/*************************************
 *
 *  Memory map
 *
 *************************************/

static ADDRESS_MAP_START( supershot_map, AS_PROGRAM, 8, supershot_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM_WRITE(supershot_vidram_w ) AM_SHARE("videoram")
	AM_RANGE(0x4100, 0x41ff) AM_RAM
	AM_RANGE(0x4200, 0x4200) AM_READ_PORT("GUNX")
	AM_RANGE(0x4201, 0x4201) AM_READ_PORT("GUNY")
	AM_RANGE(0x4202, 0x4202) AM_READ_PORT("IN0")
	AM_RANGE(0x4203, 0x4203) AM_READ_PORT("DSW")
	AM_RANGE(0x4206, 0x4206) AM_WRITE(supershot_output0_w)
	AM_RANGE(0x4207, 0x4207) AM_WRITE(supershot_output1_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Inputs
 *
 *************************************/

static INPUT_PORTS_START( supershot )
	PORT_START("GUNX")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNY")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("IN0")
	PORT_BIT( 0xf4, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START )

	PORT_START("DSW")
	PORT_DIPUNUSED( 0x01, 0x00 )
	PORT_DIPUNUSED( 0x04, 0x00 )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x40, 0x00 )
	PORT_DIPNAME( 0x82, 0x82, "Bonus" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x82, "3" )
INPUT_PORTS_END


/*************************************
 *
 *  Machine
 *
 *************************************/

static const gfx_layout supershot_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( supershot )
	GFXDECODE_ENTRY( "gfx", 0, supershot_charlayout,   0, 1  )
GFXDECODE_END

static MACHINE_CONFIG_START( supershot, supershot_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SCMP, XTAL_11_289MHz/4)
	MCFG_CPU_PROGRAM_MAP(supershot_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE((32)*8, (32)*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(supershot_state, screen_update_supershot)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", supershot)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* sound hardware */
	//...
MACHINE_CONFIG_END


ROM_START( sshot )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "ss_1m.a6",  0x0000, 0x0400, CRC(9f1625db) SHA1(734156d9858d696ac1f00706ba7716e5f818a3c5) )
	ROM_LOAD( "ss_2m.a7",  0x0400, 0x0400, CRC(09f0295d) SHA1(7b4f996e9200fd178e295b1613aad8d10b19bc78) )
	ROM_LOAD( "ss_3m.a9",  0x0800, 0x0400, CRC(88b54d54) SHA1(4f6e792276f2615d4df1867fcdb0d18f55f72b3a) )
	ROM_LOAD( "ss_4m.a10", 0x0c00, 0x0400, CRC(14199f00) SHA1(2696a42f362b39b92c8e07e2a005e0d8e00f539d) )
	ROM_LOAD( "ss_5m.a11", 0x1000, 0x0400, CRC(e70eca76) SHA1(771ae15fd4b2663090386a4ec49575895c5bfed7) )
	ROM_LOAD( "ss_6m.a12", 0x1400, 0x0400, CRC(27683d93) SHA1(5e127186a215f4bdf1912ff9d85574165c2dcc93) )
	ROM_LOAD( "ss_7m.a13", 0x1800, 0x0400, CRC(650dada4) SHA1(8a4274792952b33b87ef7d160e1a1e038041761b) )
	ROM_LOAD( "ss_8m.a15", 0x1c00, 0x0400, CRC(32bc8424) SHA1(a7b75b2c0fe6ff80148e2ba62ac2ff59ed03e09a) )

	ROM_REGION( 0x0800, "gfx", 0 )
	ROM_LOAD( "ss_a.b9",   0x0000, 0x0400, CRC(ad3413e0) SHA1(ea4c2728755fe52a00fdceddca0b641965045005) )
	ROM_LOAD( "ss_b.b10",  0x0400, 0x0400, CRC(ba70e619) SHA1(df39512de881df26ccc7fa74f6bae82d92cd9008) )
ROM_END

ROM_START( gunchamps )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "gc-1.a6",   0x0000, 0x0400, CRC(dcafc54b) SHA1(a83adbee5fc6125f90078e233af258120ae14a4d) )
	ROM_LOAD( "gc-2.a7",   0x0400, 0x0400, CRC(8b087128) SHA1(c49934dc29d24d94dda0a2b9d425abf1580a5038) )
	ROM_LOAD( "gc-3.a9",   0x0800, 0x0400, CRC(ca517d50) SHA1(ccb18b66070d02082a367ca78f9095395e997bdd) )
	ROM_LOAD( "gc-4.a10",  0x0c00, 0x0400, CRC(6a5b258c) SHA1(6a8349f4d785517877531100b3c30e02a54b98e2) )
	ROM_LOAD( "gc-5.a11",  0x1000, 0x0400, CRC(3f25c50d) SHA1(718687f421bf3ac2471b9cae7ff4514344912ef5) )
	ROM_LOAD( "gc-6.a12",  0x1400, 0x0400, CRC(85a62b89) SHA1(0a5dc97820f49a9100c99c129b4eebc649391a07) )
	ROM_LOAD( "gc-7.a13",  0x1800, 0x0400, CRC(0a6fde47) SHA1(cc596dd8c85701e1df0f513527125b006a7e1bd7) )

	ROM_REGION( 0x0800, "gfx", 0 )
	ROM_LOAD( "gc-a.b9",   0x0000, 0x0400, CRC(c07f290e) SHA1(760ce12f4f5cadbd846d361c615f5026356a6fe2) )
	ROM_LOAD( "gc-b.b10",  0x0400, 0x0400, CRC(10ce709b) SHA1(e6f194aa26cd0e01ba0de3909948cc8595031d4d) )

	ROM_REGION( 0x0120, "proms", 0 ) // proms on the sound board
	ROM_LOAD( "snd82s129.a5",  0x0000, 0x0100, CRC(1d74dc30) SHA1(b956d8c6564cc3cc1b5f5f55b05ad4aa13f247e6) )
	ROM_LOAD( "snd82s23.b7",   0x0100, 0x0020, CRC(f4fa91d4) SHA1(0e0903532c8609c2d42491c2013647a42d13749a) )
ROM_END


GAME( 1979, sshot,     0,        supershot, supershot, driver_device, 0, ROT0, "Model Racing", "Super Shot", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )
GAMEL(1980, gunchamps, gunchamp, supershot, supershot, driver_device, 0, ROT0, "Model Racing", "Gun Champ (newer, Super Shot hardware)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_NOT_WORKING, layout_gunchamps )
