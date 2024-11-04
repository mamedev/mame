// license:BSD-3-Clause
// copyright-holders: Allard van der Bas

/***************************************************************************

    Pooyan
    GX320

    Original driver by Allard van der Bas

    This hardware is very similar to Time Pilot.

***************************************************************************/

#include "emu.h"

#include "konamipt.h"
#include "timeplt_a.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class pooyan_state : public driver_device
{
public:
	pooyan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram%u", 1U)
	{ }

	void pooyan(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_spriteram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;

	// misc
	uint8_t m_irq_enable = 0;

	void irq_enable_w(int state);
	template <uint8_t Which> void coin_counter_w(int state);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vblank_irq(int state);
	void main_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Pooyan has one 32x8 palette PROM and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

void pooyan_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, rweights, 1000, 0,
			3, resistances_rg, gweights, 1000, 0,
			2, resistances_b,  bweights, 1000, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
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
	color_prom += 0x20;

	// characters
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}

	// sprites
	for (int i = 0x100; i < 0x200; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}



/*************************************
 *
 *  Tilemap info callback
 *
 *************************************/

TILE_GET_INFO_MEMBER(pooyan_state::get_bg_tile_info)
{
	int const attr = m_colorram[tile_index];
	int const code = m_videoram[tile_index];
	int const color = attr & 0x0f;
	int const flags = TILE_FLIPYX(attr >> 6);

	tileinfo.set(0, code, color, flags);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void pooyan_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pooyan_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}



/*************************************
 *
 *  Memory write handlers
 *
 *************************************/

void pooyan_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void pooyan_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

void pooyan_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0x10; offs < 0x40; offs += 2)
	{
		int const sx = m_spriteram[0][offs];
		int const sy = 240 - m_spriteram[1][offs + 1];

		int const code = m_spriteram[0][offs + 1];
		int const color = m_spriteram[1][offs] & 0x0f;
		int const flipx = ~m_spriteram[1][offs] & 0x40;
		int const flipy = m_spriteram[1][offs] & 0x80;


			m_gfxdecode->gfx(1)->transmask(bitmap, cliprect,
			code,
			color,
			flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0));
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t pooyan_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Interrupts
 *
 *************************************/

void pooyan_state::vblank_irq(int state)
{
	if (state && m_irq_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


void pooyan_state::irq_enable_w(int state)
{
	m_irq_enable = state;
	if (!m_irq_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


template <uint8_t Which>
void pooyan_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

void pooyan_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram().w(FUNC(pooyan_state::colorram_w)).share(m_colorram);
	map(0x8400, 0x87ff).ram().w(FUNC(pooyan_state::videoram_w)).share(m_videoram);
	map(0x8800, 0x8fff).ram();
	map(0x9000, 0x90ff).mirror(0x0b00).ram().share(m_spriteram[0]);
	map(0x9400, 0x94ff).mirror(0x0b00).ram().share(m_spriteram[1]);
	map(0xa000, 0xa000).mirror(0x5e7f).portr("DSW1");
	map(0xa080, 0xa080).mirror(0x5e1f).portr("IN0");
	map(0xa0a0, 0xa0a0).mirror(0x5e1f).portr("IN1");
	map(0xa0c0, 0xa0c0).mirror(0x5e1f).portr("IN2");
	map(0xa0e0, 0xa0e0).mirror(0x5e1f).portr("DSW0");
	map(0xa000, 0xa000).mirror(0x5e7f).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xa100, 0xa100).mirror(0x5e7f).w("timeplt_audio", FUNC(timeplt_audio_device::sound_data_w));
	map(0xa180, 0xa187).mirror(0x5e78).w("mainlatch", FUNC(ls259_device::write_d0));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( pooyan )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	// Invalid = both coin slots disabled

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "50K 80K+" )
	PORT_DIPSETTING(    0x00, "30K 70K+" )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6,7")
	PORT_DIPSETTING(    0x70, "1 (Easy)" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x50, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ STEP4(0,1), STEP4(8*8,1) },
	{ STEP8(0,8) },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ STEP4(0,1), STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1) },
	{ STEP8(0,8), STEP8(32*8,8) },
	64*8
};


