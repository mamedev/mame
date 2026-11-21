// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
 * Spitfire (Wizard Kings / Innovative Coin Corporation, 1976)
 *
 * Program PROM outputs are inverted. The F8 uses a 3853-compatible SMI
 * with its interrupt registers at I/O 0c-0f. Coin insertion interrupts the
 * score display; game timing is external to the CPU.
 *
 * Display RAM contains object position strobes, not pixels: four vertical
 * channels at 7000 and four horizontal channels at 8000. Aircraft graphics
 * are selected by C000/D000; the other two channels are bullets.
 *
 * The tower is physical screen-overlay artwork, not generated video. Its
 * collision tests are in the program ROM. The internal layout redraws the
 * physical overlay from photographs, including the sky and airfield scenery.
 * Colours, optical blending and registration are approximate.
 *
 * TODO: verify oscillator, raster timing and display offsets against the PCB.
 * Trace the discrete sound circuit and analog game timer. Verify artwork
 * registration and optical properties against physical measurements.
 */

#include "emu.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/netlist.h"
#include "nl_spitfire.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "spitfire.lh"

namespace {

class spitfire_state : public driver_device
{
public:
	spitfire_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_smi(*this, "smi"),
		m_sound_trigger(*this, "soundboard:trigger%u", 0),
		m_sound_engine(*this, "soundboard:engine"),
		m_sound_fall(*this, "soundboard:fall"),
		m_sound_enable(*this, "soundboard:enable"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_inputs(*this, "IN%u", 0),
		m_duration(*this, "DURATION")
	{ }

	void spitfire(machine_config &config) ATTR_COLD;
	DECLARE_INPUT_CHANGED_MEMBER(coin_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<f8_cpu_device> m_maincpu;
	required_device<f3853_device> m_smi;
	required_device_array<netlist_mame_logic_input_device, 2> m_sound_trigger;
	required_device<netlist_mame_logic_input_device> m_sound_engine;
	required_device<netlist_mame_logic_input_device> m_sound_fall;
	required_device<netlist_mame_logic_input_device> m_sound_enable;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_ioport_array<2> m_inputs;
	required_ioport m_duration;

	u8 m_y_ram[0x100];
	u8 m_x_ram[0x200];
	u8 m_sprite[2];
	u8 m_port1;
	bool m_game_on;
	emu_timer *m_game_timer;
	emu_timer *m_sound_pulse_timer[2];

	u8 input0_r();
	u8 input1_r();
	void port1_w(u8 data);
	void y_w(offs_t offset, u8 data);
	void x_w(offs_t offset, u8 data);
	void clear_w(offs_t offset, u8 data);
	template <unsigned N> void sprite_w(u8 data);
	void game_off_w(u8 data);
	void game_on_w(u8 data);
	void sound_w(u8 data);
	void sound_enable_w(int state);
	TIMER_CALLBACK_MEMBER(sound_pulse_off);
	TIMER_CALLBACK_MEMBER(game_over);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};

u8 spitfire_state::input0_r()
{
	return m_inputs[0]->read() | (m_screen->vblank() ? 0 : 0x80);
}

u8 spitfire_state::input1_r()
{
	return m_inputs[1]->read() | (m_game_on ? 0 : 0x02) | m_port1;
}

void spitfire_state::port1_w(u8 data)
{
	m_port1 = data;
}

INPUT_CHANGED_MEMBER(spitfire_state::coin_changed)
{
	m_smi->ext_int_w(!newval);
}

void spitfire_state::y_w(offs_t offset, u8 data)
{
	m_screen->update_partial(m_screen->vpos());
	m_y_ram[offset] = data & 0x0f;
}

void spitfire_state::x_w(offs_t offset, u8 data)
{
	m_screen->update_partial(m_screen->vpos());
	m_x_ram[offset] = data >> 4;
}

void spitfire_state::clear_w(offs_t offset, u8 data)
{
	m_screen->update_partial(m_screen->vpos());
	m_y_ram[offset & 0xff] = data & 0x0f;
	m_x_ram[offset] = data >> 4;
}

template <unsigned N> void spitfire_state::sprite_w(u8 data)
{
	m_screen->update_partial(m_screen->vpos());
	m_sprite[N] = data & 0x1f;
}

void spitfire_state::game_off_w(u8 data)
{
	m_game_on = false;
	sound_enable_w(0);
	m_game_timer->adjust(attotime::never);
}

void spitfire_state::game_on_w(u8 data)
{
	m_game_on = true;
	sound_enable_w(1);
	// The PCB has an analog duration adjustment, not a DIP switch.
	m_game_timer->adjust(attotime::from_seconds(m_duration->read()));
}

TIMER_CALLBACK_MEMBER(spitfire_state::game_over)
{
	m_game_on = false;
	sound_enable_w(0);
}

// Both ROM revisions write shared mono controls in the 5xxx/6xxx banks:
// D0 = shot, D1 = falling aircraft, D2 = either player at full throttle,
// D3 = impact. Repeated 22 writes hold the falling effect; D5 is unresolved.
// Address decoding and the separate game-active gate are still provisional.
void spitfire_state::sound_w(u8 data)
{
	m_sound_engine->write_line(m_game_on && BIT(data, 2));
	m_sound_fall->write_line(m_game_on && BIT(data, 1));
	if (m_game_on)
	{
		for (unsigned i = 0; i < 2; ++i)
		{
			if (BIT(data, i ? 3 : 0))
			{
				// A short charge pulse for EVERY write, not a data-bit edge.
				// P2 can fire again without an intervening zero in bank 6.
				// 100 us is selected for this substitute circuit, not measured.
				m_sound_trigger[i]->write_line(1);
				m_sound_pulse_timer[i]->adjust(attotime::from_usec(100), i);
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(spitfire_state::sound_pulse_off)
{
	m_sound_trigger[param]->write_line(0);
}

void spitfire_state::sound_enable_w(int state)
{
	// The ROM may leave 04 latched on expiry. Clear pending effects across
	// round boundaries and let the netlist discharge its transient envelopes.
	m_sound_enable->write_line(state);
	m_sound_engine->write_line(0);
	m_sound_fall->write_line(0);
	for (unsigned i = 0; i < 2; ++i)
	{
		m_sound_trigger[i]->write_line(0);
		m_sound_pulse_timer[i]->adjust(attotime::never);
	}
}

uint32_t spitfire_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// RAM bits are active-low start strobes. A plane's counters run for
	// sixteen pixels/lines after its strobe. Bullets use the RAM bits directly.
	for (int object = 0; object < 4; object++)
	{
		for (int y = 0; y < 256; y++)
		{
			if (BIT(m_y_ram[y], object))
				continue;
			for (int x = 0; x < 512; x++)
			{
				if (BIT(m_x_ram[x], object))
					continue;
				if (object >= 2)
				{
					if (cliprect.contains(x, y))
						bitmap.pix(y, x) = 1;
					continue;
				}
				m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, m_sprite[object], 0, 0, 0, x, y, 0);
			}
		}
	}
	return 0;
}

void spitfire_state::prg_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x5000, 0x6fff).w(FUNC(spitfire_state::sound_w));
	map(0x7000, 0x70ff).mirror(0x0f00).w(FUNC(spitfire_state::y_w));
	map(0x8000, 0x81ff).mirror(0x0e00).w(FUNC(spitfire_state::x_w));
	map(0x9000, 0x91ff).mirror(0x0e00).w(FUNC(spitfire_state::clear_w));
	map(0xc000, 0xcfff).w(FUNC(spitfire_state::sprite_w<0>));
	map(0xd000, 0xdfff).w(FUNC(spitfire_state::sprite_w<1>));
	map(0xe000, 0xefff).w(FUNC(spitfire_state::game_off_w));
	map(0xf000, 0xffff).w(FUNC(spitfire_state::game_on_w));
}

void spitfire_state::io_map(address_map &map)
{
	map(0x00, 0x00).r(FUNC(spitfire_state::input0_r)).nopw();
	map(0x01, 0x01).rw(FUNC(spitfire_state::input1_r), FUNC(spitfire_state::port1_w));
	map(0x0c, 0x0f).rw(m_smi, FUNC(f3853_device::read), FUNC(f3853_device::write));
}

static INPUT_PORTS_START( spitfire )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(spitfire_state::coin_changed), 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(2) PORT_NAME("P2 Accelerate")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(2) PORT_NAME("P2 Decelerate")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(1) PORT_NAME("P1 Accelerate")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(1) PORT_NAME("P1 Decelerate")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)

