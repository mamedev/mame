// license:???
// copyright-holders:Lee Taylor,John Clegg,Tomasz Slanina
/****************************************************************************

Traverse USA / Zippy Race  (c) 1983 Irem
Shot Rider                 (c) 1984 Seibu Kaihatsu / Sigma

driver by
Lee Taylor
John Clegg
Tomasz Slanina  dox@space.pl

Notes:
- I haven't understood how char/sprite priority works. This is used for
  tunnels. I hacked it just by making the two needed colors opaque. They
  don't seem to be used anywhere else. Even if it looks like a hack, it might
  really be how the hardware works - see also notes regarding Kung Fu Master
  at the beginning of m62.c.
- shtrider has some weird colors (pink cars, green "Seibu" truck, 'no'
  on hiscore table) but there's no reason to think there's something wrong
  with the driver.

TODO:
- shtrider dip switches and figure out flip screen (not same as travrusa)

****************************************************************************

Shot Rider
PCB layout:

--------------------CCCCCCCCCCC------------------

  P P              OKI                       2764
 * 2764 2764 2764      AY  AY  x 2764  2764  2764

 E           D  D                            P P

                      2764 2764 2764
--------------------------------------------------
  C  - conector
  P  - proms
  E  - connector for  big black epoxy block
  D  - dips
  *  - empty rom socket
  x  - 40 pin chip, surface scratched (6803)

Epoxy block contains main cpu (Z80)
and 2764 eprom (swapped D3/D4 and D5/D6 data lines)

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "audio/irem.h"
#include "includes/travrusa.h"


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, travrusa_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM_WRITE(travrusa_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9000, 0x9000) AM_WRITE(travrusa_scroll_x_low_w)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(travrusa_scroll_x_high_w)
	AM_RANGE(0xc800, 0xc9ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xd000, 0xd000) AM_DEVWRITE("irem_audio", irem_audio_device, cmd_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITE(travrusa_flipscreen_w)    /* + coin counters - not written by shtrider */
	AM_RANGE(0xd000, 0xd000) AM_READ_PORT("SYSTEM")     /* IN0 */
	AM_RANGE(0xd001, 0xd001) AM_READ_PORT("P1")         /* IN1 */
	AM_RANGE(0xd002, 0xd002) AM_READ_PORT("P2")         /* IN2 */
	AM_RANGE(0xd003, 0xd003) AM_READ_PORT("DSW1")       /* DSW1 */
	AM_RANGE(0xd004, 0xd004) AM_READ_PORT("DSW2")       /* DSW2 */
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( travrusa )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	/* coin input must be active for 19 frames to be consistently recognized */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(19)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Fuel Reduced on Collision" ) PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x02, "Med" )
	PORT_DIPSETTING(    0x01, "Hi" )
	PORT_DIPSETTING(    0x00, "Max" )
	PORT_DIPNAME( 0x04, 0x04, "Fuel Consumption" )      PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, "Hi" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW1:5,6,7,8")
	PORT_DIPSETTING(    0x80, "Not Used" )          PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x90, "Not Used" )          PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0xa0, DEF_STR( 6C_1C ) )        PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )        PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_1C ) )        PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0xd0, DEF_STR( 3C_1C ) )        PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_3C ) )        PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_4C ) )        PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )        PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_6C ) )        PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_7C ) )        PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x10, "Not Used" )          PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )    PORT_CONDITION("DSW2", 0x04, EQUALS, 0x04)

	PORT_DIPSETTING(    0x80, DEF_STR( Free_Play ) )    PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x90, "A 3C_1C / B 1C_3C" )     PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0xa0, "A 2C_1C / B 1C_3C" )     PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0xb0, "A 1C_1C / B 1C_3C" )     PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )    PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0xd0, "A 3C_1C / B 1C_2C" )     PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0xe0, "A 2C_1C / B 1C_2C" )     PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0xf0, "A 1C_1C / B 1C_2C" )     PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x70, "A 1C_1C / B 1C_5C" )     PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x60, "A 2C_1C / B 1C_5C" )     PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x50, "A 3C_1C / B 1C_5C" )     PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( Free_Play ) )    PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x30, "A 1C_1C / B 1C_6C" )     PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x20, "A 2C_1C / B 1C_6C" )     PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, "A 3C_1C / B 1C_6C" )     PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )    PORT_CONDITION("DSW2", 0x04, EQUALS, 0x00)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Mode" )         PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x08, 0x08, "Speed Type" )        PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, "M/H" ) //mph ?
	PORT_DIPSETTING(    0x00, "Km/H" )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x10, 0x10, "Stop Mode (Cheat)")      PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Title" )         PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, "Traverse USA" )
	PORT_DIPSETTING(    0x00, "Zippy Race" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")    PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, 0x80, "DSW2:8")
INPUT_PORTS_END

/* same as travrusa, no "Title" switch */
static INPUT_PORTS_START( motorace )
	PORT_INCLUDE( travrusa )

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "DSW2:6")
INPUT_PORTS_END

