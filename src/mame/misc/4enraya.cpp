// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, Roberto Fresca
/***************************************************************************

  IDSA 4 En Raya.

  Driver by Tomasz Slanina.
  Additional work by Roberto Fresca.


  Supported games:

  4 En Raya (set 1),                              1990, IDSA.
  4 En Raya (set 2),                              1990, IDSA.
  unknown bowling themed 'gum' poker machine      1992?,Paradise Automatique / TourVision
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
  32x32 Tilemap stored in VRAM (10 bits/tile (tile number 0-1023))

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
  ghosts, revealing the five numbers (like Italian poker games without cards).

  Now you have an arrow as cursor. Place it under the each number you want to
  discard and press START to eliminate the number and place the representative
  ghost again in the original place outside the center.

  Once done, just press UP (deal) again, and pacman will re-eat the new placed
  ghosts, revealing the new numbers (as a new deal).

  If you have a winning hand, you can press DOWN (double-up) to get a Double-Up,
  or UP (deal/take) to collect the winnings.

  If you're playing the Double-Up, choose left or right for Big and Small.
  If you win, you'll get the bet amount x2. If you lose, your pacman will die.

  Coin with A or B to exit the gambling game and play the ultra-addictive
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
  1x Volume Pot (between the audio amp and ay8910).
  1x Motorola MCT1413 (High Current Darlington Transistor Array, same as ULN2003).

  1x 2x28 Edge connector (pins 1-2-27-28 from component side are GND).

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class _4enraya_state : public driver_device
{
public:
	_4enraya_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ay(*this, "aysnd")
		, m_palette(*this, "palette")
		, m_gfxdecode(*this, "gfxdecode")
		, m_videoram(*this, "videoram", 0x1000, ENDIANNESS_LITTLE)
		, m_workram(*this, "workram", 0x1000, ENDIANNESS_LITTLE)
		, m_prom(*this, "pal_prom")
		, m_rom(*this, "maincpu")
	{
	}

	void _4enraya(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void videoram_w(offs_t offset, uint8_t data);

	void video(machine_config &config);

	required_device<cpu_device> m_maincpu;
	required_device<ay8910_device> m_ay;
	required_device<palette_device> m_palette;

private:
	required_device<gfxdecode_device> m_gfxdecode;

	// memory pointers
	memory_share_creator<uint8_t> m_videoram;
	memory_share_creator<uint8_t> m_workram;

	optional_region_ptr<uint8_t> m_prom;
	optional_region_ptr<uint8_t> m_rom;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;

	// sound-related
	uint8_t m_soundlatch = 0U;

	void sound_data_w(uint8_t data);
	uint8_t fenraya_custom_map_r(offs_t offset);
	void fenraya_custom_map_w(offs_t offset, uint8_t data);
	void sound_control_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;
};

class unk_gambl_state : public _4enraya_state
{
public:
	unk_gambl_state(const machine_config &mconfig, device_type type, const char *tag)
		: _4enraya_state(mconfig, type, tag)
	{
	}

	void unkpacg(machine_config &config);
	void unkpacga(machine_config &config);
	void tourpgum(machine_config &config);
	void chicgum(machine_config &config);

private:
	void unkpacg_main_map(address_map &map) ATTR_COLD;
	void unkpacga_main_map(address_map &map) ATTR_COLD;
	void tourpgum_main_map(address_map &map) ATTR_COLD;

	void unkpacg_main_portmap(address_map &map) ATTR_COLD;
};

class unk_gambl_enc_state : public unk_gambl_state
{
public:
	unk_gambl_enc_state(const machine_config &mconfig, device_type type, const char *tag)
		: unk_gambl_state(mconfig, type, tag)
	{
	}

private:
	virtual void driver_start() override;
};


void _4enraya_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[(offset & 0x3ff) * 2] = data;
	m_videoram[(offset & 0x3ff) * 2 + 1] = (offset & 0xc00) >> 10;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

TILE_GET_INFO_MEMBER(_4enraya_state::get_tile_info)
{
	int code = m_videoram[tile_index * 2] + (m_videoram[tile_index * 2 + 1] << 8);
	tileinfo.set(0, code, 0, 0);
}

void _4enraya_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(_4enraya_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t _4enraya_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/***********************************
*         Custom Handlers          *
***********************************/

