// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Sprint 4 driver

***************************************************************************/

#include "emu.h"
#include "sprint4_a.h"

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

#define MASTER_CLOCK    12096000

#define HTOTAL 384
#define VTOTAL 262

#define PIXEL_CLOCK    (MASTER_CLOCK / 2)

class sprint4_state : public driver_device
{
public:
	sprint4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_io_wheel(*this, "WHEEL%u", 1U),
		m_io_lever(*this, "LEVER%u", 1U),
		m_io_in0(*this, "IN0"),
		m_io_analog(*this, "ANALOG"),
		m_io_coin(*this, "COIN"),
		m_io_collision(*this, "COLLISION"),
		m_io_dip(*this, "DIP")
	{ }

	void sprint4(machine_config &config);

	template <int N> int lever_r();
	template <int N> int wheel_r();
	template <int N> int collision_flipflop_r();

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void cpu_map(address_map &map) ATTR_COLD;

	uint8_t wram_r(offs_t offset);
	uint8_t analog_r(offs_t offset);
	uint8_t coin_r(offs_t offset);
	uint8_t collision_r(offs_t offset);
	uint8_t options_r(offs_t offset);
	void wram_w(offs_t offset, uint8_t data);
	void collision_reset_w(offs_t offset, uint8_t data);
	void da_latch_w(uint8_t data);
	[[maybe_unused]] void lockout_w(offs_t offset, uint8_t data);
	void video_ram_w(offs_t offset, uint8_t data);
	void bang_w(uint8_t data);
	void attract_w(uint8_t data);

	void palette_init(palette_device &palette) const;

	TILE_GET_INFO_MEMBER(tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	TIMER_CALLBACK_MEMBER(nmi_callback);

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<discrete_sound_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;

	required_ioport_array<4> m_io_wheel;
	required_ioport_array<4> m_io_lever;
	required_ioport m_io_in0;
	required_ioport m_io_analog;
	required_ioport m_io_coin;
	required_ioport m_io_collision;
	required_ioport m_io_dip;

	int m_da_latch;
	int m_steer_FF1[4];
	int m_steer_FF2[4];
	int m_gear[4];
	uint8_t m_last_wheel[4];
	int m_collision[4];
	tilemap_t* m_playfield;
	bitmap_ind16 m_helper;
	emu_timer *m_nmi_timer;
};


template <int N>
int sprint4_state::lever_r()
{
	return 4 * m_gear[N] > m_da_latch;
}


template <int N>
int sprint4_state::wheel_r()
{
	return 8 * m_steer_FF1[N] + 8 * m_steer_FF2[N] > m_da_latch;
}


template <int N>
int sprint4_state::collision_flipflop_r()
{
	return m_collision[N];
}


TIMER_CALLBACK_MEMBER(sprint4_state::nmi_callback)
{
	int scanline = param;

	/* MAME updates controls only once per frame but the game checks them on every NMI */

	/* emulation of steering wheels isn't very accurate */

	for (int i = 0; i < 4; i++)
	{
		uint8_t wheel = uint8_t(m_io_wheel[i]->read());
		signed char delta = wheel - m_last_wheel[i];

		if (delta < 0)
		{
			m_steer_FF2[i] = 0;
		}
		if (delta > 0)
		{
			m_steer_FF2[i] = 1;
		}

		m_steer_FF1[i] = (wheel >> 4) & 1;

		uint8_t lever = uint8_t(m_io_lever[i]->read());
		if (lever & 1) m_gear[i] = 1;
		if (lever & 2) m_gear[i] = 2;
		if (lever & 4) m_gear[i] = 3;
		if (lever & 8) m_gear[i] = 4;

		m_last_wheel[i] = wheel;
	}

	scanline += 64;

	if (scanline >= VTOTAL)
	{
		scanline = 32;
	}

	/* NMI and watchdog are disabled during service mode */

	m_watchdog->watchdog_enable(m_io_in0->read() & 0x40);

	if (m_io_in0->read() & 0x40)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);

