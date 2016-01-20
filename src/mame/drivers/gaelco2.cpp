// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Gaelco CG-1V/GAE1 based games

    Driver by Manuel Abadia <emumanu+mame@gmail.com>

    Known games that run on this hardware:
    ======================================
    Game           | Year | Chip      | Ref      |Protected
    ---------------+------+-----------+----------+--------------------
    Alligator Hunt | 1994 | GAE1 449  | 940411   | DS5002FP, but unprotected version available
    World Rally 2  | 1995 | GAE1 449  | 950510   | DS5002FP
    World Rally 2  | 1995 | GAE1 506  | 950510-1 | DS5002FP
    Touch & Go     | 1995 | GAE1 501  | 950906   | DS5002FP
    Touch & Go     | 1995 | GAE1 501  | 950510-1 | DS5002FP
    Maniac Square  | 1996 | Unknown   | ???      | DS5002FP, but unprotected version available
    Snow Board     | 1996 | CG-1V 366 | 960419/1 | Lattice IspLSI 1016-80LJ
    Bang!          | 1998 | CG-1V 388 | 980921/1 | No

***************************************************************************/

#include "emu.h"
#include "machine/eepromser.h"
#include "sound/gaelco.h"
#include "rendlay.h"
#include "includes/gaelco2.h"

#define TILELAYOUT16(NUM) static const gfx_layout tilelayout16_##NUM =              \
{                                                                                   \
	16,16,                                          /* 16x16 tiles */               \
	NUM/32,                                         /* number of tiles */           \
	5,                                              /* 5 bpp */                     \
	{ 4*NUM*8, 3*NUM*8, 2*NUM*8, 1*NUM*8, 0*NUM*8 },                                \
	{ 0,1,2,3,4,5,6,7, 16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7 },   \
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8, 8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },     \
	32*8                                                                            \
};

#define GFXDECODEINFO(NUM,ENTRIES) \
static GFXDECODE_START( NUM )\
	GFXDECODE_ENTRY( "gfx1", 0x0000000, tilelayout16_##NUM,0,   ENTRIES )                       \
GFXDECODE_END


TILELAYOUT16(0x0080000)
GFXDECODEINFO(0x0080000, 128)
TILELAYOUT16(0x0200000)
GFXDECODEINFO(0x0200000, 128)
TILELAYOUT16(0x0400000)
GFXDECODEINFO(0x0400000, 128)


/*============================================================================
                            MANIAC SQUARE (FINAL)
  ============================================================================*/

static ADDRESS_MAP_START( maniacsq_map, AS_PROGRAM, 16, gaelco2_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                                                     /* ROM */
	AM_RANGE(0x202890, 0x2028ff) AM_DEVREADWRITE("gaelco", gaelco_gae1_device, gaelcosnd_r, gaelcosnd_w)    /* Sound Registers */
	AM_RANGE(0x200000, 0x20ffff) AM_RAM_WRITE(gaelco2_vram_w) AM_SHARE("spriteram")                         /* Video RAM */
	AM_RANGE(0x210000, 0x211fff) AM_RAM_WRITE(gaelco2_palette_w) AM_SHARE("paletteram")                     /* Palette */
	AM_RANGE(0x218004, 0x218009) AM_RAM AM_SHARE("vregs")                                                   /* Video Registers */
	AM_RANGE(0x300000, 0x300001) AM_READ_PORT("IN0")                                                        /* DSW #1 + Input 1P */
	AM_RANGE(0x300002, 0x300003) AM_READ_PORT("IN1")                                                        /* DSW #2 + Input 2P */
	AM_RANGE(0x30004a, 0x30004b) AM_WRITENOP                                                                /* Sound muting? */
	AM_RANGE(0x320000, 0x320001) AM_READ_PORT("COIN")                                                       /* COINSW + SERVICESW */
	AM_RANGE(0x500000, 0x500001) AM_WRITE(gaelco2_coin_w)                                                   /* Coin lockout + counters */
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM                                                                     /* Work RAM */
ADDRESS_MAP_END


static INPUT_PORTS_START( maniacsq )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0700, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, "Disabled or Free Play (if Coin B too)" )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0x7000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, "Disabled or Free Play (if Coin A too)" )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "1P Continue" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_IMPULSE(1) /* go to service mode NOW */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static MACHINE_CONFIG_START( maniacsq, gaelco2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 26000000/2)     /* 13 MHz? */
	MCFG_CPU_PROGRAM_MAP(maniacsq_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gaelco2_state,  irq6_line_hold)

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(gaelco2_state, screen_update_gaelco2)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 0x0080000)
	MCFG_PALETTE_ADD("palette", 4096*16 - 16)   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_DEVICE_ADD("gaelco", GAELCO_GAE1, 0)
	MCFG_GAELCO_SND_DATA("gfx1")
	MCFG_GAELCO_BANKS(0 * 0x0080000, 1 * 0x0080000, 0, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


ROM_START( maniacsq )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "d8-d15.1m",   0x000000, 0x020000, CRC(9121d1b6) SHA1(ad8f0d996b6d42fc0c6645466608e82ca96e0b66) )
	ROM_LOAD16_BYTE( "d0-d7.1m",    0x000001, 0x020000, CRC(a95cfd2a) SHA1(b5bad76f12d2a1f6bf6b35482f2f933ceb00e552) )

	ROM_REGION( 0x0280000, "gfx1", 0 ) /* GFX + Sound */
	ROM_LOAD( "d0-d7.4m",   0x0000000, 0x0080000, CRC(d8551b2f) SHA1(78b5b07112bd89fed18055180e7cc64f8e0bd0b1) )    /* GFX + Sound */
	ROM_LOAD( "d8-d15.4m",  0x0080000, 0x0080000, CRC(b269c427) SHA1(b7f9501529fbb7ee82700cff82740ba5770cf3c5) )    /* GFX + Sound */
	ROM_LOAD( "d16-d23.1m", 0x0100000, 0x0020000, CRC(af4ea5e7) SHA1(ffaf09dc2588e32c124e7dd2f86ba009f1b8b176) )    /* GFX only */
	ROM_FILL(               0x0120000, 0x0060000, 0x00 )         /* Empty */
	ROM_LOAD( "d24-d31.1m", 0x0180000, 0x0020000, CRC(578c3588) SHA1(c2e1fba29f21d6822677886fb2d26e050b336c14) )    /* GFX only */
	ROM_FILL(               0x01a0000, 0x0060000, 0x00 )         /* Empty */
	ROM_FILL(               0x0200000, 0x0080000, 0x00 )         /* to decode GFX as 5bpp */
ROM_END



/*============================================================================
                                BANG
  ============================================================================*/

READ16_MEMBER(bang_state::p1_gun_x){return (m_light0_x->read() * 320 / 0x100) + 1;}
READ16_MEMBER(bang_state::p1_gun_y){return (m_light0_y->read() * 240 / 0x100) - 4;}
READ16_MEMBER(bang_state::p2_gun_x){return (m_light1_x->read() * 320 / 0x100) + 1;}
READ16_MEMBER(bang_state::p2_gun_y){return (m_light1_y->read() * 240 / 0x100) - 4;}

