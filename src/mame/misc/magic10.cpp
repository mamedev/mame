// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Roberto Fresca
/************************************************************************************************************************************

  MAGIC'S 10
  ----------

  Driver by Pierpaolo Prazzoli.
  Additional work by Roberto Fresca.

  Supported games:
                                                             lex 425   boot      test     MC68000     MCU     NVRAM or  PCB
  Game                           year  manufacturer          19951006  sequence  at boot  size+place  H8/330  battery   marking

  Music Sort (ver. 2.02)         1995  ABM Games             pre lex   direct    yes       ?          no      NVRAM     ?
  Magic's 10 (ver. 16.15)        1995  A.W.P. Games          post lex  direct    yes       DIP H      no      battery   039
  Magic's 10 (ver. 16.45)        1995  A.W.P. Games          post lex  direct    yes       ?          no      NVRAM     ?
  Magic's 10 (ver. 16.54)        1995  A.W.P. Games          post lex  direct    yes       ?          no      ?         ?
  Magic's 10 (ver. 16.55)        1995  A.W.P. Games          post lex  direct    yes       DIP H      no      NVRAM     040
  Soccer 10 (ver. 16.44)         1996  Unknown               post lex  direct    yes       DIP H      no      NVRAM     none
  Hot Slot (ver. 05.01)          1996  ABM Games             post lex  direct    yes       LCC        no      battery   ?
  Super Gran Safari (ver. 3.11)  1996  New Impeuropex Corp.  post lex  [1]       yes       DIP V      no      NVRAM     COMP01
  Magic's 10 2 (ver. 1.1)        1997  ABM Games             post lex  [2]       no        ?          yes     battery   9605 Rev.02
  Super Pool (ver. 1.2)          1998  ABM Games             post lex  [3]       no        DIP V      yes     battery   9743 Rev.01
  Luna Park (ver. 1.2)           1998  ABM Games             post lex  [3]       no        LCC        yes     battery   9743 Rev.02
  Magic Colors (ver. 1.6)        1999  ABM Games             post lex  [3]       no        DIP V      yes     battery   9743 Rev.01
  Magic Colors (ver. 1.7a)       1999  ABM Games             post lex  [3]       no        LCC        yes     none      Rev.03
  Alta Tensione (ver. 2.01a)     1999  Unknown               post lex  [3]       no        LCC        yes     battery   H3
  Super Petrix                   199?  Unknown               post lex  [4]       yes       DIP V      no      NVRAM     COMP01

*************************************************************************************************************************************

  Game Notes & Boot Sequence
  ============================

  * Magic's 10

  First time boot instructions:

  - Switch "Disable Free Play" to ON
  - Enter a coin
  - Press Collect to get the 1st game over


  * [1] Super Gran Safari

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


  * [2] Magic's 10 2

  First time boot instructions:

  As soon as you get the "DATI NVRAM-68K NON VALIDI!!!" screen...

  - Press F2 to enter the Test Mode.
  - Press HOLD 5 (AZZERAMENTO) (key 'b')
  - Press HOLD 1 (AZZERAMENTO TOTALE) (key 'z')
  - Press START (SI) (key '1') when the legend "CONFERMI AZZERAMENTO TOTALE" appear.
  - Press COLLECT (key 'i') twice to leave the Test Mode and start the game...


  * [3] Alta Tensione, Luna Park, Magic Colors, Super Pool

  First time boot instructions:

  As soon as you get the "DATI NVRAM-68K NON VALIDI!!!" screen...

  - Press F2 to enter the Test Mode.
  - Press HOLD 5 (AZZERAMENTO) (key 'b')
  - Press HOLD 1 (AZZERAMENTO TOTALE) (key 'z')
  - Press HOLD 4 (SI) (key 'v') when the legend "CONFERMI AZZERAMENTO TOTALE" appear.
  - Press BET (key 'm') twice to leave the Test Mode and start the game...


  * [4] Super Petrix
  First time boot instructions:

  When you see the HARDWARE TEST screen enter the following sequence
  HOLD1, HOLD1, HOLD3, HOLD2

  The game will now boot into Super Petrix.
  There is a hidden poker game which can be selected in various ways.
  1. Toggle the "Switch to Poker Mode" input. From now on the game will only run Poker.
  2. The player can select Poker from Tetris by pressing HOLD1, HOLD2, HOLD3, HOLD4, START
     in sequence.
  3. The operator can use a hidden test to enable other methods of entering Poker such
     as completing one line of Tetris, completing one level of Tetris and other methods.

  There is a hidden test menu which allows various settings to be changed, this
  includes ticket payout, changing the player sequence to enter Poker etc.
  This is entered as follows:
  1. Set DIP switches to OFF, OFF, ON, ON, OFF, OFF, ON, ON
  2. Enter normal test by pressing F2 while VERSION 1P or HARDWARE TEST is displayed.
  3. Enter advanced setup.
  4. Change LEVEL to NORMAL.
  5. Change FREE PLAY to YES
  6. Press and hold HOLD3, HOLD5 and START until the horizontal scrolling line stops scrolling.
  7. Press HOLD4, HOLD5 and START together and release to enter hidden menu.

  Note that ticket payout won't work with the current ticket driver.

*****************************************************************************

There are basically 2 hardware setup:

The OLDER one based on
1x MC68000 (main)
1x M6295 (sound)

and the NEWER one based on
1x MC68000 (main)
1x H8/330 (MCU)
1x M6295 (sound)

Both setups show different variants for components layout, memory size, NVRAM, etc.etc.

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
  - Position of "PUSH IP START" and "SUPER PETRIX" logo don't match actual hardware
    but fixing these causes the poker winplan to be in the wrong place.
  - Super Petrix and Super Gran Safari use the same board but spetrix accesses the sound
    chip on the upper byte of the data bus and sgsafari on the lower. Is this just
    a programming error on spetrix ?
  - Super Petrix attract mode consists of interleaved blue and gold blocks with the
    blue scrolling diagonally. The blocks are out of alignment which is caused by
    update_screen doing m_tilemap[1]->set_scrollx(0, (m_vregs[2 / 2] - m_vregs[6 / 2]) + 4)
    Removing the +4 fixes the problem but does it cause other issues ?


****************************************************************************/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "sgsafari.lh"
#include "musicsrt.lh"


namespace {

#define AUX_CLOCK     XTAL(30'000'000)


class magic10_base_state : public driver_device
{
public:
	magic10_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram%u", 0U),
		m_vregs(*this, "vregs"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }


protected:
	virtual void video_start() override;

	void base(machine_config &config);
	template <uint8_t Which> void videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_layer0_tile_info);
	TILE_GET_INFO_MEMBER(get_layer1_tile_info);
	TILE_GET_INFO_MEMBER(get_layer2_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	tilemap_t *m_tilemap[3]{};
	required_shared_ptr_array<uint16_t, 3> m_videoram;
	int8_t m_layer2_offset[2]{};

	required_shared_ptr<uint16_t> m_vregs;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

class magic10_state : public magic10_base_state
{
public:
	magic10_state(const machine_config &mconfig, device_type type, const char *tag) :
		magic10_base_state(mconfig, type, tag),
		m_ticket(*this, "ticket"),
		m_hopper(*this, "hopper"),
		m_lamps(*this, "lamp%u", 1U)
	{ }

	void magic10(machine_config &config);
	void magic10a(machine_config &config);
	void sgsafari(machine_config &config);
	void soccer10(machine_config &config);

	void init_magic10();
	void init_sgsafari();
	void init_soccer10();

protected:
	virtual void machine_start() override { m_lamps.resolve(); }

	required_device<ticket_dispenser_device> m_ticket;
	required_device<ticket_dispenser_device> m_hopper;

private:
	output_finder<8> m_lamps;

	void out_w(uint16_t data);
	void magic10_map(address_map &map);
	void magic10a_map(address_map &map);
	void sgsafari_map(address_map &map);
};

class magic102_state : public magic10_base_state
{
public:
	magic102_state(const machine_config &mconfig, device_type type, const char *tag) :
		magic10_base_state(mconfig, type, tag)
	{ }

	void magic102(machine_config &config);

	void init_altaten();
	void init_magic102();
	void init_suprpool();

protected:
	virtual void machine_start() override { save_item(NAME(m_ret)); }

private:
	uint16_t r();
	uint16_t m_ret = 0;

	void map(address_map &map);
};

class hotslot_state : public magic10_state
{
public:
	hotslot_state(const machine_config &mconfig, device_type type, const char *tag) :
		magic10_state(mconfig, type, tag)
	{ }

	void hotslot(machine_config &config);

	void init_hotslot();

private:
	uint16_t copro_r();
	void copro_w(uint16_t data);

	void map(address_map &map);
};


class spetrix_state : public magic10_base_state
{
public:
	spetrix_state(const machine_config &mconfig, device_type type, const char *tag) :
		magic10_base_state(mconfig, type, tag),
		m_ticket(*this, "ticket")
	{ }

	void spetrix(machine_config &config);

	void init_spetrix();

protected:
	required_device<ticket_dispenser_device> m_ticket;

private:
	void out_w(uint16_t data);
	void spetrix_map(address_map &map);
};

/***************************
*      Video Hardware      *
***************************/

template <uint8_t Which>
void magic10_base_state::videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[Which][offset]);
	m_tilemap[Which]->mark_tile_dirty(offset >> 1);
}

