// license:BSD-3-Clause
// copyright-holders:Yochizo
// thanks-to:Richard Bush
/***************************************************************************

Taito H system
----------------------------
driver by Yochizo

This driver is heavily dependent on the Raine source.
Special thanks to Richard Bush and the Raine team. Also,
I have been given a lot of helpful information by
Yasuhiro Ogawa. Thank you, Yasu.


Supported games :
==================
 Syvalion                           (C) 1988 Taito
 Record Breaker / Go For The Gold   (C) 1988 Taito
 Dynamite League                    (C) 1990 Taito


System specs :
===============
 CPU   : MC68000 (12 MHz) x 1, Z80 (4 MHz?, sound CPU) x 1
 Sound : YM2610, YM3016
 OSC   : 20.000 MHz, 8.000 MHz, 24.000 MHz
 Chips : TC0070RGB (RGB/Video Mixer)
         TC0220IOC (Input)
         TC0140SYT (Sound communication)
         TC0130LNB (???)
         TC0160ROM (???)
         TC0080VCO (Tilemap & Motion Object Gen)

 name             irq    resolution   tx-layer   tc0220ioc
 --------------------------------------------------------------
 Syvalion          2       512x400      used     port-based
 Record Breaker    2       320x240     unused    port-based
 Dynamite League   1       320x240     unused   address-based


Known issues :
===============
 - Y coordinate of sprite zooming is non-linear, so currently implemented
   hand-tuned value and this is used for only Record Breaker.
 - Sprite and BG1 priority bit is not understood. It is defined by sprite
   priority in Record Breaker and by zoom value and some scroll value in
   Dynamite League. So, some priority problems still remain.
 - A scroll value of text layer seems to be nonexistence.
 - Background zoom effect is not working in flip screen mode.
 - Sprite zoom is a bit wrong.
 - Title screen of dynamite league is wrong a bit, which is mentioned by
   Yasuhiro Ogawa.


Stephh's notes (based on the game M68000 code and some tests) :

1) 'syvalion'

  - Region stored at 0x07fffe.w
  - Sets :
      * 'syvalion' : region = 0x0000
  - Coinage relies on the region (code at 0x003640) :
      * 0x0000 (Japan), 0x0001 (US), 0x0003 (US, Romstar license) and
        0x0004 (licensed to PHOENIX ELECTRONICS CO.) use TAITO_COINAGE_JAPAN_OLD
      * 0x0002 (World) uses TAITO_COINAGE_WORLD
  - Notice screen only if region = 0x0000
  - Text is always in Japanese regardless of the region !
  - DSW bit 6 has an unknown effect because of a write to unmapped 0x430000.w
    (see code at 0x002af8). Another write to this address is done at 0x002a96.


2) 'recordbr' and 'gogold'

  - Region stored at 0x07fffe.w
  - Title stored at 0x07ff9a.w :
      * 0x0000 : "Go For The Gold"
      * 0x0001 : "Recordbreaker"
  - Sets :
      * 'recordbr' : region = 0x0003 and title = 0x0001
      * 'gogold'   : region = 0x0001 and title = 0x0000
  - Coinage relies on the region (code at 0x00144a) :
      * 0x0001 (Japan), 0x0002 (US), 0x0005 (World, Romstar license) and
        0x0006 (US, Romstar license) use TAITO_COINAGE_JAPAN_OLD
      * 0x0003 (World), 0x0004 (licensed to xxx) and
        0x0007 (licensed to PHOENIX ELECTRONICS CO.) use TAITO_COINAGE_WORLD
  - Notice screen only if region = 0x0000 or region = 0x0001

  - Debug Mode (from Shimapong)
    1. Hold Service Switch at boot (key "9")
    2. Input the following sequence at the "Service Switch Error" message screen:
    1P Start, 1P Start, 1P Start, Service Switch, 1P Start


3) 'dleague'

  - Region stored at 0x05fffe.w
  - Sets :
      * 'dleague' : region = 0x0000
  - Coinage relies on the region (code at 0x0017be) :
      * 0x0000 (Japan) uses TAITO_COINAGE_JAPAN_OLD
      * 0x0001 (US) uses TAITO_COINAGE_US
      * 0x0002 (World) uses TAITO_COINAGE_WORLD
  - Notice screen only if region = 0x0000
  - FBI logo only if region = 0x0001
  - DSWA bit 0 determines if difficulty is constant or if it is based on the inning
    (code at 0x010964 and 0x0109ce + tables at 0x010afe and 0x010b18)
  - When region != 0, DSWB bit 0 has multiple effects
    (code at 0x001208 with $1206,A5 initialised via code at 0x00ae2c);
    here is what I found so far :
      * OFF : you can select your team, you starting pitcher and your batting order pattern,
              but there won't be additional messages ingame
      * ON  : you can't select your team, you starting pitcher and your batting order pattern,
              but there will be additional messages ingame
  - Regardless of the "Extra Credit Needed" settings, additional credit will be needed after 9th inning
    if there is a draw; the game will end if there is still a draw after 12th inning
  - The game will end if computer scores 10 points more than your score ("called game")
  - When you are playing, if you don't touch any joystick nor player button
    for 90 consecutive seconds, the game will end without any warning !
    See BTANB page on MAME Testers site for full report and more details


TODO :
========
 - Implemented BG1 : sprite priority. Currently it is not brought out priority
   bit.
 - Fix sprite coordinates.
 - Improve zoom y coordinate.
 - Text layer scroll if exists.
 - Speed up and clean up the code [some of video routines now in taitoic.c]

Dleague: junky sprites (sometimes) at bottom of screen in
flipscreen.

Recordbr: missing hand of opponent when he ends in swimming
race and you're still on the blocks. Bug?

Recordbr: loads of unmapped IOC reads and writes. Need to map Player 3 & 4 controls.

what is the rom loaded into region user1? (see go gold / dynamite league)
some kind of zoom table?

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "includes/taitoipt.h"
#include "audio/taitosnd.h"
#include "sound/2610intf.h"
#include "includes/taito_h.h"

/***************************************************************************

  Interrupt(s)

***************************************************************************/

/* Handler called by the YM2610 emulator when the internal timers cause an IRQ */
WRITE_LINE_MEMBER(taitoh_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}


/***************************************************************************

  Memory Handler(s)

***************************************************************************/

#define P1TRACKX_PORT_TAG     "P1X"
#define P1TRACKY_PORT_TAG     "P1Y"
#define P2TRACKX_PORT_TAG     "P2X"
#define P2TRACKY_PORT_TAG     "P2Y"