static ADDRESS_MAP_START( bang_map, AS_PROGRAM, 16, bang_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                                                     /* ROM */
	AM_RANGE(0x202890, 0x2028ff) AM_DEVREADWRITE("gaelco", gaelco_cg1v_device, gaelcosnd_r, gaelcosnd_w)    /* Sound Registers */
	AM_RANGE(0x200000, 0x20ffff) AM_RAM_WRITE(gaelco2_vram_w) AM_SHARE("spriteram")                         /* Video RAM */
	AM_RANGE(0x210000, 0x211fff) AM_RAM_WRITE(gaelco2_palette_w) AM_SHARE("paletteram")                     /* Palette */
	AM_RANGE(0x218004, 0x218009) AM_READONLY                                                                /* Video Registers */
	AM_RANGE(0x218004, 0x218007) AM_WRITEONLY AM_SHARE("vregs")                                             /* Video Registers */
	AM_RANGE(0x218008, 0x218009) AM_WRITENOP                                                                /* CLR INT Video */
	AM_RANGE(0x300000, 0x300001) AM_READ_PORT("P1")
	AM_RANGE(0x300002, 0x300003) AM_READNOP                                                                 /* Random number generator? */
	AM_RANGE(0x300000, 0x300003) AM_WRITE(gaelco2_coin2_w)                                                  /* Coin Counters */
	AM_RANGE(0x300008, 0x300009) AM_WRITE(gaelco2_eeprom_data_w)                                            /* EEPROM data */
	AM_RANGE(0x30000a, 0x30000b) AM_WRITE(gaelco2_eeprom_sk_w)                                              /* EEPROM serial clock */
	AM_RANGE(0x30000c, 0x30000d) AM_WRITE(gaelco2_eeprom_cs_w)                                              /* EEPROM chip select */
	AM_RANGE(0x300010, 0x300011) AM_READ_PORT("P2")
	AM_RANGE(0x300020, 0x300021) AM_READ_PORT("COIN")
	AM_RANGE(0x310000, 0x310001) AM_READ(p1_gun_x) AM_WRITE(bang_clr_gun_int_w)                             /* Gun 1P X */ /* CLR INT Gun */
	AM_RANGE(0x310002, 0x310003) AM_READ(p2_gun_x)                                                          /* Gun 2P X */
	AM_RANGE(0x310004, 0x310005) AM_READ(p1_gun_y)                                                          /* Gun 1P Y */
	AM_RANGE(0x310006, 0x310007) AM_READ(p2_gun_y)                                                          /* Gun 2P Y */
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM                                                                     /* Work RAM */
ADDRESS_MAP_END


static INPUT_PORTS_START( bang )
	PORT_START("P1")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(1) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW ) /* go to service mode NOW */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read) /* bit 6 is EEPROM data (DOUT) */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SPECIAL )  /* bit 7 is EEPROM ready */

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, -6.0 / 240, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, -6.0 / 240, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

static MACHINE_CONFIG_START( bang, bang_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 30000000/2)         /* 15 MHz */
	MCFG_CPU_PROGRAM_MAP(bang_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", bang_state, bang_irq, "screen", 0, 1)

	MCFG_EEPROM_SERIAL_93C66_ADD("eeprom")

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(gaelco2_state, screen_update_gaelco2)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 0x0200000)
	MCFG_PALETTE_ADD("palette", 4096*16 - 16)   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_DEVICE_ADD("gaelco", GAELCO_CG1V, 0)
	MCFG_GAELCO_SND_DATA("gfx1")
	MCFG_GAELCO_BANKS(0 * 0x0200000, 1 * 0x0200000, 2 * 0x0200000, 3 * 0x0200000)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


/*
PCB Layout:

REF: 980921/1
+--------------------------------------------------------------------+
|                                                                    | Plug-In Daughterboard:
|                      TDA1543          KM428C256TR (x2)             | +--------------------------+
|              POT1                                         IC43*  +-| |                          |
+--+                                                               | | | IC1     IC9     IC16   +-|
   |                                                        IC44*  |J| | IC2     IC10    IC17   | |
   |                               +----------+                    |P| | IC3     IC11    IC18   |J| JP1 - 50 pin (dual in-line) header
+--+                               |          |             IC45*  |1| | IC4*    IC12*   IC19*  |P| * Denotes unpopulated sockets
|                                  |  CG-1V   |                    |0| | IC5     IC13    IC20   |1| All roms are 27C040 type
|                     CY7C199      |   388    |             IC46*  | | | IC6*    IC14    IC21   | |
|                     CY7C199      |          |                    | | |                        +-|
| J                                +----------+             IC47*  +-| | 74LS139N   74LS373N x3   |
| A                                                                  | +--------------------------+
| M                                                                  |
| M                                                                  |
| A                                                                  |
|                         30.000MHz                         CY7C199  |
|        93C66N           30.000MHz       +---------+       CY7C199  |
|                                         | Lattice |       CY7C199  |
+--+                                      | IspLSI  |                |
   |                                      |  1016E  |      IC53      |
   |  J J                                 +---------+                |
+--+  P P                               +-----------+       CY7C199  |
|     2 3                               |           |                |
|                                       | MC68HC000 |      IC55      |
|                                       |   FN16    |                |
|                                       +-----------+                |
+--------------------------------------------------------------------+

   CPU: MC68HC000FN16
Custom: CG-1V 388 (System chip, Graphics & Sound)
   OSC: 30.000MHz (x2)
EEPROM: 93C66N

RAM: CY7C199-15PC (IC12, IC13, IC50, IC51, IC52 & IC54)
     KM428C256TR-7 (IC32 & IC33)

* IC42 through IC47 unpopulated

JP10 - 50 pin (dual in-line) header for Plug-In Daughterboard
JP2  - 4 pin light gun header (player 1)
JP3  - 4 pin light gun header (player 2)

*/

ROM_START( bang )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "bang53.ic53", 0x000000, 0x080000, CRC(014bb939) SHA1(bb245acf7a3bd4a56b3559518bcb8d0ae39dbaf4) )
	ROM_LOAD16_BYTE( "bang55.ic55", 0x000001, 0x080000, CRC(582f8b1e) SHA1(c9b0d4c1dee71cdb2c01d49f20ffde32eddc9583) )

	ROM_REGION( 0x0a00000, "gfx1", 0 ) /* GFX + Sound */
	ROM_LOAD( "bang16.ic16", 0x0000000, 0x0080000, CRC(6ee4b878) SHA1(f646380d95650a60b5a17973bdfd3b80450a4d3b) )   /* GFX only */
	ROM_LOAD( "bang17.ic17", 0x0080000, 0x0080000, CRC(0c35aa6f) SHA1(df0474b1b9466d3c199e5aade39b7233f0cb45ee) )   /* GFX only */
	ROM_LOAD( "bang18.ic18", 0x0100000, 0x0080000, CRC(2056b1ad) SHA1(b796f92eef4bbb0efa12c53580e429b8a0aa394c) )   /* Sound only */
	ROM_FILL(                0x0180000, 0x0080000, 0x00 )            /* Empty */
	ROM_LOAD( "bang9.ic9",   0x0200000, 0x0080000, CRC(078195dc) SHA1(362ff194e2579346dfc7af88559b0718bc36ec8a) )   /* GFX only */
	ROM_LOAD( "bang10.ic10", 0x0280000, 0x0080000, CRC(06711eeb) SHA1(3662ffe730fb54ee48925de9765f88be1abd5e4e) )   /* GFX only */
	ROM_LOAD( "bang11.ic11", 0x0300000, 0x0080000, CRC(2088d15c) SHA1(0c043ab9fd33836fa4b7ad60fd8e7cb96ffb6121) )   /* Sound only */
	ROM_FILL(                0x0380000, 0x0080000, 0x00 )            /* Empty */
	ROM_LOAD( "bang1.ic1",   0x0400000, 0x0080000, CRC(e7b97b0f) SHA1(b5503687ae3ca0a0faa4b867a267d89dac788d6d) )   /* GFX only */
	ROM_LOAD( "bang2.ic2",   0x0480000, 0x0080000, CRC(ff297a8f) SHA1(28819a9d7b3cb177e7a7db3fe23a94f5cba33049) )   /* GFX only */
	ROM_LOAD( "bang3.ic3",   0x0500000, 0x0080000, CRC(d3da5d4f) SHA1(b9bea0b4d20ab0bfda3fac2bb1fab974c007aaf0) )   /* Sound only */
	ROM_FILL(                0x0580000, 0x0080000, 0x00 )            /* Empty */
	ROM_LOAD( "bang20.ic20", 0x0600000, 0x0080000, CRC(a1145df8) SHA1(305cda041a6f201cb011982f1bf1fc6a4153a669) )   /* GFX only */
	ROM_LOAD( "bang13.ic13", 0x0680000, 0x0080000, CRC(fe3e8d07) SHA1(7a37561b1cf422b47cddb8751a6b6d57dec8baae) )   /* GFX only */
	ROM_LOAD( "bang5.ic5",   0x0700000, 0x0080000, CRC(9bee444c) SHA1(aebaa3306e7e5aada99ed469da9bf64507808cff) )   /* Sound only */
	ROM_FILL(                0x0780000, 0x0080000, 0x00 )            /* Empty */
	ROM_LOAD( "bang21.ic21", 0x0800000, 0x0080000, CRC(fd93d7f2) SHA1(ff9d8eb5ac8d9757132aa6d79d2f7662c14cd650) )   /* GFX only */
	ROM_LOAD( "bang14.ic14", 0x0880000, 0x0080000, CRC(858fcbf9) SHA1(1e67431c8775666f4839bdc427fabf59ffc708c0) )   /* GFX only */
	ROM_FILL(                0x0900000, 0x0100000, 0x00 )            /* Empty */