TILE_GET_INFO_MEMBER(magic10_base_state::get_layer0_tile_info)
{
	tileinfo.set(1,
		m_videoram[0][tile_index * 2],
		m_videoram[0][tile_index * 2 + 1] & 0x0f,
		TILE_FLIPYX((m_videoram[0][tile_index * 2 + 1] & 0xc0) >> 6));
}

TILE_GET_INFO_MEMBER(magic10_base_state::get_layer1_tile_info)
{
	tileinfo.set(1,
		m_videoram[1][tile_index * 2],
		m_videoram[1][tile_index * 2 + 1] & 0x0f,
		TILE_FLIPYX((m_videoram[1][tile_index * 2 + 1] & 0xc0) >> 6));
}

TILE_GET_INFO_MEMBER(magic10_base_state::get_layer2_tile_info)
{
	tileinfo.set(0,
		m_videoram[2][tile_index * 2],
		m_videoram[2][tile_index * 2 + 1] & 0x0f,0);
}


void magic10_base_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(magic10_base_state::get_layer0_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(magic10_base_state::get_layer1_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(magic10_base_state::get_layer2_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);
}

uint32_t magic10_base_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// TODO: understand where this comes from.
	m_tilemap[2]->set_scrollx(0, m_layer2_offset[0]);
	m_tilemap[2]->set_scrolly(0, m_layer2_offset[1]);

	/*
	4 and 6 are y/x global register writes.
	0 and 2 are y/x writes for the scrolling layer.
	*/
	m_tilemap[1]->set_scrolly(0, (m_vregs[0 / 2] - m_vregs[4 / 2]) + 0);
	m_tilemap[1]->set_scrollx(0, (m_vregs[2 / 2] - m_vregs[6 / 2]) + 4);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


/***************************
*       R/W Handlers       *
***************************/

uint16_t magic102_state::r()
{
	m_ret ^= 0x20;
	return m_ret;
}

uint16_t hotslot_state::copro_r()
{
	return 0x80;
}

void hotslot_state::copro_w(uint16_t data)
{
	logerror("Writing to copro: %d \n", data);
}

void magic10_state::out_w(uint16_t data)
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
  ---- --x-  Ticket Motor Out.
  ---- -x--  Coin counter.
  -x-- ----  Hopper Motor Out.
*/

//  popmessage("lamps: %02X", data);

	m_lamps[0] = BIT(data, 0);      // Lamp 1 - HOLD 1
	m_lamps[1] = BIT(data, 1);      // Lamp 2 - HOLD 2
	m_lamps[2] = BIT(data, 2);      // Lamp 3 - HOLD 3
	m_lamps[3] = BIT(data, 3);      // Lamp 4 - HOLD 4
	m_lamps[4] = BIT(data, 4);      // Lamp 5 - HOLD 5
	m_lamps[5] = BIT(data, 5);      // Lamp 6 - START
	m_lamps[6] = BIT(data, 6);      // Lamp 7 - PLAY (BET/TAKE/CANCEL)
	m_lamps[7] = BIT(data, 8);      // Lamp 8 - PAYOUT/SUPERGAME

	m_ticket->motor_w(BIT(data, 9) );
	machine().bookkeeping().coin_counter_w(0, BIT(data, 10));
	m_hopper->motor_w(BIT(data, 14) );
}

void spetrix_state::out_w(uint16_t data)
{
/*
  ----------------------------------------------
  --- Super Petrix outputs
  ----------------------------------------------

  0x0200 - Ticket Motor.
  0x0400 - Coin counter.

*/

	m_ticket->motor_w(BIT(data, 9));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 10));
}

/***************************
*       Memory Maps        *
***************************/

void magic10_state::magic10_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x100fff).ram().w(FUNC(magic10_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x101000, 0x101fff).ram().w(FUNC(magic10_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x102000, 0x103fff).ram().w(FUNC(magic10_state::videoram_w<2>)).share(m_videoram[2]);
	map(0x200000, 0x2007ff).ram().share("nvram");
	map(0x300000, 0x3001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x400001).portr("INPUTS");
	map(0x400002, 0x400003).portr("DSW");
	map(0x400008, 0x400009).w(FUNC(magic10_state::out_w));
	map(0x40000b, 0x40000b).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x40000e, 0x40000f).nopw();
	map(0x400080, 0x400087).ram().share(m_vregs);
	map(0x600000, 0x603fff).ram();
}

void magic10_state::magic10a_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x100fff).ram().w(FUNC(magic10_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x101000, 0x101fff).ram().w(FUNC(magic10_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x102000, 0x103fff).ram().w(FUNC(magic10_state::videoram_w<2>)).share(m_videoram[2]);
	map(0x200000, 0x2007ff).ram().share("nvram");
	map(0x300000, 0x3001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x500001).portr("INPUTS");
	map(0x500002, 0x500003).portr("DSW");
	map(0x500008, 0x500009).w(FUNC(magic10_state::out_w));
	map(0x50000b, 0x50000b).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x50000e, 0x50000f).nopw();
	map(0x500080, 0x500087).ram().share(m_vregs);   // video registers?
	map(0x600000, 0x603fff).ram();
}

void magic102_state::map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x100fff).ram().w(FUNC(magic102_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x101000, 0x101fff).ram().w(FUNC(magic102_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x102000, 0x103fff).ram().w(FUNC(magic102_state::videoram_w<2>)).share(m_videoram[2]);
	map(0x200000, 0x2007ff).ram().share("nvram");
	map(0x400000, 0x4001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x500001).r(FUNC(magic102_state::r));
	map(0x500004, 0x500005).nopr(); // gives credits
	map(0x500006, 0x500007).nopr(); // gives credits
	map(0x50001a, 0x50001b).portr("IN0");
	map(0x50001c, 0x50001d).portr("IN1");
//  map(0x500002, 0x50001f).nopr();
//  map(0x500002, 0x50001f).nopw();
	map(0x600000, 0x603fff).ram();
	map(0x700001, 0x700001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x700080, 0x700087).ram().share(m_vregs);   // video registers?
}

void hotslot_state::map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x100fff).ram().w(FUNC(hotslot_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x101000, 0x101fff).ram().w(FUNC(hotslot_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x102000, 0x103fff).ram().w(FUNC(hotslot_state::videoram_w<2>)).share(m_videoram[2]);
	map(0x200000, 0x2007ff).ram().share("nvram");
	map(0x400000, 0x4001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500004, 0x500005).rw(FUNC(hotslot_state::copro_r), FUNC(hotslot_state::copro_w)); // copro comm
	map(0x500006, 0x500011).ram();
	map(0x500012, 0x500013).portr("IN0");
	map(0x500014, 0x500015).portr("IN1");
	map(0x500016, 0x500017).portr("IN2");
	map(0x500018, 0x500019).portr("DSW1");
	map(0x50001a, 0x50001d).nopw();
	map(0x600000, 0x603fff).ram();
	map(0x70000b, 0x70000b).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x700080, 0x700087).ram().share(m_vregs);
}

void magic10_state::sgsafari_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x100fff).ram().w(FUNC(magic10_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x101000, 0x101fff).ram().w(FUNC(magic10_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x102000, 0x103fff).ram().w(FUNC(magic10_state::videoram_w<2>)).share(m_videoram[2]);
	map(0x200000, 0x203fff).ram().share("nvram");
	map(0x300000, 0x3001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500002, 0x500003).portr("DSW1");
	map(0x500008, 0x500009).w(FUNC(magic10_state::out_w));
	map(0x50000b, 0x50000b).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x50000e, 0x50000f).portr("IN0");
	map(0x500080, 0x500087).ram().share(m_vregs);   // video registers?
	map(0x600000, 0x603fff).ram();
}
/*

  SGSafari unimplemented writes:

  0x500000 - 0x500007 ; unknown.
  0x50000c - 0x50007f ; unknown.
  0x500080 - 0x500083 ; video registers (layer scroll)
  0x500084 - 0x500087 ; video registers (unknown)
  0x500088 - 0x5000ff ; unknown.

*/

