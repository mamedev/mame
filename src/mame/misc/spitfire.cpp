// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
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
 * collision tests are in the program ROM. The internal layout supplies a
 * simplified tower based on photographs, not the complete original overlay.
 *
 * TODO: verify oscillator, raster timing and display offsets against the PCB.
 * Trace the discrete sound circuit and analog game timer. Refine artwork
 * alignment and reproduce the full physical overlay.
 */

#include "emu.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "spitfire.lh"

namespace {

// Functional approximation, NOT a reconstruction of the PCB sound circuit.
// The photographs show a probable 4006, a 9602 and a 7475 near the MC1358.
// This suggests noise, timed pulses and analogue shaping, but the connections,
// feedback taps, clock, RC values and even the exact effect timbres are unknown.
// All synthesis frequencies, gains and time constants below are provisional.
//
// ROM-observed controls (both program revisions):
//   5000/6000 = 01: shot created by the corresponding player
//   5xxx/6xxx = 02, then repeated 22: aircraft falling after damage
//   5xxx/6xxx = 08: impact/destruction helper (can be called more than once)
//   5000 = 00/04: normal loop; 04 if EITHER aircraft's speed counter is 4
//
// Model both address banks as writes to shared mono hardware. Trigger shot and
// impact envelopes on writes, not separate per-player latch edges: player 2
// fires repeatedly with no intervening clear in the 6xxx bank. Keep D1 as a
// level so the rapid 22 writes do not repeatedly restart the falling effect.
// D5 and the exact address/enable decoding remain unresolved. No separate
// sound is invented for D5, and no CPU-internal gameplay state is consulted.
class spitfire_sound_device : public device_t, public device_sound_interface
{
public:
	spitfire_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void control_w(u8 data);
	void enable_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr unsigned SAMPLE_RATE = 48'000;
	sound_stream* m_stream;
	u8 m_control;
	bool m_enabled;
	u32 m_noise;
	u8 m_noise_count;
	float m_noise_low;
	float m_noise_high;
	float m_engine_phase;
	float m_engine_filter;
	float m_engine_level;
	float m_fall_sweep;
	float m_fall_filter;
	float m_fall_level;
	float m_shot;
	float m_impact;
	float m_output_dc;
};

DEFINE_DEVICE_TYPE_PRIVATE(SPITFIRE_SOUND, spitfire_sound_device, spitfire_sound_device, "spitfire_sound", "Spitfire sound (approximation)")

spitfire_sound_device::spitfire_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SPITFIRE_SOUND, tag, owner, clock),
	device_sound_interface(mconfig, *this)
{
}

void spitfire_sound_device::device_start()
{
	m_stream = stream_alloc(0, 1, SAMPLE_RATE);
	save_item(NAME(m_control));
	save_item(NAME(m_enabled));
	save_item(NAME(m_noise));
	save_item(NAME(m_noise_count));
	save_item(NAME(m_noise_low));
	save_item(NAME(m_noise_high));
	save_item(NAME(m_engine_phase));
	save_item(NAME(m_engine_filter));
	save_item(NAME(m_engine_level));
	save_item(NAME(m_fall_sweep));
	save_item(NAME(m_fall_filter));
	save_item(NAME(m_fall_level));
	save_item(NAME(m_shot));
	save_item(NAME(m_impact));
	save_item(NAME(m_output_dc));
}

void spitfire_sound_device::device_reset()
{
	m_stream->update();
	m_control = 0;
	m_enabled = false;
	m_noise = 1;
	m_noise_count = 0;
	m_noise_low = m_noise_high = 0.0f;
	m_engine_phase = m_engine_filter = m_engine_level = 0.0f;
	m_fall_sweep = m_fall_filter = m_fall_level = 0.0f;
	m_shot = m_impact = m_output_dc = 0.0f;
}

void spitfire_sound_device::control_w(u8 data)
{
	m_stream->update();
	if (m_enabled)
	{
		if (BIT(data, 0))
			m_shot = 1.0f;
		if (BIT(data, 3))
			m_impact = 1.0f;
		if (BIT(data, 1) && !BIT(m_control, 1))
			m_fall_sweep = 0.0f;
	}
	m_control = data;
}

void spitfire_sound_device::enable_w(int state)
{
	m_stream->update();
	m_enabled = bool(state);
	// A separate game-active gate is assumed: the ROM can leave 04 latched
	// when time expires. Clear pending effects across round boundaries.
	m_control = 0;
	m_shot = m_impact = 0.0f;
}

