// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*******************************************************************************

Oli-Boo-Chu (USA) / Punching Kid (パンチングキッド) (Japan)
There's also an English language flyer for Ali-Boo-Chu

driver by Nicola Salmoria

NOTES:
- Irem M47 hardware? (according to PCB, this is all we have)
- Doesn't seem related to later Irem designs (M52, etc.)
- PCB (Punching Kid): https://youtu.be/ImUpedYnfME
- PCB (Oli Boo Chu): https://youtu.be/nPzBmCDtHO0
- Punching Kid has a POST with zeroes across the screen, Oli Boo Chu shows nothing.
  Punching Kid also doesn't play a sound when Boo drops food. The "CHU HAS FOOD"
  sample isn't played in either version (no write to sound_command...)

TODO:
- verify video timing, PCB video reference does imply a 62.5Hz refresh rate
- accurate IRQ timing
- does it have Z80 waitstates?
- HC55516 should actually be an HC55536, the "OLI IS OUT", sound is also muffled
  compared to known PCB footage.
- verify HC55536 clock, I don't think it's from master XTAL, maybe R/C osc.
- Diagnostics outputs? See flip_screen_w for an explanation.
- Verify the PROM color resistances, seems to be standard.

--------------

Sound M-47C-A:

                Z80
             2114   OBC17
             2114   OBC18
  OBC15
  OBC16

  HC3-55536    8910


CPU M-47A-A:

          2128
          2128             2114 2114
          OBC8B            2114 2114
          OBC7C
          OBC6B
          OBC5B
          OBC4B
 SW1 Z80  OBC3B
          OBC2B
 SW2      OBC1B   18.432MHz


VIDEO M-47B-A:

    OBC10      OBC11     OBC9
    OBC12
              2125     C-3          2114
              2125                  2114
     2125     2125
     2125     2125
     2125
     2125              C-2          2114
                                    2114
               OBC14     OBC13

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/hc55516.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"
#include "video/resnet.h"