void spetrix_state::spetrix_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x100fff).ram().w(FUNC(spetrix_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x101000, 0x101fff).ram().w(FUNC(spetrix_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x102000, 0x103fff).ram().w(FUNC(spetrix_state::videoram_w<2>)).share(m_videoram[2]);
	map(0x200000, 0x203fff).ram().share("nvram");
	map(0x300000, 0x3001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500002, 0x500003).portr("DSW1");
	map(0x500008, 0x500009).w(FUNC(spetrix_state::out_w));
	map(0x50000a, 0x50000a).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x50000e, 0x50000f).portr("IN0");
	map(0x500080, 0x500087).ram().share(m_vregs);   // video registers?
	map(0x600000, 0x603fff).ram();
}

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
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME("Note D") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Display Logo" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Yes ) )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_DIPNAME( 0x0008, 0x0008, "Clear NVRAM" ) // Needs to enabled by other DSW
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ))
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM )        PORT_NAME("Hopper Refill") PORT_CODE(KEYCODE_F) // empty dispenser
	PORT_DIPNAME( 0x00e4, 0x00e4, "Disable Free Play" )
	PORT_DIPSETTING(      0x00e4, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0100, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Hardest ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Medium ) )    // Clr.NVRAM Enable
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x0c00, 0x0000, "Notes Settings" )
	PORT_DIPSETTING(      0x0000, "Note A: 10 - Note B: 20 - Note C: 50 - Note D: 100" )
	PORT_DIPSETTING(      0x0800, "Note A: 20 - Note B: 40 - Note C: 100 - Note D: 200" )
	PORT_DIPSETTING(      0x0400, "Note A: 50 - Note B: 100 - Note C: 500 - Note D: 1000" )    // Clr.NVRAM Enable
	PORT_DIPSETTING(      0x0c00, "Note A: 100 - Note B: 200 - Note C: 1000 - Note D: 2000" )
	PORT_DIPNAME( 0x3000, 0x3000, "Lots At" )           PORT_CONDITION("DSW", 0xc000, EQUALS, 0xc000)
	PORT_DIPSETTING(      0x0000, "50 200 500 1000 2000" )
	PORT_DIPSETTING(      0x1000, "100 300 1000 3000 5000" )
	PORT_DIPSETTING(      0x2000, "200 500 2000 3000 5000" )
	PORT_DIPSETTING(      0x3000, "500 1000 2000 4000 8000" )
	PORT_DIPNAME( 0x3000, 0x3000, "1 Ticket Won" )      PORT_CONDITION("DSW", 0xc000, EQUALS, 0x8000)
	PORT_DIPSETTING(      0x0000, "Every 100 Score" )
	PORT_DIPSETTING(      0x1000, "Every 100 Score" )
	PORT_DIPSETTING(      0x2000, "Every 100 Score" )
	PORT_DIPSETTING(      0x3000, "Every 100 Score" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unused ) )   PORT_CONDITION("DSW", 0xc000, EQUALS, 0x4000)
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )    // Clr.NVRAM Enable
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unused ) )   PORT_CONDITION("DSW", 0xc000, EQUALS, 0x4000)
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )    // Clr.NVRAM Enable
	PORT_DIPNAME( 0x3000, 0x3000, "1 Play Won" )        PORT_CONDITION("DSW", 0xc000, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, "Every 10 Score" )
	PORT_DIPSETTING(      0x1000, "Every 10 Score" )
	PORT_DIPSETTING(      0x2000, "Every 10 Score" )
	PORT_DIPSETTING(      0x3000, "Every 10 Score" )
	PORT_DIPNAME( 0xc000, 0xc000, "Dispenser Type" )
	PORT_DIPSETTING(      0x0000, "MKII Hopper - Supergame" )
	PORT_DIPSETTING(      0x4000, "10 Tokens" )    // Clr.NVRAM Enable
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
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR )   PORT_NAME("Door") PORT_TOGGLE
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Aux A")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Aux B")
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
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
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note A")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Note B")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Note C")
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
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
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME("Note D") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Payout / Super Game")

	PORT_START("DSW1")
	// TODO: defaults are hardwired with aforementioned startup code, is it intentional?
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
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

static INPUT_PORTS_START( spetrix )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )   PORT_NAME("Right - Hold 1")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )   PORT_NAME("Left - Hold 2 / Low")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )   PORT_NAME("Up - Hold 3")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )   PORT_NAME("Down - Hold 4 / High")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )   PORT_NAME("Rotate Left - Hold 5")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )        PORT_NAME("Select - Start / Collect")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_BET )    PORT_NAME("Rotate Right - Bet / Collect")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START2 )        PORT_NAME("Petrix Start")
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_CUSTOM )        PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect Points")

	PORT_START("DSW1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_SERVICE1 )     PORT_NAME("Switch to Poker Mode")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_DIPNAME( 0x0100,   0x0100, "DIP1" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(        0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200,   0x0200, "DIP2" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(        0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400,   0x0000, "DIP3" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(        0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800,   0x0000, "DIP4" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(        0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000,   0x1000, "DIP5" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(        0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000,   0x2000, "DIP6" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(        0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000,   0x0000, "DIP7" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(        0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000,   0x0000, "DIP8" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(        0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/****************************
*     Graphics Layouts      *
****************************/

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

static const gfx_layout tiles8x8_layout_soccer10 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{
		6*4, 7*4, 4*4, 5*4, 2*4, 3*4, 0*4, 1*4
	},
	{ STEP8(0,32) },
	8*8*4
};

static const gfx_layout tiles16x16_layout_soccer10 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{
		6*4, 7*4, 4*4, 5*4, 2*4, 3*4, 0*4, 1*4,
		4*8*16+6*4, 4*8*16+7*4, 4*8*16+4*4, 4*8*16+5*4, 4*8*16+2*4, 4*8*16+3*4, 4*8*16+0*4, 4*8*16+1*4
	},
	{ STEP16(0,4*8) },
	16*16*4
};


/****************************
*      Graphics Decode      *
****************************/

static GFXDECODE_START( gfx_magic10 )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_planar,  0, 16 )
	GFXDECODE_ENTRY( "tiles", 0, tiles16x16_layout, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_soccer10 )
	GFXDECODE_ENTRY( "tiles", 0, tiles8x8_layout_soccer10,   0, 16 )
	GFXDECODE_ENTRY( "tiles", 0, tiles16x16_layout_soccer10, 0, 16 )
GFXDECODE_END


/****************************
*      Machine Drivers      *
****************************/

void magic10_base_state::base(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(20'000'000) / 2);  // 10 MHz.
	m_maincpu->set_vblank_int("screen", FUNC(magic10_base_state::irq1_line_hold));

	// 1FILL is required by vanilla magic10 at least (otherwise gameplay won't work properly)
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 44*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(magic10_base_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 256);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_magic10);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);   // clock frequency & pin 7 not verified
}


void magic10_state::magic10(machine_config &config)
{
	base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &magic10_state::magic10_map);

	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(6), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH );
	HOPPER(config, m_hopper, attotime::from_msec(20), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH );
}


void magic10_state::magic10a(machine_config &config)
{
	magic10(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &magic10_state::magic10a_map);
}

void magic10_state::soccer10(machine_config &config)
{
	magic10a(config);

	m_maincpu->set_vblank_int("screen", FUNC(magic10_base_state::irq4_line_hold));

	m_gfxdecode->set_info(gfx_soccer10);
}

void magic102_state::magic102(machine_config &config)
{
	base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &magic102_state::map);

	subdevice<screen_device>("screen")->set_visarea(0*8, 48*8-1, 0*8, 30*8-1);
}


void hotslot_state::hotslot(machine_config &config)
{
	magic10(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &hotslot_state::map);

	subdevice<screen_device>("screen")->set_visarea(8*8, 56*8-1, 2*8, 32*8-1);
}


void magic10_state::sgsafari(machine_config &config)
{
	magic10(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &magic10_state::sgsafari_map);
	m_maincpu->set_vblank_int("screen", FUNC(magic10_state::irq2_line_hold));    // L1 interrupts

	subdevice<screen_device>("screen")->set_visarea(0*8, 44*8-1, 0*8, 30*8-1);
}


void spetrix_state::spetrix(machine_config &config)
{
	base(config);

	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(6), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH );

	m_maincpu->set_addrmap(AS_PROGRAM, &spetrix_state::spetrix_map);
	m_maincpu->set_vblank_int("screen", FUNC(spetrix_state::irq2_line_hold));    // L1 interrupts

	subdevice<screen_device>("screen")->set_visarea(0*8, 45*8-1, 0*8, 30*8-1);
}


/****************************
*        Rom Loads          *
****************************/