	m_nmi_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


void sprint4_state::machine_start()
{
	m_nmi_timer = timer_alloc(FUNC(sprint4_state::nmi_callback), this);

	save_item(NAME(m_da_latch));
	save_item(NAME(m_steer_FF1));
	save_item(NAME(m_steer_FF2));
	save_item(NAME(m_gear));
	save_item(NAME(m_last_wheel));
	save_item(NAME(m_collision));
}


void sprint4_state::machine_reset()
{
	m_nmi_timer->adjust(m_screen->time_until_pos(32), 32);

	memset(m_steer_FF1, 0, sizeof m_steer_FF1);
	memset(m_steer_FF2, 0, sizeof m_steer_FF2);

	m_gear[0] = 1;
	m_gear[1] = 1;
	m_gear[2] = 1;
	m_gear[3] = 1;

	m_da_latch = 0;
}


uint8_t sprint4_state::wram_r(offs_t offset)
{
	return m_videoram[0x380 + offset];
}


uint8_t sprint4_state::analog_r(offs_t offset)
{
	return (m_io_analog->read() << (~offset & 7)) & 0x80;
}
uint8_t sprint4_state::coin_r(offs_t offset)
{
	return (m_io_coin->read() << (~offset & 7)) & 0x80;
}
uint8_t sprint4_state::collision_r(offs_t offset)
{
	return (m_io_collision->read() << (~offset & 7)) & 0x80;
}


uint8_t sprint4_state::options_r(offs_t offset)
{
	return (m_io_dip->read() >> (2 * (offset & 3))) & 3;
}


void sprint4_state::wram_w(offs_t offset, uint8_t data)
{
	m_videoram[0x380 + offset] = data;
}


void sprint4_state::collision_reset_w(offs_t offset, uint8_t data)
{
	m_collision[(offset >> 1) & 3] = 0;
}


void sprint4_state::da_latch_w(uint8_t data)
{
	m_da_latch = data & 15;
}



void sprint4_state::lockout_w(offs_t offset, uint8_t data)
{
	machine().bookkeeping().coin_lockout_global_w(~offset & 1);
}


void sprint4_state::bang_w(uint8_t data)
{
	m_discrete->write(SPRINT4_BANG_DATA, data & 0x0f);
}


void sprint4_state::attract_w(uint8_t data)
{
	m_discrete->write(SPRINT4_ATTRACT_EN, data & 1);
}


void sprint4_state::palette_init(palette_device &palette) const
{
	palette.set_indirect_color(0, rgb_t(0x00, 0x00, 0x00)); // black
	palette.set_indirect_color(1, rgb_t(0xfc, 0xdf, 0x80)); // peach
	palette.set_indirect_color(2, rgb_t(0xf0, 0x00, 0xf0)); // violet
	palette.set_indirect_color(3, rgb_t(0x00, 0xf0, 0x0f)); // green
	palette.set_indirect_color(4, rgb_t(0x30, 0x4f, 0xff)); // blue
	palette.set_indirect_color(5, rgb_t(0xff, 0xff, 0xff)); // white

	palette.set_pen_indirect(0, 0);
	palette.set_pen_indirect(2, 0);
	palette.set_pen_indirect(4, 0);
	palette.set_pen_indirect(6, 0);
	palette.set_pen_indirect(8, 0);

	palette.set_pen_indirect(1, 1);
	palette.set_pen_indirect(3, 2);
	palette.set_pen_indirect(5, 3);
	palette.set_pen_indirect(7, 4);
	palette.set_pen_indirect(9, 5);
}


TILE_GET_INFO_MEMBER(sprint4_state::tile_info)
{
	uint8_t code = m_videoram[tile_index];

	if ((code & 0x30) == 0x30)
		tileinfo.set(0, code & ~0x40, (code >> 6) ^ 3, 0);
	else
		tileinfo.set(0, code, 4, 0);
}


void sprint4_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper);

	m_playfield = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sprint4_state::tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


uint32_t sprint4_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_playfield->draw(screen, bitmap, cliprect, 0, 0);

	for (int i = 0; i < 4; i++)
	{
		int bank = 0;

		uint8_t horz = m_videoram[0x390 + 2 * i + 0];
		uint8_t attr = m_videoram[0x390 + 2 * i + 1];
		uint8_t vert = m_videoram[0x398 + 2 * i + 0];
		uint8_t code = m_videoram[0x398 + 2 * i + 1];

		if (i & 1)
			bank = 32;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			(code >> 3) | bank,
			(attr & 0x80) ? 4 : i,
			0, 0,
			horz - 15,
			vert - 15, 0);
	}
	return 0;
}