namespace {

class olibochu_state : public driver_device
{
public:
	olibochu_state(machine_config const &mconfig, device_type type, char const *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram%u", 0U),
		m_soundlatch(*this, "soundlatch%u", 0U),
		m_ay8910(*this, "ay8910"),
		m_cvsd(*this, "cvsd"),
		m_cvsd_clock(*this, "cvsd_clock"),
		m_samplerom(*this, "samples")
	{ }

	void olibochu(machine_config &config);

	// port handlers
	DECLARE_INPUT_CHANGED_MEMBER(palette_changed) { adjust_palette(); }

protected:
	// initialization
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices, memory pointers
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_colorram;
	required_shared_ptr_array<u8, 2> m_spriteram;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;
	required_device<ay8910_device> m_ay8910;
	required_device<hc55516_device> m_cvsd;
	required_device<clock_device> m_cvsd_clock;
	required_region_ptr<u8> m_samplerom;

	// internal state
	tilemap_t *m_bg_tilemap = nullptr;
	u16 m_sound_command = 0U;
	u8 m_sample_latch = 0U;
	u16 m_sample_address = 0U;

	// video-related
	void videoram_w(offs_t offset, u8 data);
	void colorram_w(offs_t offset, u8 data);
	void flip_screen_w(u8 data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	void adjust_palette();
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, rectangle const &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void draw_sprites(bitmap_ind16 &bitmap, rectangle const &cliprect);

	// sound-related
	void sound_command_w(offs_t offset, u8 data);
	void sample_latch_w(u8 data);
	void sample_start_w(u8 data);
	u8 soundlatch_r();
	void cvsd_tick(int state);

	// address maps
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

void olibochu_state::machine_start()
{
	save_item(NAME(m_sound_command));
	save_item(NAME(m_sample_latch));
	save_item(NAME(m_sample_address));
}

void olibochu_state::machine_reset()
{
	m_sound_command = 0;
	adjust_palette();
}


/*******************************************************************************
    Video
*******************************************************************************/

void olibochu_state::palette(palette_device &palette) const
{
	u8 const *prom = memregion("proms")->base();
	static int constexpr resistances[3] = { 1000, 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances[0], rweights, 0, 0,
			3, &resistances[0], gweights, 0, 0,
			2, &resistances[1], bweights, 0, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(prom[i], 0);
		bit1 = BIT(prom[i], 1);
		bit2 = BIT(prom[i], 2);
		u8 const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(prom[i], 3);
		bit1 = BIT(prom[i], 4);
		bit2 = BIT(prom[i], 5);
		u8 const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(prom[i], 6);
		bit1 = BIT(prom[i], 7);
		u8 const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}
}

void olibochu_state::adjust_palette()
{
	int const bank = (ioport("CONF")->read() & 1) ? 0x10 : 0;
	u8 const *prom = memregion("proms")->base() + 0x20;

	for (int i = 0; i < 0x100; i++)
	{
		m_palette->set_pen_indirect(i, (prom[i] & 0xf) | bank);
		m_palette->set_pen_indirect(i + 0x100, (prom[i + 0x100] & 0xf) | bank);
	}
}

void olibochu_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void olibochu_state::colorram_w(offs_t offset, u8 data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void olibochu_state::flip_screen_w(u8 data)
{
	// TODO: output latch *maybe*, diagnostics related?

	// Q0-6 off on cold boot, Q1 on at POST, Q4-5 on after POST,
	// Q4 off at title screen, Q5 off/on at stage/intermission
	// start, Q6 on at intermission, Q6 off after intermission,
	// Q1/Q5 on after warm boot, and Q7 is obviously flip screen
	flip_screen_set(BIT(data, 7));
}

TILE_GET_INFO_MEMBER(olibochu_state::get_bg_tile_info)
{
	u8 const attr   = m_colorram[tile_index];
	bool const bank = BIT(attr, 5);
	u16 const code  = m_videoram[tile_index] | (bank << 8);
	u8 const color  = (attr & 0x1f) | 0x20;
	int const flags = TILE_FLIPYX(attr >> 6 & 3);

	tileinfo.category = bank;
	tileinfo.set(0, code, color, flags);
}

void olibochu_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(olibochu_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
}

void olibochu_state::draw_sprites(bitmap_ind16 &bitmap, rectangle const &cliprect)
{
	// fill limit of 16 sprites with first half split into pairs?
	// game never initializes the latter half of the second bank
	for (int bank = 0; bank < 2; bank++)
	{
		for (int offs = m_spriteram[bank].bytes() - 4; offs >= 0; offs -= 4)
		{
			u8 const *src  = &m_spriteram[bank][offs];
			u8 const code  = src[0];
			u8 const attr  = src[1];
			u8 const color = attr & 0x3f;
			bool flipx     = BIT(attr, 6);
			bool flipy     = BIT(attr, 7);
			int ypos       = src[2];
			int xpos       = src[3];

			if (bank == 1) // fix wrap
				ypos = ((ypos + 8) & 0xff) - 8;

			if (flip_screen())
			{
				xpos = (bank ? 240 : 248) - xpos;
				ypos = (bank ? 240 : 248) - ypos;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(bank)->transpen(bitmap, cliprect, code, color, flipx, flipy, xpos, ypos, 0);
		}
	}
}

u32 olibochu_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_CATEGORY(0));
	draw_sprites(bitmap, cliprect);

	// high priority tiles are used during intermission (after round 2)
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1));

	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(olibochu_state::scanline)
{
	int const scanline = param;

	if (scanline == 248) // VBLANK
	{
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, HOLD_LINE, 0xd7); // Z80 - RST 10h
		m_audiocpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
	}

	if (scanline == 128) // periodic
	{
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, HOLD_LINE, 0xcf); // Z80 - RST 08h
		m_audiocpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
	}
}


/*******************************************************************************
    Sound
*******************************************************************************/

void olibochu_state::sound_command_w(offs_t offset, u8 data)
{
	u16 const prev_lo = m_sound_command & 0x003f;
	if (offset == 0)
		m_sound_command = (m_sound_command & 0x00ff) | data << 8;
	else
		m_sound_command = (m_sound_command & 0xff00) | data;

	u8 c;
	u16 const hi = m_sound_command & 0xffc0;
	u16 const lo = m_sound_command & 0x003f;

	// sound command low bits (edge-triggered) = soundlatch d4-d7
	if (lo && lo != prev_lo)
	{
		c = count_leading_zeros_32(lo) - 26;
		m_soundlatch[1]->write(c & 0xf);
	}

	// sound command high bits = soundlatch d0-d3
	for (c = 0; c < 16 && !BIT(hi, c); c++) { }
	m_soundlatch[0]->write((16 - c) & 0xf);
}

void olibochu_state::sample_latch_w(u8 data)
{
	m_sample_latch = data;
}

void olibochu_state::sample_start_w(u8 data)
{
	if (BIT(data, 7))
	{
		// start sample
		m_soundlatch[1]->clear_w();
		m_sample_address = m_sample_latch * 0x20 * 8;
	}

	m_cvsd->fzq_w(BIT(data, 7));
}

u8 olibochu_state::soundlatch_r()
{
	return (m_soundlatch[0]->read() & 0xf) | (m_soundlatch[1]->read() << 4 & 0xf0);
}

void olibochu_state::cvsd_tick(int state)
{
	if (state)
	{
		m_cvsd->digin_w(BIT(m_samplerom[m_sample_address / 8], ~m_sample_address & 7));
		m_sample_address = (m_sample_address + 1) & 0xffff;
	}
}


/*******************************************************************************
    Address Maps
*******************************************************************************/

void olibochu_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram().w(FUNC(olibochu_state::videoram_w)).share(m_videoram);
	map(0x8400, 0x87ff).ram().w(FUNC(olibochu_state::colorram_w)).share(m_colorram);
	map(0x9000, 0x901f).writeonly().share(m_spriteram[1]);
	map(0x9020, 0x903f).nopw(); // discard (see above)
	map(0x9800, 0x983f).writeonly().share(m_spriteram[0]);
	map(0xa000, 0xa000).portr("IN0");
	map(0xa001, 0xa001).portr("IN1");
	map(0xa002, 0xa002).portr("IN2");
	map(0xa003, 0xa003).portr("DSW0");
	map(0xa004, 0xa004).portr("DSW1");
	map(0xa005, 0xa005).portr("DSW2");
	map(0xa800, 0xa801).w(FUNC(olibochu_state::sound_command_w));
	map(0xa802, 0xa802).w(FUNC(olibochu_state::flip_screen_w));
	map(0xf000, 0xffff).ram();
}