/*
  Magic's 10 (ver. 16.55)

  CPUs:
  1x MC68000P12 (u1) 16/32-bit Microprocessor (main).
  1x OKI M6295 (u21) 4-Channel Mixing ADCPM Voice Synthesis LSI (sound).
  1x KA358           Dual Operational Amplifier (sound).
  1x TDA2003   (u24) Audio Amplifier (sound).

  1x 20.000MHz. oscillator (OSC1, close to main CPU).
  1x 30.000MHz. oscillator (OSC2, close to sound).
  1x blu resonator 1000J (XTAL1, close to sound).

  ROMs:
  6x M27C1001 (2,3,14,15,16,17).
  1x AM27C020 (1).

  RAMs:
  2x KM6865BP-20 (u4, u59).
  2x HY6264ALP-70 (u34, u35).
  2x HM3-65728BK-5 (u50, u51).
  1x Dallas DS1220Y-200 Nonvolatile RAM.

  PLDs:
  2x TPC1020BFN-084C1 (u41, u60) (read protected).
  1x AMPAL16R4PC (u42) (dumped).

  Others:
  1x 28x2 JAMMA edge connector
  1x 12 legs connector (J1)
  1x trimmer (volume)(P1)
  1x 8 DIP switches bank (DIP1)

  Notes:
  PCB is marked: "040" and "lc" on component side ("LC" is the Italian for "Lato Componenti" which translates to "Components Side")
  PCB is labelled: "PASSED BY:_R_ DATE:_29.03.96_" on component side

*/
ROM_START( magic10 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.16.55s.u3", 0x000000, 0x20000, CRC(191a46f4) SHA1(65bc22cdcc4b2f102d3eef595626819af709cacb) )
	ROM_LOAD16_BYTE( "3.16.55s.u2", 0x000001, 0x20000, CRC(a03a80bc) SHA1(a21da8912f1d2c8c2fa4a8d3ce4d43da8a934e21) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "16.u25", 0x00000, 0x20000, CRC(7abb8136) SHA1(1d4daf6a4477853d89d08afb524516ef79f60dd6) )
	ROM_LOAD( "14.u26", 0x20000, 0x20000, CRC(fd0b912d) SHA1(1cd15fa3459e7fece9fc37595f2b6848c00ffa43) )
	ROM_LOAD( "15.u27", 0x40000, 0x20000, CRC(8178c907) SHA1(8c3440769ed4e113d84d1f8f9079783497791859) )
	ROM_LOAD( "17.u28", 0x60000, 0x20000, CRC(dfd41aab) SHA1(82248c7fa4febb1c453f35a0e4cfae062c5da2d5) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "1.u22", 0x00000, 0x40000, CRC(98885246) SHA1(752d549e6248074f2a7f6c5cc4d0bbc44c7fa4c3) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16r4.u42", 0x0000, 0x0104, CRC(6d70f3f2) SHA1(44c2be5945c052e057d4e0b03369acb7b9ff5d37) )
ROM_END

ROM_START( magic10a )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "u3.bin", 0x000000, 0x20000, CRC(3ef8736c) SHA1(0c36c516349cf2c843c4beb64c979b73da56183d) ) // sldh
	ROM_LOAD16_BYTE( "u2.bin", 0x000001, 0x20000, CRC(c30507fc) SHA1(ca15e30e9078dae2177df1ec33c94b37317ced61) ) // sldh

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "u25.bin", 0x00000, 0x20000, CRC(7abb8136) SHA1(1d4daf6a4477853d89d08afb524516ef79f60dd6) )
	ROM_LOAD( "u26.bin", 0x20000, 0x20000, CRC(fd0b912d) SHA1(1cd15fa3459e7fece9fc37595f2b6848c00ffa43) )
	ROM_LOAD( "u27.bin", 0x40000, 0x20000, CRC(8178c907) SHA1(8c3440769ed4e113d84d1f8f9079783497791859) )
	ROM_LOAD( "u28.bin", 0x60000, 0x20000, CRC(dfd41aab) SHA1(82248c7fa4febb1c453f35a0e4cfae062c5da2d5) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "u22.bin", 0x00000, 0x40000, CRC(98885246) SHA1(752d549e6248074f2a7f6c5cc4d0bbc44c7fa4c3) )
ROM_END

/*
  Magic's 10 (ver. 16.45)

  CPU:
  1x MC68000P12 (u1) 16/32-bit Microprocessor (main).
  1x OKI M6295  (u21) 4-Channel Mixing ADCPM Voice Synthesis LSI (sound).
  1x KA358            Dual Operational Amplifier (sound).
  1x TDA2003    (u24) Audio Amplifier (sound).

  1x 20.000MHz. oscillator (OSC1, close to main CPU).
  1x 30.000MHz. oscillator (OSC2, close to sound).
  1x blu resonator 1000J (XTAL1, close to sound).

  ROMs:
  6x M27C1001 (u2,u3,u25,u26,u27,u28).
  1x AM27C020 (1).

  RAMs:
  2x KM6865BP-20 (u4, u59).
  2x HY6264ALP-70 (u34, u35).
  2x HM3-65728BK-5 (u50, u51).
  1x DS1220Y-200

  PLDs:
  2x TPC1020BFN-084C1 (u41, u60) (read protected).
  1x PAL ? to be confirmed

  Others:
  1x 28x2 JAMMA edge connector
  1x 12 legs connector (J1)
  1x trimmer (volume)(P1)
  1x 8 DIP switches bank (DIP1)

*/
ROM_START( magic10b )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "u3_1645.bin",  0x00000, 0x20000, CRC(7f2549e4) SHA1(6578ad29273c357faae7c6be3fa1b49087e088a2) )
	ROM_LOAD16_BYTE( "u2_1645.bin",  0x00001, 0x20000, CRC(c075234e) SHA1(d9bc38f0b984082a77088fbb52b02c8f5c49846c) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "u25.bin", 0x00000, 0x20000, CRC(7abb8136) SHA1(1d4daf6a4477853d89d08afb524516ef79f60dd6) )
	ROM_LOAD( "u26.bin", 0x20000, 0x20000, CRC(fd0b912d) SHA1(1cd15fa3459e7fece9fc37595f2b6848c00ffa43) )
	ROM_LOAD( "u27.bin", 0x40000, 0x20000, CRC(8178c907) SHA1(8c3440769ed4e113d84d1f8f9079783497791859) )
	ROM_LOAD( "u28.bin", 0x60000, 0x20000, CRC(dfd41aab) SHA1(82248c7fa4febb1c453f35a0e4cfae062c5da2d5) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "u22.bin", 0x00000, 0x40000, CRC(98885246) SHA1(752d549e6248074f2a7f6c5cc4d0bbc44c7fa4c3) )
ROM_END

/*
  Magic's 10 (ver. 16.15)

  CPU:
  1x TS68000P12 (main)(u1)
  1x OKI M6295 (u21)
  1x LM358N
  1x TDA2003 (u24)

  1x oscillator 20.000000MHz (close to main)(osc1)
  1x oscillator 30.000MHz (close to sound)(osc2)
  1x orange resonator 1000J (close to sound)(xtal1)

  ROMs:
  1x M27C2001 (1)
  5x M27C1001 (2,3,5,6,7)
  1x TMS27C010A (4)

  RAMs:
  2x W2465-70LL (u4,u59)
  2x HY6264ALP-10 (u34,u35)
  2x HM3-65728BK-5 (u50,u51)
  1x UM6116-3L (u5)

  PLDs:
  2x TPC1020AFN-084C (PLD)(not dumped)(u41,u60)
  1x PALCE16V8H (read protected)

  Others:
  1x 28x2 JAMMA edge connector
  1x 12 legs connector (J1)
  1x trimmer (volume)(P1)
  1x 8 DIP switches bank (DIP1)
  1x battery 3V

  Notes:
  PCB is marked: "039" and "lc" on component side ("LC" is the Italian for "Lato Componenti" which translates to "Components Side")
  PCB is labelled: "PASSED BY:_&_ DATE:_26.12.95_" on component side

*/
ROM_START( magic10c )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.u3", 0x000000, 0x20000, CRC(32c12ad6) SHA1(93340df2c0f4c260837bd6649008e26a17a22015) )
	ROM_LOAD16_BYTE( "3.u2", 0x000001, 0x20000, CRC(a9945aaa) SHA1(97d4f6441b96618f2e3ce14095ffc5628cb14f0e) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "6.u25", 0x00000, 0x20000, CRC(7abb8136) SHA1(1d4daf6a4477853d89d08afb524516ef79f60dd6) )
	ROM_LOAD( "4.u26", 0x20000, 0x20000, CRC(fd0b912d) SHA1(1cd15fa3459e7fece9fc37595f2b6848c00ffa43) )
	ROM_LOAD( "5.u27", 0x40000, 0x20000, CRC(8178c907) SHA1(8c3440769ed4e113d84d1f8f9079783497791859) )
	ROM_LOAD( "7.u28", 0x60000, 0x20000, CRC(dfd41aab) SHA1(82248c7fa4febb1c453f35a0e4cfae062c5da2d5) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "1.u22", 0x00000, 0x40000, CRC(98885246) SHA1(752d549e6248074f2a7f6c5cc4d0bbc44c7fa4c3) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16v8h.u42", 0x0000, 0x0117, NO_DUMP )
ROM_END

/*
  Magic's 10 2 (ver 1.1)

  CPUs:
  1x 68000 (main)
  1x HD6473308CP10 (MCU)
  1x OKI6295 (sound)
  1x oscillator 30.000MHz
  1x oscillator 20.000MHz

  ROMs:
  6x 27010 1-6
  1x 27020 7

  PLDs:
  1x FPGA by Actel (read protected)

  Others:
  1x dipswitch
  1x battery

  Notes:
  PCB marked: ABM - 9605 Rev.02

*/
ROM_START( magic102 )
	ROM_REGION( 0x40000, "maincpu", 0 )     // 68000 code
	ROM_LOAD16_BYTE( "2.u3",  0x00000, 0x20000, CRC(6fc55fe4) SHA1(392ad92e55aeac9bf5235cceb6b0b415942105a4) )
	ROM_LOAD16_BYTE( "1.u2",  0x00001, 0x20000, CRC(501507af) SHA1(ceed50c9380a9838cd3d171d2387334edfeff77f) )

	ROM_REGION( 0x10000, "mcu", 0 ) // h8/330 HD6473308cp10 with internal ROM
	ROM_LOAD( "mcu",        0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "3.u35",        0x00000, 0x20000, CRC(df47bb12) SHA1(b8bcbc9ab764d3159344d93776d13a14c9154086) )
	ROM_LOAD( "4.u36",        0x20000, 0x20000, CRC(dc242034) SHA1(6a2983c79776df07f29b77f23799fef6f20df24f) )
	ROM_LOAD( "5.u37",        0x40000, 0x20000, CRC(a048e26e) SHA1(788c28470298896902120e74fd8b9b283b8e9b79) )
	ROM_LOAD( "6.u38",        0x60000, 0x20000, CRC(469efb34) SHA1(b16646fb0c4757132e272b3877cf546b6f616786) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "7.u32",        0x00000, 0x40000, CRC(47804af7) SHA1(602dc0361869b52532e2adcb0de3cbdd042761b3) )
ROM_END

ROM_START( magic102a )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.b3.u3", 0x00000, 0x20000, CRC(a2bf7d5b) SHA1(72f5c73ab7c7c44ed81ecfb88bcc8f4c7146c619) )
	ROM_LOAD16_BYTE( "3.b3.u2", 0x00001, 0x20000, CRC(97b4defe) SHA1(ebd63b1e589708d10a3c04e4ecd73161cc3aa386) )

	ROM_REGION( 0x10000, "mcu", 0 ) // h8/330 HD6473308cp10 with internal ROM
	ROM_LOAD( "mcu", 0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x80000, "tiles", 0 )// tiles
	ROM_LOAD( "7.u35", 0x00000, 0x20000, CRC(82cef471) SHA1(c9649e295336863c713ddf5615f0e8cacdf218e7) )
	ROM_LOAD( "6.u36", 0x20000, 0x20000, CRC(dc064229) SHA1(511af9036dbd0f3c6f668d46e4971ee0bcaaf816) )
	ROM_LOAD( "5.u37", 0x40000, 0x20000, CRC(efe2f609) SHA1(e18be34f3b67669a8852b88bda6ba85227470cc1) )
	ROM_LOAD( "4.u38", 0x60000, 0x20000, CRC(b2febd06) SHA1(a57879c89d240ab4287e285d8a50afb3fe903376) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "1.2.u32", 0x00000, 0x40000, CRC(47804af7) SHA1(602dc0361869b52532e2adcb0de3cbdd042761b3) )

	ROM_REGION( 0x0400, "plds", 0 ) // PLDs
	ROM_LOAD( "gal22cv10.u22", 0x0000, 0x02dd, NO_DUMP )
	ROM_LOAD( "palce16v8h.u54",  0x02dd, 0x0117, NO_DUMP )
ROM_END

ROM_START( soccer10 ) // PCB marked I.G.T. International Games Trade s.r.l. (Italian, as s.r.l. is an acronym for 'Società a Responsabilità Limitata')
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "12.u16", 0x000000, 0x20000, CRC(5b1d8de2) SHA1(c20f4ca235dc78acb20277407833db8906027d4f) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "13.u15", 0x000001, 0x20000, CRC(92ed3808) SHA1(eb1a062190cbcc389561504a0d0685c91952dbd9) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "4.u24", 0x00000, 0x40000, CRC(06db9866) SHA1(97c18c50c5eb0bd3bc927d3ac22c3176498017fd) )
	ROM_LOAD16_BYTE( "5.u28", 0x00001, 0x40000, CRC(f41c196d) SHA1(046b7d4bb30740a43ea9ccfb7e5a4d1405456ef8) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "1.u44", 0x00000, 0x40000, CRC(98885246) SHA1(752d549e6248074f2a7f6c5cc4d0bbc44c7fa4c3) ) // same as magic10 and clones

	ROM_REGION( 0x0800, "ds1220", 0 ) // TODO: is this needed or is it just user data? Verify once working
	ROM_LOAD( "ds1220.u20", 0x0000, 0x0800, CRC(afc7cbc3) SHA1(74f51217a4f280c20742657ef80a9b4d5a891a7e) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "gal20v8a.u4", 0x0000, 0x0157, NO_DUMP )
