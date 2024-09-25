// license:BSD-3-Clause
// copyright-holders: Ernesto Corvi

/***************************************************************************

Knuckle Joe - (c) 1985 Seibu Kaihatsu (Taito license)

driver by Ernesto Corvi

This board seems to be an Irem design.
The sound hardware is a modified version of the 6803-based one used by the
classic Irem games. There's only one AY 3-8910 chip and no MSM5205. There
are also two SN76489 controlled directly by main(!) CPU, and used only for
in-game music.
The video hardware is pretty much like Irem games too. The only
strange thing is that the screen is flipped vertically.

TODO:
- sprite-sprite priorities are imperfect (simply reversing them won't fix it,
  motorcycle level will get much worse)
- accurate screen timing (raw params), attract mode doesn't 1:1 match PCB

BTANB:
- attract mode demo play stops playing and lets the timer run out
- heads of baddies clip off when you defeat them on the lowest floor
- player sprite 1-scanline glitch at the lower part of motorcycle level
- player sprite may briefly turn into garbage after a boss fight

***************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class kncljoe_state : public driver_device
{
public:
	kncljoe_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scrollregs(*this, "scrollregs"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ay8910(*this, "aysnd"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void kncljoe(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrollregs;
	required_shared_ptr<uint8_t> m_spriteram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_tile_bank = 0U;
	uint8_t m_sprite_bank = 0U;
	uint8_t m_flipscreen = 0U;

	// misc
	uint8_t m_port1 = 0U;
	uint8_t m_port2 = 0U;

	required_device<cpu_device> m_maincpu;
	required_device<m6803_cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ay8910_device> m_ay8910;
	required_device<generic_latch_8_device> m_soundlatch;

	void sound_cmd_w(uint8_t data);
	void sound_irq_ack_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void control_w(uint8_t data);
	void scroll_w(offs_t offset, uint8_t data);
	void m6803_port1_w(uint8_t data);
	void m6803_port2_w(uint8_t data);
	uint8_t m6803_port1_r();
	uint8_t m6803_port2_r();
	void unused_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(sound_nmi);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

void kncljoe_state::palette(palette_device &palette) const
{
	uint8_t const *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x80; i++)
	{
		int const r = pal4bit(color_prom[i + 0x000]);
		int const g = pal4bit(color_prom[i + 0x100]);
		int const b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for (int i = 0; i < 0x10; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = 0;
		bit1 = BIT(color_prom[i + 0x300], 6);
		bit2 = BIT(color_prom[i + 0x300], 7);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i + 0x300], 3);
		bit1 = BIT(color_prom[i + 0x300], 4);
		bit2 = BIT(color_prom[i + 0x300], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(color_prom[i + 0x300], 0);
		bit1 = BIT(color_prom[i + 0x300], 1);
		bit2 = BIT(color_prom[i + 0x300], 2);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i + 0x80, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x320;

	// chars
	for (int i = 0; i < 0x80; i++)
		palette.set_pen_indirect(i, i);

	// sprite lookup table
	for (int i = 0; i < 0x80; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x0f) | 0x80;
		palette.set_pen_indirect(i + 0x80, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(kncljoe_state::get_bg_tile_info)
{
	int const attr = m_videoram[2 * tile_index + 1];
	int const code = m_videoram[2 * tile_index] + ((attr & 0xc0) << 2) + (m_tile_bank << 10);

	tileinfo.set(0,
			code,
			attr & 0xf,
			TILE_FLIPXY((attr & 0x30) >> 4));
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void kncljoe_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kncljoe_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_bg_tilemap->set_scroll_rows(4);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void kncljoe_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void kncljoe_state::control_w(uint8_t data)
{
	/*
	        0x01    screen flip
	        0x02    coin counter#1
	        0x04    sprite bank
	        0x10    character bank
	        0x20    coin counter#2

	        reset when IN0 - Coin 1 goes low (active)
	        set after IN0 - Coin 1 goes high AND the credit has been added
	*/
	m_flipscreen = data & 0x01;
	machine().tilemap().set_flip_all(m_flipscreen ? TILEMAP_FLIPX : TILEMAP_FLIPY);

	machine().bookkeeping().coin_counter_w(0, data & 0x02);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);

	if (m_tile_bank != BIT(data, 4))
	{
		m_tile_bank = BIT(data, 4);
		m_bg_tilemap->mark_all_dirty();
	}

	m_sprite_bank = BIT(data, 2);
}

