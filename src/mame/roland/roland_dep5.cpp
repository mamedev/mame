// license:BSD-3-Clause
// copyright-holders:Giulio Zausa
/*
 Roland DEP-5 (1986) / RE-3 (1988), original 8097 firmware.

 Both use a 12 MHz CPU, battery SRAM, an MB673106 I/O gate array and
 an MB87126 reverb DSP (40.96 MHz, 160 steps per nominal 32 kHz frame).
 DEP-5 uses the MB87126-002 and adds an MB654119 chorus; RE-3 uses the
 MB87126A-007 and has a serial ten-character VFD.

 The MB87126A-007 fixes a timing bug in the serial host interface. Its
 internal program may also differ from the MB87126-002; using the same
 program for both revisions is currently an unverified assumption.

 Host maps and panel wiring follow the service notes and firmware traces.
 The original CPU firmware handles panel scanning, MIDI and effect parameter
 calculations and uploads.
 The MB87126 executes the external die-derived 35-bit ROM word by word.
 Difficult-to-read die cells were corrected manually using chip-behavior
 probes; this canonical extraction is still marked BAD_DUMP.

 TODO: gate-array bus decode aliases/wait states, exact BUSY arbitration,
 converter bit timing / DAC low nibbles, input level meter circuit (the LEDs
 follow a peak detector on the converter feed, thresholds read off the panel
 legends), unidentified RE-3 footswitch wiring and VFD range controls. Analog
 mixing is deliberately simple: normalized mono feed, held stereo wet and
 linear balance.
*/
#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/mcs96/i8x9x.h"
#include "machine/nvram.h"
#include "sound/mb654119.h"
#include "sound/mb87126.h"

#include "speaker.h"

#include <algorithm>
#include <cmath>

#include "roland_dep5.lh"
#include "roland_re3.lh"

namespace {

static constexpr float DEP5_DB[6] = { -3, -6, -9, -15, -19, -29 };
static constexpr float RE3_DB[6] = { -3, -9, -12, -15, -18, -21 };

class dep5_base_state : public driver_device, public device_serial_interface, public device_sound_interface
{
protected:
	dep5_base_state(const machine_config &config, device_type type, const char *tag)
		: driver_device(config, type, tag)
		, device_serial_interface(config, *this)
		, device_sound_interface(config, *this)
		, m_cpu(*this, "maincpu")
		, m_reverb(*this, "reverb")
		, m_midiout(*this, "midiout")
		, m_keys(*this, "KEY%u", 0U)
		, m_knobs(*this, "KNOB%u", 0U)
		, m_input(*this, "INPUT")
		, m_balance(*this, "BALANCE")
		, m_pedals(*this, "PEDALS")
		, m_level(*this, "level%u", 0U)
	{
	}

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void rcv_complete() override;
	virtual void tra_callback() override { m_midiout->write_txd(transmit_register_get_data_bit()); }
	virtual void sound_stream_update(sound_stream &stream) override;
	virtual void display_update() = 0;
	virtual void display_tick() { }
	virtual const float *meter_thresholds() const = 0;
	virtual float input_gain2() const { return m_input->read() / 100.0f; }
	virtual bool output_muted() const { return false; }

	void common(machine_config &config) ATTR_COLD;
	void common_map(address_map &map) ATTR_COLD;
	template <int N> u16 knob_r() { return (m_knobs[N]->read() * 1023 + 50) / 100; }
	u8 port2_r() { return (m_reverb->busy_r() << 6) | m_pedals->read(); }
	void port2_w(u8 data) { m_reverb->control_w(BIT(data, 5), BIT(data, 7)); }
	void select_w(u8 data) { m_select = data; }
	void latch_w(u8 data);
	void timer_w(offs_t offset, u8 data);
	void midi_tx(u8 data) { transmit_register_setup(data); }
	TIMER_CALLBACK_MEMBER(panel_tick);