ROM_END

/*
  Super Pool (ver. 1.2)

  CPUs:
  1x MC68HC000P10 (u1)
  1x HD6473308CP10 (u24)
  1x U6295 (u31)
  1x LM358N (u33)
  1x TDA2003 (u34)

  1x oscillator 20.000MHz
  1x oscillator 30.0000MHz
  1x blu resonator 1000J (close to sound)

  ROMs:
  1x M27C2001 (1) (Sound)
  2x TMS27C010A (2,3) (main)
  4x TMS27C010A (4,5,6,7) (gfx)

  RAMs:
  4x W2465-10L (u4,u5,u43,u44)
  1x 6116 (u6)
  2x HM3-65728BK-5 (u61,u62)

  PLDs:
  1x ACTEL A1020B-PL84C (u50)(read protected)
  1x PALCE22V10H (u22)(not dumped)
  1x PALCE16V8H (u54)(not dumped)

  Others:
  1x 28x2 JAMMA edge connector
  1x 12 legs connector (J1)
  1x trimmer (volume)(P1)
  1x 8 DIP switches bank (DIP1)
  1x battery 3V

  STATUS:

  Memory map = done.
  Inputs =     done.
  Machine =    done.

  OKI 6295 =     ok.
  Screen size =  ok.
  Fixed layers = yes.

*/
ROM_START( suprpool )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2-1.22a.u3", 0x00000, 0x20000,CRC(5d15037a) SHA1(74cab79a1b08910267262a4c6b501126a4df6cda) )
	ROM_LOAD16_BYTE( "3-1.22a.u2", 0x00001, 0x20000,CRC(c762cd1c) SHA1(ee05a9e8147d613eb14333e6e7b743fc05982e7c) )

	ROM_REGION( 0x10000, "mcu", 0 ) // h8/330 HD6473308cp10 with internal ROM
	ROM_LOAD( "mcu",        0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "7.u35", 0x00000, 0x20000,  CRC(357d145f) SHA1(9fea0d0c5d6c27bf520c4f81eb0f48a65ff60142) )
	ROM_LOAD( "6.u36", 0x20000, 0x20000,  CRC(c4448813) SHA1(6e168eb8503b852179f2d743f1cba935592e0a60) )
	ROM_LOAD( "5.u37", 0x40000, 0x20000,  CRC(6e99af07) SHA1(85e7a76724fd9ce8d07b5088cb6e0d933fd95692) )
	ROM_LOAD( "4.u38", 0x60000, 0x20000,  CRC(0660a169) SHA1(1cb34b3da4b144028519a3c5b32ef7da44af0624) )

	ROM_REGION( 0x080000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "1.u32", 0x00000, 0x40000, CRC(47804af7) SHA1(602dc0361869b52532e2adcb0de3cbdd042761b3) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "palce22v10h.u22", 0x0000, 0x02dd, NO_DUMP )
	ROM_LOAD( "palce16v8h.u54",  0x02dd, 0x0117, NO_DUMP )
ROM_END

