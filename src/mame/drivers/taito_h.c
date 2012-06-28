/***************************************************************************

Taito H system
----------------------------
driver by Yochizo

This driver is heavily dependent on the Raine source.
Very thanks to Richard Bush and the Raine team. Also,
I have been given a lot of helpful informations by
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
#include "machine/taitoio.h"
#include "video/taitoic.h"
#include "sound/2610intf.h"
#include "includes/taito_h.h"

/***************************************************************************

  Interrupt(s)

***************************************************************************/

/* Handler called by the YM2610 emulator when the internal timers cause an IRQ */
static void irqhandler( device_t *device, int irq )
{
	taitoh_state *state = device->machine().driver_data<taitoh_state>();
	device_set_input_line(state->m_audiocpu, 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2610_interface ym2610_config =
{
	irqhandler
};


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

	UINT8	port = tc0220ioc_port_r(m_tc0220ioc, 0);	/* read port number */

	switch( port )
	{
		case 0x08:				/* trackball y coords bottom 8 bits for 2nd player */
			return ioport(P2TRACKY_PORT_TAG)->read();

		case 0x09:				/* trackball y coords top 8 bits for 2nd player */
			if (ioport(P2TRACKY_PORT_TAG)->read() & 0x80)	/* y- direction (negative value) */
				return 0xff;
			else												/* y+ direction (positive value) */
				return 0x00;

		case 0x0a:				/* trackball x coords bottom 8 bits for 2nd player */
			return ioport(P2TRACKX_PORT_TAG)->read();

		case 0x0b:				/* trackball x coords top 8 bits for 2nd player */
			if (ioport(P2TRACKX_PORT_TAG)->read() & 0x80)	/* x- direction (negative value) */
				return 0xff;
			else												/* x+ direction (positive value) */
				return 0x00;

		case 0x0c:				/* trackball y coords bottom 8 bits for 1st player */
			return ioport(P1TRACKY_PORT_TAG)->read();

		case 0x0d:				/* trackball y coords top 8 bits for 1st player */
			if (ioport(P1TRACKY_PORT_TAG)->read() & 0x80)	/* y- direction (negative value) */
				return 0xff;
			else												/* y+ direction (positive value) */
				return 0x00;

		case 0x0e:				/* trackball x coords bottom 8 bits for 1st player */
			return ioport(P1TRACKX_PORT_TAG)->read();

		case 0x0f:				/* trackball x coords top 8 bits for 1st player */
			if (ioport(P1TRACKX_PORT_TAG)->read() & 0x80)	/* x- direction (negative value) */
				return 0xff;
			else												/* x+ direction (positive value) */
				return 0x00;

		default:
			return tc0220ioc_portreg_r(m_tc0220ioc, offset);
	}
}

static void reset_sound_region(running_machine &machine)
{
	taitoh_state *state = machine.driver_data<taitoh_state>();
	state->membank("bank1")->set_entry(state->m_banknum);
}

WRITE8_MEMBER(taitoh_state::sound_bankswitch_w)
{
	m_banknum = data & 3;
	reset_sound_region(machine());
}


/***************************************************************************

  Memory Map(s)

***************************************************************************/

static ADDRESS_MAP_START( syvalion_map, AS_PROGRAM, 16, taitoh_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_MIRROR(0x010000) AM_RAM AM_SHARE("m68000_mainram")
	AM_RANGE(0x200000, 0x200001) AM_READ8(syvalion_input_bypass_r, 0x00ff) AM_DEVWRITE8_LEGACY("tc0220ioc", tc0220ioc_portreg_w, 0x00ff)
	AM_RANGE(0x200002, 0x200003) AM_DEVREADWRITE8_LEGACY("tc0220ioc", tc0220ioc_port_r, tc0220ioc_port_w, 0x00ff)
	AM_RANGE(0x300000, 0x300001) AM_READNOP AM_DEVWRITE8_LEGACY("tc0140syt", tc0140syt_port_w, 0x00ff)
	AM_RANGE(0x300002, 0x300003) AM_DEVREADWRITE8_LEGACY("tc0140syt", tc0140syt_comm_r, tc0140syt_comm_w, 0x00ff)
	AM_RANGE(0x400000, 0x420fff) AM_DEVREADWRITE_LEGACY("tc0080vco", tc0080vco_word_r, tc0080vco_word_w)
	AM_RANGE(0x500800, 0x500fff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_word_w) AM_SHARE("paletteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( recordbr_map, AS_PROGRAM, 16, taitoh_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_MIRROR(0x010000) AM_RAM AM_SHARE("m68000_mainram")
	AM_RANGE(0x200000, 0x200001) AM_DEVREADWRITE8_LEGACY("tc0220ioc", tc0220ioc_portreg_r, tc0220ioc_portreg_w, 0x00ff)
	AM_RANGE(0x200002, 0x200003) AM_DEVREADWRITE8_LEGACY("tc0220ioc", tc0220ioc_port_r, tc0220ioc_port_w, 0x00ff)
	AM_RANGE(0x300000, 0x300001) AM_READNOP AM_DEVWRITE8_LEGACY("tc0140syt", tc0140syt_port_w, 0x00ff)
	AM_RANGE(0x300002, 0x300003) AM_DEVREADWRITE8_LEGACY("tc0140syt", tc0140syt_comm_r, tc0140syt_comm_w, 0x00ff)
	AM_RANGE(0x400000, 0x420fff) AM_DEVREADWRITE_LEGACY("tc0080vco", tc0080vco_word_r, tc0080vco_word_w)
	AM_RANGE(0x500800, 0x500fff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_word_w) AM_SHARE("paletteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( dleague_map, AS_PROGRAM, 16, taitoh_state )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_MIRROR(0x010000) AM_RAM AM_SHARE("m68000_mainram")
	AM_RANGE(0x200000, 0x20000f) AM_DEVREADWRITE8_LEGACY("tc0220ioc", tc0220ioc_r, tc0220ioc_w, 0x00ff)
	AM_RANGE(0x300000, 0x300001) AM_READNOP AM_DEVWRITE8_LEGACY("tc0140syt", tc0140syt_port_w, 0x00ff)
	AM_RANGE(0x300002, 0x300003) AM_DEVREADWRITE8_LEGACY("tc0140syt", tc0140syt_comm_r, tc0140syt_comm_w, 0x00ff)
	AM_RANGE(0x400000, 0x420fff) AM_DEVREADWRITE_LEGACY("tc0080vco", tc0080vco_word_r, tc0080vco_word_w)
	AM_RANGE(0x500800, 0x500fff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x600000, 0x600001) AM_WRITENOP	/* ?? writes zero once per frame */
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, taitoh_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE_LEGACY("ymsnd", ym2610_r, ym2610_w)
	AM_RANGE(0xe200, 0xe200) AM_READNOP AM_DEVWRITE_LEGACY("tc0140syt", tc0140syt_slave_port_w)
	AM_RANGE(0xe201, 0xe201) AM_DEVREADWRITE_LEGACY("tc0140syt", tc0140syt_slave_comm_r, tc0140syt_slave_comm_w)
	AM_RANGE(0xe400, 0xe403) AM_WRITENOP		/* pan control */
	AM_RANGE(0xea00, 0xea00) AM_READNOP
	AM_RANGE(0xee00, 0xee00) AM_WRITENOP		/* ? */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP		/* ? */
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
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "1000k" )
	PORT_DIPSETTING(    0x0c, "1500k" )
	PORT_DIPSETTING(    0x04, "2000k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SW2:7") /* code at 0x002af8 - see notes */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )		/* Listed as "Unused" */

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

static INPUT_PORTS_START( dleague )
	/* 0x200000 -> 0x100526.b ($526,A5) */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:1") /* see notes */
	PORT_DIPSETTING(    0x01, "Constant" )
	PORT_DIPSETTING(    0x00, "Based on Inning" )
	TAITO_DSWA_BITS_1_TO_3_LOC(SW1)
	TAITO_COINAGE_US_LOC(SW1)

	/* 0x200002 -> 0x100527.b ($527,A5) */
	PORT_START("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )		/* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPNAME( 0x0c, 0x0c, "Extra Credit Needed" )	PORT_DIPLOCATION("SW2:3,4") /* see notes */
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
	16,16,	/* 16x16 pixels */
	32768,	/* 32768 tiles */
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 0x100000*8+4, 0x100000*8, 0x100000*8+12, 0x100000*8+8,
	    0x200000*8+4, 0x200000*8, 0x200000*8+12, 0x200000*8+8, 0x300000*8+4, 0x300000*8, 0x300000*8+12, 0x300000*8+8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*16
};

static const gfx_layout charlayout =
{
	8, 8,	/* 8x8 pixels */
	256,	/* 256 chars */
	4,		/* 4 bit per pixel */
	{ 0x1000*8 + 8, 0x1000*8, 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7 },
	16*8
};


static GFXDECODE_START( syvalion )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0,     32 )
GFXDECODE_END

static GFXDECODE_START( recordbr )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0,     32 )
GFXDECODE_END