	required_device<n8097bh_device> m_cpu;
	required_device<mb87126_device> m_reverb;
	required_device<midi_port_device> m_midiout;
	required_ioport_array<8> m_keys;
	required_ioport_array<8> m_knobs;
	required_ioport m_input, m_balance, m_pedals;
	output_finder<6> m_level;
	sound_stream *m_feed = nullptr, *m_mix = nullptr;
	emu_timer *m_panel_timer = nullptr;
	float m_peak = 0, m_meter = 0;
	std::array<u8, 4> m_timer{};
	u8 m_select = 0, m_port1 = 0xff, m_latch = 0;
};

class dep5_state : public dep5_base_state
{
public:
	dep5_state(const machine_config &config, device_type type, const char *tag)
		: dep5_base_state(config, type, tag)
		, m_chorus(*this, "chorus")
		, m_mono(*this, "MONO")
		, m_grid(*this, "grid%u", 0U)
	{
	}

	void dep5(machine_config &config) ATTR_COLD;
	INPUT_CHANGED_MEMBER(key_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void display_update() override;
	virtual const float *meter_thresholds() const override { return DEP5_DB; }
	virtual bool output_muted() const override { return !BIT(m_port1, 0); }

private:
	void mem_map(address_map &map) ATTR_COLD;
	u8 scan_keys();
	u8 port0_r() { return (scan_keys() & 1) << 7; }
	u8 port1_r() { return m_mono->read(); }
	u8 port2_r() { return dep5_base_state::port2_r() | (BIT(scan_keys(), 1) << 2); }
	void port1_w(u8 data);
	void select_w(u8 data);
	void update_extint();
	void chorus_rate_w(u8 data) { m_reverb->update(); m_chorus->write_rate(data); }
	void chorus_depth_w(u8 data) { m_reverb->update(); m_chorus->write_depth(data); }
	void chorus_delay_w(u8 data) { m_reverb->update(); m_chorus->write_delay(data); }
	void reverb_bus_w(offs_t signals, u32 data);

	required_device<mb654119_device> m_chorus;
	required_ioport m_mono;
	output_finder<7> m_grid;
	s16 m_chorus_left = 0, m_chorus_right = 0;
};

class re3_state : public dep5_base_state
{
public:
	re3_state(const machine_config &config, device_type type, const char *tag)
		: dep5_base_state(config, type, tag)
		, m_input2(*this, "INPUT2")
		, m_char(*this, "char%u", 0U)
		, m_mark(*this, "mark%u", 0U)
	{
	}

	void re3(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void display_update() override;
	virtual void display_tick() override;
	virtual const float *meter_thresholds() const override { return RE3_DB; }
	virtual float input_gain2() const override { return m_input2->read() / 100.0f; }

private:
	void mem_map(address_map &map) ATTR_COLD;
	u8 port1_r();
	void port1_w(u8 data);
	void vfd_w(u8 data);

	required_ioport m_input2;
	output_finder<10> m_char, m_mark;
	bool m_blink_phase = false;
	u8 m_ticks = 0;
	u8 m_vfd_shift = 0, m_vfd_bits = 0, m_vfd_first = 0;
	bool m_vfd_pending = false;
	std::array<u8, 10> m_text{};
	std::array<bool, 10> m_marks{}, m_blinks{};
};

void dep5_base_state::machine_start()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rcv_rate(31250);
	set_tra_rate(31250);

