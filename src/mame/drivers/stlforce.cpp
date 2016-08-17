// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Steel Force

ELECTRONIC DEVICES 1994 Milano Italy
ECOGAMES S.L. Barcelona, Spain

driver by David Haywood
inputs etc. by stephh

----------------------------------------

68000P12 processor
15mHZ cyrstal next to it

2 of these:

TPC 1020AFN-084c

32MHz crystal colse to this.

1 GAL
5 PROMS  (16S25H)

27c4001
u1, u27, u28, u29, u30

27c2001
u31,u32, u33, u34

27c010
u104, u105

----------------------------------------

notes:

lev 1 : 0x64 : 0000 0100 - just rte
lev 2 : 0x68 : 0000 0100 - just rte
lev 3 : 0x6c : 0000 0100 - just rte
lev 4 : 0x70 : 0000 CBD6 - vblank
lev 5 : 0x74 : 0000 0100 - just rte
lev 6 : 0x78 : 0000 0100 - just rte
lev 7 : 0x7c : 0000 0100 - just rte


  2002.02.03 : There doesn't seem to be Dip Switches
               (you make the changes in the "test mode")
               Bits 8 to 15 of IN1 seem to be unused
               The 2nd part of the "test mode" ("sound and video") is in Spanish/Italian
               (I can't tell for the moment)
               Release date and manufacturers according to the title screen

 2004.xx.10 - Pierpaolo Prazzoli
 - fixed bit 4 of IN1. it is vblank and it fixed scroll issue in attract mode
 - fixed sprite glitches with visible flag
 - added rows scroll
 - added eeprom
 - fixed sound banking

TO DO :
  - unknown registers
  - clipping issues?
  - priority issues?
  - same sprites buffer used in Mighty Warriors
  - clocks don't match on the games?

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "includes/stlforce.h"


WRITE16_MEMBER(stlforce_state::eeprom_w)
{
	if( ACCESSING_BITS_0_7 )
	{
		m_eeprom->di_write(data & 0x01);
		m_eeprom->cs_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE );
		m_eeprom->clk_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE );
	}
}

WRITE16_MEMBER(stlforce_state::oki_bank_w)
{
	m_oki->set_bank_base(0x40000 * ((data>>8) & 3));
}

static ADDRESS_MAP_START( stlforce_map, AS_PROGRAM, 16, stlforce_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x1007ff) AM_RAM_WRITE(stlforce_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x100800, 0x100fff) AM_RAM_WRITE(stlforce_mlow_videoram_w) AM_SHARE("mlow_videoram")
	AM_RANGE(0x101000, 0x1017ff) AM_RAM_WRITE(stlforce_mhigh_videoram_w) AM_SHARE("mhigh_videoram")
	AM_RANGE(0x101800, 0x1027ff) AM_RAM_WRITE(stlforce_tx_videoram_w) AM_SHARE("tx_videoram")
	AM_RANGE(0x102800, 0x102fff) AM_RAM /* unknown / ram */
	AM_RANGE(0x103000, 0x1033ff) AM_RAM AM_SHARE("bg_scrollram")
	AM_RANGE(0x103400, 0x1037ff) AM_RAM AM_SHARE("mlow_scrollram")
	AM_RANGE(0x103800, 0x103bff) AM_RAM AM_SHARE("mhigh_scrollram")
	AM_RANGE(0x103c00, 0x103fff) AM_RAM AM_SHARE("vidattrram")
	AM_RANGE(0x104000, 0x104fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x105000, 0x107fff) AM_RAM /* unknown / ram */
	AM_RANGE(0x108000, 0x108fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x109000, 0x11ffff) AM_RAM
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("INPUT")
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x400010, 0x400011) AM_WRITE(eeprom_w)
	AM_RANGE(0x400012, 0x400013) AM_WRITE(oki_bank_w)
	AM_RANGE(0x40001e, 0x40001f) AM_WRITENOP // sprites buffer commands
	AM_RANGE(0x410000, 0x410001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
ADDRESS_MAP_END

static INPUT_PORTS_START( stlforce )
	PORT_START("INPUT")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read) /* eeprom */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout stlforce_bglayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{0,1,2,3},
	{12,8,4,0,28,24,20,16,16*32+12,16*32+8,16*32+4,16*32+0,16*32+28,16*32+24,16*32+20,16*32+16},
	{0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32},
	32*32
};

static const gfx_layout stlforce_txlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{0,1,2,3},
	{12,8,4,0,28,24,20,16},
	{0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32 },
	8*32
};

static const gfx_layout stlforce_splayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{0x040000*3*8,0x040000*2*8,0x040000*1*8,0x040000*0*8},
	{16*8+7,16*8+6,16*8+5,16*8+4,16*8+3,16*8+2,16*8+1,16*8+0,7,6,5,4,3,2,1,0},
	{0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	32*8
};

static GFXDECODE_START( stlforce )
	GFXDECODE_ENTRY( "gfx1", 0, stlforce_bglayout, 0, 256  )
	GFXDECODE_ENTRY( "gfx1", 0, stlforce_txlayout, 0, 256  )
	GFXDECODE_ENTRY( "gfx2", 0, stlforce_splayout, 0, 256  )