ROM_END



ROM_START( bangj )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "bang-a.ic53", 0x000000, 0x080000, CRC(5ee514e9) SHA1(b78b507d18de41be58049f5c597acd107ec1273f) )
	ROM_LOAD16_BYTE( "bang-a.ic55", 0x000001, 0x080000, CRC(b90223ab) SHA1(7c097754a710169f41c574c3cc1a6346824853c4) )

	ROM_REGION( 0x0a00000, "gfx1", 0 ) /* GFX + Sound */
	ROM_LOAD( "bang-a.ic16", 0x0000000, 0x0080000, CRC(3b63acfc) SHA1(48f5598cdbc70f342d6b75909166571271920a8f) )   /* GFX only */
	ROM_LOAD( "bang-a.ic17", 0x0080000, 0x0080000, CRC(72865b80) SHA1(ec7753ea7961015149b9e6386fdeb9bd59aa962a) )   /* GFX only */
	ROM_LOAD( "bang18.ic18", 0x0100000, 0x0080000, CRC(2056b1ad) SHA1(b796f92eef4bbb0efa12c53580e429b8a0aa394c) )   /* Sound only */
	ROM_FILL(                0x0180000, 0x0080000, 0x00 )            /* Empty */
	ROM_LOAD( "bang-a.ic9",  0x0200000, 0x0080000, CRC(3cb86360) SHA1(c803b3add253a552a1554714218740bdfca91764) )   /* GFX only */
	ROM_LOAD( "bang-a.ic10", 0x0280000, 0x0080000, CRC(03fdd777) SHA1(9eec194239f93d961ee9902a585c872dcdc7728f) )   /* GFX only */
	ROM_LOAD( "bang11.ic11", 0x0300000, 0x0080000, CRC(2088d15c) SHA1(0c043ab9fd33836fa4b7ad60fd8e7cb96ffb6121) )   /* Sound only */
	ROM_FILL(                0x0380000, 0x0080000, 0x00 )            /* Empty */
	ROM_LOAD( "bang-a.ic1",  0x0400000, 0x0080000, CRC(965d0ad9) SHA1(eff521735129b7dd9366855c6312ed568950233c) )   /* GFX only */
	ROM_LOAD( "bang-a.ic2",  0x0480000, 0x0080000, CRC(8ea261a7) SHA1(50b59cf058ca03c0b8c888f6ddb40c720a210ece) )   /* GFX only */
	ROM_LOAD( "bang3.ic3",   0x0500000, 0x0080000, CRC(d3da5d4f) SHA1(b9bea0b4d20ab0bfda3fac2bb1fab974c007aaf0) )   /* Sound only */
	ROM_FILL(                0x0580000, 0x0080000, 0x00 )            /* Empty */
	ROM_LOAD( "bang-a.ic20", 0x0600000, 0x0080000, CRC(4b828f3c) SHA1(5227a89c05c659a85d33f092c6778ce9d57a0236) )   /* GFX only */
	ROM_LOAD( "bang-a.ic13", 0x0680000, 0x0080000, CRC(d1146b92) SHA1(2b28d49fbffea6c038160fdab177bc0045195ca8) )   /* GFX only */
	ROM_LOAD( "bang5.ic5",   0x0700000, 0x0080000, CRC(9bee444c) SHA1(aebaa3306e7e5aada99ed469da9bf64507808cff) )   /* Sound only */
	ROM_FILL(                0x0780000, 0x0080000, 0x00 )            /* Empty */
	ROM_LOAD( "bang-a.ic21", 0x0800000, 0x0080000, CRC(531ce3b6) SHA1(196bb720591acc082f815b609a7cf1609510c8c1) )   /* GFX only */
	ROM_LOAD( "bang-a.ic14", 0x0880000, 0x0080000, CRC(f8e1cf84) SHA1(559c08584094e605635c5ef3a25534ea0bcfa199) )   /* GFX only */
	ROM_FILL(                0x0900000, 0x0100000, 0x00 )            /* Empty */
ROM_END


/*============================================================================
                            ALLIGATOR HUNT
  ============================================================================*/


static ADDRESS_MAP_START( alighunt_map, AS_PROGRAM, 16, gaelco2_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                                                         /* ROM */
	AM_RANGE(0x202890, 0x2028ff) AM_DEVREADWRITE("gaelco", gaelco_gae1_device, gaelcosnd_r, gaelcosnd_w)        /* Sound Registers */
	AM_RANGE(0x200000, 0x20ffff) AM_RAM_WRITE(gaelco2_vram_w) AM_SHARE("spriteram")                             /* Video RAM */
	AM_RANGE(0x210000, 0x211fff) AM_RAM_WRITE(gaelco2_palette_w) AM_SHARE("paletteram")                         /* Palette */
	AM_RANGE(0x218004, 0x218009) AM_RAM AM_SHARE("vregs")                                                       /* Video Registers */
	AM_RANGE(0x300000, 0x300001) AM_READ_PORT("IN0")                                                            /* DSW #1 + Input 1P */
	AM_RANGE(0x300002, 0x300003) AM_READ_PORT("IN1")                                                            /* DSW #2 + Input 2P */
	AM_RANGE(0x320000, 0x320001) AM_READ_PORT("COIN")                                                           /* COINSW + SERVICESW */
	AM_RANGE(0x500000, 0x500001) AM_WRITE(gaelco2_coin_w)                                                       /* Coin lockout + counters */
	AM_RANGE(0x500006, 0x500007) AM_WRITENOP                                                                    /* ??? */
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM                                                                         /* Work RAM */
ADDRESS_MAP_END


static INPUT_PORTS_START( alighunt )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0700, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, "Disabled or Free Play (if Coin B too)" )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0x7000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, "Disabled or Free Play (if Coin A too)" )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0c00, "2" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x1000, 0x1000, "Sound Type" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Mono ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Stereo ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Joystick ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, "Analog" )        /* TO-DO */
	PORT_DIPSETTING(      0x4000, DEF_STR( Standard ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2 ) /* go to test mode NOW */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static MACHINE_CONFIG_START( alighunt, gaelco2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 24000000/2)         /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(alighunt_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gaelco2_state,  irq6_line_hold)

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(gaelco2_state, screen_update_gaelco2)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 0x0400000)
	MCFG_PALETTE_ADD("palette", 4096*16 - 16)   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_DEVICE_ADD("gaelco", GAELCO_GAE1, 0)
	MCFG_GAELCO_SND_DATA("gfx1")
	MCFG_GAELCO_BANKS(0 * 0x0400000, 1 * 0x0400000, 2 * 0x0400000, 3 * 0x0400000)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


/*
PCB Layout:

REF: 940411
------------------------------------------------------------------------------
|                POT1               KM428C256J-6 (x3)                        |
|                                                                            |
|                POT2                                                        |
|---                                                                         |
   |                                                               U47       |
   |                   30.000MHz          |----------|                       |
|---                                      |          |             U48       |
|                                         | GAE1 449 |                       |
| J                            6264       | (QFP208) |             U49       |
|                              6264       |          |                       |
| A                                       |----------|             U50       |
|                                                                            |
| M                                                                          |
|                         |-------------------------|                        |
| M                       |                         |  24.000MHz     62256   |
|                         |  62256  DS5002  BATT_3V |                62256   |
| A                       |                         |                        |
|                         |-------------------------|                        |
|                                                                            |
|---                                    62256                                |
   |                                    62256                                |
   |                                                                         |
|---                                                                         |
|   DSW1                         MC68000P12        U45                       |
|                                                  U44                       |
|   DSW2                                                                     |
|                                                                            |
-----------------------------------------------------------------------------|
*/


