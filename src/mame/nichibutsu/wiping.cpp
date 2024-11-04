// license:BSD-3-Clause
// copyright-holders: Allard van der Bas

/***************************************************************************
                Wiping
                (C) 1982 Nichibutsu

                    driver by

            Allard van der Bas (allard@mindless.com)

1 x Z80 CPU main game, 1 x Z80 with ???? sound hardware.

Given the similarities with clshroad.cpp this was probably developed by
Masao Suzuki, who later left Nichibutsu to form Woodplace Inc.

----------------------------------------------------------------------------
Main processor :

0xA800 - 0xA807 : 64 bits of input and dipswitches.

dip: 0.7 1.7 2.7
       0   0   0    coin 1: 1 coin 0 credit.

       1   1   1    coin 1: 1 coin 7 credit.

dip: 3.7 4.7 5.7
       0   0   0    coin 2: 0 coin 1 credit.

       1   1   1    coin 2: 7 coin 1 credit.

dip:  7.6
    0       bonus at 30K and 70K
    1       bonus at 50K and 150K

dip: 6.7 7.7
       0   0        2 lives
       0   1        3 lives
       1   0        4 lives
       1   1        5 lives

***************************************************************************/

#include "emu.h"

#include "wiping_a.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class wiping_state : public driver_device
{
public:
	wiping_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_io_port(*this, { "P1", "P2", "IN2", "IN3", "IN4", "IN5", "SYSTEM", "DSW" })
	{ }

	void wiping(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_ioport_array<8> m_io_port;

	uint8_t m_flipscreen = 0;
	uint8_t m_main_irq_mask = 0;
	uint8_t m_sound_irq_mask = 0;

	uint8_t ports_r(offs_t offset);
	void main_irq_mask_w(int state);
	void sound_irq_mask_w(int state);
	void flipscreen_w(int state);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(sound_timer_irq);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

void wiping_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 470, 0,
			3, &resistances_rg[0], gweights, 470, 0,
			2, &resistances_b[0],  bweights, 470, 0);

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

	// chars use colors 0-15
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = color_prom[i ^ 0x03] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	// sprites use colors 16-31
	for (int i = 0x100; i < 0x200; i++)
	{
		uint8_t const ctabentry = (color_prom[i ^ 0x03] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}
}



void wiping_state::flipscreen_w(int state)
{
	m_flipscreen = state;
}


uint32_t wiping_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0x3ff; offs > 0; offs--)
	{
		int sx, sy;

		int const mx = offs % 32;
		int const my = offs / 32;

		if (my < 2)
		{
			sx = my + 34;
			sy = mx - 2;
		}
		else if (my >= 30)
		{
			sx = my - 30;
			sy = mx - 2;
		}
		else
		{
			sx = mx + 2;
			sy = my - 2;
		}

		if (m_flipscreen)
		{
			sx = 35 - sx;
			sy = 27 - sy;
		}

		m_gfxdecode->gfx(0)->opaque(bitmap, cliprect,
				m_videoram[offs],
				m_colorram[offs] & 0x3f,
				m_flipscreen, m_flipscreen,
				sx * 8, sy * 8);
	}

	/* Note, we're counting up on purpose !
	   This way the vacuum cleaner is always on top */
	for (int offs = 0x0; offs < 128; offs += 2)
	{
		int const sx = m_spriteram[offs + 0x100 + 1] + ((m_spriteram[offs + 0x81] & 0x01) << 8) - 40;
		int sy = 224 - m_spriteram[offs + 0x100];
		int const color = m_spriteram[offs + 1] & 0x3f;

		int const otherbank = m_spriteram[offs + 0x80] & 0x01;

		int flipy = m_spriteram[offs] & 0x40;
		int flipx = m_spriteram[offs] & 0x80;

		if (m_flipscreen)
		{
			sy = 208 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transmask(bitmap, cliprect,
			(m_spriteram[offs] & 0x3f) + 64 * otherbank,
			color,
			flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0x1f));
	}

	// redraw high priority chars
	for (int offs = 0x3ff; offs > 0; offs--)
	{
		if (m_colorram[offs] & 0x80)
		{
			int sx, sy;

			int const mx = offs % 32;
			int const my = offs / 32;

			if (my < 2)
			{
				sx = my + 34;
				sy = mx - 2;
			}
			else if (my >= 30)
			{
				sx = my - 30;
				sy = mx - 2;
			}
			else
			{
				sx = mx + 2;
				sy = my - 2;
			}

			if (m_flipscreen)
			{
				sx = 35 - sx;
				sy = 27 - sy;
			}

			m_gfxdecode->gfx(0)->opaque(bitmap, cliprect,
					m_videoram[offs],
					m_colorram[offs] & 0x3f,
					m_flipscreen, m_flipscreen,
					sx * 8, sy * 8);
			}
	}

	return 0;
}


