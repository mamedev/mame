// license:BSD-3-Clause
// copyright-holders: Mirko Buffoni, Couriersud

/***************************************************************************

    IronHorse
    GX560

    driver by Mirko Buffoni

***************************************************************************/

#include "emu.h"

#include "konamipt.h"

#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/discrete.h"
#include "sound/ymopn.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_PALETTEBANK     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_PALETTEBANK)

#include "logmacro.h"

#define LOGPALETTEBANK(...)     LOGMASKED(LOG_PALETTEBANK,     __VA_ARGS__)


namespace {

class ironhors_base_state : public driver_device
{
public:
	ironhors_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_disc_ih(*this, "disc_ih"),
		m_interrupt_enable(*this, "int_enable"),
		m_scroll(*this, "scroll"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram%u", 1U)
	{ }

	void base(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void sh_irqtrigger_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void charbank_w(uint8_t data);
	void palettebank_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	void filter_w(uint8_t data);

	void palette(palette_device &palette) const;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<discrete_device> m_disc_ih;

	// memory pointers
	required_shared_ptr<uint8_t> m_interrupt_enable;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_spriteram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_palettebank = 0U;
	uint8_t m_charbank = 0U;
	uint8_t m_spriterambank = 0U;
};

class ironhors_state : public ironhors_base_state
{
public:
	ironhors_state(const machine_config &mconfig, device_type type, const char *tag) :
		ironhors_base_state(mconfig, type, tag)
	{ }

	void ironhors(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_tick);

	void master_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;
	void slave_io_map(address_map &map) ATTR_COLD;

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};

class farwest_state : public ironhors_base_state
{
public:
	farwest_state(const machine_config &mconfig, device_type type, const char *tag) :
		ironhors_base_state(mconfig, type, tag)
	{ }

	void farwest(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_tick);

	void master_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

void ironhors_base_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances[4] = { 2000, 1000, 470, 220 };

	// compute the color output resistor weights
	double rweights[4], gweights[4], bweights[4];
	compute_resistor_weights(0, 255, -1.0,
			4, resistances, rweights, 1000, 0,
			4, resistances, gweights, 1000, 0,
			4, resistances, bweights, 1000, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i | 0x000], 0);
		bit1 = BIT(color_prom[i | 0x000], 1);
		bit2 = BIT(color_prom[i | 0x000], 2);
		bit3 = BIT(color_prom[i | 0x000], 3);
		int const r = combine_weights(rweights, bit0, bit1, bit2, bit3);

		// green component
		bit0 = BIT(color_prom[i | 0x100], 0);
		bit1 = BIT(color_prom[i | 0x100], 1);
		bit2 = BIT(color_prom[i | 0x100], 2);
		bit3 = BIT(color_prom[i | 0x100], 3);
		int const g = combine_weights(gweights, bit0, bit1, bit2, bit3);

		// blue component
		bit0 = BIT(color_prom[i | 0x200], 0);
		bit1 = BIT(color_prom[i | 0x200], 1);
		bit2 = BIT(color_prom[i | 0x200], 2);
		bit3 = BIT(color_prom[i | 0x200], 3);
		int const b = combine_weights(bweights, bit0, bit1, bit2, bit3);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x300;

	// characters use colors 0x10-0x1f of each 0x20 color bank, while sprites use colors 0-0x0f
	for (int i = 0; i < 0x200; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			uint8_t const ctabentry = (j << 5) | ((~i & 0x100) >> 4) | (color_prom[i] & 0x0f);
			palette.set_pen_indirect(((i & 0x100) << 3) | (j << 8) | (i & 0xff), ctabentry);
		}
	}
}

void ironhors_base_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void ironhors_base_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void ironhors_base_state::charbank_w(uint8_t data)
{
	if (m_charbank != (data & 0x03))
	{
		m_charbank = data & 0x03;
		machine().tilemap().mark_all_dirty();
	}

	m_spriterambank = data & 0x08;

	// other bits unknown
}

void ironhors_base_state::palettebank_w(uint8_t data)
{
	if (m_palettebank != (data & 0x07))
	{
		m_palettebank = data & 0x07;
		machine().tilemap().mark_all_dirty();
	}

	machine().bookkeeping().coin_counter_w(0, data & 0x10);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);