void sprint4_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		/* check for sprite-playfield collisions */

		for (int i = 0; i < 4; i++)
		{
			int bank = 0;

			uint8_t horz = m_videoram[0x390 + 2 * i + 0];
			uint8_t vert = m_videoram[0x398 + 2 * i + 0];
			uint8_t code = m_videoram[0x398 + 2 * i + 1];

			rectangle rect(
					horz - 15,
					horz - 15 + m_gfxdecode->gfx(1)->width() - 1,
					vert - 15,
					vert - 15 + m_gfxdecode->gfx(1)->height() - 1);
			rect &= m_screen->visible_area();

			m_playfield->draw(*m_screen, m_helper, rect, 0, 0);

			if (i & 1)
				bank = 32;

			m_gfxdecode->gfx(1)->transpen(m_helper,rect,
				(code >> 3) | bank,
				4,
				0, 0,
				horz - 15,
				vert - 15, 1);

			for (int y = rect.top(); y <= rect.bottom(); y++)
				for (int x = rect.left(); x <= rect.right(); x++)
					if (m_palette->pen_indirect(m_helper.pix(y, x)) != 0)
						m_collision[i] = 1;
		}

		/* update sound status */

		m_discrete->write(SPRINT4_MOTOR_DATA_1, m_videoram[0x391] & 15);
		m_discrete->write(SPRINT4_MOTOR_DATA_2, m_videoram[0x393] & 15);
		m_discrete->write(SPRINT4_MOTOR_DATA_3, m_videoram[0x395] & 15);
		m_discrete->write(SPRINT4_MOTOR_DATA_4, m_videoram[0x397] & 15);
	}
}


void sprint4_state::video_ram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_playfield->mark_tile_dirty(offset);
}


void sprint4_state::cpu_map(address_map &map)
{

	map.global_mask(0x3fff);

	map(0x0080, 0x00ff).mirror(0x700).rw(FUNC(sprint4_state::wram_r), FUNC(sprint4_state::wram_w));
	map(0x0800, 0x0bff).mirror(0x400).ram().w(FUNC(sprint4_state::video_ram_w)).share("videoram");

	map(0x0000, 0x0007).mirror(0x718).r(FUNC(sprint4_state::analog_r));
	map(0x0020, 0x0027).mirror(0x718).r(FUNC(sprint4_state::coin_r));
	map(0x0040, 0x0047).mirror(0x718).r(FUNC(sprint4_state::collision_r));
	map(0x0060, 0x0063).mirror(0x71c).r(FUNC(sprint4_state::options_r));

	map(0x1000, 0x17ff).portr("IN0");
	map(0x1800, 0x1fff).portr("IN1");

	map(0x0000, 0x0000).mirror(0x71f).w(FUNC(sprint4_state::attract_w));
	map(0x0020, 0x0027).mirror(0x718).w(FUNC(sprint4_state::collision_reset_w));
	map(0x0040, 0x0041).mirror(0x718).w(FUNC(sprint4_state::da_latch_w));
	map(0x0042, 0x0043).mirror(0x718).w(FUNC(sprint4_state::bang_w));
	map(0x0044, 0x0045).mirror(0x718).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x0060, 0x006f).mirror(0x710).w("latch", FUNC(f9334_device::write_a0));

	map(0x2000, 0x27ff).noprw(); /* diagnostic ROM */
	map(0x2800, 0x3fff).rom();

}


