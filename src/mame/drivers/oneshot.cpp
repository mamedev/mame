// license:BSD-3-Clause
// copyright-holders:David Haywood, Paul Priest
/* One Shot One Kill & Maddonna
   Driver by David Haywood and Paul Priest
   Dip Switches and Inputs by stephh

Notes :
  - The YM3812 is used only for timing. All sound is played with ADPCM samples.

  - It doesn't seem possible to make 2 consecutive shots at the same place !
    Ingame bug or is there something missing in the emulation ?
    * several gun games are like this, changed the driver so it doesn't try

  - Gun X range is 0x0000-0x01ff and gun Y range is 0x0000-0x00ff, so you
    can shoot sometimes out of the "visible area" ... NOT A BUG !
  - Player 1 and 2 guns do NOT use the same routine to determine the
    coordonates of an impact on the screen : position both guns in the
    "upper left" corner in the "gun test" to see what I mean.
  - I've assumed that the shot was right at the place the shot was made,
    but I don't have any more information about that
    (what the hell is "Gun X Shift Left" REALLY used for ?)

TO DO :

  - fix some priorities for some tiles
  - verify the parameters for the guns (analog ports)
  - figure out year and manufacturer
    (NOTHING is displayed in "demo mode", nor when you complete ALL levels !)
  - sound too fast in Maddonna?
  - layer order register?

NOTE: An eBay auction of the PCB shows "1996.9.16 PROMAT" on the JAMMA+ adapter for
      One Shot One Kill.  This information was used for the year & manufacturer.
      Also listed in an Approved Game list on a HK government site as "Promet"

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "includes/oneshot.h"
#include "sound/okim6295.h"
#include "sound/3812intf.h"

READ16_MEMBER(oneshot_state::oneshot_in0_word_r)
{
	int data = ioport("DSW1")->read();

	switch (data & 0x0c)
	{
		case 0x00 :
			m_gun_x_shift = 35;
			break;
		case 0x04 :
			m_gun_x_shift = 30;
			break;
		case 0x08 :
			m_gun_x_shift = 40;
			break;
		case 0x0c :
			m_gun_x_shift = 50;
			break;
	}

	return data;
}

READ16_MEMBER(oneshot_state::oneshot_gun_x_p1_r)
{
	/* shots must be in a different location to register */
	m_p1_wobble ^= 1;

	return m_gun_x_p1 ^ m_p1_wobble;
}

READ16_MEMBER(oneshot_state::oneshot_gun_y_p1_r)
{
	return m_gun_y_p1;
}

READ16_MEMBER(oneshot_state::oneshot_gun_x_p2_r)
{
	/* shots must be in a different location to register */
	m_p2_wobble ^= 1;

	return m_gun_x_p2 ^ m_p2_wobble;
}

READ16_MEMBER(oneshot_state::oneshot_gun_y_p2_r)
{
	return m_gun_y_p2;
}

WRITE16_MEMBER(oneshot_state::soundbank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_oki->set_bank_base(0x40000 * ((data & 0x03) ^ 0x03));
	}
}



static ADDRESS_MAP_START( oneshot_map, AS_PROGRAM, 16, oneshot_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x087fff) AM_RAM
	AM_RANGE(0x0c0000, 0x0c07ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x120000, 0x120fff) AM_RAM AM_SHARE("sprites")
	AM_RANGE(0x180000, 0x180fff) AM_RAM_WRITE(oneshot_mid_videoram_w) AM_SHARE("mid_videoram") // some people , girl etc.
	AM_RANGE(0x181000, 0x181fff) AM_RAM_WRITE(oneshot_fg_videoram_w) AM_SHARE("fg_videoram") // credits etc.
	AM_RANGE(0x182000, 0x182fff) AM_RAM_WRITE(oneshot_bg_videoram_w) AM_SHARE("bg_videoram") // credits etc.
	AM_RANGE(0x188000, 0x18800f) AM_WRITEONLY AM_SHARE("scroll")    // scroll registers
	AM_RANGE(0x190002, 0x190003) AM_READ(soundlatch_word_r)
	AM_RANGE(0x190010, 0x190011) AM_WRITE(soundlatch_word_w)
	AM_RANGE(0x190018, 0x190019) AM_WRITE(soundbank_w)
	AM_RANGE(0x190026, 0x190027) AM_READ(oneshot_gun_x_p1_r)
	AM_RANGE(0x19002e, 0x19002f) AM_READ(oneshot_gun_x_p2_r)
	AM_RANGE(0x190036, 0x190037) AM_READ(oneshot_gun_y_p1_r)
	AM_RANGE(0x19003e, 0x19003f) AM_READ(oneshot_gun_y_p2_r)
	AM_RANGE(0x19c020, 0x19c021) AM_READ(oneshot_in0_word_r)
	AM_RANGE(0x19c024, 0x19c025) AM_READ_PORT("DSW2")
	AM_RANGE(0x19c02c, 0x19c02d) AM_READ_PORT("CREDITS")
	AM_RANGE(0x19c030, 0x19c031) AM_READ_PORT("P1")
	AM_RANGE(0x19c034, 0x19c035) AM_READ_PORT("P2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( oneshot_sound_map, AS_PROGRAM, 8, oneshot_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8000) AM_READWRITE(soundlatch_byte_r,soundlatch_byte_w)
	AM_RANGE(0x8001, 0x87ff) AM_RAM
	AM_RANGE(0xe000, 0xe001) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)
	AM_RANGE(0xe010, 0xe010) AM_DEVREADWRITE("oki", okim6295_device, read, write)