	// bit 6 unknown - set after game over

	if (data & 0x88)
		LOGPALETTEBANK("palettebank_w %02x", data);
}

void ironhors_base_state::flipscreen_w(uint8_t data)
{
	if (flip_screen() != (~data & 0x08))
	{
		flip_screen_set(~data & 0x08);
		machine().tilemap().mark_all_dirty();
	}

	// other bits are used too, but unknown
}

TILE_GET_INFO_MEMBER(ironhors_state::get_bg_tile_info)
{
	int const code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x40) << 2) +
			  ((m_colorram[tile_index] & 0x20) << 4) + (m_charbank << 10);
	int const color = (m_colorram[tile_index] & 0x0f) + 16 * m_palettebank;
	int const flags = ((m_colorram[tile_index] & 0x10) ? TILE_FLIPX : 0) |
			  ((m_colorram[tile_index] & 0x20) ? TILE_FLIPY : 0);

	tileinfo.set(0, code, color, flags);
}

void ironhors_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ironhors_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_rows(32);
}

void ironhors_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int bank = m_spriterambank ? 0 : 1;
	uint8_t *sr = m_spriteram[bank];

	// note that it has 5 bytes per sprite
	int end = m_spriteram[bank].bytes();
	end -= end % 5;

	for (int offs = 0; offs < end; offs += 5)
	{
		int sx = sr[offs + 3];
		int sy = sr[offs + 2];
		int flipx = sr[offs + 4] & 0x20;
		int flipy = sr[offs + 4] & 0x40;
		int const code = (sr[offs] << 2) + ((sr[offs + 1] & 0x03) << 10) + ((sr[offs + 1] & 0x0c) >> 2);
		int const color = ((sr[offs + 1] & 0xf0) >> 4) + 16 * m_palettebank;
	//  int mod = flip_screen() ? -8 : 8;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		switch (sr[offs + 4] & 0x0c)
		{
			case 0x00:  // 16x16
				m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
						code / 4,
						color,
						flipx, flipy,
						sx, sy, 0);
				break;

			case 0x04:  // 16x8
				{
					if (flip_screen()) sy += 8; // this fixes the train wheels' position

					m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
							code & ~1,
							color,
							flipx, flipy,
							flipx ? sx + 8 : sx, sy, 0);
					m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
							code | 1,
							color,
							flipx, flipy,
							flipx ? sx : sx + 8, sy, 0);
				}
				break;

			case 0x08:  // 8x16
				{
					m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
							code & ~2,
							color,
							flipx, flipy,
							sx, flipy ? sy + 8 : sy, 0);
					m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
							code | 2,
							color,
							flipx, flipy,
							sx, flipy ? sy : sy + 8, 0);
				}
				break;

			case 0x0c:  // 8x8
				{
					m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
							code,
							color,
							flipx, flipy,
							sx, sy, 0);
				}
				break;
		}
	}
}

uint32_t ironhors_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int row = 0; row < 32; row++)
		m_bg_tilemap->set_scrollx(row, m_scroll[row]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

TILE_GET_INFO_MEMBER(farwest_state::get_bg_tile_info)
{
	int const code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x40) << 2) +
			 ((m_colorram[tile_index] & 0x20) << 4) + (m_charbank << 10);
	int const color = (m_colorram[tile_index] & 0x0f) + 16 * m_palettebank;
	int const flags = 0;//((m_colorram[tile_index] & 0x10) ? TILE_FLIPX : 0) |  ((m_colorram[tile_index] & 0x20) ? TILE_FLIPY : 0);

	tileinfo.set(0, code, color, flags);
}

void farwest_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(farwest_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_rows(32);
}