static INPUT_PORTS_START( sprint4 )

	PORT_START("IN0")
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Track Select") PORT_CODE(KEYCODE_SPACE)

	PORT_START("COLLISION")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 1 Gas") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(sprint4_state::collision_flipflop_r<0>))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 2 Gas") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(sprint4_state::collision_flipflop_r<1>))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 3 Gas") PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(sprint4_state::collision_flipflop_r<2>))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 4 Gas") PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(sprint4_state::collision_flipflop_r<3>))

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START("DIP")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Language ) ) PORT_DIPLOCATION("DIP:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( German ) )
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x03, DEF_STR( English ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DIP:6")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x08, 0x08, "Allow Late Entry" ) PORT_DIPLOCATION("DIP:5")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xf0, 0xb0, "Play Time" ) PORT_DIPLOCATION("DIP:4,3,2,1")
	PORT_DIPSETTING(    0x70, "60 seconds" )
	PORT_DIPSETTING(    0xb0, "90 seconds" )
	PORT_DIPSETTING(    0xd0, "120 seconds" )
	PORT_DIPSETTING(    0xe0, "150 seconds" )

	PORT_START("ANALOG")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(sprint4_state::wheel_r<0>))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(sprint4_state::lever_r<0>))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(sprint4_state::wheel_r<1>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(sprint4_state::lever_r<1>))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(sprint4_state::wheel_r<2>))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(sprint4_state::lever_r<2>))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(sprint4_state::wheel_r<3>))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(sprint4_state::lever_r<3>))

	PORT_START("WHEEL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_PLAYER(1)

	PORT_START("WHEEL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_PLAYER(2)

	PORT_START("WHEEL3")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_PLAYER(3)

	PORT_START("WHEEL4")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_PLAYER(4)

	PORT_START("LEVER1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 1 Gear 1") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 1 Gear 2") PORT_CODE(KEYCODE_X) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 1 Gear 3") PORT_CODE(KEYCODE_C) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 1 Gear 4") PORT_CODE(KEYCODE_V) PORT_PLAYER(1)

	PORT_START("LEVER2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 2 Gear 1") PORT_CODE(KEYCODE_Q) PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 2 Gear 2") PORT_CODE(KEYCODE_W) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 2 Gear 3") PORT_CODE(KEYCODE_E) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 2 Gear 4") PORT_CODE(KEYCODE_R) PORT_PLAYER(2)

	PORT_START("LEVER3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 3 Gear 1") PORT_CODE(KEYCODE_Y) PORT_PLAYER(3)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 3 Gear 2") PORT_CODE(KEYCODE_U) PORT_PLAYER(3)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 3 Gear 3") PORT_CODE(KEYCODE_I) PORT_PLAYER(3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 3 Gear 4") PORT_CODE(KEYCODE_O) PORT_PLAYER(3)

	PORT_START("LEVER4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 4 Gear 1") PORT_PLAYER(4)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 4 Gear 2") PORT_PLAYER(4)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 4 Gear 3") PORT_PLAYER(4)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 4 Gear 4") PORT_PLAYER(4)

	PORT_START("MOTOR1")
	PORT_ADJUSTER( 35, "Motor 1 RPM" )

	PORT_START("MOTOR2")
	PORT_ADJUSTER( 40, "Motor 2 RPM" )

	PORT_START("MOTOR3")
	PORT_ADJUSTER( 35, "Motor 3 RPM" )

	PORT_START("MOTOR4")
	PORT_ADJUSTER( 40, "Motor 4 RPM" )

INPUT_PORTS_END


static const gfx_layout car_layout =
{
	12, 16,
	RGN_FRAC(1,3),
	1,
	{ 0 },
	{
		7 + RGN_FRAC(0,3), 6 + RGN_FRAC(0,3), 5 + RGN_FRAC(0,3), 4 + RGN_FRAC(0,3),
		7 + RGN_FRAC(1,3), 6 + RGN_FRAC(1,3), 5 + RGN_FRAC(1,3), 4 + RGN_FRAC(1,3),
		7 + RGN_FRAC(2,3), 6 + RGN_FRAC(2,3), 5 + RGN_FRAC(2,3), 4 + RGN_FRAC(2,3)
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
		0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
	},
	0x80
};


static GFXDECODE_START( gfx_sprint4 )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x1, 0, 5 )
	GFXDECODE_ENTRY( "gfx2", 0, car_layout, 0, 5 )
GFXDECODE_END


void sprint4_state::sprint4(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, PIXEL_CLOCK / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &sprint4_state::cpu_map);

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count(m_screen, 8);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, 0, 256, VTOTAL, 0, 224);
	m_screen->set_screen_update(FUNC(sprint4_state::screen_update));
	m_screen->screen_vblank().set(FUNC(sprint4_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sprint4);
	PALETTE(config, m_palette, FUNC(sprint4_state::palette_init), 10, 6);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	f9334_device &latch(F9334(config, "latch")); // at E11
	latch.q_out_cb<0>().set_output("led0"); // START LAMP 1
	latch.q_out_cb<1>().set_output("led1"); // START LAMP 2
	latch.q_out_cb<2>().set_output("led2"); // START LAMP 3
	latch.q_out_cb<3>().set_output("led3"); // START LAMP 4
	latch.q_out_cb<4>().set("discrete", FUNC(discrete_device::write_line<SPRINT4_SCREECH_EN_1>));
	latch.q_out_cb<5>().set("discrete", FUNC(discrete_device::write_line<SPRINT4_SCREECH_EN_2>));
	latch.q_out_cb<6>().set("discrete", FUNC(discrete_device::write_line<SPRINT4_SCREECH_EN_3>));
	latch.q_out_cb<7>().set("discrete", FUNC(discrete_device::write_line<SPRINT4_SCREECH_EN_4>));

	DISCRETE(config, m_discrete, sprint4_discrete);
	m_discrete->add_route(0, "speaker", 1.0, 0);
	m_discrete->add_route(1, "speaker", 1.0, 1);
}

 // NOTE:  SPRINT 4 A008716 PCB can accept both 8bit ROMs and 4bit BPROMs or combination thereof

