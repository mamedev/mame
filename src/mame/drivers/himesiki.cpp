// license:BSD-3-Clause
// copyright-holders:Uki, David Haywood
/*****************************************************************************

Himeshikibu (C) 1989 Hi-Soft
Android (C) 198? Nasco

    Driver by Uki

*****************************************************************************/

// Android uses PCBS MK-P102 and MK-P101 ONLY, there is no MK-P103 (extra sprites used on Himeshikibu)
// Real hardware video of parent set can be seen at https://www.youtube.com/watch?v=5rtqZqMBACI (uploaded by Chris Hardy)
// for some reason music fails to play the 2nd attract loop in MAME? 



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
#include "machine/i8255.h"

#define MCLK    XTAL_12MHz // this is on the video board
#define CLK2    XTAL_8MHz // near the CPUs

WRITE8_MEMBER(himesiki_state::himesiki_rombank_w)
{
	membank("bank1")->set_entry(((data & 0x0c) >> 2));

	m_flipscreen = (data & 0x10)>>4;
	flip_screen_set(m_flipscreen);

	if (data & 0xe3)
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
	AM_RANGE(0xa000, 0xa0ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xa100, 0xa7ff) AM_RAM AM_SHARE("sprram_p103a") // not on Android
	AM_RANGE(0xa800, 0xafff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xb000, 0xbfff) AM_RAM_WRITE(himesiki_bg_ram_w) AM_SHARE("bg_ram")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( himesiki_iom0, AS_IO, 8, himesiki_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write) // inputs
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write) // dips + rombank
	AM_RANGE(0x08, 0x08) AM_WRITE(himesiki_scrolly_w)
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

static INPUT_PORTS_START( androidpo )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" ) // first dragon scene only shows 3?
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, "Invalid" ) // can't coin up or start? (probably a non-functioning freeplay)

	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
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


static INPUT_PORTS_START( androidp )
	PORT_INCLUDE(androidpo)
	
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )

	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) ) 
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )

INPUT_PORTS_END

/****************************************************************************/

static const gfx_layout layout_bg =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24 },
	{ STEP8(0,32) },
	8*8*4
};

static const gfx_layout layout_p103a =
{
	32,32,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24,36,32,44,40,52,48,60,56, 68,64,76,72,84,80,92,88,100,96,108,104,116,112,124,120 },
	{ STEP32(0,128) },
	32*32*4
};

static const gfx_layout layout_spr =
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
	GFXDECODE_ENTRY( "bgtiles",   0, layout_bg, 0x000, 16 )
	GFXDECODE_ENTRY( "sprites", 0, layout_spr, 0x200, 16 )
	GFXDECODE_ENTRY( "spr_p103a", 0, layout_p103a, 0x200, 16 )
GFXDECODE_END


void himesiki_state::machine_start()
{
	UINT8 *ROM = memregion("banks")->base();

	membank("bank1")->configure_entries(0, 4, ROM, 0x4000);


	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));

	save_item(NAME(m_flipscreen));
}

void himesiki_state::machine_reset()
{
	m_scrollx[0] = 0;
	m_scrollx[1] = 0;
	m_scrolly = 0;

	m_flipscreen = 0;
}