static INPUT_PORTS_START( shtrider )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("P2")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW1:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Speed Display" )     PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x02, "km/h" )
	PORT_DIPSETTING(    0x00, "mph" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW2:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW2:8" )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout shtrider_spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( travrusa )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,      0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16*8, 16 )
GFXDECODE_END

static GFXDECODE_START( shtrider )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,               0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, shtrider_spritelayout, 16*8, 16 )
GFXDECODE_END

void travrusa_state::machine_reset()
{
	m_scrollx[0] = 0;
	m_scrollx[1] = 0;
}

static MACHINE_CONFIG_START( travrusa, travrusa_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)   /* 4 MHz (?) */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", travrusa_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(56.75)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1790)   /* accurate frequency, measured on a Moon Patrol board, is 56.75Hz. */)
				/* the Lode Runner manual (similar but different hardware) */
				/* talks about 55Hz and 1790ms vblank duration. */
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(travrusa_state, screen_update_travrusa)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", travrusa)

	MCFG_PALETTE_ADD("palette", 16*8+16*8)
	MCFG_PALETTE_INDIRECT_ENTRIES(128+16)
	MCFG_PALETTE_INIT_OWNER(travrusa_state, travrusa)

	/* sound hardware */
	//MCFG_FRAGMENT_ADD(m52_sound_c_audio)
	MCFG_DEVICE_ADD("irem_audio", IREM_M52_SOUNDC_AUDIO, 0)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( shtrider, travrusa )

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", shtrider)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(travrusa_state,shtrider)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( shtriderb, travrusa )

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", shtrider)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START( travrusa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "zr1-0.m3",     0x0000, 0x2000, CRC(be066c0a) SHA1(fed0ef114b08519b99d77485b73768a838d2f06e) )
	ROM_LOAD( "zr1-5.l3",     0x2000, 0x2000, CRC(145d6b34) SHA1(c9e2bd1d3e62c496e4c5057c4012b069dfcf592d) )
	ROM_LOAD( "zr1-6a.k3",    0x4000, 0x2000, CRC(e1b51383) SHA1(34f4476c1bcc28c53c8ffa7b614f443a329aae13) )
	ROM_LOAD( "zr1-7.j3",     0x6000, 0x2000, CRC(85cd1a51) SHA1(7eb046514845cb9d2507ee24d1b2f7cc5402ac02) )

	ROM_REGION( 0x8000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "mr10.1a",      0x7000, 0x1000, CRC(a02ad8a0) SHA1(aff80b506dbecabed2a36eb743693940f6a22d16) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "zippyrac.001", 0x0000, 0x2000, CRC(aa8994dd) SHA1(9b326ce52a03d723e5c3c1b5fd4aa8fa7f70f904) )
	ROM_LOAD( "mr8.3c",       0x2000, 0x2000, CRC(3a046dd1) SHA1(65c1dd1c0b5fb72ac5c04e11a577308245e4b312) )
	ROM_LOAD( "mr9.3a",       0x4000, 0x2000, CRC(1cc3d3f4) SHA1(e7ee365d43d783cb6b7df37c6edeadbed35318d9) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "zr1-8.n3",     0x0000, 0x2000, CRC(3e2c7a6b) SHA1(abc9eeb656ab6ed5f14e10fc988f75f21ccf037a) )
	ROM_LOAD( "zr1-9.l3",     0x2000, 0x2000, CRC(13be6a14) SHA1(47861910fe4c46cd72634cf7d834be2da2a0a4f9) )
	ROM_LOAD( "zr1-10.k3",    0x4000, 0x2000, CRC(6fcc9fdb) SHA1(88f878b9ebf07c5a16f8cb742016cac971ed3f10) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "mmi6349.ij",   0x0000, 0x0200, CRC(c9724350) SHA1(1fac20cdc0a53d94e8f67b49d7dd71d1b9f1f7ef) ) /* character palette - last $100 are unused */
	ROM_LOAD( "tbp18s.2",     0x0200, 0x0020, CRC(a1130007) SHA1(9deb0eed75dd06e86f83c819a3393158be7c9dce) ) /* sprite palette */
	ROM_LOAD( "tbp24s10.3",   0x0220, 0x0100, CRC(76062638) SHA1(7378a26cf455d9d3df90929dc665870514c34b54) ) /* sprite lookup table */