ROM_START( sprint4 ) // 4bit BPROM version of Rev 03
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "30034-03.p1", 0x2800, 0x0800, CRC(a55aef81) SHA1(f7f28deec098b271247881e96cb35dc82d7ab98d) ) // These are all N82S185 BPROMs
	ROM_LOAD_NIB_HIGH( "30035-03.j1", 0x2800, 0x0800, CRC(90c1abee) SHA1(3fb67ffcd7b217d1374241e07e8a8ce85e61b4b6) )
	ROM_LOAD_NIB_LOW ( "30036-03.n1", 0x3000, 0x0800, CRC(55a0749f) SHA1(f3c535a916c3e20de3e51c3c1dc848b103d2a630) )
	ROM_LOAD_NIB_HIGH( "30037-03.k1", 0x3000, 0x0800, CRC(7730ec67) SHA1(83ab0ee96ccad122d23551f5dd1d9266d8418366) )
	ROM_LOAD_NIB_LOW ( "30038-03.m1", 0x3800, 0x0800, CRC(142a4603) SHA1(e3b61734f3f19aaa1d674823c59018e9ad8f3bed) )
	ROM_LOAD_NIB_HIGH( "30039-03.l1", 0x3800, 0x0800, CRC(d3ffa2fc) SHA1(a7cab3e996ce3da3bb449bbe065761b169bc25ec) )

	ROM_REGION( 0x0800, "gfx1", 0 ) // playfield
	ROM_LOAD_NIB_LOW ( "30025-01.j6", 0x0000, 0x0800, CRC(748ca960) SHA1(45752e788e34b61d3079cce040df2603886a4bc3) ) // These are N82S185 BPROMs
	ROM_LOAD_NIB_HIGH( "30026-01.h6", 0x0000, 0x0800, CRC(a50253d4) SHA1(edd76c105c6577b07e4f4f9e9637a25b20c44eaa) )

	ROM_REGION( 0x0c00, "gfx2", 0 ) // motion
	ROM_LOAD( "30028-03.n6", 0x0000, 0x0400, CRC(d3337030) SHA1(3e73fc45bdcaa52dc1aa01489b46240284562ab7) ) // These are DM74S573N BPROMs
	ROM_LOAD( "30029-03.m6", 0x0400, 0x0400, CRC(77e04ba4) SHA1(f13d132eb23b59119748a893ae352bd435da7bfb) )
	ROM_LOAD( "30030-03.l6", 0x0800, 0x0400, CRC(aa1b45ab) SHA1(1ddb64d4ec92a1383866daaefa556499837decd1) )

	ROM_REGION( 0x0200, "user1", 0 ) // SYNC
	ROM_LOAD( "30024-01.p8", 0x0000, 0x0200, CRC(e71d2e22) SHA1(434c3a8237468604cce7feb40e6061d2670013b3) ) // MMI6306-1 BPROM
ROM_END


