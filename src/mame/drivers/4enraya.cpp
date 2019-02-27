// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, Roberto Fresca
/***************************************************************************

  IDSA 4 En Raya.

  Driver by Tomasz Slanina.
  Additional work by Roberto Fresca.


  Supported games:

  4 En Raya (set 1),                              1990, IDSA.
  4 En Raya (set 2),                              1990, IDSA.
  unknown 'Pac-Man' gambling game,                1990, Unknown.
  unknown 'Space Invaders' gambling game (set 1), 1990, Unknown (made in France).
  unknown 'Space Invaders' gambling game (set 2), 199?, Unknown.

  TODO:
  - Video and IRQ timings;
  - Is there a waitstate penalty on the VRAM, apparently?


***************************************************************************

  RAM :
  1 x GM76c28-10 (6116) RAM
  3 x 2114  - VRAM (only 10 bits are used )

  ROM:
  27256 + 27128 for code/data
  3x2764 for gfx

  PROM:
  82S123 32x8
  Used for system control
  (d0 - connected to ROM5 /CS , d1 - ROM4 /CS, d2 - RAM /CS , d3 - to some logic(gfx control), and Z80 WAIT )

  Memory Map :
  0x0000 - 0xbfff - ROM
  0xc000 - 0xcfff - RAM
  0xd000 - 0xdfff - VRAM mirrored write,
        tilemap offset = address & 0x3ff
        tile number =  bits 0-7 = data, bits 8,9  = address bits 10,11
  0xe000 - 0xefff - VRAM mirror
  0xf000 - 0xffff - (unconnected)

  Video :
  No scrolling , no sprites.
  32x32 Tilemap stored in VRAM (10 bits/tile (tile numebr 0-1023))

  3 gfx ROMS
  ROM1 - R component (ROM ->(parallel in) shift register 74166 (serial out) -> jamma output
  ROM2 - G component
  ROM3 - B component

  Sound :
  AY 3 8910

  sound_control :

  bit 0 - BC1
  bit 1 - BC2
  bit 2 - BDIR

  bits 3-7 - not connected

***************************************************************************

  Unknown Pac-Man gambling game.

  It's a basic Pac-Man front game, that has a gambling game hidden inside.
  The purpose of this "stealth" game, is just to be a "camouflage" for the
  real gambling game, for locations where the gambling games are forbidden.


  How to play the Pac-Man front game:

  Just Coin using Coin A or B, Start the game with START button, and use
  the arrow keys to control the Pac-Man.


  How to play the Pac-Man gambling game:

  Just coin up using Gambling Coin In (key 7). All ghosts will be placed
  around the center. (each ghost represent a number).

  Bet using START, and once done, press UP (deal), to allow the pacman eat all
  ghosts, revealing the five numbers (like italian poker games without cards).

  Now you have an arrow as cursor. Place it under the each number you want to
  discard and press START to eliminate the number and place the representative
  ghost again in the original place outside the center.

  Once done, just press UP (deal) again, and pacman will re-eat the new placed
  ghosts, revealing the new numbers (as a new deal).

  If you have a winning hand, you can press DOWN (double-up) to get a Double-Up,
  or UP (deal/take) to collect the winnings.

  If you're playing the Double-Up, choose left or right for Big and Small.
  If you win, you'll get the bet amount x2. If you lose, your pacman will die.

  Coin with A or B to exit the gambling game and play the ultra-adictive
  pacman front game again!...

***************************************************************************

  Unknown Pac-Man gambling game technical notes...

  The program checks the port 01h, bit7, for the sound hardware type.

  - Type 1: AY-3-8910 mapped at 17h, 27h, 37h.
  - Type 2: Unknown device mapped at 20h, 30h.

  I strongly think is selectable through a switch, so I hooked one till we
  have evidence of the contrary.


****************************************************************************

  Hardware Notes: (unknown 'Space Invaders' gambling game)
  ---------------

  Based on chip manufacturing should be manufactured in 1998


  Specs:
  ------

  1x Zilog Z0840004PSE (4MHz Z80 CPU).
  1x GI AY-3-8910 (sound).
  1x LM356N (Low Voltage Audio Power Amplifier).

  ROMs: 2x 27C256 Program ROMs (I, II).
        3x 27C256 GFX ROMs (R, V, B).

  RAMs: 1x KM62256ALP-10 (near prg ROMs).
        2x CY6264-70SNC (Near GFX ROMs).

  1x oscillator 18.432 MHz.

  1x 8 DIP Switches bank (near ay8910).
  1x Volume Pot (betweeen the audio amp and ay8910).
  1x Motorola MCT1413 (High Current Darlington Transistor Array, same as ULN2003).

  1x 2x28 Edge connector (pins 1-2-27-28 from component side are GND).

***************************************************************************/

