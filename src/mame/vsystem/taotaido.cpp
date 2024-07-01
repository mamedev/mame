// license:BSD-3-Clause
// copyright-holders:David Haywood

/***************************************************************************

  Tao Taido             (c) 1993 Video System


    driver by David Haywood - Dip Switches and Inputs by stephh
    based on other Video System drivers

Stephh's notes (based on the games M68000 code and some tests) :

0) all games

  - Don't trust the test mode ! It shows inputs for 4 players as well as
    3 buttons for each player, while the game is a 2 players game with only
    2 buttons (punch and kick) for each player.
    IMO, it's a leftover from a previous game.
    If you want to test all inputs, turn TAOTAIDO_SHOW_ALL_INPUTS to 1.

  - The "Buy-In" features allows a player to recover some energy and reset timer
    by pressing his "Start" button (provided he has at least one credit).

  - 'taotaido' allows to play a 2 players game with a single credit while
    this isn't possible with 'taotaida'.
    Now, telling which version is the newest one is another story ;)

  - 'taotaido' seems to show you how to do the special moves, 'taotaida' doesn't
    and they don't seem to work in the same way (unless this is a bug)

  - Coin buttons act differently depending on the "Coin Slot" Dip Switch :

      * "Coin Slot" Dip Switch set to "Same" :

          . COIN1 : adds coin(s)/credit(s) depending on "Coinage" Dip Switch
          . COIN2 : adds 1 credit
          . SERVICE1 : adds coin(s)/credit(s) depending on "Coinage" Dip Switch

      * "Coin Slot" Dip Switch set to "Individual" :

          . COIN1 : adds coin(s)/credit(s) for player 1 depending on "Coinage" Dip Switch
          . COIN2 : adds coin(s)/credit(s) for player 2 depending on "Coinage" Dip Switch
          . SERVICE1 : adds 1 credit for player 1

***************************************************************************/

/* Tao Taido
(c)1993 Video System

CPU:    68000-16
Sound:  Z80-B
        YM2610
OSC:    14.31818MHz
        20.0000MHz
        32.0000MHz
Chips:  VS9108
        VS920B
        VS9209 x2

****************************************************************************

TODO: zooming might be wrong (only used on title logo?)

***************************************************************************/

#include "emu.h"

#include "vs9209.h"
#include "vsystem_spr.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/mb3773.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_TILEREG (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_TILEREG)

#include "logmacro.h"

#define LOGTILEREG(...) LOGMASKED(LOG_TILEREG, __VA_ARGS__)


namespace {

#define TAOTAIDO_SHOW_ALL_INPUTS    0

class taotaido_state : public driver_device
{
public:
	taotaido_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_spr(*this, "vsystem_spr"),
		m_soundlatch(*this, "soundlatch"),
		m_watchdog(*this, "watchdog"),
		m_spriteram(*this, "spriteram%u", 1U),
		m_scrollram(*this, "scrollram"),
		m_bgram(*this, "bgram"),
		m_soundbank(*this, "soundbank"),
		m_spritebank(*this, "spritebank", 0x08, ENDIANNESS_BIG),
		m_spriteram_old(*this, "spriteram_old", 0x2000U, ENDIANNESS_BIG),
		m_spriteram2_old(*this, "spriteram2_old", 0x10000U, ENDIANNESS_BIG),
		m_spriteram_older(*this, "spriteram_older", 0x2000U, ENDIANNESS_BIG),
		m_spriteram2_older(*this, "spriteram2_older", 0x10000U, ENDIANNESS_BIG)
	{ }