	PORT_START("DURATION")
	PORT_CONFNAME(0xff, 90, "Game Time (seconds)")
	PORT_CONFSETTING(60, "60")
	PORT_CONFSETTING(90, "90")
	PORT_CONFSETTING(120, "120")
INPUT_PORTS_END

static const gfx_layout aircraft_layout =
{
	16, 16,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	{ STEP8(7, -1), STEP8(15, -1) },
	{ STEP16(0, 16) },
	16 * 16
};

static GFXDECODE_START(gfx_spitfire)
	GFXDECODE_ENTRY("gfx", 0, aircraft_layout, 0, 1)
GFXDECODE_END

void spitfire_state::machine_start()
{
	m_game_timer = timer_alloc(FUNC(spitfire_state::game_over), this);
	for (auto &timer : m_sound_pulse_timer)
		timer = timer_alloc(FUNC(spitfire_state::sound_pulse_off), this);
	save_item(NAME(m_y_ram));
	save_item(NAME(m_x_ram));
	save_item(NAME(m_sprite));
	save_item(NAME(m_port1));
	save_item(NAME(m_game_on));
}

void spitfire_state::machine_reset()
{
	std::fill(std::begin(m_y_ram), std::end(m_y_ram), 0x0f);
	std::fill(std::begin(m_x_ram), std::end(m_x_ram), 0x0f);
	std::fill(std::begin(m_sprite), std::end(m_sprite), 0);
	m_port1 = 0;
	m_game_on = false;
	m_game_timer->adjust(attotime::never);
	sound_enable_w(0);
}

void spitfire_state::spitfire(machine_config &config)
{
	F8(config, m_maincpu, 2'000'000); // oscillator frequency unverified
	m_maincpu->set_addrmap(AS_PROGRAM, &spitfire_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &spitfire_state::io_map);
	m_maincpu->int_cycle_callback().set(m_smi, FUNC(f3853_device::int_acknowledge));

	F3853(config, m_smi, 2'000'000);
	m_smi->int_req_callback().set_inputline(m_maincpu, F8_INPUT_LINE_INT_REQ);

	SCREEN(config, m_screen);
	// Approximate 60 Hz raster, pending oscillator/counter measurements.
	// Vertical position RAM is eight bits, but a sprite triggered at line
	// 245 (the score/start position) continues past line 255.
	m_screen->set_raw(512 * 262 * 60, 512, 96, 512, 262, 16, 262);
	m_screen->set_screen_update(FUNC(spitfire_state::screen_update));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_spitfire);
	PALETTE(config, "palette", palette_device::MONOCHROME);
	SPEAKER(config, "mono").front_center();
	NETLIST_SOUND(config, "soundboard", 48000)
		.set_source(NETLIST_NAME(spitfire))
		.add_route(ALL_OUTPUTS, "mono", 1.0);
	NETLIST_LOGIC_INPUT(config, m_sound_trigger[0], "SHOT.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_sound_trigger[1], "IMPACT.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_sound_engine, "ENGINE.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_sound_fall, "FALL.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_sound_enable, "ENABLE.IN", 0);
	NETLIST_STREAM_OUTPUT(config, "soundboard:output", 0, "OUTPUT").set_mult_offset(1.0, 0.0);
}

ROM_START( spitfire )
	ROM_REGION( 0x800, "maincpu", ROMREGION_INVERT )
	// near the CPU
	ROM_LOAD( "sf_p1a.h1", 0x000, 0x200, CRC(75e294f1) SHA1(f0cb60bd7273ca36fb0f97f1b1b69a4493437c97) )
	//ROM_LOAD( "sf_p2.h2",  0x200, 0x200, CRC(3f86e112) SHA1(01bba52db889fef70ea09fb2bb3de9142f40f49a) ) // FIXED BITS (xxxx1xxx)
	ROM_LOAD( "sf_p2.h2",  0x200, 0x200, CRC(242dc618) SHA1(b5fe5ce084b88de2d656f749a932396defc46313) ) // taken from set below, same data without the fixed bits problem
	ROM_LOAD( "sf_p3.j1",  0x400, 0x200, CRC(88caf987) SHA1(c6cf88c95fc78292678a893b4685659b27c322f5) )
	ROM_LOAD( "sf_p4.j2",  0x600, 0x200, CRC(09afd5ee) SHA1(b3e7403e10ae4764890635324f016f81e291f10a) )