static GFXDECODE_START( dleague )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0,     32 )
GFXDECODE_END


static MACHINE_RESET( taitoh )
{
	taitoh_state *state = machine.driver_data<taitoh_state>();
	state->m_banknum = 0;
}

static MACHINE_START( taitoh )
{
	taitoh_state *state = machine.driver_data<taitoh_state>();
	UINT8 *ROM = state->memregion("audiocpu")->base();

	state->membank("bank1")->configure_entries(0, 4, &ROM[0xc000], 0x4000);

	state->m_audiocpu = machine.device("audiocpu");
	state->m_tc0220ioc = machine.device("tc0220ioc");
	state->m_tc0080vco = machine.device("tc0080vco");

	state->save_item(NAME(state->m_banknum));
	machine.save().register_postload(save_prepost_delegate(FUNC(reset_sound_region), &machine));
}


static const tc0080vco_interface syvalion_tc0080vco_intf =
{
	0, 1,	/* gfxnum, txnum */
	1, 1, -2,
	1
};

static const tc0080vco_interface recordbr_tc0080vco_intf =
{
	0, 1,	/* gfxnum, txnum */
	1, 1, -2,
	0
};

static const tc0220ioc_interface taitoh_io_intf =
{
	DEVCB_INPUT_PORT("DSWA"), DEVCB_INPUT_PORT("DSWB"),
	DEVCB_INPUT_PORT("IN0"), DEVCB_INPUT_PORT("IN1"), DEVCB_INPUT_PORT("IN2")	/* port read handlers */
};

