// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca
/*******************************************************************************************

  Playmark 'Super Derby' Hardware
  -------------------------------

  Driver by David Haywood.
  Additional work by Roberto Fresca.


  Supported games:

  Lucky Boom                             1996, Playmark.
  Scacco Matto / Space Win,              1996, Playmark.
  Shiny Golds,                           1996, Playmark.
  Super Derby (2 sets),                  1996, Playmark.
  Croupier (Playmark Roulette) (2 sets), 1997, Playmark.
  Magic Touch,                           1998, Playmark.
  Tropical Fruits                        1999, Playmark.


********************************************************************************************

  NOTES:
  -----

  Payout / hopper controls not connected

  Croupier and Magical Touch have a PIC16C74 between the processor and the hopper


  Working notes: (Relating to SDERBY)

  Stephh's notes :

  - The game is playable, but :

      * it isn't possible to decrease the bet (but it might be an ingame "feature")
      * it isn't possible to insert a note
      * it isn't possible to exchange the winning points against tickets or cash

  - The settings can be modified in the "test mode", but there aren't mapped to
    input ports.


  EC notes :

  Thinks... Those three reads at the beginning - hopper, ticket and note acceptor tests?
  The system certainly performs those before printing the error.
  (There are three different error graphics in the ROM, depending on what is wrong)
  Also, hardware freezes if we try to turn the dispenser or acceptor on, because it's waiting
  for the response back from NOP?


********************************************************************************************

  TO DO :

  - figure out the reads from 0x308002.w and 0x30800e.w (see input_r read handler)
  (by default, demo sounds are OFF, so change this in the "test mode");
  - hook up the MCU for croupier, croupiera (needs PIC16C74 core);
  - dump and hook up the MCU (PIC16C65) for croupierb, magictch, tropfrt;

*******************************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "pmroulet.lh"
#include "sderby.lh"
#include "spacewin.lh"


// configurable logging
#define LOG_INPUTS           (1U << 1)
#define LOG_CROUPIER_MCU     (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_INPUTS | LOG_CROUPIER_MCU)

#include "logmacro.h"

#define LOGINPUTS(...)          LOGMASKED(LOG_INPUTS,           __VA_ARGS__)
#define LOGCROUPIERMCU(...)     LOGMASKED(LOG_CROUPIER_MCU,     __VA_ARGS__)


namespace {

class sderby_state : public driver_device
{
public:
	sderby_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg_videoram(*this, "bg_videoram"),
		m_md_videoram(*this, "md_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_in(*this, "IN%u", 0U),
		m_lamps(*this, "lamp%u", 1U)
	{ }


	void luckboom(machine_config &config);
	void pmroulet(machine_config &config);
	void sderby(machine_config &config);
	void sderbya(machine_config &config);
	void shinygld(machine_config &config);
	void spacewin(machine_config &config);

protected:
	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;

	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_md_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_ioport_array<3> m_in;
	output_finder<7> m_lamps;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_md_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	uint16_t m_scroll[6]{};

	uint8_t m_sprites_x_kludge = 0;
	uint8_t m_sprites_y_kludge = 0;

	uint16_t input_r(offs_t offset);
	uint16_t roulette_input_r(offs_t offset);
	uint16_t rprot_r();
	void rprot_w(uint16_t data);
	void sderby_out_w(uint16_t data);
	void scmatto_out_w(uint16_t data);
	void roulette_out_w(uint16_t data);
	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void md_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_md_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pmroulet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void luckboom_map(address_map &map) ATTR_COLD;
	void roulette_map(address_map &map) ATTR_COLD;
	void sderby_map(address_map &map) ATTR_COLD;
	void sderbya_map(address_map &map) ATTR_COLD;
	void shinygld_map(address_map &map) ATTR_COLD;
	void spacewin_map(address_map &map) ATTR_COLD;
};

class zw3_state : public sderby_state
{
public:
	using sderby_state::sderby_state;

	void zw3(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void zw3_map(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(sderby_state::get_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index * 2];
	int colour = m_bg_videoram[tile_index * 2 + 1] & 0x0f;

	tileinfo.set(1, tileno, colour, 0);
}

void sderby_state::bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}


TILE_GET_INFO_MEMBER(sderby_state::get_md_tile_info)
{
	int tileno = m_md_videoram[tile_index * 2];
	int colour = m_md_videoram[tile_index * 2 + 1] & 0x0f;

	tileinfo.set(1, tileno, colour + 16, 0);
}

void sderby_state::md_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_md_videoram[offset]);
	m_md_tilemap->mark_tile_dirty(offset / 2);
}


TILE_GET_INFO_MEMBER(sderby_state::get_fg_tile_info)
{
	int tileno = m_fg_videoram[tile_index * 2];
	int colour = m_fg_videoram[tile_index * 2 + 1] & 0x0f;

	tileinfo.set(0, tileno, colour + 32, 0);
}

TILE_GET_INFO_MEMBER(zw3_state::get_fg_tile_info)
{
	int tileno = (m_fg_videoram[tile_index * 2] << 2) | ((m_fg_videoram[tile_index * 2 + 1] & 0xc000) >> 14);
	int colour = m_fg_videoram[tile_index * 2 + 1] & 0x0f;

	tileinfo.set(0, tileno, colour + 32, 0);
}

void sderby_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}


void sderby_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int height = m_gfxdecode->gfx(0)->height();
	int colordiv = m_gfxdecode->gfx(0)->granularity() / 16;

	for (int offs = 4; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		int sy = m_spriteram[offs + 3 - 4]; // -4? what the... ???
		if (sy == 0x2000) return;   // end of list marker

		int flipx = sy & 0x4000;
		int sx = (m_spriteram[offs + 1] & 0x01ff) - 16 - m_sprites_x_kludge;
		sy = (256 - m_sprites_y_kludge - height - sy) & 0xff;
		int code = m_spriteram[offs + 2];
		int color = (m_spriteram[offs + 1] & 0x3e00) >> 9;

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				color / colordiv + 48,
				flipx, 0,
				sx, sy, 0);
	}
}


void sderby_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sderby_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_md_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sderby_state::get_md_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_md_tilemap->set_transparent_pen(0);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sderby_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);

	m_sprites_x_kludge = 0x07;
	m_sprites_y_kludge = 0x08;

	save_item(NAME(m_scroll));
}

void zw3_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(zw3_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_md_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(zw3_state::get_md_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_md_tilemap->set_transparent_pen(0);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(zw3_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);

	m_sprites_x_kludge = 0x00;
	m_sprites_y_kludge = 0x07;

	save_item(NAME(m_scroll));
}


uint32_t sderby_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_md_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t sderby_state::screen_update_pmroulet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_md_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void sderby_state::scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0: m_fg_tilemap->set_scrollx(0, data + 2); break;
		case 1: m_fg_tilemap->set_scrolly(0, data - 8); break;
		case 2: m_md_tilemap->set_scrollx(0, data + 4); break;
		case 3: m_md_tilemap->set_scrolly(0, data - 8); break;
		case 4: m_bg_tilemap->set_scrollx(0, data + 6); break;
		case 5: m_bg_tilemap->set_scrolly(0, data - 8); break;
	}
}


/***************************
*       R/W Handlers       *
***************************/