ROM_START( aligator )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "u45",  0x000000, 0x080000, CRC(61c47c56) SHA1(6dd3fc6fdab252e0fb43c0793eef70203c888d7f) )
	ROM_LOAD16_BYTE(    "u44",  0x000001, 0x080000, CRC(f0be007a) SHA1(2112b2e5f020028b50c8f2c72c83c9fee7a78224) )

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_FILL(               0x1000000, 0x0400000, 0x00 )     /* to decode GFX as 5 bpp */

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "u48",        0x0000000, 0x0400000, CRC(19e03bf1) SHA1(2b3a4bb438b0aebf4f6a9fd26b071e5c9dd222b8) )    /* GFX only */
	ROM_LOAD( "u47",        0x0400000, 0x0400000, CRC(74a5a29f) SHA1(8ea2aa1f8a80c5b88ca9222c5ecc3c4794e0a160) )    /* GFX + Sound */
	ROM_LOAD( "u50",        0x0800000, 0x0400000, CRC(85daecf9) SHA1(824f6d2491075b1ef96ecd6667c5510409338a2f) )    /* GFX only */
	ROM_LOAD( "u49",        0x0c00000, 0x0400000, CRC(70a4ee0b) SHA1(07b09916f0366d0c6eed94a905ec0b9d6ac9e7e1) )    /* GFX + Sound */
ROM_END

ROM_START( aligatorun )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "ahntu45n.040", 0x000000, 0x080000, CRC(fc02cb2d) SHA1(700aa60ec0d2bb705b1335de63daae678dcb8570) )
	ROM_LOAD16_BYTE(    "ahntu44n.040", 0x000001, 0x080000, CRC(7fbea3a3) SHA1(89efa5b7908c2f010a3097954dbccd9cb7adc50c) )

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_FILL(               0x1000000, 0x0400000, 0x00 )     /* to decode GFX as 5 bpp */

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "u48",        0x0000000, 0x0400000, CRC(19e03bf1) SHA1(2b3a4bb438b0aebf4f6a9fd26b071e5c9dd222b8) )    /* GFX only */
	ROM_LOAD( "u47",        0x0400000, 0x0400000, CRC(74a5a29f) SHA1(8ea2aa1f8a80c5b88ca9222c5ecc3c4794e0a160) )    /* GFX + Sound */
	ROM_LOAD( "u50",        0x0800000, 0x0400000, CRC(85daecf9) SHA1(824f6d2491075b1ef96ecd6667c5510409338a2f) )    /* GFX only */
	ROM_LOAD( "u49",        0x0c00000, 0x0400000, CRC(70a4ee0b) SHA1(07b09916f0366d0c6eed94a905ec0b9d6ac9e7e1) )    /* GFX + Sound */
ROM_END




/*============================================================================
                            TOUCH & GO
  ============================================================================*/

/* the game expects this value each frame to know that the DS5002FP is alive */
READ16_MEMBER(gaelco2_state::dallas_kludge_r)
{
	return 0x0200;
}

static ADDRESS_MAP_START( touchgo_map, AS_PROGRAM, 16, gaelco2_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                                                         /* ROM */
	AM_RANGE(0x202890, 0x2028ff) AM_DEVREADWRITE("gaelco", gaelco_gae1_device, gaelcosnd_r, gaelcosnd_w)        /* Sound Registers */
	AM_RANGE(0x200000, 0x20ffff) AM_RAM_WRITE(gaelco2_vram_w) AM_SHARE("spriteram")                             /* Video RAM */
	AM_RANGE(0x210000, 0x211fff) AM_RAM_WRITE(gaelco2_palette_w) AM_SHARE("paletteram")                         /* Palette */
	AM_RANGE(0x218004, 0x218009) AM_RAM AM_SHARE("vregs")                                                       /* Video Registers */
	AM_RANGE(0x300000, 0x300001) AM_READ_PORT("IN0")                                                            /* DSW #1 + Input 1P */
	AM_RANGE(0x300002, 0x300003) AM_READ_PORT("IN1")                                                            /* DSW #2 + Input 2P */
	AM_RANGE(0x300004, 0x300005) AM_READ_PORT("IN2")                                                            /* COINSW + Input 3P */
	AM_RANGE(0x300006, 0x300007) AM_READ_PORT("IN3")                                                            /* SERVICESW + Input 4P */
	AM_RANGE(0x500000, 0x50001f) AM_WRITE(touchgo_coin_w)                                                       /* Coin counters */
	AM_RANGE(0xfefffa, 0xfefffb) AM_RAM_READ(dallas_kludge_r)                                                   /* DS5002FP related patch */
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM                                                                         /* Work RAM */
ADDRESS_MAP_END


static INPUT_PORTS_START( touchgo )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Credit configuration" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "1 Credit Start/1 Credit Continue" )
	PORT_DIPSETTING(      0x0000, "2 Credits Start/1 Credit Continue" )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Slot" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "Independent" )
	PORT_DIPSETTING(      0x0000, "Common" )
	PORT_DIPNAME( 0x3000, 0x0000, "Monitor Type" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "Double monitor, 4 players" )
	PORT_DIPSETTING(      0x2000, "Single monitor, 4 players" )
	PORT_DIPSETTING(      0x3000, "Single monitor, 2 players" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, "Disabled or Free Play (if Coin B too)" ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0300, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, "Disabled or Free Play (if Coin A too)" ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0x3000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE3 ) /* go to test mode NOW */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("FAKE")  /* To switch between monitors at run time */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE4 ) PORT_TOGGLE
INPUT_PORTS_END

static MACHINE_CONFIG_START( touchgo, gaelco2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 32000000/2)         /* 16 MHz */
	MCFG_CPU_PROGRAM_MAP(touchgo_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", gaelco2_state,  irq6_line_hold)

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 0x0400000)
	MCFG_PALETTE_ADD("palette", 4096*16 - 16)   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */
	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 480-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(gaelco2_state, screen_update_gaelco2_left)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 480-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(gaelco2_state, screen_update_gaelco2_right)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2_dual)

	/* sound hardware */
	/* the chip is stereo, but the game sound is mono because the right channel
	   output is for cabinet 1 and the left channel output is for cabinet 2 */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_DEVICE_ADD("gaelco", GAELCO_GAE1, 0)
	MCFG_GAELCO_SND_DATA("gfx1")
	MCFG_GAELCO_BANKS(0 * 0x0400000, 1 * 0x0400000, 0, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

/*
PCB Layout:

REF: 950510-1
------------------------------------------------------------------------------
|                POT1                        KM428C256J-6 (x4)               |
|                POT2                                                        |
|                                            ----------------------------    |
|                                            | (Plug-In Daughterboard)  |    |
|                                            |                          |    |
|---                                         |     IC66        IC67     |    |
   |                                         |                          |    |
   |                                         |                          |    |
|---                                         |     IC65        IC69     |    |
|                                            |                          |    |
|                                            ----------------------------    |
|                                                                            |
|                                            |----------|                    |
| J                                          |          |                    |
|                                            | GAE1 501 |                    |
| A                               6264       | (QFP208) |                    |
|                                 6264       |          |                    |
| M                                          |----------|                    |
|                                                                            |
| M                       |-------------------------|                        |
|                         |                         |  40.000MHz     62256   |
| A                       |  62256  DS5002  BATT_3V |                62256   |
|                         |                         |                        |
|                         |-------------------------|                        |
|                                                                            |
|---                                    62256                                |
   |                                    62256                                |
   |  DSW1                                                                   |
|---  DSW2                                                                   |
|                                                                            |
|                                32.000MHz      MC68000P16      TG57         |
| CONN1                                                         TG56         |
|                                                                            |
| CONN4    CONN2    CONN3                                                    |
-----------------------------------------------------------------------------|
*/


ROM_START( touchgo ) /* REF: 950906 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "tg_56", 0x000000, 0x080000, CRC(8ab065f3) SHA1(7664abd7e5f66ffca4a2865bba56ac36bd04f4e9) )
	ROM_LOAD16_BYTE( "tg_57", 0x000001, 0x080000, CRC(0dfd3f65) SHA1(afb2ce8988c84f211ac71b84928ce4c421de7fee) )

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_LOAD( "ic69",  0x1000000, 0x0200000, CRC(18bb12d4) SHA1(ee6e7a63b86c56d71e62db0ae5892ab3ab94b0a0) ) /* GFX only */

	ROM_REGION( 0x0c00000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "ic65",  0x0000000, 0x0400000, CRC(91b89c7c) SHA1(1c24b494b56845b0f21be40ab737f251d7683c7d) ) /* GFX only */
	ROM_LOAD( "ic66",  0x0400000, 0x0200000, CRC(52682953) SHA1(82cde061bdd827ed4a47a9a4256cd0e887ebc29d) ) /* Sound only */
	ROM_FILL(          0x0600000, 0x0200000, 0x00 )          /* Empty */
	ROM_LOAD( "ic67",  0x0800000, 0x0400000, CRC(c0a2ce5b) SHA1(94b024373c7c546c0f4fe9737639f02e9c7ebbdb) ) /* GFX only */
ROM_END

ROM_START( touchgon ) /* REF 950906, no plug-in daughterboard, Non North America Notice */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "tg56.bin", 0x000000, 0x080000, CRC(fd3b4642) SHA1(3cab42aecad5ee641711763c6047b56784c2bcf3) )
	ROM_LOAD16_BYTE( "tg57.bin", 0x000001, 0x080000, CRC(ee891835) SHA1(9f8c60e5e3696b70f756c3521e10313005053cc7) )

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_LOAD( "ic69",  0x1000000, 0x0200000, CRC(18bb12d4) SHA1(ee6e7a63b86c56d71e62db0ae5892ab3ab94b0a0) ) /* GFX only */

	ROM_REGION( 0x0c00000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "ic65",  0x0000000, 0x0400000, CRC(91b89c7c) SHA1(1c24b494b56845b0f21be40ab737f251d7683c7d) ) /* GFX only */
	ROM_LOAD( "ic66",  0x0400000, 0x0200000, CRC(52682953) SHA1(82cde061bdd827ed4a47a9a4256cd0e887ebc29d) ) /* Sound only */
	ROM_FILL(          0x0600000, 0x0200000, 0x00 )          /* Empty */
	ROM_LOAD( "ic67",  0x0800000, 0x0400000, CRC(c0a2ce5b) SHA1(94b024373c7c546c0f4fe9737639f02e9c7ebbdb) ) /* GFX only */
