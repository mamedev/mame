// license:BSD-3-Clause
// copyright-holders:David Graves
// thanks-to:Richard Bush
/***************************************************************************

Taito Dual Screen Games
=======================

Sagaia / Darius 2 (c) 1989 Taito
Warrior Blade     (c) 1991 Taito

David Graves

(this is based on the F2 driver by Bryan McPhail, Brad Oliver, Andrew Prime,
Nicola Salmoria. Thanks to Richard Bush and the Raine team, whose open
source was very helpful in many areas particularly the sprites.)

                *****

The dual screen games operate on hardware with various similarities to
the Taito F2 system, as they share some custom ics e.g. the TC0100SCN.

For each screen the games have 3 separate layers of graphics: - one
128x64 tiled scrolling background plane of 8x8 tiles, a similar
foreground plane, and a 128x32 text plane with character definitions
held in ram. As well as this, there is a single sprite plane which
covers both screens. The sprites are 16x16 and are not zoomable.

Writing to the first TC0100SCN "writes through" to the subsidiary one
so both have identical contents. The only time the second TC0100SCN is
addressed on its own is during initial memory checks, I think. (?)

Warrior Blade has a slightly different gfx set for the 2nd screen
because the programmers ran out of scr gfx space (only 0xffff tiles
can be addressed by the TC0100SCN). In-game while tiles are
scrolling from one screen to the other it is necessary to have
identical gfx tiles for both screens. But for static screens (e.g. cut
scenes between levels) the gfx tiles needn't be the same. By
exploiting this they squeezed some extra graphics into the game.

There is a single 68000 processor which takes care of everything
except sound. That is done by a Z80 controlling a YM2610. Sound
commands are written to the Z80 by the 68000.


Tilemaps
========

TC0100SCN has tilemaps twice as wide as usual. The two BG tilemaps take
up twice the usual space, $8000 bytes each. The text tilemap takes up
the usual space, as its height is halved.

The double palette generator(one for each screen) is probably just a
result of the way the hardware works: they both have the same colors.


Dumper's Info
-------------

Darius II (Dual Screen Old & New JPN Ver.)
(c)1989 Taito
J1100204A
K1100483A

CPU     :MC68000P12F(16MHz),Z80A
Sound   :YM2610
OSC     :26.686MHz,16.000MHz
Other   :
TC0140SYT
TC0220IOC
TC0110PCR x2
TC0100SCN x2
TC0390LHC-1
TC0130LNB x8

Warrior Blade (JPN Ver.)
(c)1991 Taito
J1100295A
K1100710A (Label K11J0710A)

CPU     :MC68000P12F(16MHz),Z80A
Sound   :YM2610B
OSC     :26.686MHz,16.000MHz
Other   :
TC0140SYT
TC0510NIO
TC0110PCR x2
TC0100SCN x2
TC0390LHC-1
TC0130LNB x8

[The 390LHC/130LNB functions are unknown].


Stephh's notes (based on the game M68000 code and some tests) :

1) 'darius2d' and 'drius2do'

  - Region stored at 0x07fffe.w
  - Sets :
      * 'darius2d' : region = 0x0001
      * 'drius2do' : region = 0x0001
  - Comparison with 'darius2' :
      * coinage routine at 0x013e3a ('darius2d') or 0x013da4 ('drius2do')
      * same specific US coinage (4C_3C instead of 4C_1C for coinage)
      * what used to be an "Invulnerability" Dip Switch in 'darius' is now unused
        because the code to test it has been removed (not "noped")
      * different joystick mapping : 2 UDLR joys instead of 2 UDRL joys
      * same other notes as for 'darius2' (ninjaw.c driver)


2) 'warriorb'

  - Region stored at 0x0ffffe.w
  - Sets :
      * 'warriorb' : region = 0x0001
  - Coinage relies on the region (code at 0x002614) :
      * 0x0001 (Japan) uses TAITO_COINAGE_JAPAN_NEW
      * 0x0002 (US) uses TAITO_COINAGE_US
      * 0x0003 (World) uses TAITO_COINAGE_WORLD
  - Additional Japanese text when region = 0x0001
  - Notice screen only if region = 0x0001 or region = 0x0002
  - FBI logo only if region = 0x0002
  - US version doesn't reset score on continue, other versions do


TODO
====

Unknown sprite bits.


Darius 2
--------

The unpleasant sounds when some big enemies appear are wrong: they
are meant to create rumbling on a subwoofer in the cabinet, a sort of
vibration device. They still affect the other channels despite
filtering above 20Hz.


Warriorb
--------

Colscroll effects?

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "rendlay.h"
#include "cpu/m68000/m68000.h"
#include "sound/2610intf.h"
#include "includes/warriorb.h"
#include "includes/taitoipt.h"



/***********************************************************
                          SOUND
***********************************************************/

