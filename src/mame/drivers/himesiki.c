// license:BSD-3-Clause
// copyright-holders:Uki
/*****************************************************************************

Himeshikibu (C) 1989 Hi-Soft

    Driver by Uki

*****************************************************************************/

/*
Himeshikibu
(c)1989 Hi-Soft (distributed by Rollertron)

CPU: Z80x2
XTAL1: 8.000MHz
Sound: YM2203C Y3014B


MK-P102 (top board)

  -----------CN1-----------  -----------CN2-----------
A
B
C                                       TMM2018D-45 8         |
D MB8416A-15                    (27512)4            2         |
E MB8416A-15                    (27512)3            5         J
                                                    5         A
F (EPL10P8)I                                                  M
G (27256)2                                          8 DSW1    M
J             (EPL10P8)M                            2         A
K (27256)1                                          5 DSW2    |
                                                    5         |

L M5M5165P-12L    Z                 Z               O
M                 8                 8               P
N 74HC132         0                 0   MB8416A-15  N
         BATTERY           XTAL1         (27256)5      3014B
 1         2      3          4      5        6      7    8


ET-P103A (middle board)

  -----------CN4-----------  -----------CN3-----------
  -----------CN2-----------  -----------CN1-----------
12
11
10       M5M5117          M5M5117
 9                                  (EMPTY)   (EMPTY)
 8                                  (EMPTY)   (EMPTY)
 7                                  (EMPTY)   (EMPTY)
 6                                  (EMPTY)   (EMPTY)
 5                                  (EMPTY)   (EMPTY)
 4                                  (27512)11 (27512)10
 3                                  (27512)9  (27512)8
 2                                  (27512)7  (27512)6
 1
    J    H    G     F        E   D     C     B     A


MK-P101 (bottom board)
  -----------CN2-----------  -----------CN1-----------
L
K     MB8416A-15
J     MB8416A-15
H
G
F 2   2
  7   7
  5   5
E 1   1
  2   2
 (13)(15)
D              TMM2018D-45
  2   2
  7   7
C 5   5        TMM2018D-45
  1   1
  2   2
B(12)(14)

A                                                   12.000MHz
  9   8    7        6       5      4      3      2      1

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "includes/himesiki.h"

#define MCLK    XTAL_12MHz

WRITE8_MEMBER(himesiki_state::himesiki_rombank_w)
{
	membank("bank1")->set_entry(((data & 0x08) >> 3));

	if (data & 0xf7)
		logerror("p06_w %02x\n", data);
}

WRITE8_MEMBER(himesiki_state::himesiki_sound_w)
{
	soundlatch_byte_w(space, offset, data);
	m_subcpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

/****************************************************************************/

static ADDRESS_MAP_START( himesiki_prm0, AS_PROGRAM, 8, himesiki_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xa800, 0xafff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xb000, 0xbfff) AM_RAM_WRITE(himesiki_bg_ram_w) AM_SHARE("bg_ram")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( himesiki_iom0, AS_IO, 8, himesiki_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("1P")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("2P")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("OTHERS")
	AM_RANGE(0x03, 0x03) AM_WRITENOP // 8255 cw
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW1")
	AM_RANGE(0x05, 0x05) AM_READ_PORT("DSW2")
	AM_RANGE(0x06, 0x06) AM_WRITE(himesiki_rombank_w)
	AM_RANGE(0x07, 0x07) AM_WRITENOP // 8255 cw
	AM_RANGE(0x08, 0x08) AM_WRITE(himesiki_flip_w)
	AM_RANGE(0x09, 0x0a) AM_WRITE(himesiki_scrollx_w)
	AM_RANGE(0x0b, 0x0b) AM_WRITE(himesiki_sound_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( himesiki_prm1, AS_PROGRAM, 8, himesiki_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( himesiki_iom1, AS_IO, 8, himesiki_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ym2203", ym2203_device, read, write)
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

/****************************************************************************/

static INPUT_PORTS_START( himesiki )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "1-2" )                   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "1-3" )                   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, "1-5" )                   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "1-6" )                   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "2-1" )                   PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "2-2" )                   PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "2-3" )                   PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "2-4" )                   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "2-5" )                   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "2-6" )                   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "2-7" )                   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "2-8" )                   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("1P")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("2P")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("OTHERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) // coin?
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN2 ) // service?
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/****************************************************************************/

static const gfx_layout layout1 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24 },
	{ STEP8(0,32) },
	8*8*4
};

static const gfx_layout layout2 =
{
	32,32,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24,36,32,44,40,52,48,60,56, 68,64,76,72,84,80,92,88,100,96,108,104,116,112,124,120 },
	{ STEP32(0,128) },
	32*32*4
};

static const gfx_layout layout3 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24,36,32,44,40,52,48,60,56 },
	{ STEP16(0,64) },
	16*16*4
};