	// Separate streams avoid a feedback dependency: input feed -> DSP -> output mix.
	m_feed = stream_alloc(2, 1, 256000);
	m_mix = stream_alloc(4, 2, 256000);
	m_panel_timer = timer_alloc(FUNC(dep5_base_state::panel_tick), this);
	m_panel_timer->adjust(attotime::from_hz(32), 0, attotime::from_hz(32));
	save_item(NAME(m_peak));
	save_item(NAME(m_meter));
	save_item(NAME(m_timer));
	save_item(NAME(m_select));
	save_item(NAME(m_port1));
	save_item(NAME(m_latch));
}

void dep5_base_state::machine_reset()
{
	m_timer = {};
	m_select = m_latch = 0;
	m_port1 = 0xff;
	m_peak = m_meter = 0;
	receive_register_reset();
	transmit_register_reset();
}

void dep5_state::machine_start()
{
	dep5_base_state::machine_start();
	save_item(NAME(m_chorus_left));
	save_item(NAME(m_chorus_right));
}

void dep5_state::machine_reset()
{
	dep5_base_state::machine_reset();
	m_chorus_left = m_chorus_right = 0;
	for (int i = 0; i < 7; ++i)
		m_grid[i] = 0;
	update_extint();
	display_update();
}

void re3_state::machine_start()
{
	dep5_base_state::machine_start();
	save_item(NAME(m_blink_phase));
	save_item(NAME(m_ticks));
	save_item(NAME(m_vfd_shift));
	save_item(NAME(m_vfd_bits));
	save_item(NAME(m_vfd_first));
	save_item(NAME(m_vfd_pending));
	save_item(NAME(m_text));
	save_item(NAME(m_marks));
	save_item(NAME(m_blinks));
}

void re3_state::machine_reset()
{
	dep5_base_state::machine_reset();
	m_vfd_shift = m_vfd_bits = m_vfd_first = 0;
	m_vfd_pending = false;
	m_text.fill(' ');
	m_marks.fill(false);
	m_blinks.fill(false);
	m_blink_phase = false;
	m_ticks = 0;
	display_update();
}

void dep5_base_state::sound_stream_update(sound_stream &stream)
{
	const float input = m_input->read() / 100.0f;
	// The RE-3 mixes MIC 1 and MIC 2 through separate level controls.
	const float input2 = input_gain2();
	const float balance = m_balance->read() / 100.0f;
	if (&stream == m_feed)
	{
		for (int i = 0; i < stream.samples(); ++i)
		{
			const float sample = (stream.get(0, i) * input + stream.get(1, i) * input2) * 0.5f;
			m_peak = std::max(m_peak, std::abs(sample));
			stream.put(0, i, sample);
		}
	}
	else
	{
		const bool muted = output_muted();
		for (int i = 0; i < stream.samples(); ++i)
		{
			for (int ch = 0; ch < 2; ++ch)
				stream.put(ch, i, muted ? 0 : stream.get(ch, i) * (1 - balance) + stream.get(ch + 2, i) * balance);
		}
	}
}

void dep5_base_state::rcv_complete()
{
	receive_register_extract();
	m_cpu->serial_w(get_received_char());
}

u8 dep5_state::scan_keys()
{
	u8 keys = 0;
	for (int bit = 0; bit < 7; ++bit)
	{
		if (BIT(m_select, bit))
			keys |= m_keys[6 - bit]->read();
	}
	return keys;
}

u8 re3_state::port1_r()
{
	u8 keys = 0;
	for (int bit = 0; bit < 8; ++bit)
	{
		if (BIT(m_select, bit))
			keys |= m_keys[bit]->read();
	}
	return keys ? 0x7f : 0xff;
}

void dep5_state::display_update()
{
	const u16 segments = (m_latch & 0x7f) | (((m_port1 ^ 0x30) & 0x3c) << 5);
	for (int i = 0; i < 7; ++i)
	{
		if (BIT(m_select, i))
			m_grid[i] = segments;
	}
}

void re3_state::display_update()
{
	for (int i = 0; i < 10; ++i)
	{
		const bool hidden = m_blinks[i] && m_blink_phase;
		m_char[i] = hidden ? ' ' : m_text[i];
		m_mark[i] = m_marks[i] && !hidden;
	}
}

void re3_state::display_tick()
{
	m_ticks = (m_ticks + 1) & 15;
	if (!(m_ticks & 7))
	{
		m_blink_phase = m_ticks == 8;
		display_update();
	}
}

TIMER_CALLBACK_MEMBER(dep5_base_state::panel_tick)
{
	display_tick();
	m_feed->update();
	m_meter = std::max(m_peak, m_meter * 0.8f);
	m_peak = 0;
	// The top LED lights 3 dB below converter full scale; the others follow
	// the dB legends printed beside them.
	const float db = 20.0f * std::log10(std::max(m_meter, 1e-6f));
	for (int i = 0; i < 6; ++i)
		m_level[i] = db >= meter_thresholds()[i];
}

void re3_state::vfd_w(u8 data)
{
	// RE-3 serial module: MSB-first bytes, reset P1.4, rising clock P1.3.
	if (BIT(data, 4) && !BIT(m_port1, 4))
	{
		m_text.fill(' ');
		m_marks.fill(false);
		m_blinks.fill(false);
		m_vfd_pending = false;
		m_vfd_bits = m_vfd_shift = 0;
	}
	if (!BIT(data, 4) && BIT(data, 3) && !BIT(m_port1, 3))
	{
		m_vfd_shift = (m_vfd_shift << 1) | BIT(data, 2);
		if (++m_vfd_bits == 8)
		{
			m_vfd_bits = 0;
			if (m_vfd_pending)
			{
				m_vfd_pending = false;
				if ((m_vfd_first & 0xf0) == 0xb0)
				{
					for (unsigned p = m_vfd_shift & 15; p <= (m_vfd_first & 15) && p < 10; ++p)
					{
						const int column = 9 - p;
						switch (m_vfd_shift >> 4)
						{
						case 0:
							m_blinks[column] = m_marks[column] = false;
							break;
						case 2:
							m_marks[column] = false;
							break;
						case 3:
							m_blinks[column] = true;
							break;
						}
					}
				}
				else if ((m_vfd_first & 15) < 10)
				{
					const int column = 9 - (m_vfd_first & 15);
					if (m_vfd_shift == 0xce || m_vfd_shift == 0xcf)
						m_marks[column] = m_vfd_shift == 0xce;
					else
						m_text[column] = m_vfd_shift;
				}
			}
			else if ((m_vfd_shift & 0xf0) == 0x80 || (m_vfd_shift & 0xf0) == 0xb0)
			{
				m_vfd_first = m_vfd_shift;
				m_vfd_pending = true;
			}
		}
	}
}

void dep5_state::port1_w(u8 data)
{
	m_mix->update();
	m_port1 = data;
	display_update();
	m_reverb->serial_w(BIT(data, 6), BIT(data, 7));
}

void re3_state::port1_w(u8 data)
{
	vfd_w(data);
	m_port1 = data;
	display_update();
	m_reverb->serial_w(BIT(data, 0), BIT(data, 1));
}

void dep5_state::update_extint()
{
	// KEY IN 2 is also EXTINT, independent of CPU port reads.
	m_cpu->set_input_line(i8x9x_device::EXTINT_LINE, BIT(scan_keys(), 1));
}

INPUT_CHANGED_MEMBER(dep5_state::key_changed)
{
	update_extint();
}

void dep5_state::select_w(u8 data)
{
	dep5_base_state::select_w(data);
	update_extint();
	display_update();
}

void dep5_base_state::latch_w(u8 data)
{
	m_latch = data;
	display_update();
}

void dep5_state::reverb_bus_w(offs_t signals, u32 data)
{
	// DEP-5 board wiring: RD4 enables the chorus return, RD1 selects its
	// converter phase, and RD0 clocks the send into the chorus delay RAM.
	// The frame model runs the chorus one period ahead; the early return is
	// the previous right channel, the late return is the new left channel.
	if (signals & mb87126_device::BUS_CAS)
		return;
	if (signals & mb87126_device::BUS_DRIVE)
	{
		if (BIT(signals, 0))
			m_chorus->patch_input(2, s16(data >> 8));
	}
	else if (BIT(signals, 4))
	{
		s16 sample = m_chorus_left;
		if (!BIT(signals, 1))
		{
			sample = m_chorus_right;
			m_chorus->process_frame(0, m_chorus_left, m_chorus_right);
		}
		// The chorus drives only the upper sixteen data pins. Its serial
		// conversion offset is measured as -1; the low byte holds its value.
		m_reverb->bus_w(u32(s32(sample) - 1) << 8, 0xffff00);
	}
}

void dep5_base_state::timer_w(offs_t offset, u8 data)
{
	m_timer[offset & 3] = data;
	const u32 clocks = 640 + (BIT(m_timer[3], 0) ? 0x10000 - (u16(m_timer[1]) << 8 | m_timer[0]) : 0);
	m_reverb->frame_clocks_w(clocks);
}

void dep5_base_state::common_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0xffe0, 0xffe7).w(FUNC(dep5_base_state::timer_w)).umask16(0x00ff);
	map(0xffe8, 0xffe8).lr8(NAME([this]() -> u8 { return m_latch; })).w(FUNC(dep5_base_state::latch_w));
}

