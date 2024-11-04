// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Runaway hardware

    Games supported:
        * Qwak (prototype)
        * Runaway (prototype)

    original Qwak driver written by Mike Balfour

****************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "machine/er2055.h"
#include "sound/pokey.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class qwak_state : public driver_device
{
public:
	qwak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_video_ram(*this, "video_ram")
		, m_sprite_ram(*this, "sprite_ram")
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_inputs(*this, { "3000D6", "3000D7" })
		, m_pot(*this, "7000")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{ }

	void qwak(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void qwak_map(address_map &map) ATTR_COLD;

	tilemap_t *m_bg_tilemap;
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_sprite_ram;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

private:
	uint8_t input_r(offs_t offset);
	void irq_ack_w(uint8_t data);
	void paletteram_w(offs_t offset, uint8_t data);
	void video_ram_w(offs_t offset, uint8_t data);
	uint8_t pot_r(offs_t offset);
	virtual TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void get_sprite_info(int n, unsigned &code, int &flipx, int &flipy);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(interrupt_callback);

	emu_timer *m_interrupt_timer;
	required_ioport_array<2> m_inputs;
	required_ioport m_pot;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};


class runaway_state : public qwak_state
{
public:
	runaway_state(const machine_config &mconfig, device_type type, const char *tag)
		: qwak_state(mconfig, type, tag)
		, m_earom(*this, "earom")
	{ }

	void runaway(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void runaway_map(address_map &map) ATTR_COLD;

private:
	uint8_t earom_read();
	void earom_write(offs_t offset, uint8_t data);
	void earom_control_w(uint8_t data);
	void tile_bank_w(int state);
	virtual TILE_GET_INFO_MEMBER(get_tile_info) override;
	virtual void get_sprite_info(int n, unsigned &code, int &flipx, int &flipy) override;

	required_device<er2055_device> m_earom;

	uint8_t m_tile_bank = 0;
};


TIMER_CALLBACK_MEMBER(qwak_state::interrupt_callback)
{
	// assume Centipede-style interrupt timing
	int scanline = param;

	m_maincpu->set_input_line(0, (scanline & 32) ? ASSERT_LINE : CLEAR_LINE);

	scanline += 32;

	if (scanline >= 263)
		scanline = 16;

	m_interrupt_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}

void qwak_state::machine_start()
{
	m_interrupt_timer = timer_alloc(FUNC(runaway_state::interrupt_callback), this);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(qwak_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 30);
}

void runaway_state::machine_start()
{
	qwak_state::machine_start();

	save_item(NAME(m_tile_bank));
}

void qwak_state::machine_reset()
{
	m_interrupt_timer->adjust(m_screen->time_until_pos(16), 16);
}

void runaway_state::machine_reset()
{
	qwak_state::machine_reset();

	earom_control_w(0);
}


uint8_t qwak_state::input_r(offs_t offset)
{
	return
			(BIT(m_inputs[1]->read(), offset) << 7) |
			(BIT(m_inputs[0]->read(), offset) << 6);
}


uint8_t qwak_state::pot_r(offs_t offset)
{
	return BIT(m_pot->read(), offset) << 7;
}


void qwak_state::irq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


uint8_t runaway_state::earom_read()
{
	return m_earom->data();
}

void runaway_state::earom_write(offs_t offset, uint8_t data)
{
	m_earom->set_address(offset & 0x3f);
	m_earom->set_data(data);
}

void runaway_state::earom_control_w(uint8_t data)
{
	// CK = DB0, C1 = /DB2, C2 = DB1, CS1 = DB3, /CS2 = GND
	m_earom->set_control(BIT(data, 3), 1, !BIT(data, 2), BIT(data, 1));
	m_earom->set_clk(BIT(data, 0));
}


void qwak_state::paletteram_w(offs_t offset, uint8_t data)
{
	int const b =
			0x21 * 0 +
			0x47 * BIT(~data, 0) +
			0x97 * BIT(~data, 1);

	int const r =
			0x21 * BIT(~data, 2) +
			0x47 * BIT(~data, 3) +
			0x97 * BIT(~data, 4);

	int const g =
			0x21 * BIT(~data, 5) +
			0x47 * BIT(~data, 6) +
			0x97 * BIT(~data, 7);

	m_palette->set_pen_color(offset, rgb_t(r, g, b));
}



void qwak_state::video_ram_w(offs_t offset, uint8_t data)
{
	m_video_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}



void runaway_state::tile_bank_w(int state)
{
	if (state != m_tile_bank)
	{
		m_tile_bank = state;
		m_bg_tilemap->mark_all_dirty();
	}
}


TILE_GET_INFO_MEMBER(qwak_state::get_tile_info)
{
	uint8_t const code = m_video_ram[tile_index];

	tileinfo.set(
			0, // gfx
			((code & 0x7f) << 1) | ((code & 0x80) >> 7), // code
			0, // color
			0); // flags
}


TILE_GET_INFO_MEMBER(runaway_state::get_tile_info)
{
	uint8_t const code = m_video_ram[tile_index];

	tileinfo.set(
			0, // gfx
			((code & 0x3f) << 1) | ((code & 0x40) >> 6) | (m_tile_bank << 7), // code
			0, // color
			(code & 0x80) ? TILE_FLIPY : 0); // flags
}



void qwak_state::get_sprite_info(int n, unsigned &code, int &flipx, int &flipy)
{
	code = m_sprite_ram[n] & 0x7f;

	flipx = 0;
	flipy = m_sprite_ram[n] & 0x80;

	code |= (m_sprite_ram[n + 0x30] << 2) & 0x1c0;
}

void runaway_state::get_sprite_info(int n, unsigned &code, int &flipx, int &flipy)
{
	code = m_sprite_ram[n] & 0x3f;

	flipx = m_sprite_ram[n] & 0x40;
	flipy = m_sprite_ram[n] & 0x80;

	code |= (m_sprite_ram[n + 0x30] << 2) & 0x1c0;
}

uint32_t qwak_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	for (int i = 0; i < 16; i++)
	{
		unsigned code;
		int flipx, flipy;
		get_sprite_info(i, code, flipx, flipy);

		int const x = m_sprite_ram[i + 0x20];
		int const y = m_sprite_ram[i + 0x10];

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				0,
				flipx, flipy,
				x, 240 - y, 0);

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				0,
				flipx, flipy,
				x - 256, 240 - y, 0);
	}
	return 0;
}