#include "emu.h"
#include "includes/4enraya.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "screen.h"
#include "speaker.h"

#define MAIN_CLOCK XTAL(8'000'000)


/***********************************
*         Custom Handlers          *
***********************************/

WRITE8_MEMBER(_4enraya_state::sound_data_w)
{
	m_soundlatch = data;
}

WRITE8_MEMBER(_4enraya_state::sound_control_w)
{
	// BDIR must be high
	if (~data & 4)
		return;

	switch (data & 3)
	{
		case 0: case 3:
			// latch address
			m_ay->address_w(m_soundlatch);
			break;

		case 2:
			// write to psg
			m_ay->data_w(m_soundlatch);
			break;

		default:
			// inactive
			break;
	}
}

READ8_MEMBER(_4enraya_state::fenraya_custom_map_r)
{
	uint8_t prom_routing = (m_prom[offset >> 12] & 0xf) ^ 0xf;
	uint8_t res = 0;

	if (prom_routing & 1) // ROM5
	{
		res |= m_rom[offset & 0x7fff];
	}

	if (prom_routing & 2) // ROM4
	{
		res |= m_rom[(offset & 0x7fff) | 0x8000];
	}

	if (prom_routing & 4) // RAM
	{
		res |= m_workram[offset & 0xfff];
	}

	if (prom_routing & 8) // gfx control / RAM wait
	{
		res |= m_videoram[offset & 0xfff];
	}

	return res;
}

WRITE8_MEMBER(_4enraya_state::fenraya_custom_map_w)
{
	uint8_t prom_routing = (m_prom[offset >> 12] & 0xf) ^ 0xf;

	if (prom_routing & 1) // ROM5
	{
		// ...
	}

	if (prom_routing & 2) // ROM4
	{
		// ...
	}

	if (prom_routing & 4) // RAM
	{
		m_workram[offset & 0xfff] = data;
	}

	if (prom_routing & 8) // gfx control / RAM wait
	{
		fenraya_videoram_w(space, offset & 0xfff, data);
	}
}


/***********************************
*      Memory Map Information      *
***********************************/

void _4enraya_state::main_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(_4enraya_state::fenraya_custom_map_r), FUNC(_4enraya_state::fenraya_custom_map_w));
}

void _4enraya_state::main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW");
	map(0x01, 0x01).portr("INPUTS");
	map(0x02, 0x02).portr("SYSTEM");
	map(0x23, 0x23).w(FUNC(_4enraya_state::sound_data_w));
	map(0x33, 0x33).w(FUNC(_4enraya_state::sound_control_w));
}


void unk_gambl_state::unkpacg_main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x6000, 0x67ff).ram().share("nvram");
	map(0x7000, 0x7fff).w(FUNC(_4enraya_state::fenraya_videoram_w));
	map(0x8000, 0x9fff).rom();
}

void unk_gambl_state::unkpacg_main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW1");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("IN2");
//  AM_RANGE(0x03, 0x03) AM_WRITE("out_w")  // to investigate...
	map(0x17, 0x17).w(m_ay, FUNC(ay8910_device::data_w));
	map(0x27, 0x27).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x37, 0x37).w(m_ay, FUNC(ay8910_device::address_w));
}


/***********************************
*           Input Ports            *
***********************************/

static INPUT_PORTS_START( 4enraya )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Pieces" )                PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "30" )
	PORT_DIPSETTING(    0x00, "16" )
	PORT_DIPNAME( 0x08, 0x08, "Speed" )                 PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Down") // "drop" ("down")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Down") // "drop" ("down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Shot") // "fire" ("shot")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Shot") // "fire" ("shot")

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