void dep5_state::mem_map(address_map &map)
{
	common_map(map);
	map(0xc000, 0xdfff).ram().share("nvram");
	map(0xffea, 0xffea).w(FUNC(dep5_state::select_w));
	map(0xfff0, 0xfff0).w(FUNC(dep5_state::chorus_rate_w));
	map(0xfff2, 0xfff2).w(FUNC(dep5_state::chorus_depth_w));
	map(0xfff4, 0xfff4).w(FUNC(dep5_state::chorus_delay_w));
}

void re3_state::mem_map(address_map &map)
{
	common_map(map);
	map(0xc000, 0xcfff).ram().share("nvram");
	map(0xffea, 0xffea).w(FUNC(re3_state::select_w));
}

void dep5_base_state::common(machine_config &config)
{
	N8097BH(config, m_cpu, 12_MHz_XTAL);
	m_cpu->out_p2_cb().set(FUNC(dep5_base_state::port2_w));
	m_cpu->ach0_cb().set(FUNC(dep5_base_state::knob_r<0>));
	m_cpu->ach1_cb().set(FUNC(dep5_base_state::knob_r<1>));
	m_cpu->ach2_cb().set(FUNC(dep5_base_state::knob_r<2>));
	m_cpu->ach3_cb().set(FUNC(dep5_base_state::knob_r<3>));
	m_cpu->ach4_cb().set(FUNC(dep5_base_state::knob_r<4>));
	m_cpu->ach5_cb().set(FUNC(dep5_base_state::knob_r<5>));
	m_cpu->ach6_cb().set(FUNC(dep5_base_state::knob_r<6>));
	m_cpu->ach7_cb().set(FUNC(dep5_base_state::knob_r<7>));
	m_cpu->serial_tx_cb().set(FUNC(dep5_base_state::midi_tx));
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	auto &in(MIDI_PORT(config, "midiin"));
	midiin_slot(in);
	in.rxd_handler().set(FUNC(dep5_base_state::rx_w));
	MIDI_PORT(config, m_midiout);
	midiout_slot(*m_midiout);
	MB87126(config, m_reverb, 40.96_MHz_XTAL);
	auto &mic(MICROPHONE(config, "input", 2));
	mic.front();
	mic.add_route(0, *this, 1.0, 0);
	mic.add_route(1, *this, 1.0, 1);
	mic.add_route(0, *this, 1.0, 2);
	mic.add_route(1, *this, 1.0, 3);
	add_route(0, m_reverb, 1.0, 0);
	m_reverb->add_route(0, *this, 1.0, 4);
	m_reverb->add_route(1, *this, 1.0, 5);
	SPEAKER(config, "output", 2).front();
	add_route(1, "output", 1.0, 0);
	add_route(2, "output", 1.0, 1);
}

