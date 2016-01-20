// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Roberto Fresca
/****************************************************************************

    MAGIC'S 10
    ----------

    Driver by Pierpaolo Prazzoli.
    Additional work by Roberto Fresca.


    Supported games:

    Magic's 10 (ver. 16.15),        1995, AWP Games.
    Magic's 10 (ver. 16.45),        1995, AWP Games.
    Magic's 10 (ver. 16.54),        1995, AWP Games.
    Magic's 10 (ver. 16.55),        1995, AWP Games.
    Magic's 10 2,                   1997, ABM Games.
    Music Sort (ver 2.02, English), 1995, ABM Games.
    Super Pool (9743 rev.01),       1997, ABM Games.
    Hot Slot (ver. 05.01),          1996, ABM Electronics.
    Magic Colors (ver. 1.7a),       1999, Unknown.
    Super Gran Safari (ver 3.11),   1996, New Impeuropex Corp.


*****************************************************************************


    Game Notes
    ==========


    * Magic's 10

    First time boot instructions:

    - Switch "Disable Free Play" to ON
    - Enter a coin
    - Press Collect to get the 1st game over



    * Super Gran Safari

    There is a input sequence to initialize the game.

    The code expects a mask of 0x4c00 in the DIP switches port to allow
    enter the sequence, so DIP switches must be on default position.

    When you see the black screen, enter the following sequence:
    HOLD 4 (key V), HOLD 2 (key X), HOLD 5 (key B), START (key 1).

    The code is checking for a 5th entry. In fact expects HOLD 3 as the first
    entry, then the rest listed above. I don't know why bypass the first one.
    Input port bits are checked in the following order: 2, 3, 1, 4, 5.

    The player can play the "Super Game" to grab the points.
    In this subgame, you must to hit the lion to get the prize.
    For now, you must miss the shot till hopper & ticket dispenser are properly emulated.


*****************************************************************************


    TODO:

    - Ticket / Hopper support.
    - Some unknown writes
    - Finish magic10_2 (association coin - credits handling its inputs
       and some reads that drive the note displayed?)
    - Dump/decap/trojan the MCU in the later games (magic102, suprpool, hotslot, mcolors).
       The MCU shares memory addresses at $500000-$50001f (in magic102)
       It can't be simulated with a high level of confidence because all the game logic is
       in there, including rngs for the cards and combinations for the points.
    - Priorities,likely to be hardwired with the color writes (0=tile has the
       highest priority).
    - Define parent/clone relationship between Magic's 10 and Music Sort.


****************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"
#include "sgsafari.lh"
#include "musicsrt.lh"


class magic10_state : public driver_device
{
public:
	magic10_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_layer0_videoram(*this, "layer0_videoram"),
		m_layer1_videoram(*this, "layer1_videoram"),
		m_layer2_videoram(*this, "layer2_videoram"),
		m_vregs(*this, "vregs"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	tilemap_t *m_layer0_tilemap;
	tilemap_t *m_layer1_tilemap;
	tilemap_t *m_layer2_tilemap;
	required_shared_ptr<UINT16> m_layer0_videoram;
	required_shared_ptr<UINT16> m_layer1_videoram;
	required_shared_ptr<UINT16> m_layer2_videoram;
	int m_layer2_offset[2];
	required_shared_ptr<UINT16> m_vregs;
	UINT16 m_magic102_ret;
	DECLARE_WRITE16_MEMBER(layer0_videoram_w);
	DECLARE_WRITE16_MEMBER(layer1_videoram_w);
	DECLARE_WRITE16_MEMBER(layer2_videoram_w);
	DECLARE_READ16_MEMBER(magic102_r);
	DECLARE_READ16_MEMBER(hotslot_copro_r);
	DECLARE_WRITE16_MEMBER(hotslot_copro_w);
	DECLARE_WRITE16_MEMBER(magic10_out_w);
	DECLARE_DRIVER_INIT(sgsafari);
	DECLARE_DRIVER_INIT(suprpool);
	DECLARE_DRIVER_INIT(magic102);
	DECLARE_DRIVER_INIT(magic10);
	DECLARE_DRIVER_INIT(hotslot);
	TILE_GET_INFO_MEMBER(get_layer0_tile_info);
	TILE_GET_INFO_MEMBER(get_layer1_tile_info);
	TILE_GET_INFO_MEMBER(get_layer2_tile_info);
	virtual void video_start() override;
	UINT32 screen_update_magic10(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


/***************************
*      Video Hardware      *
***************************/

WRITE16_MEMBER(magic10_state::layer0_videoram_w)
{
	COMBINE_DATA(&m_layer0_videoram[offset]);
	m_layer0_tilemap->mark_tile_dirty(offset >> 1);
}

WRITE16_MEMBER(magic10_state::layer1_videoram_w)
{
	COMBINE_DATA(&m_layer1_videoram[offset]);
	m_layer1_tilemap->mark_tile_dirty(offset >> 1);
}

WRITE16_MEMBER(magic10_state::layer2_videoram_w)
{
	COMBINE_DATA(&m_layer2_videoram[offset]);
	m_layer2_tilemap->mark_tile_dirty(offset >> 1);
}

TILE_GET_INFO_MEMBER(magic10_state::get_layer0_tile_info)
{
	SET_TILE_INFO_MEMBER(1,
		m_layer0_videoram[tile_index * 2],
		m_layer0_videoram[tile_index * 2 + 1] & 0x0f,
		TILE_FLIPYX((m_layer0_videoram[tile_index * 2 + 1] & 0xc0) >> 6));
}

TILE_GET_INFO_MEMBER(magic10_state::get_layer1_tile_info)
{
	SET_TILE_INFO_MEMBER(1,
		m_layer1_videoram[tile_index * 2],
		m_layer1_videoram[tile_index * 2 + 1] & 0x0f,
		TILE_FLIPYX((m_layer1_videoram[tile_index * 2 + 1] & 0xc0) >> 6));
}