uint16_t sderby_state::input_r(offs_t offset)
{
	switch (offset)
	{
		case 0x00 >> 1:
			return m_in[0]->read();
		case 0x02 >> 1:
			return 0xffff;          // to avoid game to reset (needs more work)
	}

	LOGINPUTS("input_r : offset = %x - PC = %06x\n", offset * 2, m_maincpu->pc());

	return 0xffff;
}

uint16_t sderby_state::roulette_input_r(offs_t offset)
{
	switch (offset)
	{
		case 0x00 >> 1:
			return m_in[0]->read();
		case 0x02 >> 1:
			return m_in[1]->read();
		case 0x04 >> 1:
			return m_in[2]->read();
	}

	return 0xffff;
}


/***************************************************************

    Roulette MCU communication.
    ---------------------------

    Defeating the 'always win' protection...

    Offset: 0x70800e - 0x70800f.

    Writes to the MCU are always the same values.
    At beginning, the code writes 3 values: 0x53, 0x5a and 0x0d.
    Then, with each placed bet the code normally writes 0x4e.
    After that, there are 2 reads expecting the MCU response.

    Most probably there is a shared RAM there for communication,
    but for now, I temporarily simulated the MCU response till
    we can get the MCU hooked up.


****************************************************************/

uint16_t sderby_state::rprot_r()
{
	LOGCROUPIERMCU("rprot_r : offset = %02x\n", m_maincpu->pc());

/* This is the only mask I found that allow a normal play.
   Using other values, the game hangs waiting for response,
   or simply throw a deliberated losing number.

   If someone more skilled in 68K code can help to trace it,
   searching for an accurate response, I'll appreciate.
*/
	return machine().rand() & 0x1f;
}

void sderby_state::rprot_w(uint16_t data)
{
	LOGCROUPIERMCU("rprot_w %02x\n", data);
}


/******************************
*       Outputs / Lamps       *
******************************/

void sderby_state::sderby_out_w(uint16_t data)
{
/*
  ---------------------------
  --- Super Derby Outputs ---
  ---------------------------

  0x0000 - Normal State (lamps off).
  0x0001 - Start lamp.
  0x0002 - Bet lamp.

  0x0100 - Ticket dispenser out.
  0x0800 - Unknown (always activated).
  0x1000 - Hopper out.
  0x2000 - Coin counter.
  0x4000 - Unknown.
  0x8000 - End of Race lamp.


    - Lbits -
    7654 3210
    =========
    ---- ---x  Start lamp.
    ---- --x-  Bet lamp.

    - Hbits -
    7654 3210
    =========
    ---- ---x  Ticket dispenser out.
    ---- x---  unknown (always activated).
    ---x ----  Hopper out.
    --x- ----  Coin counter.
    -x-- ----  unknown.
    x--- ----  End of Race lamp.

*/
	m_lamps[0] = BIT(data, 0);      // Lamp 1 - START
	m_lamps[1] = BIT(data, 1);      // Lamp 2 - BET
	m_lamps[2] = BIT(data, 15);     // Lamp 3 - END OF RACE

	machine().bookkeeping().coin_counter_w(0, data & 0x2000);
}


void sderby_state::scmatto_out_w(uint16_t data)
{
/*
  ----------------------------------------
  --- Scacco Matto / Space Win Outputs ---
  ----------------------------------------

  0x0000 - Normal State (lamps off).
  0x0001 - Hold 1 lamp.
  0x0002 - Hold 2 lamp.
  0x0004 - Hold 3 lamp.
  0x0008 - Hold 4 lamp.
  0x0010 - Hold 5 lamp.
  0x0020 - Start lamp.
  0x0040 - Bet lamp.
  0x1000 - Hopper out.
  0x2000 - Coin counter.


    - Lbits -
    7654 3210
    =========
    ---- ---x  Hold1 lamp.
    ---- --x-  Hold2 lamp.
    ---- -x--  Hold3 lamp.
    ---- x---  Hold4 lamp.
    ---x ----  Hold5 lamp.
    --x- ----  Start lamp.
    -x-- ----  Bet lamp.

    - Hbits -
    7654 3210
    =========
    ---x ----  Hopper out.
    --x- ----  Coin counter.

*/
	m_lamps[0] = BIT(data, 0);      // Lamp 1 - HOLD 1
	m_lamps[1] = BIT(data, 1);      // Lamp 2 - HOLD 2
	m_lamps[2] = BIT(data, 2);      // Lamp 3 - HOLD 3
	m_lamps[3] = BIT(data, 3);      // Lamp 4 - HOLD 4
	m_lamps[4] = BIT(data, 4);      // Lamp 5 - HOLD 5
	m_lamps[5] = BIT(data, 5);      // Lamp 6 - START
	m_lamps[6] = BIT(data, 6);      // Lamp 7 - BET

	machine().bookkeeping().coin_counter_w(0, data & 0x2000);
}


void sderby_state::roulette_out_w(uint16_t data)
{
/*
  -----------------------------------
  --- Croupier (Roulette) Outputs ---
  -----------------------------------

  0x708006 - 0x708007
  ===================

  0x0000 - Normal State (lamps off).
  0x0001 - Start lamp.
  0x0002 - Bet lamp.
  0x0008 - Unknown (always activated).


    - Lbits -
    7654 3210
    =========
    ---- ---x  Start lamp.
    ---- --x-  Bet lamp.
    ---- x---  Unknown (always activated).

*/
	m_lamps[0] = BIT(data, 0);      // Lamp 1 - START
	m_lamps[1] = BIT(data, 1);      // Lamp 2 - BET
}


/***************************
*       Memory Maps        *
***************************/