	ROM_REGION( 0x400, "gfx", ROMREGION_INVERT )
	// on the opposite side of the PCB to the CPU
	ROM_LOAD( "sf_f1.a3",  0x000, 0x200, CRC(4ed940b5) SHA1(09da6c1df5f9264c226a204fb895e866f5b85138) )
	ROM_LOAD( "sf_f2.a4",  0x200, 0x200, CRC(99a1230b) SHA1(f88aa9e71031fe504cf444df65a02283c45d3f31) )
ROM_END

ROM_START( spitfirea )
	ROM_REGION( 0x800, "maincpu", ROMREGION_INVERT )
	// near the CPU
	//ROM_LOAD( "sf_p1a.h1", 0x000, 0x200, CRC(25588f41) SHA1(783daf04f357122e909b4dbc351a3effe491a4dc) ) // BADADDR xxxxx-xxx
	ROM_LOAD( "sf_p1a.h1", 0x000, 0x200, CRC(75e294f1) SHA1(f0cb60bd7273ca36fb0f97f1b1b69a4493437c97) ) // taken from set above, same data without the badaddr problem
	ROM_LOAD( "sf_p2.h2",  0x200, 0x200, CRC(242dc618) SHA1(b5fe5ce084b88de2d656f749a932396defc46313) )
	ROM_LOAD( "sf_p3.j1",  0x400, 0x200, CRC(88caf987) SHA1(c6cf88c95fc78292678a893b4685659b27c322f5) )
	ROM_LOAD( "sf_p4a.j2", 0x600, 0x200, CRC(13920a83) SHA1(2ce3884898c1851e83025eaedef846f591263a64) ) // different label, unique ROM

	ROM_REGION( 0x400, "gfx", ROMREGION_INVERT )
	// on the opposite side of the PCB to the CPU
	ROM_LOAD( "sf_f1.a3",  0x000, 0x200, CRC(4ed940b5) SHA1(09da6c1df5f9264c226a204fb895e866f5b85138) )
	ROM_LOAD( "sf_f2.a4",  0x200, 0x200, CRC(99a1230b) SHA1(f88aa9e71031fe504cf444df65a02283c45d3f31) )
ROM_END

} // anonymous namespace

GAMEL( 1976, spitfire,  0,        spitfire, spitfire,  spitfire_state, empty_init, ROT0, "Innovative Coin Corporation", "Spitfire (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_SOUND | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_spitfire )
GAMEL( 1976, spitfirea, spitfire, spitfire, spitfire,  spitfire_state, empty_init, ROT0, "Innovative Coin Corporation", "Spitfire (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_SOUND | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_spitfire )
