// license:BSD-3-Clause
// copyright-holders: Aaron Giles

/***************************************************************************

Time Pilot 84  (c) 1984 Konami

---- Master 6809 ------

Write
 2000-27ff MAFR Watch dog ?
 2800      COL0 a register that index the colors Proms
 3000      reset IRQ
 3001      OUT2  Coin Counter 2
 3002      OUT1  Coin Counter 1
 3003      MUT
 3004      HREV  Flip Screen X
 3005      VREV  Flip Screen Y
 3006      -
 3007      GMED
 3800      SON   Sound on
 3A00      SDA   Sound data
 3C00      SHF0 SHF1 J2 J3 J4 J5 J6 J7  background Y position
 3E00      L0 - L7                      background X position

Read:
 2800      in0  Buttons 1
 2820      in1  Buttons 2
 2840      in2  Buttons 3
 2860      in3  Dip switches 1
 3000      in4  Dip switches 2
 3800      in5  Dip switches 3 (not used)

Read/Write
 4000-47ff Char ram, 2 pages
 4800-4fff Background character ram, 2 pages
 5000-57ff Ram (Common for the Master and Slave 6809)  0x5000-0x517f sprites data
 6000-ffff Rom (only from $8000 to $ffff is used in this game)


------ Slave 6809 --------
 0000-1fff SAFR Watch dog ?
 2000      beam position
 4000      enable or reset IRQ
 6000-67ff DRA
 8000-87ff Ram (Common for the Master and Slave 6809)
 E000-ffff Rom


------ Sound CPU (Z80) -----
There are 3 or 4 76489AN chips driven by the Z80

0000-1fff Rom program (A6)
2000-3fff Rom Program (A4) (not used or missing?)
4000-43ff Ram
6000-7fff Sound data in
8000-9fff Timer
A000-Bfff Filters
C000      Store Data that will go to one of the 76489AN
C001      76489 #1 trigger
C002      76489 #2 (optional) trigger
C003      76489 #3 trigger
C004      76489 #4 trigger

***************************************************************************/

#include "emu.h"

#include "konamipt.h"

