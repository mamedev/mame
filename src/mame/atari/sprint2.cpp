// license:BSD-3-Clause
// copyright-holders:Mike Balfour

/***************************************************************************

    Atari Sprint 2 hardware

    driver by Mike Balfour

    Games supported:
        * Sprint 1
        * Sprint 2
        * Dominos

    All three games run on the same PCB but with minor modifications (some
    chips removed, some wire-wrap connections added).

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

***************************************************************************/

#include "emu.h"

#include "sprint2_a.h"

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

#define MACHINE_IS_SPRINT1   (m_game == 1)
#define MACHINE_IS_SPRINT2   (m_game == 2)
#define MACHINE_IS_DOMINOS   (m_game == 3)


class sprint2_state : public driver_device
{
public:
	sprint2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_outlatch(*this, "outlatch"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_in(*this, { "INA", "INB" }),
		m_dials(*this, "DIAL_P%u", 1U),
		m_gears(*this, "GEAR_P%u", 1U),
		m_dsw(*this, "DSW"),
		m_gear_sel(*this, "P%ugear", 1U)
	{ }

	void sprint1(machine_config &config);
	void sprint2(machine_config &config);
	void dominos4(machine_config &config);
	void dominos(machine_config &config);

	void init_sprint1();
	void init_sprint2();
	void init_sprint2bl();
	void init_dominos();
	void init_dominos4();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_video_ram;
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<f9334_device> m_outlatch;
	required_device<discrete_sound_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_ioport_array<2> m_in;
	optional_ioport_array<2> m_dials, m_gears;
	required_ioport m_dsw;
	output_finder<2> m_gear_sel;

	uint8_t m_steering[2]{};
	uint8_t m_gear[2]{};
	uint8_t m_game = 0;
	uint8_t m_dial[2]{};
	tilemap_t* m_bg_tilemap = nullptr;
	bitmap_ind16 m_helper;
	uint8_t m_collision[2]{};

	uint8_t wram_r(offs_t offset);
	uint8_t dip_r(offs_t offset);
	uint8_t input_a_r(offs_t offset);
	uint8_t input_b_r(offs_t offset);
	uint8_t sync_r();
	template <uint8_t Which> uint8_t steering_r();
	template <uint8_t Which> void steering_reset_w(uint8_t data);
	void wram_w(offs_t offset, uint8_t data);
	void output_latch_w(offs_t offset, uint8_t data);
	void video_ram_w(offs_t offset, uint8_t data);
	void noise_reset_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	INTERRUPT_GEN_MEMBER(irq);
	uint8_t collision_check(rectangle& rect);
	inline int get_sprite_code(int n);
	inline int get_sprite_x(int n);
	inline int get_sprite_y(int n);
	uint8_t service_mode();

	void main_map(address_map &map) ATTR_COLD;
};


void sprint2_state::palette(palette_device &palette) const
{
	palette.set_indirect_color(0, rgb_t(0x00, 0x00, 0x00));
	palette.set_indirect_color(1, rgb_t(0x5b, 0x5b, 0x5b));
	palette.set_indirect_color(2, rgb_t(0xa4, 0xa4, 0xa4));
	palette.set_indirect_color(3, rgb_t(0xff, 0xff, 0xff));

	palette.set_pen_indirect(0x0, 1);   // black playfield
	palette.set_pen_indirect(0x1, 0);
	palette.set_pen_indirect(0x2, 1);   // white playfield
	palette.set_pen_indirect(0x3, 3);

	palette.set_pen_indirect(0x4, 1);   // car #1
	palette.set_pen_indirect(0x5, 3);
	palette.set_pen_indirect(0x6, 1);   // car #2
	palette.set_pen_indirect(0x7, 0);
	palette.set_pen_indirect(0x8, 1);   // car #3
	palette.set_pen_indirect(0x9, 2);
	palette.set_pen_indirect(0xa, 1);   // car #4
	palette.set_pen_indirect(0xb, 2);
}


TILE_GET_INFO_MEMBER(sprint2_state::get_tile_info)
{
	uint8_t const code = m_video_ram[tile_index];

	tileinfo.set(0, code & 0x3f, code >> 7, 0);
}


void sprint2_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sprint2_state::get_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);
}


