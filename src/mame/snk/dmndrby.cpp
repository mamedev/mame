// license:BSD-3-Clause
// copyright-holders: Angelo Salese, David Haywood, Mike Green
/**************************************************************************************************

Diamond Derby - G4001 board (c) 1986 SNK / Electrocoin

Notes:
- Press Collect Button to "get the money";
- dmndrbybl: hold together SERVICE1 + P1 1-2 to move on from input test

TODO:
- Fix colors (check bar test on the first Service Mode menu);
- dmndrbybl: identify remaining inputs (game seems based over an original set we don't have yet);
- Enters into Service Mode (?) if you let it go in attract mode after some time (cannot reproduce);
- When booted up game disallows coin inputs (including setting with lockout disabled) for a couple timer seconds. Is this intentional?

===================================================================================================

G4001
Diamond Derby - Electrocoin on an SNK board

SWP gambling game base on horse racing

SNK/Electrocoin 1986
Re-released in 1994 (see dderbya) for changed UK gaming rules.

--------------------------------------------
G4001UP01

SWA SWB   C1        DD1
                    DD2

                6116              DD4
                          Z80     DD5
                                  DD6
                          DD3     6116
             8910  Z80    6116    6116

---------------------------------------------
G4001UP02

DD7  DD11  DD15           K1
DD8  DD12  DD16                   DD19
DD9  DD13  DD17                   DD20
DD10 DD14  DD18     H5            DD21


                                  DD22
                                  DD23
 2114                  2148 2148
 2114              H10

**************************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class dmndrby_state : public driver_device
{
public:
	dmndrby_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_scroll_ram(*this, "scroll_ram")
		, m_sprite_ram(*this, "sprite_ram")
		, m_fix_vram(*this, "fix_vram")
		, m_fix_attr(*this, "fix_attr")
		, m_racetrack_tilemap_rom(*this, "racetrack_data")
	{ }

	void dderby(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

	virtual void main_map(address_map &map) ATTR_COLD;
	virtual void ca00_w(offs_t offset, u8 data);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_scroll_ram;
	required_shared_ptr<uint8_t> m_sprite_ram;
	required_shared_ptr<uint8_t> m_fix_vram;
	required_shared_ptr<uint8_t> m_fix_attr;
	required_region_ptr<uint8_t> m_racetrack_tilemap_rom;

	tilemap_t *m_fix_tilemap = nullptr;
	tilemap_t *m_racetrack_tilemap = nullptr;
	TILE_GET_INFO_MEMBER(get_fix_tile_info);
	TILEMAP_MAPPER_MEMBER(racetrack_scan_rows);
	TILE_GET_INFO_MEMBER(get_racetrack_tile_info);
	void fix_vram_w(offs_t offset, u8 data);
	void fix_attr_w(offs_t offset, u8 data);

	void palette_init(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

	void output_w(offs_t offset, uint8_t data);

	void sound_map(address_map &map) ATTR_COLD;
};

class dmndrby_quinella_state : public dmndrby_state
{
public:
	dmndrby_quinella_state(const machine_config &mconfig, device_type type, const char *tag)
		: dmndrby_state(mconfig, type, tag)
	{ }

protected:
	virtual void main_map(address_map &map) override ATTR_COLD;
private:
	// FIXME: should really use IPT_OUTPUT instead of subclassing
	virtual void ca00_w(offs_t offset, u8 data) override;
};


/******************
 *
 * Video
 *
 *****************/

// TODO: incorrect resistors, reference shows gravel track with more charged red
void dmndrby_state::palette_init(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 470, 0,
			3, &resistances_rg[0], gweights, 470, 0,
			2, &resistances_b[0],  bweights, 470, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component */
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom = memregion("proms2")->base();

	// normal tiles use colors 0-15
	for (int i = 0x000; i < 0x300; i++)
	{
		uint8_t ctabentry = color_prom[i];
		palette.set_pen_indirect(i, ctabentry);
	}
}

void dmndrby_state::video_start()
{
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dmndrby_state::get_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_racetrack_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dmndrby_state::get_racetrack_tile_info)), tilemap_mapper_delegate(*this, FUNC(dmndrby_state::racetrack_scan_rows)), 16, 16, 128, 64);
	m_racetrack_tilemap->mark_all_dirty();

	m_fix_tilemap->set_transparent_pen(0);
	m_racetrack_tilemap->set_transparent_pen(0);
}

TILE_GET_INFO_MEMBER(dmndrby_state::get_fix_tile_info)
{
	u8 attr = m_fix_attr[tile_index];
	const u16 code = m_fix_vram[tile_index] | (BIT(m_fix_attr[tile_index], 5) << 8);
	const u8 color = attr & 0x1f;
	// dmndrbybl, in Cocktail mode
	int const flipxy = ((attr & 0x40) >> 6) * 3;

	tileinfo.set(0, code, color, TILE_FLIPYX(flipxy));
}