void sderby_state::sderby_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x100fff).ram().w(FUNC(sderby_state::bg_videoram_w)).share(m_bg_videoram);
	map(0x101000, 0x101fff).ram().w(FUNC(sderby_state::md_videoram_w)).share(m_md_videoram);
	map(0x102000, 0x103fff).ram().w(FUNC(sderby_state::fg_videoram_w)).share(m_fg_videoram);
	map(0x104000, 0x10400b).w(FUNC(sderby_state::scroll_w));
	map(0x10400c, 0x10400d).nopw();    // ??? - check code at 0x000456 (executed once at startup)
	map(0x10400e, 0x10400f).nopw();    // ??? - check code at 0x000524 (executed once at startup)
	map(0x200000, 0x200fff).ram().share(m_spriteram);
	map(0x308000, 0x30800d).r(FUNC(sderby_state::input_r));
	map(0x308008, 0x308009).w(FUNC(sderby_state::sderby_out_w));
	map(0x30800f, 0x30800f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x380000, 0x380fff).w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x500001).nopw();
	map(0xd00000, 0xd007ff).ram().share("nvram");
	map(0xffc000, 0xffffff).ram();
}

void sderby_state::sderbya_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x100fff).ram().w(FUNC(sderby_state::bg_videoram_w)).share(m_bg_videoram);
	map(0x101000, 0x101fff).ram().w(FUNC(sderby_state::md_videoram_w)).share(m_md_videoram);
	map(0x102000, 0x103fff).ram().w(FUNC(sderby_state::fg_videoram_w)).share(m_fg_videoram);
	map(0x104000, 0x10400b).w(FUNC(sderby_state::scroll_w));
	map(0x10400c, 0x10400d).nopw();    // ??? - check code at 0x000456 (executed once at startup)
	map(0x10400e, 0x10400f).nopw();    // ??? - check code at 0x000524 (executed once at startup)
	map(0x200000, 0x200fff).ram().share(m_spriteram);
	map(0x308000, 0x30800d).r(FUNC(sderby_state::input_r));
	map(0x308008, 0x308009).w(FUNC(sderby_state::sderby_out_w));
	map(0x30800f, 0x30800f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x380000, 0x380fff).w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x300001).nopw();    // unknown... write 0x01 in game, and 0x00 on reset      //MOD
	map(0xcf0000, 0xcf07ff).ram().share("nvram");
	map(0xcfc000, 0xcfffff).ram();                                                                     //MOD
}


void sderby_state::luckboom_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x100fff).ram().w(FUNC(sderby_state::bg_videoram_w)).share(m_bg_videoram);
	map(0x101000, 0x101fff).ram().w(FUNC(sderby_state::md_videoram_w)).share(m_md_videoram);
	map(0x102000, 0x103fff).ram().w(FUNC(sderby_state::fg_videoram_w)).share(m_fg_videoram);
	map(0x104000, 0x10400b).w(FUNC(sderby_state::scroll_w));
	map(0x10400c, 0x10400d).nopw();    // ??? - check code at 0x000456 (executed once at startup)
	map(0x10400e, 0x10400f).nopw();    // ??? - check code at 0x000524 (executed once at startup)
	map(0x200000, 0x200fff).ram().share(m_spriteram);
	map(0x308000, 0x30800d).r(FUNC(sderby_state::input_r));
	map(0x308008, 0x308009).w(FUNC(sderby_state::sderby_out_w));
	map(0x30800f, 0x30800f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x380000, 0x380fff).w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x500001).nopw();    // unknown... write 0x01 in game, and 0x00 on reset
	map(0xe00000, 0xe007ff).ram().share("nvram");
	map(0xff0000, 0xffffff).ram();
}

void sderby_state::spacewin_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x100fff).ram().w(FUNC(sderby_state::bg_videoram_w)).share(m_bg_videoram);
	map(0x101000, 0x101fff).ram().w(FUNC(sderby_state::md_videoram_w)).share(m_md_videoram);
	map(0x102000, 0x103fff).ram().w(FUNC(sderby_state::fg_videoram_w)).share(m_fg_videoram);
	map(0x104000, 0x10400b).w(FUNC(sderby_state::scroll_w));
	map(0x10400c, 0x10400d).nopw();    // seems another video register. constantly used
	map(0x10400e, 0x10400f).nopw();    // seems another video register. constantly used
	map(0x104010, 0x105fff).nopw();    // unknown
	map(0x300000, 0x300001).nopw();    // unknown... write 0x01 in game, and 0x00 on reset
	map(0x308000, 0x30800d).r(FUNC(sderby_state::input_r));
	map(0x308008, 0x308009).w(FUNC(sderby_state::scmatto_out_w));
	map(0x30800f, 0x30800f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x380000, 0x380fff).w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd00000, 0xd001ff).ram();
	map(0x800000, 0x800fff).ram().share(m_spriteram);
	map(0x801000, 0x80100d).nopw();    // unknown
	map(0x8f0000, 0x8f07ff).ram().share("nvram");   // 16K Dallas DS1220Y-200 NVRAM
	map(0x8fc000, 0x8fffff).ram();
}

void sderby_state::shinygld_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x100fff).ram().w(FUNC(sderby_state::bg_videoram_w)).share(m_bg_videoram);
	map(0x101000, 0x101fff).ram().w(FUNC(sderby_state::md_videoram_w)).share(m_md_videoram);
	map(0x102000, 0x103fff).ram().w(FUNC(sderby_state::fg_videoram_w)).share(m_fg_videoram);
	map(0x104000, 0x10400b).w(FUNC(sderby_state::scroll_w));
	map(0x10400c, 0x10400d).nopw();    // seems another video register. constantly used
	map(0x10400e, 0x10400f).nopw();    // seems another video register. constantly used
	map(0x104010, 0x105fff).nopw();    // unknown
	map(0x300000, 0x300001).nopw();    // unknown... write 0x01 in game, and 0x00 on reset
	map(0x308000, 0x30800d).r(FUNC(sderby_state::input_r));
	map(0x308008, 0x308009).nopw();
	map(0x30800f, 0x30800f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x380000, 0x380fff).w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x400fff).ram().share(m_spriteram);
	map(0x401000, 0x40100d).nopw();    // unknown
	map(0x700000, 0x7007ff).ram().share("nvram");   // 16K Dallas DS1220Y-200 NVRAM
	map(0x7f0000, 0x7fffff).ram();
}

void sderby_state::roulette_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x440000, 0x440fff).writeonly().share(m_spriteram);
	map(0x500000, 0x500fff).ram().w(FUNC(sderby_state::bg_videoram_w)).share(m_bg_videoram);
	map(0x501000, 0x501fff).ram().w(FUNC(sderby_state::md_videoram_w)).share(m_md_videoram);
	map(0x502000, 0x503fff).ram().w(FUNC(sderby_state::fg_videoram_w)).share(m_fg_videoram);
	map(0x504000, 0x50400b).ram().w(FUNC(sderby_state::scroll_w));
	map(0x50400e, 0x50400f).nopw();

	map(0x708000, 0x708009).r(FUNC(sderby_state::roulette_input_r));
	map(0x708006, 0x708007).w(FUNC(sderby_state::roulette_out_w));
	map(0x70800b, 0x70800b).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x70800c, 0x70800d).nopw();    // watchdog?? (0x0003)
	map(0x70800e, 0x70800f).rw(FUNC(sderby_state::rprot_r), FUNC(sderby_state::rprot_w)); // MCU communication
	map(0x780000, 0x780fff).w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0xff0000, 0xff07ff).ram().share("nvram");
	map(0xffc000, 0xffffff).ram();
}