void sprint2_state::video_ram_w(offs_t offset, uint8_t data)
{
	m_video_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


uint8_t sprint2_state::collision_check(rectangle& rect)
{
	uint8_t data = 0;

	for (int y = rect.top(); y <= rect.bottom(); y++)
		for (int x = rect.left(); x <= rect.right(); x++)
		{
			uint16_t const a = m_palette->pen_indirect(m_helper.pix(y, x));

			if (a == 0)
				data |= 0x40;

			if (a == 3)
				data |= 0x80;
		}

	return data;
}


inline int sprint2_state::get_sprite_code(int n)
{
	return m_video_ram[0x398 + 2 * n + 1] >> 3;
}
inline int sprint2_state::get_sprite_x(int n)
{
	return 2 * (248 - m_video_ram[0x390 + 1 * n]);
}
inline int sprint2_state::get_sprite_y(int n)
{
	return 1 * (248 - m_video_ram[0x398 + 2 * n]);
}


uint32_t sprint2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(*m_screen, bitmap, cliprect, 0, 0);

	// draw the sprites

	for (int i = 0; i < 4; i++)
	{
		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
			get_sprite_code(i),
			i,
			0, 0,
			get_sprite_x(i),
			get_sprite_y(i), 0);
	}
	return 0;
}


void sprint2_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		const rectangle &visarea = m_screen->visible_area();

		/*
		 * Collisions are detected for both player cars:
		 *
		 * D7 => m_collision w/ white playfield
		 * D6 => m_collision w/ black playfield or another car
		 *
		 */

		for (int i = 0; i < 2; i++)
		{
			rectangle rect(
					get_sprite_x(i),
					get_sprite_x(i) + m_gfxdecode->gfx(1)->width() - 1,
					get_sprite_y(i),
					get_sprite_y(i) + m_gfxdecode->gfx(1)->height() - 1);
			rect &= visarea;

			// check for sprite-tilemap collisions

			m_bg_tilemap->draw(*m_screen, m_helper, rect, 0, 0);

			m_gfxdecode->gfx(1)->transpen(m_helper, rect,
				get_sprite_code(i),
				0,
				0, 0,
				get_sprite_x(i),
				get_sprite_y(i), 1);

			m_collision[i] |= collision_check(rect);

			// check for sprite-sprite collisions

			for (int j = 0; j < 4; j++)
				if (j != i)
				{
					m_gfxdecode->gfx(1)->transpen(m_helper, rect,
						get_sprite_code(j),
						1,
						0, 0,
						get_sprite_x(j),
						get_sprite_y(j), 0);
				}

			m_gfxdecode->gfx(1)->transpen(m_helper, rect,
				get_sprite_code(i),
				0,
				0, 0,
				get_sprite_x(i),
				get_sprite_y(i), 1);

			m_collision[i] |= collision_check(rect);
		}
	}
}


void sprint2_state::machine_start()
{
	m_gear_sel.resolve();

	save_item(NAME(m_steering));
	save_item(NAME(m_gear));
	save_item(NAME(m_dial));
	save_item(NAME(m_collision));
}


void sprint2_state::init_sprint1()
{
	m_game = 1;
}

void sprint2_state::init_sprint2()
{
	m_game = 2;
}

void sprint2_state::init_sprint2bl()
{
	uint8_t *rom = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	std::vector<uint8_t> buffer(size);
	memcpy(&buffer[0], rom, size);

	for (int i = 0; i < size; i++)
		rom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 6, 7, 8, 9, 5, 4, 3, 2, 1, 0)];

	m_game = 2;
}

void sprint2_state::init_dominos()
{
	m_game = 3;
}

void sprint2_state::init_dominos4()
{
	m_game = 3;
	m_maincpu->space(AS_PROGRAM).install_read_port(0x0880, 0x0880, "SELFTTEST");
}

uint8_t sprint2_state::service_mode()
{
	uint8_t const v = m_in[1]->read();

	if (MACHINE_IS_SPRINT1)
	{
		return (v & 0x10) == 0;
	}
	if (MACHINE_IS_SPRINT2)
	{
		return (v & 0x04) == 0;
	}
	if (MACHINE_IS_DOMINOS)
	{
		return (v & 0x40) == 0;
	}

	return 0;
}