static const tc0140syt_interface taitoh_tc0140syt_intf =
{
	"maincpu", "audiocpu"
};

static MACHINE_CONFIG_START( syvalion, taitoh_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,24000000 / 2)		/* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(syvalion_map)
	MCFG_CPU_VBLANK_INT("screen", irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,8000000 / 2)		/* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_MACHINE_START(taitoh)
	MCFG_MACHINE_RESET(taitoh)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_TC0220IOC_ADD("tc0220ioc", taitoh_io_intf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*16, 64*16)
	MCFG_SCREEN_VISIBLE_AREA(0*16, 32*16-1, 3*16, 28*16-1)
	MCFG_SCREEN_UPDATE_STATIC(syvalion)

	MCFG_GFXDECODE(syvalion)
	MCFG_PALETTE_LENGTH(33*16)

	MCFG_TC0080VCO_ADD("tc0080vco", syvalion_tc0080vco_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2610, 8000000)
	MCFG_SOUND_CONFIG(ym2610_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
	MCFG_SOUND_ROUTE(2, "mono", 1.0)

	MCFG_TC0140SYT_ADD("tc0140syt", taitoh_tc0140syt_intf)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( recordbr, taitoh_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz / 2)		/* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(recordbr_map)
	MCFG_CPU_VBLANK_INT("screen", irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz / 2)		/* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_MACHINE_START(taitoh)
	MCFG_MACHINE_RESET(taitoh)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_TC0220IOC_ADD("tc0220ioc", taitoh_io_intf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*16, 64*16)
	MCFG_SCREEN_VISIBLE_AREA(1*16, 21*16-1, 2*16, 17*16-1)
	MCFG_SCREEN_UPDATE_STATIC(recordbr)

	MCFG_GFXDECODE(recordbr)
	MCFG_PALETTE_LENGTH(32*16)

	MCFG_TC0080VCO_ADD("tc0080vco", recordbr_tc0080vco_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2610, XTAL_8MHz)
	MCFG_SOUND_CONFIG(ym2610_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
	MCFG_SOUND_ROUTE(2, "mono", 1.0)

	MCFG_TC0140SYT_ADD("tc0140syt", taitoh_tc0140syt_intf)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( dleague, taitoh_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz / 2)		/* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(dleague_map)
	MCFG_CPU_VBLANK_INT("screen", irq1_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz / 2)		/* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_MACHINE_START(taitoh)
	MCFG_MACHINE_RESET(taitoh)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_TC0220IOC_ADD("tc0220ioc", taitoh_io_intf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*16, 64*16)
	MCFG_SCREEN_VISIBLE_AREA(1*16, 21*16-1, 2*16, 17*16-1)
	MCFG_SCREEN_UPDATE_STATIC(dleague)

	MCFG_GFXDECODE(dleague)
	MCFG_PALETTE_LENGTH(33*16)

	MCFG_TC0080VCO_ADD("tc0080vco", recordbr_tc0080vco_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2610, XTAL_8MHz)
	MCFG_SOUND_CONFIG(ym2610_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
	MCFG_SOUND_ROUTE(2, "mono", 1.0)

	MCFG_TC0140SYT_ADD("tc0140syt", taitoh_tc0140syt_intf)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( syvalion )
	ROM_REGION( 0x80000, "maincpu", 0 )		/* main cpu */
	ROM_LOAD16_BYTE( "b51-20.bin", 0x00000, 0x20000, CRC(440b6418) SHA1(262b65f39eb13c11ae7b87013951097ab0a9cb63) )
	ROM_LOAD16_BYTE( "b51-22.bin", 0x00001, 0x20000, CRC(e6c61079) SHA1(b786ef1bfc72706347c12c17616652bc8302a98c) )
	ROM_LOAD16_BYTE( "b51-19.bin", 0x40000, 0x20000, CRC(2abd762c) SHA1(97cdb9f1dba5b11b96b5d3431937669de5220512) )
	ROM_LOAD16_BYTE( "b51-21.bin", 0x40001, 0x20000, CRC(aa111f30) SHA1(77da4a8db49999f5fa2cf0209028d0f70e26dfe3) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )		/* sound cpu */
	ROM_LOAD( "b51-23.bin", 0x00000, 0x04000, CRC(734662de) SHA1(0058d6de68f26cd58b9eb8859e15f3ced6bd3489) )
	ROM_CONTINUE(           0x10000, 0x0c000 )

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

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )	/* samples */
	ROM_LOAD( "b51-18.bin", 0x00000, 0x80000, CRC(8b23ac83) SHA1(340b9e7f09c1809a332b41d3fb579f5f8cd6367f) )

	ROM_REGION( 0x80000, "ymsnd", 0 )	/* samples */
	ROM_LOAD( "b51-17.bin", 0x00000, 0x80000, CRC(d85096aa) SHA1(dac39ed182e9eda06575f1667c4c1ff9a4a56599) )