static INPUT_PORTS_START( unkpacg )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Front Game Coin A")       //  1 credits / initiate minigame
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Gambling Game Coin In")   //  5 credits / initiate gambling
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Front Game Coin B")       // 10 credits
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_DIPNAME( 0x80, 0x00, "Sound Hardware")
	PORT_DIPSETTING(    0x00, "Type 1 (AY-3-8910 mapped at 17h, 27h, 37h)" )
	PORT_DIPSETTING(    0x80, "Type 2 (Unknown device mapped at 20h, 30h)" )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Bet / Discard")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_NAME("Up / Deal / Take")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_NAME("Left / Small")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Right / Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_NAME("Down / Double-Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DSW1-1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW1-2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW1-3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW1-4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW1-5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW1-6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW1-7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW1-8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSW2-1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Gambling Game")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW2-3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW2-4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW2-5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW2-6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Gambling Game Credits Value")
	PORT_DIPSETTING(    0x40, "1 Credit/Point = 100" )
	PORT_DIPSETTING(    0x00, "1 Credit/Point = 500" )
	PORT_DIPNAME( 0x80, 0x00, "Clear NVRAM (On, reset, Off, reset)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( unkfr )
	PORT_INCLUDE( unkpacg )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )         PORT_NAME("Fire / Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_NAME("Deal / Take")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_NAME("Left / Small")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Right / Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_NAME("Down / Double-Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start Non-Gambling game")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***********************************
*     GFX Layouts & GFX decode     *
***********************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(1,3), RGN_FRAC(2,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_4enraya )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 1 )
GFXDECODE_END


/***********************************
*      Machine Start & Reset       *
***********************************/

void _4enraya_state::machine_start()
{
	save_item(NAME(m_videoram));
	save_item(NAME(m_workram));
	save_item(NAME(m_soundlatch));
}

void _4enraya_state::machine_reset()
{
	m_soundlatch = 0;
}


/***********************************
*         Machine Drivers          *
***********************************/

MACHINE_CONFIG_START(_4enraya_state::_4enraya )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, MAIN_CLOCK/2)
	MCFG_DEVICE_PROGRAM_MAP(main_map)
	MCFG_DEVICE_IO_MAP(main_portmap)
	MCFG_DEVICE_PERIODIC_INT_DRIVER(_4enraya_state, irq0_line_hold, 4*60) // unknown timing

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(_4enraya_state, screen_update_4enraya)
	MCFG_SCREEN_PALETTE(m_palette)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_4enraya);

	PALETTE(config, m_palette, palette_device::RGB_3BIT);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay, MAIN_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 0.3); /* guess */
MACHINE_CONFIG_END


MACHINE_CONFIG_START(unk_gambl_state::unkpacg)
	_4enraya(config);

	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(unkpacg_main_map)
	MCFG_DEVICE_IO_MAP(unkpacg_main_portmap)
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* sound hardware */
//  SPEAKER(config, "mono").front_center();
	AY8910(config.replace(), m_ay, MAIN_CLOCK/4); /* guess */
	m_ay->port_a_read_callback().set_ioport("DSW2");
	m_ay->add_route(ALL_OUTPUTS, "mono", 1.0);
MACHINE_CONFIG_END


/***********************************
*             Rom Load             *
***********************************/