READ8_MEMBER(taitoh_state::syvalion_input_bypass_r)
{
	/* Bypass TC0220IOC controller for analog input */

	UINT8   port = m_tc0220ioc->port_r(space, 0); /* read port number */

	switch( port )
	{
		case 0x08:              /* trackball y coords bottom 8 bits for 2nd player */
			return ioport(P2TRACKY_PORT_TAG)->read();

		case 0x09:              /* trackball y coords top 8 bits for 2nd player */
			if (ioport(P2TRACKY_PORT_TAG)->read() & 0x80)   /* y- direction (negative value) */
				return 0xff;
			else                                                /* y+ direction (positive value) */
				return 0x00;

		case 0x0a:              /* trackball x coords bottom 8 bits for 2nd player */
			return ioport(P2TRACKX_PORT_TAG)->read();

		case 0x0b:              /* trackball x coords top 8 bits for 2nd player */
			if (ioport(P2TRACKX_PORT_TAG)->read() & 0x80)   /* x- direction (negative value) */
				return 0xff;
			else                                                /* x+ direction (positive value) */
				return 0x00;

		case 0x0c:              /* trackball y coords bottom 8 bits for 1st player */
			return ioport(P1TRACKY_PORT_TAG)->read();

		case 0x0d:              /* trackball y coords top 8 bits for 1st player */
			if (ioport(P1TRACKY_PORT_TAG)->read() & 0x80)   /* y- direction (negative value) */
				return 0xff;
			else                                                /* y+ direction (positive value) */
				return 0x00;

		case 0x0e:              /* trackball x coords bottom 8 bits for 1st player */
			return ioport(P1TRACKX_PORT_TAG)->read();

		case 0x0f:              /* trackball x coords top 8 bits for 1st player */
			if (ioport(P1TRACKX_PORT_TAG)->read() & 0x80)   /* x- direction (negative value) */
				return 0xff;
			else                                                /* x+ direction (positive value) */
				return 0x00;

		default:
			return m_tc0220ioc->portreg_r(space, offset);
	}
}

WRITE8_MEMBER(taitoh_state::sound_bankswitch_w)
{
	membank("z80bank")->set_entry(data & 3);
}


/***************************************************************************

  Memory Map(s)

***************************************************************************/

static ADDRESS_MAP_START( syvalion_map, AS_PROGRAM, 16, taitoh_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_MIRROR(0x010000) AM_RAM AM_SHARE("m68000_mainram")
	AM_RANGE(0x200000, 0x200001) AM_READ8(syvalion_input_bypass_r, 0x00ff) AM_DEVWRITE8("tc0220ioc", tc0220ioc_device, portreg_w, 0x00ff)
	AM_RANGE(0x200002, 0x200003) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_device, port_r, port_w, 0x00ff)
	AM_RANGE(0x300000, 0x300001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x00ff)
	AM_RANGE(0x300002, 0x300003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff)
	AM_RANGE(0x400000, 0x420fff) AM_DEVREADWRITE("tc0080vco", tc0080vco_device, word_r, word_w)
	AM_RANGE(0x500800, 0x500fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
ADDRESS_MAP_END

static ADDRESS_MAP_START( recordbr_map, AS_PROGRAM, 16, taitoh_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_MIRROR(0x010000) AM_RAM AM_SHARE("m68000_mainram")
	AM_RANGE(0x200000, 0x200001) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_device, portreg_r, portreg_w, 0x00ff)
	AM_RANGE(0x200002, 0x200003) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_device, port_r, port_w, 0x00ff)
	AM_RANGE(0x300000, 0x300001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x00ff)
	AM_RANGE(0x300002, 0x300003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff)
	AM_RANGE(0x400000, 0x420fff) AM_DEVREADWRITE("tc0080vco", tc0080vco_device, word_r, word_w)
	AM_RANGE(0x500800, 0x500fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
ADDRESS_MAP_END

static ADDRESS_MAP_START( tetristh_map, AS_PROGRAM, 16, taitoh_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_MIRROR(0x010000) AM_RAM AM_SHARE("m68000_mainram")
	AM_RANGE(0x200000, 0x200001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x00ff)
	AM_RANGE(0x200002, 0x200003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff)
	AM_RANGE(0x300000, 0x300001) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_device, portreg_r, portreg_w, 0x00ff)
	AM_RANGE(0x300002, 0x300003) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_device, port_r, port_w, 0x00ff)
	AM_RANGE(0x400000, 0x420fff) AM_DEVREADWRITE("tc0080vco", tc0080vco_device, word_r, word_w)
	AM_RANGE(0x500800, 0x500fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
ADDRESS_MAP_END

static ADDRESS_MAP_START( dleague_map, AS_PROGRAM, 16, taitoh_state )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_MIRROR(0x010000) AM_RAM AM_SHARE("m68000_mainram")
	AM_RANGE(0x200000, 0x20000f) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_device, read, write, 0x00ff)
	AM_RANGE(0x300000, 0x300001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x00ff)
	AM_RANGE(0x300002, 0x300003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff)
	AM_RANGE(0x400000, 0x420fff) AM_DEVREADWRITE("tc0080vco", tc0080vco_device, word_r, word_w)
	AM_RANGE(0x500800, 0x500fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x600000, 0x600001) AM_WRITENOP    /* ?? writes zero once per frame */
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, taitoh_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("z80bank")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
	AM_RANGE(0xe200, 0xe200) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xe201, 0xe201) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0xe400, 0xe403) AM_WRITENOP        /* pan control */
	AM_RANGE(0xea00, 0xea00) AM_READNOP
	AM_RANGE(0xee00, 0xee00) AM_WRITENOP        /* ? */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP        /* ? */
	AM_RANGE(0xf200, 0xf200) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END


/***************************************************************************

  Input Port(s)

***************************************************************************/