ROM_END

ROM_START( recordbr )
	ROM_REGION( 0x80000, "maincpu", 0 )		/* main cpu */
	ROM_LOAD16_BYTE( "b56-17.bin", 0x00000, 0x20000, CRC(3e0a9c35) SHA1(900a741b2abbbbe883b9d78162a88b4397af1a56) )
	ROM_LOAD16_BYTE( "b56-16.bin", 0x00001, 0x20000, CRC(b447f12c) SHA1(58ee30337836f260c7fbda728dac93f06d861ec4) )
	ROM_LOAD16_BYTE( "b56-15.bin", 0x40000, 0x20000, CRC(b346e282) SHA1(f6b4a2e9093a33d19c2eaf3ef9801179f39a83a3) )
	ROM_LOAD16_BYTE( "b56-21.bin", 0x40001, 0x20000, CRC(e5f63790) SHA1(b81db7690a989146c438609d9633ddcb1fd219dd) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )		/* sound cpu */
	ROM_LOAD( "b56-19.bin", 0x00000, 0x04000, CRC(c68085ee) SHA1(78634216a622a08c20dae0422283c4a7ed360546) )
	ROM_CONTINUE(           0x10000, 0x0c000 )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b56-04.bin", 0x000000, 0x20000, CRC(f7afdff0) SHA1(8f8ea0e8da20913426ff3b58d7bb63bd352d3fb4) )
	ROM_LOAD16_BYTE( "b56-08.bin", 0x000001, 0x20000, CRC(c9f0d38a) SHA1(aa22f1a06e00f90c546eebcd8b42da3e3c7d0781) )
	ROM_LOAD16_BYTE( "b56-03.bin", 0x100000, 0x20000, CRC(4045fd44) SHA1(a84be9eedba7aed30d4f2841016784f8024d9443) )
	ROM_LOAD16_BYTE( "b56-07.bin", 0x100001, 0x20000, CRC(0c76e4c8) SHA1(e50d1bd6e8ec967ba03bd14097a9bd560aa2decc) )
	ROM_LOAD16_BYTE( "b56-02.bin", 0x200000, 0x20000, CRC(68c604ec) SHA1(75b26bfa53efa63b9c7a026f4226213364550cad) )
	ROM_LOAD16_BYTE( "b56-06.bin", 0x200001, 0x20000, CRC(5fbcd302) SHA1(22e7d835643945d501edc693dbe4efc8d4d074a7) )
	ROM_LOAD16_BYTE( "b56-01.bin", 0x300000, 0x20000, CRC(766b7260) SHA1(f7d7176af614f06e8c66e890e4d194ffb6f7af73) )
	ROM_LOAD16_BYTE( "b56-05.bin", 0x300001, 0x20000, CRC(ed390378) SHA1(0275e5ead206028bfcff7ecbe11c7ab961e648ea) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )	/* samples */
	ROM_LOAD( "b56-09.bin", 0x00000, 0x80000, CRC(7fd9ee68) SHA1(edc4455b3f6a6f30f418d03c6e53af875542a325) )

	ROM_REGION( 0x80000, "ymsnd", 0 )	/* samples */
	ROM_LOAD( "b56-10.bin", 0x00000, 0x80000, CRC(de1bce59) SHA1(aa3aea30d6f53e60d9a0d4ec767e1b261d5efc8a) )

	ROM_REGION( 0x02000, "user1", 0 ) /* zoom table / mixing? */
	ROM_LOAD( "b56-18.bin", 0x00000, 0x02000, CRC(c88f0bbe) SHA1(18c87c744fbeca35d13033e50f62e5383eb4ec2c) )