void farwest_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *sr = m_spriteram[1];
	uint8_t *sr2 = m_spriteram[0];

	for (int offs = 0; offs < m_spriteram[0].bytes(); offs += 4)
	{
		int sx = sr[offs + 2];
		int sy = sr[offs + 1];
		int flipx = sr[offs + 3] & 0x20;
		int flipy = sr[offs + 3] & 0x40;
		int const code = (sr[offs] << 2) + ((sr2[offs] & 0x03) << 10) + ((sr2[offs] & 0x0c) >> 2);
		int const color = ((sr2[offs] & 0xf0) >> 4) + 16 * m_palettebank;

	//  int mod = flip_screen() ? -8 : 8;

//      if (flip_screen())
		{
		//  sx = 240 - sx;
			sy = 240 - sy;
		//  flipx = !flipx;
		//  flipy = !flipy;
		}

		switch (sr[offs + 3] & 0x0c)
		{
			case 0x00:  // 16x16
				m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
						code / 4,
						color,
						flipx, flipy,
						sx, sy, 0);
				break;

			case 0x04:  // 16x8
				{
					if (flip_screen()) sy += 8; // this fixes the train wheels' position

					m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
							code & ~1,
							color,
							flipx, flipy,
							flipx ? sx + 8 : sx, sy, 0);
					m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
							code | 1,
							color,
							flipx, flipy,
							flipx ? sx : sx + 8, sy, 0);
				}
				break;

			case 0x08:  // 8x16
				{
					m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
							code & ~2,
							color,
							flipx, flipy,
							sx, flipy ? sy + 8 : sy, 0);
					m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
							code | 2,
							color,
							flipx, flipy,
							sx, flipy ? sy : sy + 8, 0);
				}
				break;

			case 0x0c:  // 8x8
				{
					m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
							code,
							color,
							flipx, flipy,
							sx, sy, 0);
				}
				break;
		}
	}
}