void qwak_state::qwak_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x0400, 0x07bf).ram().w(FUNC(qwak_state::video_ram_w)).share(m_video_ram);
	map(0x07c0, 0x07ff).ram().share(m_sprite_ram);
	map(0x1000, 0x1000).w(FUNC(qwak_state::irq_ack_w));
	map(0x1c00, 0x1c0f).w(FUNC(qwak_state::paletteram_w));
	map(0x2000, 0x2007).w("mainlatch", FUNC(ls259_device::write_d0));

	map(0x3000, 0x3007).r(FUNC(qwak_state::input_r));
	map(0x4000, 0x4000).portr("4000");
	map(0x6000, 0x600f).rw("pokey1", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x7000, 0x700f).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x8000, 0xcfff).rom();
	map(0xf000, 0xffff).rom(); // for the interrupt vectors
}

void runaway_state::runaway_map(address_map &map)
{
	qwak_map(map);

	map(0x1400, 0x143f).w(FUNC(runaway_state::earom_write));
	map(0x1800, 0x1800).w(FUNC(runaway_state::earom_control_w));
	map(0x5000, 0x5000).r(FUNC(runaway_state::earom_read));
}


static INPUT_PORTS_START( qwak )
	PORT_START("3000D7")    /* 3000 D7 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("3000D6")    /* 3000 D6 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("4000")  /* 4000 */
	PORT_DIPNAME( 0x01, 0x00, "DIP 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "DIP 2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "DIP 3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "DIP 4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "DIP 5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIP 6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIP 7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIP 8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_START("6008") /* 6008 not used */
	PORT_START("7000") /* 7000 not used */
INPUT_PORTS_END


static INPUT_PORTS_START( runaway )
	PORT_START("3000D7") /* 3000 D7 */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("3000D6") /* 3000 D6 */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_START1 )    /* also level skip if invincibility is on */
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("4000") /* 4000 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0c, 0x04, "Coin 3 Multiplier" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x10, "Coin 2 Multiplier" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_START("6008") /* 6008 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Bonus_Life ))
	PORT_DIPSETTING(    0x00, "Never" )
	PORT_DIPSETTING(    0x04, "Every 5000" )
	PORT_DIPSETTING(    0x08, "Every 10000" )
	PORT_DIPSETTING(    0x0c, "Every 15000" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ))
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, "Invincibility" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("7000") /* 7000 */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_TOGGLE
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x20, DEF_STR( German ) )
	PORT_DIPSETTING(    0x40, DEF_STR( French ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Spanish ) )
	PORT_BIT ( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout runaway_tile_layout =
{
	8, 8,
	256,
	3,
	{
		RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3)
	},
	{
		0, 1, 2, 3, 4, 5, 6, 7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
	},
	0x40
};