ROM_START( sprint4a ) // Mixed, 8bit ROM & 4bit BPROM version of Rev 02
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD         ( "30031-01.c1", 0x2800, 0x0800, CRC(017ee7c4) SHA1(9386cacc619669c18af31f66691a45af6dafef64) ) // == 030034-03.p1(low)+030035-03.j1(high)
	ROM_LOAD_NIB_LOW ( "30036-02.n1", 0x3000, 0x0800, CRC(883b9d7c) SHA1(af52ffdd9cd8dfed54013c9b0d3c6e48c7419d17) )
	ROM_LOAD_NIB_HIGH( "30037-02.k1", 0x3000, 0x0800, CRC(c297fbd8) SHA1(8cc0f486429e12bee21a5dd1135e799196480044) )
	ROM_LOAD         ( "30033-01.e1", 0x3800, 0x0800, CRC(b8b717b7) SHA1(2f6b1a0e9803901d9ba79d1f19a025f6a6134756) )

	ROM_REGION( 0x0800, "gfx1", 0 ) // playfield
	ROM_LOAD( "30027-01.h5", 0x0000, 0x0800, CRC(3a752e07) SHA1(d990f94b296409d47e8bada98ddbed5f76567e1b) )

	ROM_REGION( 0x0c00, "gfx2", 0 ) // motion
	ROM_LOAD( "30028-03.n6", 0x0000, 0x0400, CRC(d3337030) SHA1(3e73fc45bdcaa52dc1aa01489b46240284562ab7) ) // rev 3 graphics ROMs with rev 02 programs?? Operator upgrade??
	ROM_LOAD( "30029-03.m6", 0x0400, 0x0400, CRC(77e04ba4) SHA1(f13d132eb23b59119748a893ae352bd435da7bfb) )
	ROM_LOAD( "30030-03.l6", 0x0800, 0x0400, CRC(aa1b45ab) SHA1(1ddb64d4ec92a1383866daaefa556499837decd1) )

	ROM_REGION( 0x0200, "user1", 0 ) // SYNC
	ROM_LOAD( "30024-01.p8", 0x0000, 0x0200, CRC(e71d2e22) SHA1(434c3a8237468604cce7feb40e6061d2670013b3) ) // MMI6306-1 BPROM
ROM_END


ROM_START( sprint4b ) // Mixed, 8bit ROM & 4bit BPROM version of Rev 02
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD         ( "30031-01.c1", 0x2800, 0x0800, CRC(017ee7c4) SHA1(9386cacc619669c18af31f66691a45af6dafef64) ) // == 030034-03.p1(low)+030035-03.j1(high)
	ROM_LOAD_NIB_LOW ( "30036-02.n1", 0x3000, 0x0800, CRC(883b9d7c) SHA1(af52ffdd9cd8dfed54013c9b0d3c6e48c7419d17) )
	ROM_LOAD_NIB_HIGH( "30037-02.k1", 0x3000, 0x0800, CRC(c297fbd8) SHA1(8cc0f486429e12bee21a5dd1135e799196480044) )
	ROM_LOAD         ( "30033-01.e1", 0x3800, 0x0800, CRC(b8b717b7) SHA1(2f6b1a0e9803901d9ba79d1f19a025f6a6134756) )

	ROM_REGION( 0x0800, "gfx1", 0 ) // playfield
	ROM_LOAD( "30027-01.h5", 0x0000, 0x0800, CRC(3a752e07) SHA1(d990f94b296409d47e8bada98ddbed5f76567e1b) )

	ROM_REGION( 0x0c00, "gfx2", 0 ) // motion
	ROM_LOAD( "30028-01.n6", 0x0000, 0x0400, CRC(3ebcb13f) SHA1(e0b87239081f12f6613d3db6a8cb5b80937df7d7) )
	ROM_LOAD( "30029-01.m6", 0x0400, 0x0400, CRC(963a8424) SHA1(d52a0e73c54154531e825153012687bdb85e479a) )
	ROM_LOAD( "30030-01.l6", 0x0800, 0x0400, CRC(e94dfc2d) SHA1(9c5b1401c4aadda0a3aee76e4f92e73ae1d35cb7) )

	ROM_REGION( 0x0200, "user1", 0 ) // SYNC
	ROM_LOAD( "30024-01.p8", 0x0000, 0x0200, CRC(e71d2e22) SHA1(434c3a8237468604cce7feb40e6061d2670013b3) ) // MMI6306-1 BPROM
ROM_END