void dep5_state::dep5(machine_config &config)
{
	common(config);
	m_cpu->set_addrmap(AS_PROGRAM, &dep5_state::mem_map);
	m_cpu->in_p0_cb().set(FUNC(dep5_state::port0_r));
	m_cpu->in_p1_cb().set(FUNC(dep5_state::port1_r));
	m_cpu->in_p2_cb().set(FUNC(dep5_state::port2_r));
	m_cpu->out_p1_cb().set(FUNC(dep5_state::port1_w));
	MB654119(config, m_chorus, 40.96_MHz_XTAL / 16);
	m_reverb->bus_callback().set(FUNC(dep5_state::reverb_bus_w));
	config.set_default_layout(layout_roland_dep5);
}

void re3_state::re3(machine_config &config)
{
	common(config);
	m_cpu->set_addrmap(AS_PROGRAM, &re3_state::mem_map);
	m_cpu->in_p0_cb().set_constant(0);
	m_cpu->in_p1_cb().set(FUNC(re3_state::port1_r));
	m_cpu->in_p2_cb().set(FUNC(re3_state::port2_r));
	m_cpu->out_p1_cb().set(FUNC(re3_state::port1_w));
	config.set_default_layout(layout_roland_re3);
}

INPUT_PORTS_START(dep5)
	PORT_START("KEY0")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("KEY1")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("KEY2")
	PORT_BIT(1, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Memory +") PORT_CODE(KEYCODE_Q)
	PORT_BIT(2, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Memory -") PORT_CODE(KEYCODE_W) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dep5_state::key_changed), 0)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("KEY3")
	PORT_BIT(1, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Reverb +") PORT_CODE(KEYCODE_E)
	PORT_BIT(2, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Reverb -") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dep5_state::key_changed), 0)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("KEY4")
	PORT_BIT(1, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Write") PORT_CODE(KEYCODE_T)
	PORT_BIT(2, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Chorus / EQ") PORT_CODE(KEYCODE_Y) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dep5_state::key_changed), 0)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("KEY5")
	PORT_BIT(1, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Output Level") PORT_CODE(KEYCODE_U)
	PORT_BIT(2, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Parametric EQ") PORT_CODE(KEYCODE_I) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dep5_state::key_changed), 0)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("KEY6")
	PORT_BIT(1, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MIDI") PORT_CODE(KEYCODE_O)
	PORT_BIT(2, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Value") PORT_CODE(KEYCODE_P) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dep5_state::key_changed), 0)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("KEY7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("KNOB0")
	PORT_ADJUSTER(0, "Feedback / EQ Low")
	PORT_START("KNOB1")
	PORT_ADJUSTER(0, "Rate / Parametric EQ")
	PORT_START("KNOB2")
	PORT_ADJUSTER(0, "Depth / EQ High")
	PORT_START("KNOB3")
	PORT_ADJUSTER(0, "Algorithm Select")
	PORT_START("KNOB4")
	PORT_ADJUSTER(0, "Pre-delay")
	PORT_START("KNOB5")
	PORT_ADJUSTER(0, "Reverb Time")
	PORT_START("KNOB6")
	PORT_ADJUSTER(0, "HF Damp / Gate Time")
	PORT_START("KNOB7")
	PORT_BIT(0xff, 0, IPT_UNUSED)
	PORT_START("INPUT")
	PORT_ADJUSTER(100, "Input Attenuator")
	PORT_START("BALANCE")
	PORT_ADJUSTER(100, "Balance")
	PORT_START("MONO")
	PORT_CONFNAME(2, 0, "Output Connection")
	PORT_CONFSETTING(0, "Stereo")
	PORT_CONFSETTING(2, "Mono")
	PORT_START("PEDALS")
	PORT_BIT(8, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Preset Shift Pedal") PORT_CODE(KEYCODE_Z)
	PORT_BIT(16, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Effect Off Pedal") PORT_CODE(KEYCODE_X)
INPUT_PORTS_END
INPUT_PORTS_START(re3)
	PORT_INCLUDE(dep5)
	PORT_MODIFY("KEY0")
	PORT_BIT(1, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Mode") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_MODIFY("KEY1")
	PORT_BIT(1, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Memory") PORT_CODE(KEYCODE_W)
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_MODIFY("KEY2")
	PORT_BIT(1, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Write") PORT_CODE(KEYCODE_E)
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_MODIFY("KEY3")
	PORT_BIT(1, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MIDI") PORT_CODE(KEYCODE_R)
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_MODIFY("KEY4")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_MODIFY("KEY5")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_MODIFY("KEY6")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_MODIFY("KEY7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_MODIFY("KNOB0")
	PORT_ADJUSTER(0, "Repeat Rate")
	PORT_MODIFY("KNOB1")
	PORT_ADJUSTER(0, "Intensity")
	PORT_MODIFY("KNOB2")
	PORT_ADJUSTER(0, "Reverb")
	PORT_MODIFY("KNOB3")
	PORT_ADJUSTER(100, "Battery Voltage")
	PORT_MODIFY("KNOB4")
	PORT_BIT(0xff, 0, IPT_UNUSED)
	PORT_MODIFY("KNOB5")
	PORT_ADJUSTER(0, "Echo Level")
	PORT_MODIFY("KNOB6")
	PORT_ADJUSTER(0, "Warmth")
	PORT_MODIFY("KNOB7")
	PORT_BIT(0xff, 0, IPT_UNUSED) // M-RATE, unconnected
	PORT_MODIFY("INPUT")
	PORT_ADJUSTER(100, "Input Level 1")
	PORT_START("INPUT2")
	PORT_ADJUSTER(100, "Input Level 2")
INPUT_PORTS_END

ROM_START(dep5)
	ROM_REGION16_LE(0x8000, "maincpu", 0)
	// Program A (IC33): D0-D7; program B (IC30): D8-D15.
	ROM_LOAD16_BYTE("dep-5_1.6_a.ic33", 0, 0x4000, CRC(cda5a72a) SHA1(62e6a81adc8b53911190ae083723896dad97f31a))
	ROM_LOAD16_BYTE("dep-5_1.6_b.ic30", 1, 0x4000, CRC(18fae1b1) SHA1(520899eea0285e2ea7bb465c378d64a27e38fe31))
	ROM_REGION(160 * 5, "reverb:program", 0)
	// Furrtek die-shot extraction. Difficult cells were corrected manually
	// by probing chip behavior; retained as BAD_DUMP pending a verified read.
	ROM_LOAD("mb87126-002.bin", 0, 160 * 5, BAD_DUMP CRC(7f5232a4) SHA1(bcfc8fb6bd541255ebbfc2d62d2930f6c442afb1))
ROM_END
ROM_START(re3)
	ROM_REGION16_LE(0x8000, "maincpu", 0)
	ROM_LOAD16_BYTE("re-3_1.0.0_a.bin", 0, 0x4000, CRC(31b8e98d) SHA1(034408030f1426c8d4db6070d4e1fe090bd39aef))
	ROM_LOAD16_BYTE("re-3_1.0.0_b.bin", 1, 0x4000, CRC(543a1055) SHA1(0613e1a63dffde804762289dc2ed32f2bffdc22a))
	ROM_REGION(160 * 5, "reverb:program", 0)
	// The MB87126A-007 program is assumed to match the MB87126-002, unverified.
	// Furrtek die-shot extraction. Difficult cells were corrected manually
	// by probing chip behavior; retained as BAD_DUMP pending a verified read.
	ROM_LOAD("mb87126a-007.bin", 0, 160 * 5, BAD_DUMP CRC(7f5232a4) SHA1(bcfc8fb6bd541255ebbfc2d62d2930f6c442afb1))
ROM_END

} // anonymous namespace

SYST(1986, dep5, 0, 0, dep5, dep5, dep5_state, empty_init, "Roland", "DEP-5 Digital Effects Processor", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
SYST(1988, re3, 0, 0, re3, re3, re3_state, empty_init, "Roland", "RE-3 Digital Space Echo", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