ROM_END

	/* Bootleg - "American Top" printed on title - (c) 1983 I.P. - Zippy Race graphic logo is blanked out - Main ROM0-ROM3 test NG */
ROM_START( travrusab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "at4.m3",       0x0000, 0x2000, CRC(704ce6e4) SHA1(77385d853e3d5085c6ab155417e2b42212aff6fc) )
	ROM_LOAD( "at5.l3",       0x2000, 0x2000, CRC(686cb0e6) SHA1(64c7e682a181bae159cca60ffa617c532b1e16d3) )
	ROM_LOAD( "at6.k3",       0x4000, 0x2000, CRC(baf87d80) SHA1(761d687ef3f3dde80a47f547d3c822704a2ac821) )
	ROM_LOAD( "at7.h3",       0x6000, 0x2000, CRC(48091ebe) SHA1(6146af6f08053a5955d9b388d25bfbab7ad6b0e5) )

	ROM_REGION( 0x8000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "11.a1",        0x7000, 0x1000, CRC(d2c0bc33) SHA1(3a52ae514daf985d297416301dac0ac6cbe671d7) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "zippyrac.001", 0x0000, 0x2000, CRC(aa8994dd) SHA1(9b326ce52a03d723e5c3c1b5fd4aa8fa7f70f904) ) /* at1.e3 */
	ROM_LOAD( "mr8.3c",       0x2000, 0x2000, CRC(3a046dd1) SHA1(65c1dd1c0b5fb72ac5c04e11a577308245e4b312) ) /* at2.c3 */
	ROM_LOAD( "mr9.3a",       0x4000, 0x2000, CRC(1cc3d3f4) SHA1(e7ee365d43d783cb6b7df37c6edeadbed35318d9) ) /* at3.a3 */

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "8.n3",         0x0000, 0x2000, CRC(00c0f46b) SHA1(5fccc188af653785f3fc0f9d36dbbbab472f6fdc) )
	ROM_LOAD( "9.m3",         0x2000, 0x2000, CRC(73ade73b) SHA1(4da012d71e7c1f46407343cc8d4fbe0397b7db71) )
	ROM_LOAD( "10.k3",        0x4000, 0x2000, CRC(fcfeaa69) SHA1(a958caf70d2dc4a80298a395cb48db210e6ca16b) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "mmi6349.ij",   0x0000, 0x0200, CRC(c9724350) SHA1(1fac20cdc0a53d94e8f67b49d7dd71d1b9f1f7ef) ) /* character palette - last $100 are unused */
	ROM_LOAD( "tbp18s.2",     0x0200, 0x0020, CRC(a1130007) SHA1(9deb0eed75dd06e86f83c819a3393158be7c9dce) ) /* sprite palette */
	ROM_LOAD( "tbp24s10.3",   0x0220, 0x0100, CRC(76062638) SHA1(7378a26cf455d9d3df90929dc665870514c34b54) ) /* sprite lookup table */
ROM_END