void olibochu_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x6000, 0x63ff).ram();
	map(0x7000, 0x7000).r(FUNC(olibochu_state::soundlatch_r));
	map(0x7000, 0x7001).w(m_ay8910, FUNC(ay8910_device::address_data_w));
	map(0x7004, 0x7004).w(FUNC(olibochu_state::sample_latch_w));
	map(0x7006, 0x7006).w(FUNC(olibochu_state::sample_start_w));
}


/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( olibochu )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) // works in service mode but not in game
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )      PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(    0x0c, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x04, "15000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "DSW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "DSW1:6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )    PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Cross Hatch Pattern" ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1") // works in service mode, but PCB only has 2 DIPSW banks and this port is never read in game
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	// Freeze: press P1 start to stop, P2 start to continue
	PORT_DIPNAME( 0x01, 0x01, "Freeze (Cheat)")          PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coin_A ) )        PORT_DIPLOCATION("DSW3:2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_5C ) )
	PORT_SERVICE_DIPLOC( 0x10, IP_ACTIVE_LOW, "DSW3:5" )
	PORT_DIPNAME( 0x20, 0x20, "Invincibility (Cheat)" )  PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// Level Select: enable to select round at game start (turn off to start game)
	PORT_DIPNAME( 0x40, 0x40, "Level Select (Cheat)")    PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "DSW3:8" )

	PORT_START("CONF")
	PORT_CONFNAME( 0x01, 0x01, "Palette" ) PORT_CHANGED_MEMBER(DEVICE_SELF, olibochu_state, palette_changed, 0)
	PORT_CONFSETTING(    0x01, "Oli-Boo-Chu" )
	PORT_CONFSETTING(    0x00, "Punching Kid" )