INTERRUPT_GEN_MEMBER(sprint2_state::irq)
{
	// handle steering wheels

	if (MACHINE_IS_SPRINT1 || MACHINE_IS_SPRINT2)
	{
		for (int i = 0; i < 2; i++)
		{
			signed char const delta = m_dials[i]->read() - m_dial[i];

			if (delta < 0)
			{
				m_steering[i] = 0x00;
			}
			if (delta > 0)
			{
				m_steering[i] = 0x40;
			}

			m_dial[i] += delta;

			switch (m_gears[i]->read() & 15)
			{
			case 1: m_gear[i] = 1; break;
			case 2: m_gear[i] = 2; break;
			case 4: m_gear[i] = 3; break;
			case 8: m_gear[i] = 4; break;
			}
			m_gear_sel[0] = m_gear[0];
			m_gear_sel[1] = m_gear[1];
		}
	}

	m_discrete->write(SPRINT2_MOTORSND1_DATA, m_video_ram[0x394] & 15); // also DOMINOS_FREQ_DATA
	m_discrete->write(SPRINT2_MOTORSND2_DATA, m_video_ram[0x395] & 15);
	m_discrete->write(SPRINT2_CRASHSND_DATA, m_video_ram[0x396] & 15);  // also DOMINOS_AMP_DATA

	// interrupts and watchdog are disabled during service mode

	m_watchdog->watchdog_enable(!service_mode());

	if (!service_mode())
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


uint8_t sprint2_state::wram_r(offs_t offset)
{
	return m_video_ram[0x380 + offset % 0x80];
}


uint8_t sprint2_state::dip_r(offs_t offset)
{
	return (m_dsw->read() << (2 * ((offset & 3) ^ 3))) & 0xc0;
}


uint8_t sprint2_state::input_a_r(offs_t offset)
{
	uint8_t val = m_in[0]->read();

	if (m_game == 2)// (MACHINE_IS_SPRINT2)
	{
		if (m_gear[0] == 1) val &= ~0x01;
		if (m_gear[1] == 1) val &= ~0x02;
		if (m_gear[0] == 2) val &= ~0x04;
		if (m_gear[1] == 2) val &= ~0x08;
		if (m_gear[0] == 3) val &= ~0x10;
		if (m_gear[1] == 3) val &= ~0x20;
	}

	return (val << (offset ^ 7)) & 0x80;
}


uint8_t sprint2_state::input_b_r(offs_t offset)
{
	uint8_t val = m_in[1]->read();

	if (m_game == 1) // (MACHINE_IS_SPRINT1)
	{
		if (m_gear[0] == 1) val &= ~0x01;
		if (m_gear[0] == 2) val &= ~0x02;
		if (m_gear[0] == 3) val &= ~0x04;
	}

	return (val << (offset ^ 7)) & 0x80;
}


uint8_t sprint2_state::sync_r()
{
	uint8_t val = 0;

	if (m_outlatch->q0_r() != 0)
		val |= 0x10;

	if (m_screen->vpos() == 261)
		val |= 0x20; // VRESET

	if (m_screen->vpos() >= 224)
		val |= 0x40; // VBLANK

	if (m_screen->vpos() >= 131)
		val |= 0x80; // 60 Hz?

	return val;
}


template <uint8_t Which>
uint8_t sprint2_state::steering_r()
{
	return m_steering[Which];
}


template <uint8_t Which>
void sprint2_state::steering_reset_w(uint8_t data)
{
	m_steering[Which] |= 0x80;
}


void sprint2_state::wram_w(offs_t offset, uint8_t data)
{
	m_video_ram[0x380 + offset % 0x80] = data;
}


void sprint2_state::output_latch_w(offs_t offset, uint8_t data)
{
	m_outlatch->write_bit(offset >> 4, offset & 1);
}


void sprint2_state::noise_reset_w(uint8_t data)
{
	m_discrete->write(SPRINT2_NOISE_RESET, 0);
}


void sprint2_state::main_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x03ff).rw(FUNC(sprint2_state::wram_r), FUNC(sprint2_state::wram_w));
	map(0x0400, 0x07ff).ram().w(FUNC(sprint2_state::video_ram_w)).share(m_video_ram);
	map(0x0818, 0x081f).r(FUNC(sprint2_state::input_a_r));
	map(0x0828, 0x082f).r(FUNC(sprint2_state::input_b_r));
	map(0x0830, 0x0837).r(FUNC(sprint2_state::dip_r));
	map(0x0840, 0x087f).portr("COIN");
	map(0x0880, 0x08bf).r(FUNC(sprint2_state::steering_r<0>));
	map(0x08c0, 0x08ff).r(FUNC(sprint2_state::steering_r<1>));
	map(0x0c00, 0x0fff).r(FUNC(sprint2_state::sync_r));
	map(0x0c00, 0x0c7f).w(FUNC(sprint2_state::output_latch_w));
	map(0x0c80, 0x0cff).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x0d00, 0x0d7f).lw8(NAME([this] (uint8_t data) { m_collision[0] = 0; }));
	map(0x0d80, 0x0dff).lw8(NAME([this] (uint8_t data) { m_collision[1] = 0; }));
	map(0x0e00, 0x0e7f).w(FUNC(sprint2_state::steering_reset_w<0>));
	map(0x0e80, 0x0eff).w(FUNC(sprint2_state::steering_reset_w<1>));
	map(0x0f00, 0x0f7f).w(FUNC(sprint2_state::noise_reset_w));
	map(0x1000, 0x13ff).lr8(NAME([this] () -> uint8_t { return m_collision[0]; }));
	map(0x1400, 0x17ff).lr8(NAME([this] () -> uint8_t { return m_collision[1]; }));
	map(0x1800, 0x1800).nopr();  // debugger ROM location?
	map(0x2000, 0x3fff).rom();
}