WRITE8_MEMBER(warriorb_state::sound_bankswitch_w)
{
	membank("z80bank")->set_entry(data & 7);
}

WRITE16_MEMBER(warriorb_state::sound_w)
{
	if (offset == 0)
		m_tc0140syt->master_port_w(space, 0, data & 0xff);
	else if (offset == 1)
		m_tc0140syt->master_comm_w(space, 0, data & 0xff);
}

READ16_MEMBER(warriorb_state::sound_r)
{
	if (offset == 1)
		return ((m_tc0140syt->master_comm_r(space, 0) & 0xff));
	else
		return 0;
}


WRITE8_MEMBER(warriorb_state::pancontrol)
{
	filter_volume_device *flt = nullptr;
	offset &= 3;

	switch (offset)
	{
		case 0: flt = m_2610_1l; break;
		case 1: flt = m_2610_1r; break;
		case 2: flt = m_2610_2l; break;
		case 3: flt = m_2610_2r; break;
	}

	m_pandata[offset] = (data << 1) + data;   /* original volume*3 */
	//popmessage(" pan %02x %02x %02x %02x", m_pandata[0], m_pandata[1], m_pandata[2], m_pandata[3] );
	flt->flt_volume_set_volume(m_pandata[offset] / 100.0);
}


WRITE16_MEMBER(warriorb_state::tc0100scn_dual_screen_w)
{
	m_tc0100scn_1->word_w(space, offset, data, mem_mask);
	m_tc0100scn_2->word_w(space, offset, data, mem_mask);
}

/***********************************************************
                      MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( darius2d_map, AS_PROGRAM, 16, warriorb_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM     /* main ram */
	AM_RANGE(0x200000, 0x213fff) AM_DEVREAD("tc0100scn_1", tc0100scn_device, word_r) AM_WRITE(tc0100scn_dual_screen_w)   /* tilemaps (all screens) */
	AM_RANGE(0x214000, 0x2141ff) AM_WRITENOP                                            /* error in screen clearing code ? */
	AM_RANGE(0x220000, 0x22000f) AM_DEVREADWRITE("tc0100scn_1", tc0100scn_device, ctrl_word_r, ctrl_word_w)
	AM_RANGE(0x240000, 0x253fff) AM_DEVREADWRITE("tc0100scn_2", tc0100scn_device, word_r, word_w)      /* tilemaps (2nd screen) */
	AM_RANGE(0x260000, 0x26000f) AM_DEVREADWRITE("tc0100scn_2", tc0100scn_device, ctrl_word_r, ctrl_word_w)
	AM_RANGE(0x400000, 0x400007) AM_DEVREADWRITE("tc0110pcr_1", tc0110pcr_device, word_r, step1_word_w)    /* palette (1st screen) */
	AM_RANGE(0x420000, 0x420007) AM_DEVREADWRITE("tc0110pcr_2", tc0110pcr_device, word_r, step1_word_w)    /* palette (2nd screen) */
	AM_RANGE(0x600000, 0x6013ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x800000, 0x80000f) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_device, read, write, 0x00ff)
//  AM_RANGE(0x820000, 0x820001) AM_WRITENOP    // ???
	AM_RANGE(0x830000, 0x830003) AM_READWRITE(sound_r, sound_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( warriorb_map, AS_PROGRAM, 16, warriorb_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x200000, 0x213fff) AM_RAM
	AM_RANGE(0x300000, 0x313fff) AM_DEVREAD("tc0100scn_1", tc0100scn_device, word_r) AM_WRITE(tc0100scn_dual_screen_w)   /* tilemaps (all screens) */
	AM_RANGE(0x320000, 0x32000f) AM_DEVREADWRITE("tc0100scn_1", tc0100scn_device, ctrl_word_r, ctrl_word_w)
	AM_RANGE(0x340000, 0x353fff) AM_DEVREADWRITE("tc0100scn_2", tc0100scn_device, word_r, word_w)      /* tilemaps (2nd screen) */
	AM_RANGE(0x360000, 0x36000f) AM_DEVREADWRITE("tc0100scn_2", tc0100scn_device, ctrl_word_r, ctrl_word_w)
	AM_RANGE(0x400000, 0x400007) AM_DEVREADWRITE("tc0110pcr_1", tc0110pcr_device, word_r, step1_word_w)    /* palette (1st screen) */
	AM_RANGE(0x420000, 0x420007) AM_DEVREADWRITE("tc0110pcr_2", tc0110pcr_device, word_r, step1_word_w)    /* palette (2nd screen) */
	AM_RANGE(0x600000, 0x6013ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x800000, 0x80000f) AM_DEVREADWRITE("tc0510nio", tc0510nio_device, halfword_r, halfword_w)
//  AM_RANGE(0x820000, 0x820001) AM_WRITENOP    // ? uses bits 0,2,3
	AM_RANGE(0x830000, 0x830003) AM_READWRITE(sound_r, sound_w)
ADDRESS_MAP_END

/***************************************************************************/

static ADDRESS_MAP_START( z80_sound_map, AS_PROGRAM, 8, warriorb_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("z80bank")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
	AM_RANGE(0xe200, 0xe200) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xe201, 0xe201) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0xe400, 0xe403) AM_WRITE(pancontrol) /* pan */
	AM_RANGE(0xea00, 0xea00) AM_READNOP
	AM_RANGE(0xee00, 0xee00) AM_WRITENOP /* ? */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP /* ? */
	AM_RANGE(0xf200, 0xf200) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END