ROM_START( 4enraya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5.bin",   0x0000, 0x8000, CRC(cf1cd151) SHA1(3920b0a6ed5798859158871b578b01ec742b0d13) )
	ROM_LOAD( "4.bin",   0x8000, 0x4000, CRC(f9ec1be7) SHA1(189159129ecbc4f6909c086867b0e02821f5b976) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "1.bin",   0x0000, 0x2000, CRC(0e5072fd) SHA1(0960e81f7fd52b38111eab2c124cfded5b35aa0b) )
	ROM_LOAD( "2.bin",   0x2000, 0x2000, CRC(2b0a3793) SHA1(2c3d224251557824bb9641dc2f98a000ab72c4a2) )
	ROM_LOAD( "3.bin",   0x4000, 0x2000, CRC(f6940836) SHA1(afde21ffa0c141cf73243e50da62ecfd474aaac2) )

	ROM_REGION( 0x0020,  "pal_prom", 0 )
	ROM_LOAD( "1.bpr",   0x0000, 0x0020, CRC(dcbd2352) SHA1(ce72e84129ed1b455aaf648e1dfaa4333e7e7628) ) /* system control: used for memory mapping */
ROM_END

ROM_START( 4enrayaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5.bin",   0x0000, 0x8000, CRC(76e8656c) SHA1(8c92bf083abe5f669b1bff47444294820b711f1a) ) // sldh
	ROM_LOAD( "4.bin",   0x8000, 0x4000, CRC(f9ec1be7) SHA1(189159129ecbc4f6909c086867b0e02821f5b976) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "1.bin",   0x0000, 0x2000, CRC(0e5072fd) SHA1(0960e81f7fd52b38111eab2c124cfded5b35aa0b) )
	ROM_LOAD( "2.bin",   0x2000, 0x2000, CRC(2b0a3793) SHA1(2c3d224251557824bb9641dc2f98a000ab72c4a2) )
	ROM_LOAD( "3.bin",   0x4000, 0x2000, CRC(f6940836) SHA1(afde21ffa0c141cf73243e50da62ecfd474aaac2) )

	ROM_REGION( 0x0020,  "pal_prom", 0 )
	ROM_LOAD( "1.bpr",   0x0000, 0x0020, CRC(dcbd2352) SHA1(ce72e84129ed1b455aaf648e1dfaa4333e7e7628) ) /* system control: used for memory mapping */
ROM_END


/*
  Unknown 'Pac-Man' gambling game.
*/
ROM_START(unkpacg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("1.u14",    0x0000, 0x2000, CRC(848c4143) SHA1(3cff26181c58e5f52f1ac81df7d5d43e644585a2))
	ROM_LOAD("2.u46",    0x8000, 0x2000, CRC(9e6e0bd3) SHA1(f502132a0460108dad243632cc13d9116c534291))

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "3.u20",   0x2000, 0x2000, CRC(d00b04ea) SHA1(e65901d8586507257d74ab103001207e28fa28af) )
	ROM_LOAD( "4.u19",   0x4000, 0x2000, CRC(4a123a3d) SHA1(26300b8af0d0df0023a153a212699727311d1b74) )
	ROM_LOAD( "5.u18",   0x0000, 0x2000, CRC(44f272d2) SHA1(b39cbc1f290d9fb2453396906e4da4a682c41ef4) )
ROM_END

/*
  Unknown 'Space Invaders' gambling game.
  All roms are 0x8000 but only the last 0x2000 of each is used.
*/
ROM_START( unksig )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "i.bin",  0x0000, 0x2000, CRC(902efc27) SHA1(5992cdc647acd622c73adefac1aa66e77b5ccc4f) )
	ROM_CONTINUE(       0x0000, 0x2000)
	ROM_CONTINUE(       0x0000, 0x2000)
	ROM_CONTINUE(       0x0000, 0x2000) // only data here matters
	ROM_LOAD( "ii.bin", 0x8000, 0x2000, CRC(855c1ea3) SHA1(80c89bfbdf3d0d69aed7333e9aa93db6aff7b704) )
	ROM_CONTINUE(       0x8000, 0x2000)
	ROM_CONTINUE(       0x8000, 0x2000)
	ROM_CONTINUE(       0x8000, 0x2000) // only data here matters

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "r.bin", 0x0000, 0x2000, CRC(f8a358fe) SHA1(5c4051de156014a5c2400f4934e2136b38bfed8c) )
	ROM_CONTINUE(      0x0000, 0x2000)
	ROM_CONTINUE(      0x0000, 0x2000)
	ROM_CONTINUE(      0x0000, 0x2000) // only data here matters
	ROM_LOAD( "b.bin", 0x2000, 0x2000, CRC(56ac5874) SHA1(7ae63f930b07cb1b4989c8328fcc3627d8ff68f8) )
	ROM_CONTINUE(      0x2000, 0x2000)
	ROM_CONTINUE(      0x2000, 0x2000)
	ROM_CONTINUE(      0x2000, 0x2000) // only data here matters
	ROM_LOAD( "v.bin", 0x4000, 0x2000, CRC(4c5a7e43) SHA1(17e52ed73f9e8822b53bebc31c9320f5589ef70a) )
	ROM_CONTINUE(      0x4000, 0x2000)
	ROM_CONTINUE(      0x4000, 0x2000)
	ROM_CONTINUE(      0x4000, 0x2000) // only data here matters