TILEMAP_MAPPER_MEMBER(dmndrby_state::racetrack_scan_rows)
{
	return (col & 0xf) | ((row & 0xf) << 4) | ((col & 0x70) << 4) | ((row & 0xf0) << 7);
}

TILE_GET_INFO_MEMBER(dmndrby_state::get_racetrack_tile_info)
{
	int const code = m_racetrack_tilemap_rom[tile_index];
	int const attr = m_racetrack_tilemap_rom[tile_index + 0x2000];

	int const col = attr & 0x1f;
	int const flipx = (attr & 0x40) >> 6;

	tileinfo.category = BIT(attr, 7);

	tileinfo.set(2, code, col, TILE_FLIPYX(flipx));
}

void dmndrby_state::fix_vram_w(offs_t offset, u8 data)
{
	m_fix_vram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}

void dmndrby_state::fix_attr_w(offs_t offset, u8 data)
{
	m_fix_attr[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}

uint32_t dmndrby_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  gfx_element *gfx = m_gfxdecode->gfx(0);
	gfx_element *sprites = m_gfxdecode->gfx(1);
	//gfx_element *track = m_gfxdecode->gfx(2);

	bitmap.fill(m_palette->black_pen(), cliprect);

	// NOTE: dmndrbybl service mode horse/scroll test items expects following two to be reversed
	// That won't work in game, buggy routine?
	const bool track_enable = BIT(m_scroll_ram[4], 2);
	const bool sprite_enable = BIT(m_scroll_ram[4], 1);
	const bool fix_enable = BIT(m_scroll_ram[4], 0);
	// Track layer has a narrower window drawing, there's no text tilemap attribute that would
	// suggest otherwise.
	rectangle overlay_rect;
	overlay_rect.set(cliprect.min_x, cliprect.max_x, std::max(40, cliprect.min_y), std::min(215, cliprect.max_y));

	m_racetrack_tilemap->set_scrollx(0, m_scroll_ram[0] + m_scroll_ram[1] * 0x100);
	m_racetrack_tilemap->set_scrolly(0, m_scroll_ram[2] + m_scroll_ram[3] * 0x100);

	if (track_enable)
	{
		m_racetrack_tilemap->draw(screen, bitmap, overlay_rect, TILEMAP_DRAW_CATEGORY(0) | TILEMAP_DRAW_OPAQUE, 0);
		m_racetrack_tilemap->draw(screen, bitmap, overlay_rect, TILEMAP_DRAW_CATEGORY(1) | TILEMAP_DRAW_OPAQUE, 0);
	}

/* draw sprites

 guess work! Seems to work fine and horse labels match up
wouldn't like to say it's the most effective way though...
 -- maybe they should be decoded as 'big sprites' instead?

*/
	if (sprite_enable)
	{
		for (int count = 5; count >= 0; count--)
		{
			int a = 0;
			int b = 0;
			int const base = count * 4;
			int const sprx = m_sprite_ram[base + 3];
			int const spry = m_sprite_ram[base + 2];
			//m_sprite_ram[base + 1];
			int const col = (m_sprite_ram[base + 1] & 0x1f);
			int const anim = (m_sprite_ram[base] & 0x3f) * 0x40; // animation frame
			int const horse = (m_sprite_ram[base + 1] & 0x7) * 8 + 7;  // horse label from 1 - 6

			for (a = 0; a < 8 ; a++)
				for(b = 0; b < 7; b++)
					sprites->transpen(bitmap, cliprect, anim + a * 8 + b, col, 0, 0, sprx + a * 8, spry + b * 8, 0);

			// draw the horse number
			a = 3;
			b = 3;
			sprites->transpen(bitmap, cliprect, anim + horse, col, 0, 0, sprx + a * 8, spry + b * 8, 0);
		}
	}

	// draw higher priority categories above sprites
	if (track_enable)
	{
		m_racetrack_tilemap->draw(screen, bitmap, overlay_rect, TILEMAP_DRAW_CATEGORY(1), 0);
	}

	if (fix_enable)
		m_fix_tilemap->draw(screen, bitmap, cliprect, track_enable ? 0 : TILEMAP_DRAW_OPAQUE, 0);

	return 0;
}

/******************
 *
 * I/O (stubs)
 *
 *****************/

void dmndrby_state::output_w(offs_t offset, uint8_t data)
{
	/*
	---- x--- refill meter [4]
	---- x--- token out meter [5]
	---- x--- token in meter [6]
	---- x--- cash out meter [7]
	---- -x-- coin out (meter) [0-3]
	---- -x-- coin lockout token [4]
	---- -x-- coin counter (meter) [5]
	---- --x- coin lockout [0-3]
	---- ---x lamp [0-6]
	*/
}

void dmndrby_state::ca00_w(offs_t offset, uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);
}