static INPUT_PORTS_START( syvalion )
	/* 0x200000 (port 0) -> 0x102842.b (-$57be,A5) */
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)

	/* 0x200000 (port 1) -> 0x102843.b (-$57bd,A5) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "1000k" )
	PORT_DIPSETTING(    0x0c, "1500k" )
	PORT_DIPSETTING(    0x04, "2000k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7") /* code at 0x002af8 - see notes */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START(P1TRACKX_PORT_TAG)
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_RESET PORT_PLAYER(1)

	PORT_START(P1TRACKY_PORT_TAG)
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_RESET PORT_REVERSE PORT_PLAYER(1)

	PORT_START(P2TRACKX_PORT_TAG)
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_RESET PORT_PLAYER(2)

	PORT_START(P2TRACKY_PORT_TAG)
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_RESET PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( syvalionp )
	/* 0x200000 (port 0) -> 0x102842.b (-$57be,A5) */
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)

	/* 0x200000 (port 1) -> 0x102843.b (-$57bd,A5) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "1000k" )
	PORT_DIPSETTING(    0x0c, "1500k" )
	PORT_DIPSETTING(    0x04, "2000k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7") /* code at 0x002af8 - see notes */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	// x and y are swapped on the proto, x also isn't reversed
	PORT_START(P1TRACKX_PORT_TAG)
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_RESET PORT_PLAYER(1)

	PORT_START(P1TRACKY_PORT_TAG)
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_RESET PORT_PLAYER(1)

	PORT_START(P2TRACKX_PORT_TAG)
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_RESET PORT_PLAYER(2)

	PORT_START(P2TRACKY_PORT_TAG)
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_RESET PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( recordbr )
	/* 0x200000 (port 0) -> 0x1022e6.b (-$5d1a,A5) */
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	/* 0x200000 (port 1) -> 0x1022e7.b (-$5d19,A5) */
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )                 /* IPT_START3 in service mode */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )                 /* IPT_START4 in service mode */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )                 /* IPT_BUTTON4 (PL1) in service mode */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )                 /* IPT_BUTTON4 (PL2) in service mode */
INPUT_PORTS_END

static INPUT_PORTS_START( gogold )
	PORT_INCLUDE(recordbr)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( tetristh )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)

	// TODO: verify/complete DSWB
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( dleague )
	/* 0x200000 -> 0x100526.b ($526,A5) */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1") /* see notes */
	PORT_DIPSETTING(    0x01, "Constant" )
	PORT_DIPSETTING(    0x00, "Based on Inning" )
	TAITO_DSWA_BITS_1_TO_3_LOC(SW1)
	TAITO_COINAGE_US_LOC(SW1)

	/* 0x200002 -> 0x100527.b ($527,A5) */
	PORT_START("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )        /* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPNAME( 0x0c, 0x0c, "Extra Credit Needed" )   PORT_DIPLOCATION("SW2:3,4") /* see notes */
	PORT_DIPSETTING(    0x08, "After Inning 6" )
	PORT_DIPSETTING(    0x00, "After Innings 5 and 7" )
	PORT_DIPSETTING(    0x0c, "After Innings 3 and 6" )
	PORT_DIPSETTING(    0x04, "After Innings 3, 5 and 7" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")
	TAITO_JOY_DUAL_UDLR( 1, 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( dleaguej )
	PORT_INCLUDE(dleague)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END


/***************************************************************************

  Machine Driver(s)

***************************************************************************/

static const gfx_layout tilelayout =
{
	16,16,  /* 16x16 pixels */
	32768,  /* 32768 tiles */
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 0x100000*8+4, 0x100000*8, 0x100000*8+12, 0x100000*8+8,
		0x200000*8+4, 0x200000*8, 0x200000*8+12, 0x200000*8+8, 0x300000*8+4, 0x300000*8, 0x300000*8+12, 0x300000*8+8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*16
};

#if 0
static const gfx_layout charlayout =
{
	8, 8,   /* 8x8 pixels */
	256,    /* 256 chars */
	4,      /* 4 bit per pixel */
	{ 0x1000*8 + 8, 0x1000*8, 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7 },
	16*8
};
#endif


static GFXDECODE_START( syvalion )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0,     32 )
GFXDECODE_END

static GFXDECODE_START( recordbr )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0,     32 )
GFXDECODE_END

static GFXDECODE_START( dleague )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0,     32 )
GFXDECODE_END


void taitoh_state::machine_reset()
{
}

void taitoh_state::machine_start()
{
	membank("z80bank")->configure_entries(0, 4, memregion("audiocpu")->base(), 0x4000);
}