ROM_START( motorace )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "mr.cpu",       0x0000, 0x2000, CRC(89030b0c) SHA1(dec4209385bbccff4a3c0d93d6507110ef841331) )    /* encrypted */
	ROM_LOAD( "mr1.3l",       0x2000, 0x2000, CRC(0904ed58) SHA1(2776e031cb58f99103bc35299bffd7612d954608) )
	ROM_LOAD( "mr2.3k",       0x4000, 0x2000, CRC(8a2374ec) SHA1(7159731f5ef2485e3c822e3e8e51e9583dd1c6bc) )
	ROM_LOAD( "mr3.3j",       0x6000, 0x2000, CRC(2f04c341) SHA1(ae990d9d4abdd7d6ef9d21aa62125fe2e0067623) )

	ROM_REGION( 0x8000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "mr10.1a",      0x7000, 0x1000, CRC(a02ad8a0) SHA1(aff80b506dbecabed2a36eb743693940f6a22d16) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "mr7.3e",       0x0000, 0x2000, CRC(492a60be) SHA1(9a3d6407b834eb7c3e3c8bb292ff124550a2787c) )
	ROM_LOAD( "mr8.3c",       0x2000, 0x2000, CRC(3a046dd1) SHA1(65c1dd1c0b5fb72ac5c04e11a577308245e4b312) )
	ROM_LOAD( "mr9.3a",       0x4000, 0x2000, CRC(1cc3d3f4) SHA1(e7ee365d43d783cb6b7df37c6edeadbed35318d9) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "mr4.3n",       0x0000, 0x2000, CRC(5cf1a0d6) SHA1(ef0883e71ee1e9c38cf3f444d9d8e79a08076b78) )
	ROM_LOAD( "mr5.3m",       0x2000, 0x2000, CRC(f75f2aad) SHA1(e4a8a3da56cbc04f0c9041afac182d1bfceb1d0d) )
	ROM_LOAD( "mr6.3k",       0x4000, 0x2000, CRC(518889a0) SHA1(70b417104ce86132cb5542813c1e0509b2260756) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "mmi6349.ij",   0x0000, 0x0200, CRC(c9724350) SHA1(1fac20cdc0a53d94e8f67b49d7dd71d1b9f1f7ef) ) /* character palette - last $100 are unused */
	ROM_LOAD( "tbp18s.2",     0x0200, 0x0020, CRC(a1130007) SHA1(9deb0eed75dd06e86f83c819a3393158be7c9dce) ) /* sprite palette */
	ROM_LOAD( "tbp24s10.3",   0x0220, 0x0100, CRC(76062638) SHA1(7378a26cf455d9d3df90929dc665870514c34b54) ) /* sprite lookup table */
ROM_END

/*

Moto tour is a Tecfri's licensed version of Traverse usa/Zippy Race from Irem

This version don't have de MSM5202 but still having the sounds produced bt 5202 with a lower quality, I guess it converts the sound data to analog in some way, also this version is unprotected, doesn't have the epoxy block.

Unfortunately the eproms labels have dissapeared, so I name it similar to Traverse usa but with the letters mt (Moto Tour)

Rom Info

snd.a1  ------  sound code, 100% identical to Traverse Usa/Zippy race

mt1-1.e3 \
mt1-2.c3  -- Backgrounds?, 100% identical to Traverse Usa/Zippy race
mt1-3.a3 /

mt1-4.m3  \
mt1-5.l3  ==
mt1-6.k3  ==  Main cpu. Different from the other sets
mt1-7.j3  /


mt1-8.n3  \
mt1-9.m3 --  Sprites. Apparently all different from the other sets
mt1-10.k3 /

mm6349.k2 \
prom1.f1  --  color proms, identical to other versions
prom2.h2  /

Ricky2001

*/