void kncljoe_state::scroll_w(offs_t offset, uint8_t data)
{
	m_scrollregs[offset] = data;
	int const scrollx = m_scrollregs[0] | m_scrollregs[1] << 8;
	m_bg_tilemap->set_scrollx(0, scrollx);
	m_bg_tilemap->set_scrollx(1, scrollx);
	m_bg_tilemap->set_scrollx(2, scrollx);
	m_bg_tilemap->set_scrollx(3, 0);
}



/***************************************************************************

  Display refresh

***************************************************************************/

void kncljoe_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(1 + m_sprite_bank);

	for (int i = 0; i < 4; i++)
	{
		// clip vertical strip for each layer
		rectangle clip = cliprect;
		clip.min_y = m_flipscreen ? (191 - i * 64) : (i * 64 + 1);
		clip.max_y = clip.min_y + 63;
		clip &= cliprect;

		for (int j = 0x7c; j >= 0; j -= 4)
		{
			int const offs = bitswap<2>(~i, 0, 1) << 7 | j;
			int sy = m_spriteram[offs] + 1;
			int sx = m_spriteram[offs + 3];
			int const attr = m_spriteram[offs + 1];
			int const code = m_spriteram[offs + 2] | ((attr & 0x10) << 5) | ((attr & 0x20) << 3);
			int flipx = attr & 0x40;
			int flipy = !(attr & 0x80);
			int const color = attr & 0x0f;

			if (m_flipscreen)
			{
				flipx = !flipx;
				flipy = !flipy;
				sx = 240 - sx;
				sy = 240 - sy;
			}

			if (sx >= 256 - 8)
				sx -= 256;

			gfx->transpen(bitmap, clip,
				code,
				color,
				flipx, flipy,
				sx, sy, 0);
		}
	}
}

uint32_t kncljoe_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void kncljoe_state::sound_cmd_w(uint8_t data)
{
	if ((data & 0x80) == 0)
		m_soundlatch->write(data & 0x7f);
	else
		m_soundcpu->set_input_line(0, ASSERT_LINE);
}


void kncljoe_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram().w(FUNC(kncljoe_state::videoram_w)).share(m_videoram);
	map(0xd000, 0xd001).w(FUNC(kncljoe_state::scroll_w)).share(m_scrollregs);
	map(0xd800, 0xd800).portr("SYSTEM");
	map(0xd801, 0xd801).portr("P1");
	map(0xd802, 0xd802).portr("P2");
	map(0xd803, 0xd803).portr("DSWA");
	map(0xd804, 0xd804).portr("DSWB");
	map(0xd800, 0xd800).w(FUNC(kncljoe_state::sound_cmd_w));
	map(0xd801, 0xd801).w(FUNC(kncljoe_state::control_w));
	map(0xd802, 0xd802).w("sn1", FUNC(sn76489_device::write));
	map(0xd803, 0xd803).w("sn2", FUNC(sn76489_device::write));
	map(0xd807, 0xd807).nopr();     // unknown read
	map(0xd817, 0xd817).nopr();     // unknown read
	map(0xe800, 0xefff).ram().share(m_spriteram);
	map(0xf000, 0xffff).ram();
}

void kncljoe_state::m6803_port1_w(uint8_t data)
{
	m_port1 = data;
}

void kncljoe_state::m6803_port2_w(uint8_t data)
{
	// write latch
	if ((m_port2 & 0x01) && !(data & 0x01))
	{
		// control or data port?
		if (m_port2 & 0x08)
			m_ay8910->data_address_w(m_port2 >> 2, m_port1);
	}
	m_port2 = data;
}