INPUT_PORTS_END

static INPUT_PORTS_START( punchkid )
	PORT_INCLUDE( olibochu )

	PORT_MODIFY("CONF") // change the default
	PORT_CONFNAME( 0x01, 0x00, "Palette" ) PORT_CHANGED_MEMBER(DEVICE_SELF, olibochu_state, palette_changed, 0)
	PORT_CONFSETTING(    0x01, "Oli-Boo-Chu" )
	PORT_CONFSETTING(    0x00, "Punching Kid" )
INPUT_PORTS_END


/*******************************************************************************
    GFX Layouts
*******************************************************************************/

static gfx_layout const gfx_8x8 =
{
	8, 8,
	RGN_FRAC(1, 2),
	2,
	{ RGN_FRAC(1, 2), RGN_FRAC(0, 2) },
	{ STEP8(7, -1) },
	{ STEP8(0, 8) },
	8 * 8
};

static gfx_layout const gfx_16x16 =
{
	16, 16,
	RGN_FRAC(1, 2),
	2,
	{ RGN_FRAC(1, 2), RGN_FRAC(0, 2) },
	{ STEP8(7, -1), STEP8(128 + 7, -1) },
	{ STEP16(0, 8) },
	32 * 8
};

static GFXDECODE_START( gfx_olibochu )
	GFXDECODE_ENTRY( "gfx8x8",   0, gfx_8x8,   0,   64 )
	GFXDECODE_ENTRY( "gfx16x16", 0, gfx_16x16, 256, 64 )
GFXDECODE_END


/*******************************************************************************
    Machine Configs
*******************************************************************************/

void olibochu_state::olibochu(machine_config &config)
{
	// basic machine hardware
	XTAL constexpr MASTER_CLOCK = 18.432_MHz_XTAL;

	Z80(config, m_maincpu, MASTER_CLOCK / 3 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &olibochu_state::main_map);

	Z80(config, m_audiocpu, MASTER_CLOCK / 3 / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &olibochu_state::sound_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK / 3, 384, 0, 256, 256, 8, 248);
	m_screen->set_screen_update(FUNC(olibochu_state::screen_update));
	m_screen->set_palette(m_palette);

	TIMER(config, "scanline").configure_scanline(FUNC(olibochu_state::scanline), m_screen, 0, 1);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_olibochu);
	PALETTE(config, m_palette, FUNC(olibochu_state::palette), 256 * 2, 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);

	AY8910(config, m_ay8910, MASTER_CLOCK / 3 / 2 / 2).add_route(ALL_OUTPUTS, "mono", 0.5);

	HC55516(config, m_cvsd, 0).add_route(ALL_OUTPUTS, "mono", 0.5);
	CLOCK(config, m_cvsd_clock, 16000);
	m_cvsd_clock->signal_handler().set(FUNC(olibochu_state::cvsd_tick));
	m_cvsd_clock->signal_handler().append(m_cvsd, FUNC(hc55516_device::mclock_w));
}


