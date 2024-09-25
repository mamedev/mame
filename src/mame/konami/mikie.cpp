// license:BSD-3-Clause
// copyright-holders: Allard van der Bas

/***************************************************************************

    Mikie memory map (preliminary)
    GX469

    driver by Allard van der Bas


    MAIN BOARD:
    2800-288f Sprite RAM (288f, not 287f - quite unusual)
    3800-3bff Color RAM
    3c00-3fff Video RAM
    4000-5fff ROM (?)
    5ff0      Watchdog (?)
    6000-ffff ROM


Stephh's notes (based on the games M6809 code and some tests) :

  - To enter service mode, keep START1 and START2 pressed on reset.
    Then press START1 to cycle through the different tests.
  - According to code at 0x618f, you can start a game with 255 lives
    if you set DSW1 and DSW2 to the following settings :
      * "Coin A"      : "Free Play"
      * "Coin B"      : "No Coin B"
      * "Lives"       : "7"
      * "Cabinet"     : "Upright"
      * "Bonus Life"  : "20k 70k 50k+"
      * "Difficulty"  : "Medium"
      * "Demo Sounds" : "Off"
    DSW3 is not tested here, so settings can be anything.
  - I'm very surprised to notice that 'mikie' and 'mikiej' have the same
    PRG ROMS but different GFX ROMS.
    This is very rare (unique ?) for the Konami games.
  - There might exist undumped Konami/Centuri version(s) of this game :
    the manual I've found speaks about a "conversion kit".

***************************************************************************/

#include "emu.h"

#include "konamipt.h"

#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/sn76496.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class mikie_state : public driver_device
{
public:
	mikie_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void mikie(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_palettebank = 0;

	uint8_t m_irq_mask = 0;

	uint8_t sh_timer_r();
	void sh_irqtrigger_w(int state);
	template <uint8_t Which> void coin_counter_w(int state);
	void irq_mask_w(int state);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void palettebank_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mikie has three 256x4 palette PROMs (one per gun) and two 256x4 lookup
  table PROMs (one for characters, one for sprites).
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

void mikie_state::palette(palette_device &palette) const
{
	uint8_t const *color_prom = memregion("proms")->base();
	static constexpr int resistances[4] = { 2200, 1000, 470, 220 };

	// compute the color output resistor weights
	double rweights[4], gweights[4], bweights[4];
	compute_resistor_weights(0, 255, -1.0,
			4, resistances, rweights, 470, 0,
			4, resistances, gweights, 470, 0,
			4, resistances, bweights, 470, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i + 0x000], 0);
		bit1 = BIT(color_prom[i + 0x000], 1);
		bit2 = BIT(color_prom[i + 0x000], 2);
		bit3 = BIT(color_prom[i + 0x000], 3);
		int const r = combine_weights(rweights, bit0, bit1, bit2, bit3);

		// green component
		bit0 = BIT(color_prom[i + 0x100], 0);
		bit1 = BIT(color_prom[i + 0x100], 1);
		bit2 = BIT(color_prom[i + 0x100], 2);
		bit3 = BIT(color_prom[i + 0x100], 3);
		int const g = combine_weights(gweights, bit0, bit1, bit2, bit3);

		// blue component
		bit0 = BIT(color_prom[i + 0x200], 0);
		bit1 = BIT(color_prom[i + 0x200], 1);
		bit2 = BIT(color_prom[i + 0x200], 2);
		bit3 = BIT(color_prom[i + 0x200], 3);
		int const b = combine_weights(bweights, bit0, bit1, bit2, bit3);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table,
	color_prom += 0x300;

	// characters use colors 0x10-0x1f of each 0x20 color bank, while sprites use colors 0-0x0f
	for (int i = 0; i < 0x200; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			uint8_t const ctabentry = (j << 5) | ((~i & 0x100) >> 4) | (color_prom[i] & 0x0f);
			m_palette->set_pen_indirect(((i & 0x100) << 3) | (j << 8) | (i & 0xff), ctabentry);
		}
	}
}