ADDRESS_MAP_END


static INPUT_PORTS_START( oneshot )
	PORT_START("DSW1")  /* 0x19c020.l -> 0x08006c.l */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )      // 0x080084.l : credits (00-09)
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, "Gun X Shift Left" )      // 0x0824ec.l (not in "test mode")
	PORT_DIPSETTING(    0x04, "30" )
	PORT_DIPSETTING(    0x00, "35" )
	PORT_DIPSETTING(    0x08, "40" )
	PORT_DIPSETTING(    0x0c, "50" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  // 0x082706.l - to be confirmed
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )            // 0x0824fe.l
	PORT_DIPNAME( 0x40, 0x00, "Start Round" )           // 0x08224e.l
	PORT_DIPSETTING(    0x00, "Gun Trigger" )
	PORT_DIPSETTING(    0x40, "Start Button" )
	PORT_DIPNAME( 0x80, 0x00, "Gun Test" )          // 0x082286.l
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")  /* 0x19c024.l -> 0x08006e.l */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        // 0x082500.l
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )   // 0x082506.l
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )             // 0
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )           // 1
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )             // 2
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ) )          // 3
	PORT_DIPNAME( 0x40, 0x00, "Round Select" )      // 0x082f16.l - only after 1st stage
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )      // "On"  in the "test mode"
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )           // "Off" in the "test mode"
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Free_Play ) )    // 0x0800ca.l
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("CREDITS")   /* 0x19c02c.l -> 0x08007a.l */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")    /* Player 1 Gun Trigger (0x19c030.l) */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")    /* Player 2 Gun Trigger (0x19c034.l) */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("LIGHT0_X")  /* Player 1 Gun X       ($190026.l) */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")  /* Player 1 Gun Y       ($190036.l) */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")  /* Player 2 Gun X       ($19002e.l) */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")  /* Player 2 Gun Y       ($19003e.l) */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2)

INPUT_PORTS_END

static INPUT_PORTS_START( maddonna )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x04, 0x04, "Girl Pictures" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      // Not defined in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )      // Not defined in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Invulnerability" )       // This one was not defined in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )             // 2 Monsters at start, but "dumber"??
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )           // 2 Monsters at start
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )             // 3 Monsters at start
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )          // 4 Monsters at start
	PORT_DIPNAME( 0x0c, 0x08, "Time Per Round" )
	PORT_DIPSETTING(    0x08, "80 Seconds" )
	PORT_DIPSETTING(    0x04, "90 Seconds" )
	PORT_DIPSETTING(    0x00, "100 Seconds" )