void wiping_state::machine_start()
{
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_main_irq_mask));
	save_item(NAME(m_sound_irq_mask));
}

// input ports are rotated 90 degrees
uint8_t wiping_state::ports_r(offs_t offset)
{
	int res = 0;

	for (int i = 0; i < 8; i++)
		res |= ((m_io_port[i]->read() >> offset) & 1) << i;

	return res;
}

// irq / reset controls like in clshroad.cpp

void wiping_state::main_irq_mask_w(int state)
{
	m_main_irq_mask = state;
}

void wiping_state::sound_irq_mask_w(int state)
{
	m_sound_irq_mask = state;
}

void wiping_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x8000, 0x83ff).ram().share(m_videoram);
	map(0x8400, 0x87ff).ram().share(m_colorram);
	map(0x8800, 0x8bff).ram().share(m_spriteram);
	map(0x9000, 0x93ff).ram().share("main_sound1");
	map(0x9800, 0x9bff).ram().share("main_sound2");
	map(0xa000, 0xa007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xa800, 0xa807).r(FUNC(wiping_state::ports_r));
	map(0xb000, 0xb7ff).ram();
	map(0xb800, 0xb800).w("watchdog", FUNC(watchdog_timer_device::reset_w));
}

void wiping_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x7fff).w("wiping", FUNC(wiping_sound_device::sound_w));
	map(0x9000, 0x93ff).ram().share("main_sound1");
	map(0x9800, 0x9bff).ram().share("main_sound2");
	map(0xa000, 0xa007).w("mainlatch", FUNC(ls259_device::write_d0));
}


static INPUT_PORTS_START( wiping )
	PORT_START("P1")    // 0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")    // 1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")   // 2
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")   // 3
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN4")   // 4
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN5")   // 5
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")    // 6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x05, IP_ACTIVE_LOW, IPT_COIN2 )  // note that this changes two bits
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000 70000" )
	PORT_DIPSETTING(    0x80, "50000 150000" )

	PORT_START("DSW")   // 7
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coin_B )  )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )
//  PORT_DIPSETTING(    0x00, "Disable" )
	PORT_DIPNAME( 0x38, 0x08, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0xc0, "5" )
INPUT_PORTS_END

// identical apart from bonus life
static INPUT_PORTS_START( rugrats )
	PORT_INCLUDE( wiping )

	PORT_MODIFY("SYSTEM") // 6
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "100000 200000" )
	PORT_DIPSETTING(    0x80, "150000 300000" )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	256,    // 256 characters
	2,  // 2 bits per pixel
	{ 0, 4 },   // the two bitplanes are packed in one byte
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8    // every char takes 16 consecutive bytes
};

static const gfx_layout spritelayout =
{
	16,16,  // 16*16 sprites
	128,    // 128 sprites
	2,  // 2 bits per pixel
	{ 0, 4 },   // the two bitplanes are packed in one byte
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 17*8+0, 17*8+1, 17*8+2, 17*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	64*8    // every sprite takes 64 consecutive bytes
};