TILE_GET_INFO_MEMBER(magic10_state::get_layer2_tile_info)
{
	SET_TILE_INFO_MEMBER(0,
		m_layer2_videoram[tile_index * 2],
		m_layer2_videoram[tile_index * 2 + 1] & 0x0f,0);
}


void magic10_state::video_start()
{
	m_layer0_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(magic10_state::get_layer0_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_layer1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(magic10_state::get_layer1_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_layer2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(magic10_state::get_layer2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_layer1_tilemap->set_transparent_pen(0);
	m_layer2_tilemap->set_transparent_pen(0);
}

UINT32 magic10_state::screen_update_magic10(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/*TODO: understand where this comes from. */
	m_layer2_tilemap->set_scrollx(0, m_layer2_offset[0]);
	m_layer2_tilemap->set_scrolly(0, m_layer2_offset[1]);

	/*
	4 and 6 are y/x global register writes.
	0 and 2 are y/x writes for the scrolling layer.
	*/
	m_layer1_tilemap->set_scrolly(0, (m_vregs[0/2] - m_vregs[4/2])+0);
	m_layer1_tilemap->set_scrollx(0, (m_vregs[2/2] - m_vregs[6/2])+4);

	m_layer0_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_layer1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_layer2_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


/***************************
*       R/W Handlers       *
***************************/

READ16_MEMBER(magic10_state::magic102_r)
{
	m_magic102_ret ^= 0x20;
	return m_magic102_ret;
}

READ16_MEMBER(magic10_state::hotslot_copro_r)
{
	return 0x80;
}

WRITE16_MEMBER(magic10_state::hotslot_copro_w)
{
	logerror("Writing to copro: %d \n", data);
}

WRITE16_MEMBER(magic10_state::magic10_out_w)
{
/*
  ----------------------------------------------
  --- Super Gran Safari & Magic's 10 Outputs ---
  ----------------------------------------------

  0x0000 - Normal State (lamps off).
  0x0001 - Hold 1 lamp.
  0x0002 - Hold 2 lamp.
  0x0004 - Hold 3 lamp.
  0x0008 - Hold 4 lamp.
  0x0010 - Hold 5 lamp.
  0x0020 - Start lamp.
  0x0040 - Play (Bet/Take/Cancel) lamp.
  0x0100 - Payout lamp.
  0x0400 - Coin counter.


    - Lbits -
    7654 3210
    =========
    ---- ---x  Hold1 lamp.
    ---- --x-  Hold2 lamp.
    ---- -x--  Hold3 lamp.
    ---- x---  Hold4 lamp.
    ---x ----  Hold5 lamp.
    --x- ----  Start lamp.
    -x-- ----  Play (Bet/Take/Cancel) lamp.

    - Hbits -
    7654 3210
    =========
    ---- ---x  Payout lamp.
    ---- -x--  Coin counter.

*/

//  popmessage("lamps: %02X", data);

	output().set_lamp_value(1, (data & 1));           /* Lamp 1 - HOLD 1 */
	output().set_lamp_value(2, (data >> 1) & 1);      /* Lamp 2 - HOLD 2 */
	output().set_lamp_value(3, (data >> 2) & 1);      /* Lamp 3 - HOLD 3 */
	output().set_lamp_value(4, (data >> 3) & 1);      /* Lamp 4 - HOLD 4 */
	output().set_lamp_value(5, (data >> 4) & 1);      /* Lamp 5 - HOLD 5 */
	output().set_lamp_value(6, (data >> 5) & 1);      /* Lamp 6 - START  */
	output().set_lamp_value(7, (data >> 6) & 1);      /* Lamp 7 - PLAY (BET/TAKE/CANCEL) */
	output().set_lamp_value(8, (data >> 8) & 1);      /* Lamp 8 - PAYOUT/SUPERGAME */

	machine().bookkeeping().coin_counter_w(0, data & 0x400);
}

/***************************
*       Memory Maps        *
***************************/

static ADDRESS_MAP_START( magic10_map, AS_PROGRAM, 16, magic10_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM_WRITE(layer1_videoram_w) AM_SHARE("layer1_videoram")
	AM_RANGE(0x101000, 0x101fff) AM_RAM_WRITE(layer0_videoram_w) AM_SHARE("layer0_videoram")
	AM_RANGE(0x102000, 0x103fff) AM_RAM_WRITE(layer2_videoram_w) AM_SHARE("layer2_videoram")
	AM_RANGE(0x200000, 0x2007ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x300000, 0x3001ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("INPUTS")
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("DSW")
	AM_RANGE(0x400008, 0x400009) AM_WRITE(magic10_out_w)
	AM_RANGE(0x40000a, 0x40000b) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x40000e, 0x40000f) AM_WRITENOP
	AM_RANGE(0x400080, 0x400087) AM_RAM AM_SHARE("vregs")
	AM_RANGE(0x600000, 0x603fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( magic10a_map, AS_PROGRAM, 16, magic10_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM_WRITE(layer1_videoram_w) AM_SHARE("layer1_videoram")
	AM_RANGE(0x101000, 0x101fff) AM_RAM_WRITE(layer0_videoram_w) AM_SHARE("layer0_videoram")
	AM_RANGE(0x102000, 0x103fff) AM_RAM_WRITE(layer2_videoram_w) AM_SHARE("layer2_videoram")
	AM_RANGE(0x200000, 0x2007ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x300000, 0x3001ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("INPUTS")
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("DSW")
	AM_RANGE(0x500008, 0x500009) AM_WRITE(magic10_out_w)
	AM_RANGE(0x50000a, 0x50000b) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x50000e, 0x50000f) AM_WRITENOP
	AM_RANGE(0x500080, 0x500087) AM_RAM AM_SHARE("vregs")   // video registers?
	AM_RANGE(0x600000, 0x603fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( magic102_map, AS_PROGRAM, 16, magic10_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM_WRITE(layer1_videoram_w) AM_SHARE("layer1_videoram")
	AM_RANGE(0x101000, 0x101fff) AM_RAM_WRITE(layer0_videoram_w) AM_SHARE("layer0_videoram")
	AM_RANGE(0x102000, 0x103fff) AM_RAM_WRITE(layer2_videoram_w) AM_SHARE("layer2_videoram")
	AM_RANGE(0x200000, 0x2007ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x400000, 0x4001ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x500000, 0x500001) AM_READ(magic102_r)
	AM_RANGE(0x500004, 0x500005) AM_READNOP // gives credits
	AM_RANGE(0x500006, 0x500007) AM_READNOP // gives credits
	AM_RANGE(0x50001a, 0x50001b) AM_READ_PORT("IN0")
	AM_RANGE(0x50001c, 0x50001d) AM_READ_PORT("IN1")
	AM_RANGE(0x500002, 0x50001f) AM_READNOP
	AM_RANGE(0x500002, 0x50001f) AM_WRITENOP
	AM_RANGE(0x600000, 0x603fff) AM_RAM
	AM_RANGE(0x700000, 0x700001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x700080, 0x700087) AM_RAM AM_SHARE("vregs")   // video registers?
ADDRESS_MAP_END

static ADDRESS_MAP_START( hotslot_map, AS_PROGRAM, 16, magic10_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM_WRITE(layer1_videoram_w) AM_SHARE("layer1_videoram")
	AM_RANGE(0x101000, 0x101fff) AM_RAM_WRITE(layer0_videoram_w) AM_SHARE("layer0_videoram")
	AM_RANGE(0x102000, 0x103fff) AM_RAM_WRITE(layer2_videoram_w) AM_SHARE("layer2_videoram")
	AM_RANGE(0x200000, 0x2007ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x400000, 0x4001ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x500004, 0x500005) AM_READWRITE(hotslot_copro_r, hotslot_copro_w) // copro comm
	AM_RANGE(0x500006, 0x500011) AM_RAM
	AM_RANGE(0x500012, 0x500013) AM_READ_PORT("IN0")
	AM_RANGE(0x500014, 0x500015) AM_READ_PORT("IN1")
	AM_RANGE(0x500016, 0x500017) AM_READ_PORT("IN2")
	AM_RANGE(0x500018, 0x500019) AM_READ_PORT("DSW1")
	AM_RANGE(0x50001a, 0x50001d) AM_WRITENOP
	AM_RANGE(0x600000, 0x603fff) AM_RAM
	AM_RANGE(0x70000a, 0x70000b) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x700080, 0x700087) AM_RAM AM_SHARE("vregs")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sgsafari_map, AS_PROGRAM, 16, magic10_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM_WRITE(layer1_videoram_w) AM_SHARE("layer1_videoram")
	AM_RANGE(0x101000, 0x101fff) AM_RAM_WRITE(layer0_videoram_w) AM_SHARE("layer0_videoram")
	AM_RANGE(0x102000, 0x103fff) AM_RAM_WRITE(layer2_videoram_w) AM_SHARE("layer2_videoram")
	AM_RANGE(0x200000, 0x203fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x300000, 0x3001ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("DSW1")
	AM_RANGE(0x500008, 0x500009) AM_WRITE(magic10_out_w)
	AM_RANGE(0x50000a, 0x50000b) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x50000e, 0x50000f) AM_READ_PORT("IN0")
	AM_RANGE(0x500080, 0x500087) AM_RAM AM_SHARE("vregs")   // video registers?
	AM_RANGE(0x600000, 0x603fff) AM_RAM
ADDRESS_MAP_END
/*

  SGSafari unimplemented writes:

  0x500000 - 0x500007 ; unknown.
  0x50000c - 0x50007f ; unknown.
  0x500080 - 0x500083 ; video registers (layer scroll)
  0x500084 - 0x500087 ; video registers (unknown)
  0x500088 - 0x5000ff ; unknown.

*/

/***************************
*       Input Ports        *
***************************/

static INPUT_PORTS_START( magic10 )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet / Take)")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Lots FC") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note A")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Note B")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Note C")
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Out Hole") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME("Note D") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Display Logo" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Yes ) )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SPECIAL ) // empty dispenser
	PORT_DIPNAME( 0x00ee, 0x00ee, "Disable Free Play" )
	PORT_DIPSETTING(      0x00ee, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0100, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Hardest ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x0c00, 0x0000, "Notes Settings" )
	PORT_DIPSETTING(      0x0000, "Note A: 10 - Note B: 20 - Note C: 50 - Note D: 100" )
	PORT_DIPSETTING(      0x0800, "Note A: 20 - Note B: 40 - Note C: 100 - Note D: 200" )
	PORT_DIPSETTING(      0x0400, "Note A: 50 - Note B: 100 - Note C: 500 - Note D: 1000" )
	PORT_DIPSETTING(      0x0c00, "Note A: 100 - Note B: 200 - Note C: 1000 - Note D: 2000" )
	PORT_DIPNAME( 0x3000, 0x3000, "Lots At" )           PORT_CONDITION("DSW", 0xc000, EQUALS, 0xc000)
	PORT_DIPSETTING(      0x0000, "50 200 500 1000 2000" )
	PORT_DIPSETTING(      0x1000, "100 300 1000 3000 5000" )
	PORT_DIPSETTING(      0x2000, "200 500 2000 3000 5000" )
	PORT_DIPSETTING(      0x3000, "500 1000 2000 4000 8000" )
	PORT_DIPNAME( 0x3000, 0x3000, "1 Ticket Won" )      PORT_CONDITION("DSW", 0xc000, EQUALS, 0x8000)
//  PORT_DIPSETTING(      0x0000, "Every 100 Score" )
//  PORT_DIPSETTING(      0x1000, "Every 100 Score" )
//  PORT_DIPSETTING(      0x2000, "Every 100 Score" )
	PORT_DIPSETTING(      0x3000, "Every 100 Score" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unused ) )   PORT_CONDITION("DSW", 0xc000, EQUALS, 0x4000)
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unused ) )   PORT_CONDITION("DSW", 0xc000, EQUALS, 0x4000)
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, "1 Play Won" )        PORT_CONDITION("DSW", 0xc000, EQUALS, 0x0000)
//  PORT_DIPSETTING(      0x0000, "Every 10 Score" )
//  PORT_DIPSETTING(      0x1000, "Every 10 Score" )
//  PORT_DIPSETTING(      0x2000, "Every 10 Score" )
	PORT_DIPSETTING(      0x3000, "Every 10 Score" )
	PORT_DIPNAME( 0xc000, 0xc000, "Dispenser Type" )
	PORT_DIPSETTING(      0x0000, "MKII Hopper - Supergame" )
	PORT_DIPSETTING(      0x4000, "10 Tokens" )
	PORT_DIPSETTING(      0x8000, "Tickets Dispenser" )
	PORT_DIPSETTING(      0xc000, "Lots Dispenser" )