ROM_START( mototour )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mt1-4.m3",     0x0000, 0x2000, CRC(fe643567) SHA1(2e47b6de43ff7fc1f070d34376fde697fc719b80) )
	ROM_LOAD( "mt1-5.l3",     0x2000, 0x2000, CRC(38d9d0f5) SHA1(8b4531a28ff69df04a5eef687383dab57e0aa685) )
	ROM_LOAD( "mt1-6.k3",     0x4000, 0x2000, CRC(efd325f2) SHA1(0862c0ec87f601b6c1cba2bd25e3186b6ad0c68e) )
	ROM_LOAD( "mt1-7.j3",     0x6000, 0x2000, CRC(ab8a3a33) SHA1(e332b6e727083cf508ccec721ce42ccc3aa54e91) )

	ROM_REGION( 0x8000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "snd.a1",      0x7000, 0x1000, CRC(a02ad8a0) SHA1(aff80b506dbecabed2a36eb743693940f6a22d16) ) // == mr10.1a

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "mt1-1.e3", 0x0000, 0x2000, CRC(aa8994dd) SHA1(9b326ce52a03d723e5c3c1b5fd4aa8fa7f70f904) ) // == zippyrac.001
	ROM_LOAD( "mt1-2.c3", 0x2000, 0x2000, CRC(3a046dd1) SHA1(65c1dd1c0b5fb72ac5c04e11a577308245e4b312) ) // == mr8.3c
	ROM_LOAD( "mt1-3.a3", 0x4000, 0x2000, CRC(1cc3d3f4) SHA1(e7ee365d43d783cb6b7df37c6edeadbed35318d9) ) // == mr9.3a

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "mt1-8..n3",    0x0000, 0x2000, CRC(600a57f5) SHA1(86c2b2efb9392b7eca44510587d2459388c40435) )
	ROM_LOAD( "mt1-9..m3",    0x2000, 0x2000, CRC(6f9f2a4e) SHA1(8ebdd69895a4dd5de7fe84505359cccaa0aca6f8) )
	ROM_LOAD( "mt1-10..k3",   0x4000, 0x2000, CRC(d958def5) SHA1(198adf7e87804bd018b8cfa8bbc68623255698a2) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "mmi6349.k2", 0x0000, 0x0200, CRC(c9724350) SHA1(1fac20cdc0a53d94e8f67b49d7dd71d1b9f1f7ef) ) /* character palette - last $100 are unused */ // == mmi6349.ij
	ROM_LOAD( "prom1.f1",   0x0200, 0x0020, CRC(a1130007) SHA1(9deb0eed75dd06e86f83c819a3393158be7c9dce) ) /* sprite palette */ // == tbp18s.2
	ROM_LOAD( "prom2.h2",   0x0220, 0x0100, CRC(76062638) SHA1(7378a26cf455d9d3df90929dc665870514c34b54) ) /* sprite lookup table */ // == tbp24s10.3
ROM_END


/* it's probably a bootleg of the original Seibu version with the roms decrypted (no epoxy block) */
ROM_START( shtrider )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sr01a.bin", 0x0000, 0x2000, CRC(de76d537) SHA1(5ad90571c451b0d7b7a569556cfe075ead00fa2b) )
	ROM_LOAD( "sr02a.bin", 0x2000, 0x2000, CRC(cd1e1bfc) SHA1(fb156b5e5a5e7a24575264b391e0f3756ef3e021) )
	ROM_LOAD( "sr03a.bin", 0x4000, 0x2000, CRC(3ade11b9) SHA1(70b9dbd510cf6192194acf6876856d4c19bdf279) )
	ROM_LOAD( "sr04a.bin", 0x6000, 0x2000, CRC(02b96eaa) SHA1(ba4d61cf57142192684c45dd22720234d3521241) )

	ROM_REGION( 0x8000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "sr11a.bin", 0x6000, 0x2000, CRC(a8396b76) SHA1(614151fb1d25930e9fee4ab290a63f8fe97adbe6) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "sr05a.bin", 0x0000, 0x2000, CRC(34449f79) SHA1(30aa9da07bf32282d213f63e50c564a336fd0102) )
	ROM_LOAD( "sr06a.bin", 0x2000, 0x2000, CRC(de43653d) SHA1(a9fae236ee8e32d576123a4871ba3c46ca78ec3b) )
	ROM_LOAD( "sr07a.bin", 0x4000, 0x2000, CRC(3445b81c) SHA1(6768d411f8c3a347b10908e757a701d5b71ca2bc) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "sr08a.bin", 0x0000, 0x2000, CRC(4072b096) SHA1(e43482ac916a0fa259f74f99dc6ef72e86c23d9d) )
	ROM_LOAD( "sr09a.bin", 0x2000, 0x2000, CRC(fd4cc7e6) SHA1(3852883d32354e8c90c6cf701581ebc57d830c8b) )
	ROM_LOAD( "sr10b.bin", 0x4000, 0x2000, CRC(0a117925) SHA1(e061254428874b6153c2e9e514122373395f4da1) )

	ROM_REGION( 0x0420,  "proms", 0 )
	ROM_LOAD( "1.bpr",   0x0000, 0x0100, CRC(e9e258e5) SHA1(b98df686e8e2afa9ed05a56e1d3acb0d7cee3888) )
	ROM_LOAD( "2.bpr",   0x0100, 0x0100, CRC(6cf4591c) SHA1(3a5795758811f4fe3518216491ac13c0d17e842f) )
	ROM_LOAD( "4.bpr",   0x0200, 0x0020, CRC(ee97c581) SHA1(a5d0ba5e03f3bcbdd72f89f0495a98cef2821e59) )
	ROM_LOAD( "3.bpr",   0x0220, 0x0100, CRC(5db47092) SHA1(8e234ee88143755a4fd5ec86a03b55be5f9c5db8) )
