// license:BSD-3-Clause
// copyright-holders: Zsolt Vasvari

/***************************************************************************

Funky Bee/Sky Lancer memory map (preliminary)

driver by Zsolt Vasvari

MAIN CPU:

0000-4fff ROM
8000-87ff RAM
a000-bfff video RAM (only 0x20 bytes of each 0x100 byte block are used)
                    (also contains sprite RAM)
c000-dfff color RAM (only 0x20 bytes of each 0x100 byte block are used)
                    (also contains sprite RAM)

read:
f000      interrupt ACK
f800      IN0/watchdog
f801      IN1
f802      IN2

write:
e000      row scroll
e800      flip screen
e802-e803 coin counter
e804      ???
e805      gfx bank select
e806      ???
f800      watchdog


I/0 ports:
write
00        8912  control
01        8912  write

AY-3-8912 Port A = DSW

The Sky Lancer PCB has an unpopulated space for a second AY-3-8912.


Stephh's notes (based on the games Z80 code and some tests) :

1) 'funkybee' and clones

1a) 'funkybee'

  - Possible "Lives" settings : 3, 4, 5 or 6 (code at 0x0501)
  - Bonus life routine at 0x2d03 (test on DSW bit 6)

1b) 'funkybeeb'

  - Removal of ORCA copyright on title screen (text at 0x0e9a).
    However, high scores table remains unchanged.
  - Bypass ROM check (code at 0x3ee3)
  - Possible "Lives" settings : 1, 2, 3 or 4 (code at 0x0501)
  - Bonus life routine at 0x2d03 (test on DSW bit 6)

2) 'skylancr' and clones

2a) 'skylancr'

  - Possible "Lives" settings : 1, 2, 3 or 4 (code at 0x0601)
  - Bonus life routine at 0x1ef6 (test on DSW bit 5 !)
    I can't tell if it's an in-game bug or if this was done on purpose,
    but "Bonus Life" settings depend on the starting number of lives.
    DSW bit 6 has no effect because of this.

2a) 'skylancre'

  - Possible "Lives" settings : 3, 4, 5 or 6 (code at 0x0601)
  - Bonus life routine at 0x1f28 (test on DSW bit 6)

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class funkybee_state : public driver_device
{
public:
	funkybee_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_in0(*this, "IN0")
	{ }

	void funkybee(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// I/O
	required_ioport m_in0;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_gfx_bank = 0U;
	uint8_t input_port_0_r();
	template <uint8_t Which> void coin_counter_w(int state);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void gfx_bank_w(int state);
	void scroll_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_columns(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void funkybee_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// first, the character/sprite palette
	for (int i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = 0;
		bit1 = BIT(*color_prom, 6);
		bit2 = BIT(*color_prom, 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}

void funkybee_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void funkybee_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void funkybee_state::gfx_bank_w(int state)
{
	m_gfx_bank = state;
	machine().tilemap().mark_all_dirty();
}

void funkybee_state::scroll_w(uint8_t data)
{
	m_bg_tilemap->set_scrollx(0, flip_screen() ? -data : data);
}

TILE_GET_INFO_MEMBER(funkybee_state::get_bg_tile_info)
{
	int const code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x80) << 1);
	int const color = m_colorram[tile_index] & 0x03;

	tileinfo.set(m_gfx_bank, code, color, 0);
}

TILEMAP_MAPPER_MEMBER(funkybee_state::tilemap_scan)
{
	// logical (col,row) -> memory offset
	return 256 * row + col;
}

void funkybee_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(funkybee_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(funkybee_state::tilemap_scan)), 8, 8, 32, 32);
}

void funkybee_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0x0f; offs >= 0; offs--)
	{
		int const offs2 = offs + 0x1e00;
		int const attr = m_videoram[offs2];
		int const code = (attr >> 2) | ((attr & 2) << 5);
		int const color = m_colorram[offs2 + 0x10];
		int flipx = 0;
		int const flipy = attr & 0x01;
		int const sx = m_videoram[offs2 + 0x10];
		int sy = 224 - m_colorram[offs2];

		if (flip_screen())
		{
			sy += 32;
			flipx = !flipx;
		}

		m_gfxdecode->gfx(2 + m_gfx_bank)->transpen(bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

void funkybee_state::draw_columns(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0x1f; offs >= 0; offs--)
	{
		int const flip = flip_screen();
		int code = m_videoram[0x1c00 + offs];
		int color = m_colorram[0x1f10] & 0x03;
		int sx = flip ? m_videoram[0x1f1f] : m_videoram[0x1f10];
		int sy = offs * 8;

		if (flip)
			sy = 248 - sy;

		m_gfxdecode->gfx(m_gfx_bank)->transpen(bitmap, cliprect,
				code, color,
				flip, flip,
				sx, sy, 0);

		code = m_videoram[0x1d00 + offs];
		color = m_colorram[0x1f11] & 0x03;
		sx = flip ? m_videoram[0x1f1e] : m_videoram[0x1f11];
		sy = offs * 8;

		if (flip)
			sy = 248 - sy;

		m_gfxdecode->gfx(m_gfx_bank)->transpen(bitmap, cliprect,
				code, color,
				flip, flip,
				sx, sy, 0);
	}
}

uint32_t funkybee_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	draw_columns(bitmap, cliprect);

	return 0;
}


uint8_t funkybee_state::input_port_0_r()
{
	if (!machine().side_effects_disabled())
		m_watchdog->watchdog_reset();

	return m_in0->read();
}

template <uint8_t Which>
void funkybee_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

void funkybee_state::prg_map(address_map &map)
{
	map(0x0000, 0x4fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xbfff).ram().w(FUNC(funkybee_state::videoram_w)).share(m_videoram);
	map(0xc000, 0xdfff).ram().w(FUNC(funkybee_state::colorram_w)).share(m_colorram);
	map(0xe000, 0xe000).w(FUNC(funkybee_state::scroll_w));
	map(0xe800, 0xe807).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xf000, 0xf000).nopr(); // IRQ Ack
	map(0xf800, 0xf800).r(FUNC(funkybee_state::input_port_0_r)).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0xf801, 0xf801).portr("IN1");
	map(0xf802, 0xf802).portr("IN2");
}

void funkybee_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x02, 0x02).r("aysnd", FUNC(ay8910_device::data_r));
}


static INPUT_PORTS_START( funkybee )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "20000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( funkybeeb )
	PORT_INCLUDE(funkybee)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END

// NOTE: available manual claims Lives 3-6 as per skylancre set, without an explicit mention to Esco Trading.
// Is current skylancr set an earlier/location test rev? Also cfr. arguably worse enemy colors right off the bat.
static INPUT_PORTS_START( skylancr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4,3") // Also affects bonus life
	PORT_DIPSETTING(    0x30, "1" )             // Bonus life at 20000 and 50000
	PORT_DIPSETTING(    0x20, "2" )             // Bonus life at 20000 and 50000
	PORT_DIPSETTING(    0x10, "3" )             // Bonus life at 40000 and 70000
	PORT_DIPSETTING(    0x00, "4" )             // Bonus life at 40000 and 70000
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( skylancre )
	PORT_INCLUDE(skylancr)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4,3") // Also affects bonus life
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:2") // Manual calls this "Excent Play" (Excellent or extended?)
	PORT_DIPSETTING(    0x40, "20000 50000" )       // Manual calls this "Normal Level"
	PORT_DIPSETTING(    0x00, "40000 70000" )       // Manual calls this "High Level"
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	8,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
		32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8,
		48*8, 49*8, 50*8, 51*8, 52*8, 53*8, 54*8, 55*8 },
	4*16*8
};

static GFXDECODE_START( gfx_funkybee )
	GFXDECODE_ENTRY( "gfxbank1", 0, charlayout,   0, 8 )
	GFXDECODE_ENTRY( "gfxbank2", 0, charlayout,   0, 8 )
	GFXDECODE_ENTRY( "gfxbank1", 0, spritelayout, 16, 4 )
	GFXDECODE_ENTRY( "gfxbank2", 0, spritelayout, 16, 4 )
GFXDECODE_END


void funkybee_state::machine_start()
{
	save_item(NAME(m_gfx_bank));

	m_gfx_bank = 0;
}

void funkybee_state::funkybee(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 3'072'000);   // 3.072 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &funkybee_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &funkybee_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(funkybee_state::irq0_line_hold));

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set(FUNC(funkybee_state::flip_screen_set));
	mainlatch.q_out_cb<2>().set(FUNC(funkybee_state::coin_counter_w<0>));
	mainlatch.q_out_cb<3>().set(FUNC(funkybee_state::coin_counter_w<1>));
	mainlatch.q_out_cb<5>().set(FUNC(funkybee_state::gfx_bank_w));

	WATCHDOG_TIMER(config, m_watchdog);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*8, 32*8);
	screen.set_visarea(12, 32*8-8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(funkybee_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_funkybee);
	PALETTE(config, m_palette, FUNC(funkybee_state::palette), 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8912_device &ay8912(AY8912(config, "aysnd", 1'500'000)); // AY-3-8912 verified for Sky Lancer
	ay8912.port_a_read_callback().set_ioport("DSW");
	ay8912.add_route(ALL_OUTPUTS, "mono", 0.50);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( funkybee )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "funkybee.1",    0x0000, 0x1000, CRC(3372cb33) SHA1(09f2673cdeaadba8211d86a19e727aebb4d8be9d) )
	ROM_LOAD( "funkybee.3",    0x1000, 0x1000, CRC(7bf7c62f) SHA1(f8e5514c17fddb8ed95e5e18aab81ad0ebcc41af) )
	ROM_LOAD( "funkybee.2",    0x2000, 0x1000, CRC(8cc0fe8e) SHA1(416d97db0a2219ea46f2caa55787253e16a5ef32) )
	ROM_LOAD( "funkybee.4",    0x3000, 0x1000, CRC(1e1aac26) SHA1(a2974e6a8da5568f91aa44adb58941b0a60b1536) )

	ROM_REGION( 0x2000, "gfxbank1", 0 )
	ROM_LOAD( "funkybee.5",    0x0000, 0x2000, CRC(86126655) SHA1(d91682121d7f6a70f10a946ab81b248cc29bdf8c) )

	ROM_REGION( 0x2000, "gfxbank2", 0 )
	ROM_LOAD( "funkybee.6",    0x0000, 0x2000, CRC(5fffd323) SHA1(9de9c869bd1e2daab3b94275444ecbe904bcd6aa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "funkybee.clr",  0x0000, 0x0020, CRC(e2cf5fe2) SHA1(50b293f48f078cbcebccb045aa779ced2fb298c8) )
ROM_END

/* This is a bootleg of "Funky Bee", where ORCA copyright has been removed and difficulty is harder,
   there are 2 lives less then in the original game */