ROM_END

/*
  Unknown 'Space Invaders' gambling game
  All roms are 0x10000 but with a lot of addressing issues

  1.bin    BADADDR    ---xxxxxxxxxxxxx
  2.bin    BADADDR    ---xxxxxxxxxxxxx
  b.bin    BADADDR    x-xxxxxxxxxxxxxx
  r.bin    BADADDR    x-xxxxxxxxxxxxxx
  v.bin    BADADDR    x-xxxxxxxxxxxxxx

  The game has both (space invaders & pac-man) graphics sets.
  Maybe a leftover?...

*/
ROM_START( unksiga )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",  0x0000, 0x2000, CRC(089a4a63) SHA1(d519f6289e72299e48ed1790fa4919cae14e2a0f) )  // 0x2000 of data repeated along the dump
	ROM_IGNORE(                 0xe000)   /* Identical 0x2000 segments */
	ROM_LOAD( "2.bin",  0x8000, 0x2000, CRC(970632fd) SHA1(2aa69fda1dce201856b237ecbedfdcde470a4bb3) )  // 0x2000 of data repeated along the dump
	ROM_IGNORE(                 0xe000)   /* Identical 0x2000 segments */

	ROM_REGION( 0xc000, "gfx1", 0 )
/*  tileset 0000-03ff = Space Invaders GFX.
    tileset 0400-07ff = Pac-Man GFX.
*/
	ROM_LOAD( "b.bin", 0x0000, 0x4000, CRC(dd257fb6) SHA1(b543225615f3cbef34dbecde04c7e937eede0988) )
	ROM_CONTINUE(      0x0000, 0x4000)
	ROM_CONTINUE(      0x0000, 0x4000) // data
	ROM_IGNORE(                0x4000) // dupe
	ROM_LOAD( "r.bin", 0x4000, 0x4000, CRC(38e9feba) SHA1(76811f05dabbb608e3863eeea0c53725c7cff618) )
	ROM_CONTINUE(      0x4000, 0x4000)
	ROM_CONTINUE(      0x4000, 0x4000) // data
	ROM_IGNORE(                0x4000) // dupe
	ROM_LOAD( "v.bin", 0x8000, 0x4000, CRC(cc597c7b) SHA1(5830fa9e8f9823eb4a910d6f80c3de15f7269619) )
	ROM_CONTINUE(      0x8000, 0x4000)
	ROM_CONTINUE(      0x8000, 0x4000) // data
	ROM_IGNORE(                0x4000) // dupe
ROM_END


/***********************************
*          Driver Init             *
***********************************/

void unk_gambl_state::driver_init()
{
	_4enraya_state::driver_init();

	// descramble rom
	uint8_t *rom = memregion("maincpu")->base();
	for (int i = 0x8000; i < 0xa000; i++)
		rom[i] = bitswap<8>(rom[i], 7,6,5,4,3,2,0,1);
}


/***********************************
*           Game Drivers           *
***********************************/

/*    YEAR  NAME      PARENT   MACHINE   INPUT    CLASS            INIT        ROT   COMPANY      FULLNAME                                         FLAGS  */
GAME( 1990, 4enraya,  0,       _4enraya, 4enraya, _4enraya_state,  empty_init, ROT0, "IDSA",      "4 En Raya (set 1)",                              MACHINE_SUPPORTS_SAVE )
GAME( 1990, 4enrayaa, 4enraya, _4enraya, 4enraya, _4enraya_state,  empty_init, ROT0, "IDSA",      "4 En Raya (set 2)",                              MACHINE_SUPPORTS_SAVE )
GAME( 199?, unkpacg,  0,       unkpacg,  unkpacg, unk_gambl_state, empty_init, ROT0, "<unknown>", "unknown 'Pac-Man' gambling game",                MACHINE_SUPPORTS_SAVE )
GAME( 199?, unksig,   0,       unkpacg,  unkfr,   unk_gambl_state, empty_init, ROT0, "<unknown>", "unknown 'Space Invaders' gambling game (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 199?, unksiga,  unksig,  unkpacg,  unkfr,   unk_gambl_state, empty_init, ROT0, "<unknown>", "unknown 'Space Invaders' gambling game (set 2)", MACHINE_SUPPORTS_SAVE )