/*
  Hot Slot

  CPU:
  1x MC68HC000FN12 (u1) 16/32-bit Microprocessor (main).
  1x HD6473308CP10 (u24) 16-bit Single-Chip MCU (NOT DUMPED).
  1x 6295          (u31) 4-Channel Mixing ADCPM Voice Synthesis LSI (sound).
  1x KA358         (u33) Dual Operational Amplifier (sound).
  1x TDA2003       (u34) Audio Amplifier (sound).

  1x 20.00000 MHz. oscillator (osc1).
  1x 30.000 MHz. oscillator (osc2).
  1x blu resonator 1000J (xtal1).

  ROMs (1st PCB):
  2x 27C010 (2,3).
  2x 27C020 (5,7).
  3x 27C2001 (1,4,6).

  ROMs (2nd PCB):
  1x AM27C010 (2).
  1x M27C1001 (3).
  2x 27C020   (1,7).
  1x M27C2001 (4).
  1x AM27C020 (5,6).

  RAMs
  1x HM6116-70 (u6).
  4x ZMDU6264ADC-07LLP (u4, u5, u43, u44).
  2x HM3-65728H-8 (u61, u62).

  PLDs
  1x A40MX04-PL84 (u50) (not dumped).
  1x GAL16V8D-25LP (u54), (read protected).
  1x PALCE22V10H-25PC/4 (u22), (read protected).

  Others:
  1x 28x2 JAMMA edge connector.
  1x 12 legs connector (J1).
  1x 12x2 pins jumper (J2, J3).
  1x 2 pins jumper (J4).
  1x trimmer (volume)(P1).
  1x trimmer (unknown)(P2).
  1x 8x2 DIP switches (DIP1).
  1x CR2032 3v. lithium battery.

  Notes:
  PCB is marked: "lc" on component side ("LC" is the Italian for "Lato Componenti" which translates to "Components Side")
  PCB is marked: "ls" on solder side ("LS" is the Italian for "Lato Saldature" which translates to "Solders Side")
  PCB is labeled: "Hot Slot Non rimuovere" on component side


  - The system RAM test need the bit 7 of offset 0x500005 activated to be successful.
    This offset seems to be a kind of port connected to the MCU.

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
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "hotslot2.u3", 0x00000, 0x20000, CRC(676cbe32) SHA1(78721326f3334fcdfdaffb72dbcacfb8bb591d51) )
	ROM_LOAD16_BYTE( "hotslot3.u2", 0x00001, 0x20000, CRC(2c362765) SHA1(c41741c97fe8e5b3a66eb08ebf68d24c6c771ba8) )

	ROM_REGION( 0x10000, "mcu", 0 ) // h8/330 HD6473308cp10 with internal ROM
	ROM_LOAD( "mcu",        0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "hotslot7.u35", 0x00000, 0x40000, CRC(715073c2) SHA1(39085871fee182a9b22c3e042211e76da0ee3024) )
	ROM_LOAD( "hotslot6.u36", 0x40000, 0x40000, CRC(8ef2e25a) SHA1(d4a3288878fabab7ea193d5dadde1fe9fea6bc8a) )
	ROM_LOAD( "hotslot5.u37", 0x80000, 0x40000, CRC(98375b25) SHA1(2167f3374bdfc5e1fef7b9ec4361bc68223876b8) )
	ROM_LOAD( "hotslot4.u38", 0xc0000, 0x40000, CRC(cc8a241a) SHA1(8c6ea51d5f7475be79775df0b976ffddc5a960ed) )

	ROM_REGION( 0x080000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "hotslot1.u32", 0x00000, 0x40000, CRC(ae880970) SHA1(3c302b3f6f6bbf72a522889592add3b6ef8ce1b0) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "palce22v10h.u22", 0x0000, 0x02dd, NO_DUMP )
	ROM_LOAD( "gal6v8d.u54",     0x02dd, 0x0117, NO_DUMP )
ROM_END

/*
  Magic Colors (ver. 1.7a)

  CPU:
  1x missing CPU MC68000 (QFP68 socket, u1)
  1x HD6473308CP10 (u24)(MCU)
  1x M6295 (u31)(sound)
  1x KA358 (u33)(sound)
  1x TDA2003 (u34)(sound)

  1x oscillator 20.0000MHz (OSC1)
  1x oscillator 30.0000MHz (OSC2)
  1x 1000J blu resonator (XTAL1)

  ROMs:
  6x 27C010 (2,3,4,5,6,7)
  1x 27C020 (1)

  RAMs:
  1x U6216ADC-08L (u6)
  4x LP6264D-70LL (u4,u5,u43,u44)
  2x HM3-65728H-5 (u61,u62)

  PLDs:
  1x A40MX04-PL84-9828 (u50)
  1x GAL16V8D (as PAL16R4)(read protected)
  1x missing PAL22V10

  Others:
  1x 28x2 JAMMA edge connector
  1x 12 legs connector (J1)
  1x trimmer (volume)(P1)
  1x 8 DIP switches bank (DIP1)
  1x battery 3V

  Notes:
  PCB is marked: "OC Rev. 03", "0/088066-L0" and "lc" on component side ("LC" is the Italian for "Lato Componenti" which translates to "Components Side")
  PCB is marked: "ls" on solder side ("LS" is the Italian for "Lato Saldature" which translates to "Solders Side")
  PCB is labeled: "Non rimuovere M.Colors 2.0 68000" and "Passed 01/04/99" on component side

  STATUS:

  Memory map = done.
  Inputs =     done.
  Machine =    done.

  OKI 6295 =     ok.
  Screen size =  ok.
  Fixed layers = yes.

*/
ROM_START( mcolors )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "m.colors1.7a-2.u3", 0x00000, 0x20000, CRC(02ce6aab) SHA1(349cb639024a818cb88e911788a0146f48d25333) )
	ROM_LOAD16_BYTE( "m.colors1.7a-3.u2", 0x00001, 0x20000, CRC(076b9680) SHA1(856d1cfaca886d78a36e129a7b41455362932e66) )

	ROM_REGION( 0x10000, "mcu", 0 ) // h8/330 HD6473308cp10 with internal ROM
	ROM_LOAD( "mcu",        0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "m.colors1.7-7.u35", 0x00000, 0x20000, CRC(ec44b289) SHA1(269c965112f0ba308bb5f02d965e32df70310b2c) )
	ROM_LOAD( "m.colors1.7-6.u36", 0x20000, 0x20000, CRC(44e550e2) SHA1(abfc05b386efb0f9ad7479ff53079e6ecbaec137) )
	ROM_LOAD( "m.colors1.7-5.u37", 0x40000, 0x20000, CRC(ec363d0d) SHA1(283f0bf3e3d76d64389f0abdffbeaa3d538b8991) )
	ROM_LOAD( "m.colors1.7-4.u38", 0x60000, 0x20000, CRC(7845667d) SHA1(66b1409b8b661b95e2658385da9c2662430d8030) )

	ROM_REGION( 0x080000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "m.color1.u32", 0x00000, 0x40000, CRC(db8d6769) SHA1(2ab7730fd8ae9522e5452fe1f535002e11db5e7b) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "palce22v10h.u22", 0x0000, 0x02dd, NO_DUMP )
	ROM_LOAD( "gal6v8d.u54",     0x02dd, 0x0117, NO_DUMP )
ROM_END

/*
  Magic Colors (ver. 1.6)

  CPU:
  1x MC68000P12 (u1)(main)
  1x HD6473308CP10 (u24)(MCU)
  1x M6295 (u31)(sound)
  1x LM358N (u33)(sound)
  1x TDA2003 (u34)(sound)

  1x oscillator 20.0000MHz (OSC1)
  1x oscillator 30.0000MHz (OSC2)
  1x 1000J blu resonator (XTAL1)

  ROMs:
  6x AM27C010 (2,3,4,5,6,7)
  1x M27C2001 (1)

  RAMs:
  1x LH6116-10 (u6)
  4x UM6264D-70LL (u4,u5,u43,u44)
  2x HM3-65728H-5 (u61,u62)

  PLDs:
  1x ACTEL A1020B-PL84C (u50)
  1x AMPAL22V10APC (u22)
  1x PALCE16V8H-25PC/4 (u54)

  Others:
  1x 28x2 JAMMA edge connector
  1x 12 legs connector (J1)
  1x trimmer (volume)(P1)
  1x 8 DIP switches bank (DIP1)
  1x battery 3V

  Notes:
  PCB is marked: "OC ABM - 9743 Rev.01" and "lc" on component side ("LC" is the Italian for "Lato Componenti" which translates to "Components Side")
  PCB is marked: "ls" on solder side ("LS" is the Italian for "Lato Saldature" which translates to "Solders Side")
  PCB is labelled: "Non rimuovere 2a" and "Passed 02/03/99" on component side
  PCB is labelled: "1.99" on solder side

*/
ROM_START( mcolorsa )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "m.c.1.6-2.u3", 0x00000, 0x20000, CRC(43062c18) SHA1(5ac23eb392192131cf6745afddba6b5b32d75c9e) )
	ROM_LOAD16_BYTE( "m.c.1.6-3.u2", 0x00001, 0x20000, CRC(a24daff4) SHA1(a8f30712543c5d3b6024bcdbfa1359585495ba4a) )

	ROM_REGION( 0x10000, "mcu", 0 ) // h8/330 HD6473308cp10 with internal ROM
	ROM_LOAD( "mcu",        0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "m.c.1.6-7.u35", 0x00000, 0x20000, CRC(ac0e0520) SHA1(84f0f28260a2234db379c8c745a50d1ea3d0b695) )
	ROM_LOAD( "m.c.1.6-6.u36", 0x20000, 0x20000, CRC(fab02757) SHA1(cc67325ff512b05b910a648bd4d9143b71081675) )
	ROM_LOAD( "m.c.1.6-5.u37", 0x40000, 0x20000, CRC(41a2c761) SHA1(27d05a3132c96a9529f34fc1313f4652f7c2ce99) )
	ROM_LOAD( "m.c.1.6-4.u38", 0x60000, 0x20000, CRC(7fb74c19) SHA1(20e6ecb7f34d9b82d1073c5018a7c1c6cdf3740f) )

	ROM_REGION( 0x080000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "m.c.-1.u32", 0x00000, 0x40000, CRC(db8d6769) SHA1(2ab7730fd8ae9522e5452fe1f535002e11db5e7b) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ampal22v10apc.u22", 0x0000, 0x02dd, NO_DUMP )
	ROM_LOAD( "palce16v8h.u54",    0x02dd, 0x0117, NO_DUMP )
ROM_END

/*
  Super Gran Safari (ver 3.11)

  CPU:
  1x MC68000P12
  1x M6295
  1x TDA2002
  1x GL324
  1x oscillator 30.000MHz

  ROMs:
  2x M27C512 (1,2)
  1x M27C2001 (3)
  4x M27C1001 (4,5,6,7)

  RAMs:
  6x GM76C88AL-15
  1x M48Z02 (u37 dumped)

  PLDs:
  2x A1020B-PL84C read protected

  Others:
  1x 28x2 JAMMA edge connector (J1)
  1x 12 legs connector (J2)
  1x 4 legs connector (J3)
  1x 2 legs connector (J4)
  1x trimmer (volume)
  1x 8 DIP switches bank

  Notes:
  PCB is marked: "COMP01" and "ALL.01A" on component side.
  PCB is labelled: "GRAN SAFARI ORIGINALE NEW IMPEUROPEX CORP. COPYRIGHT 1996 No. 9603125" and "SUPER GRAN SAFARI SPC46" on component side.
  The game was developed by Nova Desitec.

  STATUS:

  Memory map = done.
  Inputs =     done.
  Machine =    done.

  OKI 6295 =     ok.
  Screen size =  ok.
  Fixed layers = yes.

*/
ROM_START( sgsafari )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.u7", 0x00000, 0x10000, CRC(797ceeac) SHA1(19055b6700f8523785790992adfeb67faa2358e0) )
	ROM_LOAD16_BYTE( "1.u2", 0x00001, 0x10000, CRC(549872f5) SHA1(2228c51541e3b059d5b16f50387e4215b82f78f6) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "4.u15", 0x00000, 0x20000, CRC(f9233481) SHA1(1d1aca9a61f0285a6f6f12f6169d9cfc2c5e6991) )
	ROM_LOAD( "5.u18", 0x20000, 0x20000, CRC(9561aa47) SHA1(140e0d9104c677de911d4d12ff617d84449d907b) )
	ROM_LOAD( "6.u16", 0x40000, 0x20000, CRC(91c22541) SHA1(e419a2d5e71b6c64992a08fa9bd82718350ca7da) )
	ROM_LOAD( "7.u19", 0x60000, 0x20000, CRC(3e3a5fbd) SHA1(c3511b488ecb4759a5fdea478007a4a1c2b5f9e0) )

	ROM_REGION( 0x040000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "3.u39", 0x00000, 0x40000, CRC(43257bb5) SHA1(993fbeb6ee0a8a4da185303ec24eee8424b90cd0) )
ROM_END

/*
    Super Petrix - Tetris type game.
    Unknown manufacturer.
*/
ROM_START( spetrix ) // same PCB as sgsafari but with a M48Z08 instead of a M48Z02
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "u7", 0x00000, 0x10000, CRC(d1bd325e) SHA1(0b4836356304145761dc894e308fa467c1ca9882) )
	ROM_LOAD16_BYTE( "u2", 0x00001, 0x10000, CRC(e84cf453) SHA1(5ab3aff3e91de486c5791c2917443ee4b94706d6) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "u15", 0x00000, 0x20000, CRC(3becd06b) SHA1(6ea84577ec363062692bb7ad79698bbe93603abb) )
	ROM_LOAD( "u18", 0x20000, 0x20000, CRC(2cb619f0) SHA1(f58ec59ea50ea6a12c3be08c635876058c1cdf22) )
	ROM_LOAD( "u16", 0x40000, 0x20000, CRC(39d33d9b) SHA1(b8c13f7c18f3f064fad05cac07409fd6753df911) )
	ROM_LOAD( "u19", 0x60000, 0x20000, CRC(0f84fad3) SHA1(d74553498f12cb77d9bef525524a7c75c3427e50) )

	ROM_REGION( 0x040000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "u39", 0x00000, 0x40000, CRC(eaaedf14) SHA1(b1a9dcbf3ee0542d61b1691d5f70375a44c06e21) )