INPUT_PORTS_END

static INPUT_PORTS_START( magic102 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note A")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Note B")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Note C")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME("Note D") PORT_CODE(KEYCODE_9)
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

/*
    credits inputs

    PORT_START("CRED1")
    PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x01, DEF_STR( On ) )
    PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x02, DEF_STR( On ) )
    PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x04, DEF_STR( On ) )
    PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x08, DEF_STR( On ) )
    PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x10, DEF_STR( On ) )
    PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x20, DEF_STR( On ) )
    PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x40, DEF_STR( On ) )
    PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x80, DEF_STR( On ) )
    PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

    PORT_START("CRED2")
    PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x01, DEF_STR( On ) )
    PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x02, DEF_STR( On ) )
    PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x04, DEF_STR( On ) )
    PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x08, DEF_STR( On ) )
    PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x10, DEF_STR( On ) )
    PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x20, DEF_STR( On ) )
    PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x40, DEF_STR( On ) )
    PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x80, DEF_STR( On ) )
    PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )
*/
INPUT_PORTS_END

static INPUT_PORTS_START( musicsrt )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Heads")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Tails")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Half Gamble")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet / Take)")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Door") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Aux A")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Aux B")
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME("Aux C") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("OK")

	PORT_START("DSW")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0100, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1: 1, 2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1: 3, 4")
	PORT_DIPSETTING(      0x0000, "Coin A: 50 - Coin B: 50" )
	PORT_DIPSETTING(      0x0800, "Coin A: 50 - Coin B: 50" )
	PORT_DIPSETTING(      0x0400, "Coin A: 100 - Coin B: 100" )
	PORT_DIPSETTING(      0x0c00, "Coin A: 100 - Coin B: 100" )
	PORT_DIPNAME( 0x3000, 0x3000, "Bonus?" )                PORT_DIPLOCATION("SW1: 5, 6")
	PORT_DIPSETTING(      0x3000, "1000= 1 Play; 2000= 2 Play; 3000= 3 Play" )
	PORT_DIPSETTING(      0x2000, "2000= 1 Play; 4000= 2 Play; 6000= 3 Play" )
	PORT_DIPSETTING(      0x1000, "2500= 1 Play; 5000= 2 Play; 7500= 3 Play" )
	PORT_DIPSETTING(      0x0000, "5000= 1 Play; 10000= 2 Play; 15000= 3 Play" )
	PORT_DIPNAME( 0x4000, 0x4000, "Hopper" )                PORT_DIPLOCATION("SW1: 7")
	PORT_DIPSETTING(      0x0000, "Disabled" )
	PORT_DIPSETTING(      0x4000, "Enabled" )
	PORT_DIPNAME( 0x8000, 0x8000, "Score" )                 PORT_DIPLOCATION("SW1: 8")
	PORT_DIPSETTING(      0x0000, "Play Score" )
	PORT_DIPSETTING(      0x8000, "No Play Score" )
