// license:BSD-3-Clause
// copyright-holders: Zsolt Vasvari

/***************************************************************************

Super Basketball memory map (preliminary)
(Hold down Start 1 & Start 2 keys to enter test mode on start up;
 use Start 1 to change modes)

driver by Zsolt Vasvari

MAIN BOARD:
2000-2fff RAM
3000-33ff Color RAM
3400-37ff Video RAM
3800-39ff Sprite RAM
6000-ffff ROM


Konami designated Super Basketball with the label of GX405.

Super Basketball is composed of two boards.  A Sound board with the
label PWB(B)3000288A and a CPU/Video board with the label
PWB(A)2000177C silked screened onto them.  All Konami custom chips have
had their labels scratched off.

Sound Board Parts:

    VLM5030 @ 11e (According to the schematics, part number scratched off)
    Z80 @ 6a (According to the schematics, part number scratched off)
    14.31818MHz @ x1
    3.579545MHz @ x2
    SN76489AN @ 8d (According to the schematics, part number scratched off)
    4118 or 6116 @ 10a (According to the schematics pins match these two types of SRAM, part number scratched off)


CPU/Video Board Parts:

    18.432000MHz @ 1f
    M2BC200 (CR2032) Battery @ 19j

***************************************************************************/

#include "emu.h"

#include "konami1.h"
#include "konamipt.h"
#include "trackfld_a.h"

#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/dac.h"
#include "sound/sn76496.h"
#include "sound/vlm5030.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class sbasketb_state : public driver_device
{
public:
	sbasketb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_palettebank(*this, "palettebank"),
		m_scroll(*this, "scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_sn(*this, "snsnd"),
		m_vlm(*this, "vlm"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void sbasketb(machine_config &config);
	void sbasketbu(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_palettebank;
	required_shared_ptr<uint8_t> m_scroll;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<sn76489_device> m_sn;
	required_device<vlm5030_device> m_vlm;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	bool m_spriteram_select = false;

	bool m_irq_mask = false;
	uint8_t m_sn76496_latch = 0;

	void sh_irqtrigger_w(uint8_t data);
	template <uint8_t Which> void coin_counter_w(int state);
	void irq_mask_w(int state);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void spriteram_select_w(int state);
	void konami_sn76496_latch_w(uint8_t data) { m_sn76496_latch = data; }
	void konami_sn76496_w(uint8_t data) { m_sn->write(m_sn76496_latch); }
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

  Super Basketball has three 256x4 palette PROMs (one per gun) and two 256x4
  lookup table PROMs (one for characters, one for sprites).
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

void sbasketb_state::palette(palette_device &palette) const
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

	// characters use colors 0xf0-0xff
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x0f) | 0xf0;
		palette.set_pen_indirect(i, ctabentry);
	}

	// sprites use colors 0-256 (?) in 16 banks
	for (int i = 0; i < 0x100; i++)
	{
		for (int j = 0; j < 0x10; j++)
		{
			uint8_t const ctabentry = (j << 4) | (color_prom[i + 0x100] & 0x0f);
			palette.set_pen_indirect(0x100 + ((j << 8) | i), ctabentry);
		}
	}
}

void sbasketb_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void sbasketb_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void sbasketb_state::spriteram_select_w(int state)
{
	m_spriteram_select = state;
}

TILE_GET_INFO_MEMBER(sbasketb_state::get_bg_tile_info)
{
	int const code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x20) << 3);
	int const color = m_colorram[tile_index] & 0x0f;
	int const flags = ((m_colorram[tile_index] & 0x40) ? TILE_FLIPX : 0) | ((m_colorram[tile_index] & 0x80) ? TILE_FLIPY : 0);

	tileinfo.set(0, code, color, flags);
}

void sbasketb_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sbasketb_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_cols(32);

	save_item(NAME(m_spriteram_select));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_sn76496_latch));
}

void sbasketb_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs = m_spriteram_select ? 0x100 : 0;

	for (int i = 0; i < 64; i++, offs += 4)
	{
		int sx = m_spriteram[offs + 2];
		int sy = m_spriteram[offs + 3];

		if (sx || sy)
		{
			int const code = m_spriteram[offs + 0] | ((m_spriteram[offs + 1] & 0x20) << 3);
			int const color = (m_spriteram[offs + 1] & 0x0f) + 16 * *m_palettebank;
			int flipx = m_spriteram[offs + 1] & 0x40;
			int flipy = m_spriteram[offs + 1] & 0x80;

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}


				m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code, color,
				flipx, flipy,
				sx, sy, 0);
		}
	}
}