/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( olibochu )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "obc_1b.n3", 0x0000, 0x1000, CRC(bf17f4f4) SHA1(1075456f4b70a68548e0e1b6271fd4b845a77ce4) )
	ROM_LOAD( "obc_2b.m3", 0x1000, 0x1000, CRC(63833b0d) SHA1(0135c449c92470241d03a87709c739209139d660) )
	ROM_LOAD( "obc_3b.k3", 0x2000, 0x1000, CRC(a4038e8b) SHA1(d7dce830239c8975ac135b213a99eec0c20ec3e2) )
	ROM_LOAD( "obc_4b.j3", 0x3000, 0x1000, CRC(aad4bec4) SHA1(9203564ac841a8de2f9b8183d4086acce95e3d47) )
	ROM_LOAD( "obc_5b.h3", 0x4000, 0x1000, CRC(66efa79f) SHA1(535369d958461834435d3202cd7310ecd0aa528c) )
	ROM_LOAD( "obc_6b.f3", 0x5000, 0x1000, CRC(1123d1ef) SHA1(6094e732e61915c45b14acd90c1343f05385daf4) )
	ROM_LOAD( "obc_7c.e3", 0x6000, 0x1000, CRC(89c26fb4) SHA1(ebc51e40612af894b20bd7fc3a5179cd35aaac9b) )
	ROM_LOAD( "obc_8b.d3", 0x7000, 0x1000, CRC(af19e5a5) SHA1(5a55bbee5b2f20e2988171a310c8293dabbd9a72) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "obc_17.j4", 0x0000, 0x1000, CRC(57f07402) SHA1(a763a835ac512c69b4351c1ec72b0a64e46203aa) )
	ROM_LOAD( "obc_18.l4", 0x1000, 0x1000, CRC(0a903e9c) SHA1(d893c2f5373f748d8bebf3673b15014f4a8d4b5c) )

	ROM_REGION( 0x2000, "samples", 0 )
	ROM_LOAD( "obc_15.k1", 0x0000, 0x1000, CRC(fb5dd281) SHA1(fba947ae7b619c2559b5af69ef02acfb15733f0d) )
	ROM_LOAD( "obc_16.m1", 0x1000, 0x1000, CRC(c07614a5) SHA1(d13d271a324f99d008429c16193c4504e5894493) )

	ROM_REGION( 0x2000, "gfx8x8", 0 )
	ROM_LOAD( "obc_13.n6", 0x0000, 0x1000, CRC(b4fcf9af) SHA1(b360daa0670160dca61512823c98bc37ad99b9cf) )
	ROM_LOAD( "obc_14.n4", 0x1000, 0x1000, CRC(af54407e) SHA1(1883928b721e03e452fd0c626c403dc374b02ed7) )

	ROM_REGION( 0x4000, "gfx16x16", 0 )
	ROM_LOAD( "obc_9.a6",  0x0000, 0x1000, CRC(fa69e16e) SHA1(5a493a0a108b3e496884d1f499f3445d4e241ecd) )
	ROM_LOAD( "obc_10.a2", 0x1000, 0x1000, CRC(10359f84) SHA1(df55f06fd98233d0efbc30e3e24bf9b8cab1a5cc) )
	ROM_LOAD( "obc_11.a4", 0x2000, 0x1000, CRC(1d968f5f) SHA1(4acf78d865ca36355bb15dc1d476f5e97a5d91b7) )
	ROM_LOAD( "obc_12.b2", 0x3000, 0x1000, CRC(d8f0c157) SHA1(a7b0c873e016c3b3252c2c9b6400b0fd3d650b2f) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "c-1.n2",    0x0000, 0x0020, CRC(e488e831) SHA1(6264741f7091c614093ae1ea4f6ead3d0cef83d3) ) // palette
	ROM_LOAD( "c-2.k6",    0x0020, 0x0100, CRC(698a3ba0) SHA1(3c1a6cb881ef74647c651462a27d812234408e45) ) // char lookup table
	ROM_LOAD( "c-3.d6",    0x0120, 0x0100, CRC(efc4e408) SHA1(f0796426cf324791853aa2ae6d0c3d1f8108d5c2) ) // sprite lookup table
ROM_END