ROM_END

ROM_START( touchgoe ) /* REF: 950510-1 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "tg56", 0x000000, 0x080000, CRC(6d0f5c65) SHA1(00db7a7da3ec1676169aa78fe4f08a7746c3accf) )
	ROM_LOAD16_BYTE( "tg57", 0x000001, 0x080000, CRC(845787b5) SHA1(27c9910cd9f38328326ecb5cd093dfeb6d4f6244) )

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_LOAD( "ic69",  0x1000000, 0x0200000, CRC(18bb12d4) SHA1(ee6e7a63b86c56d71e62db0ae5892ab3ab94b0a0) ) /* GFX only */

	ROM_REGION( 0x0c00000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "ic65",  0x0000000, 0x0400000, CRC(91b89c7c) SHA1(1c24b494b56845b0f21be40ab737f251d7683c7d) ) /* GFX only */
	ROM_LOAD( "ic66",  0x0400000, 0x0200000, CRC(52682953) SHA1(82cde061bdd827ed4a47a9a4256cd0e887ebc29d) ) /* Sound only */
	ROM_FILL(          0x0600000, 0x0200000, 0x00 )          /* Empty */
	ROM_LOAD( "ic67",  0x0800000, 0x0400000, CRC(c0a2ce5b) SHA1(94b024373c7c546c0f4fe9737639f02e9c7ebbdb) ) /* GFX only */
ROM_END


/*============================================================================
                            SNOW BOARD
  ============================================================================*/

static ADDRESS_MAP_START( snowboar_map, AS_PROGRAM, 16, gaelco2_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                                                         /* ROM */
	AM_RANGE(0x202890, 0x2028ff) AM_DEVREADWRITE("gaelco", gaelco_cg1v_device, gaelcosnd_r, gaelcosnd_w)        /* Sound Registers */
	AM_RANGE(0x200000, 0x20ffff) AM_RAM_WRITE(gaelco2_vram_w) AM_SHARE("spriteram")                             /* Video RAM */
	AM_RANGE(0x210000, 0x211fff) AM_RAM_WRITE(gaelco2_palette_w) AM_SHARE("paletteram")                         /* Palette */
	AM_RANGE(0x212000, 0x213fff) AM_RAM                                                                         /* Extra RAM */
	AM_RANGE(0x218004, 0x218009) AM_RAM AM_SHARE("vregs")                                                       /* Video Registers */
	AM_RANGE(0x300000, 0x300001) AM_READ_PORT("P1")
	AM_RANGE(0x300000, 0x300003) AM_WRITE(gaelco2_coin2_w)                                                      /* Coin Counters */
	AM_RANGE(0x300008, 0x300009) AM_WRITE(gaelco2_eeprom_data_w)                                                /* EEPROM data */
	AM_RANGE(0x30000a, 0x30000b) AM_WRITE(gaelco2_eeprom_sk_w)                                                  /* EEPROM serial clock */
	AM_RANGE(0x30000c, 0x30000d) AM_WRITE(gaelco2_eeprom_cs_w)                                                  /* EEPROM chip select */
	AM_RANGE(0x300010, 0x300011) AM_READ_PORT("P2")
	AM_RANGE(0x300020, 0x300021) AM_READ_PORT("COIN")
	AM_RANGE(0x310000, 0x31ffff) AM_READWRITE(snowboar_protection_r,snowboar_protection_w) AM_SHARE("snowboar_prot")    /* Protection */
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM                                                                                 /* Work RAM */
ADDRESS_MAP_END


static INPUT_PORTS_START( snowboar )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   /* go to service mode NOW */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)   /* bit 6 is EEPROM data (DOUT) */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END

static MACHINE_CONFIG_START( snowboar, gaelco2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 30000000/2)         /* 15 MHz */
	MCFG_CPU_PROGRAM_MAP(snowboar_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gaelco2_state,  irq6_line_hold)

	MCFG_EEPROM_SERIAL_93C66_ADD("eeprom")

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 384-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(gaelco2_state, screen_update_gaelco2)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 0x0400000)
	MCFG_PALETTE_ADD("palette", 4096*16 - 16)   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_DEVICE_ADD("gaelco", GAELCO_CG1V, 0)
	MCFG_GAELCO_SND_DATA("gfx1")
	MCFG_GAELCO_BANKS(0 * 0x0400000, 1 * 0x0400000, 0, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


/*
PCB Layout:

REF: 960419/1
------------------------------------------------------------------------------
|                                   KM428C256J-6 (x2)                        |
|                                                                            |
|                POT1                                                        |
|---                                                                         |
   |         SW1                                                   IC43      |
   |                   34.000MHz          |----------|                       |
|---        93C66                         |          |             IC44      |
|                                         |  CG-1V   |                       |
| J                            6264       |   366    |             IC45      |
|                              6264       |          |                       |
| A                                       |----------|             IC46      |
|                                                                            |
| M                                                                          |
|                                                                            |
| M                         62256                                    62256   |
|                                                                    62256   |
| A                         62256                                    62256   |
|                                            |----------|                    |
|                                30.000MHz   | Lattice  |                    |
|---                                         | IspLSI   |                    |
   |                                         |   1016   |          SB53      |
   |                                         |----------|                    |
|---                                        |------------|           62256   |
|                                           |            |                   |
|                                           |  MC68HC000 |         SB55      |
|                                           |    FN16    |                   |
|                                           |------------|                   |
-----------------------------------------------------------------------------|
*/

ROM_START( snowboara )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "sb53", 0x000000, 0x080000, CRC(e4eaefd4) SHA1(c7de2ae3a4a919fbe16d4997e3f9e2303b8c96b1) ) /* Version 2.0 program roms */
	ROM_LOAD16_BYTE(    "sb55", 0x000001, 0x080000, CRC(e2476994) SHA1(2ad18652a1fc6ac058c8399373fb77e7a81d5bbd) ) /* Version 2.0 program roms */

	ROM_REGION( 0x1400000, "gfx1", 0 )  /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_LOAD( "sb43",       0x1000000, 0x0200000, CRC(afce54ed) SHA1(1d2933d64790612918adbaabcd2a82dad79953c9) )    /* GFX only */
	ROM_FILL(               0x1200000, 0x0200000, 0x00 )         /* Empty */

	ROM_REGION( 0x0c00000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "sb44",       0x0000000, 0x0400000, CRC(1bbe88bc) SHA1(15bce9ada2b742ba4d537fa8efc0f29f661bff00) )    /* GFX only */
	ROM_LOAD( "sb45",       0x0400000, 0x0400000, CRC(373983d9) SHA1(05e35a8b27cab469885f0ec2a5df200a366b50a1) )    /* Sound only */
	ROM_LOAD( "sb46",       0x0800000, 0x0400000, CRC(22e7c648) SHA1(baddb9bc13accd83bea61533d7286cf61cd89279) )    /* GFX only */

	DISK_REGION( "decrypt" )
	DISK_IMAGE( "snowboar", 0, SHA1(fecf611bd9289d24a0b1cabaaf030e2cee322cfa) )