	void taotaido(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<vsystem_spr_device> m_spr;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<mb3773_device> m_watchdog;

	required_shared_ptr_array<uint16_t, 2> m_spriteram;
	required_shared_ptr<uint16_t> m_scrollram;
	required_shared_ptr<uint16_t> m_bgram;

	required_memory_bank m_soundbank;

	memory_share_creator<uint8_t> m_spritebank;
	memory_share_creator<uint16_t> m_spriteram_old;
	memory_share_creator<uint16_t> m_spriteram2_old;
	memory_share_creator<uint16_t> m_spriteram_older;
	memory_share_creator<uint16_t> m_spriteram2_older;

	uint8_t m_bgbank[8]{};
	tilemap_t *m_bg_tilemap = nullptr;

	uint16_t soundlatch_pending_r();
	void soundlatch_pending_w(int state);
	void unknown_output_w(uint8_t data);
	void sh_bankswitch_w(uint8_t data);
	void spritebank_w(offs_t offset, uint8_t data);
	void tileregs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bgvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(bg_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	uint32_t tile_callback(uint32_t code);
	void main_map(address_map &map);
	void sound_map(address_map &map);
	void sound_port_map(address_map &map);
};


// sprite tile codes 0x4000 - 0x7fff get remapped according to the content of these registers
void taotaido_state::spritebank_w(offs_t offset, uint8_t data)
{
	m_spritebank[offset] = data;
}

// sprites are like the other Video System / Psikyo games, we can merge this with aerofgt and plenty of other things eventually


// the tilemap

void taotaido_state::tileregs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
		case 0: // would normally be x scroll?
		case 1: // would normally be y scroll?
		case 2:
		case 3:
			LOGTILEREG("unhanded tilemap register write offset %02x data %04x \n", offset, data);
			break;

		// tile banks
		case 4:
		case 5:
		case 6:
		case 7:
			if (ACCESSING_BITS_8_15)
				m_bgbank[(offset - 4) << 1] = data >> 8;
			if (ACCESSING_BITS_0_7)
				m_bgbank[((offset - 4) << 1) + 1] = data & 0xff;
			m_bg_tilemap->mark_all_dirty();
			break;
	}
}

void taotaido_state::bgvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bgram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(taotaido_state::bg_tile_info)
{
	int code = m_bgram[tile_index] & 0x01ff;
	int const bank = (m_bgram[tile_index] & 0x0e00) >> 9;
	int const col = (m_bgram[tile_index] & 0xf000) >> 12;

	code |= m_bgbank[bank] << 9;

	tileinfo.set(0, code, col, 0);
}

TILEMAP_MAPPER_MEMBER(taotaido_state::tilemap_scan_rows)
{
	// logical (col,row) -> memory offset
	return (col & 0x3f) | ((row & 0x3f) << 6) | ((col & 0x40) << 6);
}


uint32_t taotaido_state::tile_callback(uint32_t code)
{
	code = m_spriteram2_older[code & 0x7fff];

	if (code > 0x3fff)
	{
		int const block = (code & 0x3800) >> 11;
		code &= 0x07ff;
		code |= m_spritebank[block] << 11;
	}

	return code;
}


void taotaido_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taotaido_state::bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(taotaido_state::tilemap_scan_rows)), 16, 16, 128, 64);

	save_item(NAME(m_bgbank));
}


uint32_t taotaido_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  m_bg_tilemap->set_scrollx(0,(m_scrollram[0x380 / 2] >> 4)); // the values put here end up being wrong every other frame
//  m_bg_tilemap->set_scrolly(0,(m_scrollram[0x382 / 2] >> 4)); // the values put here end up being wrong every other frame

	// not amazingly efficient however it should be functional for row select and linescroll
	rectangle clip = cliprect;

	for (int line = cliprect.top(); line <= cliprect.bottom(); line++)
	{
		clip.min_y = clip.max_y = line;

		m_bg_tilemap->set_scrollx(0, ((m_scrollram[(0x00 + 4 * line) / 2]) >> 4) + 30);
		m_bg_tilemap->set_scrolly(0, ((m_scrollram[(0x02 + 4 * line) / 2]) >> 4) - line);

		m_bg_tilemap->draw(screen, bitmap, clip, 0, 0);
	}

	m_spr->draw_sprites(m_spriteram_older.target(), m_spriteram[0].bytes(), screen, bitmap, cliprect);
	return 0;
}

void taotaido_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		// sprites need to be delayed by 2 frames?

		memcpy(m_spriteram2_older.target(), m_spriteram2_old.target(), 0x10000);
		memcpy(m_spriteram2_old.target(), m_spriteram[1], 0x10000);

		memcpy(m_spriteram_older.target(), m_spriteram_old.target(), 0x2000);
		memcpy(m_spriteram_old.target(), m_spriteram[0], 0x2000);
	}
}


void taotaido_state::machine_start()
{
	m_soundbank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x8000);
}


uint16_t taotaido_state::soundlatch_pending_r()
{
	// Only bit 0 is tested
	return m_soundlatch->pending_r();
}