uint8_t kncljoe_state::m6803_port1_r()
{
	if (m_port2 & 0x08)
		return m_ay8910->data_r();
	return 0xff;
}

uint8_t kncljoe_state::m6803_port2_r()
{
	return 0;
}

void kncljoe_state::sound_irq_ack_w(uint8_t data)
{
	m_soundcpu->set_input_line(0, CLEAR_LINE);
}

void kncljoe_state::unused_w(uint8_t data)
{
	// unused - no MSM on the PCB
}

void kncljoe_state::sound_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x0fff).nopw();
	map(0x1000, 0x1fff).w(FUNC(kncljoe_state::sound_irq_ack_w));
	map(0x2000, 0x7fff).rom();
}


/******************************************************************************/

static INPUT_PORTS_START( kncljoe )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x20, "Infinite Energy (Cheat)")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Free Play (Not Working)")    // Not working due to code at 0x296f
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "Unused SW B-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x18, "10k and every 20k" )
	PORT_DIPSETTING(    0x10, "20k and every 40k" )
	PORT_DIPSETTING(    0x08, "30k and every 60k" )
	PORT_DIPSETTING(    0x00, "40k and every 80k" )
	PORT_DIPNAME( 0x60, 0x60, "Difficulty?" )           // Stored at 0xf018
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( gfx_kncljoe )
	GFXDECODE_ENTRY( "tiles",    0, gfx_8x8x3_planar,   0x00, 16 )    // colors 0x00-0x7f direct mapped
	GFXDECODE_ENTRY( "sprites1", 0, spritelayout,       0x80, 16 )    // colors 0x80-0x8f with lookup table
	GFXDECODE_ENTRY( "sprites2", 0, spritelayout,       0x80, 16 )
GFXDECODE_END


INTERRUPT_GEN_MEMBER(kncljoe_state::sound_nmi)
{
	device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void kncljoe_state::machine_start()
{
	save_item(NAME(m_port1));
	save_item(NAME(m_port2));
	save_item(NAME(m_tile_bank));
	save_item(NAME(m_sprite_bank));
	save_item(NAME(m_flipscreen));
}

void kncljoe_state::machine_reset()
{
	m_port1 = 0;
	m_port2 = 0;
	m_tile_bank = 0;
	m_sprite_bank = 0;
	m_flipscreen = 0;
}

void kncljoe_state::kncljoe(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(6'000'000));  // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &kncljoe_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(kncljoe_state::irq0_line_hold));

	M6803(config, m_soundcpu, XTAL(3'579'545)); // verified on PCB
	m_soundcpu->set_addrmap(AS_PROGRAM, &kncljoe_state::sound_map);
	m_soundcpu->in_p1_cb().set(FUNC(kncljoe_state::m6803_port1_r));
	m_soundcpu->out_p1_cb().set(FUNC(kncljoe_state::m6803_port1_w));
	m_soundcpu->in_p2_cb().set(FUNC(kncljoe_state::m6803_port2_r));
	m_soundcpu->out_p2_cb().set(FUNC(kncljoe_state::m6803_port2_w));
	m_soundcpu->set_periodic_int(FUNC(kncljoe_state::sound_nmi), attotime::from_hz(3970)); // measured 3.970 kHz

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1500));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(1*8, 31*8-1, 0*8, 32*8-1);
	m_screen->set_screen_update(FUNC(kncljoe_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_kncljoe);
	PALETTE(config, m_palette, FUNC(kncljoe_state::palette), 16*8+16*8, 128+16);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, m_ay8910, XTAL(3'579'545) / 4); // verified on PCB
	m_ay8910->port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	m_ay8910->port_b_write_callback().set(FUNC(kncljoe_state::unused_w));
	m_ay8910->add_route(ALL_OUTPUTS, "mono", 0.30);

	SN76489(config, "sn1", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 0.30); // verified on PCB

	SN76489(config, "sn2", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 0.30); // verified on PCB
}