void mikie_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void mikie_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void mikie_state::palettebank_w(uint8_t data)
{
	if (m_palettebank != (data & 0x07))
	{
		m_palettebank = data & 0x07;
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(mikie_state::get_bg_tile_info)
{
	int const code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x20) << 3);
	int const color = (m_colorram[tile_index] & 0x0f) + 16 * m_palettebank;
	int const flags = ((m_colorram[tile_index] & 0x40) ? TILE_FLIPX : 0) | ((m_colorram[tile_index] & 0x80) ? TILE_FLIPY : 0);
	if (m_colorram[tile_index] & 0x10)
		tileinfo.category = 1;
	else
		tileinfo.category = 0;

	tileinfo.set(0, code, color, flags);
}

void mikie_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mikie_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void mikie_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int const gfxbank = (m_spriteram[offs + 2] & 0x40) ? 2 : 1;
		int const code = (m_spriteram[offs + 2] & 0x3f) + ((m_spriteram[offs + 2] & 0x80) >> 1) + ((m_spriteram[offs] & 0x40) << 1);
		int const color = (m_spriteram[offs] & 0x0f) + 16 * m_palettebank;
		int const sx = m_spriteram[offs + 3];
		int sy = 244 - m_spriteram[offs + 1];
		int const flipx = ~m_spriteram[offs] & 0x10;
		int flipy = m_spriteram[offs] & 0x20;

		if (flip_screen())
		{
			sy = 242 - sy;
			flipy = !flipy;
		}


		m_gfxdecode->gfx(gfxbank)->transpen(bitmap, cliprect,
		code, color,
		flipx, flipy,
		sx, sy, 0);
	}
}

uint32_t mikie_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0), 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1), 0);
	return 0;
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

uint8_t mikie_state::sh_timer_r()
{
	static constexpr int MIKIE_TIMER_RATE = 512;

	int const clock = m_audiocpu->total_cycles() / MIKIE_TIMER_RATE;

	return clock;
}

void mikie_state::sh_irqtrigger_w(int state)
{
	if (state)
	{
		// setting bit 0 low then high triggers IRQ on the sound CPU
		m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
	}
}

template <uint8_t Which>
void mikie_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

void mikie_state::irq_mask_w(int state)
{
	m_irq_mask = state;
	if (!m_irq_mask)
		m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void mikie_state::main_map(address_map &map)
{
	map(0x0000, 0x00ff).ram();
	map(0x2000, 0x2007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x2100, 0x2100).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x2200, 0x2200).w(FUNC(mikie_state::palettebank_w));
	map(0x2300, 0x2300).nopw();    // ???
	map(0x2400, 0x2400).portr("SYSTEM").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x2401, 0x2401).portr("P1");
	map(0x2402, 0x2402).portr("P2");
	map(0x2403, 0x2403).portr("DSW3");
	map(0x2500, 0x2500).portr("DSW1");
	map(0x2501, 0x2501).portr("DSW2");
	map(0x2800, 0x288f).ram().share(m_spriteram);
	map(0x2890, 0x37ff).ram();
	map(0x3800, 0x3bff).ram().w(FUNC(mikie_state::colorram_w)).share(m_colorram);
	map(0x3c00, 0x3fff).ram().w(FUNC(mikie_state::videoram_w)).share(m_videoram);
	map(0x4000, 0x5fff).rom(); // Machine checks for extra ROM
	map(0x6000, 0xffff).rom();
}

void mikie_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x8000, 0x8000).nopw();    // sound command latch
	map(0x8001, 0x8001).nopw();    // ???
	map(0x8002, 0x8002).w("sn1", FUNC(sn76489a_device::write)); // trigger read of latch
	map(0x8003, 0x8003).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x8004, 0x8004).w("sn2", FUNC(sn76489a_device::write)); // trigger read of latch
	map(0x8005, 0x8005).r(FUNC(mikie_state::sh_timer_r));
	map(0x8079, 0x8079).nopw();    // ???
	map(0xa003, 0xa003).nopw();    // ???
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