ROM_END

ROM_START( shtridera )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",   0x0000, 0x2000, CRC(eb51315c) SHA1(0101c008b6731cd8ec796fee645113e2be79bd08) ) /* was inside epoxy block with cpu, encrypted */
	ROM_LOAD( "2.bin",   0x2000, 0x2000, CRC(97675d19) SHA1(774ce4d370fcbbd8a4109df023bf21db92d2e839) )
	ROM_LOAD( "3.bin",   0x4000, 0x2000, CRC(78d051cd) SHA1(e1dc2dcfc4af35bdd5245d23977e8640d81a43f1) )
	ROM_LOAD( "4.bin",   0x6000, 0x2000, CRC(02b96eaa) SHA1(ba4d61cf57142192684c45dd22720234d3521241) )

	ROM_REGION( 0x8000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "11.bin",   0x6000, 0x2000, CRC(a8396b76) SHA1(614151fb1d25930e9fee4ab290a63f8fe97adbe6) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "5.bin",   0x0000, 0x2000, CRC(34449f79) SHA1(30aa9da07bf32282d213f63e50c564a336fd0102) )
	ROM_LOAD( "6.bin",   0x2000, 0x2000, CRC(de43653d) SHA1(a9fae236ee8e32d576123a4871ba3c46ca78ec3b) )
	ROM_LOAD( "7.bin",   0x4000, 0x2000, CRC(3445b81c) SHA1(6768d411f8c3a347b10908e757a701d5b71ca2bc) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "8.bin",   0x0000, 0x2000, CRC(4072b096) SHA1(e43482ac916a0fa259f74f99dc6ef72e86c23d9d) )
	ROM_LOAD( "9.bin",   0x2000, 0x2000, CRC(fd4cc7e6) SHA1(3852883d32354e8c90c6cf701581ebc57d830c8b) )
	ROM_LOAD( "10.bin",  0x4000, 0x2000, CRC(0a117925) SHA1(e061254428874b6153c2e9e514122373395f4da1) )

	ROM_REGION( 0x0420,  "proms", 0 )
	ROM_LOAD( "1.bpr",   0x0000, 0x0100, CRC(e9e258e5) SHA1(b98df686e8e2afa9ed05a56e1d3acb0d7cee3888) )
	ROM_LOAD( "2.bpr",   0x0100, 0x0100, CRC(6cf4591c) SHA1(3a5795758811f4fe3518216491ac13c0d17e842f) )
	ROM_LOAD( "4.bpr",   0x0200, 0x0020, CRC(ee97c581) SHA1(a5d0ba5e03f3bcbdd72f89f0495a98cef2821e59) )
	ROM_LOAD( "3.bpr",   0x0220, 0x0100, CRC(5db47092) SHA1(8e234ee88143755a4fd5ec86a03b55be5f9c5db8) )
ROM_END

