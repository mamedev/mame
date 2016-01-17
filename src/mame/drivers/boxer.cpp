// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

    Atari Boxer (prototype) driver

    AKA Boxing, both game titles appear in the schematics

    This game had some weird controls that don't work well in MAME.

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"


#define MASTER_CLOCK XTAL_12_096MHz

/*************************************
 *
 *  Driver data
 *
 *************************************/

class boxer_state : public driver_device
{
public:
	enum
	{
		TIMER_POT_INTERRUPT,
		TIMER_PERIODIC
	};

	boxer_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_tile_ram(*this, "tile_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_tile_ram;
	required_shared_ptr<UINT8> m_sprite_ram;

	/* misc */
	UINT8 m_pot_state;
	UINT8 m_pot_latch;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_READ8_MEMBER(boxer_input_r);
	DECLARE_READ8_MEMBER(boxer_misc_r);
	DECLARE_WRITE8_MEMBER(boxer_bell_w);
	DECLARE_WRITE8_MEMBER(boxer_sound_w);
	DECLARE_WRITE8_MEMBER(boxer_pot_w);
	DECLARE_WRITE8_MEMBER(boxer_irq_reset_w);
	DECLARE_WRITE8_MEMBER(boxer_crowd_w);
	DECLARE_WRITE8_MEMBER(boxer_led_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(boxer);
	UINT32 screen_update_boxer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(pot_interrupt);
	TIMER_CALLBACK_MEMBER(periodic_callback);
	void draw_boxer( bitmap_ind16 &bitmap, const rectangle &cliprect );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

/*************************************
 *
 *  Interrupts / Timers
 *
 *************************************/

void boxer_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_POT_INTERRUPT:
		pot_interrupt(ptr, param);
		break;
	case TIMER_PERIODIC:
		periodic_callback(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in boxer_state::device_timer");
	}
}

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
		UINT8 mask[256];

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
				timer_set(m_screen->time_until_pos(i), TIMER_POT_INTERRUPT, mask[i]);

		m_pot_state = 0;
	}

	scanline += 64;

	if (scanline >= 262)
		scanline = 0;

	timer_set(m_screen->time_until_pos(scanline), TIMER_PERIODIC, scanline);
}


/*************************************
 *
 *  Video system
 *
 *************************************/

PALETTE_INIT_MEMBER(boxer_state, boxer)
{
	palette.set_pen_color(0, rgb_t(0x00,0x00,0x00));
	palette.set_pen_color(1, rgb_t(0xff,0xff,0xff));

	palette.set_pen_color(2, rgb_t(0xff,0xff,0xff));
	palette.set_pen_color(3, rgb_t(0x00,0x00,0x00));
}

void boxer_state::draw_boxer( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int n;

	for (n = 0; n < 2; n++)
	{
		const UINT8* p = memregion(n == 0 ? "user1" : "user2")->base();

		int i, j;

		int x = 196 - m_sprite_ram[0 + 2 * n];
		int y = 192 - m_sprite_ram[1 + 2 * n];

		int l = m_sprite_ram[4 + 2 * n] & 15;
		int r = m_sprite_ram[5 + 2 * n] & 15;

		for (i = 0; i < 8; i++)
		{
			for (j = 0; j < 4; j++)
			{
				UINT8 code;

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


UINT32 boxer_state::screen_update_boxer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, j;

	bitmap.fill(1, cliprect);

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 32; j++)
		{
			UINT8 code = m_tile_ram[32 * i + j];


				m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
				code,
				0,
				code & 0x40, code & 0x40,
				8 * j + 4,
				8 * (i % 2) + 32 * (i / 2), 0);
		}
	}

	draw_boxer(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

READ8_MEMBER(boxer_state::boxer_input_r)
{
	UINT8 val = ioport("IN0")->read();

	if (ioport("IN3")->read() < m_screen->vpos())
		val |= 0x02;

	return (val << ((offset & 7) ^ 7)) & 0x80;
}


READ8_MEMBER(boxer_state::boxer_misc_r)
{
	UINT8 val = 0;

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




WRITE8_MEMBER(boxer_state::boxer_bell_w)
{
}


WRITE8_MEMBER(boxer_state::boxer_sound_w)
{
}


WRITE8_MEMBER(boxer_state::boxer_pot_w)
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


WRITE8_MEMBER(boxer_state::boxer_irq_reset_w)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


WRITE8_MEMBER(boxer_state::boxer_crowd_w)
{
	/* BIT0 => ATTRACT */
	/* BIT1 => CROWD-1 */
	/* BIT2 => CROWD-2 */
	/* BIT3 => CROWD-3 */

	machine().bookkeeping().coin_lockout_global_w(data & 1);
}


WRITE8_MEMBER(boxer_state::boxer_led_w)
{
	output().set_led_value(1, !(data & 1));
	output().set_led_value(0, !(data & 2));
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( boxer_map, AS_PROGRAM, 8, boxer_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x0200, 0x03ff) AM_RAM AM_SHARE("tile_ram")
	AM_RANGE(0x0800, 0x08ff) AM_READ(boxer_input_r)
	AM_RANGE(0x1000, 0x17ff) AM_READ(boxer_misc_r)
	AM_RANGE(0x1800, 0x1800) AM_WRITE(boxer_pot_w)
	AM_RANGE(0x1900, 0x19ff) AM_WRITE(boxer_led_w)
	AM_RANGE(0x1a00, 0x1aff) AM_WRITE(boxer_sound_w)
	AM_RANGE(0x1b00, 0x1bff) AM_WRITE(boxer_crowd_w)
	AM_RANGE(0x1c00, 0x1cff) AM_WRITE(boxer_irq_reset_w)
	AM_RANGE(0x1d00, 0x1dff) AM_WRITE(boxer_bell_w)
	AM_RANGE(0x1e00, 0x1eff) AM_WRITEONLY AM_SHARE("sprite_ram")
	AM_RANGE(0x1f00, 0x1fff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x3000, 0x3fff) AM_ROM
ADDRESS_MAP_END


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


static GFXDECODE_START( boxer )
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
	save_item(NAME(m_pot_state));
	save_item(NAME(m_pot_latch));
}

void boxer_state::machine_reset()
{
	timer_set(m_screen->time_until_pos(0), TIMER_PERIODIC);

	m_pot_state = 0;
	m_pot_latch = 0;
}


static MACHINE_CONFIG_START( boxer, boxer_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MASTER_CLOCK / 16)
	MCFG_CPU_PROGRAM_MAP(boxer_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 262)
	MCFG_SCREEN_VISIBLE_AREA(8, 247, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(boxer_state, screen_update_boxer)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", boxer)
	MCFG_PALETTE_ADD("palette", 4)
	MCFG_PALETTE_INIT_OWNER(boxer_state, boxer)

	/* sound hardware */
MACHINE_CONFIG_END


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


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1978, boxer, 0, boxer, boxer, driver_device, 0, 0, "Atari", "Boxer (prototype)", MACHINE_NO_SOUND )