INPUT_PORTS_END

static INPUT_PORTS_START( hotslot )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Win-Tab") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_HALF ) PORT_NAME("1/2 Win")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note A")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Note B")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Note C")
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME("Note D") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "Coin A=10; B=10" )
	PORT_DIPSETTING(    0x08, "Coin A=10; B=20" )
	PORT_DIPSETTING(    0x04, "Coin A=10; B=50" )
	PORT_DIPSETTING(    0x0c, "Coin A=10; B=100" )
	PORT_DIPNAME( 0x10, 0x10, "Bet Max" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x20, 0x20, "Cum" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Payout" )
	PORT_DIPSETTING(    0x00, "Replay Only" )
	PORT_DIPSETTING(    0x40, "Tokens Only" )
	PORT_DIPSETTING(    0x80, "Tickets Only" )
	PORT_DIPSETTING(    0xc0, "Tickets & Tokens" )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( sgsafari )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Head")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Tail")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Half Gamble")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet / Take / Cancel)")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Coin 1")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Coin 2")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Note B")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Note C")
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME("Note D") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Payout / Super Game")

	PORT_START("DSW1")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x0300,   0x0000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(        0x0300, DEF_STR( Easy ) )
	PORT_DIPSETTING(        0x0200, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00,   0x0c00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(        0x0c00, DEF_STR( 1C_1C ) )      PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(        0x0800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(        0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(        0x0000, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x3000,   0x0000, "Payout Options" )      PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(        0x3000, "Pay at 100 points" )
	PORT_DIPSETTING(        0x2000, "Pay at 200 points" )
	PORT_DIPSETTING(        0x1000, "Pay at 400 points" )
	PORT_DIPSETTING(        0x0000, "Pay at 500 points" )
	PORT_DIPNAME( 0x4000,   0x4000, "Tickets" )             PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(        0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000,   0x0000, "Hopper" )              PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(        0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/****************************
*     Graphics Layouts      *
****************************/

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4),RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};


/****************************
*      Graphics Decode      *
****************************/

static GFXDECODE_START( magic10 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout,   0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16_layout, 0, 16 )
GFXDECODE_END


/****************************
*      Machine Drivers      *
****************************/

static MACHINE_CONFIG_START( magic10, magic10_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) // ?
	MCFG_CPU_PROGRAM_MAP(magic10_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", magic10_state,  irq1_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 44*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(magic10_state, screen_update_magic10)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(xxxxBBBBRRRRGGGG)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", magic10)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH)   /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( magic10a, magic10 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(magic10a_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( magic102, magic10 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(magic102_map)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 0*8, 30*8-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( hotslot, magic10 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hotslot_map)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(8*8, 56*8-1, 2*8, 32*8-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sgsafari, magic10 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sgsafari_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", magic10_state,  irq2_line_hold)    /* L1 interrupts */

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0*8, 44*8-1, 0*8, 30*8-1)
MACHINE_CONFIG_END


/****************************
*        Rom Loads          *
****************************/

/*

Magic 10 (videopoker)

1x 68k
1x 20mhz OSC near 68k
1x Oki M6295
1x 30mhz OSC near oki chip
2x fpga
1x bank of Dipswitch
1x Dallas Ds1220y-200 Nonvolatile ram

*/
ROM_START( magic10 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "u3.bin", 0x000000, 0x20000, CRC(191a46f4) SHA1(65bc22cdcc4b2f102d3eef595626819af709cacb) )
	ROM_LOAD16_BYTE( "u2.bin", 0x000001, 0x20000, CRC(a03a80bc) SHA1(a21da8912f1d2c8c2fa4a8d3ce4d43da8a934e21) )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "u25.bin", 0x00000, 0x20000, CRC(7abb8136) SHA1(1d4daf6a4477853d89d08afb524516ef79f60dd6) )
	ROM_LOAD( "u26.bin", 0x20000, 0x20000, CRC(fd0b912d) SHA1(1cd15fa3459e7fece9fc37595f2b6848c00ffa43) )
	ROM_LOAD( "u27.bin", 0x40000, 0x20000, CRC(8178c907) SHA1(8c3440769ed4e113d84d1f8f9079783497791859) )
	ROM_LOAD( "u28.bin", 0x60000, 0x20000, CRC(dfd41aab) SHA1(82248c7fa4febb1c453f35a0e4cfae062c5da2d5) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "u22.bin", 0x00000, 0x40000, CRC(98885246) SHA1(752d549e6248074f2a7f6c5cc4d0bbc44c7fa4c3) )
ROM_END

ROM_START( magic10a )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "u3.bin", 0x000000, 0x20000, CRC(3ef8736c) SHA1(0c36c516349cf2c843c4beb64c979b73da56183d) ) // sldh
	ROM_LOAD16_BYTE( "u2.bin", 0x000001, 0x20000, CRC(c30507fc) SHA1(ca15e30e9078dae2177df1ec33c94b37317ced61) ) // sldh

	ROM_REGION( 0x80000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "u25.bin", 0x00000, 0x20000, CRC(7abb8136) SHA1(1d4daf6a4477853d89d08afb524516ef79f60dd6) )
	ROM_LOAD( "u26.bin", 0x20000, 0x20000, CRC(fd0b912d) SHA1(1cd15fa3459e7fece9fc37595f2b6848c00ffa43) )
	ROM_LOAD( "u27.bin", 0x40000, 0x20000, CRC(8178c907) SHA1(8c3440769ed4e113d84d1f8f9079783497791859) )
	ROM_LOAD( "u28.bin", 0x60000, 0x20000, CRC(dfd41aab) SHA1(82248c7fa4febb1c453f35a0e4cfae062c5da2d5) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "u22.bin", 0x00000, 0x40000, CRC(98885246) SHA1(752d549e6248074f2a7f6c5cc4d0bbc44c7fa4c3) )
ROM_END

ROM_START( magic10b )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "u3_1645.bin",  0x00000, 0x20000, CRC(7f2549e4) SHA1(6578ad29273c357faae7c6be3fa1b49087e088a2) )
	ROM_LOAD16_BYTE( "u2_1645.bin",  0x00001, 0x20000, CRC(c075234e) SHA1(d9bc38f0b984082a77088fbb52b02c8f5c49846c) )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "u25.bin", 0x00000, 0x20000, CRC(7abb8136) SHA1(1d4daf6a4477853d89d08afb524516ef79f60dd6) )
	ROM_LOAD( "u26.bin", 0x20000, 0x20000, CRC(fd0b912d) SHA1(1cd15fa3459e7fece9fc37595f2b6848c00ffa43) )
	ROM_LOAD( "u27.bin", 0x40000, 0x20000, CRC(8178c907) SHA1(8c3440769ed4e113d84d1f8f9079783497791859) )
	ROM_LOAD( "u28.bin", 0x60000, 0x20000, CRC(dfd41aab) SHA1(82248c7fa4febb1c453f35a0e4cfae062c5da2d5) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "u22.bin", 0x00000, 0x40000, CRC(98885246) SHA1(752d549e6248074f2a7f6c5cc4d0bbc44c7fa4c3) )