void taotaido_state::soundlatch_pending_w(int state)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);

	// sound comms is 2-way (see soundlatch_pending_r),
	// NMI routine is very short, so briefly set perfect_quantum to make sure that the timing is right
	if (state)
		machine().scheduler().perfect_quantum(attotime::from_usec(100));
}

void taotaido_state::unknown_output_w(uint8_t data)
{
	m_watchdog->write_line_ck(BIT(data, 7));

	// Bits 5, 4 also used?
}

void taotaido_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x800000, 0x803fff).ram().w(FUNC(taotaido_state::bgvideoram_w)).share(m_bgram);
	map(0xa00000, 0xa01fff).ram().share(m_spriteram[0]);
	map(0xc00000, 0xc0ffff).ram().share(m_spriteram[1]); // sprite tile lookup RAM
	map(0xfe0000, 0xfeffff).ram(); // main RAM
	map(0xffc000, 0xffcfff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0xffe000, 0xffe3ff).ram().share(m_scrollram); // rowscroll / rowselect / scroll RAM
	map(0xffff80, 0xffff9f).rw("io1", FUNC(vs9209_device::read), FUNC(vs9209_device::write)).umask16(0x00ff);
	map(0xffffa0, 0xffffbf).rw("io2", FUNC(vs9209_device::read), FUNC(vs9209_device::write)).umask16(0x00ff);
	map(0xffff00, 0xffff0f).w(FUNC(taotaido_state::tileregs_w));
	map(0xffff10, 0xffff11).nopw(); // unknown
	map(0xffff20, 0xffff21).nopw(); // unknown - flip screen related
	map(0xffff40, 0xffff47).w(FUNC(taotaido_state::spritebank_w));
	map(0xffffc1, 0xffffc1).w(m_soundlatch, FUNC(generic_latch_8_device::write)); // seems right
	map(0xffffe0, 0xffffe1).r(FUNC(taotaido_state::soundlatch_pending_r)); // guess - seems to be needed for all the sounds to work
}

// sound CPU - same as aerofgt


void taotaido_state::sh_bankswitch_w(uint8_t data)
{
	m_soundbank->set_entry(data & 0x03);
}

void taotaido_state::sound_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_soundbank);
}