#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/flt_rc.h"
#include "sound/sn76496.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class tp84_state : public driver_device
{
public:
	tp84_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "cpu1"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_filter(*this, "filter%u", 1U),
		m_palette_bank(*this, "palette_bank"),
		m_scroll_x(*this, "scroll_x"),
		m_scroll_y(*this, "scroll_y"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_colorram(*this, "bg_colorram"),
		m_fg_colorram(*this, "fg_colorram"),
		m_spriteram(*this, "spriteram")
	{ }

	void tp84(machine_config &config);
	void tp84b(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device_array<filter_rc_device, 3> m_filter;

	required_shared_ptr<uint8_t> m_palette_bank;
	required_shared_ptr<uint8_t> m_scroll_x;
	required_shared_ptr<uint8_t> m_scroll_y;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_bg_colorram;
	required_shared_ptr<uint8_t> m_fg_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	bool m_flipscreen_x = false;
	bool m_flipscreen_y = false;

	bool m_irq_enable = false;
	bool m_sub_irq_mask = false;

	void irq_enable_w(int state);
	template <uint8_t Which> void coin_counter_w(int state);
	void flip_screen_x_w(int state);
	void flip_screen_y_w(int state);
	uint8_t sh_timer_r();
	void filter_w(offs_t offset, uint8_t data);
	void sh_irqtrigger_w(uint8_t data);
	void sub_irq_mask_w(uint8_t data);
	void spriteram_w(offs_t offset, uint8_t data);
	uint8_t scanline_r();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void audio_map(address_map &map) ATTR_COLD;
	void cpu2_map(address_map &map) ATTR_COLD;
	void tp84_cpu1_map(address_map &map) ATTR_COLD;
	void tp84b_cpu1_map(address_map &map) ATTR_COLD;
};


/*
-The colortable is divided in 2 part:
 -The characters colors
 -The sprites colors

-The characters colors are indexed like this:
 -2 bits from the characters
 -4 bits from the attribute in m_bg_colorram
 -2 bits from m_palette_bank (d3-d4)
 -3 bits from m_palette_bank (d0-d1-d2)
-So, there is 2048 bytes for the characters

-The sprites colors are indexed like this:
 -4 bits from the sprites (16 colors)
 -4 bits from the attribute of the sprites
 -3 bits from m_palette_bank (d0-d1-d2)
-So, there is 2048 bytes for the sprites

*/
/*
     The RGB signals are generated by 3 PROMs 256X4 (prom 2C, 2D and 1E)
        The resistors values are:
            1K  ohm
            470 ohm
            220 ohm
            100 ohm
*/

void tp84_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances[4] = { 1000, 470, 220, 100 };

	// compute the color output resistor weights
	double weights[4];
	compute_resistor_weights(0, 255, -1.0,
			4, resistances, weights, 470, 0,
			0, nullptr, nullptr, 0, 0,
			0, nullptr, nullptr, 0, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i | 0x000], 0);
		bit1 = BIT(color_prom[i | 0x000], 1);
		bit2 = BIT(color_prom[i | 0x000], 2);
		bit3 = BIT(color_prom[i | 0x000], 3);
		int const r = combine_weights(weights, bit0, bit1, bit2, bit3);

		// green component
		bit0 = BIT(color_prom[i | 0x100], 0);
		bit1 = BIT(color_prom[i | 0x100], 1);
		bit2 = BIT(color_prom[i | 0x100], 2);
		bit3 = BIT(color_prom[i | 0x100], 3);
		int const g = combine_weights(weights, bit0, bit1, bit2, bit3);

		// blue component
		bit0 = BIT(color_prom[i | 0x200], 0);
		bit1 = BIT(color_prom[i | 0x200], 1);
		bit2 = BIT(color_prom[i | 0x200], 2);
		bit3 = BIT(color_prom[i | 0x200], 3);
		int const b = combine_weights(weights, bit0, bit1, bit2, bit3);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x300;

	// characters use colors 0x80-0xff, sprites use colors 0-0x7f
	for (int i = 0; i < 0x200; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			uint8_t const ctabentry = ((~i & 0x100) >> 1) | (j << 4) | (color_prom[i] & 0x0f);
			palette.set_pen_indirect(((i & 0x100) << 3) | (j << 8) | (i & 0xff), ctabentry);
		}
	}
}


void tp84_state::spriteram_w(offs_t offset, uint8_t data)
{
	// the game multiplexes the sprites, so update now
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
	m_spriteram[offset] = data;
}


uint8_t tp84_state::scanline_r()
{
	// reads 1V - 128V
	return m_screen->vpos();
}


TILE_GET_INFO_MEMBER(tp84_state::get_bg_tile_info)
{
	int const code = ((m_bg_colorram[tile_index] & 0x30) << 4) | m_bg_videoram[tile_index];
	int const color = ((*m_palette_bank & 0x07) << 6) |
					  ((*m_palette_bank & 0x18) << 1) |
					  (m_bg_colorram[tile_index] & 0x0f);
	int const flags = TILE_FLIPYX(m_bg_colorram[tile_index] >> 6);

	tileinfo.set(0, code, color, flags);
}

TILE_GET_INFO_MEMBER(tp84_state::get_fg_tile_info)
{
	int const code = ((m_fg_colorram[tile_index] & 0x30) << 4) | m_fg_videoram[tile_index];
	int const color = ((*m_palette_bank & 0x07) << 6) |
					  ((*m_palette_bank & 0x18) << 1) |
					  (m_fg_colorram[tile_index] & 0x0f);
	int const flags = TILE_FLIPYX(m_fg_colorram[tile_index] >> 6);

	tileinfo.set(0, code, color, flags);
}


void tp84_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tp84_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tp84_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


void tp84_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const palette_base = ((*m_palette_bank & 0x07) << 4);

	for (int offs = 0x5c; offs >= 0; offs -= 4)
	{
		int const x = m_spriteram[offs];
		int const y = 240 - m_spriteram[offs + 3];

		int const code = m_spriteram[offs + 1];
		int const color = palette_base | (m_spriteram[offs + 2] & 0x0f);
		int const flip_x = BIT(~m_spriteram[offs + 2], 6);
		int const flip_y = BIT( m_spriteram[offs + 2], 7);

		m_gfxdecode->gfx(1)->transmask(bitmap, cliprect, code, color, flip_x, flip_y, x, y,
				m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, palette_base));

	}
}


