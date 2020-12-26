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
#include "includes/warriorb.h"
#include "includes/taitoipt.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2610intf.h"

#include "rendlay.h"
#include "screen.h"
#include "speaker.h"


void warriorb_state::coin_control_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}


/***********************************************************
                          SOUND
***********************************************************/

void warriorb_state::sound_bankswitch_w(u8 data)
{
	m_z80bank->set_entry(data & 7);
}


void warriorb_state::pancontrol_w(offs_t offset, u8 data)
{
	filter_volume_device *flt = nullptr;
	offset &= 3;
	offset ^= 1;

	switch (offset)
	{
		case 0: flt = m_2610_l[0]; break;
		case 1: flt = m_2610_r[0]; break;
		case 2: flt = m_2610_l[1]; break;
		case 3: flt = m_2610_r[1]; break;
	}

	m_pandata[offset] = (data << 1) + data;   /* original volume*3 */
	//popmessage(" pan %02x %02x %02x %02x", m_pandata[0], m_pandata[1], m_pandata[2], m_pandata[3] );
	flt->flt_volume_set_volume(m_pandata[offset] / 100.0);
}


void warriorb_state::tc0100scn_dual_screen_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_tc0100scn[0]->ram_w(offset, data, mem_mask);
	m_tc0100scn[1]->ram_w(offset, data, mem_mask);
}

/***********************************************************
                      MEMORY STRUCTURES
***********************************************************/

void warriorb_state::darius2d_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram();     /* main ram */
	map(0x200000, 0x213fff).r(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r)).w(FUNC(warriorb_state::tc0100scn_dual_screen_w));   /* tilemaps (all screens) */
	map(0x214000, 0x2141ff).nopw();                                            /* error in screen clearing code ? */
	map(0x220000, 0x22000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x240000, 0x253fff).rw(m_tc0100scn[1], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));      /* tilemaps (2nd screen) */
	map(0x260000, 0x26000f).rw(m_tc0100scn[1], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x400000, 0x400007).rw(m_tc0110pcr[0], FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));    /* palette (1st screen) */
	map(0x420000, 0x420007).rw(m_tc0110pcr[1], FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));    /* palette (2nd screen) */
	map(0x600000, 0x6013ff).ram().share("spriteram");
	map(0x800000, 0x80000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
//  map(0x820000, 0x820001).nopw();    // ???
	map(0x830001, 0x830001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x830003, 0x830003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
}

void warriorb_state::warriorb_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x213fff).ram();
	map(0x300000, 0x313fff).r(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r)).w(FUNC(warriorb_state::tc0100scn_dual_screen_w));   /* tilemaps (all screens) */
	map(0x320000, 0x32000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x340000, 0x353fff).rw(m_tc0100scn[1], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));      /* tilemaps (2nd screen) */
	map(0x360000, 0x36000f).rw(m_tc0100scn[1], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x400000, 0x400007).rw(m_tc0110pcr[0], FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));    /* palette (1st screen) */
	map(0x420000, 0x420007).rw(m_tc0110pcr[1], FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));    /* palette (2nd screen) */
	map(0x600000, 0x6013ff).ram().share("spriteram");
	map(0x800000, 0x80000f).rw(m_tc0510nio, FUNC(tc0510nio_device::read), FUNC(tc0510nio_device::write)).umask16(0x00ff);
//  map(0x820000, 0x820001).nopw();    // ? uses bits 0,2,3
	map(0x830001, 0x830001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x830003, 0x830003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
}

/***************************************************************************/

void warriorb_state::z80_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("z80bank");
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0xe200, 0xe200).nopr().w(m_tc0140syt, FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw(m_tc0140syt, FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xe400, 0xe403).w(FUNC(warriorb_state::pancontrol_w)); /* pan */
	map(0xea00, 0xea00).nopr();
	map(0xee00, 0xee00).nopw(); /* ? */
	map(0xf000, 0xf000).nopw(); /* ? */
	map(0xf200, 0xf200).w(FUNC(warriorb_state::sound_bankswitch_w));
}


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

static GFXDECODE_START( gfx_warriorb )
	GFXDECODE_ENTRY( "sprites", 0, gfx_16x16x4_packed_lsb, 0, 256 )   /* sprites */
GFXDECODE_END

/***********************************************************
                       MACHINE DRIVERS
***********************************************************/

void warriorb_state::machine_start()
{
	m_z80bank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_pandata));
}

void warriorb_state::machine_reset()
{
	/**** mixer control enable ****/
	machine().sound().system_mute(false);  /* mixer enabled */
}