void taotaido_state::sound_port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0x04, 0x04).w(FUNC(taotaido_state::sh_bankswitch_w));
	map(0x08, 0x08).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
	map(0x0c, 0x0c).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( taotaido )
	PORT_START("P1")    // 0xffff81.b
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // "Punch"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // "Kick"
#if TAOTAIDO_SHOW_ALL_INPUTS
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
#else
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
#endif
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")    // 0xffff83.b
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // "Punch"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // "Kick"
#if TAOTAIDO_SHOW_ALL_INPUTS
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
#else
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
#endif
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// These inputs are only to fit the test mode - leftover from another game ?
	PORT_START("P3")    // 0xffffa1.b
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
#if TAOTAIDO_SHOW_ALL_INPUTS
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
#endif

	PORT_START("P4")    // 0xffffa3.b
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
#if TAOTAIDO_SHOW_ALL_INPUTS
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
#endif

	PORT_START("SYSTEM")    // 0xffff85.b
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )  // see notes
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
#if TAOTAIDO_SHOW_ALL_INPUTS
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
#else
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )    // "Test" in "test mode"
#endif
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )       // not working ?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )   // see notes - SERVICE in "test mode"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // VBLANK ? The game freezes when ON

	PORT_START("DSW1")  // 0xffff87.b -> !0xfe2f6c.w or !0xfe30d0
	PORT_DIPNAME( 0x01, 0x01, "Coin Slot" )
	PORT_DIPSETTING(    0x01, "Same" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, "2 Coins to Start, 1 to Continue" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  // 0xffff89.b -> !0xfe73c2.w or !0xfe751c.w
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  // check code at 0x0963e2 or 0x845e2
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW3")  // 0xffff8b.b -> !0xfe2f94.w or !0xfe30f8.w
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  // doesn't seem to be demo sounds
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Buy In" )            // see notes
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("JP")    // Jumpers (0xffff8f.b)
	PORT_DIPNAME( 0x0f, 0x08, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x00, DEF_STR( USA ) )              // also (c) Mc O'River Inc
	PORT_DIPSETTING(    0x01, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x02, "Hong Kong/Taiwan" )
//  PORT_DIPSETTING(    0x03, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Korea ) )
//  PORT_DIPSETTING(    0x05, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x06, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x07, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x08, DEF_STR( World ) )
	/* 0x09 to 0x0f : DEF_STR( Japan ) */
INPUT_PORTS_END

static INPUT_PORTS_START( taotaido3 )
	PORT_INCLUDE(taotaido)

	PORT_MODIFY("P1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x04, 0x04, "Number of Buttons" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
INPUT_PORTS_END

static INPUT_PORTS_START( taotaido6 )
	PORT_INCLUDE(taotaido)

	PORT_MODIFY("P1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x08, 0x08, "Debug Info 1" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Debug Info 2" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Debug Info 3" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Debug Info 4" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_taotaido )
	GFXDECODE_ENTRY( "bgtiles", 0, gfx_16x16x4_packed_lsb, 0x300, 256 )
GFXDECODE_END

static GFXDECODE_START( gfx_taotaido_spr )
	GFXDECODE_ENTRY( "sprites", 0, gfx_16x16x4_packed_lsb, 0x000, 256 )
GFXDECODE_END



void taotaido_state::taotaido(machine_config &config)
{
	M68000(config, m_maincpu, 32'000'000 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &taotaido_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(taotaido_state::irq1_line_hold));

	Z80(config, m_audiocpu, 20'000'000 / 4); // ??
	m_audiocpu->set_addrmap(AS_PROGRAM, &taotaido_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &taotaido_state::sound_port_map); // IRQs are triggered by the YM2610

	vs9209_device &io1(VS9209(config, "io1", 0));
	io1.porta_input_cb().set_ioport("P1");
	io1.portb_input_cb().set_ioport("P2");
	io1.portc_input_cb().set_ioport("SYSTEM");
	io1.portd_input_cb().set_ioport("DSW1");
	io1.porte_input_cb().set_ioport("DSW2");
	io1.portf_input_cb().set_ioport("DSW3");
	io1.portg_output_cb().set(FUNC(taotaido_state::unknown_output_w));
	io1.porth_input_cb().set_ioport("JP");

	vs9209_device &io2(VS9209(config, "io2", 0));
	io2.porta_input_cb().set_ioport("P3"); // used only by taotaida
	io2.portb_input_cb().set_ioport("P4"); // used only by taotaida

	MB3773(config, m_watchdog, 0);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_taotaido);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(taotaido_state::screen_update));
	screen.screen_vblank().set(FUNC(taotaido_state::screen_vblank));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x800);

	VSYSTEM_SPR(config, m_spr, 0, "palette", gfx_taotaido_spr);
	m_spr->set_tile_indirect_cb(FUNC(taotaido_state::tile_callback));

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(FUNC(taotaido_state::soundlatch_pending_w));
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 8'000'000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}


ROM_START( taotaido )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "1-u90.bin", 0x00000, 0x80000, CRC(a3ee30da) SHA1(920a83ce9192bf785bffdc041e280f1a420de4c9) )
	ROM_LOAD16_WORD_SWAP( "2-u91.bin", 0x80000, 0x80000, CRC(30b7e4fb) SHA1(15e1f6d252c736fdee33b691a0a1a45f0307bffb) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "3-u113.bin", 0x000000, 0x20000, CRC(a167c4e4) SHA1(d32184e7040935cd440d4d82c66491b710ec87a8) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "u127.bin", 0x00000, 0x200000, CRC(0cf0cb23) SHA1(a87e7159db2fa0d50446cbf45ec9fbf585b8f396) )

	ROM_REGION( 0x100000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "u104.bin", 0x000000, 0x100000, CRC(e89387a9) SHA1(1deeee056af367d1a5aa0722dd3d6c68a82d0489) )

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD( "u86.bin", 0x000000, 0x200000, CRC(908e251e) SHA1(5a135787f3263bfb195f8fd1e814c580d840531f) )
	ROM_LOAD( "u87.bin", 0x200000, 0x200000, CRC(c4290ba6) SHA1(4132ffad4668f1dd3f708f009e18435e7dd60120) )
	ROM_LOAD( "u88.bin", 0x400000, 0x200000, CRC(407d9aeb) SHA1(d532c7b80f6c192dba86542fb6eb3ef24fbbbdb9) )

	ROM_REGION( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "u15.bin", 0x000000, 0x200000, CRC(e95823e9) SHA1(362583944ad4fdde4f9e29928cf34376c7ad931f) )