static GFXDECODE_START( gfx_pooyan )
	GFXDECODE_ENTRY( "tiles",   0, charlayout,       0, 16 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 16*16, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void pooyan_state::machine_start()
{
	save_item(NAME(m_irq_enable));
}


void pooyan_state::pooyan(machine_config &config)
{
	static constexpr XTAL MASTER_CLOCK = XTAL(18'432'000);

	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK / 3 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &pooyan_state::main_map);

	ls259_device &mainlatch(LS259(config, "mainlatch")); // B2
	mainlatch.q_out_cb<0>().set(FUNC(pooyan_state::irq_enable_w));
	mainlatch.q_out_cb<1>().set("timeplt_audio", FUNC(timeplt_audio_device::sh_irqtrigger_w));
	mainlatch.q_out_cb<2>().set("timeplt_audio", FUNC(timeplt_audio_device::mute_w));
	mainlatch.q_out_cb<3>().set(FUNC(pooyan_state::coin_counter_w<0>));
	mainlatch.q_out_cb<4>().set(FUNC(pooyan_state::coin_counter_w<1>));
	mainlatch.q_out_cb<5>().set_nop(); // PAY OUT - not used
	mainlatch.q_out_cb<7>().set(FUNC(pooyan_state::flip_screen_set)).invert();

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(pooyan_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(pooyan_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pooyan);
	PALETTE(config, m_palette, FUNC(pooyan_state::palette), 16*16+16*16, 32);

	// sound hardware
	TIMEPLT_AUDIO(config, "timeplt_audio");
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( pooyan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.4a",         0x0000, 0x2000, CRC(bb319c63) SHA1(5401b8ef586127c8cf5a431e5c44e38be2254a98) )
	ROM_LOAD( "2.5a",         0x2000, 0x2000, CRC(a1463d98) SHA1(b23cc7e61276c61a78e80fe08c7f0c8adadf2ffe) )
	ROM_LOAD( "3.6a",         0x4000, 0x2000, CRC(fe1a9e08) SHA1(5206893760f188ac71a5e6bd42561cf25fcc3d49) )
	ROM_LOAD( "4.7a",         0x6000, 0x2000, CRC(9e0f9bcc) SHA1(4d9707423ad531ac535db432e329b3d52cbb4559) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "xx.7a",        0x0000, 0x1000, CRC(fbe2b368) SHA1(5689a84ef110bdc0039ad1a6c5778e0b8eccfce0) )
	ROM_LOAD( "xx.8a",        0x1000, 0x1000, CRC(e1795b3d) SHA1(9ab4e5362f9f7d9b46b750e14b1d9d71c57be40f) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "8.10g",        0x0000, 0x1000, CRC(931b29eb) SHA1(0325c1c1fdb44e0044b82b7c79b5eeabf5c11ce7) )
	ROM_LOAD( "7.9g",         0x1000, 0x1000, CRC(bbe6d6e4) SHA1(de5447d59a99c4c08c4f40c0b7dd3c3c609c11d4) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "6.9a",         0x0000, 0x1000, CRC(b2d8c121) SHA1(189ad488869f34d7a38b82ef70eb805acfe04312) )
	ROM_LOAD( "5.8a",         0x1000, 0x1000, CRC(1097c2b6) SHA1(c815f0d27593efd23923511bdd13835456ef7f76) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pooyan.pr1",   0x0000, 0x0020, CRC(a06a6d0e) SHA1(ae131320b66d76d4bc9108da6708f6f874b2e123) ) // palette
	ROM_LOAD( "pooyan.pr3",   0x0020, 0x0100, CRC(8cd4cd60) SHA1(e0188ecd5b53a8e6e28c1de80def676740772334) ) // characters
	ROM_LOAD( "pooyan.pr2",   0x0120, 0x0100, CRC(82748c0b) SHA1(9ce8eb92e482eba5a9077e9db99841d65b011346) ) // sprites
ROM_END

ROM_START( pooyans )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic22_a4.cpu",  0x0000, 0x2000, CRC(916ae7d7) SHA1(e96eba381e6ad228acf4b74240d618f9d0bae39d) )
	ROM_LOAD( "ic23_a5.cpu",  0x2000, 0x2000, CRC(8fe38c61) SHA1(4588f9f80a5884e056a1d429785c7331e92d5654) )
	ROM_LOAD( "ic24_a6.cpu",  0x4000, 0x2000, CRC(2660218a) SHA1(606b10a4bab2432e20471440105e04d15d384570) )
	ROM_LOAD( "ic25_a7.cpu",  0x6000, 0x2000, CRC(3d2a10ad) SHA1(962c621a19e9797b8f3d12c150aa0b90958c9498) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "xx.7a",        0x0000, 0x1000, CRC(fbe2b368) SHA1(5689a84ef110bdc0039ad1a6c5778e0b8eccfce0) )
	ROM_LOAD( "xx.8a",        0x1000, 0x1000, CRC(e1795b3d) SHA1(9ab4e5362f9f7d9b46b750e14b1d9d71c57be40f) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "ic13_g10.cpu", 0x0000, 0x1000, CRC(7433aea9) SHA1(a5ad6311f097fefb6e7b747ebe9d01d72d7755d0) )
	ROM_LOAD( "ic14_g9.cpu",  0x1000, 0x1000, CRC(87c1789e) SHA1(7637a9604a3ad4f9a27105d87252de3d923672aa) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "6.9a",         0x0000, 0x1000, CRC(b2d8c121) SHA1(189ad488869f34d7a38b82ef70eb805acfe04312) )
	ROM_LOAD( "5.8a",         0x1000, 0x1000, CRC(1097c2b6) SHA1(c815f0d27593efd23923511bdd13835456ef7f76) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pooyan.pr1",   0x0000, 0x0020, CRC(a06a6d0e) SHA1(ae131320b66d76d4bc9108da6708f6f874b2e123) ) // palette
	ROM_LOAD( "pooyan.pr3",   0x0020, 0x0100, CRC(8cd4cd60) SHA1(e0188ecd5b53a8e6e28c1de80def676740772334) ) // characters
	ROM_LOAD( "pooyan.pr2",   0x0120, 0x0100, CRC(82748c0b) SHA1(9ce8eb92e482eba5a9077e9db99841d65b011346) ) // sprites
ROM_END

ROM_START( pootan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poo_ic22.bin", 0x0000, 0x2000, CRC(41b23a24) SHA1(366efcc45613391c1ab1514654ecac1ae3d39d0e) )
	ROM_LOAD( "poo_ic23.bin", 0x2000, 0x2000, CRC(c9d94661) SHA1(af1e818335adb4398ea0dc41be0d6399999f3946) )
	ROM_LOAD( "3.6a",         0x4000, 0x2000, CRC(fe1a9e08) SHA1(5206893760f188ac71a5e6bd42561cf25fcc3d49) )
	ROM_LOAD( "poo_ic25.bin", 0x6000, 0x2000, CRC(8ae459ef) SHA1(995eba204bbb82da20063b965bf79a64441a907a) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "xx.7a",        0x0000, 0x1000, CRC(fbe2b368) SHA1(5689a84ef110bdc0039ad1a6c5778e0b8eccfce0) )
	ROM_LOAD( "xx.8a",        0x1000, 0x1000, CRC(e1795b3d) SHA1(9ab4e5362f9f7d9b46b750e14b1d9d71c57be40f) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "poo_ic13.bin", 0x0000, 0x1000, CRC(0be802e4) SHA1(07adc17bcb7332ddc00b7c71bf4919eda80b0bdb) )
	ROM_LOAD( "poo_ic14.bin", 0x1000, 0x1000, CRC(cba29096) SHA1(b5a4cf75089cf04f7361e00074816facd57452b2) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "6.9a",         0x0000, 0x1000, CRC(b2d8c121) SHA1(189ad488869f34d7a38b82ef70eb805acfe04312) )
	ROM_LOAD( "5.8a",         0x1000, 0x1000, CRC(1097c2b6) SHA1(c815f0d27593efd23923511bdd13835456ef7f76) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pooyan.pr1",   0x0000, 0x0020, CRC(a06a6d0e) SHA1(ae131320b66d76d4bc9108da6708f6f874b2e123) ) // palette
	ROM_LOAD( "pooyan.pr3",   0x0020, 0x0100, CRC(8cd4cd60) SHA1(e0188ecd5b53a8e6e28c1de80def676740772334) ) // characters
	ROM_LOAD( "pooyan.pr2",   0x0120, 0x0100, CRC(82748c0b) SHA1(9ce8eb92e482eba5a9077e9db99841d65b011346) ) // sprites
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

//    YEAR, NAME,    PARENT, MACHINE,INPUT,  STATE,        INIT,       MONITOR,COMPANY,                              FULLNAME,                     FLAGS
GAME( 1982, pooyan,  0,      pooyan, pooyan, pooyan_state, empty_init, ROT90,  "Konami",                             "Pooyan",                     MACHINE_SUPPORTS_SAVE )
GAME( 1982, pooyans, pooyan, pooyan, pooyan, pooyan_state, empty_init, ROT90,  "Konami (Stern Electronics license)", "Pooyan (Stern Electronics)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, pootan,  pooyan, pooyan, pooyan, pooyan_state, empty_init, ROT90,  "bootleg",                            "Pootan",                     MACHINE_SUPPORTS_SAVE )