ROM_START( punchkid )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pka_1.n3",  0x0000, 0x1000, CRC(18f1fa10) SHA1(cc586d9502c7c1e922570400063a0f2277cf6b3c) )
	ROM_LOAD( "pka_2.m3",  0x1000, 0x1000, CRC(7766d9be) SHA1(bd7c3b4499f9dd6eb4b2c4a8e4e8fe4851b67b74) )
	ROM_LOAD( "pka_3.k3",  0x2000, 0x1000, CRC(bb90e21b) SHA1(ae58c05058197943ecfa0612163da5cc38f99fd2) )
	ROM_LOAD( "pka_4.j3",  0x3000, 0x1000, CRC(ce18a851) SHA1(104e82b8f2fa662655e6eb33f1115c81c058c365) )
	ROM_LOAD( "pka_5.h3",  0x4000, 0x1000, CRC(426c8254) SHA1(df0c119edcf27f547051c320300920d9c9b12148) )
	ROM_LOAD( "pka_6.f3",  0x5000, 0x1000, CRC(288b223e) SHA1(7ce0c948d2024ee9495f4cdcaa5807fc9425b4bf) )
	ROM_LOAD( "pka_7.e3",  0x6000, 0x1000, CRC(c689e057) SHA1(9b94fac8d1608412f8900aa9ddf56be12bdb847a) )
	ROM_LOAD( "pka_8.d3",  0x7000, 0x1000, CRC(61c118e0) SHA1(9c0f17283b42e6d4622b83a08ac2ac59913be50f) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "pka_17.j4", 0x0000, 0x1000, CRC(57f07402) SHA1(a763a835ac512c69b4351c1ec72b0a64e46203aa) )
	ROM_LOAD( "pka_18.l4", 0x1000, 0x1000, CRC(0a903e9c) SHA1(d893c2f5373f748d8bebf3673b15014f4a8d4b5c) )

	ROM_REGION( 0x2000, "samples", 0 )
	ROM_LOAD( "pka_15.k1", 0x0000, 0x1000, CRC(fb5dd281) SHA1(fba947ae7b619c2559b5af69ef02acfb15733f0d) )
	ROM_LOAD( "pka_16.m1", 0x1000, 0x1000, CRC(c07614a5) SHA1(d13d271a324f99d008429c16193c4504e5894493) )

	ROM_REGION( 0x2000, "gfx8x8", 0 )
	ROM_LOAD( "pka_13.n6", 0x0000, 0x1000, CRC(388f2bfd) SHA1(939cf2dcb21965106aa2fcdcd3abcee5e4071770) )
	ROM_LOAD( "pka_14.n4", 0x1000, 0x1000, CRC(b5bf456f) SHA1(5db9af314d74cef8a3405162c4ba99204b79ae5a) )

	ROM_REGION( 0x4000, "gfx16x16", 0 )
	ROM_LOAD( "pka_9.a6",  0x0000, 0x1000, CRC(fa69e16e) SHA1(5a493a0a108b3e496884d1f499f3445d4e241ecd) )
	ROM_LOAD( "pka_10.a2", 0x1000, 0x1000, CRC(10359f84) SHA1(df55f06fd98233d0efbc30e3e24bf9b8cab1a5cc) )
	ROM_LOAD( "pka_11.a4", 0x2000, 0x1000, CRC(1d968f5f) SHA1(4acf78d865ca36355bb15dc1d476f5e97a5d91b7) )
	ROM_LOAD( "pka_12.b2", 0x3000, 0x1000, CRC(d8f0c157) SHA1(a7b0c873e016c3b3252c2c9b6400b0fd3d650b2f) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "c-1.n2",    0x0000, 0x0020, CRC(e488e831) SHA1(6264741f7091c614093ae1ea4f6ead3d0cef83d3) ) // palette
	ROM_LOAD( "c-2.k6",    0x0020, 0x0100, CRC(698a3ba0) SHA1(3c1a6cb881ef74647c651462a27d812234408e45) ) // char lookup table
	ROM_LOAD( "c-3.d6",    0x0120, 0x0100, CRC(efc4e408) SHA1(f0796426cf324791853aa2ae6d0c3d1f8108d5c2) ) // sprite lookup table
ROM_END

} // anonymous namespace


/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT    MACHINE   INPUT     STATE           INIT        SCREEN  COMPANY               FULLNAME                FLAGS
GAME( 1981, olibochu, 0,        olibochu, olibochu, olibochu_state, empty_init, ROT270, "Irem (GDI license)", "Oli-Boo-Chu (USA)",    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, punchkid, olibochu, olibochu, punchkid, olibochu_state, empty_init, ROT270, "Irem",               "Punching Kid (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