ROM_END

ROM_START( taotaidoa )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "tt0-u90.bin", 0x00000, 0x80000, CRC(69d4cca7) SHA1(f1aba74fef8fe4271d19763f428fc0e2674d08b3) )
	ROM_LOAD16_WORD_SWAP( "tt1-u91.bin", 0x80000, 0x80000, CRC(41025469) SHA1(fa3a424ca3ecb513f418e436e4191ff76f6a0de1) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "3-u113.bin", 0x000000, 0x20000, CRC(a167c4e4) SHA1(d32184e7040935cd440d4d82c66491b710ec87a8) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "u127.bin", 0x00000, 0x200000, CRC(0cf0cb23) SHA1(a87e7159db2fa0d50446cbf45ec9fbf585b8f396) )

	ROM_REGION( 0x100000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "u104.bin", 0x000000, 0x100000, CRC(e89387a9) SHA1(1deeee056af367d1a5aa0722dd3d6c68a82d0489) )

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD( "u86.bin", 0x000000, 0x200000, CRC(908e251e) SHA1(5a135787f3263bfb195f8fd1e814c580d840531f) )
	ROM_LOAD( "u87.bin", 0x200000, 0x200000, CRC(c4290ba6) SHA1(4132ffad4668f1dd3f708f009e18435e7dd60120) )
	ROM_LOAD( "u88.bin", 0x400000, 0x200000, CRC(407d9aeb) SHA1(d532c7b80f6c192dba86542fb6eb3ef24fbbbdb9) )

	ROM_REGION( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "u15.bin", 0x000000, 0x200000, CRC(e95823e9) SHA1(362583944ad4fdde4f9e29928cf34376c7ad931f) )
ROM_END

ROM_START( taotaido3 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "1.u90", 0x00000, 0x80000, CRC(27c5c626) SHA1(f2aea45a15db24c914aa889be21cd8994d138a59) )
	ROM_LOAD16_WORD_SWAP( "2.u91", 0x80000, 0x80000, CRC(71a4e538) SHA1(608c2d77aa8c1c4bb39a419bdfdf73a2fd587403) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "3-u113.bin", 0x000000, 0x20000, CRC(a167c4e4) SHA1(d32184e7040935cd440d4d82c66491b710ec87a8) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "u127.bin", 0x00000, 0x200000, CRC(0cf0cb23) SHA1(a87e7159db2fa0d50446cbf45ec9fbf585b8f396) )

	ROM_REGION( 0x100000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "u104.bin", 0x000000, 0x100000, CRC(e89387a9) SHA1(1deeee056af367d1a5aa0722dd3d6c68a82d0489) )

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD( "u86.bin", 0x000000, 0x200000, CRC(908e251e) SHA1(5a135787f3263bfb195f8fd1e814c580d840531f) )
	ROM_LOAD( "u87.bin", 0x200000, 0x200000, CRC(c4290ba6) SHA1(4132ffad4668f1dd3f708f009e18435e7dd60120) )
	ROM_LOAD( "u88.bin", 0x400000, 0x200000, CRC(407d9aeb) SHA1(d532c7b80f6c192dba86542fb6eb3ef24fbbbdb9) )

	ROM_REGION( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "u15.bin", 0x000000, 0x200000, CRC(e95823e9) SHA1(362583944ad4fdde4f9e29928cf34376c7ad931f) )
ROM_END

} // anonymous namespace


GAME( 1993, taotaido,  0,        taotaido, taotaido,  taotaido_state, empty_init, ROT0, "Video System Co.", "Tao Taido (2 button version)",   MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, taotaidoa, taotaido, taotaido, taotaido6, taotaido_state, empty_init, ROT0, "Video System Co.", "Tao Taido (6 button version)",   MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // maybe a prototype? has various debug features
GAME( 1993, taotaido3, taotaido, taotaido, taotaido3, taotaido_state, empty_init, ROT0, "Video System Co.", "Tao Taido (2/3 button version)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
