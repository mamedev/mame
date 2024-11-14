// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

    Atari Boxer (prototype) driver

    AKA Boxing, both game titles appear in the schematics

    This game had some weird controls that don't work well in MAME.

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/watchdog.h"
#include "emupal.h"
#include "screen.h"


namespace {

#define MASTER_CLOCK XTAL(12'096'000)

/*************************************
 *
 *  Driver data
 *
 *************************************/

class boxer_state : public driver_device
{
public:
	boxer_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tile_ram(*this, "tile_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_leds(*this, "led%u", 0U)
	{ }

	void boxer(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void boxer_map(address_map &map) ATTR_COLD;

	uint8_t input_r(offs_t offset);
	uint8_t misc_r(offs_t offset);
	void bell_w(uint8_t data);
	void sound_w(uint8_t data);
	void pot_w(uint8_t data);
	void irq_reset_w(uint8_t data);
	void crowd_w(uint8_t data);
	void led_w(uint8_t data);
	void boxer_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(pot_interrupt);
	TIMER_CALLBACK_MEMBER(periodic_callback);
	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_tile_ram;
	required_shared_ptr<uint8_t> m_sprite_ram;

	/* misc */
	uint8_t m_pot_state = 0;
	uint8_t m_pot_latch = 0;
	emu_timer *m_pot_interrupt = nullptr;
	emu_timer *m_periodic_timer = nullptr;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	output_finder<2> m_leds;
};

/*************************************
 *
 *  Interrupts / Timers
 *
 *************************************/

TIMER_CALLBACK_MEMBER(boxer_state::pot_interrupt)
{
	int mask = param;

	if (m_pot_latch & mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	m_pot_state |= mask;
}


TIMER_CALLBACK_MEMBER(boxer_state::periodic_callback)
{
	int scanline = param;

	m_maincpu->set_input_line(0, ASSERT_LINE);

	if (scanline == 0)
	{
		uint8_t mask[256];

		int i;

		memset(mask, 0, sizeof mask);

		mask[ioport("STICK0_X")->read()] |= 0x01;
		mask[ioport("STICK0_Y")->read()] |= 0x02;
		mask[ioport("PADDLE0")->read()]  |= 0x04;
		mask[ioport("STICK1_X")->read()] |= 0x08;
		mask[ioport("STICK1_Y")->read()] |= 0x10;
		mask[ioport("PADDLE1")->read()]  |= 0x20;

		for (i = 1; i < 256; i++)
			if (mask[i] != 0)
				m_pot_interrupt->adjust(m_screen->time_until_pos(i), mask[i]);

		m_pot_state = 0;
	}

	scanline += 64;

	if (scanline >= 262)
		scanline = 0;

	m_periodic_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


/*************************************
 *
 *  Video system
 *
 *************************************/

void boxer_state::boxer_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x00,0x00,0x00));
	palette.set_pen_color(1, rgb_t(0xff,0xff,0xff));

	palette.set_pen_color(2, rgb_t(0xff,0xff,0xff));
	palette.set_pen_color(3, rgb_t(0x00,0x00,0x00));
}

void boxer_state::draw( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int n;

	for (n = 0; n < 2; n++)
	{
		const uint8_t* p = memregion(n == 0 ? "user1" : "user2")->base();

		int i, j;

		int x = 196 - m_sprite_ram[0 + 2 * n];
		int y = 192 - m_sprite_ram[1 + 2 * n];

		int l = m_sprite_ram[4 + 2 * n] & 15;
		int r = m_sprite_ram[5 + 2 * n] & 15;

		for (i = 0; i < 8; i++)
		{
			for (j = 0; j < 4; j++)
			{
				uint8_t code;

				code = p[32 * l + 4 * i + j];


					m_gfxdecode->gfx(n)->transpen(bitmap,cliprect,
					code,
					0,
					code & 0x80, 0,
					x + 8 * j,
					y + 8 * i, 1);

				code = p[32 * r + 4 * i - j + 3];


					m_gfxdecode->gfx(n)->transpen(bitmap,cliprect,
					code,
					0,
					!(code & 0x80), 0,
					x + 8 * j + 32,
					y + 8 * i, 1);
			}
		}
	}
}


uint32_t boxer_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, j;