uint32_t tp84_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip = cliprect;
	const rectangle &visarea = screen.visible_area();

	if (cliprect.min_y == screen.visible_area().min_y)
	{
		machine().tilemap().mark_all_dirty();

		m_bg_tilemap->set_scrollx(0, *m_scroll_x);
		m_bg_tilemap->set_scrolly(0, *m_scroll_y);

		machine().tilemap().set_flip_all((m_flipscreen_x ? TILEMAP_FLIPX : 0) |
										(m_flipscreen_y ? TILEMAP_FLIPY : 0));
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);

	// draw top status region
	clip.min_x = visarea.min_x;
	clip.max_x = visarea.min_x + 15;
	m_fg_tilemap->draw(screen, bitmap, clip, 0, 0);

	// draw bottom status region
	clip.min_x = visarea.max_x - 15;
	clip.max_x = visarea.max_x;
	m_fg_tilemap->draw(screen, bitmap, clip, 0, 0);

	return 0;
}


void tp84_state::machine_start()
{
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_sub_irq_mask));
	save_item(NAME(m_flipscreen_x));
	save_item(NAME(m_flipscreen_y));
}


void tp84_state::vblank_irq(int state)
{
	if (state && m_irq_enable)
		m_maincpu->set_input_line(0, ASSERT_LINE);
	if (state && m_sub_irq_mask)
		m_subcpu->set_input_line(0, ASSERT_LINE);
}


void tp84_state::irq_enable_w(int state)
{
	m_irq_enable = state;
	if (!m_irq_enable)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}


template <uint8_t Which>
void tp84_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}


void tp84_state::flip_screen_x_w(int state)
{
	m_flipscreen_x = state;
}


void tp84_state::flip_screen_y_w(int state)
{
	m_flipscreen_y = state;
}


uint8_t tp84_state::sh_timer_r()
{
	/* main xtal 14.318MHz, divided by 4 to get the CPU clock, further
	   divided by 2048 to get this timer
	   (divide by (2048/2), and not 1024, because the CPU cycle counter is
	   incremented every other state change of the clock) */
	return (m_audiocpu->total_cycles() / (2048/2)) & 0x0f;
}


void tp84_state::filter_w(offs_t offset, uint8_t data)
{
	// 76489 #0
	int C = 0;
	if (BIT(offset, 3)) C +=  47000;    //  47000pF = 0.047uF
	if (BIT(offset, 4)) C += 470000;    // 470000pF = 0.47uF
	m_filter[0]->filter_rc_set_RC(filter_rc_device::LOWPASS_3R, 1000, 2200, 1000, CAP_P(C));

	// 76489 #1 (optional)
	C = 0;
	if (BIT(offset, 5)) C +=  47000;    //  47000pF = 0.047uF
	if (BIT(offset, 6)) C += 470000;    // 470000pF = 0.47uF
	//m_filter[1]->filter_rc_set_RC(filter_rc_device::LOWPASS_3R, 1000, 2200, 1000, CAP_P(C));

	// 76489 #2
	C = 0;
	if (BIT(offset, 7)) C += 470000;    // 470000pF = 0.47uF
	m_filter[1]->filter_rc_set_RC(filter_rc_device::LOWPASS_3R, 1000, 2200, 1000, CAP_P(C));

	// 76489 #3
	C = 0;
	if (BIT(offset, 8)) C += 470000;    // 470000pF = 0.47uF
	m_filter[2]->filter_rc_set_RC(filter_rc_device::LOWPASS_3R, 1000, 2200, 1000, CAP_P(C));
}

void tp84_state::sh_irqtrigger_w(uint8_t data)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}