static INPUT_PORTS_START( sprint2 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Tracks on Demo" )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Easy Track Only" )
	PORT_DIPSETTING(    0x01, "Cycle 12 Tracks" )
	PORT_DIPNAME( 0x02, 0x00, "Oil Slicks" )            PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused) )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Extended Play" )         PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Play Time" )             PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "60 seconds" )
	PORT_DIPSETTING(    0x80, "90 seconds" )
	PORT_DIPSETTING(    0x40, "120 seconds" )
	PORT_DIPSETTING(    0x00, "150 seconds" )

	PORT_START("INA")   // input A
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // P1 1st gear
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // P2 1st gear
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_UNUSED ) // P1 2nd gear
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // P2 2nd gear
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // P1 3rd gear
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_UNUSED ) // P2 3rd gear

	PORT_START("INB")   // input B
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 1 Gas") PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 2 Gas") PORT_PLAYER(2)
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Track Select") PORT_CODE(KEYCODE_SPACE)

	PORT_START("COIN")
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("GEAR_P1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 1 Gear 1") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 1 Gear 2") PORT_CODE(KEYCODE_X) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 1 Gear 3") PORT_CODE(KEYCODE_C) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 1 Gear 4") PORT_CODE(KEYCODE_V) PORT_PLAYER(1)

	PORT_START("GEAR_P2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 2 Gear 1") PORT_CODE(KEYCODE_Q) PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 2 Gear 2") PORT_CODE(KEYCODE_W) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 2 Gear 3") PORT_CODE(KEYCODE_E) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 2 Gear 4") PORT_CODE(KEYCODE_R) PORT_PLAYER(2)

	PORT_START("DIAL_P1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("DIAL_P2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("MOTOR1")
	PORT_ADJUSTER( 30, "Motor 1 RPM" )

	PORT_START("MOTOR2")
	PORT_ADJUSTER( 40, "Motor 2 RPM" )
INPUT_PORTS_END


static INPUT_PORTS_START( sprint1 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Change Track" )          PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Every Lap" )
	PORT_DIPSETTING(    0x00, "Every 2 Laps" )
	PORT_DIPNAME( 0x02, 0x00, "Oil Slicks" )            PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused) )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Extended Play" )         PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Play Time" )             PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "60 seconds" )
	PORT_DIPSETTING(    0x80, "90 seconds" )
	PORT_DIPSETTING(    0x40, "120 seconds" )
	PORT_DIPSETTING(    0x00, "150 seconds" )

	PORT_START("INA")   // input A

	PORT_START("INB")   // input B
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // 1st gear
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // 2nd gear
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) // 3rd gear
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gas")
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("COIN")
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("GEAR_P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Gear 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gear 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Gear 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Gear 4") PORT_CODE(KEYCODE_V)

	PORT_START("GEAR_P2")

	PORT_START("DIAL_P1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("DIAL_P2")

	PORT_START("MOTOR")
	PORT_ADJUSTER( 30, "Motor RPM" )
INPUT_PORTS_END


static INPUT_PORTS_START( dominos )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, "Points to Win" )         PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x0C, 0x08, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )      PORT_DIPLOCATION("SW1:5") // Manual says "Always on" for dips 5-8
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("INA")   // input A
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("INB")   // input B
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("R23")
	PORT_ADJUSTER( 50, "R23 - Tone Freq" )
INPUT_PORTS_END

static INPUT_PORTS_START( dominos4 )
	PORT_INCLUDE(dominos)

	PORT_MODIFY("INA")   // input A
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)

	PORT_MODIFY("INB")   // input B
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(4)

	PORT_START("SELFTTEST")
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

static const gfx_layout tile_layout =
{
	16, 8,
	64,
	1,
	{ 0 },
	{
		0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
	},
	0x40
};