uint32_t sbasketb_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int col = 6; col < 32; col++)
		m_bg_tilemap->set_scrolly(col, *m_scroll);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void sbasketb_state::sh_irqtrigger_w(uint8_t data)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

template <uint8_t Which>
void sbasketb_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

void sbasketb_state::irq_mask_w(int state)
{
	m_irq_mask = state;
	if (!m_irq_mask)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void sbasketb_state::main_map(address_map &map)
{
	map(0x2000, 0x2fff).ram();
	map(0x3000, 0x33ff).ram().w(FUNC(sbasketb_state::colorram_w)).share(m_colorram);
	map(0x3400, 0x37ff).ram().w(FUNC(sbasketb_state::videoram_w)).share(m_videoram);
	map(0x3800, 0x39ff).ram().share(m_spriteram);
	map(0x3a00, 0x3bff).ram();           // Probably unused, but initialized
	map(0x3c00, 0x3c00).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x3c10, 0x3c10).nopr();    // ????
	map(0x3c20, 0x3c20).writeonly().share(m_palettebank);
	map(0x3c80, 0x3c87).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x3d00, 0x3d00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x3d80, 0x3d80).w(FUNC(sbasketb_state::sh_irqtrigger_w));
	map(0x3e00, 0x3e00).portr("SYSTEM");
	map(0x3e01, 0x3e01).portr("P1");
	map(0x3e02, 0x3e02).portr("P2");
	map(0x3e03, 0x3e03).nopr();
	map(0x3e80, 0x3e80).portr("DSW2");
	map(0x3f00, 0x3f00).portr("DSW1");
	map(0x3f80, 0x3f80).writeonly().share(m_scroll);
	map(0x6000, 0xffff).rom();
}

void sbasketb_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x6000, 0x6000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x8000, 0x8000).r("soundbrd", FUNC(trackfld_audio_device::hyperspt_sh_timer_r));
	map(0xa000, 0xa000).w(m_vlm, FUNC(vlm5030_device::data_w)); // speech
	map(0xc000, 0xdfff).w("soundbrd", FUNC(trackfld_audio_device::hyperspt_sound_w));     // speech and output control
	map(0xe000, 0xe000).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe001, 0xe001).w(FUNC(sbasketb_state::konami_sn76496_latch_w));  // Loads the snd command into the snd latch
	map(0xe002, 0xe002).w(FUNC(sbasketb_state::konami_sn76496_w));      // This address triggers the SN chip to read the data port.
}