void dmndrby_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x8000, 0x8fff).ram().share("nvram");
	map(0xc000, 0xc000).portr("IN0");
	map(0xc001, 0xc001).portr("IN1");
	map(0xc002, 0xc002).portr("IN2");
	map(0xc003, 0xc003).portr("IN3");
	map(0xc004, 0xc004).portr("IN4");
	map(0xc005, 0xc005).portr("IN5");
	map(0xc006, 0xc006).portr("IN6");
	map(0xc007, 0xc007).portr("IN7");
	map(0xc000, 0xc007).w(FUNC(dmndrby_state::output_w));
	map(0xc802, 0xc802).portr("DSW1");
	map(0xc803, 0xc803).portr("DSW2");
	map(0xca00, 0xca00).w(FUNC(dmndrby_state::ca00_w));
	map(0xca01, 0xca01).nopw(); //watchdog
	map(0xca02, 0xca02).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xca03, 0xca03).w("soundlatch2", FUNC(generic_latch_8_device::write));
	map(0xcc00, 0xcc05).ram().share(m_scroll_ram);
	map(0xce08, 0xce1f).ram().share(m_sprite_ram); // horse sprites
	map(0xd000, 0xd3ff).ram().w(FUNC(dmndrby_state::fix_vram_w)).share(m_fix_vram);
	map(0xd400, 0xd7ff).ram().w(FUNC(dmndrby_state::fix_attr_w)).share(m_fix_attr);
}

void dmndrby_quinella_state::ca00_w(offs_t offset, u8 data)
{
	dmndrby_state::ca00_w(offset, data);
	// bit 7: coin counter bit 6: coin out counter bit 3: hopper motor 1, bit 2: hopper motor 2
	machine().bookkeeping().coin_counter_w(0, BIT(data, 7));
}

void dmndrby_quinella_state::main_map(address_map &map)
{
	dmndrby_state::main_map(map);
	map(0xc000, 0xc007).unmaprw();
	// different I/O ports
	map(0xc000, 0xc000).portr("IN0");
	map(0xc001, 0xc001).portr("IN1");
	map(0xc002, 0xc002).portr("IN2");
	map(0xc200, 0xc200).portr("IN3");
	map(0xc201, 0xc201).portr("IN4");
	map(0xc202, 0xc202).portr("IN5");
	map(0xc400, 0xc41f).nopw(); // 7 segs, in upper/lower digits (0F clears, [0] upper [1] lower)
	map(0xc600, 0xc61f).nopw(); // cocktail side 7 segs
	map(0xc800, 0xc800).portr("IN6");
	map(0xc801, 0xc801).portr("IN7");
//  map(0xca00, 0xca00)
}

void dmndrby_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x1000).ram(); //???
	map(0x4000, 0x4001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x4000, 0x4000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x4001, 0x4001).r("soundlatch2", FUNC(generic_latch_8_device::read));
	map(0x6000, 0x67ff).ram();
}