static GFXDECODE_START( himesiki )
	GFXDECODE_ENTRY( "bgtiles",   0, layout1, 0x000, 16 )
	GFXDECODE_ENTRY( "sprites_1", 0, layout2, 0x200, 16 )
	GFXDECODE_ENTRY( "sprites_2", 0, layout3, 0x200, 16 )
GFXDECODE_END


void himesiki_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 2, &ROM[0x10000], 0x4000);


	save_item(NAME(m_scrollx));
	save_item(NAME(m_flipscreen));
}

void himesiki_state::machine_reset()
{
	m_scrollx[0] = 0;
	m_scrollx[1] = 0;
	m_flipscreen = 0;
}

static MACHINE_CONFIG_START( himesiki, himesiki_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MCLK/2) /* 6.000 MHz */
	MCFG_CPU_PROGRAM_MAP(himesiki_prm0)
	MCFG_CPU_IO_MAP(himesiki_iom0)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", himesiki_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, MCLK/3) /* 4.000 MHz */
	MCFG_CPU_PROGRAM_MAP(himesiki_prm1)
	MCFG_CPU_IO_MAP(himesiki_iom1)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 24*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(himesiki_state, screen_update_himesiki)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", himesiki)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ym2203", YM2203, MCLK/4)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
	MCFG_SOUND_ROUTE(1, "mono", 0.10)
	MCFG_SOUND_ROUTE(2, "mono", 0.10)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

MACHINE_CONFIG_END

/****************************************************************************/

ROM_START( himesiki )
	ROM_REGION( 0x018000, "maincpu", 0 ) /* z80 */
	ROM_LOAD( "1.1k",  0x00000,  0x08000, CRC(fb4604b3) SHA1(e8155bbafb881125e1bf9a04808d6a6546887e90) )
	ROM_LOAD( "2.1g",  0x10000,  0x08000, CRC(0c30ded1) SHA1(0ad67115fa15d0b6261a278a946a6d46c06430ef) )

	ROM_REGION( 0x010000, "sub", 0 ) /* z80 */
	ROM_LOAD( "5.6n",  0x00000,  0x08000, CRC(b1214ac7) SHA1(ee5459c28d9c3c2eb3467261716b1259ec486534) )

	ROM_REGION( 0x020000, "bgtiles", 0 ) /* bg */
	ROM_LOAD( "3.5f",  0x000000,  0x010000, CRC(73843e60) SHA1(0d8a397d8798e15f3fa7bf7a83e4c2ee44f6fa86) )
	ROM_LOAD( "4.5d",  0x010000,  0x010000, CRC(443a3164) SHA1(08aa002214251a870581a01d775f497dd390957c) )

	ROM_REGION( 0x060000, "sprites_1", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "6.1a",  0x000000,  0x010000, CRC(14989c22) SHA1(fe0c31df10237294ea8ef0ab8965ba5bb25113a2) )
	ROM_LOAD16_BYTE( "7.1c",  0x000001,  0x010000, CRC(cec56e16) SHA1(836ff413301044313fdf7af5d304c145137b898a) )
	ROM_LOAD16_BYTE( "8.2a",  0x020000,  0x010000, CRC(44ba127e) SHA1(d756b6c3075d75287f9c8be662c1eab02f4245a3) )
	ROM_LOAD16_BYTE( "9.2c",  0x020001,  0x010000, CRC(0dda724a) SHA1(2b064b1d657f896e8385f17def9e4ffc0802bf97) )
	ROM_LOAD16_BYTE( "10.4a", 0x040000,  0x010000, CRC(0adda8d1) SHA1(dfee2c7921fdc972b4e95fdf89520f74a4e8b4ee) )
	ROM_LOAD16_BYTE( "11.4c", 0x040001,  0x010000, CRC(aa032946) SHA1(bd8900e4a22580e3bfe33b8164909db19bb07a8f) )

	ROM_REGION( 0x040000, "sprites_2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "13.9e", 0x000000,  0x010000, CRC(43102682) SHA1(0d4bde8bece0cbc6c06071aa8ad210a0636d862f) )
	ROM_LOAD16_BYTE( "12.9c", 0x000001,  0x010000, CRC(19c8f9f4) SHA1(b14c8a6b94fd474be375e7a6a03d7f4517da2247) )
	ROM_LOAD16_BYTE( "15.8e", 0x020000,  0x010000, CRC(2630d394) SHA1(b2e9e836b1f053fce3212912c53d3cdca3372439) )
	ROM_LOAD16_BYTE( "14.8c", 0x020001,  0x010000, CRC(8103a207) SHA1(0dde8a0aaf2618d9c1589f35841db210439d0388) )
ROM_END

GAME( 1989, himesiki, 0, himesiki, himesiki, driver_device, 0, ROT90, "Hi-Soft", "Himeshikibu (Japan)", MACHINE_SUPPORTS_SAVE )