static MACHINE_CONFIG_START( syvalion, taitoh_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz / 2)     /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(syvalion_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taitoh_state,  irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz / 2)        /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(sound_map)


	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_DEVICE_ADD("tc0220ioc", TC0220IOC, 0)
	MCFG_TC0220IOC_READ_0_CB(IOPORT("DSWA"))
	MCFG_TC0220IOC_READ_1_CB(IOPORT("DSWB"))
	MCFG_TC0220IOC_READ_2_CB(IOPORT("IN0"))
	MCFG_TC0220IOC_READ_3_CB(IOPORT("IN1"))
	MCFG_TC0220IOC_READ_7_CB(IOPORT("IN2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*16, 64*16)
	MCFG_SCREEN_VISIBLE_AREA(0*16, 32*16-1, 3*16, 28*16-1)
	MCFG_SCREEN_UPDATE_DRIVER(taitoh_state, screen_update_syvalion)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", syvalion)
	MCFG_PALETTE_ADD("palette", 33*16)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("tc0080vco", TC0080VCO, 0)
	MCFG_TC0080VCO_GFX_REGION(0)
	MCFG_TC0080VCO_TX_REGION(1)
	MCFG_TC0080VCO_OFFSETS(1, 1)
	MCFG_TC0080VCO_BGFLIP_OFFS(-2)
	MCFG_TC0080VCO_GFXDECODE("gfxdecode")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2610, XTAL_8MHz)
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(taitoh_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
	MCFG_SOUND_ROUTE(2, "mono", 1.0)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( recordbr, taitoh_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz / 2)     /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(recordbr_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taitoh_state,  irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz / 2)        /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)


	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_DEVICE_ADD("tc0220ioc", TC0220IOC, 0)
	MCFG_TC0220IOC_READ_0_CB(IOPORT("DSWA"))
	MCFG_TC0220IOC_READ_1_CB(IOPORT("DSWB"))
	MCFG_TC0220IOC_READ_2_CB(IOPORT("IN0"))
	MCFG_TC0220IOC_READ_3_CB(IOPORT("IN1"))
	MCFG_TC0220IOC_READ_7_CB(IOPORT("IN2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*16, 64*16)
	MCFG_SCREEN_VISIBLE_AREA(1*16, 21*16-1, 2*16, 17*16-1)
	MCFG_SCREEN_UPDATE_DRIVER(taitoh_state, screen_update_recordbr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", recordbr)
	MCFG_PALETTE_ADD("palette", 32*16)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("tc0080vco", TC0080VCO, 0)
	MCFG_TC0080VCO_GFX_REGION(0)
	MCFG_TC0080VCO_TX_REGION(1)
	MCFG_TC0080VCO_OFFSETS(1, 1)
	MCFG_TC0080VCO_BGFLIP_OFFS(-2)
	MCFG_TC0080VCO_GFXDECODE("gfxdecode")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2610, XTAL_8MHz)
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(taitoh_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
	MCFG_SOUND_ROUTE(2, "mono", 1.0)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( tetristh, recordbr )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(tetristh_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( dleague, taitoh_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz / 2)     /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(dleague_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taitoh_state,  irq1_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz / 2)        /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(sound_map)


	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_DEVICE_ADD("tc0220ioc", TC0220IOC, 0)
	MCFG_TC0220IOC_READ_0_CB(IOPORT("DSWA"))
	MCFG_TC0220IOC_READ_1_CB(IOPORT("DSWB"))
	MCFG_TC0220IOC_READ_2_CB(IOPORT("IN0"))
	MCFG_TC0220IOC_READ_3_CB(IOPORT("IN1"))
	MCFG_TC0220IOC_READ_7_CB(IOPORT("IN2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*16, 64*16)
	MCFG_SCREEN_VISIBLE_AREA(1*16, 21*16-1, 2*16, 17*16-1)
	MCFG_SCREEN_UPDATE_DRIVER(taitoh_state, screen_update_dleague)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dleague)
	MCFG_PALETTE_ADD("palette", 33*16)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("tc0080vco", TC0080VCO, 0)
	MCFG_TC0080VCO_GFX_REGION(0)
	MCFG_TC0080VCO_TX_REGION(1)
	MCFG_TC0080VCO_OFFSETS(1, 1)
	MCFG_TC0080VCO_BGFLIP_OFFS(-2)
	MCFG_TC0080VCO_GFXDECODE("gfxdecode")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2610, XTAL_8MHz)
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(taitoh_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
	MCFG_SOUND_ROUTE(2, "mono", 1.0)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( syvalion )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* main cpu */
	ROM_LOAD16_BYTE( "b51-20.bin", 0x00000, 0x20000, CRC(440b6418) SHA1(262b65f39eb13c11ae7b87013951097ab0a9cb63) )
	ROM_LOAD16_BYTE( "b51-22.bin", 0x00001, 0x20000, CRC(e6c61079) SHA1(b786ef1bfc72706347c12c17616652bc8302a98c) )
	ROM_LOAD16_BYTE( "b51-19.bin", 0x40000, 0x20000, CRC(2abd762c) SHA1(97cdb9f1dba5b11b96b5d3431937669de5220512) )
	ROM_LOAD16_BYTE( "b51-21.bin", 0x40001, 0x20000, CRC(aa111f30) SHA1(77da4a8db49999f5fa2cf0209028d0f70e26dfe3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "b51-23.bin", 0x00000, 0x10000, CRC(734662de) SHA1(0058d6de68f26cd58b9eb8859e15f3ced6bd3489) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b51-16.bin", 0x000000, 0x20000, CRC(c0fcf7a5) SHA1(4550ba6d822ba12ad39576bcbed09b5fa54279e8) )
	ROM_LOAD16_BYTE( "b51-12.bin", 0x000001, 0x20000, CRC(6b36d358) SHA1(4101c110e99fe2ac1a989c84857f6438439b79a1) )
	ROM_LOAD16_BYTE( "b51-15.bin", 0x040000, 0x20000, CRC(30b2ee02) SHA1(eacd179c8760ce9ba01e234dfd3f159773e4f2ab) )
	ROM_LOAD16_BYTE( "b51-11.bin", 0x040001, 0x20000, CRC(ae9a9ac5) SHA1(f1f5216e51fea3173f5317e0dda404a29b2c45fe) )
	ROM_LOAD16_BYTE( "b51-08.bin", 0x100000, 0x20000, CRC(9f6a535c) SHA1(40d52d3f572dd87b41d89707a2ec189760d806b0) )
	ROM_LOAD16_BYTE( "b51-04.bin", 0x100001, 0x20000, CRC(03aea658) SHA1(439f08948e57c9a0f450d1319e3bc99c6fd4f82d) )
	ROM_LOAD16_BYTE( "b51-07.bin", 0x140000, 0x20000, CRC(764d4dc8) SHA1(700de70134ade3901dad51d4bf14d91f92bc5381) )
	ROM_LOAD16_BYTE( "b51-03.bin", 0x140001, 0x20000, CRC(8fd9b299) SHA1(3dc2a66678dfa13f2264bb4f5ca8a31477cc59ff) )
	ROM_LOAD16_BYTE( "b51-14.bin", 0x200000, 0x20000, CRC(dea7216e) SHA1(b97d08b24a3dd9b061ef118fd6d8b3edfa3a3008) )
	ROM_LOAD16_BYTE( "b51-10.bin", 0x200001, 0x20000, CRC(6aa97fbc) SHA1(d546dd5a276cce36e879bb7bfabbdd63d36c0f72) )
	ROM_LOAD16_BYTE( "b51-13.bin", 0x240000, 0x20000, CRC(dab28958) SHA1(da7e7fdd1d1e5a4d72b5e7df235fc304f77fa2c9) )
	ROM_LOAD16_BYTE( "b51-09.bin", 0x240001, 0x20000, CRC(cbb4f33d) SHA1(6c6560603f7fd5578a866b11031d8480bc4a9eee) )
	ROM_LOAD16_BYTE( "b51-06.bin", 0x300000, 0x20000, CRC(81bef4f0) SHA1(83b3a762b6df6f6ca193e639116345a20f874636) )
	ROM_LOAD16_BYTE( "b51-02.bin", 0x300001, 0x20000, CRC(906ba440) SHA1(9a1a147caf7eac534e739b8ad60f0c71514a64c7) )
	ROM_LOAD16_BYTE( "b51-05.bin", 0x340000, 0x20000, CRC(47976ae9) SHA1(a2b19a39d8968b886412a85c082806917e02d9fd) )
	ROM_LOAD16_BYTE( "b51-01.bin", 0x340001, 0x20000, CRC(8dab004a) SHA1(1772cdcb9d0ca5ebf429f371c041b9ae12fafcd0) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )    /* samples */
	ROM_LOAD( "b51-18.bin", 0x00000, 0x80000, CRC(8b23ac83) SHA1(340b9e7f09c1809a332b41d3fb579f5f8cd6367f) )

	ROM_REGION( 0x80000, "ymsnd", 0 )   /* samples */
	ROM_LOAD( "b51-17.bin", 0x00000, 0x80000, CRC(d85096aa) SHA1(dac39ed182e9eda06575f1667c4c1ff9a4a56599) )
ROM_END



ROM_START( syvalionp )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* main cpu */
	ROM_LOAD16_BYTE( "prg-1e.ic28", 0x00000, 0x20000, CRC(c778005b) SHA1(db918c8a0f05e3ea8db6cfd40026b580e937d626) )
	ROM_LOAD16_BYTE( "prg-0e.ic31", 0x00001, 0x20000, CRC(5a484040) SHA1(ecff75e420299da85ee50d17a079804b67ae1d4e) )
	ROM_LOAD16_BYTE( "prg-3e.ic27", 0x40000, 0x20000, CRC(0babb15b) SHA1(8673b0eed34af58e0b0253fdbd5c081bebe4dc10) )
	ROM_LOAD16_BYTE( "prg-2e.ic30", 0x40001, 0x20000, CRC(f4aacaa9) SHA1(6145afe0f4aad48f14dabdd3c29f76dfa746d863) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "c69b.ic58", 0x00000, 0x10000, CRC(07d3d789) SHA1(dbbe308f74637bb5a2651654bbada6a07f99ae14) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "chr-00.ic16", 0x000000, 0x20000, CRC(b0c66db7) SHA1(e3a1e9b0d6157e5085a55fdac1daa61f5a03b048) )
	ROM_LOAD16_BYTE( "chr-01.ic12", 0x000001, 0x20000, CRC(dd07db12) SHA1(76317c27b1e649d73f639b565b67f42af0233118) )
	ROM_LOAD16_BYTE( "chr-10.ic15", 0x040000, 0x20000, CRC(c8942dde) SHA1(811d77359c6445f721d70d9c37c51d76d5f82a83) )
	ROM_LOAD16_BYTE( "chr-11.ic11", 0x040001, 0x20000, CRC(fdaa72f5) SHA1(2a351b525cdb4bf421b874478b0489f793129f62) )
	ROM_LOAD16_BYTE( "chr-02.ic8",  0x100000, 0x20000, CRC(323a9ad9) SHA1(7ae6fa2dcc2078ae64d6d28cadacb52c6a069575) )
	ROM_LOAD16_BYTE( "chr-03.ic4",  0x100001, 0x20000, CRC(5ab28400) SHA1(c2ba0419cfa69a2bff8a1241162bc7ca6ae73b1d) )
	ROM_LOAD16_BYTE( "chr-12.ic7",  0x140000, 0x20000, CRC(094a5e0b) SHA1(864e26ddd41e2e8e53de0fd2e18ffac6263cfee2) )
	ROM_LOAD16_BYTE( "chr-13.ic3",  0x140001, 0x20000, CRC(cf39cf1d) SHA1(bd757206bad792befffd0cae8704cb746493fb7f) )
	ROM_LOAD16_BYTE( "chr-04.ic14", 0x200000, 0x20000, CRC(dd2ea978) SHA1(8b772e2658265eb0eeb4bb4db8c260df43231f02) )
	ROM_LOAD16_BYTE( "chr-05.ic10", 0x200001, 0x20000, CRC(1c305d4e) SHA1(7057cc74b390959f0ff5fa59e60d66e9b7d60f2b) )
	ROM_LOAD16_BYTE( "chr-14.ic13", 0x240000, 0x20000, CRC(083806c3) SHA1(443b2cfb10c34b260e339f6b5e8fc005118a70f5) )
	ROM_LOAD16_BYTE( "chr-15.ic9",  0x240001, 0x20000, CRC(6afb076e) SHA1(d806602164f9bc9ffd1787d9dfba9719a190833f) )
	ROM_LOAD16_BYTE( "chr-06.ic6",  0x300000, 0x20000, CRC(00cd0493) SHA1(151d9176ebe5385d26a6f3fc4a409ecdb87acaf0) )
	ROM_LOAD16_BYTE( "chr-07.ic2",  0x300001, 0x20000, CRC(58fb0f65) SHA1(4a11c99239749d3c8ea3709788b8b648a9e9c51a) )
	ROM_LOAD16_BYTE( "chr-16.ic5",  0x340000, 0x20000, CRC(a169194e) SHA1(1399c95fd32f93808b8f2cf95809c3bf15521729) )
	ROM_LOAD16_BYTE( "chr-17.ic7",  0x340001, 0x20000, CRC(c259bd61) SHA1(cb17be30e2f330d979b3f2a4b691a3a0bb336a44) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )    /* samples */
	ROM_LOAD( "sb-00.ic6",      0x00000, 0x20000, CRC(5188f459) SHA1(261bdfe9f4200f0296f78c086934700df997875d) )
	ROM_LOAD( "sb01.ic8",       0x20000, 0x20000, CRC(4dab7a6b) SHA1(7991313e10ee504454e391a4e6cf603ad2cfc0e4) )
	ROM_LOAD( "sb-02.ic7",      0x40000, 0x20000, CRC(8f5cc936) SHA1(df76ab9a38eef0726d74fcb0d951b0b5065345a7) )
	ROM_LOAD( "sb-03-e66a.ic9", 0x60000, 0x20000, CRC(9013b407) SHA1(eea010fa32fb3e5270246248083d4aa9449cf6b1) )

	ROM_REGION( 0x80000, "ymsnd", 0 )   /* samples */
	ROM_LOAD( "sa-00.ic1", 0x00000, 0x20000, CRC(27a97abc) SHA1(891cfafe9a460dbaa83711dcd7ab1bf6d8922b4d) )
	ROM_LOAD( "sa-01.ic2", 0x20000, 0x20000, CRC(0140452b) SHA1(aa2e664ffc501b5c53fb3fe75b205f18cfe0f67e) )
	ROM_LOAD( "sa-02.ic3", 0x40000, 0x20000, CRC(970cd4ee) SHA1(674ee5ec0c51e6303baef62c1c4582ef6d7a9590) )
	ROM_LOAD( "sa-03.ic4", 0x60000, 0x20000, CRC(936cd1b5) SHA1(e8b9e8e867a2d08b0004850359872c174ed687af) )

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "cpu1-pal20l10a.ic38.bin", 0x00000, 0xcc, CRC(2e7b5e3f) SHA1(79bd46842ee9330e8545980b94d49f8f728279c1) )
	ROM_LOAD( "cpu2-pal20l8a.ic39.bin",  0x00000, 0x144, CRC(c0abf131) SHA1(58de646f26652f0e2abc3470612c230f4a365699) )
ROM_END



ROM_START( recordbr )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* main cpu */
	ROM_LOAD16_BYTE( "b56-17.bin", 0x00000, 0x20000, CRC(3e0a9c35) SHA1(900a741b2abbbbe883b9d78162a88b4397af1a56) )
	ROM_LOAD16_BYTE( "b56-16.bin", 0x00001, 0x20000, CRC(b447f12c) SHA1(58ee30337836f260c7fbda728dac93f06d861ec4) )
	ROM_LOAD16_BYTE( "b56-15.bin", 0x40000, 0x20000, CRC(b346e282) SHA1(f6b4a2e9093a33d19c2eaf3ef9801179f39a83a3) )
	ROM_LOAD16_BYTE( "b56-21.bin", 0x40001, 0x20000, CRC(e5f63790) SHA1(b81db7690a989146c438609d9633ddcb1fd219dd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "b56-19.bin", 0x00000, 0x10000, CRC(c68085ee) SHA1(78634216a622a08c20dae0422283c4a7ed360546) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b56-04.bin", 0x000000, 0x20000, CRC(f7afdff0) SHA1(8f8ea0e8da20913426ff3b58d7bb63bd352d3fb4) )
	ROM_LOAD16_BYTE( "b56-08.bin", 0x000001, 0x20000, CRC(c9f0d38a) SHA1(aa22f1a06e00f90c546eebcd8b42da3e3c7d0781) )
	ROM_LOAD16_BYTE( "b56-03.bin", 0x100000, 0x20000, CRC(4045fd44) SHA1(a84be9eedba7aed30d4f2841016784f8024d9443) )
	ROM_LOAD16_BYTE( "b56-07.bin", 0x100001, 0x20000, CRC(0c76e4c8) SHA1(e50d1bd6e8ec967ba03bd14097a9bd560aa2decc) )
	ROM_LOAD16_BYTE( "b56-02.bin", 0x200000, 0x20000, CRC(68c604ec) SHA1(75b26bfa53efa63b9c7a026f4226213364550cad) )
	ROM_LOAD16_BYTE( "b56-06.bin", 0x200001, 0x20000, CRC(5fbcd302) SHA1(22e7d835643945d501edc693dbe4efc8d4d074a7) )
	ROM_LOAD16_BYTE( "b56-01.bin", 0x300000, 0x20000, CRC(766b7260) SHA1(f7d7176af614f06e8c66e890e4d194ffb6f7af73) )
	ROM_LOAD16_BYTE( "b56-05.bin", 0x300001, 0x20000, CRC(ed390378) SHA1(0275e5ead206028bfcff7ecbe11c7ab961e648ea) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )    /* samples */
	ROM_LOAD( "b56-09.bin", 0x00000, 0x80000, CRC(7fd9ee68) SHA1(edc4455b3f6a6f30f418d03c6e53af875542a325) )

	ROM_REGION( 0x80000, "ymsnd", 0 )   /* samples */
	ROM_LOAD( "b56-10.bin", 0x00000, 0x80000, CRC(de1bce59) SHA1(aa3aea30d6f53e60d9a0d4ec767e1b261d5efc8a) )

	ROM_REGION( 0x02000, "user1", 0 ) /* zoom table / mixing? */
	ROM_LOAD( "b56-18.bin", 0x00000, 0x02000, CRC(c88f0bbe) SHA1(18c87c744fbeca35d13033e50f62e5383eb4ec2c) )
ROM_END

ROM_START( gogold )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* main cpu */
	ROM_LOAD16_BYTE( "b56-17.bin", 0x00000, 0x20000, CRC(3e0a9c35) SHA1(900a741b2abbbbe883b9d78162a88b4397af1a56) )
	ROM_LOAD16_BYTE( "b56-16.bin", 0x00001, 0x20000, CRC(b447f12c) SHA1(58ee30337836f260c7fbda728dac93f06d861ec4) )
	ROM_LOAD16_BYTE( "b56-15.bin", 0x40000, 0x20000, CRC(b346e282) SHA1(f6b4a2e9093a33d19c2eaf3ef9801179f39a83a3) )
	ROM_LOAD16_BYTE( "b56-14.bin", 0x40001, 0x20000, CRC(b6c195b9) SHA1(80541d9a686fdc1850d764d8e00ba03526e7174c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "b56-19.bin", 0x00000, 0x10000, CRC(c68085ee) SHA1(78634216a622a08c20dae0422283c4a7ed360546) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b56-04.bin", 0x000000, 0x20000, CRC(f7afdff0) SHA1(8f8ea0e8da20913426ff3b58d7bb63bd352d3fb4) )
	ROM_LOAD16_BYTE( "b56-08.bin", 0x000001, 0x20000, CRC(c9f0d38a) SHA1(aa22f1a06e00f90c546eebcd8b42da3e3c7d0781) )
	ROM_LOAD16_BYTE( "b56-03.bin", 0x100000, 0x20000, CRC(4045fd44) SHA1(a84be9eedba7aed30d4f2841016784f8024d9443) )
	ROM_LOAD16_BYTE( "b56-07.bin", 0x100001, 0x20000, CRC(0c76e4c8) SHA1(e50d1bd6e8ec967ba03bd14097a9bd560aa2decc) )
	ROM_LOAD16_BYTE( "b56-02.bin", 0x200000, 0x20000, CRC(68c604ec) SHA1(75b26bfa53efa63b9c7a026f4226213364550cad) )
	ROM_LOAD16_BYTE( "b56-06.bin", 0x200001, 0x20000, CRC(5fbcd302) SHA1(22e7d835643945d501edc693dbe4efc8d4d074a7) )
	ROM_LOAD16_BYTE( "b56-01.bin", 0x300000, 0x20000, CRC(766b7260) SHA1(f7d7176af614f06e8c66e890e4d194ffb6f7af73) )
	ROM_LOAD16_BYTE( "b56-05.bin", 0x300001, 0x20000, CRC(ed390378) SHA1(0275e5ead206028bfcff7ecbe11c7ab961e648ea) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )    /* samples */
	ROM_LOAD( "b56-09.bin", 0x00000, 0x80000, CRC(7fd9ee68) SHA1(edc4455b3f6a6f30f418d03c6e53af875542a325) )

	ROM_REGION( 0x80000, "ymsnd", 0 )   /* samples */
	ROM_LOAD( "b56-10.bin", 0x00000, 0x80000, CRC(de1bce59) SHA1(aa3aea30d6f53e60d9a0d4ec767e1b261d5efc8a) )

	ROM_REGION( 0x02000, "user1", 0 ) /* zoom table / mixing? */
	ROM_LOAD( "b56-18.bin", 0x00000, 0x02000, CRC(c88f0bbe) SHA1(18c87c744fbeca35d13033e50f62e5383eb4ec2c) )
ROM_END

// Sega Tetris on a Taito H-System board, with some roms from ?Go For The Gold? still on the board.
ROM_START( tetristh )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* main cpu */
	ROM_LOAD16_BYTE( "c26-12-1.ic36", 0x00000, 0x20000, CRC(77e80c82) SHA1(840dc5a54a865b8cd2e0d03001a493987d66c23b) )
	ROM_LOAD16_BYTE( "c26-11-1.ic18", 0x00001, 0x20000, CRC(069d77d2) SHA1(06c229d1b335797fcd2ac8df09ba3da11e3e43f7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "c26-13.ic56", 0x00000, 0x10000, CRC(efa89dfa) SHA1(556e77c63cb95e441ea1d1beb3d43c61a48a3bb1) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "c26-04.ic51", 0x000000, 0x20000, CRC(23ddf00f) SHA1(f7bb19db62d5e6cb27a6e98db68c54c01e34b776) )
	ROM_LOAD16_BYTE( "c26-08.ic65", 0x000001, 0x20000, CRC(86071824) SHA1(00ba24b35ad93aa0f29e05068ddc9276f2d333af) )
	ROM_LOAD16_BYTE( "c26-03.ic50", 0x100000, 0x20000, CRC(341be9ac) SHA1(a93c55cb20cb0433855ceb125bffcbcecb0e552d) )
	ROM_LOAD16_BYTE( "c26-07.ic64", 0x100001, 0x20000, CRC(c236311f) SHA1(9c292083064793a99887fe2de25a169c1af73432) )
	ROM_LOAD16_BYTE( "c26-02.ic49", 0x200000, 0x20000, CRC(0b0bc88f) SHA1(bf9707beed1eb553e6603c88a65d7365cd66e8ab) )
	ROM_LOAD16_BYTE( "c26-06.ic63", 0x200001, 0x20000, CRC(deae0394) SHA1(6072e04a5d9422c1f8ae88efcfc97659bef2aa99) )
	ROM_LOAD16_BYTE( "c26-01.ic48", 0x300000, 0x20000, CRC(7efc7311) SHA1(ee3357dfc77eb4b9af846deaf89c910fc25c9f12) )
	ROM_LOAD16_BYTE( "c26-05.ic62", 0x300001, 0x20000, CRC(12718d97) SHA1(5c7a79d45ee38a16d7ed70fbe3303f415d6af986) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )    /* samples */
	ROM_LOAD( "b56-09.bin", 0x00000, 0x80000, CRC(7fd9ee68) SHA1(edc4455b3f6a6f30f418d03c6e53af875542a325) )

	ROM_REGION( 0x80000, "ymsnd", 0 )   /* samples */
	ROM_LOAD( "b56-10.bin", 0x00000, 0x80000, CRC(de1bce59) SHA1(aa3aea30d6f53e60d9a0d4ec767e1b261d5efc8a) )

	ROM_REGION( 0x02000, "user1", 0 ) /* zoom table / mixing? */
	ROM_LOAD( "b56-18.bin", 0x00000, 0x02000, CRC(c88f0bbe) SHA1(18c87c744fbeca35d13033e50f62e5383eb4ec2c) )
ROM_END

ROM_START( dleague )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c02-xx.33", 0x00000, 0x20000, CRC(eda870a7) SHA1(1749a6eb80c137b042d4935fcb9cff1ca50d4397) ) /* Need to verify proper Taito chip number */
	ROM_LOAD16_BYTE( "c02-xx.36", 0x00001, 0x20000, CRC(f52114af) SHA1(d500933852332a9463a5738cf5890625c9b35d65) ) /* Need to verify proper Taito chip number */
	ROM_LOAD16_BYTE( "c02-20.34", 0x40000, 0x10000, CRC(cdf593f3) SHA1(6afbd9d8d74e6801dc991eb9fd3205057747b986) )
	ROM_LOAD16_BYTE( "c02-xx.37", 0x40001, 0x10000, CRC(820a8241) SHA1(a1b75e76f6806d5cbdb97f59d29aa846a6f3bb8b) ) /* Need to verify proper Taito chip number */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c02-23.40", 0x00000, 0x10000, CRC(5632ee49) SHA1(90dedaf40ab526529cd7d569b78a9d5451ec3e25) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD       ( "c02-02.15", 0x000000, 0x80000, CRC(b273f854) SHA1(5961b9fe2c49fb05f5bc3e27e05925dbef8577e9) )
	ROM_LOAD16_BYTE( "c02-06.6",  0x080000, 0x20000, CRC(b8473c7b) SHA1(8fe8d838bdba7aaaf4527ac1c5c16604922bb245) )
	ROM_LOAD16_BYTE( "c02-10.14", 0x080001, 0x20000, CRC(50c02f0f) SHA1(7d13b798c0a98387719ab738b9178ee6079327b2) )
	ROM_LOAD       ( "c02-03.17", 0x100000, 0x80000, CRC(c3fd0dcd) SHA1(43f32cefbca203bd0453e1c3d4523f0834900418) )
	ROM_LOAD16_BYTE( "c02-07.7",  0x180000, 0x20000, CRC(8c1e3296) SHA1(088b028189131186c82c61c38d5a936a0cc9830f) )
	ROM_LOAD16_BYTE( "c02-11.16", 0x180001, 0x20000, CRC(fbe548b8) SHA1(c2b453fc213c21d118810b8502836e7a2ba5626b) )
	ROM_LOAD       ( "c02-24.19", 0x200000, 0x80000, CRC(18ef740a) SHA1(27f0445c053e28267e5688627d4f91d158d4fb07) )
	ROM_LOAD16_BYTE( "c02-08.8",  0x280000, 0x20000, CRC(1a3c2f93) SHA1(0e45f8211dae8e17e67d26173262ca9831ccd283) )
	ROM_LOAD16_BYTE( "c02-12.18", 0x280001, 0x20000, CRC(b1c151c5) SHA1(3fc3d4270cad52c4a82c217b452e534d24bd8548) )
	ROM_LOAD       ( "c02-05.21", 0x300000, 0x80000, CRC(fe3a5179) SHA1(34a98969c553ee8c52aeb4fb09670a4ad572446e) )
	ROM_LOAD16_BYTE( "c02-09.9",  0x380000, 0x20000, CRC(a614d234) SHA1(dc68a6a8cf89ab82edc571853249643aa304d37f) )
	ROM_LOAD16_BYTE( "c02-13.20", 0x380001, 0x20000, CRC(8eb3194d) SHA1(98290f77a03826cdf7c8238dd35da1f9349d5cf5) )

	ROM_REGION( 0x02000, "user1", 0 ) /* zoom table / mixing? */
	ROM_LOAD( "c02-18.22", 0x00000, 0x02000, CRC(c88f0bbe) SHA1(18c87c744fbeca35d13033e50f62e5383eb4ec2c) )

	ROM_REGION( 0x80000, "ymsnd", 0 )   /* samples */
	ROM_LOAD( "c02-01.31", 0x00000, 0x80000, CRC(d5a3d1aa) SHA1(544f807015b5d854a4d8cb73e4dbae4b953fd440) )
ROM_END

ROM_START( dleaguej )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c02-19a.33", 0x00000, 0x20000, CRC(7e904e45) SHA1(04ac470c973753e71fba3998099a88ab0e6fcbab) )
	ROM_LOAD16_BYTE( "c02-21a.36", 0x00001, 0x20000, CRC(18c8a32b) SHA1(507cd7a83dcb6eaefa52f2661b9f3a6fabbfbd46) )
	ROM_LOAD16_BYTE( "c02-20.34",  0x40000, 0x10000, CRC(cdf593f3) SHA1(6afbd9d8d74e6801dc991eb9fd3205057747b986) )
	ROM_LOAD16_BYTE( "c02-22.37",  0x40001, 0x10000, CRC(f50db2d7) SHA1(4f16cc42469f1e5bf6dc1aee0919712db089f9cc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c02-23.40", 0x00000, 0x10000, CRC(5632ee49) SHA1(90dedaf40ab526529cd7d569b78a9d5451ec3e25) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD       ( "c02-02.15", 0x000000, 0x80000, CRC(b273f854) SHA1(5961b9fe2c49fb05f5bc3e27e05925dbef8577e9) )
	ROM_LOAD16_BYTE( "c02-06.6",  0x080000, 0x20000, CRC(b8473c7b) SHA1(8fe8d838bdba7aaaf4527ac1c5c16604922bb245) )
	ROM_LOAD16_BYTE( "c02-10.14", 0x080001, 0x20000, CRC(50c02f0f) SHA1(7d13b798c0a98387719ab738b9178ee6079327b2) )
	ROM_LOAD       ( "c02-03.17", 0x100000, 0x80000, CRC(c3fd0dcd) SHA1(43f32cefbca203bd0453e1c3d4523f0834900418) )
	ROM_LOAD16_BYTE( "c02-07.7",  0x180000, 0x20000, CRC(8c1e3296) SHA1(088b028189131186c82c61c38d5a936a0cc9830f) )
	ROM_LOAD16_BYTE( "c02-11.16", 0x180001, 0x20000, CRC(fbe548b8) SHA1(c2b453fc213c21d118810b8502836e7a2ba5626b) )
	ROM_LOAD       ( "c02-24.19", 0x200000, 0x80000, CRC(18ef740a) SHA1(27f0445c053e28267e5688627d4f91d158d4fb07) )
	ROM_LOAD16_BYTE( "c02-08.8",  0x280000, 0x20000, CRC(1a3c2f93) SHA1(0e45f8211dae8e17e67d26173262ca9831ccd283) )
	ROM_LOAD16_BYTE( "c02-12.18", 0x280001, 0x20000, CRC(b1c151c5) SHA1(3fc3d4270cad52c4a82c217b452e534d24bd8548) )
	ROM_LOAD       ( "c02-05.21", 0x300000, 0x80000, CRC(fe3a5179) SHA1(34a98969c553ee8c52aeb4fb09670a4ad572446e) )
	ROM_LOAD16_BYTE( "c02-09.9",  0x380000, 0x20000, CRC(a614d234) SHA1(dc68a6a8cf89ab82edc571853249643aa304d37f) )
	ROM_LOAD16_BYTE( "c02-13.20", 0x380001, 0x20000, CRC(8eb3194d) SHA1(98290f77a03826cdf7c8238dd35da1f9349d5cf5) )

	ROM_REGION( 0x02000, "user1", 0 ) /* zoom table / mixing? */
	ROM_LOAD( "c02-18.22", 0x00000, 0x02000, CRC(c88f0bbe) SHA1(18c87c744fbeca35d13033e50f62e5383eb4ec2c) )

	ROM_REGION( 0x80000, "ymsnd", 0 )   /* samples */
	ROM_LOAD( "c02-01.31", 0x00000, 0x80000, CRC(d5a3d1aa) SHA1(544f807015b5d854a4d8cb73e4dbae4b953fd440) )
ROM_END


/*  ( YEAR  NAME      PARENT    MACHINE   INPUT     INIT              MONITOR  COMPANY                      FULLNAME */
GAME( 1988, syvalion, 0,        syvalion, syvalion, driver_device, 0, ROT0,    "Taito Corporation",         "Syvalion (Japan)",         MACHINE_SUPPORTS_SAVE )
GAME( 1988, syvalionp, syvalion,syvalion, syvalionp,driver_device, 0, ROT0,    "Taito Corporation",         "Syvalion (World, prototype)",         MACHINE_SUPPORTS_SAVE )
GAME( 1988, recordbr, 0,        recordbr, recordbr, driver_device, 0, ROT0,    "Taito Corporation Japan",   "Recordbreaker (World)",    MACHINE_SUPPORTS_SAVE )
GAME( 1988, gogold,   recordbr, recordbr, gogold,   driver_device, 0, ROT0,    "Taito Corporation",         "Go For The Gold (Japan)",  MACHINE_SUPPORTS_SAVE )
GAME( 1988, tetristh, tetris,   tetristh, tetristh, driver_device, 0, ROT0,    "Sega",                      "Tetris (Japan, Taito H-System)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, dleague,  0,        dleague,  dleague,  driver_device, 0, ROT0,    "Taito America Corporation", "Dynamite League (US)",     MACHINE_SUPPORTS_SAVE )
GAME( 1990, dleaguej, dleague,  dleague,  dleaguej, driver_device, 0, ROT0,    "Taito Corporation",         "Dynamite League (Japan)",  MACHINE_SUPPORTS_SAVE )