static INPUT_PORTS_START( dderby )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_DIPNAME( 0x0002, 0x0002, "Out Coin 1" )//out coin 1
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("BET Horse 5")  PORT_CODE(KEYCODE_B)
	PORT_BIT( 0xf4, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_DIPNAME( 0x0002, 0x0002, "Out Coin 2" )//out coin 2
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("BET Horse 6")  PORT_CODE(KEYCODE_N)
	PORT_BIT( 0xf4, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0002, 0x0002, "Out Coin 3" )//out coin 3
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Collect")  PORT_CODE(KEYCODE_2_PAD) //to get coins
	PORT_BIT( 0xf4, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Refill Key")  PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("BET Horse 4")  PORT_CODE(KEYCODE_V)
	PORT_BIT( 0xf5, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("BET Horse 3")  PORT_CODE(KEYCODE_C)
	PORT_BIT( 0xf5, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Back Door")  PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("BET Horse 2")  PORT_CODE(KEYCODE_X)
	PORT_BIT( 0xf5, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN7")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Token Coin")  PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("BET Horse 1")  PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0xf5, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, " Unknown 1-1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, " Unknown 1-2" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, "Max Prize" )
	PORT_DIPSETTING(    0x06, "240p (cash)" )
	PORT_DIPSETTING(    0x02, "300p (cash)" )
	PORT_DIPSETTING(    0x04, "480p (cash + tokens)" )
	PORT_DIPSETTING(    0x00, "600p (cash + tokens)" )
	PORT_DIPNAME( 0x30, 0x00, "Percentage Payout" )
	PORT_DIPSETTING(    0x00, "76%" )   PORT_CONDITION("DSW1", 0xc0, LESSTHAN, 0x80)
	PORT_DIPSETTING(    0x10, "80%" )   PORT_CONDITION("DSW1", 0xc0, LESSTHAN, 0x80)
	PORT_DIPSETTING(    0x20, "86%" )   PORT_CONDITION("DSW1", 0xc0, LESSTHAN, 0x80)
	PORT_DIPSETTING(    0x30, "88%" )   PORT_CONDITION("DSW1", 0xc0, LESSTHAN, 0x80)
	PORT_DIPSETTING(    0x00, "78%" )   PORT_CONDITION("DSW1", 0xc0, NOTLESSTHAN, 0x80)
	PORT_DIPSETTING(    0x10, "82%" )   PORT_CONDITION("DSW1", 0xc0, NOTLESSTHAN, 0x80)
	PORT_DIPSETTING(    0x20, "86%" )   PORT_CONDITION("DSW1", 0xc0, NOTLESSTHAN, 0x80)
	PORT_DIPSETTING(    0x30, "90%" )   PORT_CONDITION("DSW1", 0xc0, NOTLESSTHAN, 0x80)
	PORT_DIPNAME( 0xc0, 0x80, "Price Per Game" )
	PORT_DIPSETTING(    0x00, "2p" )
	PORT_DIPSETTING(    0x40, "5p" )
	PORT_DIPSETTING(    0x80, "10p" )
	PORT_DIPSETTING(    0xc0, "20p" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Show Results")
	PORT_DIPSETTING(    0x01, "Last Race" )
	PORT_DIPSETTING(    0x00, "Last 6 Races" )
	PORT_DIPNAME( 0x02, 0x02, " Unknown 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, " Unknown 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, " Unknown 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, " Unknown 2-5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, " Unknown 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, " Unknown 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, " Unknown 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dderbya )
	PORT_INCLUDE( dderby )
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Price Per Play")
	PORT_DIPSETTING(    0x01, "5p" )
	PORT_DIPSETTING(    0x00, "10p" )
	PORT_DIPNAME( 0x06, 0x02, "Max Prize" )
	PORT_DIPSETTING(    0x00, "400p (cash + tokens)" )
	PORT_DIPSETTING(    0x04, "300p (cash + tokens)" )
	PORT_DIPSETTING(    0x02, "200p (cash)" )
	PORT_DIPSETTING(    0x06, "150p (cash)" )
	PORT_DIPNAME( 0x18, 0x08, "Percentage" )
	PORT_DIPSETTING(    0x00, "76%" )
	PORT_DIPSETTING(    0x08, "80%" )
	PORT_DIPSETTING(    0x10, "84%" )
	PORT_DIPSETTING(    0x18, "88%" )
	PORT_DIPNAME( 0x20, 0x20, " Unknown 1-2" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, " Unknown 1-3" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, " Unknown 1-4" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

// TODO: no clear service mode, following is guesswork
static INPUT_PORTS_START( dderbybl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("1P 2-5") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("1P 2-4") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("1P 2-3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("1P 1-6") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("1P 1-5") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("1P 1-4") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("1P 1-3") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1P 1-2") PORT_CODE(KEYCODE_Q)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_NAME("1P 5-6") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("1P 4-6") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME("1P 4-5") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("1P 3-6") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("1P 3-5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("1P 3-4") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("1P 2-6") PORT_CODE(KEYCODE_F)

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("2P 2-5") PORT_CODE(KEYCODE_D) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("2P 2-4") PORT_CODE(KEYCODE_S) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("2P 2-3") PORT_CODE(KEYCODE_A) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("2P 1-6") PORT_CODE(KEYCODE_T) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("2P 1-5") PORT_CODE(KEYCODE_R) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("2P 1-4") PORT_CODE(KEYCODE_E) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("2P 1-3") PORT_CODE(KEYCODE_W) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("2P 1-2") PORT_CODE(KEYCODE_Q) PORT_COCKTAIL

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_NAME("2P 5-6") PORT_CODE(KEYCODE_V) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("2P 4-6") PORT_CODE(KEYCODE_C) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME("2P 4-5") PORT_CODE(KEYCODE_X) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("2P 3-6") PORT_CODE(KEYCODE_Z) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("2P 3-5") PORT_CODE(KEYCODE_H) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("2P 3-4") PORT_CODE(KEYCODE_G) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("2P 2-6") PORT_CODE(KEYCODE_F) PORT_COCKTAIL

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, "IN5" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN6")
	PORT_DIPNAME( 0x01, 0x01, "IN6" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Coin 1 Cocktail side")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME("Coin 2 Cocktail side")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN7")
	PORT_DIPNAME( 0x01, 0x01, "IN7" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 ) // augments 'meter' 10 at a time, 'credit up' in book-keeping


	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown DSW1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown DSW1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown DSW1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown DSW1-5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown DSW1-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown DSW1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown DSW1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Unknown DSW2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown DSW2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown DSW2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown DSW2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown DSW2-5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown DSW2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown DSW2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2),4,0 },
	{ 0, 1, 2, 3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,3),  // 256 sprites
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,  16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout tiles8x8_layout2 =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ 0,RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8},
	8*8,
};

static GFXDECODE_START( gfx_dmndrby )
	GFXDECODE_ENTRY( "tiles",  0, tiles8x8_layout, 32*16, 32 )
	GFXDECODE_ENTRY( "horses", 0, tiles8x8_layout2, 0, 8)
	GFXDECODE_ENTRY( "track", 0, tiles16x16_layout, 16*16, 32 )
GFXDECODE_END

// Main Z80 is IM0, HW-latched irqs
TIMER_DEVICE_CALLBACK_MEMBER(dmndrby_state::scanline_cb)
{
	int scanline = param;

	// vblank irq
	if(scanline == 224)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7); // Z80 - RST 10h

	// timer irq, will throw token error if fired at wrong time
	if(scanline == 32)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf); // Z80 - RST 08h
}

void dmndrby_state::dderby(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000);         // ? MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &dmndrby_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(dmndrby_state::scanline_cb), "screen", 0, 1);

	Z80(config, m_audiocpu, 4'000'000);  // verified on schematics
	m_audiocpu->set_addrmap(AS_PROGRAM, &dmndrby_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(dmndrby_state::irq0_line_hold), attotime::from_hz(60));

	config.set_maximum_quantum(attotime::from_hz(6000));
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(dmndrby_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dmndrby);
	PALETTE(config, m_palette, FUNC(dmndrby_state::palette_init), 0x300, 0x20);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");
	GENERIC_LATCH_8(config, "soundlatch2");

	AY8910(config, "ay1", 4'000'000 / 2).add_route(ALL_OUTPUTS, "mono", 0.35); // frequency guessed, tied with sound timer irq above
}


ROM_START( dmndrby )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dd04.m6", 0x00000, 0x02000, CRC(a3cfd28e) SHA1(7ba14876fa4409634a699e049bce3bc457522932) )
	ROM_LOAD( "dd05.m7", 0x02000, 0x02000, CRC(16f7ac0b) SHA1(030b8c2b294a0287f3aaf72424304fc191315888) )
	ROM_LOAD( "dd06.m8", 0x04000, 0x02000, CRC(914ba8f5) SHA1(d1b3f3d5d2625e42ea6cb5c777942cec7faea58e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dd03.j9", 0x00000, 0x01000, CRC(660f0cae) SHA1(b3b414e52342de877a5c20886a87002a63310a94) )

	ROM_REGION( 0x04000, "tiles", 0 )
	ROM_LOAD( "dd01.e1", 0x00000, 0x02000, CRC(2e120288) SHA1(0ea29aff07e956b19080f05bd18b427195694ce8) )
	ROM_LOAD( "dd02.e2", 0x02000, 0x02000, CRC(ca028c8c) SHA1(f882eea2191cf1f3ea57d49fd6862f98401555be) )

	ROM_REGION( 0x4000, "racetrack_data", 0 )
	ROM_LOAD( "dd22.n6", 0x00000, 0x02000, CRC(db6b13fc) SHA1(3415deb2ffa86679e4f8abb644b75963e5368ba0) )
	ROM_LOAD( "dd23.n7", 0x02000, 0x02000, CRC(595fdb9b) SHA1(133d227bb156be52337da974e37973b049722e49) )

	ROM_REGION( 0x18000, "horses", 0 ) // horse sprites (kinda)
	ROM_LOAD( "dd07.b1", 0x00000, 0x02000, CRC(207a534a) SHA1(ddbd292f79cc9fb7bd9f0ee9874da87909147789) )
	ROM_LOAD( "dd08.b2", 0x02000, 0x02000, CRC(f380e2c4) SHA1(860a6557ae8b81d310c353f88f9194e1ffd551ec) )
	ROM_LOAD( "dd09.b3", 0x04000, 0x02000, CRC(68ebf74c) SHA1(959ee6c4ce700cff86af39442063dc79b8f8913e) )
	ROM_LOAD( "dd10.b5", 0x06000, 0x01000, CRC(38b1568a) SHA1(f7e04db49708dfc8c8512026d3460af0f3fb6780) )
	ROM_LOAD( "dd11.d1", 0x08000, 0x02000, CRC(fe615561) SHA1(808f703d0ca1576feb78f21c380e4006dd634a9c) )
	ROM_LOAD( "dd12.d2", 0x0a000, 0x02000, CRC(4df63aae) SHA1(a0b224fb1157fc25c21f9f0664bb8385e94e5c77) )
	ROM_LOAD( "dd13.d4", 0x0c000, 0x02000, CRC(cace0dfc) SHA1(41902f3ee2fa18798e3b441ee18f7b953d977b93) )
	ROM_LOAD( "dd14.d5", 0x0e000, 0x01000, CRC(2c602cbe) SHA1(78ffe79e3f2c4a3e9c6adc8f4635ed1a93528dc8) )
	ROM_LOAD( "dd15.e1", 0x10000, 0x02000, CRC(2ce23b64) SHA1(5cbeabc015cb167c7fd485ab4d9f1329bc2e94b3) )
	ROM_LOAD( "dd16.e2", 0x12000, 0x02000, CRC(6af9796c) SHA1(4cd818d488ac85fd6f8732fdca80cc29db86d3f4) )
	ROM_LOAD( "dd17.e4", 0x14000, 0x02000, CRC(b451cde2) SHA1(1c7340cc39d9beca1640c88000112c898d3de941) )
	ROM_LOAD( "dd18.e5", 0x16000, 0x01000, CRC(56228aaf) SHA1(74e96ebefc1b69310b23e47a35affbb7cd7d9acc) )

	ROM_REGION( 0x6000, "track", 0 )
	ROM_LOAD( "dd19.n2", 0x00000, 0x02000, CRC(fd536051) SHA1(556dfe064eeb9be1db751410ec128385c463e706) )
	ROM_LOAD( "dd20.n3", 0x02000, 0x02000, CRC(1497e52f) SHA1(f08c20c97c8d2148fcc705297cf1129bc65c9b83) )
	ROM_LOAD( "dd21.n4", 0x04000, 0x02000, CRC(87605d44) SHA1(c985fb15eac7bcc89e92909cf588b5982d0cabd0) )

	ROM_REGION( 0x020, "proms", 0 ) // palette
	ROM_LOAD( "ddprom3.h5", 0x0000, 0x0020, CRC(aea3cff6) SHA1(1139dd52c127436873a674be8a14527190091a82) )

	ROM_REGION( 0x300, "proms2", 0 ) // lookup
	ROM_LOAD( "ddprom4.h10", 0x0000, 0x0100, CRC(7280f000) SHA1(bfad2b547b8abe6c67928bec08e7d00431c539d5) ) // HORSES
	ROM_LOAD( "ddprom5.k1",  0x0100, 0x0100, CRC(15edbdac) SHA1(298640afb24830d32f54c0c7c5960d777f51f2bd) ) // TILES?
	ROM_LOAD( "ddprom1.c1",  0x0200, 0x0100, CRC(e1c2fa1b) SHA1(6b8b0b2c1ac4b2796070452c923ba96dd8b29048) ) // ???

	ROM_REGION( 0x200, "proms3", 0 ) // other
	ROM_LOAD( "ddprom2.j5",  0x0000, 0x0100, CRC(3e5402dc) SHA1(2f497333f49064c54995cec8919f3aebdc17e977) )
	ROM_LOAD( "ddprom6.m12", 0x0100, 0x0100, CRC(7f677b7d) SHA1(946014cb01f5954a3cb196796741ee174a0de641) )
ROM_END

ROM_START( dmndrbya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dd4", 0x00000, 0x02000, CRC(29b06e0f) SHA1(301fc2fe25ce47c2ad5112f0b795cd6bae605071) )
	ROM_LOAD( "dd5", 0x02000, 0x02000, CRC(5299d020) SHA1(678d338d2cee5250154454be97456d5f80bb4759) )
	ROM_LOAD( "dd6", 0x04000, 0x02000, CRC(f7e30ec0) SHA1(bf898987366ee9def190e3575108395b0aaef2d6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dd03.j9", 0x00000, 0x01000, CRC(660f0cae) SHA1(b3b414e52342de877a5c20886a87002a63310a94) )

	ROM_REGION( 0x04000, "tiles", 0 )
	ROM_LOAD( "dd1", 0x00000, 0x02000, CRC(7fe475a6) SHA1(008bbaff87baad7f4c2497e40bf5e3fc95f993b4) )
	ROM_LOAD( "dd2", 0x02000, 0x02000, CRC(54def3ee) SHA1(fb88852ada2b5b555c0e8c0a082ed9978ff27434) )

	ROM_REGION( 0x4000, "racetrack_data", 0 )
	ROM_LOAD( "dd22.n6", 0x00000, 0x02000, CRC(db6b13fc) SHA1(3415deb2ffa86679e4f8abb644b75963e5368ba0) )
	ROM_LOAD( "dd23.n7", 0x02000, 0x02000, CRC(595fdb9b) SHA1(133d227bb156be52337da974e37973b049722e49) )

	ROM_REGION( 0x18000, "horses", 0 )
	ROM_LOAD( "dd07.b1", 0x00000, 0x02000, CRC(207a534a) SHA1(ddbd292f79cc9fb7bd9f0ee9874da87909147789) )
	ROM_LOAD( "dd08.b2", 0x02000, 0x02000, CRC(f380e2c4) SHA1(860a6557ae8b81d310c353f88f9194e1ffd551ec) )
	ROM_LOAD( "dd09.b3", 0x04000, 0x02000, CRC(68ebf74c) SHA1(959ee6c4ce700cff86af39442063dc79b8f8913e) )
	ROM_LOAD( "dd10.b5", 0x06000, 0x01000, CRC(38b1568a) SHA1(f7e04db49708dfc8c8512026d3460af0f3fb6780) )
	ROM_LOAD( "dd11.d1", 0x08000, 0x02000, CRC(fe615561) SHA1(808f703d0ca1576feb78f21c380e4006dd634a9c) )
	ROM_LOAD( "dd12.d2", 0x0a000, 0x02000, CRC(4df63aae) SHA1(a0b224fb1157fc25c21f9f0664bb8385e94e5c77) )
	ROM_LOAD( "dd13.d4", 0x0c000, 0x02000, CRC(cace0dfc) SHA1(41902f3ee2fa18798e3b441ee18f7b953d977b93) )
	ROM_LOAD( "dd14.d5", 0x0e000, 0x01000, CRC(2c602cbe) SHA1(78ffe79e3f2c4a3e9c6adc8f4635ed1a93528dc8) )
	ROM_LOAD( "dd15.e1", 0x10000, 0x02000, CRC(2ce23b64) SHA1(5cbeabc015cb167c7fd485ab4d9f1329bc2e94b3) )
	ROM_LOAD( "dd16.e2", 0x12000, 0x02000, CRC(6af9796c) SHA1(4cd818d488ac85fd6f8732fdca80cc29db86d3f4) )
	ROM_LOAD( "dd17.e4", 0x14000, 0x02000, CRC(b451cde2) SHA1(1c7340cc39d9beca1640c88000112c898d3de941) )
	ROM_LOAD( "dd18.e5", 0x16000, 0x01000, CRC(56228aaf) SHA1(74e96ebefc1b69310b23e47a35affbb7cd7d9acc) )

	ROM_REGION( 0x6000, "track", 0 )
	ROM_LOAD( "dd19.n2", 0x00000, 0x02000, CRC(fd536051) SHA1(556dfe064eeb9be1db751410ec128385c463e706) )
	ROM_LOAD( "dd20.n3", 0x02000, 0x02000, CRC(1497e52f) SHA1(f08c20c97c8d2148fcc705297cf1129bc65c9b83) )
	ROM_LOAD( "dd21.n4", 0x04000, 0x02000, CRC(87605d44) SHA1(c985fb15eac7bcc89e92909cf588b5982d0cabd0) )

	ROM_REGION( 0x020, "proms", 0 ) // palette
	ROM_LOAD( "ddprom3.h5", 0x0000, 0x0020, CRC(aea3cff6) SHA1(1139dd52c127436873a674be8a14527190091a82) )

	ROM_REGION( 0x300, "proms2", 0 ) // lookup
	ROM_LOAD( "ddprom4.h10", 0x0000, 0x0100, CRC(7280f000) SHA1(bfad2b547b8abe6c67928bec08e7d00431c539d5) ) // HORSES
	ROM_LOAD( "ddprom5.k1",  0x0100, 0x0100, CRC(15edbdac) SHA1(298640afb24830d32f54c0c7c5960d777f51f2bd) ) // TILES?
	ROM_LOAD( "ddprom1.c1",  0x0200, 0x0100, CRC(e1c2fa1b) SHA1(6b8b0b2c1ac4b2796070452c923ba96dd8b29048) ) // ???

	ROM_REGION( 0x200, "proms3", 0 ) // other
	ROM_LOAD( "ddprom2.j5",  0x0000, 0x0100, CRC(3e5402dc) SHA1(2f497333f49064c54995cec8919f3aebdc17e977) )
	ROM_LOAD( "ddprom6.m12", 0x0100, 0x0100, CRC(7f677b7d) SHA1(946014cb01f5954a3cb196796741ee174a0de641) )
ROM_END

ROM_START( dmndrbybl ) // 51a (main) + 51b (GFX) PCBs
	ROM_REGION( 0x10000, "maincpu", 0 ) // on main PCB
	ROM_LOAD( "k1_04.m6", 0x00000, 0x02000, CRC(23213b3f) SHA1(e281b8877b0cc59541143312518decd8f97a6afa) )
	ROM_LOAD( "k1_05.m7", 0x02000, 0x02000, CRC(44a68cd0) SHA1(8dc0cc34d2cbf168d6b1985786170730d74ac6ac) )
	ROM_LOAD( "k1_06.m8", 0x04000, 0x02000, CRC(6623fb91) SHA1(c9fc1f1ed3362f546be0c9ae16b31f57e7834249) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on main PCB, identical to the original
	ROM_LOAD( "k1_03.j9", 0x00000, 0x01000, CRC(660f0cae) SHA1(b3b414e52342de877a5c20886a87002a63310a94) )

	ROM_REGION( 0x04000, "tiles", 0 ) // on main PCB
	ROM_LOAD( "k1_01.e1", 0x00000, 0x02000, CRC(d2cacafb) SHA1(f6e8495262614144ca2f71bb700de573f14ebc73) )
	ROM_LOAD( "k1_02.e2", 0x02000, 0x02000, CRC(6fcb7c9a) SHA1(e6be34cb2bb8a6e22f13cc560ab3c931861894d1) )

	ROM_REGION( 0x4000, "racetrack_data", 0 ) // on GFX PCB, identical to the original
	ROM_LOAD( "k1_13.n6", 0x00000, 0x02000, CRC(db6b13fc) SHA1(3415deb2ffa86679e4f8abb644b75963e5368ba0) )
	ROM_LOAD( "k1_14.n7", 0x02000, 0x02000, CRC(595fdb9b) SHA1(133d227bb156be52337da974e37973b049722e49) )

	ROM_REGION( 0x18000, "horses", 0 ) // on GFX PCB, some differences
	ROM_LOAD( "k1_07.b1", 0x00000, 0x08000, CRC(c4ac04b2) SHA1(d0310ca7190b4e625cc223a7bbce9fa73e4b9b9b) )
	ROM_LOAD( "k1_08.d1", 0x08000, 0x08000, CRC(ee49b7e0) SHA1(4fbc367226ca13e994504c625186eaceebbb44d6) )
	ROM_LOAD( "k1_09.e1", 0x10000, 0x08000, CRC(36762ac5) SHA1(4226a2a558c13d1e6d5378f47b2887baa34c79c7) )

	ROM_REGION( 0x6000, "track", 0 ) // on GFX PCB, identical to the original
	ROM_LOAD( "k1_10.n2", 0x00000, 0x02000, CRC(fd536051) SHA1(556dfe064eeb9be1db751410ec128385c463e706) )
	ROM_LOAD( "k1_11.n4", 0x02000, 0x02000, CRC(1497e52f) SHA1(f08c20c97c8d2148fcc705297cf1129bc65c9b83) )
	ROM_LOAD( "k1_12.n4", 0x04000, 0x02000, CRC(87605d44) SHA1(c985fb15eac7bcc89e92909cf588b5982d0cabd0) )

	ROM_REGION( 0x020, "proms", 0 ) // palette, not dumped for this set, on GFX PCB
	ROM_LOAD( "dm74s288n.h5", 0x0000, 0x0020, BAD_DUMP CRC(aea3cff6) SHA1(1139dd52c127436873a674be8a14527190091a82) )

	ROM_REGION( 0x300, "proms2", 0 ) // lookup, not dumped for this set
	ROM_LOAD( "dm74s287n.h10", 0x0000, 0x0100, BAD_DUMP CRC(7280f000) SHA1(bfad2b547b8abe6c67928bec08e7d00431c539d5) ) // HORSES, on GFX PCB
	ROM_LOAD( "dm74s287n.k1",  0x0100, 0x0100, BAD_DUMP CRC(15edbdac) SHA1(298640afb24830d32f54c0c7c5960d777f51f2bd) ) // TILES?, on GFX PCB
	ROM_LOAD( "tbp24s10n1.c1", 0x0200, 0x0100, BAD_DUMP CRC(e1c2fa1b) SHA1(6b8b0b2c1ac4b2796070452c923ba96dd8b29048) ) // ???, on main PCB

	ROM_REGION( 0x200, "proms3", 0 ) // other, not dumped for this set
	ROM_LOAD( "tbp24s10n.j5",  0x0000, 0x0100, BAD_DUMP CRC(3e5402dc) SHA1(2f497333f49064c54995cec8919f3aebdc17e977) ) // on main PCB
	ROM_LOAD( "dm74s287n.m12", 0x0100, 0x0100, BAD_DUMP CRC(7f677b7d) SHA1(946014cb01f5954a3cb196796741ee174a0de641) ) // on GFX PCB
ROM_END

} // anonymous namespace


GAME( 1994, dmndrby,   0,       dderby,   dderby,   dmndrby_state, empty_init, ROT0,    "Electrocoin",              "Diamond Derby (Win bet, newer)",                  MACHINE_IMPERFECT_COLORS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // hack?
GAME( 1986, dmndrbya,  dmndrby, dderby,   dderbya,  dmndrby_state, empty_init, ROT0,    "Electrocoin",              "Diamond Derby (Win bet, original)",               MACHINE_IMPERFECT_COLORS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1986, dmndrbybl, dmndrby, dderby,   dderbybl, dmndrby_quinella_state, empty_init, ROT0,    "bootleg (EDG Impeuropex)", "Diamond Derby (Quinella bet, EDG Impeuropex bootleg)", MACHINE_IMPERFECT_COLORS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
// Original Japanese Quinella version known to exist, cfr. flyer, published by Tsumara Ltd. (SNK spin-off?)