static INPUT_PORTS_START( sbasketb )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_MONO_B123_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_B123_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( Free_Play ), SW1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Game_Time ) )   PORT_DIPLOCATION( "SW2:1,2" )
	PORT_DIPSETTING(    0x03, "30" )
	PORT_DIPSETTING(    0x01, "40" )
	PORT_DIPSETTING(    0x02, "50" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )     PORT_DIPLOCATION( "SW2:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, "Starting Score" )       PORT_DIPLOCATION( "SW2:4" )
	PORT_DIPSETTING(    0x08, "70-78" )
	PORT_DIPSETTING(    0x00, "100-115" )
	PORT_DIPNAME( 0x10, 0x00, "Ranking" )              PORT_DIPLOCATION( "SW2:5" )
	PORT_DIPSETTING(    0x00, "Data Remaining" )
	PORT_DIPSETTING(    0x10, "Data Initialized" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )  PORT_DIPLOCATION( "SW2:6,7" )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION( "SW2:8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END




static GFXDECODE_START( gfx_sbasketb )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_packed_msb, 0, 16 )
	GFXDECODE_ENTRY( "sprites", 0, gfx_16x16x4_packed_msb, 16*16, 16*16 )
GFXDECODE_END

void sbasketb_state::vblank_irq(int state)
{
	if (state && m_irq_mask)
		m_maincpu->set_input_line(0, HOLD_LINE);
}

void sbasketb_state::sbasketb(machine_config &config)
{
	// basic machine hardware
	KONAMI1(config, m_maincpu, 1'400'000);        // 1.400 MHz ??? TODO: From a 18.432 MHz XTAL this doesn't seem probable
	m_maincpu->set_addrmap(AS_PROGRAM, &sbasketb_state::main_map);

	Z80(config, m_audiocpu, XTAL(14'318'181) / 4); // 3.5795 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &sbasketb_state::sound_map);

	ls259_device &mainlatch(LS259(config, "mainlatch")); // B3
	mainlatch.q_out_cb<0>().set(FUNC(sbasketb_state::flip_screen_set)); // FLIP
	mainlatch.q_out_cb<1>().set(FUNC(sbasketb_state::irq_mask_w)); // INTST
	mainlatch.q_out_cb<2>().set_nop(); // MUT - not used?
	mainlatch.q_out_cb<3>().set(FUNC(sbasketb_state::coin_counter_w<0>)); // COIN 1
	mainlatch.q_out_cb<4>().set(FUNC(sbasketb_state::coin_counter_w<1>)); // COIN 2
	mainlatch.q_out_cb<5>().set(FUNC(sbasketb_state::spriteram_select_w)); // OBJ CHE
	mainlatch.q_out_cb<6>().set_nop(); // END - not used

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(sbasketb_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(sbasketb_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sbasketb);
	PALETTE(config, m_palette, FUNC(sbasketb_state::palette), 16*16+16*16*16, 256);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	TRACKFLD_AUDIO(config, "soundbrd", 0, m_audiocpu, m_vlm);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.4); // unknown DAC

	SN76489(config, m_sn, XTAL(14'318'181) / 8).add_route(ALL_OUTPUTS, "speaker", 1.0);

	VLM5030(config, m_vlm, XTAL(3'579'545)).add_route(ALL_OUTPUTS, "speaker", 1.0); // Schematics say 3.58MHz, but board uses 3.579545MHz xtal
}

void sbasketb_state::sbasketbu(machine_config &config)
{
	sbasketb(config);
	MC6809E(config.replace(), m_maincpu, 1'400'000);        // 6809E at 1.400 MHz ???
	m_maincpu->set_addrmap(AS_PROGRAM, &sbasketb_state::main_map);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/


/*
    Super Basketball (version I, encrypted)
*/

ROM_START( sbasketb )
	ROM_REGION( 0x10000, "maincpu", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405g05.14j", 0x6000, 0x2000, CRC(336dc0ab) SHA1(0fe47fdbf183683c569785fc6b980337a9cfde95) )
	ROM_LOAD( "405i03.11j", 0x8000, 0x4000, CRC(d33b82dd) SHA1(9f0a1e2b0a43a2ec5029dd50dbd315291838fa39) )
	ROM_LOAD( "405i01.9j",  0xc000, 0x4000, CRC(1c09cc3f) SHA1(881c0a9313f7f1ca17e1fa956a7b13e77d71957c) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // ROMs located on Sound Board
	ROM_LOAD( "405e13.7a",  0x0000, 0x2000, CRC(1ec7458b) SHA1(a015b982bff5f9e7ece33f2e69ff8c6c2174e710) )

	ROM_REGION( 0x04000, "tiles", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405e12.22f", 0x0000, 0x4000, CRC(e02c54da) SHA1(2fa19f3bce894ef05820f95e0b88428e4f946a35) )

	ROM_REGION( 0x0c000, "sprites", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405h06.14g", 0x0000, 0x4000, CRC(cfbbff07) SHA1(39b19866b21372524933b5eef511bb5b7ad92556) )
	ROM_LOAD( "405h08.17g", 0x4000, 0x4000, CRC(c75901b6) SHA1(4ff87123228da068f0c0ffffa4a3f03765eccd8d) )
	ROM_LOAD( "405h10.20g", 0x8000, 0x4000, CRC(95bc5942) SHA1(55bf35283385d0ae768210706720a3b289ebd9a2) )

	ROM_REGION( 0x0500, "proms", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405e17.5a",  0x0000, 0x0100, CRC(b4c36d57) SHA1(c4a63f57edce2b9588e2394ff54a28f91213d550) ) // palette red component
	ROM_LOAD( "405e16.4a",  0x0100, 0x0100, CRC(0b7b03b8) SHA1(81297cb2b0b28b0fc0939a37ff30844d69fb65ac) ) // palette green component
	ROM_LOAD( "405e18.6a",  0x0200, 0x0100, CRC(9e533bad) SHA1(611e7af6813caaf2bc36c311ae48a5efd30e6f0c) ) // palette blue component
	ROM_LOAD( "405e20.19d", 0x0300, 0x0100, CRC(8ca6de2f) SHA1(67d29708d1a07d17c5dc5793a3293e7ace3a4e19) ) // character lookup table
	ROM_LOAD( "405e19.16d", 0x0400, 0x0100, CRC(e0bc782f) SHA1(9f71e696d11a60f771535f6837ecad6132047b0a) ) // sprite lookup table

	ROM_REGION( 0x10000, "vlm", 0 ) // speech, located on Sound Board
	ROM_LOAD( "405e15.11f", 0x0000, 0x2000, CRC(01bb5ce9) SHA1(f48477b4011befba13c8bcd83e0c9f7deb14a1e1) )
ROM_END

/*
    Super Basketball (version H, unprotected)

    Jumper Settings for CPU/Video Board

        JP6 set to 27128 with a null resistor
        JP5 set to A with a null resistor
        JP4 set to 27128 with a null resistor
        JP3 connected with a solder blob
        JP2 connected

    Jumper Settings for Sound Board

        J1 connected with a null resistor
*/

ROM_START( sbasketh )
	ROM_REGION( 0x10000, "maincpu", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405h05.14j", 0x6000, 0x2000, CRC(263ec36b) SHA1(b445b600726ba4935623311e1a178aeb4a356b0a) )
	ROM_LOAD( "405h03.11j", 0x8000, 0x4000, CRC(0a4d7a82) SHA1(2e0153b41e23284427881258a44bd55be3570eb2) )
	ROM_LOAD( "405h01.9j",  0xc000, 0x4000, CRC(4f9dd9a0) SHA1(97f4c208509d50a7ce4c1ebe8a3f643ad75e833b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // ROMs located on Sound Board
	ROM_LOAD( "405e13.7a",  0x0000, 0x2000, CRC(1ec7458b) SHA1(a015b982bff5f9e7ece33f2e69ff8c6c2174e710) )

	ROM_REGION( 0x04000, "tiles", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405e12.22f", 0x0000, 0x4000, CRC(e02c54da) SHA1(2fa19f3bce894ef05820f95e0b88428e4f946a35) )

	ROM_REGION( 0x0c000, "sprites", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405h06.14g", 0x0000, 0x4000, CRC(cfbbff07) SHA1(39b19866b21372524933b5eef511bb5b7ad92556) )
	ROM_LOAD( "405h08.17g", 0x4000, 0x4000, CRC(c75901b6) SHA1(4ff87123228da068f0c0ffffa4a3f03765eccd8d) )
	ROM_LOAD( "405h10.20g", 0x8000, 0x4000, CRC(95bc5942) SHA1(55bf35283385d0ae768210706720a3b289ebd9a2) )

	ROM_REGION( 0x0500, "proms", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405e17.5a",  0x0000, 0x0100, CRC(b4c36d57) SHA1(c4a63f57edce2b9588e2394ff54a28f91213d550) ) // palette red component
	ROM_LOAD( "405e16.4a",  0x0100, 0x0100, CRC(0b7b03b8) SHA1(81297cb2b0b28b0fc0939a37ff30844d69fb65ac) ) // palette green component
	ROM_LOAD( "405e18.6a",  0x0200, 0x0100, CRC(9e533bad) SHA1(611e7af6813caaf2bc36c311ae48a5efd30e6f0c) ) // palette blue component
	ROM_LOAD( "405e20.19d", 0x0300, 0x0100, CRC(8ca6de2f) SHA1(67d29708d1a07d17c5dc5793a3293e7ace3a4e19) ) // character lookup table
	ROM_LOAD( "405e19.16d", 0x0400, 0x0100, CRC(e0bc782f) SHA1(9f71e696d11a60f771535f6837ecad6132047b0a) ) // sprite lookup table

	ROM_REGION( 0x10000, "vlm", 0 ) // speech, located on Sound Board
	ROM_LOAD( "405e15.11f", 0x0000, 0x2000, CRC(01bb5ce9) SHA1(f48477b4011befba13c8bcd83e0c9f7deb14a1e1) )
ROM_END


/*
    Super Basketball (version G, encrypted)

    Jumper Settings for CPU/Video Board

        JP6 set to 2764 with a null resistor
        JP5 set to A with a null resistor
        JP4 set to 2764 with a null resistor
        JP3 not connected
        JP2 connected

    Jumper Settings for Sound Board

        J1 connected with a null resistor
*/

ROM_START( sbasketg )
	ROM_REGION( 0x10000, "maincpu", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405g05.14j", 0x6000, 0x2000, CRC(336dc0ab) SHA1(0fe47fdbf183683c569785fc6b980337a9cfde95) )
	ROM_LOAD( "405g04.13j", 0x8000, 0x2000, CRC(f064a9bc) SHA1(4f1b94a880385c6ba74cc0883b24f6fec934e35d) )
	ROM_LOAD( "405g03.11j", 0xa000, 0x2000, CRC(b9de7d53) SHA1(5a4e5491ff3511992d949367fd7b5d383c2727db) )
	ROM_LOAD( "405g02.10j", 0xc000, 0x2000, CRC(e98470a0) SHA1(79af25af941fe357a8c9f0a2f11e5558670b8027) )
	ROM_LOAD( "405g01.9j",  0xe000, 0x2000, CRC(1bd0cd2e) SHA1(d162f9b989f718d9882a02a8c64743adf3d8e239) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // ROMs located on Sound Board
	ROM_LOAD( "405e13.7a",  0x0000, 0x2000, CRC(1ec7458b) SHA1(a015b982bff5f9e7ece33f2e69ff8c6c2174e710) )

	ROM_REGION( 0x04000, "tiles", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405e12.22f", 0x0000, 0x4000, CRC(e02c54da) SHA1(2fa19f3bce894ef05820f95e0b88428e4f946a35) )

	ROM_REGION( 0x0c000, "sprites", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405e06.14g", 0x0000, 0x2000, CRC(7e2f5bb2) SHA1(e22008c0ef7ae000dcca7f43a386d43064aaea62) )
	ROM_LOAD( "405e07.16g", 0x2000, 0x2000, CRC(963a44f9) SHA1(03cd7699668b010f27af025ba6bd44509526ec7b) )
	ROM_LOAD( "405e08.17g", 0x4000, 0x2000, CRC(63901deb) SHA1(c65d896298846ed8b70a4d38b32820746214fa5c) )
	ROM_LOAD( "405e09.19g", 0x6000, 0x2000, CRC(e1873677) SHA1(19788e43cc1a6cf5ab375cbc2c745bb6cc8c163d) )
	ROM_LOAD( "405e10.20g", 0x8000, 0x2000, CRC(824815e8) SHA1(470e9d74fa2c397605a74e0bf173a6d9db4cc721) )
	ROM_LOAD( "405e11.22g", 0xa000, 0x2000, CRC(dca9b447) SHA1(12d7e85dc2fc6bd4ea7ad9035ae0b7487e4bc4bc) )

	ROM_REGION( 0x0500, "proms", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405e17.5a",  0x0000, 0x0100, CRC(b4c36d57) SHA1(c4a63f57edce2b9588e2394ff54a28f91213d550) ) // palette red component
	ROM_LOAD( "405e16.4a",  0x0100, 0x0100, CRC(0b7b03b8) SHA1(81297cb2b0b28b0fc0939a37ff30844d69fb65ac) ) // palette green component
	ROM_LOAD( "405e18.6a",  0x0200, 0x0100, CRC(9e533bad) SHA1(611e7af6813caaf2bc36c311ae48a5efd30e6f0c) ) // palette blue component
	ROM_LOAD( "405e20.19d", 0x0300, 0x0100, CRC(8ca6de2f) SHA1(67d29708d1a07d17c5dc5793a3293e7ace3a4e19) ) // character lookup table
	ROM_LOAD( "405e19.16d", 0x0400, 0x0100, CRC(e0bc782f) SHA1(9f71e696d11a60f771535f6837ecad6132047b0a) ) // sprite lookup table

	ROM_REGION( 0x10000, "vlm", 0 ) // speech, located on Sound Board
	ROM_LOAD( "405e15.11f", 0x0000, 0x2000, CRC(01bb5ce9) SHA1(f48477b4011befba13c8bcd83e0c9f7deb14a1e1) )
ROM_END


/*
    Super Basketball (version E, encrypted)
*/

ROM_START( sbaskete )
	ROM_REGION( 0x10000, "maincpu", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405e05.14j", 0x6000, 0x2000, CRC(32ea5b71) SHA1(d917c31d2c9a7229396e4a930e8d27394329533a) )
	ROM_LOAD( "405e04.13j", 0x8000, 0x2000, CRC(7abf3087) SHA1(fbaaaaae0b8bed1bc6ad7f2da267c2ef8bd75b15) )
	ROM_LOAD( "405e03.11j", 0xa000, 0x2000, CRC(9c6fcdcd) SHA1(a644ec98f49f84311829149c181aba25e7681793) )
	ROM_LOAD( "405e02.10j", 0xc000, 0x2000, CRC(0f145648) SHA1(2e238eb0663295887bf6b4905f1fd386db16d82a) )
	ROM_LOAD( "405e01.9j",  0xe000, 0x2000, CRC(6a27f1b1) SHA1(38c0be98fb122a7a6ed833af011bda5663a06510) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // ROMs located on Sound Board
	ROM_LOAD( "405e13.7a",  0x0000, 0x2000, CRC(1ec7458b) SHA1(a015b982bff5f9e7ece33f2e69ff8c6c2174e710) )

	ROM_REGION( 0x04000, "tiles", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405e12.22f", 0x0000, 0x4000, CRC(e02c54da) SHA1(2fa19f3bce894ef05820f95e0b88428e4f946a35) )

	ROM_REGION( 0x0c000, "sprites", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405e06.14g", 0x0000, 0x2000, CRC(7e2f5bb2) SHA1(e22008c0ef7ae000dcca7f43a386d43064aaea62) )
	ROM_LOAD( "405e07.16g", 0x2000, 0x2000, CRC(963a44f9) SHA1(03cd7699668b010f27af025ba6bd44509526ec7b) )
	ROM_LOAD( "405e08.17g", 0x4000, 0x2000, CRC(63901deb) SHA1(c65d896298846ed8b70a4d38b32820746214fa5c) )
	ROM_LOAD( "405e09.19g", 0x6000, 0x2000, CRC(e1873677) SHA1(19788e43cc1a6cf5ab375cbc2c745bb6cc8c163d) )
	ROM_LOAD( "405e10.20g", 0x8000, 0x2000, CRC(824815e8) SHA1(470e9d74fa2c397605a74e0bf173a6d9db4cc721) )
	ROM_LOAD( "405e11.22g", 0xa000, 0x2000, CRC(dca9b447) SHA1(12d7e85dc2fc6bd4ea7ad9035ae0b7487e4bc4bc) )

	ROM_REGION( 0x0500, "proms", 0 ) // ROMs located on the CPU/Video board
	ROM_LOAD( "405e17.5a",  0x0000, 0x0100, CRC(b4c36d57) SHA1(c4a63f57edce2b9588e2394ff54a28f91213d550) ) // palette red component
	ROM_LOAD( "405e16.4a",  0x0100, 0x0100, CRC(0b7b03b8) SHA1(81297cb2b0b28b0fc0939a37ff30844d69fb65ac) ) // palette green component
	ROM_LOAD( "405e18.6a",  0x0200, 0x0100, CRC(9e533bad) SHA1(611e7af6813caaf2bc36c311ae48a5efd30e6f0c) ) // palette blue component
	ROM_LOAD( "405e20.19d", 0x0300, 0x0100, CRC(8ca6de2f) SHA1(67d29708d1a07d17c5dc5793a3293e7ace3a4e19) ) // character lookup table
	ROM_LOAD( "405e19.16d", 0x0400, 0x0100, CRC(e0bc782f) SHA1(9f71e696d11a60f771535f6837ecad6132047b0a) ) // sprite lookup table

	ROM_REGION( 0x10000, "vlm", 0 ) // speech, located on Sound Board
	ROM_LOAD( "405e15.11f", 0x0000, 0x2000, CRC(01bb5ce9) SHA1(f48477b4011befba13c8bcd83e0c9f7deb14a1e1) )
ROM_END

} // anonymous namespace


GAME( 1984, sbasketb, 0,        sbasketb,  sbasketb, sbasketb_state, empty_init, ROT90, "Konami", "Super Basketball (version I, encrypted)",   MACHINE_SUPPORTS_SAVE )
GAME( 1984, sbasketh, sbasketb, sbasketbu, sbasketb, sbasketb_state, empty_init, ROT90, "Konami", "Super Basketball (version H, unprotected)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, sbasketg, sbasketb, sbasketb,  sbasketb, sbasketb_state, empty_init, ROT90, "Konami", "Super Basketball (version G, encrypted)",   MACHINE_SUPPORTS_SAVE )
GAME( 1984, sbaskete, sbasketb, sbasketb,  sbasketb, sbasketb_state, empty_init, ROT90, "Konami", "Super Basketball (version E, encrypted)",   MACHINE_SUPPORTS_SAVE )