ROM_END

/*

Magic's 10 (ver. 16.15)

1995, A.W.P. Games
Version: 16.15

CPU:
1x TS68000P12 (main)(u1)
2x TPC1020AFN-084C (PLD)(not dumped)(u41,u60)

Sound:
1x OKI M6295 (u21)
1x TDA2003 (u24)
1x LM358N

1x oscillator 20.000000MHz (close to main)(osc1)
1x oscillator 30.000MHz (close to sound)(osc2)
1x orange resonator 1000J (close to sound)(xtal1)

ROMs:
1x M27C2001 (1)
6x M27C1001 (2,3,5,6,7)
1x TMS27C010A (4)
1x PALCE16V8H (read protected)

Note:
1x 28x2 edge connector
1x trimmer (volume)
1x 8x2 switches dip
1x battery

*/
ROM_START( magic10c )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "2.u3", 0x000000, 0x20000, CRC(32c12ad6) SHA1(93340df2c0f4c260837bd6649008e26a17a22015) )
	ROM_LOAD16_BYTE( "3.u2", 0x000001, 0x20000, CRC(a9945aaa) SHA1(97d4f6441b96618f2e3ce14095ffc5628cb14f0e) )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "6.u25", 0x00000, 0x20000, CRC(7abb8136) SHA1(1d4daf6a4477853d89d08afb524516ef79f60dd6) )
	ROM_LOAD( "4.u26", 0x20000, 0x20000, CRC(fd0b912d) SHA1(1cd15fa3459e7fece9fc37595f2b6848c00ffa43) )
	ROM_LOAD( "5.u27", 0x40000, 0x20000, CRC(8178c907) SHA1(8c3440769ed4e113d84d1f8f9079783497791859) )
	ROM_LOAD( "7.u28", 0x60000, 0x20000, CRC(dfd41aab) SHA1(82248c7fa4febb1c453f35a0e4cfae062c5da2d5) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "1.u22", 0x00000, 0x40000, CRC(98885246) SHA1(752d549e6248074f2a7f6c5cc4d0bbc44c7fa4c3) )