void spitfire_sound_device::sound_stream_update(sound_stream &stream)
{
	// Selected substitute values, not measured component values:
	// 12 kHz noise clock; 80 Hz engine flutter; shot/impact decay 25/160 ms.
	// Filters are first-order approximations, with 5/10 ms control smoothing
	// and a 20 Hz output DC blocker. A falling aircraft darkens the noise
	// over 300 ms; repeated 22 writes maintain, rather than restart, this.
	const float shot_decay = std::exp(-1.0f / (0.025f * SAMPLE_RATE));
	const float impact_decay = std::exp(-1.0f / (0.160f * SAMPLE_RATE));
	const float engine_target = m_enabled && BIT(m_control, 2) ? 1.0f : 0.0f;
	const float fall_target = m_enabled && BIT(m_control, 1) ? 1.0f : 0.0f;
	for (int i = 0; i < stream.samples(); i++)
	{
		if (++m_noise_count == 4)
		{
			m_noise_count = 0;
			// Maximal 17-bit sequence chosen as a deterministic noise source.
			// This does NOT establish the actual 4006 feedback configuration.
			m_noise = (m_noise >> 1) | ((BIT(m_noise, 0) ^ BIT(m_noise, 3)) << 16);
		}
		const float noise = BIT(m_noise, 0) ? 1.0f : -1.0f;
		m_noise_low += 0.0755f * (noise - m_noise_low); // approximately 600 Hz
		m_noise_high += 0.4076f * (noise - m_noise_high); // approximately 4 kHz

		m_engine_phase += 80.0f / SAMPLE_RATE;
		if (m_engine_phase >= 1.0f)
			m_engine_phase -= 1.0f;
		const float engine = noise * (m_engine_phase < 0.25f ? 1.0f : 0.25f);
		m_engine_filter += 0.0448f * (engine - m_engine_filter); // 350 Hz
		m_engine_level += (engine_target - m_engine_level) / (0.005f * SAMPLE_RATE);

		if (fall_target)
			m_fall_sweep = std::min(1.0f, m_fall_sweep + 1.0f / (0.300f * SAMPLE_RATE));
		m_fall_filter += (0.21f - 0.16f * m_fall_sweep) * (noise - m_fall_filter);
		m_fall_level += (fall_target - m_fall_level) / (0.010f * SAMPLE_RATE);

		const float mix = 0.24f * m_engine_filter * m_engine_level
				+ 0.26f * m_fall_filter * m_fall_level
				+ 0.32f * (m_noise_high - m_noise_low) * m_shot
				+ 0.42f * m_noise_low * m_impact;
		m_output_dc += 0.002615f * (mix - m_output_dc);
		stream.put(0, i, mix - m_output_dc);
		m_shot *= shot_decay;
		m_impact *= impact_decay;
		// Avoid denormals and leave the idle output exactly silent after tails.
		if (m_shot < 1.0e-7f) m_shot = 0.0f;
		if (m_impact < 1.0e-7f) m_impact = 0.0f;
		if (!engine_target && m_engine_level < 1.0e-7f) m_engine_level = 0.0f;
		if (!fall_target && m_fall_level < 1.0e-7f) m_fall_level = 0.0f;
		if (std::abs(m_output_dc) < 1.0e-12f) m_output_dc = 0.0f;
	}
}

class spitfire_state : public driver_device
{
public:
	spitfire_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_smi(*this, "smi"),
		m_sound(*this, "soundboard"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_inputs(*this, "IN%u", 0),
		m_duration(*this, "DURATION")
	{ }

	void spitfire(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(coin_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<f8_cpu_device> m_maincpu;
	required_device<f3853_device> m_smi;
	required_device<spitfire_sound_device> m_sound;
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

	u8 input0_r();
	u8 input1_r();
	void port1_w(u8 data);
	void y_w(offs_t offset, u8 data);
	void x_w(offs_t offset, u8 data);
	void clear_w(offs_t offset, u8 data);
	template <unsigned N> void sprite_w(u8 data);
	void game_off_w(u8 data);
	void game_on_w(u8 data);
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
	m_sound->enable_w(0);
	m_game_timer->adjust(attotime::never);
}

void spitfire_state::game_on_w(u8 data)
{
	m_game_on = true;
	m_sound->enable_w(1);
	// The PCB has an analog duration adjustment, not a DIP switch.
	m_game_timer->adjust(attotime::from_seconds(m_duration->read()));
}

TIMER_CALLBACK_MEMBER(spitfire_state::game_over)
{
	m_game_on = false;
	m_sound->enable_w(0);
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
	map(0x5000, 0x6fff).w(m_sound, FUNC(spitfire_sound_device::control_w));
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
	SPITFIRE_SOUND(config, m_sound).add_route(ALL_OUTPUTS, "mono", 1.0);
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