ROM_END

ROM_START( gogold )
	ROM_REGION( 0x80000, "maincpu", 0 )		/* main cpu */
	ROM_LOAD16_BYTE( "b56-17.bin", 0x00000, 0x20000, CRC(3e0a9c35) SHA1(900a741b2abbbbe883b9d78162a88b4397af1a56) )
	ROM_LOAD16_BYTE( "b56-16.bin", 0x00001, 0x20000, CRC(b447f12c) SHA1(58ee30337836f260c7fbda728dac93f06d861ec4) )
	ROM_LOAD16_BYTE( "b56-15.bin", 0x40000, 0x20000, CRC(b346e282) SHA1(f6b4a2e9093a33d19c2eaf3ef9801179f39a83a3) )
	ROM_LOAD16_BYTE( "b56-14.bin", 0x40001, 0x20000, CRC(b6c195b9) SHA1(80541d9a686fdc1850d764d8e00ba03526e7174c) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )		/* sound cpu */
	ROM_LOAD( "b56-19.bin", 0x00000, 0x04000, CRC(c68085ee) SHA1(78634216a622a08c20dae0422283c4a7ed360546) )
	ROM_CONTINUE(           0x10000, 0x0c000 )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b56-04.bin", 0x000000, 0x20000, CRC(f7afdff0) SHA1(8f8ea0e8da20913426ff3b58d7bb63bd352d3fb4) )
	ROM_LOAD16_BYTE( "b56-08.bin", 0x000001, 0x20000, CRC(c9f0d38a) SHA1(aa22f1a06e00f90c546eebcd8b42da3e3c7d0781) )
	ROM_LOAD16_BYTE( "b56-03.bin", 0x100000, 0x20000, CRC(4045fd44) SHA1(a84be9eedba7aed30d4f2841016784f8024d9443) )
	ROM_LOAD16_BYTE( "b56-07.bin", 0x100001, 0x20000, CRC(0c76e4c8) SHA1(e50d1bd6e8ec967ba03bd14097a9bd560aa2decc) )
	ROM_LOAD16_BYTE( "b56-02.bin", 0x200000, 0x20000, CRC(68c604ec) SHA1(75b26bfa53efa63b9c7a026f4226213364550cad) )
	ROM_LOAD16_BYTE( "b56-06.bin", 0x200001, 0x20000, CRC(5fbcd302) SHA1(22e7d835643945d501edc693dbe4efc8d4d074a7) )
	ROM_LOAD16_BYTE( "b56-01.bin", 0x300000, 0x20000, CRC(766b7260) SHA1(f7d7176af614f06e8c66e890e4d194ffb6f7af73) )
	ROM_LOAD16_BYTE( "b56-05.bin", 0x300001, 0x20000, CRC(ed390378) SHA1(0275e5ead206028bfcff7ecbe11c7ab961e648ea) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )	/* samples */
	ROM_LOAD( "b56-09.bin", 0x00000, 0x80000, CRC(7fd9ee68) SHA1(edc4455b3f6a6f30f418d03c6e53af875542a325) )

	ROM_REGION( 0x80000, "ymsnd", 0 )	/* samples */
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

	ROM_REGION( 0x1c000, "audiocpu", 0 )
	ROM_LOAD( "c02-23.40", 0x00000, 0x04000, CRC(5632ee49) SHA1(90dedaf40ab526529cd7d569b78a9d5451ec3e25) )
	ROM_CONTINUE(          0x10000, 0x0c000 )

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

	ROM_REGION( 0x80000, "ymsnd", 0 )	/* samples */
	ROM_LOAD( "c02-01.31", 0x00000, 0x80000, CRC(d5a3d1aa) SHA1(544f807015b5d854a4d8cb73e4dbae4b953fd440) )