ROM_END

/*
  Music Sort (Ver. 2.02).
  Same PCB than Magic's 10 (ver. 16.15)

  CPU:
  1x TS68000P12 (main)(u1)
  1x OKI M6295 (u21)
  1x LM358N
  1x TDA2003 (u24)

  1x oscillator 20.000000MHz (close to main)(osc1)
  1x oscillator 30.000MHz (close to sound)(osc2)
  1x orange resonator 1000J (close to sound)(xtal1)

  ROMs:
  1x M27C2001 (1)
  5x M27C1001 (2,3,5,6,7)
  1x TMS27C010A (4)

  RAMs:
  2x W2465-70LL (u4,u59)
  2x HY6264ALP-10 (u34,u35)
  2x HM3-65728BK-5 (u50,u51)
  1x NVRAM (u5)

  PLDs:
  2x TPC1020AFN-084C (PLD)(not dumped)(u41,u60)
  1x PALCE16V8H (read protected)

  Others:
  1x 28x2 JAMMA edge connector
  1x 12 legs connector (J1)
  1x trimmer (volume)(P1)
  1x 8 DIP switches bank (DIP1)

*/
ROM_START( musicsrt )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.u3", 0x000000, 0x20000, CRC(6a5cd39f) SHA1(c7ec0d9a640ff876bd9362bfe896ebc09795b418) )
	ROM_LOAD16_BYTE( "3.u2", 0x000001, 0x20000, CRC(7af68760) SHA1(08d333037a70cda60df9b0c288e9f6eb6fa7eb84) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "6.u25", 0x00000, 0x20000, CRC(9bcf89a6) SHA1(5b16ef9482249585a714cf2d3efffddd3f0e5834) )
	ROM_LOAD( "4.u26", 0x20000, 0x20000, CRC(b9397659) SHA1(f809f612fd6a7ecfdb0fa55260ef7a57f00c0733) )
	ROM_LOAD( "5.u27", 0x40000, 0x20000, CRC(36d7aeb3) SHA1(2c0863f2f366008640e8a19587460a30fda4ad6e) )
	ROM_LOAD( "7.u28", 0x60000, 0x20000, CRC(a03e750b) SHA1(046e3eb5671bed09d9e5fd3572a8d41ac9e8b69e) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "1.u22", 0x00000, 0x40000, CRC(98885246) SHA1(752d549e6248074f2a7f6c5cc4d0bbc44c7fa4c3) )

	ROM_REGION( 0x0800, "nvram", 0 ) // default Non Volatile RAM
	ROM_LOAD( "musicsrt_nv.u5", 0x0000, 0x0800, CRC(f4e063cf) SHA1(a60bbd960bb7dcf023417e8c7164303b6ce71014) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "palce6v8h.u42",     0x0000, 0x0117, NO_DUMP )
ROM_END

/*
  Luna Park (ver. 1.2)

  CPU:
  1x  MC68HC000FN10 (u1)    16/32-bit Microprocessor.
  1x  HD6473308CP10 (u24)   label: version 1.2 - 16-bit Single-Chip Microcomputer. NOT DUMPED.
  1x  M6295         (u31)   4-Channel Mixing ADCPM Voice Synthesis LSI.
  1x  LM358N        (u33)   Dual Operational Amplifier.
  1x  TDA2003       (u34)   Audio Amplifier.

  1x 20.000000MHz oscillator (osc1).
  1x 30.000MHz oscillator (osc2).
  1x blu resonator 1000J (xtal1).

  ROMs:
  6x AM27C010 ROMs(2-7).
  1x AM27C020 ROM (1).

  RAMs:
  1x LH5116-10 RAM (u6).
  4x HY6264ALP-10 RAM (u4, u5, u43, u44).
  2x HM3-65728H-8 RAM (u61, u62).

  PLDs:
  1x TPC1020AFN-084C (u50), read protected.
  1x PALCE16V8H-25PC/4 (u54), read protected.
  1x PALCE22V10H-25PC/4 (u22), read protected.

  Others:
  1x 28x2 JAMMA edge connector.
  1x 12-pins male connector (JP1).
  1x trimmer (volume)(P1).
  1x 8x2 DIP switches (DIP1).
  1x Renata 3V. CR2032 lithium battery.

  Notes:
  PCB is marked: "OC ABM - 9743 Rev.02" and "lc" on component side ("LC" is the Italian for "Lato Componenti" which translates to "Components Side")
  PCB is marked: "44-98 ls" on solder side ("LS" is the Italian for "Lato Saldature" which translates to "Solders Side")
  PCB is labeled: "CAUTION! DO NOT REMOVE THIS CHIP - ATTENZIONE! NON RIMUOVERE GAME:____ VERSION:_1.2_" and "25/11/98" on component side


  STATUS:

  Memory map = done.
  Inputs =     done.
  Machine =    done.

  OKI 6295 =     ok.
  Screen size =  ok.
  Fixed layers = yes.

*/
ROM_START( lunaprk )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2_2.00a.u3", 0x00000, 0x20000, CRC(5ec3d238) SHA1(a9e257275cd81b74309d20bc64b10f788ca1b22a) )
	ROM_LOAD16_BYTE( "3_2.00a.u2", 0x00001, 0x20000, CRC(6fceb57b) SHA1(f9cf566c60f9c1c604dbfeb9c3ad4831bb3922d4) )

	ROM_REGION( 0x10000, "mcu", 0 ) // h8/330 HD6473308cp10 with internal ROM
	ROM_LOAD( "mcu",        0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "7_l.p..u35", 0x00000, 0x20000, CRC(dfd6795d) SHA1(01929c31b5cc9468674830d9f687b2d3607d8052) )
	ROM_LOAD( "6_l.p..u36", 0x20000, 0x20000, CRC(fe323a28) SHA1(1cfba6c8359efed48506e8ae231926fb77469aaa) )
	ROM_LOAD( "5_l.p..u37", 0x40000, 0x20000, CRC(445b6564) SHA1(3568bcbcbdafa8503b50de960c370c85f2fbf62a) )
	ROM_LOAD( "4_l.p..u38", 0x60000, 0x20000, CRC(81567520) SHA1(4a1990ee19b2346824bb5b9f2880db12a414fdf7) )

	ROM_REGION( 0x080000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "1.u32", 0x00000, 0x40000, CRC(47804af7) SHA1(602dc0361869b52532e2adcb0de3cbdd042761b3) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "palce22v10h.u22", 0x0000, 0x02dd, NO_DUMP )
	ROM_LOAD( "palce16v8h.u54",  0x02dd, 0x0117, NO_DUMP )
ROM_END