/***********************************************************
                     INPUT PORTS, DIPs
***********************************************************/

static INPUT_PORTS_START( darius2d )
	/* 0x800000 -> 0x109e16 ($1e16,A5) and 0x109e1a ($1e1a,A5) */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Difficulty Enhancement" ) PORT_DIPLOCATION("SW1:1")    /* code at 0x0170f2 ('darius2d') or 0x01705c ('drius2do') */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) ) // Easy  Medium  Hard  Hardest  // Japan factory default = "Off"
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )  // Easy- Medium+ Hard+ Hardest+ // "Easy-" is easier than "Easy". "Medium+","Hard+" and "hardest+" are harder than "Medium","Hard" and "hardest".
	PORT_DIPNAME( 0x02, 0x02, "Auto Fire" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Fast" )
	TAITO_DSWA_BITS_2_TO_3_LOC(SW1)
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)

	/* 0x800002 -> 0x109e18 ($1e18,A5) and 0x109e1c ($1e1c,A5)  */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "every 700k" )
	PORT_DIPSETTING(    0x08, "every 800k" )
	PORT_DIPSETTING(    0x04, "every 900k" )
	PORT_DIPSETTING(    0x00, "every 1000k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	TAITO_JOY_DUAL_UDLR( 1, 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Freeze") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( sagaia )
	PORT_INCLUDE(darius2d)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x00, "Difficulty Enhancement" ) PORT_DIPLOCATION("SW1:1")    /* code at 0x0170f2 ('darius2d') or 0x01705c ('drius2do') */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) ) // Easy  Medium  Hard  Hardest
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )  // Easy- Medium+ Hard+ Hardest+
	// MAME 0.143u7 SW1:1="Unknown / Off / On" default="On"
	// I don't have World manual. Is it written "Unused : Must be kept On" ?
	TAITO_COINAGE_WORLD_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( warriorb )
	PORT_INCLUDE(darius2d)

	/* 0x800000 -> 0x202912.b (-$56ee,A5) */
	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x03, "Vitality Recovery" ) PORT_DIPLOCATION("SW1:1,2") /* table at 0x00d508 - 4 * 4 words */
	PORT_DIPSETTING(    0x02, "Less" )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "More" )
	PORT_DIPSETTING(    0x00, "Most" )
	TAITO_DSWA_BITS_2_TO_3_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	/* 0x800002 -> 0x202913.b (-$56ed,A5) */
	PORT_MODIFY("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x04, 0x04, "Gold Sheep at" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "50k only" )
	PORT_DIPSETTING(    0x00, "50k then every 70k" )
	PORT_DIPNAME( 0x08, 0x08, "Magic Energy Loss" ) PORT_DIPLOCATION("SW2:4")   /* code at 0x0587de - when BUTTON3 pressed */
	PORT_DIPSETTING(    0x08, "Always Player" )
	PORT_DIPSETTING(    0x00, "Player or Magician" )
	PORT_DIPNAME( 0x10, 0x10, "Player Starting Strength" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Full" )
	PORT_DIPNAME( 0x20, 0x20, "Magician appears" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "When you get a Crystal" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Rounds" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "Normal (10-14, depends on skill)" )
	PORT_DIPSETTING(    0x00, "Long (14)" )

	PORT_MODIFY("IN0")
	/* Japanese version actually doesn't have the third button */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Button 3 (Cheat)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Button 3 (Cheat)")
INPUT_PORTS_END


/***********************************************************
                        GFX DECODING
***********************************************************/

static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 3*4, 2*4, 11*4, 10*4, 1*4, 0*4, 9*4, 8*4, 7*4, 6*4, 15*4, 14*4, 5*4, 4*4, 13*4, 12*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( warriorb )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,  0, 256 )   /* sprites */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0, 256 )   /* scr tiles (screen 1) */
	GFXDECODE_ENTRY( "gfx3", 0, charlayout,  0, 256 )   /* scr tiles (screen 2) */
GFXDECODE_END

/***********************************************************
                       MACHINE DRIVERS
***********************************************************/

void warriorb_state::machine_start()
{
	membank("z80bank")->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_pandata));
}