void tp84_state::tp84_cpu1_map(address_map &map)
{
	map(0x2000, 0x2000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x2800, 0x2800).portr("SYSTEM").writeonly().share(m_palette_bank);
	map(0x2820, 0x2820).portr("P1");
	map(0x2840, 0x2840).portr("P2");
	map(0x2860, 0x2860).portr("DSW1");
	map(0x3000, 0x3000).portr("DSW2");
	map(0x3000, 0x3007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x3800, 0x3800).w(FUNC(tp84_state::sh_irqtrigger_w));
	map(0x3a00, 0x3a00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x3c00, 0x3c00).writeonly().share(m_scroll_x);
	map(0x3e00, 0x3e00).writeonly().share(m_scroll_y);
	map(0x4000, 0x43ff).ram().share(m_bg_videoram);
	map(0x4400, 0x47ff).ram().share(m_fg_videoram);
	map(0x4800, 0x4bff).ram().share(m_bg_colorram);
	map(0x4c00, 0x4fff).ram().share(m_fg_colorram);
	map(0x5000, 0x57ff).ram().share("sharedram");
	map(0x8000, 0xffff).rom();
}

void tp84_state::tp84b_cpu1_map(address_map &map)
{
	map(0x0000, 0x03ff).ram().share(m_bg_videoram);
	map(0x0400, 0x07ff).ram().share(m_fg_videoram);
	map(0x0800, 0x0bff).ram().share(m_bg_colorram);
	map(0x0c00, 0x0fff).ram().share(m_fg_colorram);
	map(0x1000, 0x17ff).ram().share("sharedram");
	map(0x1800, 0x1800).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1a00, 0x1a00).portr("SYSTEM").writeonly().share(m_palette_bank);
	map(0x1a20, 0x1a20).portr("P1");
	map(0x1a40, 0x1a40).portr("P2");
	map(0x1a60, 0x1a60).portr("DSW1");
	map(0x1c00, 0x1c00).portr("DSW2");
	map(0x1c00, 0x1c07).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x1e00, 0x1e00).w(FUNC(tp84_state::sh_irqtrigger_w));
	map(0x1e80, 0x1e80).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x1f00, 0x1f00).writeonly().share(m_scroll_x);
	map(0x1f80, 0x1f80).writeonly().share(m_scroll_y);
	map(0x8000, 0xffff).rom();
}


void tp84_state::sub_irq_mask_w(uint8_t data)
{
	m_sub_irq_mask = data & 1;
	if (!m_sub_irq_mask)
		m_subcpu->set_input_line(0, CLEAR_LINE);
}


void tp84_state::cpu2_map(address_map &map)
{
//  map(0x0000, 0x0000).ram(); // Watch dog ?
	map(0x2000, 0x2000).r(FUNC(tp84_state::scanline_r)); // beam position
	map(0x4000, 0x4000).w(FUNC(tp84_state::sub_irq_mask_w));
	map(0x6000, 0x679f).ram();
	map(0x67a0, 0x67ff).ram().w(FUNC(tp84_state::spriteram_w)).share(m_spriteram);
	map(0x8000, 0x87ff).ram().share("sharedram");
	map(0xe000, 0xffff).rom();
}


void tp84_state::audio_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x6000, 0x6000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x8000, 0x8000).r(FUNC(tp84_state::sh_timer_r));
	map(0xa000, 0xa1ff).w(FUNC(tp84_state::filter_w));
	map(0xc000, 0xc000).nopw();
	map(0xc001, 0xc001).w("y2404_1", FUNC(y2404_device::write));
	map(0xc003, 0xc003).w("y2404_2", FUNC(y2404_device::write));
	map(0xc004, 0xc004).w("y2404_3", FUNC(y2404_device::write));
}



static INPUT_PORTS_START( tp84 )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_MONO_B12_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_B12_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	// "Invalid" = both coin slots disabled

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "10000 and every 50000" )
	PORT_DIPSETTING(    0x10, "20000 and every 60000" )
	PORT_DIPSETTING(    0x08, "30000 and every 70000" )
	PORT_DIPSETTING(    0x00, "40000 and every 80000" )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) ) // JP default
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )   // US default
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tp84a )
	PORT_INCLUDE( tp84 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ STEP4(0,1), STEP4(8*8,1) },
	{ STEP8(0,8) },
	8*8*2
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4 ,0 },
	{ STEP4(0,1), STEP4(8*8*1,1), STEP4(8*8*2,1), STEP4(8*8*3,1) },
	{ STEP8(0,8), STEP8(8*8*4,8) },
	16*16*2
};