/*
  Alta Tensione (ver. 2.01a)

  CPU:
  1x  MC68HC000FN12 (u1)    16/32-bit Microprocessor.
  1x  HD6473308CP10 (u24)   16-bit Single-Chip Microcomputer. NOT DUMPED.
  1x  M6295         (u31)   4-Channel Mixing ADCPM Voice Synthesis LSI.
  1x  KA358         (u33)   Dual Operational Amplifier.
  1x  TDA2003       (u34)   Audio Amplifier.

  1x 20.000000MHz oscillator (osc1).
  1x 30.000MHz oscillator (osc2).
  1x blu resonator 1000J (xtal1).

  ROMs:
  5x MX27C1000APC-12 ROMs(2-6).
  1x M271001 ROM (7).
  1x MX27C2000DC-90 ROM (1).

  RAMs:
  1x ZMDU6216ADC-08L RAM (u6).
  4x LP6264D-70LL RAM (u4, u5, u43, u44).
  2x HM3-65728H-5 RAM (u61, u62).

  PLDs:
  1x A40MX04-PL84 (u50), read protected.
  1x PALCE16V8H-25PC/4 (u54), read protected.
  1x PALC22V10H-25PC/4 (u22), read protected.

  Others:
  1x 28x2 JAMMA edge connector.
  1x 12-legs connector (J1).
  1x 12-pins jumper (J2, J3).
  1x 2 pins jumper (J4).
  1x trimmer (volume)(P1).
  1x trimmer (unknown)(P2).
  1x 8x2 DIP switches (DIP1).
  1x Rayovac 3V. BR20xx lithium battery.

  Notes:
  PCB is marked: "lc" on component side ("LC" is the Italian for "Lato Componenti" which translates to "Components Side")
  PCB is marked: "H3" and "ls" on solder side ("LS" is the Italian for "Lato Saldature" which translates to "Solders Side")
  PCB is labeled: "ALTA TENSIONE", "Non rimuovere Alta Tensione 1.00" and "Passed 23/06/99" on component side

  STATUS:

  Memory map = done.
  Inputs =     done.
  Machine =    done.

  OKI 6295 =     ok.
  Screen size =  ok.
  Fixed layers = yes.

*/
ROM_START( altaten )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "alta_t.2.01a_2.u3", 0x00000, 0x20000, CRC(2ea79d6d) SHA1(2fc5a5c33e3e970b2b631b93238fe2411bdc2be9) )
	ROM_LOAD16_BYTE( "alta_t.2.01a_3.u2", 0x00001, 0x20000, CRC(62d57606) SHA1(1ad0935f511e22387ce7248f97ce4b89910570d2) )

	ROM_REGION( 0x10000, "mcu", 0 ) // h8/330 HD6473308cp10 with internal ROM
	ROM_LOAD( "mcu",        0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "alta_tensione_7.u35", 0x00000, 0x20000, CRC(90446541) SHA1(5d0e11221a762c9c11392c27e6bae931c8d2ad86) )
	ROM_LOAD( "alta_tensione_6.u36", 0x20000, 0x20000, CRC(84070651) SHA1(00d17d74e0923be41978064331940d145dc5f5e3) )
	ROM_LOAD( "alta_tensione_5.u37", 0x40000, 0x20000, CRC(68b26756) SHA1(7df0db4ec60b5179f27c08a401a9fa9f7dc316e9) )
	ROM_LOAD( "alta_tensione_4.u38", 0x60000, 0x20000, CRC(7683d3f5) SHA1(fc6ee8e6763eeb9d2bc5bdadf0507c5d606a69e9) )

	ROM_REGION( 0x080000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "alta_tensione_1.u32", 0x00000, 0x40000, CRC(4fe79e43) SHA1(7c154cb00e9b64fbdcc218280f2183b816cef20b) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "palce22v10h.u22", 0x0000, 0x02dd, NO_DUMP )
	ROM_LOAD( "palce16v8h.u54",  0x02dd, 0x0117, NO_DUMP )
ROM_END


/****************************
*       Driver Init         *
****************************/

void magic10_state::init_magic10()
{
	m_layer2_offset[0] = 32;
	m_layer2_offset[1] = 2;
}

void magic102_state::init_magic102()
{
	m_layer2_offset[0] = 8;
	m_layer2_offset[1] = 20;
}

void magic102_state::init_suprpool()
{
	m_layer2_offset[0] = 8;
	m_layer2_offset[1] = 16;
}

void hotslot_state::init_hotslot()
{
/*  a value of -56 center the playfield, but displace the intro and initial screen.
    a value of -64 center the intro and initial screen, but displace the playfield.
*/
	m_layer2_offset[0] = -56;   // X offset.
	m_layer2_offset[1] = 0; // Y offset.
}

void magic10_state::init_sgsafari()
{
	m_layer2_offset[0] = 16;
	m_layer2_offset[1] = 20;
}

void magic10_state::init_soccer10()
{
	m_layer2_offset[0] = 24;
	m_layer2_offset[1] = 0;

	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	for (int i = 0; i < 0x40000 / 2; i++)
		rom[i] = bitswap<16>(rom[i], 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 0, 1);
}

void spetrix_state::init_spetrix()
{
	m_layer2_offset[0] = 16;
	m_layer2_offset[1] = 16;
}

void magic102_state::init_altaten()
{
	m_layer2_offset[0] = 8;
	m_layer2_offset[1] = 16;

	// patching the boot protection...
	uint8_t *rom = memregion("maincpu")->base();

	rom[0x7668] = 0x71;
	rom[0x7669] = 0x4e;
}

} // anonymous namespace


/******************************
*        Game Drivers         *
******************************/

//     YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT           ROT   COMPANY                             FULLNAME                         FLAGS                                         LAYOUT
GAMEL( 1995, magic10,   0,        magic10,  magic10,  magic10_state,  init_magic10,  ROT0, "A.W.P. Games",                     "Magic's 10 (ver. 16.55)",       MACHINE_SUPPORTS_SAVE,                        layout_sgsafari )
GAMEL( 1995, magic10a,  magic10,  magic10,  magic10,  magic10_state,  init_magic10,  ROT0, "A.W.P. Games",                     "Magic's 10 (ver. 16.54)",       MACHINE_SUPPORTS_SAVE,                        layout_sgsafari )
GAMEL( 1995, magic10b,  magic10,  magic10a, magic10,  magic10_state,  init_magic10,  ROT0, "A.W.P. Games",                     "Magic's 10 (ver. 16.45)",       MACHINE_SUPPORTS_SAVE,                        layout_sgsafari )
GAMEL( 1995, magic10c,  magic10,  magic10a, magic10,  magic10_state,  init_magic10,  ROT0, "A.W.P. Games",                     "Magic's 10 (ver. 16.15)",       MACHINE_SUPPORTS_SAVE,                        layout_sgsafari )
GAMEL( 1996, soccer10,  0,        soccer10, magic10,  magic10_state,  init_soccer10, ROT0, "I.G.T. International Games Trade", "Soccer 10 (ver. 16.44)",        MACHINE_SUPPORTS_SAVE,                        layout_sgsafari )
GAME(  1997, magic102,  0,        magic102, magic102, magic102_state, init_magic102, ROT0, "ABM Games",                        "Magic's 10 2 (ver. 1.1)",       MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME(  1997, magic102a, magic102, magic102, magic102, magic102_state, init_magic102, ROT0, "ABM Games",                        "Magic's 10 2 (ver. BETA3)",     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME(  1998, suprpool,  0,        magic102, magic102, magic102_state, init_suprpool, ROT0, "ABM Games",                        "Super Pool (ver. 1.2)",         MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME(  1996, hotslot,   0,        hotslot,  hotslot,  hotslot_state,  init_hotslot,  ROT0, "ABM Games",                        "Hot Slot (ver. 05.01)",         MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME(  1999, mcolors,   0,        magic102, magic102, magic102_state, init_magic102, ROT0, "ABM Games",                        "Magic Colors (ver. 1.7a)",      MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME(  1999, mcolorsa,  mcolors,  magic102, magic102, magic102_state, init_magic102, ROT0, "ABM Games",                        "Magic Colors (ver. 1.6)",       MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAMEL( 1996, sgsafari,  0,        sgsafari, sgsafari, magic10_state,  init_sgsafari, ROT0, "New Impeuropex Corp.",             "Super Gran Safari (ver. 3.11)", MACHINE_SUPPORTS_SAVE,                        layout_sgsafari )
GAMEL( 1995, musicsrt,  0,        magic10a, musicsrt, magic10_state,  init_magic10,  ROT0, "ABM Games",                        "Music Sort (ver. 2.02)",        MACHINE_SUPPORTS_SAVE,                        layout_musicsrt )
GAME(  1998, lunaprk,   0,        magic102, magic102, magic102_state, init_suprpool, ROT0, "ABM Games",                        "Luna Park (ver. 1.2)",          MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME(  1999, altaten,   0,        magic102, magic102, magic102_state, init_altaten,  ROT0, "<unknown>",                        "Alta Tensione (ver. 2.01a)",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME(  199?, spetrix,   0,        spetrix,  spetrix,  spetrix_state,  init_spetrix,  ROT0, "<unknown>",                        "Super Petrix (ver. 1P)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