static const gfx_layout qwak_tile_layout =
{
	8, 8,
	256,
	4,
	{
		RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4)
	},
	{
		0, 1, 2, 3, 4, 5, 6, 7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
	},
	0x40
};


static const gfx_layout runaway_sprite_layout =
{
	8, 16,
	384,
	3,
	{
		RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3)
	},
	{
		0, 1, 2, 3, 4, 5, 6, 7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
		0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
	},
	0x80
};


static const gfx_layout qwak_sprite_layout =
{
	8, 16,
	128,
	4,
	{
		RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4)
	},
	{
		0, 1, 2, 3, 4, 5, 6, 7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
		0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
	},
	0x80
};


static GFXDECODE_START( gfx_runaway )
	GFXDECODE_ENTRY( "gfx1", 0x000, runaway_tile_layout,   0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x800, runaway_sprite_layout, 8, 1 )
GFXDECODE_END


static GFXDECODE_START( gfx_qwak )
	GFXDECODE_ENTRY( "gfx1", 0x800, qwak_tile_layout,   0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x000, qwak_sprite_layout, 0, 1 )
GFXDECODE_END


void qwak_state::qwak(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 12096000 / 8); /* ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &qwak_state::qwak_map);

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set_nop(); // coin counter?
	mainlatch.q_out_cb<1>().set_nop(); // coin counter?
	mainlatch.q_out_cb<3>().set_output("led0").invert();
	mainlatch.q_out_cb<4>().set_output("led1").invert();
	mainlatch.q_out_cb<5>().set_nop();

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(256, 263);
	m_screen->set_visarea(0, 255, 0, 239);
	m_screen->set_screen_update(FUNC(qwak_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_qwak);
	PALETTE(config, m_palette).set_entries(16);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	pokey_device &pokey1(POKEY(config, "pokey1", 12096000 / 8));
	pokey1.allpot_r().set_ioport("6008");
	pokey1.add_route(ALL_OUTPUTS, "mono", 0.50);

	pokey_device &pokey2(POKEY(config, "pokey2", 12096000 / 8));
	pokey2.pot_r<0>().set(FUNC(qwak_state::pot_r));
	pokey2.pot_r<1>().set(FUNC(qwak_state::pot_r));
	pokey2.pot_r<2>().set(FUNC(qwak_state::pot_r));
	pokey2.pot_r<3>().set(FUNC(qwak_state::pot_r));
	pokey2.pot_r<4>().set(FUNC(qwak_state::pot_r));
	pokey2.pot_r<5>().set(FUNC(qwak_state::pot_r));
	pokey2.pot_r<6>().set(FUNC(qwak_state::pot_r));
	pokey2.pot_r<7>().set(FUNC(qwak_state::pot_r));
	pokey2.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void runaway_state::runaway(machine_config &config)
{
	qwak(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &runaway_state::runaway_map);

	subdevice<ls259_device>("mainlatch")->q_out_cb<5>().set(FUNC(runaway_state::tile_bank_w));

	ER2055(config, m_earom);

	// video hardware
	m_gfxdecode->set_info(gfx_runaway);
}

ROM_START( runaway )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "choo8000.d1", 0x8000, 0x1000, CRC(35794abe) SHA1(5ce872bda8bb2ed4888ba8b47ecd1afbe24b22eb) )
	ROM_LOAD( "choo9000.d1", 0x9000, 0x1000, CRC(0d63756d) SHA1(2549a57ca106635f5c53ea1b03f5a0d6e901ab47) )
	ROM_LOAD( "chooa000.e1", 0xa000, 0x1000, CRC(e6806b6b) SHA1(c260eaa35cbc46f0c0fd4006ec6d04315c3bb851) )
	ROM_LOAD( "choob000.f1", 0xb000, 0x1000, CRC(6aa52bc4) SHA1(5992d441ae8607859abd111c946783036ef6253f) )
	ROM_LOAD( "chooc000",    0xc000, 0x1000, CRC(452ddea2) SHA1(1072de8935aae23eb1ef7b16e308180cd3e91da2) )
	ROM_RELOAD(              0xf000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chop0", 0x0000, 0x2000, CRC(225a8c5e) SHA1(13394320640355d67414e085ad28364814147b63) )
	ROM_LOAD( "chop1", 0x2000, 0x2000, CRC(70389c0f) SHA1(6baf4a17c11e9b27a1e09cce301f931f5099978d) )
	ROM_LOAD( "chop2", 0x4000, 0x2000, CRC(63655f1c) SHA1(c235be3945067c873c03ce8a0c5cfb76984f66ff) )
ROM_END


ROM_START( qwak )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qwak8000.bin", 0x8000, 0x1000, CRC(4d002d8a) SHA1(8621e7ec1ab3cb8d003858227e858354cd79dbf1) )
	ROM_LOAD( "qwak9000.bin", 0x9000, 0x1000, CRC(e0c78fd7) SHA1(f5f397950971d12a7ae47fc64aa8f5751463b8a5) )
	ROM_LOAD( "qwaka000.bin", 0xa000, 0x1000, CRC(e5770fc9) SHA1(c9556e9c2f7b6c37755ac9f10d95027118317b4a) )
	ROM_LOAD( "qwakb000.bin", 0xb000, 0x1000, CRC(90771cc0) SHA1(5715e5bfccb05c51d871b443e42b0950ec23e330) )
	ROM_RELOAD(               0xf000, 0x1000 )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "qwakgfx0.bin", 0x0000, 0x1000, CRC(bed2c067) SHA1(53d909b414042d54fe2e86ae0d6c7a4ded16b87e) )
	ROM_LOAD( "qwakgfx1.bin", 0x1000, 0x1000, CRC(73a31d28) SHA1(bbe076432866398bcd02962dd90eb178e3a38fb1) )
	ROM_LOAD( "qwakgfx2.bin", 0x2000, 0x1000, CRC(07fd9e80) SHA1(83d5f22b8316ac7e88d8ecdb238182a35a6f6362) )
	ROM_LOAD( "qwakgfx3.bin", 0x3000, 0x1000, CRC(e8416f2b) SHA1(171f6539575f2c06b431ab5118e5cbaf740f557d) )
ROM_END

} // anonymous namespace


GAME( 1982, qwak,    0, qwak,    qwak,    qwak_state,    empty_init, ROT270, "Atari", "Qwak (prototype)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, runaway, 0, runaway, runaway, runaway_state, empty_init, ROT0,   "Atari", "Runaway (Atari, prototype)", MACHINE_SUPPORTS_SAVE )