// verified from M6809 code
static INPUT_PORTS_START( mikie )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_MONO_4WAY_B12_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_4WAY_B12_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	// "No Coin B" = coins produce sound, but no effect on coin counter

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k 70k 50k+" )
	PORT_DIPSETTING(    0x10, "30K 90k 60k+" )
	PORT_DIPSETTING(    0x08, "30k only" )
	PORT_DIPSETTING(    0x00, "40K only" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )             /* 1 */
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )           /* 2 */
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )             /* 3 */
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )          /* 4 */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// DSW3 is not mounted on PCB nor listed in manual
	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_DIPUNUSED( 0x04, 0x04 )
	PORT_DIPUNUSED( 0x08, 0x08 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16,16,       /* 16*16 sprites */
	256,            /* 256 sprites */
	4,             /* 4 bits per pixel */
	{ 0, 4, 256*128*8+0, 256*128*8+4 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			0, 1, 2, 3, 48*8+0, 48*8+1, 48*8+2, 48*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			32*16, 33*16, 34*16, 35*16, 36*16, 37*16, 38*16, 39*16 },
	128*8   /* every sprite takes 64 bytes */
};

static GFXDECODE_START( gfx_mikie )
	GFXDECODE_ENTRY( "tiles",   0x0000, gfx_8x8x4_packed_msb,       0, 16*8 )
	GFXDECODE_ENTRY( "sprites", 0x0000, spritelayout,         16*8*16, 16*8 )
	GFXDECODE_ENTRY( "sprites", 0x0001, spritelayout,         16*8*16, 16*8 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mikie_state::machine_start()
{
	save_item(NAME(m_palettebank));
	save_item(NAME(m_irq_mask));
}

void mikie_state::machine_reset()
{
	m_palettebank = 0;
}

void mikie_state::vblank_irq(int state)
{
	if (state && m_irq_mask)
		m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

void mikie_state::mikie(machine_config &config)
{
	static constexpr XTAL AUDIO_XTAL = XTAL(14'318'181);
	static constexpr XTAL OSC = XTAL(18'432'000);
	static constexpr XTAL CLK = AUDIO_XTAL / 4;

	// basic machine hardware
	MC6809E(config, m_maincpu, OSC / 12); // 9A (surface scratched)
	m_maincpu->set_addrmap(AS_PROGRAM, &mikie_state::main_map);

	Z80(config, m_audiocpu, CLK); // 4E (surface scratched)
	m_audiocpu->set_addrmap(AS_PROGRAM, &mikie_state::sound_map);

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 6I
	mainlatch.q_out_cb<0>().set(FUNC(mikie_state::coin_counter_w<0>)); // COIN1
	mainlatch.q_out_cb<1>().set(FUNC(mikie_state::coin_counter_w<1>)); // COIN2
	mainlatch.q_out_cb<2>().set(FUNC(mikie_state::sh_irqtrigger_w)); // SOUNDON
	mainlatch.q_out_cb<3>().set_nop(); // END (not used?)
	mainlatch.q_out_cb<6>().set(FUNC(mikie_state::flip_screen_set)); // FLIP
	mainlatch.q_out_cb<7>().set(FUNC(mikie_state::irq_mask_w)); // INT

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60.59);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(mikie_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(mikie_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mikie);
	PALETTE(config, m_palette, FUNC(mikie_state::palette), 16*8*16+16*8*16, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	SN76489A(config, "sn1", AUDIO_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.60);

	SN76489A(config, "sn2", CLK).add_route(ALL_OUTPUTS, "mono", 0.60);
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mikie )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "n14.11c", 0x6000, 0x2000, CRC(f698e6dd) SHA1(99220eeee4e7b88caa26f2d08502689e1f1fcdf8) )
	ROM_LOAD( "o13.12a", 0x8000, 0x4000, CRC(826e7035) SHA1(bd62783cb1ba4e7f0196f337280461bb7627f70f) )
	ROM_LOAD( "o17.12d", 0xc000, 0x4000, CRC(161c25c8) SHA1(373a92b8412676ad9870cb562c73e47db7b40bea) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "n10.6e",  0x0000, 0x2000, CRC(2cf9d670) SHA1(b324b92aff70d7878160128611dd5fdec6949659) )

	ROM_REGION( 0x4000, "tiles", 0 )
	ROM_LOAD( "o11.8i",  0x0000, 0x4000, CRC(3c82aaf3) SHA1(c84256ac5fd5e40b197651c56e303c69aae72950) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "001.f1",  0x0000, 0x4000, CRC(a2ba0df5) SHA1(873d49c1c2efbb222d1bf63396729d4b7d9477c3) )
	ROM_LOAD( "003.f3",  0x4000, 0x4000, CRC(9775ab32) SHA1(0271567c5f5a6bb2eaffb9d5dc2af6b8142dc8a9) )
	ROM_LOAD( "005.h1",  0x8000, 0x4000, CRC(ba44aeef) SHA1(410bfd9146242254a920092e280af87586709527) )
	ROM_LOAD( "007.h3",  0xc000, 0x4000, CRC(31afc153) SHA1(31ca33f585fddb86131ef61de73e3563fa027455) )

	ROM_REGION( 0x500, "proms", 0 )
	ROM_LOAD( "d19.1i",  0x0000, 0x0100, CRC(8b83e7cf) SHA1(4fce779947f9f318023c7c54a71a4751f6bb8eb1) ) // red component
	ROM_LOAD( "d21.3i",  0x0100, 0x0100, CRC(3556304a) SHA1(6f4fc3ef6b1b44278e7c8c1034ee4fbef90cf85a) ) // green component
	ROM_LOAD( "d20.2i",  0x0200, 0x0100, CRC(676a0669) SHA1(14236a831204d52cdf8c2ef318a565d6c5587ce0) ) // blue component
	ROM_LOAD( "d22.12h", 0x0300, 0x0100, CRC(872be05c) SHA1(1525303589d7ed909bc6e2827fbaa2c16ad4030b) ) // character lookup table
	ROM_LOAD( "d18.f9",  0x0400, 0x0100, CRC(7396b374) SHA1(fedcc421a61d6623dc9c41b0a3e164efeb50ec7c) ) // sprite lookup table
ROM_END

ROM_START( mikiej )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "n14.11c", 0x6000, 0x2000, CRC(f698e6dd) SHA1(99220eeee4e7b88caa26f2d08502689e1f1fcdf8) )
	ROM_LOAD( "o13.12a", 0x8000, 0x4000, CRC(826e7035) SHA1(bd62783cb1ba4e7f0196f337280461bb7627f70f) )
	ROM_LOAD( "o17.12d", 0xc000, 0x4000, CRC(161c25c8) SHA1(373a92b8412676ad9870cb562c73e47db7b40bea) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "n10.6e",  0x0000, 0x2000, CRC(2cf9d670) SHA1(b324b92aff70d7878160128611dd5fdec6949659) )

	ROM_REGION( 0x4000, "tiles", 0 )
	ROM_LOAD( "q11.8i",  0x0000, 0x4000, CRC(c48b269b) SHA1(d7fcfa44fcda90f1a7df6c974210716ae82c47a3) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "q01.f1",  0x0000, 0x4000, CRC(31551987) SHA1(b6cbdb8b511d99b27546a6c4d01f2948d5ad3a42) )
	ROM_LOAD( "q03.f3",  0x4000, 0x4000, CRC(34414df0) SHA1(0189deac6f19de386b4e49cfe6322b212e74264a) )
	ROM_LOAD( "q05.h1",  0x8000, 0x4000, CRC(f9e1ebb1) SHA1(c88c1fc22f21b3e7d558c47de2716dac01fdd621) )
	ROM_LOAD( "q07.h3",  0xc000, 0x4000, CRC(15dc093b) SHA1(0b5a5aea25283b8edb7f534fc84b13f5176e26d6) )

	ROM_REGION( 0x500, "proms", 0 )
	ROM_LOAD( "d19.1i",  0x0000, 0x0100, CRC(8b83e7cf) SHA1(4fce779947f9f318023c7c54a71a4751f6bb8eb1) ) // red component
	ROM_LOAD( "d21.3i",  0x0100, 0x0100, CRC(3556304a) SHA1(6f4fc3ef6b1b44278e7c8c1034ee4fbef90cf85a) ) // green component
	ROM_LOAD( "d20.2i",  0x0200, 0x0100, CRC(676a0669) SHA1(14236a831204d52cdf8c2ef318a565d6c5587ce0) ) // blue component
	ROM_LOAD( "d22.12h", 0x0300, 0x0100, CRC(872be05c) SHA1(1525303589d7ed909bc6e2827fbaa2c16ad4030b) ) // character lookup table
	ROM_LOAD( "d18.f9",  0x0400, 0x0100, CRC(7396b374) SHA1(fedcc421a61d6623dc9c41b0a3e164efeb50ec7c) ) // sprite lookup table
ROM_END

ROM_START( mikiek )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "n14.11c", 0x6000, 0x2000, CRC(f698e6dd) SHA1(99220eeee4e7b88caa26f2d08502689e1f1fcdf8) )
	ROM_LOAD( "o13.12a", 0x8000, 0x4000, CRC(826e7035) SHA1(bd62783cb1ba4e7f0196f337280461bb7627f70f) )
	ROM_LOAD( "o17.12d", 0xc000, 0x4000, CRC(161c25c8) SHA1(373a92b8412676ad9870cb562c73e47db7b40bea) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "n10.6e",  0x0000, 0x2000, CRC(2cf9d670) SHA1(b324b92aff70d7878160128611dd5fdec6949659) )

	ROM_REGION( 0x4000, "tiles", 0 )
	ROM_LOAD( "q11.8i",  0x0000, 0x4000, CRC(29286fce) SHA1(699706cf7300c98352e355f81ba40635b4380d7a) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "q01.f1",  0x0000, 0x4000, CRC(31551987) SHA1(b6cbdb8b511d99b27546a6c4d01f2948d5ad3a42) )
	ROM_LOAD( "q03.f3",  0x4000, 0x4000, CRC(707cc98e) SHA1(850c973053f3ae8a93e7c630d69298f25708941e) )
	ROM_LOAD( "q05.h1",  0x8000, 0x4000, CRC(f9e1ebb1) SHA1(c88c1fc22f21b3e7d558c47de2716dac01fdd621) )
	ROM_LOAD( "q07.h3",  0xc000, 0x4000, CRC(44502ca9) SHA1(452e38512a4463602d98301f660dc7bf662e49f4) )

	ROM_REGION( 0x500, "proms", 0 )
	ROM_LOAD( "d19.1i",  0x0000, 0x0100, CRC(8b83e7cf) SHA1(4fce779947f9f318023c7c54a71a4751f6bb8eb1) ) // red component
	ROM_LOAD( "d21.3i",  0x0100, 0x0100, CRC(3556304a) SHA1(6f4fc3ef6b1b44278e7c8c1034ee4fbef90cf85a) ) // green component
	ROM_LOAD( "d20.2i",  0x0200, 0x0100, CRC(676a0669) SHA1(14236a831204d52cdf8c2ef318a565d6c5587ce0) ) // blue component
	ROM_LOAD( "d22.12h", 0x0300, 0x0100, CRC(872be05c) SHA1(1525303589d7ed909bc6e2827fbaa2c16ad4030b) ) // character lookup table
	ROM_LOAD( "d18.f9",  0x0400, 0x0100, CRC(7396b374) SHA1(fedcc421a61d6623dc9c41b0a3e164efeb50ec7c) ) // sprite lookup table