void zw3_state::zw3_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x440000, 0x440fff).writeonly().share(m_spriteram);
	map(0x500000, 0x500fff).ram().w(FUNC(zw3_state::bg_videoram_w)).share(m_bg_videoram);
	map(0x501000, 0x501fff).ram().w(FUNC(zw3_state::md_videoram_w)).share(m_md_videoram);
	map(0x502000, 0x503fff).ram().w(FUNC(zw3_state::fg_videoram_w)).share(m_fg_videoram);
	map(0x504000, 0x50400b).ram().w(FUNC(zw3_state::scroll_w));
	map(0x50400e, 0x50400f).nopw();
	map(0x708000, 0x708001).portr("IN0");
	map(0x708002, 0x708003).portr("IN1");
	map(0x708008, 0x708009).w(FUNC(zw3_state::roulette_out_w));
	map(0x70800c, 0x70800d).rw(FUNC(zw3_state::rprot_r), FUNC(zw3_state::rprot_w));
	map(0x70800f, 0x70800f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x780000, 0x780fff).w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xf00000, 0xf007ff).ram().share("nvram");
	map(0xff0000, 0xffffff).ram();
}


/***************************
*       Input Ports        *
***************************/

static INPUT_PORTS_START( sderby )
	PORT_START("IN0")   // 0x308000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_4WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_4WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Collect")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )        // Adds n credits depending on settings in service menu
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_NO_TOGGLE(0x1000, IP_ACTIVE_LOW)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )      // check code at 0x00765e
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( sderbya )
	PORT_START("IN0")   // 0x308000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_4WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_4WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Collect")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )        // Adds n credits depending on settings in service menu
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_NO_TOGGLE(0x1000, IP_ACTIVE_LOW)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )      // check code at 0x00765e
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( luckboom )
	PORT_START("IN0")   // 0x308000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_NO_TOGGLE(0x1000, IP_ACTIVE_LOW)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( spacewin )
	PORT_START("IN0")   // 0x308000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x1000, IP_ACTIVE_LOW)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( shinygld )
	PORT_START("IN0")   // 0x308000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x1000, IP_ACTIVE_LOW)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( pmroulet )
	PORT_START("IN0")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") // it must be toggled to boot anyway
	PORT_SERVICE_NO_TOGGLE(0x0020, IP_ACTIVE_LOW)
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x000e, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) // to cancel bets in 3-button mode
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( croupierb )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) // to cancel bets in 3-button mode
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START1 ) // to turn roulette
	PORT_SERVICE_NO_TOGGLE(0x1000, IP_ACTIVE_LOW)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") // it must be toggled to boot anyway
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( magictch )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) // buttons 1-6 have different uses depending on the game selected
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x1000, IP_ACTIVE_LOW)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") // it must be toggled to boot anyway
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( tropfrt )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Low")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / High")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / Double Up")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Collect")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x1000, IP_ACTIVE_LOW)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") // it must be toggled to boot anyway
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/****************************
*     Graphics Layouts      *
****************************/

static const gfx_layout tiles16x16_layout =
{
	16, 16,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(4,5), RGN_FRAC(3,5), RGN_FRAC(2,5), RGN_FRAC(1,5), RGN_FRAC(0,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		128+0, 128+1, 128+2, 128+3, 128+4, 128+5, 128+6, 128+7
	},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		64+0*8,64+1*8,64+2*8,64+3*8,64+4*8,64+5*8,64+6*8,64+7*8
	},
	256,
};


/****************************
*      Graphics Decode      *
****************************/

static GFXDECODE_START( gfx_sderby )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x5_planar,  0x000, 256  )
	GFXDECODE_ENTRY( "gfx", 0, tiles16x16_layout, 0x000, 256  )
GFXDECODE_END


/****************************
*      Machine Drivers      *
****************************/