static GFXDECODE_START( gfx_tp84 )
	GFXDECODE_ENTRY( "tiles",   0, charlayout,        0, 64*8 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 64*4*8, 16*8 )
GFXDECODE_END


void tp84_state::tp84(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, XTAL(18'432'000) / 12); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &tp84_state::tp84_cpu1_map);

	MC6809E(config, m_subcpu, XTAL(18'432'000) / 12);   // verified on PCB
	m_subcpu->set_addrmap(AS_PROGRAM, &tp84_state::cpu2_map);

	Z80(config, m_audiocpu, XTAL(14'318'181) / 4); // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &tp84_state::audio_map);

	config.set_maximum_quantum(attotime::from_hz(6000));  /* 100 CPU slices per frame - a high value to ensure proper
	                                                         synchronization of the CPUs */

	ls259_device &mainlatch(LS259(config, "mainlatch", 0)); // 3B
	mainlatch.q_out_cb<0>().set(FUNC(tp84_state::irq_enable_w));
	mainlatch.q_out_cb<1>().set(FUNC(tp84_state::coin_counter_w<1>));
	mainlatch.q_out_cb<2>().set(FUNC(tp84_state::coin_counter_w<0>));
	mainlatch.q_out_cb<4>().set(FUNC(tp84_state::flip_screen_x_w));
	mainlatch.q_out_cb<5>().set(FUNC(tp84_state::flip_screen_y_w));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(tp84_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(tp84_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tp84);
	PALETTE(config, m_palette, FUNC(tp84_state::palette), 4096, 256);

	// audio hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	Y2404(config, "y2404_1", XTAL(14'318'181) / 8).add_route(ALL_OUTPUTS, "filter1", 0.75); // verified on PCB

	Y2404(config, "y2404_2", XTAL(14'318'181) / 8).add_route(ALL_OUTPUTS, "filter2", 0.75); // verified on PCB

	Y2404(config, "y2404_3", XTAL(14'318'181) / 8).add_route(ALL_OUTPUTS, "filter3", 0.75); // verified on PCB

	for (auto &filter : m_filter)
		FILTER_RC(config, filter).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void tp84_state::tp84b(machine_config &config)
{
	tp84(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &tp84_state::tp84b_cpu1_map);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tp84 )
	ROM_REGION( 0x10000, "cpu1", ROMREGION_ERASE00 )
	ROM_LOAD( "388_f04.7j",  0x8000, 0x2000, CRC(605f61c7) SHA1(6848ef35ec7f92cccefb0fb2de42c4b0e9ec476f) )
	ROM_LOAD( "388_05.8j",   0xa000, 0x2000, CRC(4b4629a4) SHA1(f3bb1ee66c9e47d050370ac9ca74f3020cb9cfa3) )
	ROM_LOAD( "388_f06.9j",  0xc000, 0x2000, CRC(dbd5333b) SHA1(65dee1fd4c940a5423d57cb55a7f2ad89c59c5c6) )
	ROM_LOAD( "388_07.10j",  0xe000, 0x2000, CRC(a45237c4) SHA1(896e31c59aedf1c7e73e6f30fbe78cc020b457ab) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "388_f08.10d", 0xe000, 0x2000, CRC(36462ff1) SHA1(118a1b46ee01a583e6cf39af59b073321c76dbff) ) // E08?

	ROM_REGION( 0x4000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "388j13.6a",   0x0000, 0x2000, CRC(c44414da) SHA1(981289f5bdf7dc1348f4ca547ac933ef503b6588) )

	ROM_REGION( 0x4000, "tiles", 0 )
	ROM_LOAD( "388_h02.2j",  0x0000, 0x2000, CRC(05c7508f) SHA1(1a3c7cd47ad34e37a7b0f3014e10c055cbb2b559) )
	ROM_LOAD( "388_d01.1j",  0x2000, 0x2000, CRC(498d90b7) SHA1(6975f3a1603b14132aab58329195a4845a6e28bb) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "388_e09.12a", 0x0000, 0x2000, CRC(cd682f30) SHA1(6f48d3efc53d63171ec655e64b225412de1374e4) )
	ROM_LOAD( "388_e10.13a", 0x2000, 0x2000, CRC(888d4bd6) SHA1(7e2dde080bb614709561431a81b0490b2aaa42a9) )
	ROM_LOAD( "388_e11.14a", 0x4000, 0x2000, CRC(9a220b39) SHA1(792aaa4daedc8eb807d5a66d87da4641739b1660) )
	ROM_LOAD( "388_e12.15a", 0x6000, 0x2000, CRC(fac98397) SHA1(d90f99b19ab3cddfdfd37a273fb437be098088bc) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "388d14.2c",   0x0000, 0x0100, CRC(d737eaba) SHA1(e39026f87f5b995cf4a38b5d3d3fee7561762ae6) ) // palette red component
	ROM_LOAD( "388d15.2d",   0x0100, 0x0100, CRC(2f6a9a2a) SHA1(f09d8b92c7f9bf046cdd815c5282d0510e61b6e0) ) // palette green component
	ROM_LOAD( "388d16.1e",   0x0200, 0x0100, CRC(2e21329b) SHA1(9ba8af294dbd6f3a5d039c74a56e0605a913c037) ) // palette blue component
	ROM_LOAD( "388d18.1f",   0x0300, 0x0100, CRC(61d2d398) SHA1(3f74ad733b07b6a31cf9d4956d171eb9253dd6bf) ) // char lookup table
	ROM_LOAD( "388j17.16c",  0x0400, 0x0100, CRC(13c4e198) SHA1(42ab23206be99e840bd9c52cefa175c12fac8e5b) ) // sprite lookup table
ROM_END

ROM_START( tp84a )
	ROM_REGION( 0x10000, "cpu1", ROMREGION_ERASE00 )
	ROM_LOAD( "388_f04.7j",  0x8000, 0x2000, CRC(605f61c7) SHA1(6848ef35ec7f92cccefb0fb2de42c4b0e9ec476f) )
	ROM_LOAD( "388_f05.8j",  0xa000, 0x2000, CRC(e97d5093) SHA1(c76c119574d19d2ac10e6987150744542803ef5b) )
	ROM_LOAD( "388_f06.9j",  0xc000, 0x2000, CRC(dbd5333b) SHA1(65dee1fd4c940a5423d57cb55a7f2ad89c59c5c6) )
	ROM_LOAD( "388_f07.10j", 0xe000, 0x2000, CRC(8fbdb4ef) SHA1(e615c4d9964ab00f6776147c54925b4b6100b360) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "388_f08.10d", 0xe000, 0x2000, CRC(36462ff1) SHA1(118a1b46ee01a583e6cf39af59b073321c76dbff) ) // E08?

	ROM_REGION( 0x4000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "388j13.6a",   0x0000, 0x2000, CRC(c44414da) SHA1(981289f5bdf7dc1348f4ca547ac933ef503b6588) )

	ROM_REGION( 0x4000, "tiles", 0 )
	ROM_LOAD( "388_h02.2j",  0x0000, 0x2000, CRC(05c7508f) SHA1(1a3c7cd47ad34e37a7b0f3014e10c055cbb2b559) )
	ROM_LOAD( "388_d01.1j",  0x2000, 0x2000, CRC(498d90b7) SHA1(6975f3a1603b14132aab58329195a4845a6e28bb) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "388_e09.12a", 0x0000, 0x2000, CRC(cd682f30) SHA1(6f48d3efc53d63171ec655e64b225412de1374e4) )
	ROM_LOAD( "388_e10.13a", 0x2000, 0x2000, CRC(888d4bd6) SHA1(7e2dde080bb614709561431a81b0490b2aaa42a9) )
	ROM_LOAD( "388_e11.14a", 0x4000, 0x2000, CRC(9a220b39) SHA1(792aaa4daedc8eb807d5a66d87da4641739b1660) )
	ROM_LOAD( "388_e12.15a", 0x6000, 0x2000, CRC(fac98397) SHA1(d90f99b19ab3cddfdfd37a273fb437be098088bc) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "388d14.2c",   0x0000, 0x0100, CRC(d737eaba) SHA1(e39026f87f5b995cf4a38b5d3d3fee7561762ae6) ) // palette red component
	ROM_LOAD( "388d15.2d",   0x0100, 0x0100, CRC(2f6a9a2a) SHA1(f09d8b92c7f9bf046cdd815c5282d0510e61b6e0) ) // palette green component
	ROM_LOAD( "388d16.1e",   0x0200, 0x0100, CRC(2e21329b) SHA1(9ba8af294dbd6f3a5d039c74a56e0605a913c037) ) // palette blue component
	ROM_LOAD( "388d18.1f",   0x0300, 0x0100, CRC(61d2d398) SHA1(3f74ad733b07b6a31cf9d4956d171eb9253dd6bf) ) // char lookup table
	ROM_LOAD( "388d17.16c",  0x0400, 0x0100, CRC(af8f839c) SHA1(b469785a4153a221403fcf72c65c9c35ae75df5d) ) // sprite lookup table (dump miss?)
ROM_END

ROM_START( tp84b )
	ROM_REGION( 0x10000, "cpu1", ROMREGION_ERASE00 )
	// 0x6000 - 0x7fff space for diagnostic ROM
	ROM_LOAD( "388j05.8j",   0x8000, 0x4000, CRC(a59e2fda) SHA1(7d776d5d3fcfbe81d42580cfe93614dc4618a440) )
	ROM_LOAD( "388j07.10j",  0xc000, 0x4000, CRC(d25d18e6) SHA1(043f515cc66f6af004be81d6a6b5a92b553107ff) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "388j08.10d", 0xe000, 0x2000, CRC(2aea6b42) SHA1(58c3b4852f22a766f440b98904b73c00a31eae01) )

	ROM_REGION( 0x4000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "388j13.6a", 0x0000, 0x2000, CRC(c44414da) SHA1(981289f5bdf7dc1348f4ca547ac933ef503b6588) )

	ROM_REGION( 0x4000, "tiles", 0 )
	ROM_LOAD( "388j02.2j",  0x0000, 0x4000, CRC(e1225f53) SHA1(59d07dc4faafc82999e9716f0bba1cb7350c03e3) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "388j09.12a", 0x0000, 0x4000, CRC(aec90936) SHA1(3420c24bbedb140cb20fdaf51acbe9493830b64a) )
	ROM_LOAD( "388j11.14a", 0x4000, 0x4000, CRC(29257f03) SHA1(ebbb980bd226e8ada7e517e92487a32bfbc82f91) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "388j14.2c",  0x0000, 0x0100, CRC(d737eaba) SHA1(e39026f87f5b995cf4a38b5d3d3fee7561762ae6) ) // palette red component
	ROM_LOAD( "388j15.2d",  0x0100, 0x0100, CRC(2f6a9a2a) SHA1(f09d8b92c7f9bf046cdd815c5282d0510e61b6e0) ) // palette green component
	ROM_LOAD( "388j16.1e",  0x0200, 0x0100, CRC(2e21329b) SHA1(9ba8af294dbd6f3a5d039c74a56e0605a913c037) ) // palette blue component
	ROM_LOAD( "388j18.1f",  0x0300, 0x0100, CRC(61d2d398) SHA1(3f74ad733b07b6a31cf9d4956d171eb9253dd6bf) ) // char lookup table
	ROM_LOAD( "388j17.16c", 0x0400, 0x0100, CRC(13c4e198) SHA1(42ab23206be99e840bd9c52cefa175c12fac8e5b) ) // sprite lookup table
ROM_END

} // anonymous namespace


GAME( 1984, tp84,  0,    tp84,  tp84,  tp84_state, empty_init, ROT90, "Konami", "Time Pilot '84 (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, tp84a, tp84, tp84,  tp84a, tp84_state, empty_init, ROT90, "Konami", "Time Pilot '84 (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, tp84b, tp84, tp84b, tp84,  tp84_state, empty_init, ROT90, "Konami", "Time Pilot '84 (set 3)", MACHINE_SUPPORTS_SAVE )