uint32_t farwest_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int row = 0; row < 32; row++)
		m_bg_tilemap->set_scrollx(row, m_scroll[row]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(ironhors_state::scanline_tick)
{
	int const scanline = param;

	if (scanline == 240 && (m_screen->frame_number() & 1) == 0)
	{
		if (*m_interrupt_enable & 4)
			m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
	}
	else if (((scanline+16) % 64) == 0)
	{
		if (*m_interrupt_enable & 1)
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

void ironhors_base_state::sh_irqtrigger_w(uint8_t data)
{
	m_soundcpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

void ironhors_base_state::filter_w(uint8_t data)
{
	m_disc_ih->write(NODE_11, (data & 0x04) >> 2);
	m_disc_ih->write(NODE_12, (data & 0x02) >> 1);
	m_disc_ih->write(NODE_13, (data & 0x01) >> 0);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void ironhors_state::master_map(address_map &map)
{
	map(0x0000, 0x0002).ram();
	map(0x0003, 0x0003).ram().w(FUNC(ironhors_state::charbank_w));
	map(0x0004, 0x0004).ram().share(m_interrupt_enable);
	map(0x0005, 0x001f).ram();
	map(0x0020, 0x003f).ram().share(m_scroll);
	map(0x0040, 0x005f).ram();
	map(0x0060, 0x00df).ram();
	map(0x0800, 0x0800).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x0900, 0x0900).portr("DSW3").w(FUNC(ironhors_state::sh_irqtrigger_w));
	map(0x0a00, 0x0a00).portr("DSW2").w(FUNC(ironhors_state::palettebank_w));
	map(0x0b00, 0x0b00).portr("DSW1").w(FUNC(ironhors_state::flipscreen_w));
	map(0x0b01, 0x0b01).portr("P2");
	map(0x0b02, 0x0b02).portr("P1");
	map(0x0b03, 0x0b03).portr("SYSTEM");
	map(0x1800, 0x1800).nopw(); // ???
	map(0x1a00, 0x1a01).nopw(); // ???
	map(0x1c00, 0x1dff).nopw(); // ???
	map(0x2000, 0x23ff).ram().w(FUNC(ironhors_state::colorram_w)).share(m_colorram);
	map(0x2400, 0x27ff).ram().w(FUNC(ironhors_state::videoram_w)).share(m_videoram);
	map(0x2800, 0x2fff).ram();
	map(0x3000, 0x30ff).ram().share(m_spriteram[1]);
	map(0x3100, 0x37ff).ram();
	map(0x3800, 0x38ff).ram().share(m_spriteram[0]);
	map(0x3900, 0x3fff).ram();
	map(0x4000, 0xffff).rom();
}

void ironhors_state::slave_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x8000, 0x8000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void ironhors_state::slave_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ym2203", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}

void farwest_state::master_map(address_map &map)
{
	map(0x0000, 0x1bff).rom();

	map(0x0000, 0x0002).ram();
	//20=31db

	map(0x0005, 0x001f).ram();
	map(0x0040, 0x005f).ram();
	map(0x0060, 0x00ff).ram();
	map(0x0800, 0x0800).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x0900, 0x0900) /*.protr("DSW3") */ .w(FUNC(farwest_state::sh_irqtrigger_w));
	map(0x0a00, 0x0a00).portr("DSW2"); //.w(FUNC(farwest_state::palettebank_w));
	map(0x0b00, 0x0b00).portr("DSW1").w(FUNC(farwest_state::flipscreen_w));
	map(0x0b01, 0x0b01).portr("DSW2"); //.w(FUNC(farwest_state::palettebank_w));
	map(0x0b02, 0x0b02).portr("P1");
	map(0x0b03, 0x0b03).portr("SYSTEM");



	map(0x1800, 0x1800).w(FUNC(farwest_state::sh_irqtrigger_w));
	map(0x1a00, 0x1a00).ram().share(m_interrupt_enable);
	map(0x1a01, 0x1a01).ram().w(FUNC(farwest_state::charbank_w));
	map(0x1a02, 0x1a02).w(FUNC(farwest_state::palettebank_w));
	map(0x1e00, 0x1eff).ram().share(m_spriteram[0]);
	map(0x2000, 0x23ff).ram().w(FUNC(farwest_state::colorram_w)).share(m_colorram);
	map(0x2400, 0x27ff).ram().w(FUNC(farwest_state::videoram_w)).share(m_videoram);
	map(0x2800, 0x2fff).ram();
	map(0x1c00, 0x1dff).ram().share(m_spriteram[1]);
	map(0x3000, 0x31da).ram();
	map(0x31db, 0x31fa).ram().share(m_scroll);
	map(0x31fb, 0x3fff).ram();
	map(0x4000, 0xffff).rom();
}

void farwest_state::slave_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x8000, 0x8001).rw("ym2203", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( dairesya )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_MONO_4WAY_B123_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_4WAY_B123_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	// "No Coin B" = coins produce sound, but no effect on coin counter

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30K 70K+" )
	PORT_DIPSETTING(    0x10, "40K 80K+" )
	PORT_DIPSETTING(    0x08, "40K" )
	PORT_DIPSETTING(    0x00, "50K" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )       // factory default (JP)
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )         // factory default (US)
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_DIPNAME( 0x04, 0x04, "Button Layout" )         PORT_DIPLOCATION("SW3:3") // though US manual says unused
	PORT_DIPSETTING(    0x04, "Power Attack Squat" )
	PORT_DIPSETTING(    0x00, "Squat Attack Power" )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( ironhors )
	PORT_INCLUDE( dairesya )

	// here button 3 for player 1 and 2 are exchanged
	PORT_MODIFY("P1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL

	PORT_MODIFY("P2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout ironhors_spritelayout =
{
	16,16,
	512,
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	32*32
};

static GFXDECODE_START( gfx_ironhors )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x4_packed_msb,        0, 16*8 )
	GFXDECODE_ENTRY( "gfx", 0, ironhors_spritelayout, 16*8*16, 16*8 )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x4_packed_msb,  16*8*16, 16*8 )  // to handle 8x8 sprites
GFXDECODE_END


static const gfx_layout farwest_charlayout =
{
	8,8,    // 8*8 characters
	2048,   // 2048 characters
	4,  // 4 bits per pixel
	{ 0, 2, 4, 6 }, // the four bitplanes are packed in one byte
	{ 3*8+1, 3*8+0, 0*8+1, 0*8+0, 1*8+1, 1*8+0, 2*8+1, 2*8+0 },
	{ 0*4*8, 1*4*8, 2*4*8, 3*4*8, 4*4*8, 5*4*8, 6*4*8, 7*4*8 },
	32*8    // every char takes 32 consecutive bytes
};

static const gfx_layout farwest_spritelayout =
{
	16,16,  // 16*16 sprites
	512,    // 512 sprites
	4,  // 4 bits per pixel
	{ 0, 512*32*8, 2*512*32*8, 3*512*32*8 },    // the four bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every sprite takes 32 consecutive bytes
};

static const gfx_layout farwest_spritelayout2 =
{
	8,8,    // 8*8 characters
	2048,   // 2048 characters
	4,  // 4 bits per pixel
	{ 0, 2048*8*8, 2*2048*8*8, 3*2048*8*8 },    // the four bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every char takes 8 consecutive bytes
};

static GFXDECODE_START( gfx_farwest )
	GFXDECODE_ENTRY( "tiles",   0, farwest_charlayout,         0, 16*8 )
	GFXDECODE_ENTRY( "sprites", 0, farwest_spritelayout, 16*8*16, 16*8 )
	GFXDECODE_ENTRY( "sprites", 0, farwest_spritelayout2,16*8*16, 16*8 )  // to handle 8x8 sprites
GFXDECODE_END


/*************************************
 *
 *  Discrete sound
 *
 *************************************/

static const discrete_mixer_desc ironhors_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
		{RES_K(2.2), RES_K(2.2), RES_K(2.2)},
		{0,0,0,0,0,0},  // no variable resistors
		{0,0,0,0,0,0},  // no node capacitors
		0, RES_K(1),   // RF
		0,
		0,
		0, 1};

static const discrete_mixer_desc ironhors_mixer_desc_final =
	{DISC_MIXER_IS_RESISTOR,
		{RES_K(0.5), RES_K(1)},
		{0,0,0,0,0,0},  // no variable resistors
		{CAP_U(4.7), CAP_U(4.7)},  // node capacitors
		0, RES_K(1),   // RF
		0,
		0,
		0, 1};

static DISCRETE_SOUND_START( ironhors_discrete )

	DISCRETE_INPUTX_STREAM(NODE_01, 0, 5.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_02, 1, 5.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_03, 2, 5.0, 0)

	DISCRETE_INPUTX_STREAM(NODE_04, 3, 1.0, 0)

	DISCRETE_INPUT_DATA(NODE_11)
	DISCRETE_INPUT_DATA(NODE_12)
	DISCRETE_INPUT_DATA(NODE_13)

	DISCRETE_RCFILTER_SW(NODE_21, 1, NODE_01, NODE_11, 1000, CAP_U(0.22), 0, 0, 0)
	DISCRETE_RCFILTER_SW(NODE_22, 1, NODE_02, NODE_12, 1000, CAP_U(0.22), 0, 0, 0)
	DISCRETE_RCFILTER_SW(NODE_23, 1, NODE_03, NODE_13, 1000, CAP_U(0.22), 0, 0, 0)

	DISCRETE_MIXER3(NODE_30, 1, NODE_21, NODE_22, NODE_23, &ironhors_mixer_desc)

	DISCRETE_RCFILTER(NODE_31,NODE_04, RES_K(1), CAP_N(33) )
	DISCRETE_MIXER2(NODE_33, 1, NODE_30, NODE_31, &ironhors_mixer_desc_final)

	DISCRETE_OUTPUT(NODE_33, 5.0 )

DISCRETE_SOUND_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void ironhors_base_state::machine_start()
{
	save_item(NAME(m_palettebank));
	save_item(NAME(m_charbank));
	save_item(NAME(m_spriterambank));
}

void ironhors_base_state::machine_reset()
{
	m_palettebank = 0;
	m_charbank = 0;
	m_spriterambank = 0;
}

/*
clock measurements:
main Xtal is 18.432mhz

Z80 runs at 3.072mhz

M6809E runs at 1.532mhz ( NOT 3.072mhz)

Vsync is 61hz

Hsync is 15,56khz

These clocks make the emulation run too fast.
*/

void ironhors_base_state::base(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 18'432'000 / 6);        // 3.072 MHz??? mod by Shingo Suzuki 1999/10/15

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
//  m_screen->set_refresh_hz(61);
//  m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
//  m_screen->set_size(32*8, 32*8);
//  m_screen->set_visarea(1*8, 31*8-1, 2*8, 30*8-1);
	m_screen->set_raw(18'432'000 / 4, 296, 8, 256 - 8, 255, 16, 240); // pixel clock is a guesswork
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(ironhors_state::palette), 16*8*16+16*8*16, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ym2203(YM2203(config, "ym2203", 18'432'000 / 6));
	ym2203.port_a_write_callback().set(FUNC(ironhors_state::filter_w));
	ym2203.add_route(0, "disc_ih", 1.0, 0);
	ym2203.add_route(1, "disc_ih", 1.0, 1);
	ym2203.add_route(2, "disc_ih", 1.0, 2);
	ym2203.add_route(3, "disc_ih", 1.0, 3);

	DISCRETE(config, m_disc_ih, ironhors_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void ironhors_state::ironhors(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &ironhors_state::master_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(ironhors_state::scanline_tick), "screen", 0, 1);

	Z80(config, m_soundcpu, 18'432'000 / 6);      // 3.072 MHz
	m_soundcpu->set_addrmap(AS_PROGRAM, &ironhors_state::slave_map);
	m_soundcpu->set_addrmap(AS_IO, &ironhors_state::slave_io_map);

	m_screen->set_screen_update(FUNC(ironhors_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ironhors);
}

TIMER_DEVICE_CALLBACK_MEMBER(farwest_state::scanline_tick)
{
	int const scanline = param;

	if ((scanline % 2) == 1)
	{
		if (*m_interrupt_enable & 4)
			m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
	}
	else if ((scanline % 2) == 0)
	{
		if (*m_interrupt_enable & 1)
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

void farwest_state::farwest(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &farwest_state::master_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(farwest_state::scanline_tick), "screen", 0, 1);

	Z80(config, m_soundcpu, 18'432'000 / 6);      // 3.072 MHz
	m_soundcpu->set_addrmap(AS_PROGRAM, &farwest_state::slave_map);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_farwest);

	m_screen->set_screen_update(FUNC(farwest_state::screen_update));

	subdevice<ym2203_device>("ym2203")->port_b_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( ironhors )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "560_k03.13c",  0x4000, 0x8000, CRC(395351b4) SHA1(21cf2f39a208571d06f20ca8ca346999541e870a) )
	ROM_LOAD( "560_k02.12c",  0xc000, 0x4000, CRC(1cff3d59) SHA1(9194f31ec1e9a90850f2f44aef10b5b32a28ef1d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "560_h01.10c",  0x0000, 0x4000, CRC(2b17930f) SHA1(be7b21f050f6b74c75a33c9284455bbed5b03c63) )

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD16_BYTE( "560_h06.08f",  0x00000, 0x8000, CRC(f21d8c93) SHA1(4245fff5360e10441e11d0d207d510e5c317bb0e) )
	ROM_LOAD16_BYTE( "560_h05.07f",  0x00001, 0x8000, CRC(60107859) SHA1(ab59b6be155d36811a37dc873abbd97cd0a4120d) )
	ROM_LOAD16_BYTE( "560_h07.09f",  0x10000, 0x8000, CRC(c761ec73) SHA1(78266c9ff3ea74a59fd3ce84afb4f8a1164c8bba) )
	ROM_LOAD16_BYTE( "560_h04.06f",  0x10001, 0x8000, CRC(c1486f61) SHA1(4b96aebe5d35fd1d73bde8576689addbb1ff66ed) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "03f_h08.bin",  0x0000, 0x0100, CRC(9f6ddf83) SHA1(08a37182a974c5448156637f10fe60bfe5f225ad) ) // palette red
	ROM_LOAD( "04f_h09.bin",  0x0100, 0x0100, CRC(e6773825) SHA1(7523e7fa090d850fe79ff0069d3260c76645d65a) ) // palette green
	ROM_LOAD( "05f_h10.bin",  0x0200, 0x0100, CRC(30a57860) SHA1(3ec7535286c8bc65e203320f47e4ed6f1d3d61c9) ) // palette blue
	ROM_LOAD( "10f_h12.bin",  0x0300, 0x0100, CRC(5eb33e73) SHA1(f34916dc4617b0c48e0a7ac6ace97b35dfcf1c40) ) // character lookup table
	ROM_LOAD( "10f_h11.bin",  0x0400, 0x0100, CRC(a63e37d8) SHA1(1a0a76ecd14310125bdf41a8431d562ed498eb27) ) // sprite lookup table
ROM_END

ROM_START( ironhorsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "13c_h03.bin",  0x4000, 0x8000, CRC(24539af1) SHA1(1eb96a2cb03007665587d6ec114894ab4cafdb23) )
	ROM_LOAD( "12c_h02.bin",  0xc000, 0x4000, CRC(fab07f86) SHA1(9f599d32d473d873113b89f2b24a54a435dbcbe5) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "10c_h01.bin",  0x0000, 0x4000, CRC(2b17930f) SHA1(be7b21f050f6b74c75a33c9284455bbed5b03c63) )

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD16_BYTE( "08f_h06.bin",  0x00000, 0x8000, CRC(f21d8c93) SHA1(4245fff5360e10441e11d0d207d510e5c317bb0e) )
	ROM_LOAD16_BYTE( "07f_h05.bin",  0x00001, 0x8000, CRC(60107859) SHA1(ab59b6be155d36811a37dc873abbd97cd0a4120d) )
	ROM_LOAD16_BYTE( "09f_h07.bin",  0x10000, 0x8000, CRC(c761ec73) SHA1(78266c9ff3ea74a59fd3ce84afb4f8a1164c8bba) )
	ROM_LOAD16_BYTE( "06f_h04.bin",  0x10001, 0x8000, CRC(c1486f61) SHA1(4b96aebe5d35fd1d73bde8576689addbb1ff66ed) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "03f_h08.bin",  0x0000, 0x0100, CRC(9f6ddf83) SHA1(08a37182a974c5448156637f10fe60bfe5f225ad) ) // palette red
	ROM_LOAD( "04f_h09.bin",  0x0100, 0x0100, CRC(e6773825) SHA1(7523e7fa090d850fe79ff0069d3260c76645d65a) ) // palette green
	ROM_LOAD( "05f_h10.bin",  0x0200, 0x0100, CRC(30a57860) SHA1(3ec7535286c8bc65e203320f47e4ed6f1d3d61c9) ) // palette blue
	ROM_LOAD( "10f_h12.bin",  0x0300, 0x0100, CRC(5eb33e73) SHA1(f34916dc4617b0c48e0a7ac6ace97b35dfcf1c40) ) // character lookup table
	ROM_LOAD( "10f_h11.bin",  0x0400, 0x0100, CRC(a63e37d8) SHA1(1a0a76ecd14310125bdf41a8431d562ed498eb27) ) // sprite lookup table
ROM_END

ROM_START( dairesya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "560-k03.13c",  0x4000, 0x8000, CRC(2ac6103b) SHA1(331e1be3f29df85d65081831c215743354d76778) ) // sldh
	ROM_LOAD( "560-k02.12c",  0xc000, 0x4000, CRC(07bc13a9) SHA1(1d3a44ad41799f89bfa84cc05fbe0792e57305af) ) // sldh

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "560-j01.10c",  0x0000, 0x4000, CRC(a203b223) SHA1(fd19ae55bda467a09151539be6dce3791c28f18a) )

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD16_BYTE( "560-j06.8f",   0x00000, 0x8000, CRC(a6e8248d) SHA1(7df653bb3a2257c249c3cf2c3f4f324d687a6b39) )
	ROM_LOAD16_BYTE( "560-j05.7f",   0x00001, 0x8000, CRC(f75893d4) SHA1(dc71b912d9bf5104dc633f687c52043df37852f0) )
	ROM_LOAD16_BYTE( "560-k07.9f",   0x10000, 0x8000, CRC(c8a1b840) SHA1(753b6fcbb4b28bbb63a392cdef90568734eac9bd) )
	ROM_LOAD16_BYTE( "560-k04.6f",   0x10001, 0x8000, CRC(c883d856) SHA1(4c4f91b72dab841ec15ca62121ed0c0878dfff23) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "03f_h08.bin",  0x0000, 0x0100, CRC(9f6ddf83) SHA1(08a37182a974c5448156637f10fe60bfe5f225ad) ) // palette red
	ROM_LOAD( "04f_h09.bin",  0x0100, 0x0100, CRC(e6773825) SHA1(7523e7fa090d850fe79ff0069d3260c76645d65a) ) // palette green
	ROM_LOAD( "05f_h10.bin",  0x0200, 0x0100, CRC(30a57860) SHA1(3ec7535286c8bc65e203320f47e4ed6f1d3d61c9) ) // palette blue
	ROM_LOAD( "10f_h12.bin",  0x0300, 0x0100, CRC(5eb33e73) SHA1(f34916dc4617b0c48e0a7ac6ace97b35dfcf1c40) ) // character lookup table
	ROM_LOAD( "10f_h11.bin",  0x0400, 0x0100, CRC(a63e37d8) SHA1(1a0a76ecd14310125bdf41a8431d562ed498eb27) ) // sprite lookup table
ROM_END

ROM_START( farwest )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "ironhors.008", 0x04000, 0x4000, CRC(b1c8246c) SHA1(4ceb098bb0b4efcbe50bb4b23bd27a60dabf2b3e) )
	ROM_LOAD( "ironhors.009", 0x08000, 0x8000, CRC(ea34ecfc) SHA1(8c7f12e76d2b9eb592ebf1bfd3e16a6b130da8e5) )
	ROM_LOAD( "ironhors.007", 0x00000, 0x2000, CRC(471182b7) SHA1(48ff58cbbf971b257e8099ec331397cf73dc8325) )   // don't know what this is for

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ironhors.010", 0x0000, 0x4000, CRC(a28231a6) SHA1(617e8fdf8129081c6a1bbbf140837a375a51da72) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "ironhors.005", 0x00000, 0x8000, CRC(f77e5b83) SHA1(6c72732dc96c1652713b2aba6f0a2410f9457818) )
	ROM_LOAD( "ironhors.006", 0x08000, 0x8000, CRC(7bbc0b51) SHA1(9b4890f2d20a8ddf5ba3f4325df070509252e06e) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "ironhors.001", 0x00000, 0x4000, CRC(a8fc21d3) SHA1(1e898aaccad1919bbacf8d7957f5a0761df20767) )
	ROM_LOAD( "ironhors.002", 0x04000, 0x4000, CRC(9c1e5593) SHA1(7d41d2224f0653e09d8728ccdec2df60f549e36e) )
	ROM_LOAD( "ironhors.003", 0x08000, 0x4000, CRC(3a0bf799) SHA1(b34d5c7edda06b8a579d6d390511781a43ffce83) )
	ROM_LOAD( "ironhors.004", 0x0c000, 0x4000, CRC(1fab18a3) SHA1(cc7ddf60b719e7c5a689f716ebee9bc04ade406a) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "ironcol.003",  0x0000, 0x0100, CRC(3e3fca11) SHA1(c92737659f063889a2b210cfe5c294b8a4864489) ) // palette red
	ROM_LOAD( "ironcol.001",  0x0100, 0x0100, CRC(dfb13014) SHA1(d9f9a5bed1300faf7c3864d5c5ae07087de25824) ) // palette green
	ROM_LOAD( "ironcol.002",  0x0200, 0x0100, CRC(77c88430) SHA1(e3041945b14955de109a505d9aa9f79046bed6a8) ) // palette blue
	ROM_LOAD( "10f_h12.bin",  0x0300, 0x0100, CRC(5eb33e73) SHA1(f34916dc4617b0c48e0a7ac6ace97b35dfcf1c40) ) // character lookup table
	ROM_LOAD( "ironcol.005",  0x0400, 0x0100, CRC(15077b9c) SHA1(c7fe24e3d481150452ff774f3908510db9e28367) ) // sprite lookup table
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/
// versions are taken from the letters on the program ROMs' labels
GAME( 1986, ironhors,  0,        ironhors, ironhors, ironhors_state, empty_init, ROT0, "Konami",                    "Iron Horse (version K)",               MACHINE_SUPPORTS_SAVE )
GAME( 1986, ironhorsh, ironhors, ironhors, ironhors, ironhors_state, empty_init, ROT0, "Konami",                    "Iron Horse (version H)",               MACHINE_SUPPORTS_SAVE )
GAME( 1986, dairesya,  ironhors, ironhors, dairesya, ironhors_state, empty_init, ROT0, "Konami (Kawakusu license)", "Dai Ressya Goutou (Japan, version K)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, farwest,   ironhors, farwest,  ironhors, farwest_state,  empty_init, ROT0, "bootleg?",                  "Far West",                             MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