void sderby_state::sderby(machine_config &config)
{
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sderby_state::sderby_map);
	m_maincpu->set_vblank_int("screen", FUNC(sderby_state::irq2_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(4*8, 44*8-1, 3*8, 33*8-1);
	screen.set_screen_update(FUNC(sderby_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sderby);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_555, 0x1000);

	SPEAKER(config, "mono").front_center();
	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}

void sderby_state::sderbya(machine_config &config)
{
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sderby_state::sderbya_map);
	m_maincpu->set_vblank_int("screen", FUNC(sderby_state::irq2_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(4*8, 44*8-1, 3*8, 33*8-1);
	screen.set_screen_update(FUNC(sderby_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sderby);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_555, 0x1000);

	SPEAKER(config, "mono").front_center();
	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}

void sderby_state::luckboom(machine_config &config)
{
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sderby_state::luckboom_map);
	m_maincpu->set_vblank_int("screen", FUNC(sderby_state::irq4_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(4*8, 44*8-1, 3*8, 33*8-1);
	screen.set_screen_update(FUNC(sderby_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sderby);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_555, 0x1000);

	SPEAKER(config, "mono").front_center();
	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}

void sderby_state::spacewin(machine_config &config)
{
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sderby_state::spacewin_map);
	m_maincpu->set_vblank_int("screen", FUNC(sderby_state::irq2_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(4*8, 44*8-1, 3*8, 33*8-1);
	screen.set_screen_update(FUNC(sderby_state::screen_update_pmroulet));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sderby);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_555, 0x1000);

	SPEAKER(config, "mono").front_center();
	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}

void sderby_state::shinygld(machine_config &config)
{
	M68000(config, m_maincpu, 24_MHz_XTAL / 2); // verified
	m_maincpu->set_addrmap(AS_PROGRAM, &sderby_state::shinygld_map);
	m_maincpu->set_vblank_int("screen", FUNC(sderby_state::irq4_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57.47); // measured on PCB
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(4*8, 44*8-1, 3*8, 33*8-1);
	screen.set_screen_update(FUNC(sderby_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sderby);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_555, 0x1000);

	SPEAKER(config, "mono").front_center();
	OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 verified
}

void sderby_state::pmroulet(machine_config &config)
{
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sderby_state::roulette_map);
	m_maincpu->set_vblank_int("screen", FUNC(sderby_state::irq2_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(64*8, 64*8);
	screen.set_visarea(4*8, 44*8-1, 3*8, 33*8-1);
	screen.set_screen_update(FUNC(sderby_state::screen_update_pmroulet));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sderby);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_555, 0x1000);

	SPEAKER(config, "mono").front_center();
	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}

void zw3_state::zw3(machine_config &config)
{
	pmroulet(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &zw3_state::zw3_map);
	m_maincpu->set_vblank_int("screen", FUNC(sderby_state::irq4_line_hold));

	subdevice<okim6295_device>("oki")->set_clock(1_MHz_XTAL); // pin 7 verified
}


/****************************
*        Rom Loads          *
****************************/
/*

Super Derby
Playmark '96

mc 68k 12mhz
OKI m6295

2x GM 76c88al (8kx8 6264) near program ROMs
2x 6264 near gfx ROMs
16k nonvolatile SRAM (Dallas 1220Y)
two FPGA chips - one labelled 'Playmark 010412'
4x hm3-65728bk-5
--
21.bin 6F9F2F2B - near OKI (samples?)

22.bin A319F1E0 - program code
23.bin 1D6E2321 /

24.bin 93C917DF  - gfx
25.bin 7BA485CD
26.bin BEABE4F7
27.bin 672CE5DF
28.bin 39CA3B52

*/
ROM_START( sderby )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "22.bin", 0x00000, 0x20000, CRC(a319f1e0) SHA1(d932cc7e990aa87308dcd9ffa5af2aaea333aa9a) )
	ROM_LOAD16_BYTE( "23.bin", 0x00001, 0x20000, CRC(1d6e2321) SHA1(3bb32021cc9ee6bd6d1fd79a89159fef70f34f41) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "21.bin", 0x00000, 0x80000, CRC(6f9f2f2b) SHA1(9778439979bc21b3e49f0c16353488a33b93c01b) )

	ROM_REGION( 0xa0000, "gfx", 0 )
	ROM_LOAD( "24.bin", 0x00000, 0x20000, CRC(93c917df) SHA1(dc2fa5e29749ec92871c66146c0412a23f47e316) )
	ROM_LOAD( "25.bin", 0x20000, 0x20000, CRC(7ba485cd) SHA1(b0170614d713af9d1556251c76ae762de872abe6) )
	ROM_LOAD( "26.bin", 0x40000, 0x20000, CRC(beabe4f7) SHA1(a5615450fae930cb2408f201a9faa12551de0d70) )
	ROM_LOAD( "27.bin", 0x60000, 0x20000, CRC(672ce5df) SHA1(cdf3af842cbcbf53cc73d9986744dc9cfa92c71a) )
	ROM_LOAD( "28.bin", 0x80000, 0x20000, CRC(39ca3b52) SHA1(9a03e73d88a1551cd3cfe616ab71e67dced1272a) )
ROM_END

ROM_START( sderbya )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "22.u16", 0x00000, 0x20000, CRC(5baadc33) SHA1(475843d3f99f5a6aa25bdba75b251ad6be32802f) )
	ROM_LOAD16_BYTE( "23.u15", 0x00001, 0x20000, CRC(04518b8c) SHA1(97598c43c1cb0a757bca70c0a498838144b2302b) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "274001.u147", 0x00000, 0x80000,  CRC(6f9f2f2b) SHA1(9778439979bc21b3e49f0c16353488a33b93c01b) )

	ROM_REGION( 0x140000, "gfx", 0 )
	ROM_LOAD( "24.u141", 0x000000, 0x40000, CRC(54234f72) SHA1(c2e3e547255174daa9a6acad743c210b92ded385) )
	ROM_LOAD( "25.u142", 0x040000, 0x40000, CRC(1bd98cf7) SHA1(67143a16e2cf01868fac3d8d6d9db423f77d09b9) )
	ROM_LOAD( "26.u143", 0x080000, 0x40000, CRC(21bf2004) SHA1(7f7cbdcd89807c9e8ca64023a0060b18ef9b6536) )
	ROM_LOAD( "27.u144", 0x0c0000, 0x40000, CRC(097e0b27) SHA1(26ea6c0996a62e52241b335be64f46d7586f997a) )
	ROM_LOAD( "28.u145", 0x100000, 0x40000, CRC(580daff7) SHA1(2813faa3509d86ad1f721381fa600f28f8f51e09) )
ROM_END


/* Scacco Matto / Space Win
   Playmark, 1996.

CPU:
1x MC68000P12 (main)(u24)

Sound:
1x M6295 (sound)(u146)
1x TDA2003 (sound)(td1)
1x 358D (sound)(u155)

PLDs:
2x A1020B-PL84C (u110,u137)(not dumped)

Xtals:
1x oscillator 24.000000MHz (xl1)
1x oscillator 28.000000MHz (xl2)
1x blu resonator 1000J (sound)(y1)

ROMs:
1x M27C2001 (1)
7x M27C1001 (2,3,4,5,6,7,8)

4x PALCE22V10H (not dumped)
1x PALCE16V8H (not dumped)
1x DS1220Y-200 (non volatile ram)

Note:
1x JAMMA edge connector
1x 12 legs connector
2x 6 legs connector
1x pushbutton (cb1)
1x trimmer (volume)

Title depends on graphics type in test mode.

*/
ROM_START( spacewin )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "2.u16", 0x00000, 0x20000, CRC(2d17c2ab) SHA1(833ab39081fbc3d114103055e3a3f2ea2a28f158) )
	ROM_LOAD16_BYTE( "3.u15", 0x00001, 0x20000, CRC(fd6f93ef) SHA1(1dc35fd92a0185434b44aa3c7da47a408fb2ce27) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "1.u147", 0x00000, 0x40000,  CRC(eedc7090) SHA1(fc8cabf7a11a1de3ccc3b8860afd8f32669608b8) )

	ROM_REGION( 0xa0000, "gfx", 0 )
	ROM_LOAD( "4.u141", 0x000000, 0x20000, CRC(ce20c599) SHA1(93358c3a891c66ca944eb684bd47e9c25bfcb88d) )
	ROM_LOAD( "5.u142", 0x020000, 0x20000, CRC(ae4f8e06) SHA1(cb3b941c67ae5c9df005d9f8b2105dc5a114f19f) )
	ROM_LOAD( "6.u143", 0x040000, 0x20000, CRC(b99afc96) SHA1(589270dbd5022db2d032da85a9813c271fa71a90) )
	ROM_LOAD( "7.u144", 0x060000, 0x20000, CRC(30f212ad) SHA1(ecafd93b11ff15386ef02e9f6657a52baf7932b4) )
	ROM_LOAD( "8.u145", 0x080000, 0x20000, CRC(541a73fd) SHA1(fede5e2fcbb18e90cc50995d44e831c3f9b56614) )
ROM_END