static MACHINE_CONFIG_START( himesiki, himesiki_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CLK2) /* it's a 6.000 MHz rated part, but near the 8 Mhz XTAL?? - Android skips lots of frames at 6, crashes at 4 */
	MCFG_CPU_PROGRAM_MAP(himesiki_prm0)
	MCFG_CPU_IO_MAP(himesiki_iom0)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", himesiki_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, CLK2/2) /* 4.000 MHz (4Mhz rated part, near the 8 Mhz XTAL) */
	MCFG_CPU_PROGRAM_MAP(himesiki_prm1)
	MCFG_CPU_IO_MAP(himesiki_iom1)

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("1P"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("2P"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("OTHERS"))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("DSW1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("DSW2"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(himesiki_state, himesiki_rombank_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 24*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(himesiki_state, screen_update_himesiki)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", himesiki)
	MCFG_PALETTE_ADD_INIT_BLACK("palette", 1024)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ym2203", YM2203, CLK2/4) // ?? 
	MCFG_YM2203_IRQ_HANDLER(INPUTLINE("sub", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
	MCFG_SOUND_ROUTE(1, "mono", 0.10)
	MCFG_SOUND_ROUTE(2, "mono", 0.10)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

MACHINE_CONFIG_END

/****************************************************************************/

ROM_START( himesiki )
	ROM_REGION( 0x08000, "maincpu", 0 ) /* z80 */
	ROM_LOAD( "1.1k",  0x00000,  0x08000, CRC(fb4604b3) SHA1(e8155bbafb881125e1bf9a04808d6a6546887e90) )

	ROM_REGION( 0x10000, "banks", 0 )
	ROM_LOAD( "2.1g",  0x00000,  0x04000, CRC(0c30ded1) SHA1(0ad67115fa15d0b6261a278a946a6d46c06430ef) )
	ROM_CONTINUE(      0x08000,  0x04000)
	// 1j is unpopulated on this game

	ROM_REGION( 0x010000, "sub", 0 ) /* z80 */
	ROM_LOAD( "5.6n",  0x00000,  0x08000, CRC(b1214ac7) SHA1(ee5459c28d9c3c2eb3467261716b1259ec486534) )

	ROM_REGION( 0x020000, "bgtiles", 0 ) /* bg */
	ROM_LOAD( "3.5f",  0x000000,  0x010000, CRC(73843e60) SHA1(0d8a397d8798e15f3fa7bf7a83e4c2ee44f6fa86) )
	ROM_LOAD( "4.5d",  0x010000,  0x010000, CRC(443a3164) SHA1(08aa002214251a870581a01d775f497dd390957c) )

	ROM_REGION( 0x040000, "sprites", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "13.9e", 0x000000,  0x010000, CRC(43102682) SHA1(0d4bde8bece0cbc6c06071aa8ad210a0636d862f) )
	ROM_LOAD16_BYTE( "12.9c", 0x000001,  0x010000, CRC(19c8f9f4) SHA1(b14c8a6b94fd474be375e7a6a03d7f4517da2247) )
	ROM_LOAD16_BYTE( "15.8e", 0x020000,  0x010000, CRC(2630d394) SHA1(b2e9e836b1f053fce3212912c53d3cdca3372439) )
	ROM_LOAD16_BYTE( "14.8c", 0x020001,  0x010000, CRC(8103a207) SHA1(0dde8a0aaf2618d9c1589f35841db210439d0388) )


	ROM_REGION( 0x060000, "spr_p103a", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "6.1a",  0x000000,  0x010000, CRC(14989c22) SHA1(fe0c31df10237294ea8ef0ab8965ba5bb25113a2) )
	ROM_LOAD16_BYTE( "7.1c",  0x000001,  0x010000, CRC(cec56e16) SHA1(836ff413301044313fdf7af5d304c145137b898a) )
	ROM_LOAD16_BYTE( "8.2a",  0x020000,  0x010000, CRC(44ba127e) SHA1(d756b6c3075d75287f9c8be662c1eab02f4245a3) )
	ROM_LOAD16_BYTE( "9.2c",  0x020001,  0x010000, CRC(0dda724a) SHA1(2b064b1d657f896e8385f17def9e4ffc0802bf97) )
	ROM_LOAD16_BYTE( "10.4a", 0x040000,  0x010000, CRC(0adda8d1) SHA1(dfee2c7921fdc972b4e95fdf89520f74a4e8b4ee) )
	ROM_LOAD16_BYTE( "11.4c", 0x040001,  0x010000, CRC(aa032946) SHA1(bd8900e4a22580e3bfe33b8164909db19bb07a8f) )

ROM_END



ROM_START( androidpo )
	ROM_REGION( 0x08000, "maincpu", 0 )
	ROM_LOAD( "MITSUBISHI_A01.toppcb.m5l27256k.k1.BIN", 0x00000, 0x08000, CRC(25ab85eb) SHA1(e1fab149c83ff880b119258206d5818f3db641c5) )

	ROM_REGION( 0x10000, "banks", 0 )
	ROM_LOAD( "MITSUBISHI_A03.toppcb.m5l27256k.G1.BIN", 0x00000, 0x04000, CRC(6cf5f48a) SHA1(b9b4e5e7bace0e8d98fbc9f4ad91bc56ef42099e) )
	ROM_CONTINUE(                                       0x08000, 0x04000)
	ROM_LOAD( "MITSUBISHI_A02.toppcb.m5l27256k.J1.BIN", 0x04000, 0x04000, CRC(e41426be) SHA1(e7e06ef3ff5160bb7d870e148ba2799da52cf24c) )
	ROM_CONTINUE(                                       0x0c000, 0x04000)

	ROM_REGION( 0x18000, "sub", 0 )
	ROM_LOAD( "MITSUBISHI_A04.toppcb.m5l27256k.N6.BIN", 0x00000, 0x08000, CRC(13c38fe4) SHA1(34a35fa057159a5c83892a88b8c908faa39d5cb3) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "MITSUBISHI_A05.toppcb.m5l27512k.F5.BIN", 0x00000, 0x10000, CRC(4c72a930) SHA1(f1542844391b55fe43293eef7ce48c09b7aca75a) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD16_BYTE( "MITSUBISHI_A06.botpcb.m5l27512k.9E.BIN", 0x00000, 0x10000, CRC(5e42984e) SHA1(2a928960c740dfb94589e011cce093bed2fd7685) )
	ROM_LOAD16_BYTE( "MITSUBISHI_A07.botpcb.m5l27512k.9B.BIN", 0x00001, 0x10000, CRC(611ff400) SHA1(1a9aed33d0e3f063811f92b9fee3ecbff0e965bf) )

	ROM_REGION( 0x20000, "spr_p103a", ROMREGION_ERASEFF )
	// there's no P103A PCB for this on Android


	// + 2 undumped PLDs
ROM_END

ROM_START( androidp )
	ROM_REGION( 0x08000, "maincpu", 0 )
	ROM_LOAD( "ANDR1.BIN", 0x00000, 0x08000, CRC(fff04130) SHA1(9bdafa8b311cc5d0851b04df3c6dd16eb087a5dd) )

	ROM_REGION( 0x10000, "banks", 0 )
	ROM_LOAD( "ANDR3.BIN", 0x00000, 0x04000, CRC(112d5123) SHA1(653109eae7b58d9dcb8892ea9aca17427f14c145) )
	ROM_CONTINUE(                                       0x08000, 0x04000)

	ROM_REGION( 0x18000, "sub", 0 )
	ROM_LOAD( "ANDR4.BIN", 0x00000, 0x08000, CRC(65f5e98b) SHA1(69f979d653695413a1c503c402d4bf5ffcfb6e5d) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "ANDR5.BIN", 0x00000, 0x10000, CRC(0a0b44c0) SHA1(8d359b802c7dee5faea9464f06b672fd401799cf) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ANDR6.BIN", 0x00000, 0x10000, CRC(122b7dd1) SHA1(5dffd2b97c8222afc98552513b84a91d6127f41b) )
	ROM_LOAD16_BYTE( "ANDR7.BIN", 0x00001, 0x10000, CRC(fc0f9234) SHA1(496a918cc1f4d0e7191a49cc43c51fbd71e0bdf5) )

	ROM_REGION( 0x20000, "spr_p103a", ROMREGION_ERASEFF )
	// there's no P103A PCB for this on Android


	// + 2 undumped PLDs (?)
ROM_END


GAME( 1989, himesiki, 0,         himesiki, himesiki,  driver_device, 0, ROT90, "Hi-Soft", "Himeshikibu (Japan)", MACHINE_SUPPORTS_SAVE )

// the game changed significantly between these 2 versions
GAME( 198?, androidp,  0,          himesiki, androidp,  driver_device, 0, ROT90, "Nasco", "Android (prototype, later build)", MACHINE_SUPPORTS_SAVE )
GAME( 198?, androidpo, androidp,   himesiki, androidpo, driver_device, 0, ROT90, "Nasco", "Android (prototype, early build)", MACHINE_SUPPORTS_SAVE )