ROM_START( funkybeeb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "senza_orca.fb1", 0x0000, 0x1000, CRC(7f2e7f85) SHA1(d4b63add3a97fc80aeafcd72a261302ab52d60a7) )
	ROM_LOAD( "funkybee.3",     0x1000, 0x1000, CRC(7bf7c62f) SHA1(f8e5514c17fddb8ed95e5e18aab81ad0ebcc41af) )
	ROM_LOAD( "funkybee.2",     0x2000, 0x1000, CRC(8cc0fe8e) SHA1(416d97db0a2219ea46f2caa55787253e16a5ef32) )
	ROM_LOAD( "senza_orca.fb4", 0x3000, 0x1000, CRC(53c2db3b) SHA1(0bda1eb87d7c41b67a5ff00b6675defdc8fe9274) )

	ROM_REGION( 0x2000, "gfxbank1", 0 )
	ROM_LOAD( "funkybee.5",     0x0000, 0x2000, CRC(86126655) SHA1(d91682121d7f6a70f10a946ab81b248cc29bdf8c) )

	ROM_REGION( 0x2000, "gfxbank2", 0 )
	ROM_LOAD( "funkybee.6",     0x0000, 0x2000, CRC(5fffd323) SHA1(9de9c869bd1e2daab3b94275444ecbe904bcd6aa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "funkybee.clr",   0x0000, 0x0020, CRC(e2cf5fe2) SHA1(50b293f48f078cbcebccb045aa779ced2fb298c8) )
ROM_END

ROM_START( skylancr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1sl.5a",        0x0000, 0x2000, CRC(e80b315e) SHA1(0c02aa9f0d4bdfc3482c400d0e4e38fd3912a512) )
	ROM_LOAD( "2sl.5c",        0x2000, 0x2000, CRC(9d70567b) SHA1(05ff6f0c4b4d928e937556d9943a76f6cbc0f05f) )
	ROM_LOAD( "3sl.5d",        0x4000, 0x2000, CRC(64c39457) SHA1(b54a57a8576c2f852b765350c4504ccc3f5a431c) )

	ROM_REGION( 0x2000, "gfxbank1", 0 )
	ROM_LOAD( "4sl.6a",        0x0000, 0x2000, CRC(9b4469a5) SHA1(a0964e6d4fbdd15153be258f0d78680559a962f2) )

	ROM_REGION( 0x2000, "gfxbank2", 0 )
	ROM_LOAD( "5sl.6c",        0x0000, 0x2000, CRC(29afa134) SHA1(d94f483b4d234fe0b1d406322409417daec092f2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "18s030.1a",     0x0000, 0x0020, CRC(e645bacb) SHA1(5f4c299c4cf165fd229731c0e5799a34892bf28e) )
ROM_END

ROM_START( skylancre )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.5a",          0x0000, 0x2000, CRC(82d55824) SHA1(5c457e720ac8611bea4bc7e63ba4ee1c11200471) )
	ROM_LOAD( "2.5c",          0x2000, 0x2000, CRC(dff3a682) SHA1(e3197e106c2c6d198d2769b63701222d48a196d1) )
	ROM_LOAD( "3.5d",          0x4000, 0x1000, CRC(7c006ee6) SHA1(22719d4d0ad5c4f534a1613e0d74cab73973bab7) )

	ROM_REGION( 0x2000, "gfxbank1", 0 )
	ROM_LOAD( "4.6a",          0x0000, 0x2000, CRC(0f8ede07) SHA1(e04456fe12e2282191aee4823941f23ad8bda99d) )

	ROM_REGION( 0x2000, "gfxbank2", 0 )
	ROM_LOAD( "5.6b",          0x0000, 0x2000, CRC(24cec070) SHA1(2b7977b07acbe1394765675cd469db13a3b495f2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "18s030.1a",     0x0000, 0x0020, CRC(e645bacb) SHA1(5f4c299c4cf165fd229731c0e5799a34892bf28e) )
ROM_END

} // anonymous namespace


GAME( 1982, funkybee,  0,        funkybee, funkybee,  funkybee_state, empty_init, ROT90, "Orca",                           "Funky Bee",                            MACHINE_SUPPORTS_SAVE )
GAME( 1982, funkybeeb, funkybee, funkybee, funkybeeb, funkybee_state, empty_init, ROT90, "bootleg",                        "Funky Bee (bootleg, harder)",          MACHINE_SUPPORTS_SAVE )
GAME( 1983, skylancr,  0,        funkybee, skylancr,  funkybee_state, empty_init, ROT90, "Orca",                           "Sky Lancer",                           MACHINE_SUPPORTS_SAVE )
GAME( 1983, skylancre, skylancr, funkybee, skylancre, funkybee_state, empty_init, ROT90, "Orca (Esco Trading Co license)", "Sky Lancer (Esco Trading Co license)", MACHINE_SUPPORTS_SAVE )