// This board seems very similar to the spacewin's one.
ROM_START( shinygld )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "16.u16", 0x00000, 0x20000, CRC(4b08a668) SHA1(8444e7624a40e14eca73c425db1cd574c2bf02e0) ) // 27C1001
	ROM_LOAD16_BYTE( "17.u15", 0x00001, 0x20000, CRC(eb126e19) SHA1(d02ba03551f71f3ca46bee49291e4423daeea0ce) ) // 27C1001

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "15.u147", 0x00000, 0x40000,  CRC(66db6b15) SHA1(4a1ca962c61d60db8ff66c81253131d067bec8c6) ) // 27C2001

	ROM_REGION( 0xa0000, "gfx", 0 )
	ROM_LOAD( "18.u141", 0x000000, 0x20000, CRC(a905c02d) SHA1(07a0a70424aa759e4c552408154c2d1515f512d5) ) // 27C1001
	ROM_LOAD( "19.u142", 0x020000, 0x20000, CRC(8db53f3b) SHA1(aeb2da8bb065bb49edae1de51f6db9a7a02989c3) ) // 27C1001
	ROM_LOAD( "20.u143", 0x040000, 0x20000, CRC(be54ae86) SHA1(23c8a01aa05204a6cd186a59269e515a068e23c4) ) // 27C1001
	ROM_LOAD( "21.u144", 0x060000, 0x20000, CRC(de3be666) SHA1(647e054b9aefa80d95f6fb26cc89a976538f0d7c) ) // 27C1001
	ROM_LOAD( "22.u145", 0x080000, 0x20000, CRC(113ccf81) SHA1(c2192e81a4be38911c971e1593b902f92d9a02d5) ) // 27C1001

	/* Dumper's note: Board carries five PLDs : one GAL22V10 near the 68000 CPU (which most likely acts as main address decoder),
	   three PALCE22V10H (two are near the GFX ROMs, probably for enabling them), one PALCE16V8H.*/
	ROM_REGION( 0xd00, "plds", 0)
	ROM_LOAD( "gal22v10.bin",       0x000, 0x2e5, NO_DUMP ) // soldered
	ROM_LOAD( "palce22v10_1.bin",   0x300, 0x2dd, NO_DUMP ) // soldered
	ROM_LOAD( "palce22v10_2.bin",   0x600, 0x2dd, NO_DUMP ) // soldered
	ROM_LOAD( "palce22v10_3.bin",   0x900, 0x2dd, NO_DUMP ) // soldered
	ROM_LOAD( "palce16v8h.bin",     0xb00, 0x117, NO_DUMP ) // soldered
ROM_END

/*
Croupier (PlaYMark)
-------------------

1x MC68000FN12 (U24, QFP)
1x PIC16C74 (U39, scratched)

1x OKI M6295 (U34, near ROM 1)
1x Resonator 1000J (Y1)

2x KM6264BL-10L SRAM (U2/U6, near ROMs 2 & 3)
2x KM6264BL-10 SRAM (U36/U37, near ROMs 4, 5, 6, 7 & 8)
1x DALLAS DS1220Y-100 NONVOLATILE SRAM (U17)

2x ACTEL A1020B PL84C
4x GAL 22CV10-25LNC (U1, U77, U111, U112. Undumped)

1x 24.000000 MHz. Xtal. (XTL1)
1x 28.000000 MHz. Xtal. (XTL2)

1x Pot (TR1, near OKI M6295)
1x Push Button.
1x JAMMA connector.

ROMs:
-----

1.bin : AMD Am27C020.
checksum : 03FC0000h
CRC-32 : B7094978h

2.bin : ST M27C1001.
checksum : 00B958B6h
CRC-32 : E7941975h

3.bin : ST M27C1001.
checksum : 011DE502h
CRC-32 : 29D06A38h

4.bin : MACRONIX MX27C4000.
checksum : 01CADC6Ch
CRC-32 : EFCDDAC9h

5.bin : MACRONIX MX27C4000.
checksum : 01F8DEB8h
CRC-32 : BC75EF8Fh

6.bin : MACRONIX MX27C4000.
checksum : 0195B5C0h
CRC-32 : E47D5F55h

7.bin : MACRONIX MX27C4000.
checksum : 013483F4h
CRC-32 : 0FA6CE7Dh

8.bin : MACRONIX MX27C4000.
checksum : 0106A95Ch
CRC-32 : D4C2B7DAh

*/
ROM_START( croupier )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x20000, CRC(e7941975) SHA1(ea32cd51b8d87205a1d6c6a83ebf8b50e03c55fc))
	ROM_LOAD16_BYTE( "3.bin", 0x00001, 0x20000, CRC(29d06a38) SHA1(c6fdca1a31fad9abf854e521e593f3ec8018ae6d))

	ROM_REGION( 0x4008, "pic16c74", 0 )
	ROM_LOAD( "pic16c74.u39", 0x00000, 0x4008, CRC(9cb88e5b) SHA1(3c6b371efeda757f2bcab6c860e7585b628c210a))

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x40000, CRC(6673de85) SHA1(df390cd6268efc0e743a9020f19bc0cbeb757cfa))

	ROM_REGION( 0x280000, "gfx", 0 )
	ROM_LOAD( "4.bin", 0x000000, 0x80000, CRC(efcddac9) SHA1(72435ec478b70a067d47f3daf7c224169ee5827a))
	ROM_LOAD( "5.bin", 0x080000, 0x80000, CRC(bc75ef8f) SHA1(1f3dc457e5ae143d53cfef0e1fcb4586dceefb67))
	ROM_LOAD( "6.bin", 0x100000, 0x80000, CRC(e47d5f55) SHA1(a341e24f98125265cb3986f8c7ce84eedd056b71))
	ROM_LOAD( "7.bin", 0x180000, 0x80000, CRC(0fa6ce7d) SHA1(5ba96c9c0625a131d890d9c0c0f65cb2a03fa084))
	ROM_LOAD( "8.bin", 0x200000, 0x80000, CRC(d4c2b7da) SHA1(515be861443acc5b911241dbaafa42e02f79985a))
ROM_END