void warriorb_state::darius2d(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL);    /* MC68000P12F 16 MHz, 16 MHz XTAL */
	m_maincpu->set_addrmap(AS_PROGRAM, &warriorb_state::darius2d_map);
	m_maincpu->set_vblank_int("lscreen", FUNC(warriorb_state::irq4_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 16_MHz_XTAL / 4));  /* 4 MHz (16 MHz XTAL / 4) */
	audiocpu.set_addrmap(AS_PROGRAM, &warriorb_state::z80_sound_map);

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(warriorb_state::coin_control_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	GFXDECODE(config, m_gfxdecode[0], m_tc0110pcr[0], gfx_warriorb);
	GFXDECODE(config, m_gfxdecode[1], m_tc0110pcr[1], gfx_warriorb);

	config.set_default_layout(layout_dualhsxs);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lscreen.set_size(40*8, 32*8);
	lscreen.set_visarea(0*8, 40*8-1, 3*8, 32*8-1);
	lscreen.set_screen_update(FUNC(warriorb_state::screen_update_left));
	lscreen.set_palette(m_tc0110pcr[0]);

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(4, 0);
	m_tc0100scn[0]->set_palette(m_tc0110pcr[0]);

	TC0110PCR(config, m_tc0110pcr[0], 0);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	rscreen.set_size(40*8, 32*8);
	rscreen.set_visarea(0*8, 40*8-1, 3*8, 32*8-1);
	rscreen.set_screen_update(FUNC(warriorb_state::screen_update_right));
	rscreen.set_palette(m_tc0110pcr[1]);

	TC0100SCN(config, m_tc0100scn[1], 0);
	m_tc0100scn[1]->set_offsets(4, 0);
	m_tc0100scn[1]->set_multiscr_hack(1);
	m_tc0100scn[1]->set_palette(m_tc0110pcr[1]);

	TC0110PCR(config, m_tc0110pcr[1], 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	SPEAKER(config, "subwoofer").seat();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 16_MHz_XTAL / 2));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "subwoofer", 0.25);
	ymsnd.add_route(1, "2610.1.l", 1.0);
	ymsnd.add_route(1, "2610.1.r", 1.0);
	ymsnd.add_route(2, "2610.2.l", 1.0);
	ymsnd.add_route(2, "2610.2.r", 1.0);

	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->set_master_tag(m_maincpu);
	m_tc0140syt->set_slave_tag("audiocpu");
}