ROM_START( kncljoe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kj-1.bin", 0x0000, 0x4000, CRC(4e4f5ff2) SHA1(7d889aa4f4138f01014c1dda391f82396074cfab) )
	ROM_LOAD( "kj-2.bin", 0x4000, 0x4000, CRC(cb11514b) SHA1(c75d4019d1617493ff074ce8187a81ad70d9b60c) )
	ROM_LOAD( "kj-3.bin", 0x8000, 0x4000, CRC(0f50697b) SHA1(412c6aba270824299ca2a74e9bea42b83e69797b) )

	ROM_REGION( 0x8000, "soundcpu", 0 )
	ROM_LOAD( "kj-13.bin",0x6000, 0x2000, CRC(0a0be3f5) SHA1(00be47fc76500843b6f5de63622edb1748ef5f7d) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "kj-10.bin", 0x0000,  0x4000, CRC(74d3ba33) SHA1(c7887d690cb7f7a7b24d59d490ffc088fb6cc49c) )
	ROM_LOAD( "kj-11.bin", 0x4000,  0x4000, CRC(8ea01455) SHA1(b4b42fe373a1019b4f2a4b763a8a7219a5c9987e) )
	ROM_LOAD( "kj-12.bin", 0x8000,  0x4000, CRC(33367c41) SHA1(e6c56bcad008f3af4bc0f7d7afe8e23c8eb9d943) )

	ROM_REGION( 0x18000, "sprites1", 0 )
	ROM_LOAD( "kj-4.bin", 0x00000,  0x8000, CRC(a499ea10) SHA1(cb671cc75b3c6029dd3529e62d83025f78b45271) )
	ROM_LOAD( "kj-6.bin", 0x08000,  0x8000, CRC(815f5c0a) SHA1(ad0b59eeebb2e57035a3f643ac0ef575569bec0f) )
	ROM_LOAD( "kj-5.bin", 0x10000,  0x8000, CRC(11111759) SHA1(504c62fc6778a4afa86cba69634652708535bef6) )

	ROM_REGION( 0xc000, "sprites2", 0 )
	ROM_LOAD( "kj-7.bin", 0x0000,   0x4000, CRC(121fcccb) SHA1(77f3e7e49787d6a893c5d8c0c3ac612b1180e866) )
	ROM_LOAD( "kj-9.bin", 0x4000,   0x4000, CRC(affbe3eb) SHA1(056111fc5b04ff14b114b5f724d02789c8e3ee10) )
	ROM_LOAD( "kj-8.bin", 0x8000,   0x4000, CRC(e057e72a) SHA1(3a85750c72caaa027f302dc6ca4086bdbd49b5ff) )

	ROM_REGION( 0x420, "proms", 0 )
	ROM_LOAD( "kjclr1.bin",  0x000, 0x100, CRC(c3378ac2) SHA1(264fdc0718b36e02fc1fc1064a9566e349f4bf25) ) // tile red
	ROM_LOAD( "kjclr2.bin",  0x100, 0x100, CRC(2126da97) SHA1(6ca394a5977fab72200a00716a1f25f2a9447896) ) // tile green
	ROM_LOAD( "kjclr3.bin",  0x200, 0x100, CRC(fde62164) SHA1(d0f6b8d0dce63ce592a5f0c9dc8e6260f69a9141) ) // tile blue
	ROM_LOAD( "kjprom5.bin", 0x300, 0x020, CRC(5a81dd9f) SHA1(090ec9135b12e85ed02ab71fca55cc8d1ea8215a) ) // sprite palette
	ROM_LOAD( "kjprom4.bin", 0x320, 0x100, CRC(48dc2066) SHA1(b8007a5115d475b535284965681ae341f819d3db) ) // sprite clut
ROM_END

ROM_START( kncljoea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kj01.bin", 0x0000, 0x4000, CRC(f251019e) SHA1(a7ca2fae57ce698ec19e118e967c74eb92341803) )
	ROM_LOAD( "kj-2.bin", 0x4000, 0x4000, CRC(cb11514b) SHA1(c75d4019d1617493ff074ce8187a81ad70d9b60c) )
	ROM_LOAD( "kj-3.bin", 0x8000, 0x4000, CRC(0f50697b) SHA1(412c6aba270824299ca2a74e9bea42b83e69797b) )

	ROM_REGION( 0x8000, "soundcpu", 0 )
	ROM_LOAD( "kj-13.bin",0x6000, 0x2000, CRC(0a0be3f5) SHA1(00be47fc76500843b6f5de63622edb1748ef5f7d) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "kj-10.bin", 0x0000,  0x4000, CRC(74d3ba33) SHA1(c7887d690cb7f7a7b24d59d490ffc088fb6cc49c) )
	ROM_LOAD( "kj-11.bin", 0x4000,  0x4000, CRC(8ea01455) SHA1(b4b42fe373a1019b4f2a4b763a8a7219a5c9987e) )
	ROM_LOAD( "kj-12.bin", 0x8000,  0x4000, CRC(33367c41) SHA1(e6c56bcad008f3af4bc0f7d7afe8e23c8eb9d943) )

	ROM_REGION( 0x18000, "sprites1", 0 )
	ROM_LOAD( "kj-4.bin", 0x00000,  0x8000, CRC(a499ea10) SHA1(cb671cc75b3c6029dd3529e62d83025f78b45271) )
	ROM_LOAD( "kj-6.bin", 0x08000,  0x8000, CRC(815f5c0a) SHA1(ad0b59eeebb2e57035a3f643ac0ef575569bec0f) )
	ROM_LOAD( "kj-5.bin", 0x10000,  0x8000, CRC(11111759) SHA1(504c62fc6778a4afa86cba69634652708535bef6) )

	ROM_REGION( 0xc000, "sprites2", 0 )
	ROM_LOAD( "kj-7.bin", 0x0000,   0x4000, CRC(121fcccb) SHA1(77f3e7e49787d6a893c5d8c0c3ac612b1180e866) )
	ROM_LOAD( "kj-9.bin", 0x4000,   0x4000, CRC(affbe3eb) SHA1(056111fc5b04ff14b114b5f724d02789c8e3ee10) )
	ROM_LOAD( "kj-8.bin", 0x8000,   0x4000, CRC(e057e72a) SHA1(3a85750c72caaa027f302dc6ca4086bdbd49b5ff) )

	ROM_REGION( 0x420, "proms", 0 )
	ROM_LOAD( "kjclr1.bin",  0x000, 0x100, CRC(c3378ac2) SHA1(264fdc0718b36e02fc1fc1064a9566e349f4bf25) ) // tile red
	ROM_LOAD( "kjclr2.bin",  0x100, 0x100, CRC(2126da97) SHA1(6ca394a5977fab72200a00716a1f25f2a9447896) ) // tile green
	ROM_LOAD( "kjclr3.bin",  0x200, 0x100, CRC(fde62164) SHA1(d0f6b8d0dce63ce592a5f0c9dc8e6260f69a9141) ) // tile blue
	ROM_LOAD( "kjprom5.bin", 0x300, 0x020, CRC(5a81dd9f) SHA1(090ec9135b12e85ed02ab71fca55cc8d1ea8215a) ) // sprite palette
	ROM_LOAD( "kjprom4.bin", 0x320, 0x100, CRC(48dc2066) SHA1(b8007a5115d475b535284965681ae341f819d3db) ) // sprite clut
ROM_END

ROM_START( bcrusher )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bcrush1.bin", 0x0000, 0x4000, CRC(e8979196) SHA1(f1aff40e645760c786510c77a4841acb782ba157) )
	ROM_LOAD( "bcrush2.bin", 0x4000, 0x4000, CRC(1be4c731) SHA1(11f3a33263d66172902dfb6f3fe2d0ab5cad38d7) )
	ROM_LOAD( "bcrush3.bin", 0x8000, 0x4000, CRC(0772d993) SHA1(430f0319bd4765add2f1ee197e7217fdf9ae79c8) )

	ROM_REGION( 0x8000, "soundcpu", 0 )
	ROM_LOAD( "kj-13.bin",0x6000, 0x2000, CRC(0a0be3f5) SHA1(00be47fc76500843b6f5de63622edb1748ef5f7d) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "bcrush10.bin", 0x0000,  0x4000, CRC(a62f4572) SHA1(4e38e175e25a955e5f83cac8c935163e2e861e94) )
	ROM_LOAD( "bcrush11.bin", 0x4000,  0x4000, CRC(79cc5644) SHA1(bc356065a2475d0e0921fc5c84fa46f6629caae7) )
	ROM_LOAD( "bcrush12.bin", 0x8000,  0x4000, CRC(8f09641d) SHA1(5ccc423b15148d96c0a348d41a3f4fff7bbae7b9) )

	ROM_REGION( 0x18000, "sprites1", 0 )
	ROM_LOAD( "kj-4.bin", 0x00000,  0x8000, CRC(a499ea10) SHA1(cb671cc75b3c6029dd3529e62d83025f78b45271) )
	ROM_LOAD( "kj-6.bin", 0x08000,  0x8000, CRC(815f5c0a) SHA1(ad0b59eeebb2e57035a3f643ac0ef575569bec0f) )
	ROM_LOAD( "kj-5.bin", 0x10000,  0x8000, CRC(11111759) SHA1(504c62fc6778a4afa86cba69634652708535bef6) )

	ROM_REGION( 0xc000, "sprites2", 0 )
	ROM_LOAD( "kj-7.bin", 0x0000,   0x4000, CRC(121fcccb) SHA1(77f3e7e49787d6a893c5d8c0c3ac612b1180e866) )
	ROM_LOAD( "kj-9.bin", 0x4000,   0x4000, CRC(affbe3eb) SHA1(056111fc5b04ff14b114b5f724d02789c8e3ee10) )
	ROM_LOAD( "kj-8.bin", 0x8000,   0x4000, CRC(e057e72a) SHA1(3a85750c72caaa027f302dc6ca4086bdbd49b5ff) )

	ROM_REGION( 0x420, "proms", 0 )
	ROM_LOAD( "kjclr1.bin",  0x000, 0x100, CRC(c3378ac2) SHA1(264fdc0718b36e02fc1fc1064a9566e349f4bf25) ) // tile red
	ROM_LOAD( "kjclr2.bin",  0x100, 0x100, CRC(2126da97) SHA1(6ca394a5977fab72200a00716a1f25f2a9447896) ) // tile green
	ROM_LOAD( "kjclr3.bin",  0x200, 0x100, CRC(fde62164) SHA1(d0f6b8d0dce63ce592a5f0c9dc8e6260f69a9141) ) // tile blue
	ROM_LOAD( "kjprom5.bin", 0x300, 0x020, CRC(5a81dd9f) SHA1(090ec9135b12e85ed02ab71fca55cc8d1ea8215a) ) // sprite palette
	ROM_LOAD( "kjprom4.bin", 0x320, 0x100, CRC(48dc2066) SHA1(b8007a5115d475b535284965681ae341f819d3db) ) // sprite clut
ROM_END

} // anonymous namespace


GAME( 1985, kncljoe,  0,       kncljoe, kncljoe, kncljoe_state, empty_init, ROT0, "Seibu Kaihatsu (Taito license)", "Knuckle Joe (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, kncljoea, kncljoe, kncljoe, kncljoe, kncljoe_state, empty_init, ROT0, "Seibu Kaihatsu (Taito license)", "Knuckle Joe (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, bcrusher, kncljoe, kncljoe, kncljoe, kncljoe_state, empty_init, ROT0, "bootleg",                        "Bone Crusher",        MACHINE_SUPPORTS_SAVE )