ROM_START( croupiera )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x20000, CRC(1677a2de) SHA1(4dcbb3c1ce9b65e06ba7e0cffa00c0c8016538f5)) // sldh
	ROM_LOAD16_BYTE( "3.bin", 0x00001, 0x20000, CRC(11acaac2) SHA1(19e7bbbf4356fc9a866f9f36d0568c42d6a36c07)) // sldh

	ROM_REGION( 0x4008, "pic16c74", 0 )
	ROM_LOAD( "pic16c74.u39", 0x00000, 0x4008, CRC(9cb88e5b) SHA1(3c6b371efeda757f2bcab6c860e7585b628c210a))

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x40000, CRC(6673de85) SHA1(df390cd6268efc0e743a9020f19bc0cbeb757cfa))

	ROM_REGION( 0x280000, "gfx", 0 )
	ROM_LOAD( "4.bin", 0x000000, 0x80000, CRC(efcddac9) SHA1(72435ec478b70a067d47f3daf7c224169ee5827a))
	ROM_LOAD( "5.bin", 0x080000, 0x80000, CRC(bc75ef8f) SHA1(1f3dc457e5ae143d53cfef0e1fcb4586dceefb67))
	ROM_LOAD( "6.bin", 0x100000, 0x80000, CRC(e47d5f55) SHA1(a341e24f98125265cb3986f8c7ce84eedd056b71))
	ROM_LOAD( "7.bin", 0x180000, 0x80000, CRC(0fa6ce7d) SHA1(5ba96c9c0625a131d890d9c0c0f65cb2a03fa084))
	ROM_LOAD( "8.bin", 0x200000, 0x80000, CRC(d4c2b7da) SHA1(515be861443acc5b911241dbaafa42e02f79985a))
ROM_END

/*
Lucky Boom (c)1996 Playmark

QTY     Type                    clock           position    function
1x      MC68000P12                              u24         16/32-bit Microprocessor - main
1x      M6295                                   u146        4-Channel Mixing ADCPM Voice Synthesis LSI - sound
1x      KA358                                   u155        Dual Operational Amplifier - sound
1x      TDA2003                                 TD1         Audio Amplifier - sound
1x      oscillator              24.000000MHz    XL1
1x      oscillator              28.000000MHz    XL2
1x      blu resonator   1000J                   Y1

ROMs
QTY     Type        position    status
1x      TMS27C020   1           dumped
2x      AM27C010    2,3         dumped
5x      TMS27C010   4-8         dumped

RAMs
QTY     Type            position
8x      HM3-65728BK-5   u42,u73,u74,u75,u80,u81,u124,u125
2x      HY62256ALP-10   u2,u6
2x      KM6264BL-7L     u36,u37
PLDs
QTY     Type                position            status
2x      A1020B-PL84C        u110,u137           read protected
4x      TIBPAL22V10ACNT     u1,u77,u111,u112    read protected
*/



ROM_START( luckboom )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "2.u16", 0x00000, 0x20000, CRC(0a20eaca) SHA1(edd325b60b3a3ce84e89f8be91b9e19f82d15317) )
	ROM_LOAD16_BYTE( "3.u15", 0x00001, 0x20000, CRC(0d3bb24c) SHA1(31826ec29650aa8b70e1b713678401113e008832) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "1.u147", 0x00000, 0x40000, CRC(0d42c0a3) SHA1(1b1d4c7dcbb063e8bf133063770b753947d1a017) )

	ROM_REGION( 0xa0000, "gfx", 0 )
	ROM_LOAD( "4.u141", 0x000000, 0x20000, CRC(3aeccad7) SHA1(4104dd3ae928c1a876ac9b460fe18b58ce1206f7) )
	ROM_LOAD( "5.u142", 0x020000, 0x20000, CRC(4e4f9ac6) SHA1(e9b0c48195d8b21cdab16d66423ff43e6292ef2c) )
	ROM_LOAD( "6.u143", 0x040000, 0x20000, CRC(d1b4910e) SHA1(cfcb14bcd992bceaa634b20eb2d446caa62b6d82) )
	ROM_LOAD( "7.u144", 0x060000, 0x20000, CRC(00334bad) SHA1(2d8af2a4b517ceb1ae5504b15086c8faaf934b53) )
	ROM_LOAD( "8.u145", 0x080000, 0x20000, CRC(dc12df50) SHA1(0796d6428dc3a032ea55e06e8e07245a256ed036) )
ROM_END


/*
ZW3 PCB with 'MAGIC' sticker
1x MC68000FN12
1x PIC16C65 (scratched)
1x M48Z02-150PC1 ZEROPOWER RAM
1x 12 MHz XTAL
1x 14.318180 MHz XTAL
3x Actel A1020B FPGA
1x Oki M6295
1x 1.000J blue resonator
*/

ROM_START( magictch )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "22.u43", 0x00000, 0x20000, CRC(47f047b1) SHA1(f47ab9734f6bb1dc50baf159bca144fa79eac1a5) ) // TMS27C010A
	ROM_LOAD16_BYTE( "23.u42", 0x00001, 0x20000, CRC(f63e31bf) SHA1(e96da519a8d6488d600e031ac48f5ce1a8a376f5) ) // TMS27C010A

	ROM_REGION( 0x4008, "pic16c65", 0 )
	ROM_LOAD( "pic16c65.u28", 0x0000, 0x4008, NO_DUMP )

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "21.u16", 0x00000, 0x40000, CRC(e06a023f) SHA1(b4cd64f6c97e9c3e50a9658e171d748cb9f1c4ef) ) // ST M27C2001, 1xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0xa0000, "gfx", 0 )
	ROM_LOAD( "28.u76", 0x000000, 0x20000, CRC(ca49b54c) SHA1(69d6a3b32ebc357231b22ded40468971ba9ef8c3) ) // TMS27C010A
	ROM_LOAD( "27.u77", 0x020000, 0x20000, CRC(55b10fe5) SHA1(caf16512afe1b3fa66018075598cbc2626a63b3e) ) // TMS27C010A
	ROM_LOAD( "26.u78", 0x040000, 0x20000, CRC(d7974bd9) SHA1(b49678697e30d104a88adcc8e2c09cd62233c7b4) ) // TMS27C010A
	ROM_LOAD( "25.u79", 0x060000, 0x20000, CRC(f825cb9d) SHA1(10ffa614ac82e5c625c36095b298a8acfc7465bf) ) // TMS27C010A
	ROM_LOAD( "24.u80", 0x080000, 0x20000, CRC(3bdaea12) SHA1(8d9493ab80f96e6f941039ce6d1b8f1ed3c78379) ) // TMS27C010A

	ROM_REGION( 0x300, "plds", 0)
	ROM_LOAD( "gal22cv10-15lnc.u40", 0x000, 0x2e5, NO_DUMP ) // soldered
ROM_END