//  PORT_DIPSETTING(    0x0c, "?? Seconds" )        // Not Defined for On+On
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )      // Not defined in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Hurry Up!" )         // Controls "Hurry Up!" banner & Vampire - Not defined the in manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )      // No Hurry up
	PORT_DIPSETTING(    0x40, "On - 10" )           // The rest show the banner but is there a difference in how the Vampire shows up???
	PORT_DIPSETTING(    0x80, "On - 01" )
	PORT_DIPSETTING(    0xc0, "On - 11" )

	PORT_START("CREDITS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout oneshot16x16_layout =
{
	16,16,
	RGN_FRAC(1,8),
	8,
	{ RGN_FRAC(0,8),RGN_FRAC(1,8),RGN_FRAC(2,8),RGN_FRAC(3,8),RGN_FRAC(4,8),RGN_FRAC(5,8),RGN_FRAC(6,8),RGN_FRAC(7,8) },
	{ 0,1,2,3,4,5,6,7,
		64+0,64+1,64+2,64+3,64+4,64+5,64+6,64+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		128+0*8, 128+1*8, 128+2*8, 128+3*8, 128+4*8, 128+5*8, 128+6*8, 128+7*8 },
	16*16
};

static const gfx_layout oneshot8x8_layout =
{
	8,8,
	RGN_FRAC(1,8),
	8,
	{ RGN_FRAC(0,8),RGN_FRAC(1,8),RGN_FRAC(2,8),RGN_FRAC(3,8),RGN_FRAC(4,8),RGN_FRAC(5,8),RGN_FRAC(6,8),RGN_FRAC(7,8) },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( oneshot )
	GFXDECODE_ENTRY( "gfx1", 0, oneshot16x16_layout,   0x00, 4  ) /* sprites */
	GFXDECODE_ENTRY( "gfx1", 0, oneshot8x8_layout,     0x00, 4  ) /* sprites */
GFXDECODE_END

void oneshot_state::machine_start()
{
	save_item(NAME(m_gun_x_p1));
	save_item(NAME(m_gun_y_p1));
	save_item(NAME(m_gun_x_p2));
	save_item(NAME(m_gun_y_p2));
	save_item(NAME(m_gun_x_shift));
	save_item(NAME(m_p1_wobble));
	save_item(NAME(m_p2_wobble));
}

void oneshot_state::machine_reset()
{
	m_gun_x_p1 = 0;
	m_gun_y_p1 = 0;
	m_gun_x_p2 = 0;
	m_gun_y_p2 = 0;
	m_gun_x_shift = 0;
	m_p1_wobble = 0;
	m_p2_wobble = 0;
}

static MACHINE_CONFIG_START( oneshot, oneshot_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(oneshot_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", oneshot_state,  irq4_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 5000000)
	MCFG_CPU_PROGRAM_MAP(oneshot_sound_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0*16, 20*16-1, 0*16, 15*16-1)
	MCFG_SCREEN_UPDATE_DRIVER(oneshot_state, screen_update_oneshot)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", oneshot)
	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, 3500000)
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maddonna, oneshot )

	/* basic machine hardware */

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(oneshot_state, screen_update_maddonna)
MACHINE_CONFIG_END


ROM_START( oneshot )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "1shot-u.a24", 0x00000, 0x20000, CRC(0ecd33da) SHA1(d050e9a1900cd9f629818034b1445e034b6cf81c) )
	ROM_LOAD16_BYTE( "1shot-u.a22", 0x00001, 0x20000, CRC(26c3ae2d) SHA1(47e479abe06d508a9d9fe677d34d6a485bde5533) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "1shot.ua2", 0x00000, 0x010000, CRC(f655b80e) SHA1(2574a812c35801755c187a47f46ccdb0983c5feb) )

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "1shot-ui.16a",0x000000, 0x080000, CRC(f765f9a2) SHA1(f6c386e0421fcb0e420585dd27d9dad951bb2556) )
	ROM_LOAD( "1shot-ui.13a",0x080000, 0x080000, CRC(3361b5d8) SHA1(f7db674d479765d4e58fb663aa5e13dde2abcce7) )
	ROM_LOAD( "1shot-ui.11a",0x100000, 0x080000, CRC(8f8bd027) SHA1(fbec952ab5604c8e20c5e7cfd2844f4fe5441186) )
	ROM_LOAD( "1shot-ui.08a",0x180000, 0x080000, CRC(254b1701) SHA1(163bfa70508fca20be70dd0af8b768ab6bf211b9) )
	ROM_LOAD( "1shot-ui.16", 0x200000, 0x080000, CRC(ff246b27) SHA1(fef6029030268174ef9648b8f437aeda68475346) )
	ROM_LOAD( "1shot-ui.13", 0x280000, 0x080000, CRC(80342e83) SHA1(2ac2b300382a607a539d2b0982ab596f05be3ad3) )
	ROM_LOAD( "1shot-ui.11", 0x300000, 0x080000, CRC(b8938345) SHA1(318cf0d070db786680a45811bbd765fa37caaf62) )
	ROM_LOAD( "1shot-ui.08", 0x380000, 0x080000, CRC(c9953bef) SHA1(21917a9dcc0afaeec20672ad863d0c9d583369e3) )

	ROM_REGION( 0x100000, "oki", 0 ) /* Samples */
	ROM_LOAD( "1shot.u15", 0x000000, 0x080000, CRC(e3759a47) SHA1(1159335924a6d68a0a24bfbe0c9182107f3f05f8) )
	ROM_LOAD( "1shot.u14", 0x080000, 0x080000, CRC(222e33f8) SHA1(2665afdf4cb1a29325df62efc1843a4b2cf34a4e) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "1shot.mb", 0x00000, 0x10000, CRC(6b213183) SHA1(599c59d155d11edb151bfaed1d24ef964462a447) ) // motherboard rom, zooming?
ROM_END