void _4enraya_state::sound_data_w(uint8_t data)
{
	m_soundlatch = data;
}

void _4enraya_state::sound_control_w(uint8_t data)
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

uint8_t _4enraya_state::fenraya_custom_map_r(offs_t offset)
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

void _4enraya_state::fenraya_custom_map_w(offs_t offset, uint8_t data)
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
		videoram_w(offset & 0xfff, data);
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
	map(0x7000, 0x7fff).w(FUNC(unk_gambl_state::videoram_w));
	map(0x8000, 0x9fff).rom();
}

void unk_gambl_state::unkpacga_main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0x6000);
	map(0x6000, 0x67ff).ram().share("nvram");
	map(0x7000, 0x7fff).w(FUNC(unk_gambl_state::videoram_w));
	map(0x8000, 0xffff).rom().region("maincpu", 0x8000);
}

void unk_gambl_state::tourpgum_main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x6000, 0x67ff).ram().share("nvram");
	map(0x7000, 0x7fff).w(FUNC(unk_gambl_state::videoram_w));
}

void unk_gambl_state::unkpacg_main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW1");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("IN2");
//  map(0x03, 0x03).w(FUNC(unk_gambl_state::out_w));  // to investigate...
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


static INPUT_PORTS_START( tourpgum )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) // 1 credit (5 needed to start)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) // 5 credits?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) // 5 credits?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )  PORT_NAME("Bet / Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Lot 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Lot 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Lot 3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Lot 5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Lot 1")

	PORT_START("DSW1") // only one bank of DSW on this PCB, presumably the 2nd one as service input is there? needs checking in code
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
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
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x80, 0x00, "DSW1-8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
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
	GFXDECODE_ENTRY( "chars", 0, charlayout, 0, 1 )
GFXDECODE_END


/***********************************
*      Machine Start & Reset       *
***********************************/

void _4enraya_state::machine_start()
{
	save_item(NAME(m_soundlatch));
}

void _4enraya_state::machine_reset()
{
	m_soundlatch = 0;
}


/***********************************
*         Machine Drivers          *
***********************************/

void _4enraya_state::video(machine_config &config)
{
	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(_4enraya_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_4enraya);

	PALETTE(config, m_palette, palette_device::RGB_3BIT);
}


static constexpr XTAL MAIN_CLOCK = XTAL(8'000'000);

void _4enraya_state::_4enraya(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, MAIN_CLOCK / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &_4enraya_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &_4enraya_state::main_portmap);
	m_maincpu->set_periodic_int(FUNC(_4enraya_state::irq0_line_hold), attotime::from_hz(4*60)); // unknown timing

	video(config);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay, MAIN_CLOCK / 4).add_route(ALL_OUTPUTS, "mono", 0.3); // guess
}


void unk_gambl_state::unkpacg(machine_config &config)
{
	_4enraya(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &unk_gambl_state::unkpacg_main_map);
	m_maincpu->set_addrmap(AS_IO, &unk_gambl_state::unkpacg_main_portmap);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// sound hardware
	AY8910(config.replace(), m_ay, MAIN_CLOCK / 4); // guess
	m_ay->port_a_read_callback().set_ioport("DSW2");
	m_ay->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void unk_gambl_state::unkpacga(machine_config &config)
{
	unkpacg(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &unk_gambl_state::unkpacga_main_map);
}

void unk_gambl_state::tourpgum(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(18'000'000) / 4); // can only see an 18Mhz XTAL on this PCB?
	m_maincpu->set_addrmap(AS_PROGRAM, &unk_gambl_state::tourpgum_main_map);
	m_maincpu->set_addrmap(AS_IO, &unk_gambl_state::unkpacg_main_portmap);
	m_maincpu->set_periodic_int(FUNC(_4enraya_state::irq0_line_hold), attotime::from_hz(4*60)); // unknown timing

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	video(config);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay, XTAL(18'000'000) / 4 / 4).add_route(ALL_OUTPUTS, "mono", 1.0); // guess
	m_ay->port_a_read_callback().set_ioport("DSW2");
	m_ay->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void unk_gambl_state::chicgum(machine_config &config)
{
	tourpgum(config);

	PALETTE(config.replace(), m_palette, palette_device::BRG_3BIT);
}