ROM_START( shtriderb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sr1.20.m3", 0x0000, 0x2000, CRC(8bca38d7) SHA1(97482ea1b6e5415df7149184dd5662d96a3e155a) )
	ROM_LOAD( "sr2.21.l3", 0x2000, 0x2000, CRC(56d4a66a) SHA1(e51a409ecc9f4d808678467ca7ce9bc0a35a7863) )
	ROM_LOAD( "sr3.22.k3", 0x4000, 0x2000, CRC(44cab4cc) SHA1(9e87d88353c858c25fb4ff9528c3f369505ffd88) )
	ROM_LOAD( "sr4.23.h3", 0x6000, 0x2000, CRC(02b96eaa) SHA1(ba4d61cf57142192684c45dd22720234d3521241) )

	ROM_REGION( 0x8000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "sr11.7.a1", 0x6000, 0x2000, CRC(a8396b76) SHA1(614151fb1d25930e9fee4ab290a63f8fe97adbe6) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "sr5.f3",  0x0000, 0x2000, CRC(34449f79) SHA1(30aa9da07bf32282d213f63e50c564a336fd0102) )
	ROM_LOAD( "sr6.c3",  0x2000, 0x2000, CRC(de43653d) SHA1(a9fae236ee8e32d576123a4871ba3c46ca78ec3b) )
	ROM_LOAD( "sr7.a3",  0x4000, 0x2000, CRC(3445b81c) SHA1(6768d411f8c3a347b10908e757a701d5b71ca2bc) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "sr8.17.n3", 0x0000, 0x2000, CRC(4072b096) SHA1(e43482ac916a0fa259f74f99dc6ef72e86c23d9d) )
	ROM_LOAD( "sr9.18.m3", 0x2000, 0x2000, CRC(fd4cc7e6) SHA1(3852883d32354e8c90c6cf701581ebc57d830c8b))
	ROM_LOAD( "sr10.19.k3",0x4000, 0x2000, CRC(0a117925) SHA1(e061254428874b6153c2e9e514122373395f4da1) )

	ROM_REGION( 0x0420,  "proms", 0 )
	ROM_LOAD( "6349-2.k2",   0x0000, 0x0200, CRC(854487a7) SHA1(5f3a2a7f7ba89f945fda97debb5436af8a2c6885) )
	ROM_LOAD( "prom1.6.f1",   0x0200, 0x0020, CRC(ee97c581) SHA1(a5d0ba5e03f3bcbdd72f89f0495a98cef2821e59))
	ROM_LOAD( "prom2.12.h2",  0x0220, 0x0100, CRC(5db47092) SHA1(8e234ee88143755a4fd5ec86a03b55be5f9c5db8) )
ROM_END

DRIVER_INIT_MEMBER(travrusa_state,motorace)
{
	int A, j;
	UINT8 *rom = memregion("maincpu")->base();
	dynamic_buffer buffer(0x2000);

	memcpy(&buffer[0], rom, 0x2000);

	/* The first CPU ROM has the address and data lines scrambled */
	for (A = 0; A < 0x2000; A++)
	{
		j = BITSWAP16(A,15,14,13,9,7,5,3,1,12,10,8,6,4,2,0,11);
		rom[j] = BITSWAP8(buffer[A],2,7,4,1,6,3,0,5);
	}
}

DRIVER_INIT_MEMBER(travrusa_state,shtridra)
{
	int A;
	UINT8 *rom = memregion("maincpu")->base();

	/* D3/D4  and  D5/D6 swapped */
	for (A = 0; A < 0x2000; A++)
		rom[A] = BITSWAP8(rom[A],7,5,6,3,4,2,1,0);
}

READ8_MEMBER(travrusa_state::shtridrb_port11_r)
{
	printf("shtridrb_port11_r %04x\n", space.device().safe_pc());
	// reads, masks with 0xa8, checks for 0x88, resets game if not happy with value?
	return 0x88;
}


DRIVER_INIT_MEMBER(travrusa_state, shtridrb)
{
	m_maincpu->space(AS_IO).install_read_handler(0x11, 0x11, 0x0000, 0xff00, read8_delegate(FUNC(travrusa_state::shtridrb_port11_r),this));
}


GAME( 1983, travrusa, 0,        travrusa, travrusa, driver_device, 0,         ROT270, "Irem",                    "Traverse USA / Zippy Race", GAME_SUPPORTS_SAVE )
GAME( 1983, travrusab,travrusa, travrusa, travrusa, driver_device, 0,         ROT270, "bootleg (I.P.)",          "Traverse USA (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1983, mototour, travrusa, travrusa, travrusa, driver_device, 0,         ROT270, "Irem (Tecfri license)",   "MotoTour / Zippy Race (Tecfri license)", GAME_SUPPORTS_SAVE )
GAME( 1983, motorace, travrusa, travrusa, motorace, travrusa_state, motorace, ROT270, "Irem (Williams license)", "MotoRace USA", GAME_SUPPORTS_SAVE )

GAME( 1985, shtrider, 0,        shtrider, shtrider, driver_device, 0,         ROT270|ORIENTATION_FLIP_X, "Seibu Kaihatsu",                 "Shot Rider", GAME_SUPPORTS_SAVE ) // possible bootleg
GAME( 1984, shtridera,shtrider, shtrider, shtrider, travrusa_state, shtridra, ROT270|ORIENTATION_FLIP_X, "Seibu Kaihatsu (Sigma license)", "Shot Rider (Sigma license)", GAME_SUPPORTS_SAVE )
GAME( 1985, shtriderb,shtrider, shtriderb,shtrider, travrusa_state, shtridrb, ROT270|ORIENTATION_FLIP_X, "bootleg",                        "Shot Rider (bootleg)", GAME_SUPPORTS_SAVE ) // resets when you attempt to start a game?