ROM_START( maddonna )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "maddonna.b16", 0x00000, 0x20000, CRC(643f9054) SHA1(77907ecdb02a525f9beed7fee203431eda16c831) )
	ROM_LOAD16_BYTE( "maddonna.b15", 0x00001, 0x20000, CRC(e36c0e26) SHA1(f261b2c74eeca05df302aa4956f5d02121d42054) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "x13.ua2", 0x00000, 0x010000, CRC(f2080071) SHA1(68cbae9559879b2dc19c41a7efbd13ab4a569d3f) ) // b13

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "maddonna.b5",  0x000000, 0x080000, CRC(838d3244) SHA1(7339143481ec043219825f282450ff53bb718f8c) )
	ROM_LOAD( "maddonna.b7",  0x080000, 0x080000, CRC(4920d2ec) SHA1(e72a374bca81ffa4f925326455e007df7227ae08) )
	ROM_LOAD( "maddonna.b9",  0x100000, 0x080000, CRC(3a8a3feb) SHA1(832654902963c163644134431fd1221e1895cfec) )
	ROM_LOAD( "maddonna.b11", 0x180000, 0x080000, CRC(6f9b7fdf) SHA1(14ced1d43eae3b6db4a0a4c12fb26cbd13eb7428) )
	ROM_LOAD( "maddonna.b6",   0x200000, 0x080000, CRC(b02e9e0e) SHA1(6e527a2bfda0f4f420c10139c75dac2704e08d08) )
	ROM_LOAD( "maddonna.b8",   0x280000, 0x080000, CRC(03f1de40) SHA1(bb0c0525155404c0740ac5f048f71ae7651a5941) )
	ROM_LOAD( "maddonna.b10",  0x300000, 0x080000, CRC(87936423) SHA1(dda42f3685427edad7686d9712ff07d2fd9bf57e) )
	ROM_LOAD( "maddonna.b12",  0x380000, 0x080000, CRC(879ab23c) SHA1(5288016542a10e60ccb28a930d8dfe4db41c6fc6) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) /* Samples */
	/* no samples for this game */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x1", 0x00000, 0x10000, CRC(6b213183) SHA1(599c59d155d11edb151bfaed1d24ef964462a447) ) // motherboard rom, zooming?
ROM_END

// The tiles containing the copyright string (tiles 0x3979 onwards) differ in this set.
// Both versions have tiles containing the 'Tuning - Germany' copyright messages, but
// the parent set has additional tiles containing the '(c)Copyright 1995' which is shown
// on the title screen.
//
// The lack of these tiles in this set causes all subsequent tiles to be shifted.  It is
// likely that the correct program roms for this set either don't show '(c)Copyright 1995'
// or display it using the regular font instead.

ROM_START( maddonnb )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	/* program roms missing in this dump, gfx don't seem 100% correct for other ones */
	ROM_LOAD16_BYTE( "maddonnb.b16", 0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "maddonnb.b15", 0x00001, 0x20000, NO_DUMP )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "x13.ua2", 0x00000, 0x010000, CRC(f2080071) SHA1(68cbae9559879b2dc19c41a7efbd13ab4a569d3f) )

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "x5.16a",  0x000000, 0x080000, CRC(1aae0ad3) SHA1(a5afe699c66dcc5e7928807ae1c8be7ffdda798c) )
	ROM_LOAD( "x7.13a",  0x080000, 0x080000, CRC(39d13e25) SHA1(bfe8b187c7fc9dc1ac2cc3f840a686a25ec55340) )
	ROM_LOAD( "x9.11a",  0x100000, 0x080000, CRC(2027faeb) SHA1(cb8c697705ac70ec3cf74901a2becf6abd8be63d) )
	ROM_LOAD( "x11.08a", 0x180000, 0x080000, CRC(4afcfba6) SHA1(b3fff9217db2770e703bf8317a718aeee1e5c44d) )
	ROM_LOAD( "x6.16",   0x200000, 0x080000, CRC(7b893e78) SHA1(d38c5b159031976e7864021e59cc4fff61ffb53f) )
	ROM_LOAD( "x8.13",   0x280000, 0x080000, CRC(fed90a1f) SHA1(e2cff7ce24697308c50dadb0c042d87f3e46abdb) )
	ROM_LOAD( "x10.11",  0x300000, 0x080000, CRC(479d718c) SHA1(4fbc2568744cf78b15c6e0f3caba4d7109743cdd) )
	ROM_LOAD( "x12.08",  0x380000, 0x080000, CRC(d56ca9f8) SHA1(49bca5dbc048e7b7efa34e1c08ee1b76767ffe38) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) /* Samples */
	/* no samples for this game */

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x1", 0x00000, 0x10000, CRC(6b213183) SHA1(599c59d155d11edb151bfaed1d24ef964462a447) ) // motherboard rom, zooming?
ROM_END



GAME( 1995, maddonna, 0,        maddonna, maddonna, driver_device, 0, ROT0, "Tuning",  "Mad Donna (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, maddonnb, maddonna, maddonna, maddonna, driver_device, 0, ROT0, "Tuning",  "Mad Donna (set 2)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1996, oneshot,  0,        oneshot,  oneshot , driver_device, 0, ROT0, "Promat",  "One Shot One Kill", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