void warriorb_state::warriorb(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL);    /* MC68000P12F 16 MHz, 16 MHz XTAL */
	m_maincpu->set_addrmap(AS_PROGRAM, &warriorb_state::warriorb_map);
	m_maincpu->set_vblank_int("lscreen", FUNC(warriorb_state::irq4_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 16_MHz_XTAL / 4));  /* 4 MHz (16 MHz XTAL / 4) */
	audiocpu.set_addrmap(AS_PROGRAM, &warriorb_state::z80_sound_map);

	TC0510NIO(config, m_tc0510nio, 0);
	m_tc0510nio->read_0_callback().set_ioport("DSWA");
	m_tc0510nio->read_1_callback().set_ioport("DSWB");
	m_tc0510nio->read_2_callback().set_ioport("IN0");
	m_tc0510nio->read_3_callback().set_ioport("IN1");
	m_tc0510nio->write_4_callback().set(FUNC(warriorb_state::coin_control_w));
	m_tc0510nio->read_7_callback().set_ioport("IN2");

	/* video hardware */
	GFXDECODE(config, m_gfxdecode[0], m_tc0110pcr[0], gfx_warriorb);
	GFXDECODE(config, m_gfxdecode[1], m_tc0110pcr[1], gfx_warriorb);

	config.set_default_layout(layout_dualhsxs);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lscreen.set_size(40*8, 32*8);
	lscreen.set_visarea(0*8, 40*8-1, 2*8, 32*8-1);
	lscreen.set_screen_update(FUNC(warriorb_state::screen_update_left));
	lscreen.set_palette(m_tc0110pcr[0]);

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(4, 0);
	m_tc0100scn[0]->set_palette(m_tc0110pcr[0]);

	TC0110PCR(config, m_tc0110pcr[0], 0);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	rscreen.set_size(40*8, 32*8);
	rscreen.set_visarea(0*8, 40*8-1, 2*8, 32*8-1);
	rscreen.set_screen_update(FUNC(warriorb_state::screen_update_right));
	rscreen.set_palette(m_tc0110pcr[1]);

	TC0100SCN(config, m_tc0100scn[1], 0);
	m_tc0100scn[1]->set_offsets(4, 0);
	m_tc0100scn[1]->set_multiscr_xoffs(1);
	m_tc0100scn[1]->set_multiscr_hack(1);
	m_tc0100scn[1]->set_palette(m_tc0110pcr[1]);

	TC0110PCR(config, m_tc0110pcr[1], 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	// subwoofer under seat is exists like other taito multiscreen games?

	ym2610b_device &ymsnd(YM2610B(config, "ymsnd", 16_MHz_XTAL / 2));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "2610.1.l", 1.0);
	ymsnd.add_route(1, "2610.1.r", 1.0);
	ymsnd.add_route(2, "2610.2.l", 1.0);
	ymsnd.add_route(2, "2610.2.r", 1.0);

	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->set_master_tag(m_maincpu);
	m_tc0140syt->set_slave_tag("audiocpu");
}


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

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD( "c07-05.24", 0x00000, 0x80000, CRC(fb6d0550) SHA1(2d570ff5ef262cb4cb52e8584a7f167263194d37) )  /* OBJ */
	ROM_LOAD64_WORD( "c07-06.27", 0x00002, 0x80000, CRC(5eebbcd6) SHA1(d4d860bf6b099956c45c7273ad77b1d35deba4c1) )
	ROM_LOAD64_WORD( "c07-07.26", 0x00004, 0x80000, CRC(fd9f9e74) SHA1(e89beb5cac844fe16662465b0c76337692591aae) )
	ROM_LOAD64_WORD( "c07-08.25", 0x00006, 0x80000, CRC(a07dc846) SHA1(7199a604fcd693215ddb7670bfb2daf150145fd7) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )
	ROM_LOAD16_WORD_SWAP( "c07-03.12", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) ) /* SCr(screen 1) */
	ROM_LOAD16_WORD_SWAP( "c07-04.11", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x100000, "tc0100scn_2", 0 )
	ROM_LOAD16_WORD_SWAP( "c07-03.47", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) ) /* SCr(screen 2) */
	ROM_LOAD16_WORD_SWAP( "c07-04.48", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

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

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD( "c07-05.24", 0x00000, 0x80000, CRC(fb6d0550) SHA1(2d570ff5ef262cb4cb52e8584a7f167263194d37) )  /* OBJ */
	ROM_LOAD64_WORD( "c07-06.27", 0x00002, 0x80000, CRC(5eebbcd6) SHA1(d4d860bf6b099956c45c7273ad77b1d35deba4c1) )
	ROM_LOAD64_WORD( "c07-07.26", 0x00004, 0x80000, CRC(fd9f9e74) SHA1(e89beb5cac844fe16662465b0c76337692591aae) )
	ROM_LOAD64_WORD( "c07-08.25", 0x00006, 0x80000, CRC(a07dc846) SHA1(7199a604fcd693215ddb7670bfb2daf150145fd7) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )
	ROM_LOAD16_WORD_SWAP( "c07-03.12", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) ) /* SCr(screen 1) */
	ROM_LOAD16_WORD_SWAP( "c07-04.11", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x100000, "tc0100scn_2", 0 )
	ROM_LOAD16_WORD_SWAP( "c07-03.47", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) ) /* SCr(screen 2) */
	ROM_LOAD16_WORD_SWAP( "c07-04.48", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

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

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD( "c07-05.24", 0x00000, 0x80000, CRC(fb6d0550) SHA1(2d570ff5ef262cb4cb52e8584a7f167263194d37) )  /* OBJ */
	ROM_LOAD64_WORD( "c07-06.27", 0x00002, 0x80000, CRC(5eebbcd6) SHA1(d4d860bf6b099956c45c7273ad77b1d35deba4c1) )
	ROM_LOAD64_WORD( "c07-07.26", 0x00004, 0x80000, CRC(fd9f9e74) SHA1(e89beb5cac844fe16662465b0c76337692591aae) )
	ROM_LOAD64_WORD( "c07-08.25", 0x00006, 0x80000, CRC(a07dc846) SHA1(7199a604fcd693215ddb7670bfb2daf150145fd7) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )
	ROM_LOAD16_WORD_SWAP( "c07-03.12", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) ) /* SCr(screen 1) */
	ROM_LOAD16_WORD_SWAP( "c07-04.11", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x100000, "tc0100scn_2", 0 )
	ROM_LOAD16_WORD_SWAP( "c07-03.47", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) ) /* SCr(screen 2) */
	ROM_LOAD16_WORD_SWAP( "c07-04.48", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

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

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD( "d24-03.24", 0x000000, 0x100000, CRC(46db9fd7) SHA1(f08f3c9833d80ce161b06f4ae484c5c79539639c) )    /* OBJ */
	ROM_LOAD64_WORD( "d24-06.27", 0x000002, 0x100000, CRC(918486fe) SHA1(cc9e287221ef33dba77a22975e23b250ba50b758) )
	ROM_LOAD64_WORD( "d24-05.26", 0x000004, 0x100000, CRC(9f414317) SHA1(204cf47404e5e1085c1108abacd2b79a6cd0f74a) )
	ROM_LOAD64_WORD( "d24-04.25", 0x000006, 0x100000, CRC(148e0493) SHA1(f1cb819830e5bd544b11762784e228b5cb62b7e4) )

	ROM_REGION( 0x200000, "tc0100scn_1", 0 )
	ROM_LOAD16_WORD_SWAP( "d24-02.12", 0x000000, 0x100000, CRC(9f50c271) SHA1(1a1b2ae7cb7785e7f66aa26258a6cd2921a29545) )   /* SCR A, screen 1 */
	ROM_LOAD16_WORD_SWAP( "d24-01.11", 0x100000, 0x100000, CRC(326dcca9) SHA1(1993776d71bca7d6dfc6f84dd9262d0dcae87f69) )

	ROM_REGION( 0x200000, "tc0100scn_2", 0 )
	ROM_LOAD16_WORD_SWAP( "d24-07.47", 0x000000, 0x100000, CRC(9f50c271) SHA1(1a1b2ae7cb7785e7f66aa26258a6cd2921a29545) )   /* SCR B, screen 2 */
	ROM_LOAD16_WORD_SWAP( "d24-08.48", 0x100000, 0x100000, CRC(1e6d1528) SHA1(d6843aa67befd7db44f468be16ba2f0efb85d40f) )

	ROM_REGION( 0x300000, "ymsnd", 0 )  /* ADPCM samples */
	ROM_LOAD( "d24-12.107", 0x000000, 0x100000, CRC(279203a1) SHA1(ed75e811a1f0863c134034457ce2e97372726bdb) )
	ROM_LOAD( "d24-10.95",  0x100000, 0x100000, CRC(0e0c716d) SHA1(5e2f334dd484678766c5a71196d9bad0ba0fe8d9) )
	ROM_LOAD( "d24-11.118", 0x200000, 0x100000, CRC(15362573) SHA1(8602c9f24134cac6fe1375fb189b152f0c68aeb7) )

	/* No Delta-T samples */

	ROM_REGION( 0x01000, "user1", 0 )   /* unknown roms */
	ROM_LOAD( "d24-13.37", 0x00000, 0x400, CRC(3ca18eb3) SHA1(54560f02c2be67993940831222130e90cd171991) ) /* AM27S33A or compatible like N82HS137A */
	ROM_LOAD( "d24-14.38", 0x00000, 0x400, CRC(baf2a193) SHA1(b7f103b5f5aab0702dd21fd7e3a82261ae1760e9) ) /* AM27S33A or compatible like N82HS137A */
	ROM_LOAD( "d24-15.78", 0x00000, 0x144, CRC(04992a7d) SHA1(82ce7ab7e3e7045776b660c32dac4abc28cabfa5) ) // PAL20L8BCNS
	ROM_LOAD( "d24-16.79", 0x00000, 0x144, CRC(92c59a8d) SHA1(a83eb70cdc47af688a33505f60e5cb9960f8ba9f) ) // PAL20L8BCNS
ROM_END


/* Working Games */

//    YEAR, NAME,      PARENT,  MACHINE,  INPUT,    STATE,          INIT,       MONITOR,COMPANY,FULLNAME,          FLAGS
GAME( 1989, sagaia,    darius2, darius2d, sagaia,   warriorb_state, empty_init, ROT0,   "Taito Corporation Japan", "Sagaia (dual screen) (World)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1989, darius2d,  darius2, darius2d, darius2d, warriorb_state, empty_init, ROT0,   "Taito Corporation",       "Darius II (dual screen) (Japan, Rev 2)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1989, darius2do, darius2, darius2d, darius2d, warriorb_state, empty_init, ROT0,   "Taito Corporation",       "Darius II (dual screen) (Japan, Rev 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1991, warriorb,  0,       warriorb, warriorb, warriorb_state, empty_init, ROT0,   "Taito Corporation",       "Warrior Blade - Rastan Saga Episode III (Japan)", MACHINE_SUPPORTS_SAVE )