/*

ROM_START( sprint4r38 ) // 8bit ROM version of Rev 03
    ROM_REGION( 0x4000, "maincpu", 0 )
    ROM_LOAD ( "30031-03.c1", 0x2800, 0x0800, CRC(017ee7c4) SHA1(9386cacc619669c18af31f66691a45af6dafef64) ) // == 30031-01.c1
    ROM_LOAD ( "30032-03.d1", 0x3000, 0x0800, CRC(55183642) SHA1(ff5b6ab8fc39d96ab9902667ee3bb595f5722c2d) )
    ROM_LOAD ( "30033-03.e1", 0x3800, 0x0800, CRC(58c8cae0) SHA1(4903579ddab753f50da17911a7e842225efaf7f4) )

    ROM_REGION( 0x0800, "gfx1", 0 ) // playfield
    ROM_LOAD( "30027-01.h5", 0x0000, 0x0800, CRC(3a752e07) SHA1(d990f94b296409d47e8bada98ddbed5f76567e1b) )

    ROM_REGION( 0x0c00, "gfx2", 0 ) // motion
    ROM_LOAD( "30028-03.n6", 0x0000, 0x0400, CRC(d3337030) SHA1(3e73fc45bdcaa52dc1aa01489b46240284562ab7) )
    ROM_LOAD( "30029-03.m6", 0x0400, 0x0400, CRC(77e04ba4) SHA1(f13d132eb23b59119748a893ae352bd435da7bfb) )
    ROM_LOAD( "30030-03.l6", 0x0800, 0x0400, CRC(aa1b45ab) SHA1(1ddb64d4ec92a1383866daaefa556499837decd1) )

    ROM_REGION( 0x0200, "user1", 0 ) // SYNC
    ROM_LOAD( "30024-01.p8", 0x0000, 0x0200, CRC(e71d2e22) SHA1(434c3a8237468604cce7feb40e6061d2670013b3) ) // MMI6306-1 BPROM
ROM_END

ROM_START( sprint4r24 ) // 4bit BPROM version of Rev 02
    ROM_REGION( 0x4000, "maincpu", 0 )
    ROM_LOAD_NIB_LOW ( "30034-01.p1", 0x2800, 0x0800, CRC(a55aef81) SHA1(f7f28deec098b271247881e96cb35dc82d7ab98d) ) // == 30034-03.p1
    ROM_LOAD_NIB_HIGH( "30035-01.j1", 0x2800, 0x0800, CRC(90c1abee) SHA1(3fb67ffcd7b217d1374241e07e8a8ce85e61b4b6) ) // == 30035-03.p1
    ROM_LOAD_NIB_LOW ( "30036-02.n1", 0x3000, 0x0800, CRC(883b9d7c) SHA1(af52ffdd9cd8dfed54013c9b0d3c6e48c7419d17) )
    ROM_LOAD_NIB_HIGH( "30037-02.k1", 0x3000, 0x0800, CRC(c297fbd8) SHA1(8cc0f486429e12bee21a5dd1135e799196480044) )
    ROM_LOAD_NIB_LOW ( "30038-01.m1", 0x3800, 0x0800, CRC(ed1d6c3a) SHA1(7ae09eb12c463566736f65be68932dc3d4a9ad2c) )
    ROM_LOAD_NIB_HIGH( "30039-01.l1", 0x3800, 0x0800, CRC(7261cff2) SHA1(17af3555a765eef2c20a676484bb62bf553f8760) )

    ROM_REGION( 0x0800, "gfx1", 0 ) // playfield
    ROM_LOAD_NIB_LOW ( "30025-01.j6", 0x0000, 0x0800, CRC(748ca960) SHA1(45752e788e34b61d3079cce040df2603886a4bc3) )
    ROM_LOAD_NIB_HIGH( "30026-01.h6", 0x0000, 0x0800, CRC(a50253d4) SHA1(edd76c105c6577b07e4f4f9e9637a25b20c44eaa) )

    ROM_REGION( 0x0c00, "gfx2", 0 ) // motion
    ROM_LOAD( "30028-01.n6", 0x0000, 0x0400, CRC(3ebcb13f) SHA1(e0b87239081f12f6613d3db6a8cb5b80937df7d7) )
    ROM_LOAD( "30029-01.m6", 0x0400, 0x0400, CRC(963a8424) SHA1(d52a0e73c54154531e825153012687bdb85e479a) )
    ROM_LOAD( "30030-01.l6", 0x0800, 0x0400, CRC(e94dfc2d) SHA1(9c5b1401c4aadda0a3aee76e4f92e73ae1d35cb7) )

    ROM_REGION( 0x0200, "user1", 0 ) // SYNC
    ROM_LOAD( "30024-01.p8", 0x0000, 0x0200, CRC(e71d2e22) SHA1(434c3a8237468604cce7feb40e6061d2670013b3) ) // MMI6306-1 BPROM
ROM_END

ROM_START( sprint4r28 ) // 8bit ROM version of Rev 02
    ROM_REGION( 0x4000, "maincpu", 0 )
    ROM_LOAD ( "30031-01.c1", 0x2800, 0x0800, CRC(017ee7c4) SHA1(9386cacc619669c18af31f66691a45af6dafef64) )
    ROM_LOAD ( "30032-02.d1", 0x3000, 0x0800, CRC(092d859e) SHA1(4e932e7ad9cec4cb87b5196418083971fb2900d0) )
    ROM_LOAD ( "30033-01.e1", 0x3800, 0x0800, CRC(b8b717b7) SHA1(2f6b1a0e9803901d9ba79d1f19a025f6a6134756) )

    ROM_REGION( 0x0800, "gfx1", 0 ) // playfield
    ROM_LOAD( "30027-01.h5", 0x0000, 0x0800, CRC(3a752e07) SHA1(d990f94b296409d47e8bada98ddbed5f76567e1b) )

    ROM_REGION( 0x0c00, "gfx2", 0 ) // motion
    ROM_LOAD( "30028-01.n6", 0x0000, 0x0400, CRC(3ebcb13f) SHA1(e0b87239081f12f6613d3db6a8cb5b80937df7d7) )
    ROM_LOAD( "30029-01.m6", 0x0400, 0x0400, CRC(963a8424) SHA1(d52a0e73c54154531e825153012687bdb85e479a) )
    ROM_LOAD( "30030-01.l6", 0x0800, 0x0400, CRC(e94dfc2d) SHA1(9c5b1401c4aadda0a3aee76e4f92e73ae1d35cb7) )

    ROM_REGION( 0x0200, "user1", 0 ) // SYNC
    ROM_LOAD( "30024-01.p8", 0x0000, 0x0200, CRC(e71d2e22) SHA1(434c3a8237468604cce7feb40e6061d2670013b3) ) // MMI6306-1 BPROM
ROM_END

ROM_START( sprint4r18 ) // 8bit ROM version of Rev 01 - Currently undumped!
    ROM_REGION( 0x4000, "maincpu", 0 )
    ROM_LOAD ( "30031-01.c1", 0x2800, 0x0800, CRC(017ee7c4) SHA1(9386cacc619669c18af31f66691a45af6dafef64) )
    ROM_LOAD ( "30032-01.d1", 0x3000, 0x0800, NO_DUMP ) // CRC(58ecd1b9) SHA1(0) - Only expected CRC32 value is known
    ROM_LOAD ( "30033-01.e1", 0x3800, 0x0800, CRC(b8b717b7) SHA1(2f6b1a0e9803901d9ba79d1f19a025f6a6134756) )

    ROM_REGION( 0x0800, "gfx1", 0 ) // playfield
    ROM_LOAD( "30027-01.h5", 0x0000, 0x0800, CRC(3a752e07) SHA1(d990f94b296409d47e8bada98ddbed5f76567e1b) )

    ROM_REGION( 0x0c00, "gfx2", 0 ) // motion
    ROM_LOAD( "30028-01.n6", 0x0000, 0x0400, CRC(3ebcb13f) SHA1(e0b87239081f12f6613d3db6a8cb5b80937df7d7) )
    ROM_LOAD( "30029-01.m6", 0x0400, 0x0400, CRC(963a8424) SHA1(d52a0e73c54154531e825153012687bdb85e479a) )
    ROM_LOAD( "30030-01.l6", 0x0800, 0x0400, CRC(e94dfc2d) SHA1(9c5b1401c4aadda0a3aee76e4f92e73ae1d35cb7) )

    ROM_REGION( 0x0200, "user1", 0 ) // SYNC
    ROM_LOAD( "30024-01.p8", 0x0000, 0x0200, CRC(e71d2e22) SHA1(434c3a8237468604cce7feb40e6061d2670013b3) ) // MMI6306-1 BPROM
ROM_END
*/

} // anonymous namespace


GAME( 1977, sprint4,  0,       sprint4,  sprint4, sprint4_state, empty_init, ROT180, "Atari", "Sprint 4 (Rev 03)",        MACHINE_SUPPORTS_SAVE )
GAME( 1977, sprint4a, sprint4, sprint4,  sprint4, sprint4_state, empty_init, ROT180, "Atari", "Sprint 4 (Rev 02, set 1)", MACHINE_SUPPORTS_SAVE ) // Rev 02 program, Rev 03 car graphics
GAME( 1977, sprint4b, sprint4, sprint4,  sprint4, sprint4_state, empty_init, ROT180, "Atari", "Sprint 4 (Rev 02, set 2)", MACHINE_SUPPORTS_SAVE ) // Rev 02 program, Rev 01 car graphics