ROM_END

ROM_START( dleaguej )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c02-19a.33", 0x00000, 0x20000, CRC(7e904e45) SHA1(04ac470c973753e71fba3998099a88ab0e6fcbab) )
	ROM_LOAD16_BYTE( "c02-21a.36", 0x00001, 0x20000, CRC(18c8a32b) SHA1(507cd7a83dcb6eaefa52f2661b9f3a6fabbfbd46) )
	ROM_LOAD16_BYTE( "c02-20.34",  0x40000, 0x10000, CRC(cdf593f3) SHA1(6afbd9d8d74e6801dc991eb9fd3205057747b986) )
	ROM_LOAD16_BYTE( "c02-22.37",  0x40001, 0x10000, CRC(f50db2d7) SHA1(4f16cc42469f1e5bf6dc1aee0919712db089f9cc) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )
	ROM_LOAD( "c02-23.40", 0x00000, 0x04000, CRC(5632ee49) SHA1(90dedaf40ab526529cd7d569b78a9d5451ec3e25) )
	ROM_CONTINUE(          0x10000, 0x0c000 )

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

	ROM_REGION( 0x80000, "ymsnd", 0 )	/* samples */
	ROM_LOAD( "c02-01.31", 0x00000, 0x80000, CRC(d5a3d1aa) SHA1(544f807015b5d854a4d8cb73e4dbae4b953fd440) )
ROM_END


/*  ( YEAR  NAME      PARENT    MACHINE   INPUT     INIT     MONITOR  COMPANY                      FULLNAME */
GAME( 1988, syvalion, 0,        syvalion, syvalion, 0,       ROT0,    "Taito Corporation",         "Syvalion (Japan)",        GAME_SUPPORTS_SAVE )
GAME( 1988, recordbr, 0,        recordbr, recordbr, 0,       ROT0,    "Taito Corporation Japan",   "Recordbreaker (World)",   GAME_SUPPORTS_SAVE )
GAME( 1988, gogold,   recordbr, recordbr, gogold,   0,       ROT0,    "Taito Corporation",         "Go For The Gold (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1990, dleague,  0,        dleague,  dleague,  0,       ROT0,    "Taito America Corporation", "Dynamite League (US)", GAME_SUPPORTS_SAVE )
GAME( 1990, dleaguej, dleague,  dleague,  dleaguej, 0,       ROT0,    "Taito Corporation",         "Dynamite League (Japan)", GAME_SUPPORTS_SAVE )