ROM_END

/*

pcb is marked: Copyright ABM - 9605 Rev.02

1x 68000
1x osc 30mhz
1x osc 20mhz (near the 68k)
1x h8/330 HD6473308cp10
1x dipswitch
1x battery
1x fpga by Actel
1x oki6295

*/
ROM_START( magic102 )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "2.u3",  0x00000, 0x20000, CRC(6fc55fe4) SHA1(392ad92e55aeac9bf5235cceb6b0b415942105a4) )
	ROM_LOAD16_BYTE( "1.u2",  0x00001, 0x20000, CRC(501507af) SHA1(ceed50c9380a9838cd3d171d2387334edfeff77f) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* h8/330 HD6473308cp10 with internal ROM */
	ROM_LOAD( "mcu",        0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "3.u35",        0x00000, 0x20000, CRC(df47bb12) SHA1(b8bcbc9ab764d3159344d93776d13a14c9154086) )
	ROM_LOAD( "4.u36",        0x20000, 0x20000, CRC(dc242034) SHA1(6a2983c79776df07f29b77f23799fef6f20df24f) )
	ROM_LOAD( "5.u37",        0x40000, 0x20000, CRC(a048e26e) SHA1(788c28470298896902120e74fd8b9b283b8e9b79) )
	ROM_LOAD( "6.u38",        0x60000, 0x20000, CRC(469efb34) SHA1(b16646fb0c4757132e272b3877cf546b6f616786) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "7.u32",        0x00000, 0x40000, CRC(47804af7) SHA1(602dc0361869b52532e2adcb0de3cbdd042761b3) )
ROM_END

/*

Super Pool

ABM (Nazionale Elettronica Giochi S.A.S.), 1998.
9743 Rev.01

1x MC68HC000P10
1x ACTEL A1020B-PL84C
1x HD6473308CP10 (label says: do not remove version 1.2)
1x U6295 (sound)
1x LM358N (sound)
1x TDA2003 (sound)
1x oscillator 20.000MHz
1x oscillator 30.0000MHz
1x blu resonator 1000J (close to sound)

1x M27C2001 (1) (Sound)
2x TMS27C010A (2,3) (main)
4x TMS27C010A (4,5,6,7) (gfx)
1x PALCE22V10H (not dumped)
1x PALCE16V8H (not dumped)

1x 28x2 JAMMA edge connector
1x 12 legs connector (J1)
1x trimmer (volume)
1x 8x2 switches dip
1x lithium battery


  STATUS:

  Memory map = done.
  Inputs =     done.
  Machine =    done.

  OKI 6295 =     ok.
  Screen size =  ok.
  Fixed layers = yes.

*/
ROM_START( suprpool )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "2-1.22a.u3", 0x00000, 0x20000,CRC(5d15037a) SHA1(74cab79a1b08910267262a4c6b501126a4df6cda) )
	ROM_LOAD16_BYTE( "3-1.22a.u2", 0x00001, 0x20000,CRC(c762cd1c) SHA1(ee05a9e8147d613eb14333e6e7b743fc05982e7c) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* h8/330 HD6473308cp10 with internal ROM */
	ROM_LOAD( "mcu",        0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* graphics */
	ROM_LOAD( "7.u35", 0x00000, 0x20000,  CRC(357d145f) SHA1(9fea0d0c5d6c27bf520c4f81eb0f48a65ff60142) )
	ROM_LOAD( "6.u36", 0x20000, 0x20000,  CRC(c4448813) SHA1(6e168eb8503b852179f2d743f1cba935592e0a60) )
	ROM_LOAD( "5.u37", 0x40000, 0x20000,  CRC(6e99af07) SHA1(85e7a76724fd9ce8d07b5088cb6e0d933fd95692) )
	ROM_LOAD( "4.u38", 0x60000, 0x20000,  CRC(0660a169) SHA1(1cb34b3da4b144028519a3c5b32ef7da44af0624) )

	ROM_REGION( 0x080000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "1.u32", 0x00000, 0x40000, CRC(47804af7) SHA1(602dc0361869b52532e2adcb0de3cbdd042761b3) )
ROM_END

/*

Hot Slot

CPU:
1x HD6473308CP10 (u24)(main)
1x A40MX04-PL84-9828 (u50)(main)
1x missing PLD (u1)

1x 6295 (u31)(sound)
1x KA358 (u33)(sound)
1x TDA2003 (u34)(sound)

1x oscillator 20.0000MHz (OSC1)
1x 1000J blu resonator (XTAL1)

ROMs:
3x 27C2001 (1,4,6)
2x 27C020 (5,7)
2x 27C010 (2,3)
1x GAL16V8D (as PAL16R4)(read protected)
1x missing PAL22V10

Note:
1x 28x2 edge connector
1x trimmer (volume)
1n trimmer (unknown)
3x 12 legs connector (J1,J2,J3)
1x 8x2 switches DIP

- Co-processor is unknown, but fits in a QFP68 socket.
- The system RAM test need the bit 7 of offset 0x500005 activated to be successful.
  This offset seems to be a kind of port.

  code:

  0x00f550  move.b  #$b,  $500005
  0x00f558  btst    #$7,  $500005
  0x00f560  beq     $f558
  ....

  seems to copy some bytes (maybe commands) and wait for the status on bit 7


  STATUS:

  Memory map = done.
  Inputs =     done.
  Machine =    done.

  OKI 6295 =     ok.
  Screen size =  ok.
  Fixed layers = yes.

*/
ROM_START( hotslot )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "hotslot2.u3", 0x00000, 0x20000, CRC(676cbe32) SHA1(78721326f3334fcdfdaffb72dbcacfb8bb591d51) )
	ROM_LOAD16_BYTE( "hotslot3.u2", 0x00001, 0x20000, CRC(2c362765) SHA1(c41741c97fe8e5b3a66eb08ebf68d24c6c771ba8) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* h8/330 HD6473308cp10 with internal ROM */
	ROM_LOAD( "mcu",        0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* graphics */
	ROM_LOAD( "hotslot7.u35", 0x00000, 0x40000, CRC(715073c2) SHA1(39085871fee182a9b22c3e042211e76da0ee3024) )
	ROM_LOAD( "hotslot6.u36", 0x40000, 0x40000, CRC(8ef2e25a) SHA1(d4a3288878fabab7ea193d5dadde1fe9fea6bc8a) )
	ROM_LOAD( "hotslot5.u37", 0x80000, 0x40000, CRC(98375b25) SHA1(2167f3374bdfc5e1fef7b9ec4361bc68223876b8) )
	ROM_LOAD( "hotslot4.u38", 0xc0000, 0x40000, CRC(cc8a241a) SHA1(8c6ea51d5f7475be79775df0b976ffddc5a960ed) )

	ROM_REGION( 0x080000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "hotslot1.u32", 0x00000, 0x40000, CRC(ae880970) SHA1(3c302b3f6f6bbf72a522889592add3b6ef8ce1b0) )
ROM_END

/*

Magic Colors

CPU:
1x HD6473308CP10 (u24)(main)
1x A40MX04-PL84-9828 (u50)(main)
1x missing PLD (u1)

1x M6295 (u31)(sound)
1x KA358 (u33)(sound)
1x TDA2003 (u34)(sound)

1x oscillator 20.0000MHz (OSC1)
1x 1000J blu resonator (XTAL1)

ROMs:
6x 27C010 (2,3,4,5,6,7)
1x 27C020 (1)
1x GAL16V8D (as PAL16R4)(read protected)
1x missing PAL22V10

Note:
1x 28x2 edge connector
1x trimmer (volume)
1x 12 legs connector (J1,J2,J3)


  STATUS:

  Memory map = done.
  Inputs =     done.
  Machine =    done.

  OKI 6295 =     ok.
  Screen size =  ok.
  Fixed layers = yes.

*/
ROM_START( mcolors )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "m.colors1.7a-2.u3", 0x00000, 0x20000, CRC(02ce6aab) SHA1(349cb639024a818cb88e911788a0146f48d25333) )
	ROM_LOAD16_BYTE( "m.colors1.7a-3.u2", 0x00001, 0x20000, CRC(076b9680) SHA1(856d1cfaca886d78a36e129a7b41455362932e66) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* h8/330 HD6473308cp10 with internal ROM */
	ROM_LOAD( "mcu",        0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* graphics */
	ROM_LOAD( "m.colors1.7-7.u35", 0x00000, 0x20000, CRC(ec44b289) SHA1(269c965112f0ba308bb5f02d965e32df70310b2c) )
	ROM_LOAD( "m.colors1.7-6.u36", 0x20000, 0x20000, CRC(44e550e2) SHA1(abfc05b386efb0f9ad7479ff53079e6ecbaec137) )
	ROM_LOAD( "m.colors1.7-5.u37", 0x40000, 0x20000, CRC(ec363d0d) SHA1(283f0bf3e3d76d64389f0abdffbeaa3d538b8991) )
	ROM_LOAD( "m.colors1.7-4.u38", 0x60000, 0x20000, CRC(7845667d) SHA1(66b1409b8b661b95e2658385da9c2662430d8030) )

	ROM_REGION( 0x080000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "m.color1.u32", 0x00000, 0x40000, CRC(db8d6769) SHA1(2ab7730fd8ae9522e5452fe1f535002e11db5e7b) )
ROM_END

/*

Super Gran Safari
1996 - New Impeuropex Corp.

CPU:
1x MC68000P12 (main)
2x A1020B-PL84C (not dumped)

1x M6295 (sound)
1x TDA2002 (sound)
1x GL324 (sound)

1x oscillator 30.000MHz

ROMs:
2x M27C512 (1,2)
1x M27C2001 (3)
4x M27C1001 (4,5,6,7)

Note:
1x JAMMA edge connector
1x 12 legs connector (j2)
1x 8x2 switches dip
1x 4 legs jumper (j3)
1x 2 legs jumper (j4)
1x trimmer (volume)


  STATUS:

  Memory map = done.
  Inputs =     done.
  Machine =    done.

  OKI 6295 =     ok.
  Screen size =  ok.
  Fixed layers = yes.

*/
ROM_START( sgsafari )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "2.u7", 0x00000, 0x10000, CRC(797ceeac) SHA1(19055b6700f8523785790992adfeb67faa2358e0) )
	ROM_LOAD16_BYTE( "1.u2", 0x00001, 0x10000, CRC(549872f5) SHA1(2228c51541e3b059d5b16f50387e4215b82f78f6) )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* graphics */
	ROM_LOAD( "4.u15", 0x00000, 0x20000, CRC(f9233481) SHA1(1d1aca9a61f0285a6f6f12f6169d9cfc2c5e6991) )
	ROM_LOAD( "5.u18", 0x20000, 0x20000, CRC(9561aa47) SHA1(140e0d9104c677de911d4d12ff617d84449d907b) )
	ROM_LOAD( "6.u16", 0x40000, 0x20000, CRC(91c22541) SHA1(e419a2d5e71b6c64992a08fa9bd82718350ca7da) )
	ROM_LOAD( "7.u19", 0x60000, 0x20000, CRC(3e3a5fbd) SHA1(c3511b488ecb4759a5fdea478007a4a1c2b5f9e0) )

	ROM_REGION( 0x040000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "3.u39", 0x00000, 0x40000, CRC(43257bb5) SHA1(993fbeb6ee0a8a4da185303ec24eee8424b90cd0) )
ROM_END

/*

Music Sort (Ver. 2.02).
Same PCB than Magic's 10 (ver. 16.15)

CPU:
1x TS68000P12 (main)
2x TPC1020AFN-084C (PLD)(not dumped)

Sound:
1x OKI M6295
1x TDA2003
1x LM358N

1x oscillator 20.000000MHz (close to main)
1x oscillator 30.000MHz (close to sound)
1x orange resonator 1000J (close to sound)

Note:
1x 28x2 edge connector
1x trimmer (volume)
1x 8x2 switches dip
1x battery

*/
ROM_START( musicsrt )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "2.u3", 0x000000, 0x20000, CRC(6a5cd39f) SHA1(c7ec0d9a640ff876bd9362bfe896ebc09795b418) )
	ROM_LOAD16_BYTE( "3.u2", 0x000001, 0x20000, CRC(7af68760) SHA1(08d333037a70cda60df9b0c288e9f6eb6fa7eb84) )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "6.u25", 0x00000, 0x20000, CRC(9bcf89a6) SHA1(5b16ef9482249585a714cf2d3efffddd3f0e5834) )
	ROM_LOAD( "4.u26", 0x20000, 0x20000, CRC(b9397659) SHA1(f809f612fd6a7ecfdb0fa55260ef7a57f00c0733) )
	ROM_LOAD( "5.u27", 0x40000, 0x20000, CRC(36d7aeb3) SHA1(2c0863f2f366008640e8a19587460a30fda4ad6e) )
	ROM_LOAD( "7.u28", 0x60000, 0x20000, CRC(a03e750b) SHA1(046e3eb5671bed09d9e5fd3572a8d41ac9e8b69e) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "1.u22", 0x00000, 0x40000, CRC(98885246) SHA1(752d549e6248074f2a7f6c5cc4d0bbc44c7fa4c3) )

	ROM_REGION( 0x0800, "nvram", 0 ) /* default Non Volatile RAM */
	ROM_LOAD( "musicsrt_nv.bin", 0x0000, 0x0800, CRC(f4e063cf) SHA1(a60bbd960bb7dcf023417e8c7164303b6ce71014) )
ROM_END


/****************************
*       Driver Init         *
****************************/

DRIVER_INIT_MEMBER(magic10_state,magic10)
{
	m_layer2_offset[0] = 32;
	m_layer2_offset[1] = 2;
}

DRIVER_INIT_MEMBER(magic10_state,magic102)
{
	m_layer2_offset[0] = 8;
	m_layer2_offset[1] = 20;
}

DRIVER_INIT_MEMBER(magic10_state,suprpool)
{
	m_layer2_offset[0] = 8;
	m_layer2_offset[1] = 16;
}

DRIVER_INIT_MEMBER(magic10_state,hotslot)
{
/*  a value of -56 center the playfield, but displace the intro and initial screen.
    a value of -64 center the intro and initial screen, but displace the playfield.
*/
	m_layer2_offset[0] = -56;   // X offset.
	m_layer2_offset[1] = 0; // Y offset.
}

DRIVER_INIT_MEMBER(magic10_state,sgsafari)
{
	m_layer2_offset[0] = 16;
	m_layer2_offset[1] = 20;
}


/******************************
*        Game Drivers         *
******************************/

/*     YEAR  NAME      PARENT    MACHINE   INPUT     STATE          INIT      ROT    COMPANY                 FULLNAME                         FLAGS            LAYOUT  */
GAMEL( 1995, magic10,  0,        magic10,  magic10,  magic10_state, magic10,  ROT0, "A.W.P. Games",         "Magic's 10 (ver. 16.55)",        0,               layout_sgsafari )
GAMEL( 1995, magic10a, magic10,  magic10,  magic10,  magic10_state, magic10,  ROT0, "A.W.P. Games",         "Magic's 10 (ver. 16.54)",        0,               layout_sgsafari )
GAMEL( 1995, magic10b, magic10,  magic10a, magic10,  magic10_state, magic10,  ROT0, "A.W.P. Games",         "Magic's 10 (ver. 16.45)",        0,               layout_sgsafari )
GAMEL( 1995, magic10c, magic10,  magic10a, magic10,  magic10_state, magic10,  ROT0, "A.W.P. Games",         "Magic's 10 (ver. 16.15)",        0,               layout_sgsafari )
GAME(  1997, magic102, 0,        magic102, magic102, magic10_state, magic102, ROT0, "ABM Games",            "Magic's 10 2 (ver 1.1)",         MACHINE_NOT_WORKING                 )
GAME(  1997, suprpool, 0,        magic102, magic102, magic10_state, suprpool, ROT0, "ABM Games",            "Super Pool (9743 rev.01)",       MACHINE_NOT_WORKING                 )
GAME(  1996, hotslot,  0,        hotslot,  hotslot,  magic10_state, hotslot,  ROT0, "ABM Electronics",      "Hot Slot (ver. 05.01)",          MACHINE_NOT_WORKING                 )
GAME(  1999, mcolors,  0,        magic102, magic102, magic10_state, magic102, ROT0, "<unknown>",            "Magic Colors (ver. 1.7a)",       MACHINE_NOT_WORKING                 )
GAMEL( 1996, sgsafari, 0,        sgsafari, sgsafari, magic10_state, sgsafari, ROT0, "New Impeuropex Corp.", "Super Gran Safari (ver 3.11)",   0,               layout_sgsafari )
GAMEL( 1995, musicsrt, 0,        magic10a, musicsrt, magic10_state, magic10,  ROT0, "ABM Games",            "Music Sort (ver 2.02, English)", 0,               layout_musicsrt )