ROM_END

ROM_START( snowboar )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "sb.53",    0x000000, 0x080000, CRC(4742749e) SHA1(933e39893ab74895ae4a99a932f8245a03ea0b5d) ) /* Version 2.1 program roms */
	ROM_LOAD16_BYTE(    "sb.55",    0x000001, 0x080000, CRC(6ddc431f) SHA1(8801c0cf1711bb956447ba1e631db28bd075caea) ) /* Version 2.1 program roms */

	ROM_REGION( 0x1400000, "gfx1", 0 )  /* GFX + Sound */
	ROM_LOAD( "sb.a0",      0x0000000, 0x0080000, CRC(aa476e44) SHA1(2b87689489b9619e9e5ca32c3e3d2aec8ef31c88) )    /* GFX only */
	ROM_LOAD( "sb.a1",      0x0080000, 0x0080000, CRC(6bc99195) SHA1(276e9383fac9cb5141b23ffdf381b0d7e60a6861) )    /* GFX only */
	ROM_LOAD( "sb.a2",      0x0100000, 0x0080000, CRC(fae2ebba) SHA1(653a12846abe4de36f5565c3bf849fce7c2893b6) )    /* GFX only */
	ROM_LOAD( "sb.a3",      0x0180000, 0x0080000, CRC(17ed9cf8) SHA1(c6cab61bbba3b2b1d06b64a68313946299205cc5) )    /* GFX only */
	ROM_LOAD( "sb.a4",      0x0200000, 0x0080000, CRC(2ba3a5c8) SHA1(93de0382cbb41806ae3349ce7cfecdc1404bfb88) )    /* Sound only */
	ROM_LOAD( "sb.a5",      0x0280000, 0x0080000, CRC(ae011eb3) SHA1(17223404640c55637364fa6e51cf07d8e64df085) )    /* Sound only */
	ROM_FILL(               0x0300000, 0x0100000, 0x00 )         /* Empty */
	ROM_LOAD( "sb.b0",      0x0400000, 0x0080000, CRC(96c714cd) SHA1(c6225c43b88531a70436cc8a631b8ba401903e45) )  /* GFX only */
	ROM_LOAD( "sb.b1",      0x0480000, 0x0080000, CRC(39a4c30c) SHA1(4598a68ef41483ba372aa3a40383de8eb70d706e) )    /* GFX only */
	ROM_LOAD( "sb.b2",      0x0500000, 0x0080000, CRC(b58fcdd6) SHA1(21a8c00778be77165f89421fb2e3123244cf02c6) )    /* GFX only */
	ROM_LOAD( "sb.b3",      0x0580000, 0x0080000, CRC(96afdebf) SHA1(880cfb365efa93bbee882aeb483ad6d75d8b7430) )    /* GFX only */
	ROM_LOAD( "sb.b4",      0x0600000, 0x0080000, CRC(e62cf8df) SHA1(8df8df45d99967e52dcec5b589246799f7a39601) )    /* Sound only */
	ROM_LOAD( "sb.b5",      0x0680000, 0x0080000, CRC(caa90856) SHA1(a8f18a878b211366faaf66911c09d0452770cc3f) )    /* Sound only */
	ROM_FILL(               0x0700000, 0x0100000, 0x00 )         /* Empty */
	ROM_LOAD( "sb.c0",      0x0800000, 0x0080000, CRC(c9d57a71) SHA1(4e8b7d821e31afc0750db283470f9c76bceb54da) )    /* GFX only */
	ROM_LOAD( "sb.c1",      0x0880000, 0x0080000, CRC(1d14a3d4) SHA1(eb89cadfe331f77dbc0463151574ba801c248238) )    /* GFX only */
	ROM_LOAD( "sb.c2",      0x0900000, 0x0080000, CRC(55026352) SHA1(7b92f45624dbd122c29e44f82c3c2ffded190efa) )    /* GFX only */
	ROM_LOAD( "sb.c3",      0x0980000, 0x0080000, CRC(d9b62dee) SHA1(409ab4d9a6f9341cf59510c130c705d1ec42d1b3) )    /* GFX only */
	ROM_FILL(               0x0a00000, 0x0200000, 0x00 )         /* Empty */
	ROM_LOAD( "sb.d0",      0x0c00000, 0x0080000, CRC(7434c1ae) SHA1(8e0e6567a461c694a8ba2de5d4cf9ad73e0c83c8) )    /* GFX only */
	ROM_LOAD( "sb.d1",      0x0c80000, 0x0080000, CRC(f00cc6c8) SHA1(b4835e2187e1a985993471d09495cbc1f5cd9417) )    /* GFX only */
	ROM_LOAD( "sb.d2",      0x0d00000, 0x0080000, CRC(019f9aec) SHA1(1a97b84ebbf57e860792ef7a7dc6f51553ae3e26) )    /* GFX only */
	ROM_LOAD( "sb.d3",      0x0d80000, 0x0080000, CRC(d05bd286) SHA1(9eff6f5a4755375b7a16b9d4967a0df933e1b9c4) )    /* GFX only */
	ROM_FILL(               0x0e00000, 0x0200000, 0x00 )         /* Empty */
	ROM_LOAD( "sb.e0",      0x1000000, 0x0080000, CRC(e6195323) SHA1(5ba1cb750dd8cfd0721905174bda6cfbf8c8e694) )    /* GFX only */
	ROM_LOAD( "sb.e1",      0x1080000, 0x0080000, CRC(9f38910b) SHA1(0243c19c7b1bdd3361fc6e177c64528bacafcc33) )    /* GFX only */
	ROM_LOAD( "sb.e2",      0x1100000, 0x0080000, CRC(f5948c6c) SHA1(91bba817ced194b02885ce84b7a8132ef5ca631a) )    /* GFX only */
	ROM_LOAD( "sb.e3",      0x1180000, 0x0080000, CRC(4baa678f) SHA1(a7fbbd687e2d8d7e96207c8ace0799a3cc9c3272) )    /* GFX only */
	ROM_FILL(               0x1200000, 0x0200000, 0x00 )         /* Empty */

	DISK_REGION( "decrypt" )
	DISK_IMAGE( "snowboar", 0, SHA1(fecf611bd9289d24a0b1cabaaf030e2cee322cfa) )
ROM_END



/*============================================================================
                            WORLD RALLY 2
  ============================================================================*/