ROM_END

ROM_START( mikiehs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "l14.11c", 0x6000, 0x2000, CRC(633f3a6d) SHA1(9255e0cb8d53773a52cade2fbd2e4c1968164313) )
	ROM_LOAD( "m13.12a", 0x8000, 0x4000, CRC(9c42d715) SHA1(533ae7c5bde6d341b9138dd439d9ff46fe0767f4) )
	ROM_LOAD( "m17.12d", 0xc000, 0x4000, CRC(cb5c03c9) SHA1(6bc2f4c5f4f97c2bbcac8ff51358369ce07948e9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "h10.6e",  0x0000, 0x2000, CRC(4ed887d2) SHA1(953218a3b41019e2e52932dd3522741812c46c75) )

	ROM_REGION( 0x4000, "tiles", 0 )
	ROM_LOAD( "l11.8i",  0x0000, 0x4000, CRC(5ba9d86b) SHA1(2246795dd68a62efb2c70a9177ee97a58ccb2566) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "i01.f1",  0x0000, 0x4000, CRC(0c0cab5f) SHA1(c3eb4c3a432e86f4664329a0de5583cb5de7b6f5) )
	ROM_LOAD( "i03.f3",  0x4000, 0x4000, CRC(694da32f) SHA1(02bd83d77f42822e42e48977856dfa0e3abfcab0) )
	ROM_LOAD( "i05.h1",  0x8000, 0x4000, CRC(00e357e1) SHA1(d5b46709083d74950d0deedecb4fd631d0e74afb) )
	ROM_LOAD( "i07.h3",  0xc000, 0x4000, CRC(ceeba6ac) SHA1(ca5f715dece88540d9ed0e0146cff09f6868d09f) )

	ROM_REGION( 0x500, "proms", 0 )
	ROM_LOAD( "d19.1i",  0x0000, 0x0100, CRC(8b83e7cf) SHA1(4fce779947f9f318023c7c54a71a4751f6bb8eb1) ) // red component
	ROM_LOAD( "d21.3i",  0x0100, 0x0100, CRC(3556304a) SHA1(6f4fc3ef6b1b44278e7c8c1034ee4fbef90cf85a) ) // green component
	ROM_LOAD( "d20.2i",  0x0200, 0x0100, CRC(676a0669) SHA1(14236a831204d52cdf8c2ef318a565d6c5587ce0) ) // blue component
	ROM_LOAD( "d22.12h", 0x0300, 0x0100, CRC(872be05c) SHA1(1525303589d7ed909bc6e2827fbaa2c16ad4030b) ) // character lookup table
	ROM_LOAD( "d18.f9",  0x0400, 0x0100, CRC(7396b374) SHA1(fedcc421a61d6623dc9c41b0a3e164efeb50ec7c) ) // sprite lookup table
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1984, mikie,   0,     mikie, mikie, mikie_state, empty_init, ROT270, "Konami",  "Mikie",                        MACHINE_SUPPORTS_SAVE )
GAME( 1984, mikiej,  mikie, mikie, mikie, mikie_state, empty_init, ROT270, "Konami",  "Shinnyuushain Tooru-kun",      MACHINE_SUPPORTS_SAVE )
GAME( 1984, mikiek,  mikie, mikie, mikie, mikie_state, empty_init, ROT270, "bootleg", "Shin-ip Sawon - Seok Dol-i",   MACHINE_SUPPORTS_SAVE ) // 新入社員 - 石돌이 (신입사원 - 석돌이)
GAME( 1984, mikiehs, mikie, mikie, mikie, mikie_state, empty_init, ROT270, "Konami",  "Mikie (High School Graffiti)", MACHINE_SUPPORTS_SAVE )