/***********************************
*             Rom Load             *
***********************************/

ROM_START( 4enraya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5.bin",   0x0000, 0x8000, CRC(cf1cd151) SHA1(3920b0a6ed5798859158871b578b01ec742b0d13) )
	ROM_LOAD( "4.bin",   0x8000, 0x4000, CRC(f9ec1be7) SHA1(189159129ecbc4f6909c086867b0e02821f5b976) )

	ROM_REGION( 0x6000, "chars", 0 )
	ROM_LOAD( "1.bin",   0x0000, 0x2000, CRC(0e5072fd) SHA1(0960e81f7fd52b38111eab2c124cfded5b35aa0b) )
	ROM_LOAD( "2.bin",   0x2000, 0x2000, CRC(2b0a3793) SHA1(2c3d224251557824bb9641dc2f98a000ab72c4a2) )
	ROM_LOAD( "3.bin",   0x4000, 0x2000, CRC(f6940836) SHA1(afde21ffa0c141cf73243e50da62ecfd474aaac2) )

	ROM_REGION( 0x0020,  "pal_prom", 0 )
	ROM_LOAD( "1.bpr",   0x0000, 0x0020, CRC(dcbd2352) SHA1(ce72e84129ed1b455aaf648e1dfaa4333e7e7628) ) // system control: used for memory mapping
ROM_END

ROM_START( 4enrayaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5.bin",   0x0000, 0x8000, CRC(76e8656c) SHA1(8c92bf083abe5f669b1bff47444294820b711f1a) ) // sldh
	ROM_LOAD( "4.bin",   0x8000, 0x4000, CRC(f9ec1be7) SHA1(189159129ecbc4f6909c086867b0e02821f5b976) )

	ROM_REGION( 0x6000, "chars", 0 )
	ROM_LOAD( "1.bin",   0x0000, 0x2000, CRC(0e5072fd) SHA1(0960e81f7fd52b38111eab2c124cfded5b35aa0b) )
	ROM_LOAD( "2.bin",   0x2000, 0x2000, CRC(2b0a3793) SHA1(2c3d224251557824bb9641dc2f98a000ab72c4a2) )
	ROM_LOAD( "3.bin",   0x4000, 0x2000, CRC(f6940836) SHA1(afde21ffa0c141cf73243e50da62ecfd474aaac2) )

	ROM_REGION( 0x0020,  "pal_prom", 0 )
	ROM_LOAD( "1.bpr",   0x0000, 0x0020, CRC(dcbd2352) SHA1(ce72e84129ed1b455aaf648e1dfaa4333e7e7628) ) // system control: used for memory mapping
ROM_END

/*  ________________________________________________________________________________________________________
   |                          __________  __________  ______________  __________  __________ VIDEOGUM/TV   |
   |                         |T74LS14B1| |_74LS74AN| | EPROM 1     | |MC1454BCP| |_TC4023BP|               |
   |                                                 |_____________|  __________              __________   |
   |  __________  __________  __________  __________  ______________ |SN74LS166N             |__7406N__| __|
   | |_74LS08N_| |_74LS393_| |_74LS153N| |W2416K-10L | EPROM 2     |  __________              __________ |_
   |  __________  __________  __________             |_____________| |SN74LS166N             |4116R-001| __|
   | |_74LS393_| |__74LS10_| |74LS257AN|              ______________  __________  __________             __|
   |  __________  __________  __________  __________ | EPROM 3     | |SN74LS166N |__7406N__|             __|
   | |SN74LS08N| |SN74LS32N| |GD74LS157| |W2416K-10L |_____________|                                     __|
   |  __________  __________  __________  __________      __________              __________  __________ __|
   | |_GS74LS20| |74LS125AN| PC74HCT138P |SN74LS08N|     |SN74LS245N|             |_74LS273_| |ULN2003A| __|
   |  __________  __________  __________  __________    ____________  __________  __________  __________ __|
   | |_74LS74AN| |SN74LS32N| |T74LS00B1| PC74HCT138P   | W2416-10L | |SN74LS32N| |SN74LS245N  |4116R-001 __|
   |  __________  __________  __________  __________   |___________|              __________             __|
   | |GD72LS393| |_74LS74AN| |_74LS241N| |_74LS241N|  _________________          |T74LS273B1             __|
   |  __________  __________  _________________      | AY3910A/P      |                       __________ __|
   | |SN74LS92N| |_74LS74AN| | Z0840004PSC Z80|      |________________|                      |ULN2003A_| __|
   |  __________  __________ |________________|                       __________  __________  __________ __|
   | |SN74LS04N| |SN74LS00N|                                         |__8xDIPS_| |SN74LS245N |SN7407N_|  __|
   |                              __________                          __________  __________  __________ |_
   |                             |_74LS245N|                         |_74LS02N_| |74LS273B1| |4116R-001|   |
   |    Xtal                     ______________   BATT                __________  __________               |
   |   18.000 MHz               | EPROM 4     |                      |_ULN2003A| |74LS273B1|               |
   |                            |_____________|                  CONN-> ······     ······ <-CONN           |
   |_______________________________________________________________________________________________________|

  The PCB here was marked as a 'Gum' machine and is from a gambling machine that instead of paying out money would dispense chewing gum as prizes
  Other games with 'Gum' in the title also exist, see 'Chewing Gum' and 'Royal Gum' in other drivers for example, these were likely used with similar
  chewing gum dispensers.

  TourVision was a Spanish developer, PCB had TourVision stickers, but this kind of machine was illegal in Spain, so made for the French market instead

  Ariège Amusements was the exclusive distributor of TourVision products until 1991, Paradise Automatique was a spin-off of this distributor
  and was legally created in 1992 to import/export food vending machines, video games and audiovisual appliances and continued to work with
  TourVision.

  A version of this exists (on newer hardware?) with the title 'Lucky Gum' or 'Luck Gum' however the supported game shows no title screen so the title
  is unknown.
*/
ROM_START( tourpgum )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4.ic52",   0x0000, 0x8000, CRC(58d68a5a) SHA1(e1eb9113d6ebb1cedf5c6724c15b96934e357504) )

	ROM_REGION( 0x18000, "chars", 0 )
	ROM_LOAD( "1_tourvision.ic19",   0x00000, 0x8000, CRC(dbfb5b72) SHA1(efdc66f2288cd66f0b91211d3d1e7e6b20079ab1) )
	ROM_LOAD( "2_tourvision.ic18",   0x08000, 0x8000, CRC(af25ed99) SHA1(9605b36151791b84c2d0648070b0f97e31300dbb) )
	ROM_LOAD( "3_tourvision.ic17",   0x10000, 0x8000, CRC(0b081663) SHA1(86dbf69e819ced12ac7cb7a4839fe0ba677580ae) )
ROM_END

ROM_START( chicgum )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "k13.ic52", 0x0000, 0x8000, CRC(3e01a610) SHA1(86be3d1c3a9810f29701c22d79f262c7e89a2b9b) ) // 1xxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x18000, "chars", 0 )
	ROM_LOAD( "kb.ic19", 0x02000, 0x6000, CRC(90eaa64b) SHA1(867d94a65d7350fa3c0cf84f081056b035385a4a) ) // 00xxxxxxxxxxxxx = 0x00
	ROM_CONTINUE(        0x00000, 0x2000 )
	ROM_LOAD( "kg.ic18", 0x0a000, 0x6000, CRC(0f1394b9) SHA1(9c21b03b080d007ff3c9ec93881efd11a5740bd4) ) // 00xxxxxxxxxxxxx = 0x00
	ROM_CONTINUE(        0x08000, 0x2000 )
	ROM_LOAD( "kr.ic17", 0x12000, 0x6000, CRC(45590724) SHA1(0b8544be3a2b28b7bc0ed8ca72af0e558acd3f1d) ) // 00xxxxxxxxxxxxx = 0x00
	ROM_CONTINUE(        0x10000, 0x2000 )
ROM_END

ROM_START( strker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic52",   0x0000, 0x8000, CRC(745beb7f) SHA1(1ead50897d27e338b768b0335d4dbd9581c93372) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x18000, "chars", 0 )
	ROM_LOAD( "1.ic19",   0x2000, 0x2000, CRC(30b66fcd) SHA1(42b2fb20036e0126abf44b39855eaab449206c71) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(         0x2000, 0x4000 )
	ROM_CONTINUE(         0x6000, 0x2000 )
	ROM_CONTINUE(         0x0000, 0x6000 )
	ROM_CONTINUE(         0x0000, 0x2000 )
	ROM_LOAD( "2.ic18",   0xa000, 0x2000, CRC(002b5537) SHA1(c293e4307a817064ee1c868491ac927c096b9f5d) ) // x00xxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(         0xa000, 0x4000 )
	ROM_CONTINUE(         0xe000, 0x2000 )
	ROM_CONTINUE(         0x8000, 0x6000 )
	ROM_CONTINUE(         0x8000, 0x2000 )
	ROM_LOAD( "3.ic17",   0x12000, 0x2000, CRC(9be6aeb7) SHA1(cc7daa39f30c7dfd529b22815af3b62aad79934d) ) // x00xxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(         0x12000, 0x4000 )
	ROM_CONTINUE(         0x16000, 0x2000 )
	ROM_CONTINUE(         0x10000, 0x6000 )
	ROM_CONTINUE(         0x10000, 0x2000 )
ROM_END

ROM_START( bowlgum ) // VIDEOGUM/TV MADE IN SPAIN PCB, all ROM labels handwritten
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "copy.ic52",   0x0000, 0x8000, CRC(42d70078) SHA1(bc37bfb7e50547250df020872f935c8ab9c3ff9a) )

	ROM_REGION( 0x18000, "chars", 0 )
	ROM_LOAD( "bb.ic19",   0x02000, 0x6000, CRC(cdf78197) SHA1(1aaad919f506ea42345d9d1ee789b19f1fef41a7) )
	ROM_CONTINUE(          0x00000, 0x2000 )
	ROM_LOAD( "bg.ic18",   0x0a000, 0x6000, CRC(b7ab5c2e) SHA1(efe281bbabaef834d3c00ef504503d4c91e2c925) )
	ROM_CONTINUE(          0x08000, 0x2000 )
	ROM_LOAD( "br.ic17",   0x12000, 0x6000, CRC(b87bc6e9) SHA1(1fb29a933b7683965db9d76640dc4fe2651ae19e) ) // 00xxxxxxxxxxxxx = 0x00
	ROM_CONTINUE(          0x10000, 0x2000 )
ROM_END

/*
  Unknown 'Pac-Man' gambling game.
*/
ROM_START(unkpacg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "1.u14",   0x0000, 0x2000, CRC(848c4143) SHA1(3cff26181c58e5f52f1ac81df7d5d43e644585a2) )
	ROM_LOAD( "2.u46",   0x8000, 0x2000, CRC(9e6e0bd3) SHA1(f502132a0460108dad243632cc13d9116c534291) )

	ROM_REGION( 0x6000, "chars", 0 )
	ROM_LOAD( "3.u20",   0x2000, 0x2000, CRC(d00b04ea) SHA1(e65901d8586507257d74ab103001207e28fa28af) )
	ROM_LOAD( "4.u19",   0x4000, 0x2000, CRC(4a123a3d) SHA1(26300b8af0d0df0023a153a212699727311d1b74) )
	ROM_LOAD( "5.u18",   0x0000, 0x2000, CRC(44f272d2) SHA1(b39cbc1f290d9fb2453396906e4da4a682c41ef4) )
ROM_END

ROM_START(unkpacga)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "p1.bin",  0x0000, 0x8000, CRC(386bd2da) SHA1(fa786c25dd5ec1a26ebe021ca701dccebfcbb64f) )  // first 0x5fff are 0xff filled
	ROM_LOAD( "p2.bin",  0x8000, 0x8000, CRC(7878d7f3) SHA1(cacdd4b8e33a93e2913d0f5d740195ef0f439031) )

	ROM_REGION( 0x6000, "chars", 0 )
	ROM_LOAD( "r.bin",   0x0000, 0x2000, CRC(b0d7b67a) SHA1(87bd150ed46d1346a363dc45c226e72967426f2a) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(        0x0000, 0x2000)
	ROM_IGNORE(0x4000)
	ROM_LOAD( "b.bin",   0x2000, 0x2000, CRC(5b26dce5) SHA1(d00434ab352169eca3c458917d5d1a04d0d2c2df) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(        0x2000, 0x2000)
	ROM_IGNORE(0x4000)
	ROM_LOAD( "g.bin",   0x4000, 0x2000, CRC(e12d34e0) SHA1(96790eec9032ca6f513cf0f6a1962d91a21ce2ae) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(        0x4000, 0x2000)
	ROM_IGNORE(0x4000)
ROM_END

ROM_START(unkpacgb)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "p1.bin",  0x0000, 0x8000, CRC(5cc6b5e1) SHA1(80325eef389f7d6a8c78531fdc6e5b73721eb0b1) )
	ROM_LOAD( "p2.bin",  0x8000, 0x8000, CRC(06b42740) SHA1(0639ec2e31bd81e85a45689929bb67a61599497c) )

	ROM_REGION( 0x6000, "chars", 0 )
	ROM_LOAD( "r.bin",   0x0000, 0x2000, CRC(b0d7b67a) SHA1(87bd150ed46d1346a363dc45c226e72967426f2a) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(        0x0000, 0x2000)
	ROM_IGNORE(0x4000)
	ROM_LOAD( "b.bin",   0x2000, 0x2000, CRC(5b26dce5) SHA1(d00434ab352169eca3c458917d5d1a04d0d2c2df) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(        0x2000, 0x2000)
	ROM_IGNORE(0x4000)
	ROM_LOAD( "g.bin",   0x4000, 0x2000, CRC(e12d34e0) SHA1(96790eec9032ca6f513cf0f6a1962d91a21ce2ae) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(        0x4000, 0x2000)
	ROM_IGNORE(0x4000)
ROM_END

ROM_START(unkpacgc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "4",   0x0000, 0x2000, CRC(9f620694) SHA1(957d5c6636d40a74579d3f20be8f0b7e58516935) )
	ROM_LOAD( "5",   0x8000, 0x2000, CRC(b107ad7e) SHA1(33ab0a63f8a57dd7efd5c5efae7c6e8bda1a65cc) )

	ROM_REGION( 0x6000, "chars", 0 )
	ROM_LOAD( "1",   0x2000, 0x2000, CRC(d00b04ea) SHA1(e65901d8586507257d74ab103001207e28fa28af) )
	ROM_LOAD( "2",   0x4000, 0x2000, CRC(4a123a3d) SHA1(26300b8af0d0df0023a153a212699727311d1b74) )
	ROM_LOAD( "3",   0x0000, 0x2000, CRC(f7cd9de0) SHA1(e0a6b316811ef7c3d3aeb853a9c50f9fdf1f2ff2) )
ROM_END

ROM_START(unkpacgd)
	ROM_REGION(0x10000, "maincpu", 0) // only the first program ROM differs from unkpacgc and only slightly
	ROM_LOAD( "2.bin",   0x0000, 0x2000, CRC(4a545bf6) SHA1(1f71ad1c24e1a9ae6379e3136692fa01509ad5a0) )
	ROM_LOAD( "1.bin",   0x8000, 0x2000, CRC(b107ad7e) SHA1(33ab0a63f8a57dd7efd5c5efae7c6e8bda1a65cc) )

	ROM_REGION( 0x6000, "chars", 0 ) // these are different: they change the characters from pacman related to car racing related
	ROM_LOAD( "3.bin",   0x0000, 0x2000, CRC(ce47a9da) SHA1(12314760d09644a85aef9b1c7b9aa8a965cc9d63) )
	ROM_LOAD( "4.bin",   0x2000, 0x2000, CRC(9a404e8c) SHA1(e7e80f5771250f54d7eaca78f97bb086f9604fd0) )
	ROM_LOAD( "5.bin",   0x4000, 0x2000, CRC(32d8d105) SHA1(d793801a9b761b1713ea8bba747130c31e8571fd) )
ROM_END

/*
  Unknown 'Space Invaders' gambling game.
  All ROMs are 0x8000 but only the last 0x2000 of each is used.
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

	ROM_REGION( 0x6000, "chars", 0 )
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
  All ROMs are 0x10000 but with a lot of addressing issues

  1.bin    BADADDR    ---xxxxxxxxxxxxx
  2.bin    BADADDR    ---xxxxxxxxxxxxx
  b.bin    BADADDR    x-xxxxxxxxxxxxxx
  r.bin    BADADDR    x-xxxxxxxxxxxxxx
  v.bin    BADADDR    x-xxxxxxxxxxxxxx

  The game has both (Space Invaders & Pac-Man) graphics sets.
  Maybe a leftover?...

*/
ROM_START( unksiga )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",  0x0000, 0x2000, CRC(089a4a63) SHA1(d519f6289e72299e48ed1790fa4919cae14e2a0f) )  // 0x2000 of data repeated along the dump
	ROM_IGNORE(                 0xe000)   /* Identical 0x2000 segments */
	ROM_LOAD( "2.bin",  0x8000, 0x2000, CRC(970632fd) SHA1(2aa69fda1dce201856b237ecbedfdcde470a4bb3) )  // 0x2000 of data repeated along the dump
	ROM_IGNORE(                 0xe000)   /* Identical 0x2000 segments */

	ROM_REGION( 0xc000, "chars", 0 )
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

ROM_START( unksigb ) // this set has been found with GFX ROMs of different sizes, but same relevant data. Program ROM isn't encrypted and has further differences to the two other sets.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u144",  0x8000, 0x2000, CRC(8f19f1d3) SHA1(6c7364cdb68974ac600c75d4b8c7646a7f218e27) )
	ROM_CONTINUE(      0x0000, 0x2000 )
	ROM_IGNORE(                0x4000 ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x6000, "chars", 0 )
	ROM_LOAD( "u172", 0x0000, 0x2000, CRC(f8a358fe) SHA1(5c4051de156014a5c2400f4934e2136b38bfed8c) )
	ROM_CONTINUE(     0x0000, 0x2000)
	ROM_CONTINUE(     0x0000, 0x2000)
	ROM_CONTINUE(     0x0000, 0x2000) // only data here matters
	ROM_LOAD( "u171", 0x2000, 0x2000, CRC(56ac5874) SHA1(7ae63f930b07cb1b4989c8328fcc3627d8ff68f8) )
	ROM_CONTINUE(     0x2000, 0x2000)
	ROM_CONTINUE(     0x2000, 0x2000)
	ROM_CONTINUE(     0x2000, 0x2000) // only data here matters
	ROM_LOAD( "u170", 0x4000, 0x2000, CRC(f9c686fc) SHA1(b34412be047e04fc6aca218adf61bbe233908bd7) )
ROM_END

/***********************************
*          Driver Init             *
***********************************/

void unk_gambl_enc_state::driver_start()
{
	// descramble ROM
	uint8_t *rom = memregion("maincpu")->base();
	for (int i = 0x8000; i < 0x10000; i++)
		rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 3, 2, 0, 1);
}

} // anonymous namespace


/***********************************
*           Game Drivers           *
***********************************/

//    YEAR  NAME       PARENT   MACHINE   INPUT     CLASS            INIT        ROT   COMPANY      FULLNAME                                          FLAGS
GAME( 1990, 4enraya,   0,       _4enraya, 4enraya,  _4enraya_state,  empty_init, ROT0, "IDSA",      "4 En Raya (set 1)",                              MACHINE_SUPPORTS_SAVE )
GAME( 1990, 4enrayaa,  4enraya, _4enraya, 4enraya,  _4enraya_state,  empty_init, ROT0, "IDSA",      "4 En Raya (set 2)",                              MACHINE_SUPPORTS_SAVE )

GAME( 1992?, tourpgum, 0,       tourpgum, tourpgum, unk_gambl_state, empty_init, ROT0, u8"Paradise Automatique / TourVisión", u8"unknown Paradise Automatique / TourVisión bowling themed poker game with gum prizes (France)", MACHINE_SUPPORTS_SAVE )
GAME( 1992?, chicgum,  0,       chicgum,  tourpgum, unk_gambl_state, empty_init, ROT0, "<unknown>", "Chic Gum Video", MACHINE_SUPPORTS_SAVE )
GAME( 1992?, strker,   0,       chicgum,  tourpgum, unk_gambl_state, empty_init, ROT0, "<unknown>", "Striker", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // 'RAM NO GOOD', if bypassed it resets after coining up
GAME( 1992?, bowlgum,  0,       chicgum,  tourpgum, unk_gambl_state, empty_init, ROT0, "<unknown>", "Bowling Gum", MACHINE_SUPPORTS_SAVE )

GAME( 199?, unkpacg,   0,       unkpacg,  unkpacg,  unk_gambl_enc_state, empty_init, ROT0, "<unknown>", "unknown 'Pac-Man' gambling game (set 1)",   MACHINE_SUPPORTS_SAVE )
GAME( 199?, unkpacgb,  unkpacg, unkpacg,  unkpacg,  unk_gambl_enc_state, empty_init, ROT0, "<unknown>", "unknown 'Pac-Man' gambling game (set 2)",   MACHINE_SUPPORTS_SAVE )
GAME( 1988, unkpacgc,  unkpacg, unkpacg,  unkpacg,  unk_gambl_state,     empty_init, ROT0, "<unknown>", "Coco Louco",                                MACHINE_SUPPORTS_SAVE )
GAME( 1988, unkpacgd,  unkpacg, unkpacg,  unkpacg,  unk_gambl_state,     empty_init, ROT0, "<unknown>", "unknown 'Pac Man with cars' gambling game", MACHINE_SUPPORTS_SAVE )
GAME( 199?, unkpacga,  unkpacg, unkpacga, unkpacg,  unk_gambl_enc_state, empty_init, ROT0, "IDI SRL",   "Pucman",                                    MACHINE_SUPPORTS_SAVE )

GAME( 199?, unksig,    0,       unkpacg,  unkfr,    unk_gambl_enc_state, empty_init, ROT0, "<unknown>", "unknown 'Space Invaders' gambling game (encrypted, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 199?, unksiga,   unksig,  unkpacg,  unkfr,    unk_gambl_enc_state, empty_init, ROT0, "<unknown>", "unknown 'Space Invaders' gambling game (encrypted, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 199?, unksigb,   unksig,  unkpacg,  unkfr,    unk_gambl_state,     empty_init, ROT0, "<unknown>", "unknown 'Space Invaders' gambling game (unencrypted)",      MACHINE_SUPPORTS_SAVE )