ROM_START( croupierb ) // identical PCB to magictch, but with 'ROULETTE' sticker and a Dallas DS1220Y instead of the M48Z02
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "12.u43", 0x00000, 0x20000, CRC(fe6c95f6) SHA1(9a90e15753fab2304a05192202456a3ee7adbc38) ) // TMS27C010A
	ROM_LOAD16_BYTE( "13.u42", 0x00001, 0x20000, CRC(9e76bd67) SHA1(19951f6a1201feecf8caa79bff6b46508db0f999) ) // TMS27C010A

	ROM_REGION( 0x4008, "pic16c65", 0 )
	ROM_LOAD( "pic16c65.u28", 0x0000, 0x4008, NO_DUMP )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "1.u16", 0x00000, 0x40000, CRC(6673de85) SHA1(df390cd6268efc0e743a9020f19bc0cbeb757cfa) ) // TMS27C020, same as other croupier sets

	ROM_REGION( 0x280000, "gfx", 0 )
	ROM_LOAD( "8.u76", 0x000000, 0x80000, CRC(d3d1343b) SHA1(7ac8d84567920f9f0ad8cfd406de1eb39b65b93e) ) // MX27C4000DC
	ROM_LOAD( "7.u77", 0x080000, 0x80000, CRC(c6563b8a) SHA1(6bb05ca006efc8cccf2f8eb6f696d0116a0e8302) ) // MX27C4000DC
	ROM_LOAD( "6.u78", 0x100000, 0x80000, CRC(e1e181c2) SHA1(f60d3df5fd9c2841e4f6dc656f4ac3cab5c404c7) ) // MX27C4000DC
	ROM_LOAD( "5.u79", 0x180000, 0x80000, CRC(2c59e118) SHA1(dffbd47f18276a11e964ec4a4bd3ff81d21e2a76) ) // MX27C4000DC
	ROM_LOAD( "4.u80", 0x200000, 0x80000, CRC(f48c59f3) SHA1(5fd0ec940a50ec923790597723785cca10e81479) ) // MX27C4000DC

	ROM_REGION( 0x300, "plds", 0)
	ROM_LOAD( "gal22cv10-15lnc.u40", 0x000, 0x2e5, NO_DUMP ) // soldered
ROM_END

ROM_START( tropfrt ) // identical PCB to magictch, but with 'TROPICAL' sticker and not scratched PIC16C65
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "9.u43",  0x00000, 0x20000, CRC(25c675c7) SHA1(ba89fec39028d96c88a22bed71c82361d50eef12) ) // M27C1001
	ROM_LOAD16_BYTE( "10.u42", 0x00001, 0x20000, CRC(cd1a533a) SHA1(c611cc26239309d8ea21724f20ed88e8cf076fc5) ) // M27C1001

	ROM_REGION( 0x4008, "pic16c65", 0 )
	ROM_LOAD( "pic16c65.u28", 0x0000, 0x4008, NO_DUMP )

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "16.u16", 0x00000, 0x40000, CRC(85acb618) SHA1(b7f1cb288bf155cebd8aa47286d1147b538b27e6) ) // M27C2001, 1xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0xa0000, "gfx", 0 )
	ROM_LOAD( "11.u76", 0x000000, 0x20000, CRC(02b8ab49) SHA1(3f63af482ac582c90ea89e9d5c54ac87ff7c1d76) ) // M27C1001
	ROM_LOAD( "12.u77", 0x020000, 0x20000, CRC(d7e39de0) SHA1(acb5204d611f8a7305d7261700b04093800a3463) ) // M27C1001
	ROM_LOAD( "13.u78", 0x040000, 0x20000, CRC(e0b9fbeb) SHA1(8c2cc0f28e6f61eca79aa5957f294219d2d95694) ) // M27C1001
	ROM_LOAD( "14.u79", 0x060000, 0x20000, CRC(5909db4d) SHA1(5258586bd16e31cc20576a68fd701fbcf7c4a53c) ) // M27C1001
	ROM_LOAD( "15.u80", 0x080000, 0x20000, CRC(46f09c0e) SHA1(1324ce512a5e617bcd47850f5ffcbd85ff1c3e61) ) // M27C1001

	ROM_REGION( 0x800, "nvram", 0)
	ROM_LOAD( "nvram", 0x000, 0x800, CRC(8278a06f) SHA1(a1e6905250102d8be4be681123585b8e131a4ffd) ) // pre-initialized (game doesn't accept coins without initializing)

	ROM_REGION( 0x300, "plds", 0)
	ROM_LOAD( "gal22v10b-25lp.u40", 0x000, 0x2e5, NO_DUMP ) // soldered
ROM_END

} // anonymous namespace


/******************************
*        Game Drivers         *
******************************/

//     YEAR  NAME       PARENT    MACHINE   INPUT      CLASS         INIT        ROT   COMPANY     FULLNAME                                   FLAGS                                                LAYOUT
GAMEL( 1996, sderby,    0,        sderby,   sderby,    sderby_state, empty_init, ROT0, "Playmark", "Super Derby (Playmark, v.07.03)",         0,                                                   layout_sderby   )
GAMEL( 1996, sderbya,   sderby,   sderbya,  sderbya,   sderby_state, empty_init, ROT0, "Playmark", "Super Derby (Playmark, v.10.04)",         0,                                                   layout_sderby   )
GAMEL( 1996, spacewin,  0,        spacewin, spacewin,  sderby_state, empty_init, ROT0, "Playmark", "Scacco Matto / Space Win",                0,                                                   layout_spacewin )
GAME(  1996, shinygld,  0,        shinygld, shinygld,  sderby_state, empty_init, ROT0, "Playmark", "Shiny Golds",                             0                                                                    )
GAMEL( 1997, croupier,  0,        pmroulet, pmroulet,  sderby_state, empty_init, ROT0, "Playmark", "Croupier (Playmark Roulette v.20.05)",    MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING, layout_pmroulet )
GAMEL( 1997, croupiera, croupier, pmroulet, pmroulet,  sderby_state, empty_init, ROT0, "Playmark", "Croupier (Playmark Roulette v.09.04)",    MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING, layout_pmroulet )
GAMEL( 1997, croupierb, croupier, zw3,      croupierb, zw3_state,    empty_init, ROT0, "Playmark", "Croupier II (Playmark Roulette v.03.09)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS, layout_pmroulet ) // title screen says Croupier 2 but every string in ROM says Croupier.
GAME(  1996, luckboom,  0,        luckboom, luckboom,  sderby_state, empty_init, ROT0, "Playmark", "Lucky Boom",                              0                                                                    )
GAME(  1998, magictch,  0,        zw3,      magictch,  zw3_state,    empty_init, ROT0, "Playmark", "Magic Touch",                             MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // sprite offsets aren't 100% correct, no PIC16C65 emulation, needs proper layout
GAME(  1999, tropfrt,   0,        zw3,      tropfrt,   zw3_state,    empty_init, ROT0, "Playmark", "Tropical Fruits (V. 24-06.00 Rev. 4.0)",  MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // sprite offsets aren't 100% correct, no PIC16C74 emulation, needs proper layout