void warriorb_state::machine_reset()
{
	/**** mixer control enable ****/
	machine().sound().system_enable(true);  /* mixer enabled */
}

static MACHINE_CONFIG_START( darius2d, warriorb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)   /* 12 MHz ??? (Might well be 16!) */
	MCFG_CPU_PROGRAM_MAP(darius2d_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", warriorb_state,  irq4_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,16000000/4)    /* 4 MHz ? */
	MCFG_CPU_PROGRAM_MAP(z80_sound_map)

	MCFG_DEVICE_ADD("tc0220ioc", TC0220IOC, 0)
	MCFG_TC0220IOC_READ_0_CB(IOPORT("DSWA"))
	MCFG_TC0220IOC_READ_1_CB(IOPORT("DSWB"))
	MCFG_TC0220IOC_READ_2_CB(IOPORT("IN0"))
	MCFG_TC0220IOC_READ_3_CB(IOPORT("IN1"))
	MCFG_TC0220IOC_READ_7_CB(IOPORT("IN2"))

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", warriorb)
	MCFG_PALETTE_ADD("palette", 4096)
	MCFG_PALETTE_ADD("palette2", 4096)

	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 3*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(warriorb_state, screen_update_left)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("tc0100scn_1", TC0100SCN, 0)
	MCFG_TC0100SCN_GFX_REGION(1)
	MCFG_TC0100SCN_TX_REGION(3)
	MCFG_TC0100SCN_OFFSETS(4, 0)
	MCFG_TC0100SCN_GFXDECODE("gfxdecode")
	MCFG_TC0100SCN_PALETTE("palette")

	MCFG_TC0110PCR_ADD("tc0110pcr_1")
	MCFG_TC0110PCR_PALETTE("palette")

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 3*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(warriorb_state, screen_update_right)
	MCFG_SCREEN_PALETTE("palette2")

	MCFG_DEVICE_ADD("tc0100scn_2", TC0100SCN, 0)
	MCFG_TC0100SCN_GFX_REGION(2)
	MCFG_TC0100SCN_TX_REGION(3)
	MCFG_TC0100SCN_OFFSETS(4, 0)
	MCFG_TC0100SCN_MULTISCR_HACK(1)
	MCFG_TC0100SCN_GFXDECODE("gfxdecode")
	MCFG_TC0100SCN_PALETTE("palette2")

	MCFG_TC0110PCR_ADD("tc0110pcr_2")
	MCFG_TC0110PCR_PALETTE("palette2")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2610, 16000000/2)
	MCFG_YM2610_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "2610.1.l", 1.0)
	MCFG_SOUND_ROUTE(1, "2610.1.r", 1.0)
	MCFG_SOUND_ROUTE(2, "2610.2.l", 1.0)
	MCFG_SOUND_ROUTE(2, "2610.2.r", 1.0)

	MCFG_FILTER_VOLUME_ADD("2610.1.l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("2610.1.r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("2610.2.l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("2610.2.r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( warriorb, warriorb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 16000000)   /* 16 MHz ? */
	MCFG_CPU_PROGRAM_MAP(warriorb_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", warriorb_state,  irq4_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,16000000/4)    /* 4 MHz ? */
	MCFG_CPU_PROGRAM_MAP(z80_sound_map)

	MCFG_DEVICE_ADD("tc0510nio", TC0510NIO, 0)
	MCFG_TC0510NIO_READ_0_CB(IOPORT("DSWA"))
	MCFG_TC0510NIO_READ_1_CB(IOPORT("DSWB"))
	MCFG_TC0510NIO_READ_2_CB(IOPORT("IN0"))
	MCFG_TC0510NIO_READ_3_CB(IOPORT("IN1"))
	MCFG_TC0510NIO_READ_7_CB(IOPORT("IN2"))

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", warriorb)
	MCFG_PALETTE_ADD("palette", 4096)
	MCFG_PALETTE_ADD("palette2", 4096)

	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(warriorb_state, screen_update_left)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("tc0100scn_1", TC0100SCN, 0)
	MCFG_TC0100SCN_GFX_REGION(1)
	MCFG_TC0100SCN_TX_REGION(3)
	MCFG_TC0100SCN_OFFSETS(4, 0)
	MCFG_TC0100SCN_GFXDECODE("gfxdecode")
	MCFG_TC0100SCN_PALETTE("palette")

	MCFG_TC0110PCR_ADD("tc0110pcr_1")
	MCFG_TC0110PCR_PALETTE("palette")

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(warriorb_state, screen_update_right)
	MCFG_SCREEN_PALETTE("palette2")

	MCFG_DEVICE_ADD("tc0100scn_2", TC0100SCN, 0)
	MCFG_TC0100SCN_GFX_REGION(2)
	MCFG_TC0100SCN_TX_REGION(3)
	MCFG_TC0100SCN_OFFSETS(4, 0)
	MCFG_TC0100SCN_MULTISCR_XOFFS(1)
	MCFG_TC0100SCN_MULTISCR_HACK(1)
	MCFG_TC0100SCN_GFXDECODE("gfxdecode")
	MCFG_TC0100SCN_PALETTE("palette2")

	MCFG_TC0110PCR_ADD("tc0110pcr_2")
	MCFG_TC0110PCR_PALETTE("palette2")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2610, 16000000/2)
	MCFG_YM2610_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "2610.1.l", 1.0)
	MCFG_SOUND_ROUTE(1, "2610.1.r", 1.0)
	MCFG_SOUND_ROUTE(2, "2610.2.l", 1.0)
	MCFG_SOUND_ROUTE(2, "2610.2.r", 1.0)

	MCFG_FILTER_VOLUME_ADD("2610.1.l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("2610.1.r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("2610.2.l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("2610.2.r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


/***************************************************************************
                                 DRIVERS
***************************************************************************/

ROM_START( sagaia )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 512K for 68000 code */
	ROM_LOAD16_BYTE( "c07_44.74", 0x00000, 0x20000, CRC(d0ca72d8) SHA1(13b47a4fb976167141dd36968f9e8d932ba78dcb) )
	ROM_LOAD16_BYTE( "c07_43.73", 0x00001, 0x20000, CRC(a34ea5ba) SHA1(300d168a8602b3c871fcd403fb72a8c8740eb013) )
	ROM_LOAD16_BYTE( "c07_45.76", 0x40000, 0x20000, CRC(8a043c14) SHA1(018647f3d3f4850ed0319266258fb33223c8a9ea) )
	ROM_LOAD16_BYTE( "c07_42.71", 0x40001, 0x20000, CRC(b6cb642f) SHA1(54f4848e0a411ac6e6cfc911800dfeba19c62936) )

	ROM_LOAD16_WORD_SWAP( "c07-09.75",   0x80000, 0x80000, CRC(cc69c2ce) SHA1(47883b9e14d8b6dd74db221bff396477231938f2) )   /* data rom */

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c07_41.69", 0x00000, 0x20000, CRC(b50256ea) SHA1(6ed271e4dafd1c759adaa55d5b2343d7374c721a) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "c07-03.12", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) ) /* SCr(screen 1) */
	ROM_LOAD( "c07-04.11", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "c07-06.27", 0x00000, 0x80000, CRC(5eebbcd6) SHA1(d4d860bf6b099956c45c7273ad77b1d35deba4c1) )  /* OBJ */
	ROM_LOAD32_BYTE( "c07-05.24", 0x00001, 0x80000, CRC(fb6d0550) SHA1(2d570ff5ef262cb4cb52e8584a7f167263194d37) )
	ROM_LOAD32_BYTE( "c07-08.25", 0x00002, 0x80000, CRC(a07dc846) SHA1(7199a604fcd693215ddb7670bfb2daf150145fd7) )
	ROM_LOAD32_BYTE( "c07-07.26", 0x00003, 0x80000, CRC(fd9f9e74) SHA1(e89beb5cac844fe16662465b0c76337692591aae) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_COPY( "gfx1", 0x00000, 0x000000, 0x100000 )    /* SCr(screen 2) */

/* The actual board duplicates the SCR gfx roms for the 2nd TC0100SCN */
//  ROM_LOAD( "c07-03.47", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) )
//  ROM_LOAD( "c07-04.48", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x100000, "ymsnd", 0 )  /* ADPCM samples */
	ROM_LOAD( "c07-10.95", 0x00000, 0x80000, CRC(4bbe0ed9) SHA1(081b73c4e4d4fa548445e5548573099bcb1e9213) )
	ROM_LOAD( "c07-11.96", 0x80000, 0x80000, CRC(3c815699) SHA1(0471ff5b0c0da905267f2cee52fd68c8661cccc9) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )   /* Delta-T samples */
	ROM_LOAD( "c07-12.107", 0x00000, 0x80000, CRC(e0b71258) SHA1(0258e308b643d723475824752ebffc4ea29d1ac4) )

	ROM_REGION( 0x001000, "user1", 0 )  /* unknown roms */
	ROM_LOAD( "c07-13.37", 0x00000, 0x00400, CRC(3ca18eb3) SHA1(54560f02c2be67993940831222130e90cd171991) )
	ROM_LOAD( "c07-14.38", 0x00000, 0x00400, CRC(baf2a193) SHA1(b7f103b5f5aab0702dd21fd7e3a82261ae1760e9) )

// Pals, not dumped
//  ROM_LOAD( "C07-15.78", 0x00000, 0x00?00, NO_DUMP )
//  ROM_LOAD( "C07-16.79", 0x00000, 0x00?00, NO_DUMP )
ROM_END

ROM_START( darius2d )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 512K for 68000 code */
	ROM_LOAD16_BYTE( "c07_20-2.74", 0x00000, 0x20000, CRC(a0f345b8) SHA1(1ce46e9707ec9ad51b26acf613eedc0536d227ae) )
	ROM_LOAD16_BYTE( "c07_19-2.73", 0x00001, 0x20000, CRC(925412c6) SHA1(7f1f62b7b2261c440dccd512ebd3faea141b7c83) )
	ROM_LOAD16_BYTE( "c07_21-2.76", 0x40000, 0x20000, CRC(bdd60e37) SHA1(777d3f67deba7df0da9d2605b2e2198f4bf47ebc) )
	ROM_LOAD16_BYTE( "c07_18-2.71", 0x40001, 0x20000, CRC(23fcd89b) SHA1(8aaf4ac836773d9b064ded68a6f092fe9eec7ac2) )

	ROM_LOAD16_WORD_SWAP( "c07-09.75",   0x80000, 0x80000, CRC(cc69c2ce) SHA1(47883b9e14d8b6dd74db221bff396477231938f2) )   /* data rom */

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c07_17.69", 0x00000, 0x20000, CRC(ae16c905) SHA1(70ba5aacd8a8e00b94719e3955abad8827c67aa8) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "c07-03.12", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) ) /* SCr(screen 1) */
	ROM_LOAD( "c07-04.11", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "c07-06.27", 0x00000, 0x80000, CRC(5eebbcd6) SHA1(d4d860bf6b099956c45c7273ad77b1d35deba4c1) )  /* OBJ */
	ROM_LOAD32_BYTE( "c07-05.24", 0x00001, 0x80000, CRC(fb6d0550) SHA1(2d570ff5ef262cb4cb52e8584a7f167263194d37) )
	ROM_LOAD32_BYTE( "c07-08.25", 0x00002, 0x80000, CRC(a07dc846) SHA1(7199a604fcd693215ddb7670bfb2daf150145fd7) )
	ROM_LOAD32_BYTE( "c07-07.26", 0x00003, 0x80000, CRC(fd9f9e74) SHA1(e89beb5cac844fe16662465b0c76337692591aae) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_COPY( "gfx1", 0x00000, 0x000000, 0x100000 )    /* SCr(screen 2) */

/* The actual board duplicates the SCR gfx roms for the 2nd TC0100SCN */
//  ROM_LOAD( "c07-03.47", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) )
//  ROM_LOAD( "c07-04.48", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x100000, "ymsnd", 0 )  /* ADPCM samples */
	ROM_LOAD( "c07-10.95", 0x00000, 0x80000, CRC(4bbe0ed9) SHA1(081b73c4e4d4fa548445e5548573099bcb1e9213) )
	ROM_LOAD( "c07-11.96", 0x80000, 0x80000, CRC(3c815699) SHA1(0471ff5b0c0da905267f2cee52fd68c8661cccc9) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )   /* Delta-T samples */
	ROM_LOAD( "c07-12.107", 0x00000, 0x80000, CRC(e0b71258) SHA1(0258e308b643d723475824752ebffc4ea29d1ac4) )

	ROM_REGION( 0x001000, "user1", 0 )  /* unknown roms */
	ROM_LOAD( "c07-13.37", 0x00000, 0x00400, CRC(3ca18eb3) SHA1(54560f02c2be67993940831222130e90cd171991) )
	ROM_LOAD( "c07-14.38", 0x00000, 0x00400, CRC(baf2a193) SHA1(b7f103b5f5aab0702dd21fd7e3a82261ae1760e9) )

// Pals, not dumped
//  ROM_LOAD( "C07-15.78", 0x00000, 0x00?00, NO_DUMP )
//  ROM_LOAD( "C07-16.79", 0x00000, 0x00?00, NO_DUMP )
ROM_END

ROM_START( darius2do )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 512K for 68000 code */
	ROM_LOAD16_BYTE( "c07_20-1.74", 0x00000, 0x20000, CRC(48b0804a) SHA1(932fb2cd55e6bfef84cf3cfaf3e75b4297a92b34) )
	ROM_LOAD16_BYTE( "c07_19-1.73", 0x00001, 0x20000, CRC(1f9a4f83) SHA1(d02caef350bdcac0ff771b5c92bb4e7435e0c9fa) )
	ROM_LOAD16_BYTE( "c07_21-1.76", 0x40000, 0x20000, CRC(b491b0ca) SHA1(dd7aa196c6002abc8e2f885f3f997f2279e59769) )
	ROM_LOAD16_BYTE( "c07_18-1.71", 0x40001, 0x20000, CRC(c552e42f) SHA1(dc952002a9a738cb1789f7c51acb71693ae03549) )

	ROM_LOAD16_WORD_SWAP( "c07-09.75",   0x80000, 0x80000, CRC(cc69c2ce) SHA1(47883b9e14d8b6dd74db221bff396477231938f2) )   /* data rom */

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c07_17.69", 0x00000, 0x20000, CRC(ae16c905) SHA1(70ba5aacd8a8e00b94719e3955abad8827c67aa8) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "c07-03.12", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) ) /* SCr(screen 1) */
	ROM_LOAD( "c07-04.11", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "c07-06.27", 0x00000, 0x80000, CRC(5eebbcd6) SHA1(d4d860bf6b099956c45c7273ad77b1d35deba4c1) )  /* OBJ */
	ROM_LOAD32_BYTE( "c07-05.24", 0x00001, 0x80000, CRC(fb6d0550) SHA1(2d570ff5ef262cb4cb52e8584a7f167263194d37) )
	ROM_LOAD32_BYTE( "c07-08.25", 0x00002, 0x80000, CRC(a07dc846) SHA1(7199a604fcd693215ddb7670bfb2daf150145fd7) )
	ROM_LOAD32_BYTE( "c07-07.26", 0x00003, 0x80000, CRC(fd9f9e74) SHA1(e89beb5cac844fe16662465b0c76337692591aae) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_COPY( "gfx1", 0x00000, 0x000000, 0x100000 )    /* SCr(screen 2) */

/* The actual board duplicates the SCR gfx roms for the 2nd TC0100SCN */
//  ROM_LOAD( "c07-03.47", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) )
//  ROM_LOAD( "c07-04.48", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x100000, "ymsnd", 0 )  /* ADPCM samples */
	ROM_LOAD( "c07-10.95", 0x00000, 0x80000, CRC(4bbe0ed9) SHA1(081b73c4e4d4fa548445e5548573099bcb1e9213) )
	ROM_LOAD( "c07-11.96", 0x80000, 0x80000, CRC(3c815699) SHA1(0471ff5b0c0da905267f2cee52fd68c8661cccc9) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )   /* Delta-T samples */
	ROM_LOAD( "c07-12.107", 0x00000, 0x80000, CRC(e0b71258) SHA1(0258e308b643d723475824752ebffc4ea29d1ac4) )

	ROM_REGION( 0x001000, "user1", 0 )  /* unknown roms */
	ROM_LOAD( "c07-13.37", 0x00000, 0x00400, CRC(3ca18eb3) SHA1(54560f02c2be67993940831222130e90cd171991) )
	ROM_LOAD( "c07-14.38", 0x00000, 0x00400, CRC(baf2a193) SHA1(b7f103b5f5aab0702dd21fd7e3a82261ae1760e9) )
ROM_END

ROM_START( warriorb )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 1024K for 68000 code */
	ROM_LOAD16_BYTE( "d24_20-1.74", 0x000000, 0x40000, CRC(4452dc25) SHA1(bbb4fbc25a3f263ce2716698cacaca201cb9591b) )
	ROM_LOAD16_BYTE( "d24_19-1.73", 0x000001, 0x40000, CRC(15c16016) SHA1(5b28834d8d5296c562c90a861c6ccdd46cc3c204) )
	ROM_LOAD16_BYTE( "d24_21-1.76", 0x080000, 0x40000, CRC(783ef8e1) SHA1(28a43d5231031b2ff3e437c3b6b8604f0d2b521b) )
	ROM_LOAD16_BYTE( "d24_18-1.71", 0x080001, 0x40000, CRC(4502db60) SHA1(b29c441ab79f753378ea47e7c22924db0cd5eb89) )

	ROM_LOAD16_WORD_SWAP( "d24-09.75",   0x100000, 0x100000, CRC(ece5cc59) SHA1(337db41d5a74fa4202b1be1a672a068ec3b205a8) ) /* data rom */
	/* Note: Raine wrongly doubles up d24-09 as delta-t samples */

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "d24_17.69",  0x00000, 0x20000, CRC(e41e4aae) SHA1(9bf40b6e8aa5c6ec62c5d21edbb2214f6550c94f) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "d24-02.12", 0x000000, 0x100000, CRC(9f50c271) SHA1(1a1b2ae7cb7785e7f66aa26258a6cd2921a29545) )   /* SCR A, screen 1 */
	ROM_LOAD( "d24-01.11", 0x100000, 0x100000, CRC(326dcca9) SHA1(1993776d71bca7d6dfc6f84dd9262d0dcae87f69) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "d24-06.27", 0x000000, 0x100000, CRC(918486fe) SHA1(cc9e287221ef33dba77a22975e23b250ba50b758) )    /* OBJ */
	ROM_LOAD32_BYTE( "d24-03.24", 0x000001, 0x100000, CRC(46db9fd7) SHA1(f08f3c9833d80ce161b06f4ae484c5c79539639c) )
	ROM_LOAD32_BYTE( "d24-04.25", 0x000002, 0x100000, CRC(148e0493) SHA1(f1cb819830e5bd544b11762784e228b5cb62b7e4) )
	ROM_LOAD32_BYTE( "d24-05.26", 0x000003, 0x100000, CRC(9f414317) SHA1(204cf47404e5e1085c1108abacd2b79a6cd0f74a) )

	ROM_REGION( 0x200000, "gfx3", 0 )
	ROM_LOAD( "d24-07.47", 0x000000, 0x100000, CRC(9f50c271) SHA1(1a1b2ae7cb7785e7f66aa26258a6cd2921a29545) )   /* SCR B, screen 2 */
	ROM_LOAD( "d24-08.48", 0x100000, 0x100000, CRC(1e6d1528) SHA1(d6843aa67befd7db44f468be16ba2f0efb85d40f) )

	ROM_REGION( 0x300000, "ymsnd", 0 )  /* ADPCM samples */
	ROM_LOAD( "d24-12.107", 0x000000, 0x100000, CRC(279203a1) SHA1(ed75e811a1f0863c134034457ce2e97372726bdb) )
	ROM_LOAD( "d24-10.95",  0x100000, 0x100000, CRC(0e0c716d) SHA1(5e2f334dd484678766c5a71196d9bad0ba0fe8d9) )
	ROM_LOAD( "d24-11.118", 0x200000, 0x100000, CRC(15362573) SHA1(8602c9f24134cac6fe1375fb189b152f0c68aeb7) )

	/* No Delta-T samples */

	ROM_REGION( 0x01000, "user1", 0 )   /* unknown roms */
	ROM_LOAD( "d24-13.37", 0x00000, 0x400, CRC(3ca18eb3) SHA1(54560f02c2be67993940831222130e90cd171991) ) /* AM27S33A or compatible like N82HS137A */
	ROM_LOAD( "d24-14.38", 0x00000, 0x400, CRC(baf2a193) SHA1(b7f103b5f5aab0702dd21fd7e3a82261ae1760e9) ) /* AM27S33A or compatible like N82HS137A */
//  ROM_LOAD( "d24-15.78", 0x00000, 0xa??, NO_DUMP )    /* 20L8B Pal */
//  ROM_LOAD( "d24-16.79", 0x00000, 0xa??, NO_DUMP )    /* 20L8B Pal */
ROM_END


/* Working Games */

//    YEAR, NAME,      PARENT,  MACHINE,  INPUT,    INIT,MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1989, sagaia,    darius2, darius2d, sagaia, driver_device,   0,   ROT0,   "Taito Corporation Japan", "Sagaia (dual screen) (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, darius2d,  darius2, darius2d, darius2d, driver_device, 0,   ROT0,   "Taito Corporation", "Darius II (dual screen) (Japan, Rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, darius2do, darius2, darius2d, darius2d, driver_device, 0,   ROT0,   "Taito Corporation", "Darius II (dual screen) (Japan, Rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, warriorb,  0,       warriorb, warriorb, driver_device, 0,   ROT0,   "Taito Corporation", "Warrior Blade - Rastan Saga Episode III (Japan)", MACHINE_SUPPORTS_SAVE )