	bitmap.fill(1, cliprect);

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 32; j++)
		{
			uint8_t code = m_tile_ram[32 * i + j];


				m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
				code,
				0,
				code & 0x40, code & 0x40,
				8 * j + 4,
				8 * (i % 2) + 32 * (i / 2), 0);
		}
	}

	draw(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

uint8_t boxer_state::input_r(offs_t offset)
{
	uint8_t val = ioport("IN0")->read();

	if (ioport("IN3")->read() < m_screen->vpos())
		val |= 0x02;

	return (val << ((offset & 7) ^ 7)) & 0x80;
}


uint8_t boxer_state::misc_r(offs_t offset)
{
	uint8_t val = 0;

	switch (offset & 3)
	{
	case 0:
		val = m_pot_state & m_pot_latch;
		break;

	case 1:
		val = m_screen->vpos();
		break;

	case 2:
		val = ioport("IN1")->read();
		break;

	case 3:
		val = ioport("IN2")->read();
		break;
	}

	return val ^ 0x3f;
}




void boxer_state::bell_w(uint8_t data)
{
}


void boxer_state::sound_w(uint8_t data)
{
}


void boxer_state::pot_w(uint8_t data)
{
	/* BIT0 => HPOT1 */
	/* BIT1 => VPOT1 */
	/* BIT2 => RPOT1 */
	/* BIT3 => HPOT2 */
	/* BIT4 => VPOT2 */
	/* BIT5 => RPOT2 */

	m_pot_latch = data & 0x3f;

	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


void boxer_state::irq_reset_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


void boxer_state::crowd_w(uint8_t data)
{
	/* BIT0 => ATTRACT */
	/* BIT1 => CROWD-1 */
	/* BIT2 => CROWD-2 */
	/* BIT3 => CROWD-3 */

	machine().bookkeeping().coin_lockout_global_w(data & 1);
}


void boxer_state::led_w(uint8_t data)
{
	m_leds[1] = BIT(~data, 0);
	m_leds[0] = BIT(~data, 1);
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void boxer_state::boxer_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x01ff).ram();
	map(0x0200, 0x03ff).ram().share("tile_ram");
	map(0x0800, 0x08ff).r(FUNC(boxer_state::input_r));
	map(0x1000, 0x17ff).r(FUNC(boxer_state::misc_r));
	map(0x1800, 0x1800).w(FUNC(boxer_state::pot_w));
	map(0x1900, 0x19ff).w(FUNC(boxer_state::led_w));
	map(0x1a00, 0x1aff).w(FUNC(boxer_state::sound_w));
	map(0x1b00, 0x1bff).w(FUNC(boxer_state::crowd_w));
	map(0x1c00, 0x1cff).w(FUNC(boxer_state::irq_reset_w));
	map(0x1d00, 0x1dff).w(FUNC(boxer_state::bell_w));
	map(0x1e00, 0x1eff).writeonly().share("sprite_ram");
	map(0x1f00, 0x1fff).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x3000, 0x3fff).rom();
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( boxer )
	PORT_START("IN0")
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT ( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) /* TIMER */
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT ( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT ( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x01, "Number of Rounds" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )

	PORT_START("STICK0_X")
	PORT_BIT ( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(30) PORT_KEYDELTA(16) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("STICK0_Y")
	PORT_BIT ( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(30) PORT_KEYDELTA(16) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("PADDLE0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(30) PORT_KEYDELTA(16) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START("STICK1_X")
	PORT_BIT ( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(30) PORT_KEYDELTA(16) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("STICK1_Y")
	PORT_BIT ( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(30) PORT_KEYDELTA(16) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("PADDLE1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(30) PORT_KEYDELTA(16) PORT_CODE_DEC(KEYCODE_Q) PORT_CODE_INC(KEYCODE_W) PORT_CENTERDELTA(0) PORT_PLAYER(2)

	PORT_START("IN3")
	PORT_DIPNAME( 0xff, 0x5c, "Round Time" ) /* actually a potentiometer */
	PORT_DIPSETTING(    0x3c, "15 seconds" )
	PORT_DIPSETTING(    0x5c, "30 seconds" )
	PORT_DIPSETTING(    0x7c, "45 seconds" )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tile_layout =
{
	8, 8,
	64,
	1,
	{ 0 },
	{
		0x7, 0x6, 0x5, 0x4, 0xf, 0xe, 0xd, 0xc
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80
};


static const gfx_layout sprite_layout =
{
	8, 8,
	64,
	1,
	{ 0 },
	{
		0x4, 0x5, 0x6, 0x7, 0xc, 0xd, 0xe, 0xf
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80
};


static GFXDECODE_START( gfx_boxer )
	GFXDECODE_ENTRY( "gfx1", 0, sprite_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, tile_layout, 2, 1 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void boxer_state::machine_start()
{
	m_leds.resolve();
	m_pot_interrupt = timer_alloc(FUNC(boxer_state::pot_interrupt), this);
	m_periodic_timer = timer_alloc(FUNC(boxer_state::periodic_callback), this);

	save_item(NAME(m_pot_state));
	save_item(NAME(m_pot_latch));
}

void boxer_state::machine_reset()
{
	m_periodic_timer->adjust(m_screen->time_until_pos(0));

	m_pot_state = 0;
	m_pot_latch = 0;
}


void boxer_state::boxer(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, MASTER_CLOCK / 16);
	m_maincpu->set_addrmap(AS_PROGRAM, &boxer_state::boxer_map);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(256, 262);
	m_screen->set_visarea(8, 247, 0, 239);
	m_screen->set_screen_update(FUNC(boxer_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_boxer);
	PALETTE(config, m_palette, FUNC(boxer_state::boxer_palette), 4);

	/* sound hardware */
}


/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( boxer )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "3400l.e1", 0x3400, 0x0400, CRC(df85afa4) SHA1(5a74a08f1e0b0bbec02999d5e46513d8afd333ac) )
	ROM_LOAD_NIB_HIGH( "3400m.a1", 0x3400, 0x0400, CRC(23fe06aa) SHA1(03a4eedbf60f07d1dd8d7af576828df5f032146e) )
	ROM_LOAD_NIB_LOW ( "3800l.j1", 0x3800, 0x0400, CRC(087263fb) SHA1(cc3715a68bd05f23b4abf9f18ca14a8fe55163f7) )
	ROM_LOAD_NIB_HIGH( "3800m.d1", 0x3800, 0x0400, CRC(3bbf605e) SHA1(be4ff1702eb837710421a7dafcdc60fe2d3259e8) )
	ROM_LOAD_NIB_LOW ( "3c00l.h1", 0x3C00, 0x0400, CRC(09e204f2) SHA1(565d4c8865da7d96a45e909973d570101de61f63) )
	ROM_LOAD_NIB_HIGH( "3c00m.c1", 0x3C00, 0x0400, CRC(2f8ebc85) SHA1(05a4e29ec7e49173200d5fe5344274fd6afd16d7) )

	ROM_REGION( 0x0400, "gfx1", 0 ) /* lower boxer */
	ROM_LOAD( "bx137l.c8", 0x0000, 0x0400, CRC(e91f2048) SHA1(64039d07557e210aa4f6663cd7e72814cb881310) )

	ROM_REGION( 0x0400, "gfx2", 0 ) /* upper boxer */
	ROM_LOAD( "bx137u.m8", 0x0000, 0x0400, CRC(e4fee386) SHA1(79b70aca4a92c56363689a363b643d46294d3e88) )

	ROM_REGION( 0x0400, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "9417.k2", 0x0000, 0x0400, CRC(7e3d22cf) SHA1(92e6bbe049dc8fcd674f2ff96cde3786f714508d) )

	ROM_REGION( 0x0200, "user1", 0 ) /* lower boxer map */
	ROM_LOAD( "bx115l.b7", 0x0000, 0x0200, CRC(31f2234f) SHA1(d53f3a1d0db3cf3024de61ef64f76c6dfdf6861c) )

	ROM_REGION( 0x0200, "user2", 0 ) /* upper boxer map */
	ROM_LOAD( "bx115u.l7", 0x0000, 0x0200, CRC(124d3f24) SHA1(09fab2ae218b8584c0e3c8e02f5680ce083a33d6) )

	ROM_REGION( 0x0100, "proms", 0 ) /* sync prom */
	ROM_LOAD( "9402.m3", 0x0000, 0x0100, CRC(00e224a0) SHA1(1a384ef488791c62566c91b18d6a1fb4a5def2ba) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1978, boxer, 0, boxer, boxer, boxer_state, empty_init, 0, "Atari", "Boxer (prototype)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