static GFXDECODE_START( gfx_wiping )
	GFXDECODE_ENTRY( "chars",   0, charlayout,      0, 64 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 64*4, 64 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(wiping_state::vblank_irq)
{
	if (m_main_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(wiping_state::sound_timer_irq)
{
	if (m_sound_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}



void wiping_state::wiping(machine_config &config)
{
	static constexpr XTAL MASTER_CLOCK = XTAL(18'432'000);

	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK / 6); // 3.072 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &wiping_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(wiping_state::vblank_irq));

	Z80(config, m_audiocpu, MASTER_CLOCK / 6); // 3.072 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &wiping_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(wiping_state::sound_timer_irq), attotime::from_hz(120));    // periodic interrupt, don't know about the frequency

	config.set_maximum_quantum(attotime::from_hz(MASTER_CLOCK / 6 / 512)); // 6000 Hz

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 5A
	mainlatch.q_out_cb<0>().set(FUNC(wiping_state::main_irq_mask_w)); // INT1
	mainlatch.q_out_cb<1>().set(FUNC(wiping_state::sound_irq_mask_w)); // INT2
	mainlatch.q_out_cb<2>().set(FUNC(wiping_state::flipscreen_w)); // INV
	mainlatch.q_out_cb<3>().set_inputline(m_audiocpu, INPUT_LINE_RESET).invert(); // CP2RE

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_CLOCK / 3, 384, 0, 288, 264, 0, 224); // unknown, single XTAL on PCB & 288x224 suggests 60.606060 Hz like Galaxian HW
	screen.set_screen_update(FUNC(wiping_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wiping);
	PALETTE(config, m_palette, FUNC(wiping_state::palette), 64 * 4 + 64 * 4, 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	WIPING_CUSTOM(config, "wiping", 96'000 / 2).add_route(ALL_OUTPUTS, "mono", 1.0); // 48000 Hz?
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wiping )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",            0x0000, 0x2000, CRC(b55d0d19) SHA1(dac6096d3ee9dd8b1b6da5c2c613b54ce303cb7b) )
	ROM_LOAD( "2",            0x2000, 0x2000, CRC(b1f96e47) SHA1(8f3f882a3c366e6a2d2682603d425eb0491b5487) )
	ROM_LOAD( "3",            0x4000, 0x2000, CRC(c67bab5a) SHA1(3d74ed4be5a6bdc02cf1feb3ce3f4b1607ec6b80) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "4",            0x0000, 0x1000, CRC(a1547e18) SHA1(1f86d770e42ff1d94bf1f8b12f9b74accc3bb193) )

	ROM_REGION( 0x1000, "chars", 0 )
	ROM_LOAD( "8",            0x0000, 0x1000, CRC(601160f6) SHA1(2465a1319d442a96d3b1b5e3ad544b0a0126762c) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "7",            0x0000, 0x2000, CRC(2c2cc054) SHA1(31851983de61bb8616856b0067c4e237819df5fb) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "wip-g13.bin",  0x0000, 0x0020, CRC(b858b897) SHA1(5fc87e210bdaa675fdf8c6762526c345bd451eab) )    // palette
	ROM_LOAD( "wip-f4.bin",   0x0020, 0x0100, CRC(3f56c8d5) SHA1(7d279b2f29911c44b4136068770accf7196057d7) )    // char lookup table
	ROM_LOAD( "wip-e11.bin",  0x0120, 0x0100, CRC(e7400715) SHA1(c67193e5f0a43942ddf03058a0bb8b3275308459) )    // sprite lookup table

	ROM_REGION( 0x4000, "wiping:samples", 0 )
	ROM_LOAD( "rugr5c8",      0x0000, 0x2000, CRC(67bafbbf) SHA1(2085492b58ce44f61a42320c54595b79fdf7a91c) )
	ROM_LOAD( "rugr6c9",      0x2000, 0x2000, CRC(cac84a87) SHA1(90f6c514d0cdbeb4c8c979597db79ebcdf443df4) )

	ROM_REGION( 0x0200, "wiping:soundproms", 0 )   // 4bit -> 8bit sample expansion PROMs
	ROM_LOAD( "wip-e8.bin",   0x0000, 0x0100, CRC(bd2c080b) SHA1(9782bb5001e96db56bc29df398187f700bce4f8e) )    // low 4 bits
	ROM_LOAD( "wip-e9.bin",   0x0100, 0x0100, CRC(4017a2a6) SHA1(dadef2de7a1119758c8e6d397aa42815b0218889) )    // high 4 bits
ROM_END

ROM_START( rugrats )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.1d",      0x0000, 0x2000, CRC(e7e1bd6d) SHA1(985799b1bfd001c6304e6166180745cb019f834e) )
	ROM_LOAD( "2.2d",      0x2000, 0x2000, CRC(5f47b9ad) SHA1(2d3eb737ea8e86691293e432e866d2623d6b6b1b) )
	ROM_LOAD( "3.3d",      0x4000, 0x2000, CRC(3d748d1a) SHA1(2b301119b6eb3f0f9bb2ad734cff1d25365dfe99) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "4.3b",      0x0000, 0x2000, CRC(d4a92c38) SHA1(4a31cfef9f084b4d2934595155bf0f3dd589efb3) )

	ROM_REGION( 0x1000, "chars", 0 )
	ROM_LOAD( "8.2d",      0x0000, 0x1000, CRC(a3dcaca5) SHA1(d71f9090bf95dfd035ee0e0619a1cce575033cf3) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "7.13c",     0x0000, 0x2000, CRC(fe1191dd) SHA1(80ebf093f7a32f4cc9dc89dcc44cab6e3db4fca1) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "g13.13g",   0x0000, 0x0020, CRC(f21238f0) SHA1(944627d1551453c7f828d96b83fd4eeb038b20ad) )    // palette
	ROM_LOAD( "eiif4.4f",  0x0020, 0x0100, CRC(cfc90f3d) SHA1(99f7dc0d14c62d4c676c96310c219c696c9a7897) )    // char lookup table
	ROM_LOAD( "eiif4.11e", 0x0120, 0x0100, CRC(cfc90f3d) SHA1(99f7dc0d14c62d4c676c96310c219c696c9a7897) )    // sprite lookup table

	ROM_REGION( 0x4000, "wiping:samples", 0 )
	ROM_LOAD( "5.8c",      0x0000, 0x2000, CRC(67bafbbf) SHA1(2085492b58ce44f61a42320c54595b79fdf7a91c) )
	ROM_LOAD( "6.9c",      0x2000, 0x2000, CRC(cac84a87) SHA1(90f6c514d0cdbeb4c8c979597db79ebcdf443df4) )

	ROM_REGION( 0x0200, "wiping:soundproms", 0 )   // 4bit -> 8bit sample expansion PROMs
	ROM_LOAD( "e8.8e",     0x0000, 0x0100, CRC(bd2c080b) SHA1(9782bb5001e96db56bc29df398187f700bce4f8e) )    // low 4 bits
	ROM_LOAD( "e9.9e",     0x0100, 0x0100, CRC(4017a2a6) SHA1(dadef2de7a1119758c8e6d397aa42815b0218889) )    // high 4 bits
ROM_END

} // anonymous namespace


GAME( 1982, wiping,  0,      wiping, wiping,  wiping_state, empty_init, ROT90, "Nichibutsu", "Wiping",   MACHINE_SUPPORTS_SAVE )
GAME( 1983, rugrats, wiping, wiping, rugrats, wiping_state, empty_init, ROT90, "Nichibutsu", "Rug Rats", MACHINE_SUPPORTS_SAVE )