static const gfx_layout car_layout =
{
	16, 8,
	32,
	1,
	{ 0 },
	{
		0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0,
		0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80
};


static GFXDECODE_START( gfx_sprint2 )
	GFXDECODE_ENTRY( "tiles", 0, tile_layout, 0, 2 )
	GFXDECODE_ENTRY( "sprites", 0, car_layout, 4, 4 )
GFXDECODE_END


void sprint2_state::sprint2(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 12.096_MHz_XTAL / 16);
	m_maincpu->set_addrmap(AS_PROGRAM, &sprint2_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(sprint2_state::irq));

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count(m_screen, 8);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(12.096_MHz_XTAL, 768, 0, 512, 262, 0, 224);
	m_screen->set_screen_update(FUNC(sprint2_state::screen_update));
	m_screen->screen_vblank().set(FUNC(sprint2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sprint2);
	PALETTE(config, m_palette, FUNC(sprint2_state::palette), 12, 4);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	F9334(config, m_outlatch); // at H8
	m_outlatch->q_out_cb<0>().set("discrete", FUNC(discrete_device::write_line<SPRINT2_ATTRACT_EN>)); // also DOMINOS_ATTRACT_EN
	m_outlatch->q_out_cb<1>().set("discrete", FUNC(discrete_device::write_line<SPRINT2_SKIDSND1_EN>)); // also DOMINOS_TUMBLE_EN
	m_outlatch->q_out_cb<2>().set("discrete", FUNC(discrete_device::write_line<SPRINT2_SKIDSND2_EN>));
	m_outlatch->q_out_cb<3>().set_output("led0"); // START LAMP1
	m_outlatch->q_out_cb<4>().set_output("led1"); // START LAMP2
	//m_outlatch->q_out_cb<6>().set(FUNC(sprint2_state::spare_w));

	DISCRETE(config, m_discrete, sprint2_discrete);
	m_discrete->add_route(0, "lspeaker", 1.0);
	m_discrete->add_route(1, "rspeaker", 1.0);
}


void sprint2_state::sprint1(machine_config &config)
{
	sprint2(config);

	// sound hardware
	config.device_remove("lspeaker");
	config.device_remove("rspeaker");
	SPEAKER(config, "mono").front_center();

	DISCRETE(config.replace(), m_discrete, sprint1_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}


void sprint2_state::dominos(machine_config &config)
{
	sprint1(config);

	// sound hardware
	DISCRETE(config.replace(), m_discrete, dominos_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void sprint2_state::dominos4(machine_config &config)
{
	dominos(config);
	m_outlatch->q_out_cb<5>().set_output("led2"); // START LAMP3
	m_outlatch->q_out_cb<6>().set_output("led3"); // START LAMP4
}

ROM_START( sprint1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6290-01.b1", 0x2000, 0x0800, CRC(41fc985e) SHA1(7178846480cbf8d15955ccd987d0b0e902ab9f90) )
	ROM_LOAD( "6291-01.c1", 0x2800, 0x0800, CRC(07f7a920) SHA1(845f65d2bd290eb295ca6bae2575f27aaa08c0dd) )
	ROM_LOAD( "6442-01.d1", 0x3000, 0x0800, CRC(e9ff0124) SHA1(42fe028e2e595573ccc0821de3bb6970364c585d) )
	ROM_LOAD( "6443-01.e1", 0x3800, 0x0800, CRC(d6bb00d0) SHA1(cdcd4bb7b32be7a11480d3312fcd8d536e2d0caf) )

	ROM_REGION( 0x0200, "tiles", 0 )
	ROM_LOAD_NIB_HIGH( "6396-01.p4", 0x0000, 0x0200, CRC(801b42dd) SHA1(1db58390d803f404253cbf36d562016441ca568d) )
	ROM_LOAD_NIB_LOW ( "6397-01.r4", 0x0000, 0x0200, CRC(135ba1aa) SHA1(0465259440f73e1a2c8d8101f29e99b4885420e4) )

	ROM_REGION( 0x0200, "sprites", 0 ) // cars
	ROM_LOAD_NIB_HIGH( "6399-01.j6", 0x0000, 0x0200, CRC(63d685b2) SHA1(608746163e25dbc14cde43c17aecbb9a14fac875) )
	ROM_LOAD_NIB_LOW ( "6398-01.k6", 0x0000, 0x0200, CRC(c9e1017e) SHA1(e7279a13e4a812d2e0218be0bc5162f2e56c6b66) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6400-01.m2", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  // SYNC
	ROM_LOAD( "6401-01.e2", 0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )  // address
ROM_END


ROM_START( sprint2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6290-01.b1", 0x2000, 0x0800, CRC(41fc985e) SHA1(7178846480cbf8d15955ccd987d0b0e902ab9f90) )
	ROM_LOAD( "6291-01.c1", 0x2800, 0x0800, CRC(07f7a920) SHA1(845f65d2bd290eb295ca6bae2575f27aaa08c0dd) )
	ROM_LOAD( "6404.d1",    0x3000, 0x0800, CRC(d2878ff6) SHA1(b742a8896c1bf1cfacf48d06908920d88a2c9ea8) )
	ROM_LOAD( "6405.e1",    0x3800, 0x0800, CRC(6c991c80) SHA1(c30a5b340f05dd702c7a186eb62607a48fa19f72) )

	ROM_REGION( 0x0200, "tiles", 0 )
	ROM_LOAD_NIB_HIGH( "6396-01.p4", 0x0000, 0x0200, CRC(801b42dd) SHA1(1db58390d803f404253cbf36d562016441ca568d) )
	ROM_LOAD_NIB_LOW ( "6397-01.r4", 0x0000, 0x0200, CRC(135ba1aa) SHA1(0465259440f73e1a2c8d8101f29e99b4885420e4) )

	ROM_REGION( 0x0200, "sprites", 0 ) // cars
	ROM_LOAD_NIB_HIGH( "6399-01.j6", 0x0000, 0x0200, CRC(63d685b2) SHA1(608746163e25dbc14cde43c17aecbb9a14fac875) )
	ROM_LOAD_NIB_LOW ( "6398-01.k6", 0x0000, 0x0200, CRC(c9e1017e) SHA1(e7279a13e4a812d2e0218be0bc5162f2e56c6b66) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6400-01.m2", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  // SYNC
	ROM_LOAD( "6401-01.e2", 0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )  // address
ROM_END


ROM_START( sprint2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6290-01.b1", 0x2000, 0x0800, CRC(41fc985e) SHA1(7178846480cbf8d15955ccd987d0b0e902ab9f90) )
	ROM_LOAD( "6291-01.c1", 0x2800, 0x0800, CRC(07f7a920) SHA1(845f65d2bd290eb295ca6bae2575f27aaa08c0dd) )
	ROM_LOAD( "6404.d1",    0x3000, 0x0800, CRC(d2878ff6) SHA1(b742a8896c1bf1cfacf48d06908920d88a2c9ea8) )
	ROM_LOAD( "6405-02.e1", 0x3800, 0x0800, CRC(e80fd249) SHA1(7bcf7dfd72ca83fdd80593eaf392570da1f71298) ) // sldh

	ROM_REGION( 0x0200, "tiles", 0 )
	ROM_LOAD_NIB_HIGH( "6396-01.p4", 0x0000, 0x0200, CRC(801b42dd) SHA1(1db58390d803f404253cbf36d562016441ca568d) )
	ROM_LOAD_NIB_LOW ( "6397-01.r4", 0x0000, 0x0200, CRC(135ba1aa) SHA1(0465259440f73e1a2c8d8101f29e99b4885420e4) )

	ROM_REGION( 0x0200, "sprites", 0 ) // cars
	ROM_LOAD_NIB_HIGH( "6399-01.j6", 0x0000, 0x0200, CRC(63d685b2) SHA1(608746163e25dbc14cde43c17aecbb9a14fac875) )
	ROM_LOAD_NIB_LOW ( "6398-01.k6", 0x0000, 0x0200, CRC(c9e1017e) SHA1(e7279a13e4a812d2e0218be0bc5162f2e56c6b66) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6400-01.m2", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  // SYNC
	ROM_LOAD( "6401-01.e2", 0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )  // address
ROM_END


ROM_START( sprint2h )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6290-01.b1", 0x2000, 0x0800, CRC(41fc985e) SHA1(7178846480cbf8d15955ccd987d0b0e902ab9f90) )
	ROM_LOAD( "6291-01.c1", 0x2800, 0x0800, CRC(07f7a920) SHA1(845f65d2bd290eb295ca6bae2575f27aaa08c0dd) )
	ROM_LOAD( "6404.d1",    0x3000, 0x0800, CRC(d2878ff6) SHA1(b742a8896c1bf1cfacf48d06908920d88a2c9ea8) )
	ROM_LOAD( "6405-02.e1", 0x3800, 0x0800, CRC(6de291f1) SHA1(00c2826011d80ac0784649a7bc156a97c26565fd) ) // sldh

	ROM_REGION( 0x0200, "tiles", 0 )
	ROM_LOAD_NIB_HIGH( "6396-01.p4", 0x0000, 0x0200, CRC(801b42dd) SHA1(1db58390d803f404253cbf36d562016441ca568d) )
	ROM_LOAD_NIB_LOW ( "6397-01.r4", 0x0000, 0x0200, CRC(135ba1aa) SHA1(0465259440f73e1a2c8d8101f29e99b4885420e4) )

	ROM_REGION( 0x0200, "sprites", 0 ) // cars
	ROM_LOAD_NIB_HIGH( "6399-01.j6", 0x0000, 0x0200, CRC(63d685b2) SHA1(608746163e25dbc14cde43c17aecbb9a14fac875) )
	ROM_LOAD_NIB_LOW ( "6398-01.k6", 0x0000, 0x0200, CRC(c9e1017e) SHA1(e7279a13e4a812d2e0218be0bc5162f2e56c6b66) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6400-01.m2", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  // SYNC
	ROM_LOAD( "6401-01.e2", 0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )  // address
ROM_END


ROM_START( sprint2bl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s2_1.bin", 0x2000, 0x0400, CRC(149ae63d) SHA1(0502b1ef151e380319c1d40207f51a187d28516f) )
	ROM_LOAD( "s2_2.bin", 0x2400, 0x0400, CRC(4ecb1cb1) SHA1(c59b1e1487d3b646cff20033c8bf9ce912bd3ee3) )
	ROM_LOAD( "s2_3.bin", 0x2800, 0x0400, CRC(40c0e3d1) SHA1(841d438b81f0d8459b274fc8f974493db62d0138) )
	ROM_LOAD( "s2_4.bin", 0x2c00, 0x0400, CRC(181bb070) SHA1(bf16647aac47039501aacfe71161862815b333fc) )
	ROM_LOAD( "s2_5.bin", 0x3000, 0x0400, CRC(15343ea9) SHA1(f93519833a75bc0a578272689fc79f040d48d17e) )
	ROM_LOAD( "s2_6.bin", 0x3400, 0x0400, CRC(4b3b1eeb) SHA1(2f30a748ce87bb600a2570fcd82071ddd1228a9e) )
	ROM_LOAD( "s2_8.bin", 0x3800, 0x0400, CRC(efebe532) SHA1(a67067defbdd7932b3806ff65c9b0da87254de1c) )
	ROM_LOAD( "s2_7.bin", 0x3c00, 0x0400, CRC(6c63dc81) SHA1(2eea1e88c5fd86cff515689dd20760851d36216d) )

	ROM_REGION( 0x0200, "tiles", 0 )
	ROM_LOAD_NIB_HIGH( "6396-01.p4", 0x0000, 0x0200, CRC(801b42dd) SHA1(1db58390d803f404253cbf36d562016441ca568d) )
	ROM_LOAD_NIB_LOW ( "6397-01.r4", 0x0000, 0x0200, CRC(135ba1aa) SHA1(0465259440f73e1a2c8d8101f29e99b4885420e4) )

	ROM_REGION( 0x0200, "sprites", 0 ) // cars
	ROM_LOAD_NIB_HIGH( "6399-01.j6", 0x0000, 0x0200, CRC(63d685b2) SHA1(608746163e25dbc14cde43c17aecbb9a14fac875) )
	ROM_LOAD_NIB_LOW ( "6398-01.k6", 0x0000, 0x0200, CRC(c9e1017e) SHA1(e7279a13e4a812d2e0218be0bc5162f2e56c6b66) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6400-01.m2", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  // SYNC
	ROM_LOAD( "6401-01.e2", 0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )  // address
ROM_END


ROM_START( dominos )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7352-02.d1",   0x3000, 0x0800, CRC(738b4413) SHA1(3a90ab25bb5f65504692f97da43f03e21392dcd8) )
	ROM_LOAD( "7438-02.e1",   0x3800, 0x0800, CRC(c84e54e2) SHA1(383b388a1448a195f28352fc5e4ff1a2af80cc95) )

	ROM_REGION( 0x200, "tiles", 0 )
	ROM_LOAD_NIB_HIGH( "7439-01.p4",   0x0000, 0x0200, CRC(4f42fdd6) SHA1(f8ea4b582e26cad37b746174cdc9f1c7ae0819c3) )
	ROM_LOAD_NIB_LOW ( "7440-01.r4",   0x0000, 0x0200, CRC(957dd8df) SHA1(280457392f40cd66eae34d2fcdbd4d2142793402) )

	ROM_REGION( 0x200, "sprites", ROMREGION_ERASE00 ) // not used

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6400-01.m2", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  // SYNC
	ROM_LOAD( "6401-01.e2", 0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )  // address
ROM_END


ROM_START( dominos4 ) // built from original Atari source code
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "007754-01.l1",   0x3000, 0x0400, CRC(03fae4a9) SHA1(a132bd8bc866e33cdf6b4881064c8d265c2b25f4) )
	ROM_LOAD_NIB_LOW ( "007755-01.l0",   0x3000, 0x0400, CRC(fa2d0c04) SHA1(fcf618c7089db46d55933d58ea04701af515ad49) )
	ROM_LOAD_NIB_HIGH( "007756-01.m1",   0x3400, 0x0400, CRC(d2acb1b5) SHA1(ad81eed9dd0a2d5ecfd42daf90825726e64063b3) )
	ROM_LOAD_NIB_LOW ( "007757-01.m0",   0x3400, 0x0400, CRC(69f2db90) SHA1(a064c840599c4e7cb65670e5480adeb310247f16) )
	ROM_LOAD_NIB_HIGH( "007758-01.n1",   0x3800, 0x0400, CRC(b49083b4) SHA1(41999e8d3fd6104c42f3a034045f9f9c75d8247a) )
	ROM_LOAD_NIB_LOW ( "007759-01.n0",   0x3800, 0x0400, CRC(542200c7) SHA1(111f06e942e247b00b9f90fae2986c3c8d9ec8c5) )
	ROM_LOAD_NIB_HIGH( "007760-01.p1",   0x3c00, 0x0400, CRC(7dc2a7a1) SHA1(9d02572cf689c6476b33226a5358dd1f72c4e61d) )
	ROM_LOAD_NIB_LOW ( "007761-01.p0",   0x3c00, 0x0400, CRC(04365e0d) SHA1(fefc3c04e55f1aa8c80b1e5e1e403af8698c3530) )

	ROM_REGION( 0x200, "tiles", 0 )
	ROM_LOAD_NIB_HIGH( "007764-01.p4",   0x0000, 0x0200, CRC(e4332dc0) SHA1(1f16c5b9f9fd7d478fd729cc79968f17746111f4) )
	ROM_LOAD_NIB_LOW ( "007765-01.r4",   0x0000, 0x0200, CRC(6e4e6c75) SHA1(0fc77fecaa73eac57baf778bc51387c75883aad4) )

	ROM_REGION( 0x200, "sprites", ROMREGION_ERASE00 ) // not used

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6400-01.m2", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  // SYNC
	ROM_LOAD( "6401-01.e2", 0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )  // address
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT   MACHINE   INPUT     CLASS          INIT            ROT   COMPANY              FULLNAME                       FLAGS
GAME( 1978, sprint1,   0,       sprint1,  sprint1,  sprint2_state, init_sprint1,   ROT0, "Atari (Kee Games)", "Sprint 1",                    MACHINE_SUPPORTS_SAVE )
GAME( 1976, sprint2,   sprint1, sprint2,  sprint2,  sprint2_state, init_sprint2,   ROT0, "Atari (Kee Games)", "Sprint 2 (set 1)",            MACHINE_SUPPORTS_SAVE )
GAME( 1976, sprint2a,  sprint1, sprint2,  sprint2,  sprint2_state, init_sprint2,   ROT0, "Atari (Kee Games)", "Sprint 2 (set 2)",            MACHINE_SUPPORTS_SAVE )
GAME( 1976, sprint2h,  sprint1, sprint2,  sprint2,  sprint2_state, init_sprint2,   ROT0, "hack",              "Sprint 2 (color kit, Italy)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE ) // Italian hack, supposedly is color instead of b/w? how?
GAME( 1976, sprint2bl, sprint1, sprint2,  sprint2,  sprint2_state, init_sprint2bl, ROT0, "bootleg",           "Sprint 2 (bootleg)",          MACHINE_SUPPORTS_SAVE )
GAME( 1977, dominos,   0,       dominos,  dominos,  sprint2_state, init_dominos,   ROT0, "Atari",             "Dominos",                     MACHINE_SUPPORTS_SAVE )
GAME( 1977, dominos4,  dominos, dominos4, dominos4, sprint2_state, init_dominos4,  ROT0, "Atari",             "Dominos 4 (Cocktail)",        MACHINE_SUPPORTS_SAVE )