GFXDECODE_END


static MACHINE_CONFIG_START( stlforce, stlforce_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 15000000)
	MCFG_CPU_PROGRAM_MAP(stlforce_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", stlforce_state,  irq4_line_hold)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 47*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(stlforce_state, screen_update_stlforce)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", stlforce)
	MCFG_PALETTE_ADD("palette", 0x800)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 937500 , OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( twinbrat, stlforce )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(14745600)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(3*8, 45*8-1, 0*8, 30*8-1)
MACHINE_CONFIG_END

ROM_START( stlforce )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "stlforce.105", 0x00000, 0x20000, CRC(3ec804ca) SHA1(4efcf3321b7111644ac3ee0a83ad95d0571a4021) )
	ROM_LOAD16_BYTE( "stlforce.104", 0x00001, 0x20000, CRC(69b5f429) SHA1(5bd20fad91a22f4d62f85a5190d72dd824ee26a5) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* 16x16 bg tiles & 8x8 tx tiles merged */
	ROM_LOAD16_BYTE( "stlforce.u27", 0x000001, 0x080000, CRC(c42ef365) SHA1(40e9ee29ea14b3bc2fbfa4e6acb7d680cf72f01a) )
	ROM_LOAD16_BYTE( "stlforce.u28", 0x000000, 0x080000, CRC(6a4b7c98) SHA1(004d7f3c703c6abc79286fa58a4c6793d66fca39) )
	ROM_LOAD16_BYTE( "stlforce.u29", 0x100001, 0x080000, CRC(30488f44) SHA1(af0d92d8952ce3cd893ab9569afdda12e17795e7) )
	ROM_LOAD16_BYTE( "stlforce.u30", 0x100000, 0x080000, CRC(cf19d43a) SHA1(dc04930548ac5b7e2b74c6041325eac06e773ed5) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* 16x16 sprites */
	ROM_LOAD( "stlforce.u36", 0x00000, 0x40000, CRC(037dfa9f) SHA1(224f5cd1a95d55b065aef5c0bd03b50cabcb619b) )
	ROM_LOAD( "stlforce.u31", 0x40000, 0x40000, CRC(305a8eb5) SHA1(3a8d26f8bc4ec2e8246d1c59115e21cad876630d) )
	ROM_LOAD( "stlforce.u32", 0x80000, 0x40000, CRC(760e8601) SHA1(a61f1d8566e09ce811382c6e23f3881e6c438f15) )
	ROM_LOAD( "stlforce.u33", 0xc0000, 0x40000, CRC(19415cf3) SHA1(31490a1f3321558f82667b63f3963b2ec3fa0c59) )

	/* only one bank */
	ROM_REGION( 0x80000, "oki", 0 ) /* samples */
	ROM_LOAD( "stlforce.u1", 0x00000, 0x80000, CRC(0a55edf1) SHA1(091f12e8110c62df22b370a2e710c930ba06e8ca) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-stlforce.bin", 0x0000, 0x0080, CRC(3fb83951) SHA1(0cbf09751e46f100db847cf0594a4440126a7b6e) )
ROM_END



/*

Twin Brats
Elettronica Video-Games S.R.L, 1995

PCB Layout
----------

|----------------------------------------------|
|  1.BIN                                       |
|      M6295  PAL                  6116   6116 |
|                                  6116   6116 |
|   62256                PAL                   |
|   62256                                      |
|                        6116                  |
|J  6116   |---------|   6116                  |
|A  6116   |ACTEL    |                         |
|M         |A1020A   |                         |
|M         |PL84C    |                         |
|A    PAL  |         |       30MHz       11.BIN|
|          |---------|6264   PAL         10.BIN|
|     62256    62256  6264   |---------| 9.BIN |
|     2.BIN    3.BIN         |ACTEL    | 8.BIN |
|   |-----------------|      |A1020A   | 7.BIN |
|   |   MC68000P12    |      |PL84C    | 6.BIN |
| * |                 | PAL  |         | 5.BIN |
|   |-----------------| PAL  |---------| 4.BIN |
| 93C46  14.7456MHz                            |
|----------------------------------------------|
Notes:
      68000 clock : 14.7456MHz
      M6295 clock : 0.9375MHz (30/32). Sample Rate = 937500 / 132
      VSync       : 58Hz
      *           : Push button test switch

*/


ROM_START( twinbrat )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "2.u105", 0x00000, 0x20000, CRC(33a9bb82) SHA1(0f54239397c93e264b9b211f67bf626acf1246a9) )
	ROM_LOAD16_BYTE( "3.u104", 0x00001, 0x20000, CRC(b1186a67) SHA1(502074063101885874db76ae707db1082313efcf) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "6.bin", 0x000000, 0x80000, CRC(af10ddfd) SHA1(e5e83044f20d6cbbc1b4ef1812ac57b6dc958a8a) )
	ROM_LOAD16_BYTE( "7.bin", 0x000001, 0x80000, CRC(3696345a) SHA1(ea38be3586757527b2a1aad2e22b83937f8602da) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x80000, CRC(1ae8a751) SHA1(5f30306580c6ab4af0ddbdc4519eb4e0ab9bd23a) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x80000, CRC(cf235eeb) SHA1(d067e2dd4f28a8986dd76ec0eba90e1adbf5787c) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "11.bin", 0x000000, 0x40000, CRC(00eecb03) SHA1(5913da4d2ad97c1ce5e8e601a22b499cd93af744) )
	ROM_LOAD( "10.bin", 0x040000, 0x40000, CRC(7556bee9) SHA1(3fe99c7e9378791b79c43b04f5d0a36404448beb) )
	ROM_LOAD( "9.bin",  0x080000, 0x40000, CRC(13194d89) SHA1(95c35b6012f98a64630abb40fd55b24ff8a5e031) )
	ROM_LOAD( "8.bin",  0x0c0000, 0x40000, CRC(79f14528) SHA1(9c07d9a9e59f69a525bbaec05d74eb8d21bb9563) )

	ROM_REGION( 0x080000, "user1", 0 ) /* Samples */
	ROM_LOAD( "1.bin", 0x00000, 0x80000, CRC(76296578) SHA1(04eca78abe60b283269464c0d12815579126ac08) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki", 0 ) /* Samples */
	ROM_COPY( "user1", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user1", 0x000000, 0x020000, 0x020000)
	ROM_COPY( "user1", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user1", 0x020000, 0x060000, 0x020000)
	ROM_COPY( "user1", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user1", 0x040000, 0x0a0000, 0x020000)
	ROM_COPY( "user1", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "user1", 0x060000, 0x0e0000, 0x020000)

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-twinbrat.bin", 0x0000, 0x0080, CRC(9366263d) SHA1(ff5155498ed0b349ecc1ce98a39566b642201cf2) )
ROM_END

ROM_START( twinbrata )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x20000, CRC(5e75f568) SHA1(f42d2a73d737e6b01dd049eea2a10fc8c8096d8f) )
	ROM_LOAD16_BYTE( "3.bin", 0x00001, 0x20000, CRC(0e3fa9b0) SHA1(0148cc616eac84dc16415e1557ec6040d14392d4) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "6.bin", 0x000000, 0x80000, CRC(af10ddfd) SHA1(e5e83044f20d6cbbc1b4ef1812ac57b6dc958a8a) )
	ROM_LOAD16_BYTE( "7.bin", 0x000001, 0x80000, CRC(3696345a) SHA1(ea38be3586757527b2a1aad2e22b83937f8602da) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x80000, CRC(1ae8a751) SHA1(5f30306580c6ab4af0ddbdc4519eb4e0ab9bd23a) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x80000, CRC(cf235eeb) SHA1(d067e2dd4f28a8986dd76ec0eba90e1adbf5787c) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "11.bin", 0x000000, 0x40000, CRC(00eecb03) SHA1(5913da4d2ad97c1ce5e8e601a22b499cd93af744) )
	ROM_LOAD( "10.bin", 0x040000, 0x40000, CRC(7556bee9) SHA1(3fe99c7e9378791b79c43b04f5d0a36404448beb) )
	ROM_LOAD( "9.bin",  0x080000, 0x40000, CRC(13194d89) SHA1(95c35b6012f98a64630abb40fd55b24ff8a5e031) )
	ROM_LOAD( "8.bin",  0x0c0000, 0x40000, CRC(79f14528) SHA1(9c07d9a9e59f69a525bbaec05d74eb8d21bb9563) )

	ROM_REGION( 0x080000, "user1", 0 ) /* Samples */
	ROM_LOAD( "1.bin", 0x00000, 0x80000, CRC(76296578) SHA1(04eca78abe60b283269464c0d12815579126ac08) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki", 0 ) /* Samples */
	ROM_COPY( "user1", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user1", 0x000000, 0x020000, 0x020000)
	ROM_COPY( "user1", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user1", 0x020000, 0x060000, 0x020000)
	ROM_COPY( "user1", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user1", 0x040000, 0x0a0000, 0x020000)
	ROM_COPY( "user1", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "user1", 0x060000, 0x0e0000, 0x020000)

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-twinbrat.bin", 0x0000, 0x0080, CRC(9366263d) SHA1(ff5155498ed0b349ecc1ce98a39566b642201cf2) )
ROM_END

DRIVER_INIT_MEMBER(stlforce_state,stlforce)
{
	m_sprxoffs = 0;
}

DRIVER_INIT_MEMBER(stlforce_state,twinbrat)
{
	m_sprxoffs = 9;
}


GAME( 1994, stlforce, 0,        stlforce, stlforce, stlforce_state, stlforce, ROT0, "Electronic Devices Italy / Ecogames S.L. Spain", "Steel Force", MACHINE_SUPPORTS_SAVE )
GAME( 1995, twinbrat, 0,        twinbrat, stlforce, stlforce_state, twinbrat, ROT0, "Elettronica Video-Games S.R.L.", "Twin Brats (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, twinbrata,twinbrat, twinbrat, stlforce, stlforce_state, twinbrat, ROT0, "Elettronica Video-Games S.R.L.", "Twin Brats (set 2)", MACHINE_SUPPORTS_SAVE )