static ADDRESS_MAP_START( wrally2_map, AS_PROGRAM, 16, wrally2_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                                                     /* ROM */
	AM_RANGE(0x202890, 0x2028ff) AM_DEVREADWRITE("gaelco", gaelco_gae1_device, gaelcosnd_r, gaelcosnd_w)    /* Sound Registers */
	AM_RANGE(0x200000, 0x20ffff) AM_RAM_WRITE(gaelco2_vram_w) AM_SHARE("spriteram")                         /* Video RAM */
	AM_RANGE(0x210000, 0x211fff) AM_RAM_WRITE(gaelco2_palette_w) AM_SHARE("paletteram")                     /* Palette */
	AM_RANGE(0x212000, 0x213fff) AM_RAM                                                                     /* Extra RAM */
	AM_RANGE(0x218004, 0x218009) AM_RAM AM_SHARE("vregs")                                                   /* Video Registers */
	AM_RANGE(0x300000, 0x300001) AM_READ_PORT("IN0")                                                        /* DIPSW #2 + Inputs 1P */
	AM_RANGE(0x300002, 0x300003) AM_READ_PORT("IN1")                                                        /* DIPSW #1 */
	AM_RANGE(0x300004, 0x300005) AM_READ_PORT("IN2")                                                        /* Inputs 2P + COINSW */
	AM_RANGE(0x300006, 0x300007) AM_READ_PORT("IN3")                                                        /* SERVICESW */
	AM_RANGE(0x400000, 0x400011) AM_WRITE(wrally2_coin_w)                                                   /* Coin Counters */
	AM_RANGE(0x400028, 0x400029) AM_WRITE(wrally2_adc_clk)                                                  /* ADCs clock-in line */
	AM_RANGE(0x400030, 0x400031) AM_WRITE(wrally2_adc_cs)                                                   /* ADCs chip select line */
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM                                                                     /* Work RAM */
ADDRESS_MAP_END


static INPUT_PORTS_START( wrally2 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Acc.")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Gear") PORT_TOGGLE
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, wrally2_state,wrally2_analog_bit_r, (void *)0x00)   /* ADC_1 serial input */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPNAME( 0x0200, 0x0000, "Coin mechanism" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, "Common" )
	PORT_DIPSETTING(      0x0200, "Independent" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Cabinet 1 Control Configuration" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, "Pot Wheel" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Cabinet 2 Control Configuration" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, "Pot Wheel" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Monitors" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, "One" )
	PORT_DIPSETTING(      0x2000, "Two" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x4000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )

	PORT_START("IN1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Credit configuration" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, "Start 2C/Continue 1C" )
	PORT_DIPSETTING(      0x0200, "Start 1C/Continue 1C" )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x1800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Acc.")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Gear") PORT_TOGGLE
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, wrally2_state,wrally2_analog_bit_r, (void *)1)   /* ADC_2 serial input */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfa00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE3 ) /* go to test mode NOW */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("FAKE")  /* Fake: To switch between monitors at run time */
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SERVICE4 ) PORT_TOGGLE

	PORT_START("ANALOG0")   /* steering wheel player 1 */
	PORT_BIT( 0xff, 0x8A, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(25) PORT_REVERSE PORT_NAME("P1 Wheel")

	PORT_START("ANALOG1")   /* steering wheel player 2 */
	PORT_BIT( 0xff, 0x8A, IPT_PADDLE_V ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(25) PORT_REVERSE PORT_NAME("P2 Wheel")
INPUT_PORTS_END

static MACHINE_CONFIG_START( wrally2, wrally2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 26000000/2)         /* 13 MHz */
	MCFG_CPU_PROGRAM_MAP(wrally2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", gaelco2_state,  irq6_line_hold)

	MCFG_EEPROM_SERIAL_93C66_ADD("eeprom")

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 0x0200000)
	MCFG_PALETTE_ADD("palette", 4096*16 - 16)   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */
	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(384, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 384-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(gaelco2_state, screen_update_gaelco2_left)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(384, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 384-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(gaelco2_state, screen_update_gaelco2_right)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")


	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2_dual)

	/* sound hardware */
	/* the chip is stereo, but the game sound is mono because the right channel
	   output is for cabinet 1 and the left channel output is for cabinet 2 */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_DEVICE_ADD("gaelco", GAELCO_GAE1, 0)
	MCFG_GAELCO_SND_DATA("gfx1")
	MCFG_GAELCO_BANKS(0 * 0x0200000, 1 * 0x0200000, 0, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

/*
PCB Layout:

REF: 950510
------------------------------------------------------------------------------
|                POT1                        KM428C256J-6 (x4)               |
|                POT2                                                        |
|                                            ----------------------------    |
|                                            | (Plug-In Daughterboard)  |    |
|                                            | WR2.1   WR2.9    WR2.16  |    |
|---                                         | WR2.2   WR2.10   WR2.17  |    |
   |                                         |         WR2.11   WR2.18  |    |
   |                                         |         WR2.12   WR2.19  |    |
|---                                         |         WR2.13   WR2.20  |    |
|                                            |         WR2.14   WR2.21  |    |
|                                            ----------------------------    |
|                                                                            |
|                                            |----------|                    |
| J                                          |          |                    |
|                                            | GAE1 449 |                    |
| A                               6264       | (QFP208) |                    |
|                                 6264       |          |                    |
| M                                          |----------|                    |
|                                                                            |
| M                       |-------------------------|                        |
|                         |                         |  34.000MHz     62256   |
| A                       |  62256  DS5002  BATT_3V |                62256   |
|                         |                         |                        |
|                         |-------------------------|                        |
|                                                                            |
|---                                    62256                                |
   |                                    62256                                |
   |  DSW1                                                                   |
|---  DSW2                                                                   |
|                                                                            |
|                                26.000MHz      MC68000P12      WR2.63       |
| CONN1                                                         WR2.64       |
|                                                                            |
| CONN2    CONN3                                                             |
-----------------------------------------------------------------------------|


Notes
-----
All ROMs are type 27C040
CONN1: RGBSync OUT (additional to JAMMA RGBSync)
CONN2: Right speaker sound OUT (for second cabinat)
CONN3: For connection of wheel etc
POT1/2: Volume adjust of left/right channel

PCB Layout:

REF: 950510-1

------------------------------------------------------------------------------
 |         POT1              TI F20LB         KM428C256J-6 (x4)               |
 |         POT2                                                               |
 |                                                                            |
 |                                                                            |
 |                                              PROM IC68                     |
 |---                                           PROM IC69                     |
    |                                           PROM IC70                     |
    |                                                                         |
 |---                                                                         |
 |                                                                            |
 |                                                                            |
 |                                                                            |
 |                                            |----------|                    |
 | J                                          |          |                    |
 |                                            | GAE1 506 |                    |
 | A                              65764       | (QFP160) |                    |
 |                                65764       |          |                    |
 | M                                          |----------|                    |
 |                                                                            |
 | M                       |-------------------------|                        |
 |                         |                         |  34.000MHz     62256   |
 | A                       |  62256  DS5002  BATT_3V |                62256   |
 |                         |                         |                        |
 |                         |-------------------------|                        |
 |    TLC569   TLC569                                                         |
 |---                                    62256                                |
    |                                    62256                                |
    |  DSW1                                                                   |
 |---  DSW2                                                                   |
 |                                                                            |
 |                                26.000MHz      MC68000P12      WR2.63       |
 | CONN1                                                         WR2.64       |
 |                                                                            |
 | CONN2    CONN3                                                             |
 -----------------------------------------------------------------------------|


Notes
-----
Gaelco's PROMs IC70 and IC69 has DIP42 package (gfx rom)
Gaelco's PROM IC68 has DIP32 package (sound rom)
TI F20L8 is a Texas Ins. DIP24 (may be a PAL). Is marked as F 406 XF 21869 F20L8-25CNT
TLC569 (IC2 and IC7) is a 8-bit serial ADC

*/

ROM_START( wrally2 )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "wr2.64",  0x000000, 0x080000, CRC(4cdf4e1e) SHA1(a3b3ff4a70336b61c7bba5d518527bf4bd901867) )
	ROM_LOAD16_BYTE( "wr2.63",  0x000001, 0x080000, CRC(94887c9f) SHA1(ad09f1fbeff4c3ba47f72346d261b22fa6a51457) )

	ROM_REGION( 0x0a00000, "gfx1", 0 )  /* GFX + Sound */
	ROM_LOAD( "wr2.16d",    0x0000000, 0x0080000, CRC(ad26086b) SHA1(487ffaaca57c9d030fc486b8cae6735ee40a0ac3) )    /* GFX only */
	ROM_LOAD( "wr2.17d",    0x0080000, 0x0080000, CRC(c1ec0745) SHA1(a6c3ce9c889e6a53f4155f54d6655825af34a35b) )    /* GFX only */
	ROM_LOAD( "wr2.18d",    0x0100000, 0x0080000, CRC(e3617814) SHA1(9f9514052bb07d7e243f33b11bae409a444b7d9f) )    /* Sound only */
	ROM_LOAD( "wr2.19d",    0x0180000, 0x0080000, CRC(2dae988c) SHA1(a585e10b0e1519b828738b0b90698f8600082250) )    /* Sound only */
	ROM_LOAD( "wr2.09d",    0x0200000, 0x0080000, CRC(372d70c8) SHA1(a6d8419765eab1fa20c6d3ddff9d026adaab5cd9) )    /* GFX only */
	ROM_LOAD( "wr2.10d",    0x0280000, 0x0080000, CRC(5db67eb3) SHA1(faa58dafa26befb3291e5185ee04c39ce3b45b3f) )    /* GFX only */
	ROM_LOAD( "wr2.11d",    0x0300000, 0x0080000, CRC(ae66b97c) SHA1(bd0eba0b1c77864e06a9e136cfd834b35f200683) )    /* Sound only */
	ROM_LOAD( "wr2.12d",    0x0380000, 0x0080000, CRC(6dbdaa95) SHA1(f23df65e3df92d79f7b1e99d611c067a79fc849a) )    /* Sound only */
	ROM_LOAD( "wr2.01d",    0x0400000, 0x0080000, CRC(753a138d) SHA1(b05348af6d25e95208fc39007eb2082b759384e8) )    /* GFX only */
	ROM_LOAD( "wr2.02d",    0x0480000, 0x0080000, CRC(9c2a723c) SHA1(5259c8fa1ad73518e89a8df6e76a565b8f8799e3) )    /* GFX only */
	ROM_FILL(               0x0500000, 0x0100000, 0x00 )         /* Empty */
	ROM_LOAD( "wr2.20d",    0x0600000, 0x0080000, CRC(4f7ade84) SHA1(c8efcd4bcb1f2ad6ab8104ec0daea8324cefd3fd) )    /* GFX only */
	ROM_LOAD( "wr2.13d",    0x0680000, 0x0080000, CRC(a4cd32f8) SHA1(bc4cc73b7a58aecd735bf55bb5062baa6dd22f83) )    /* GFX only */
	ROM_FILL(               0x0700000, 0x0100000, 0x00 )         /* Empty */
	ROM_LOAD( "wr2.21d",    0x0800000, 0x0080000, CRC(899b0583) SHA1(a313e679980cc4da22bc70f2c7c9685af4f3d6df) )    /* GFX only */
	ROM_LOAD( "wr2.14d",    0x0880000, 0x0080000, CRC(6eb781d5) SHA1(d5c13db88e6de606b34805391cef9f3fbf09fac4) )    /* GFX only */
	ROM_FILL(               0x0900000, 0x0100000, 0x00 )         /* Empty */
ROM_END

/*
CPU 1x MC68HC000FN12 (main)(u18)
1x CG-1V-149 (u42)(sound/gfx? maybe the same as Gaelco)
1x DALLAS DS5002FP (not dumped)(u44)
1x TDA1543 (sound)(u20)
1x LM358N (sound)(u7)
1x TDA2003 (sound)(u1)
1x oscillator 34.000 MHz (xtal1)
1x oscillator 11.0592 MHz (xtal2)
ROMs    2x AM27C010 (u39,u40)
4x NM27C040Q (u50,u52,u53,u54)
1x M27C801 (u51)

6x RAM CY7C199 (u22,u23,u26,u27,u55,u56)
2x RAM KM428C256TR (u37,u41)
1x RAM GM76C256CLLFW70 (u47)(close to Dallas)

1x PALCE16V8H (u28)(read protected -> extracted with CmD's PALdumper - it's registered)
1x PALCE16V8H (u29)(read protected -> extracted with CmD's PALdumper)
Note    1x 28x2 edge connector
1x 2 legs connector (jp1)
1x 4 legs connector (jp2)
1x 12 legs connector (jp3)
1x 5 legs connector (jp4)
1x RS232 connector
1x trimmer (volume)
1x battery 3V (bt1)

Title is uncertain. A string at 27e00 says: "Play 2000 v5.01 (c) 1999", but
there are also some gfxs that says "Gran Tesoro" all over the place.
I don't know what's the correct title for this one...

*/

ROM_START( grtesoro )
	/*at least 1.u40 is bad, on every 0x40 bytes the first four are always 0xff.*/
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "2.u39",   0x000000, 0x020000, BAD_DUMP CRC(9939299e) SHA1(55303a2adf199f4b5a60f57be7480b0e119f8624) )
	ROM_LOAD16_BYTE( "1.u40",   0x000001, 0x020000, BAD_DUMP CRC(311c2f94) SHA1(963d6b5f479598145146fcb8b7c6ce77fbc92b07) )

	ROM_REGION( 0x0300000, "gfx1", 0 ) /* GFX + Sound */
	ROM_LOAD( "3.u54", 0x0000000, 0x0080000, CRC(085008ed) SHA1(06eb4f972d79eab13b1b3b6829ef280e079abdb6) )
	ROM_LOAD( "4.u53", 0x0080000, 0x0080000, CRC(94dc37a7) SHA1(28f9832b61541b292682a6e2d2264abccd138a2e) )
	ROM_LOAD( "5.u52", 0x0100000, 0x0080000, CRC(19b939f4) SHA1(7281709aa3ab1decb84bf7ab10492fb6ec197c80) )
	ROM_LOAD( "6.u51", 0x0180000, 0x0100000, CRC(6dafc11c) SHA1(2aa3d6318418578433b3060bda6e27adf794dea4) )
	ROM_LOAD( "7.u50", 0x0280000, 0x0080000, CRC(e80c6d39) SHA1(b3ae5d66c48c2ba6665a181e311b0c834384258a) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h.u29",  0x0000, 0x0117, BAD_DUMP CRC(4a0a6f39) SHA1(57351e471649391c9abf110828fe2f128fe84eee) )
ROM_END

ROM_START( grtesoro4 ) /* there are version 4.0 and version 1.0 strings in this, go with the higher one */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "2.u39_v4",    0x000000, 0x020000, CRC(fff16141) SHA1(8493c3e58a231c03b152b336f43422a9a2d2618c) )
	ROM_LOAD16_BYTE( "1.u40_v4",    0x000001, 0x020000, CRC(39f9d58e) SHA1(1cbdae2adc570f2a2e10a707075312ef717e2643) )

	ROM_REGION( 0x0300000, "gfx1", 0 ) /* GFX + Sound */
	ROM_LOAD( "3.u54", 0x0000000, 0x0080000, CRC(085008ed) SHA1(06eb4f972d79eab13b1b3b6829ef280e079abdb6) )
	ROM_LOAD( "4.u53", 0x0080000, 0x0080000, CRC(94dc37a7) SHA1(28f9832b61541b292682a6e2d2264abccd138a2e) )
	ROM_LOAD( "5.u52", 0x0100000, 0x0080000, CRC(19b939f4) SHA1(7281709aa3ab1decb84bf7ab10492fb6ec197c80) )
	ROM_LOAD( "6.u51", 0x0180000, 0x0100000, CRC(6dafc11c) SHA1(2aa3d6318418578433b3060bda6e27adf794dea4) )
	ROM_LOAD( "7.u50", 0x0280000, 0x0080000, CRC(e80c6d39) SHA1(b3ae5d66c48c2ba6665a181e311b0c834384258a) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h.u29",  0x0000, 0x0117, BAD_DUMP CRC(4a0a6f39) SHA1(57351e471649391c9abf110828fe2f128fe84eee) )
ROM_END

GAME( 1994, aligator,  0,       alighunt, alighunt, gaelco2_state, alighunt, ROT0, "Gaelco", "Alligator Hunt", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1994, aligatorun,aligator,alighunt, alighunt, gaelco2_state, alighunt, ROT0, "Gaelco", "Alligator Hunt (unprotected)", 0 )
GAME( 1995, touchgo,  0,        touchgo,  touchgo,  gaelco2_state, touchgo,  ROT0, "Gaelco", "Touch & Go (World)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1995, touchgon, touchgo,  touchgo,  touchgo,  gaelco2_state, touchgo,  ROT0, "Gaelco", "Touch & Go (Non North America)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1995, touchgoe, touchgo,  touchgo,  touchgo,  gaelco2_state, touchgo,  ROT0, "Gaelco", "Touch & Go (earlier revision)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1995, wrally2,  0,        wrally2,  wrally2,  driver_device, 0,        ROT0, "Gaelco", "World Rally 2: Twin Racing", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1996, maniacsq, 0,        maniacsq, maniacsq, driver_device, 0,        ROT0, "Gaelco", "Maniac Square (unprotected)", 0 )
GAME( 1996, snowboar, 0,        snowboar, snowboar, driver_device, 0,        ROT0, "Gaelco", "Snow Board Championship (Version 2.1)", 0 )
GAME( 1996, snowboara,snowboar, snowboar, snowboar, gaelco2_state, snowboar, ROT0, "Gaelco", "Snow Board Championship (Version 2.0)", 0 )
GAME( 1998, bang,     0,        bang,     bang,     bang_state,    bang,     ROT0, "Gaelco", "Bang!", 0 )
GAME( 1998, bangj,    bang,     bang,     bang,     bang_state,    bang,     ROT0, "Gaelco", "Gun Gabacho (Japan)", 0 )
// are these ACTUALLY Gaelco hardware, or do they just use the same Dallas?
GAME( 1999, grtesoro,  0,       maniacsq, maniacsq, driver_device, 0,        ROT0, "Nova Desitec", "Gran Tesoro? / Play 2000 (v5.01) (Italy)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1999, grtesoro4, grtesoro,maniacsq, maniacsq, driver_device, 0,        ROT0, "Nova Desitec", "Gran Tesoro? / Play 2000 (v4.0) (Italy)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
