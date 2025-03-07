// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
The Oberheim DMX is a digital drum machine.

The firmware, running on a Z80, implements the drum machine functionality (e.g.
sequencer), controls the UI (scans for button presses, drives the display),
triggers voices, and controls voice variations. All hardware functionality is
mapped to I/O ports. Only ROM and RAM are mapped to memory.

There are 8 voice cards, each of which can produce 3 variations, for a total of
24 sounds. The 8 voice cards can play back simultaneously, but only 1 variation
per card can be active at a time, for a total of 8 voices of polyphony.

The drum sounds are samples stored in ROMs. During playback, they are
post-processed by an analog VCA, which is modulated by a Release envelope
genereator. Pitch and volume variations for each voice are also controlled by
analog circuitry.

With the exception of the Cymbal voice, the bare voice PCBs are identical.
Other than having different ROMs, the voice cards are further configured by
multiple jumpers and differing component values. This configuration controls
nominal pitch, variation type (pitch, volume, decay, sample), and the
frequency response of the reconstruction filter.

The Cymbal voice consists of 2 voice cards. One implements most of the circuit
found in the other voice cards, while the other holds the 8 2732 ROMS.

Currently, this driver emulates the early version of the DMX. A later hardware
revision added additional memory, and the final hardware revision added MIDI.
Furthermore, later versions shipped with the "Mark II" voice cards.
These subsequent hardware revisions and the Mark II voice cards are not yet
emulated.

The driver is based on the DMX's service manual, DMX schematics, and
http://www.electrongate.com/dmxfiles/dmxcards.html. It is intended as an
educational tool.

PCBoards:
- Processor Board (includes the power supply and connectors)
- Switch Board (buttons, faders and display)
- 7 x voice cards: Bass, Snare, Hi-hat, Tom 1, Tom 2, Perc 1, Perc 2.
- 2 x cymbal cards: A single voice that occupies two card slots.

Possible audio inaccuracies:
- Some uncertainty on component values for HIHAT and PERC2 (see comments in
  HIHAT_CONFIG and PERC_CONFIG).
- Linear- instead of audio-taper faders.
- Envelope decay ignores diodes in capacitor discharge path. Given the quick
  decay, and that the error is larger at low volumes, this might not be
  noticeable.
- Simplified diode model in pitch control. Very unlikely to be noticeable.
  Error is within component tolerance margins.
- Simplified diode model in volume control. Very unlikely to be noticeable.
  Error is within component tolerance margins. It is very small at high volumes,
  and approaches component tolerance extremes at low volumes.

Usage notes:
- Interactive layout included.
- The mixer faders can be controlled with the mouse, or from the "Slider
  Controls" menu.
- Voices can be tuned with the mouse, or the "Sider Controls" menu.
- The drum keys are mapped to the keyboard, starting at "Q". Specifically:
  Q - Bass 1, W - Snare 1, ...
  A - Bass 2, S - Snare 2, ...
  Z - Bass 3, X - Snare 3, ...
- The number buttons on the layout are mapped to the numeric keypad.
- Can choose between the stereo (hardcoded voice panning) and mono outputs
  from the Machine Configuration menu.
- Can run with a high sample rate: ./mame -window obdmx -samplerate 96000
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/output_latch.h"
#include "machine/rescap.h"
#include "machine/timer.h"
#include "sound/dac76.h"
#include "sound/flt_biquad.h"
#include "sound/flt_rc.h"
#include "sound/mixer.h"
#include "sound/spkrdev.h"
#include "video/dl1416.h"
#include "speaker.h"

#include "oberheim_dmx.lh"

#define LOG_TRIGGERS      (1U << 1)
#define LOG_INT_TIMER     (1U << 2)
#define LOG_FADERS        (1U << 3)
#define LOG_SOUND         (1U << 4)
#define LOG_PITCH         (1U << 5)
#define LOG_VOLUME        (1U << 6)
#define LOG_SAMPLES       (1U << 7)
#define LOG_SAMPLES_DECAY (1U << 8)
#define LOG_METRONOME     (1U << 9)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

static constexpr const float VCC = 5;

// Voice card configuration. Captures jumper configuration and component values.
struct dmx_voice_card_config
{
	const float c2;  // Pitch control capacitor. In Farads.
	const float c3;  // EG capacitor. In Farads. 0 if unused.

	// R12 and R17 control how the different trigger modes affect pitch (for
	// voices wired for pitch control), or volume and decay (for voices wired
	// for volume control).
	// The values of R12 and R17 in the 1981 service manual are flipped compared
	// to the values in the 1983 schematics and in
	// http://www.electrongate.com/dmxfiles/dmxcards.html. The values in the
	// 1983 schematic (and electrongate.com) are most probably the right ones,
	// since they create more variation, especially for volume. Using those.
	const float r12;
	const float r17;

	// ROM contains 2 different samples.
	const bool split_rom;

	// Contains 8 ROMs instead of 1. Used for Cymbal voice cards.
	const bool multi_rom;

	// The triggers control pitch variations (rather than volume variations).
	const bool pitch_control;

	enum class decay_mode : s8  // Controlled by jumper Z on voice card.
	{
		DISABLED,  // 4.7K resistor connected to position 1 (+5V).
		ENABLED,  // Jumper disconnected.
		ENABLED_ON_TR12,  // 4.7K resistor connected to position 2 (/Q of U1B).
						  // Decay enabled when trigger mode is 1 or 2.
	};
	const decay_mode decay;

	// This jumper won't make a difference when `decay` is `DISABLED`.
	enum class early_decay_mode : s8  // Controlled by jumper S on voice card.
	{
		ENABLED,  // Jumper connected to position 2 (+5V).
		ENABLED_ON_TR1,  // Jumper connected to position 1 (Q of U1A).
						 // Early decay enabled when trigger mode is 1.
	};
	const early_decay_mode early_decay;

	// A cascade of 3 Sallen-Key filters. Components ordered from left to right
	// on the schematic, which also matches the order in opamp_sk_lowpass_setup.
	struct filter_components
	{
		const float r15;
		const float r14;
		const float c5;
		const float c4;

		const float r13;
		const float r18;
		const float c6;
		const float c7;

		const float r19;
		const float r20;
		const float c8;
		const float c9;
	};
	const filter_components filter;
};

// The combination of the gain control circuit (which includes decay for some
// voices), and the multiplying DAC form a VCA. The gain control circuit sets
// the reference current into the DAC, and the DAC multiplies that with the
// digital value to produce and output current.
class dmx_voice_card_vca_device : public device_t, public device_sound_interface
{
public:
	dmx_voice_card_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, const dmx_voice_card_config &config) ATTR_COLD;
	dmx_voice_card_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	void start(int trigger_mode);
	void decay();

	bool in_decay() const { return m_decaying; }

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream, const std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs) override;

private:
	void init_gain_and_decay_variations(const dmx_voice_card_config &config) ATTR_COLD;

	bool has_decay() const { return !m_decay_rc_inv.empty(); }
	bool has_decay_variations() const { return m_decay_rc_inv.size() > 1; }

	// Configuration. Do not include in save state.
	sound_stream *m_stream = nullptr;
	const bool m_gain_control;  // Is VCA configured for gain variations?
	std::vector<float> m_gain;  // Gain variations.
	std::vector<float> m_decay_rc_inv;  // Decay 1/RC variations.

	// Device state.
	float m_selected_gain = 1;
	bool m_decaying = false;
	bool m_decay_done = false;
	float m_selected_rc_inv = 1;
	attotime m_decay_start_time;
};

DEFINE_DEVICE_TYPE(DMX_VOICE_CARD_VCA, dmx_voice_card_vca_device, "dmx_voice_card_vca", "DMX Voice Card VCA");

dmx_voice_card_vca_device::dmx_voice_card_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, const dmx_voice_card_config &config)
	: device_t(mconfig, DMX_VOICE_CARD_VCA, tag, owner, 0)
	, device_sound_interface(mconfig, *this)
	, m_gain_control(!config.pitch_control)
{
	init_gain_and_decay_variations(config);
}

dmx_voice_card_vca_device::dmx_voice_card_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DMX_VOICE_CARD_VCA, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_gain_control(false)
{
}

void dmx_voice_card_vca_device::start(int trigger_mode)
{
	assert(trigger_mode >= 1 && trigger_mode <= 3);

	m_stream->update();
	m_decaying = false;
	m_decay_done = false;

	if (m_gain_control)
		m_selected_gain = m_gain[trigger_mode];
	else
		m_selected_gain = m_gain[0];

	if (has_decay_variations())
		m_selected_rc_inv = m_decay_rc_inv[trigger_mode];
	else if (has_decay())
		m_selected_rc_inv = m_decay_rc_inv[0];
	else
		m_selected_rc_inv = 1;

	LOGMASKED(LOG_VOLUME, "Selected gain: %f, 1/RC: %f\n",
			m_selected_gain, m_selected_rc_inv);
}

void dmx_voice_card_vca_device::decay()
{
	assert(has_decay());
	if (!has_decay())
		return;

	m_stream->update();
	m_decaying = true;
	m_decay_start_time = machine().time();
}

void dmx_voice_card_vca_device::device_start()
{
	m_stream = stream_alloc(1, 1, machine().sample_rate());

	save_item(NAME(m_selected_gain));
	save_item(NAME(m_decaying));
	save_item(NAME(m_decay_done));
	save_item(NAME(m_selected_rc_inv));
	save_item(NAME(m_decay_start_time));
}

void dmx_voice_card_vca_device::device_reset()
{
	m_selected_gain = 1;
	m_decaying = false;
	m_decay_done = false;
	m_selected_rc_inv = 1;
}

void dmx_voice_card_vca_device::sound_stream_update(sound_stream &stream, const std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs)
{
	// Gain lower than MIN_GAIN will be treated as 0.
	static constexpr const float MIN_GAIN = 0.0001F;

	const read_stream_view &in = inputs[0];
	write_stream_view &out = outputs[0];
	const int n = in.samples();

	if (!m_decaying)  // Just gain variation without decay.
	{
		for (int i = 0; i < n; ++i)
			out.put(i, m_selected_gain * in.get(i));

		LOGMASKED(LOG_SAMPLES, "%s VCA - just gain: %f. Samples: %f, %f.\n",
				tag(), m_selected_gain, in.get(0), in.get(n - 1));
		return;
	}

	if (m_decay_done)  // Avoid expensive expf() call if volume has decayed.
	{
		out.fill(0);
		LOGMASKED(LOG_SAMPLES, "%s VCA - decay done.\n", tag());
		return;
	}

	attotime t = in.start_time() - m_decay_start_time;
	assert(!m_decaying || t >= attotime::from_double(0));

	float gain = 1;
	for (int i = 0; i < n; ++i, t += in.sample_period())
	{
		const float decay = expf(-t.as_double() * m_selected_rc_inv);
		gain = decay * m_selected_gain;
		out.put(i, gain * in.get(i));
	}

	if (gain < MIN_GAIN)
		m_decay_done = true;

	LOGMASKED(LOG_SAMPLES_DECAY, "%s VCA - in decay: %f. Samples: %f, %f.\n",
			tag(), gain, in.get(0), in.get(n - 1));
}

void dmx_voice_card_vca_device::init_gain_and_decay_variations(const dmx_voice_card_config &config)
{
	static constexpr const float VD = 0.6;  // Diode drop.
	static constexpr const float R8 = RES_K(2.7);
	static constexpr const float R9 = RES_K(5.6);
	static constexpr const float MAX_IREF = VCC / (R8 + R9);

	const float r12 = config.r12;
	const float r17 = config.r17;
	const float c3 = config.c3;

	// Precompute gain variations.
	m_gain.clear();
	m_gain.push_back(MAX_IREF);
	if (m_gain_control)  // Configured for gain variations.
	{
		// The equations below were derived from Kirchhoff analysis and verified
		// with simulations: https://tinyurl.com/22wxwh8h

		// For trigger mode 1.
		m_gain.push_back((r12 * r17 * VCC + R8 * r12 * VD + R8 * r17 * VD) /
						 ((R8 * r12 * r17) + (r12 * r17 * R9) + (R8 * r17 * R9) + (R8 * r12 * R9)));
		// For trigger mode 2.
		m_gain.push_back((r12 * VCC + R8 * VD) / (r12 * R8 + R8 * R9 + r12 * R9));
		// For trigger mode 3.
		m_gain.push_back(m_gain[0]);
	}
	for (int i = 0; i < m_gain.size(); ++i)
	{
		LOGMASKED(LOG_VOLUME, "%s: Gain variation %d: %f uA, %f\n",
				tag(), i, m_gain[i] * 1e6F, m_gain[i] / MAX_IREF);
		m_gain[i] /= MAX_IREF;  // Normalize.
	}

	// Precompute decay variations.
	m_decay_rc_inv.clear();
	if (config.decay != dmx_voice_card_config::decay_mode::DISABLED && c3 > 0)
	{
		std::vector<float> r_lower;
		r_lower.push_back(R9);  // For when there are no variations.
		if (m_gain_control)
		{
			r_lower.push_back(RES_3_PARALLEL(R9, r12, r17));  // For trigger mode 1.
			r_lower.push_back(RES_2_PARALLEL(R9, r12)); // For trigger mode 2.
			r_lower.push_back(R9);  // For trigger mode 3.
		}
		for (float r : r_lower)
		{
			m_decay_rc_inv.push_back(1.0F / ((R8 + r) * c3));
			LOGMASKED(LOG_VOLUME, "%s: Decay 1/RC variation %d: %f\n",
					tag(), m_decay_rc_inv.size() - 1, m_decay_rc_inv.back());
		}
	}
}

// Emulates the original DMX voice cards, including the cymbal card. Later
// DMX models shipped with the "Mark II" voice cards for the Tom voices.
// The Mark II cards are not yet emulated.
class dmx_voice_card_device : public device_t, public device_sound_interface
{
public:
	// Default value of pitch adjustment trimpot.
	static constexpr const s32 T1_DEFAULT_PERCENT = 50;

	dmx_voice_card_device(const machine_config &mconfig, const char *tag, device_t *owner, const dmx_voice_card_config &config, required_memory_region *sample_rom) ATTR_COLD;
	dmx_voice_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	void trigger(bool tr0, bool tr1);
	void set_pitch_adj(s32 t1_percent);  // Valid values: 0-100.

protected:
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream, const std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs) override;

private:
	void reset_counter();
	void init_pitch() ATTR_COLD;
	void compute_pitch_variations();
	void select_pitch();

	bool is_decay_enabled() const;
	bool is_early_decay_enabled() const;
	TIMER_DEVICE_CALLBACK_MEMBER(clock_callback);

	sound_stream *m_stream = nullptr;

	required_device<timer_device> m_timer;  // 555, U5.
	required_device<dac76_device> m_dac;  // AM6070, U8. Compatible with DAC76.
	required_device<dmx_voice_card_vca_device> m_vca;
	required_device_array<filter_biquad_device, 3> m_filters;

	// Configuration. Do not include in save state.
	const dmx_voice_card_config m_config;
	required_memory_region *m_sample_rom = nullptr;
	std::vector<float> m_cv;  // 555 CV (pin 5) voltage variations.
	std::vector<attotime> m_sample_t;  // Sample period variations.
	s32 m_t1_percent = T1_DEFAULT_PERCENT;

	// Device state.
	bool m_counting = false;
	u16 m_counter = 0;  // 4040 counter.
	u8 m_trigger_mode = 0;  // Valid modes: 1-3. 0 OK after reset.
};

DEFINE_DEVICE_TYPE(DMX_VOICE_CARD, dmx_voice_card_device, "dmx_voice_card", "DMX Voice Card");

dmx_voice_card_device::dmx_voice_card_device(const machine_config &mconfig, const char *tag, device_t *owner, const dmx_voice_card_config &config, required_memory_region *sample_rom)
	: device_t(mconfig, DMX_VOICE_CARD, tag, owner, 0)
	, device_sound_interface(mconfig, *this)
	, m_timer(*this, "555_u5")
	, m_dac(*this, "dac_u8")
	, m_vca(*this, "dmx_vca")
	, m_filters(*this, "aa_sk_filter_%d", 0)
	, m_config(config)
	, m_sample_rom(sample_rom)
{
	init_pitch();
}

dmx_voice_card_device::dmx_voice_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DMX_VOICE_CARD, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_timer(*this, "555_u5")
	, m_dac(*this, "dac_u8")
	, m_vca(*this, "dmx_vca")
	, m_filters(*this, "aa_sk_filter_%d", 0)
	// Need non-zero entries for the filter for validation to pass.
	, m_config(dmx_voice_card_config{.filter={1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}})
{
}

void dmx_voice_card_device::trigger(bool tr0, bool tr1)
{
	assert(tr0 || tr1);
	if (tr1 && tr0)
		m_trigger_mode = 3;
	else if (tr1)
		m_trigger_mode = 2;
	else if (tr0)
		m_trigger_mode = 1;
	else
		return

	m_stream->update();
	m_counter = 0;
	m_counting = true;
	if (m_config.pitch_control)
		select_pitch();
	m_vca->start(m_trigger_mode);

	LOGMASKED(LOG_SOUND, "Trigger: (%d, %d) %d %f\n", tr0, tr1, m_trigger_mode);
}

void dmx_voice_card_device::set_pitch_adj(s32 t1_percent)
{
	m_stream->update();
	m_t1_percent = t1_percent;
	compute_pitch_variations();
}

void dmx_voice_card_device::device_add_mconfig(machine_config &config)
{
	static constexpr const double SK_R3 = RES_M(999.99);
	static constexpr const double SK_R4 = RES_R(0.001);

	TIMER(config, m_timer).configure_generic(FUNC(dmx_voice_card_device::clock_callback));
	DAC76(config, m_dac, 0U);
	DMX_VOICE_CARD_VCA(config, m_vca, m_config);

	FILTER_BIQUAD(config, m_filters[0]).opamp_sk_lowpass_setup(
		m_config.filter.r15, m_config.filter.r14, SK_R3, SK_R4,
		m_config.filter.c5, m_config.filter.c4);
	FILTER_BIQUAD(config, m_filters[1]).opamp_sk_lowpass_setup(
		m_config.filter.r13, m_config.filter.r18, SK_R3, SK_R4,
		m_config.filter.c6, m_config.filter.c7);
	FILTER_BIQUAD(config, m_filters[2]).opamp_sk_lowpass_setup(
		m_config.filter.r19, m_config.filter.r20, SK_R3, SK_R4,
		m_config.filter.c8, m_config.filter.c9);

	m_dac->add_route(ALL_OUTPUTS, m_vca, 1.0);
	m_vca->add_route(ALL_OUTPUTS, m_filters[0], 1.0);

	m_filters[0]->add_route(ALL_OUTPUTS, m_filters[1], 1.0);
	m_filters[1]->add_route(ALL_OUTPUTS, m_filters[2], 1.0);
	m_filters[2]->add_route(ALL_OUTPUTS, *this, 1.0);
}

void dmx_voice_card_device::device_start()
{
	m_stream = stream_alloc(1, 1, machine().sample_rate());

	save_item(NAME(m_counting));
	save_item(NAME(m_counter));
	save_item(NAME(m_trigger_mode));
}

void dmx_voice_card_device::device_reset()
{
	m_trigger_mode = 0;
	reset_counter();
	compute_pitch_variations();
}

void dmx_voice_card_device::sound_stream_update(sound_stream &stream, const std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs)
{
	outputs[0] = inputs[0];
}

void dmx_voice_card_device::reset_counter()
{
	m_counter = 0;
	m_counting = false;
}

void dmx_voice_card_device::init_pitch()
{
	// Precompute all variations of CV (pin 5 of 555 timer).

	// The CV equations were derived from Kirchhoff analysis and verified with
	// simulations: https://tinyurl.com/26x8oq75

	// Diode drop for pitch circuit. The value is based on simulations with the
	// various pitch configurations, instead of picking the typical 0.6V. This
	// should help with accuracy.
	static constexpr const float VD = 0.48F;
	static constexpr const float R_555 = RES_K(5);
	static constexpr const float R5 = RES_K(3.3);

	m_cv.clear();
	m_cv.push_back(VCC * 2 / 3); // The 555 default, if pin 5 is floating.

	if (m_config.pitch_control)
	{
		const float r12 = m_config.r12;

		// For trigger mode 1.
		const float alpha = 1.0F + r12 / m_config.r17;
		m_cv.push_back((alpha * R5 + r12) * (2 * VCC - 3 * VD) /
					   (3 * alpha * R5 + 3 * r12 + 2 * alpha * R_555) + VD);

		// For trigger mode 2.
		m_cv.push_back((R5 + r12) * (2 * VCC - 3 * VD) /
					   (3 * R5 + 3 * r12 + 2 * R_555) + VD);

		// For trigger mode 3.
		m_cv.push_back(m_cv[0]);
	}

	for (int i = 0; i < m_cv.size(); ++i)
		LOGMASKED(LOG_PITCH, "%s 555 CV %d: %f\n", tag(), i, m_cv[i]);

	// m_sample_t will be populated by subsequent calls to configure_pitch().
	m_sample_t.clear();
	m_sample_t.resize(m_cv.size());
}

void dmx_voice_card_device::compute_pitch_variations()
{
	static constexpr const float R3 = RES_K(1);
	static constexpr const float R4 = RES_K(10);
	static constexpr const float T1_MAX = RES_K(10);

	// The baseline pitch (and sampling rate) for all voice cards is controlled
	// by a 555 timer (U5). Users can adjust the pitch with a trimpot (T1).

	// For voice cards configured for pitch control (m_config.pitch_control is
	// true), pitch variations are accomplished by changing the Control Voltage
	// (pin 5) of the 555 (see init_pitch()).

	// Computing the timer period is a bit involved, because of the use of CV,
	// and because the 555 is not configured in the typical astable mode.
	// For an RC circuit, V(t) = Vstart + (Vend - Vstart) * (1 - exp(-t / RC)).
	// Solving for t: t = -RC * log( (Vend - V) / (Vend - Vstart) ).
	// Let t_high be the time interval for which the 555 output is high. This is
	// the time it takes for the capacitor to charge from CV/2 to CV.
	// Let t_low be the time interval for which the 555 output is low. This is
	// the time it takes for the capacitor to discharge from CV to CV/2.
	// The timer period is then: t_high + t_low. t_* can be computed by
	// substituting appropriate values in the function for `t`, keeping in mind
	// that RC, Vstart, Vend and V are different for charging and discharging.

	// Compute RC time constant for charging.
	assert(m_t1_percent >= 0 && m_t1_percent <= 100);
	const float r_charge = m_t1_percent * T1_MAX / 100 + R4;
	const float rc_charge = r_charge * m_config.c2;

	// Compute Vend and RC time constant for discharging, taking into account
	// the atypical 555 configuration.
	const float rc_discharge = RES_2_PARALLEL(r_charge, R3) * m_config.c2;
	const float ve_discharge = VCC * RES_VOLTAGE_DIVIDER(r_charge, R3);

	// Optimization: when m_config.pitch_control is true,
	// m_sample_t[0] = m_sample_t[3]. So skip index 0 and copy after the loop.
	const int start_i = m_config.pitch_control ? 1 : 0;

	for (int i = start_i; i < m_sample_t.size(); ++i)
	{
		const float cv = m_cv[i];
		const float half_cv = 0.5F * cv;

		// Time for C2 to charge from CV/2 (Vstart) to CV (V). Vend = VCC
		const float t_high = -rc_charge * logf((VCC - cv) / (VCC - half_cv));
		assert(t_high > 0);

		// Time for C2 to discharge from CV (Vstart) to CV/2 (V).
		const float t_low = -rc_discharge * logf((ve_discharge - half_cv) / (ve_discharge - cv));
		assert(t_low > 0);

		m_sample_t[i] = attotime::from_double(t_high + t_low);
		LOGMASKED(LOG_PITCH, "%s Pitch variation %d: %f (%f, %f)\n",
				tag(), i, 1.0 / m_sample_t[i].as_double(), t_high, t_low);
	}

	if (m_config.pitch_control)
		m_sample_t[0] = m_sample_t[3];

	select_pitch();
}

void dmx_voice_card_device::select_pitch()
{
	attotime sampling_t;
	if (m_config.pitch_control)
		sampling_t = m_sample_t[m_trigger_mode];
	else
		sampling_t = m_sample_t[0];

	if (sampling_t == m_timer->period())
		return;  // Avoid resetting the timer in this case.

	m_timer->adjust(sampling_t, 0, sampling_t);
	LOGMASKED(LOG_PITCH, "Setting sampling frequency: %f\n",
			1.0 / sampling_t.as_double());
}

bool dmx_voice_card_device::is_decay_enabled() const
{
	switch (m_config.decay)
	{
		case dmx_voice_card_config::decay_mode::ENABLED:
			return true;
		case dmx_voice_card_config::decay_mode::ENABLED_ON_TR12:
			return m_trigger_mode == 1 || m_trigger_mode == 2;
		case dmx_voice_card_config::decay_mode::DISABLED:
			return false;
	}
	return false;
}

bool dmx_voice_card_device::is_early_decay_enabled() const
{
	switch (m_config.early_decay)
	{
		case dmx_voice_card_config::early_decay_mode::ENABLED:
			return true;
		case dmx_voice_card_config::early_decay_mode::ENABLED_ON_TR1:
			return m_trigger_mode == 1;
	}
	return false;
}

TIMER_DEVICE_CALLBACK_MEMBER(dmx_voice_card_device::clock_callback)
{
	if (!m_counting)
		return;

	++m_counter;

	const u16 rom_size = m_config.multi_rom ? 8 * 0x1000 : 0x1000;
	const u16 max_count = m_config.split_rom ? rom_size / 2 : rom_size;
	if (m_counter >= max_count)
	{
		reset_counter();
		LOGMASKED(LOG_SOUND, "Done counting %d\n\n", m_config.split_rom);
	}

	const u16 offset = (m_config.split_rom && m_trigger_mode != 3) ? max_count : 0;
	const u8 sample = (*m_sample_rom)->as_u8(m_counter + offset);
	m_dac->update();
	m_dac->sb_w(BIT(sample, 7));
	m_dac->b1_w(BIT(sample, 6));
	m_dac->b2_w(BIT(sample, 5));
	m_dac->b3_w(BIT(sample, 4));
	m_dac->b4_w(BIT(sample, 3));
	m_dac->b5_w(BIT(sample, 2));
	m_dac->b6_w(BIT(sample, 1));
	m_dac->b7_w(BIT(sample, 0));

	// Early decay starts when the counter's bit 6 transitions to 1.
	static constexpr const u16 EARLY_DECAY_START = 1 << 6;
	// If early decay is not enabled, but decay is enabled, it will start when
	// the counter's bit 10 transitions to 1.
	static constexpr const u16 LATE_DECAY_START = 1 << 10;

	if (!m_vca->in_decay() && is_decay_enabled())
	{
		if ((is_early_decay_enabled() && m_counter >= EARLY_DECAY_START) ||
			m_counter >= LATE_DECAY_START)
		{
			m_vca->decay();
		}
	}
}

namespace {

constexpr const char MAINCPU_TAG[] = "z80";
constexpr const char NVRAM_TAG[] = "nvram";

// There are two anti-aliasing / reconstruction filter setups used. One for
// lower- and one for higher-frequency voices.

// ~10Khz cuttoff.
constexpr const dmx_voice_card_config::filter_components FILTER_CONFIG_LOW =
{
	.r15 = RES_K(6.8),
	.r14 = RES_K(6.2),
	.c5 = CAP_U(0.01),
	.c4 = CAP_U(0.0047),

	.r13 = RES_K(9.1),
	.r18 = RES_K(9.1),
	.c6 = CAP_U(0.01),
	.c7 = CAP_P(560),

	.r19 = RES_K(5.1),
	.r20 = RES_K(5.1),
	.c8 = CAP_U(0.047),
	.c9 = CAP_P(200),
};

// ~16 KHz cuttoff.
constexpr const dmx_voice_card_config::filter_components FILTER_CONFIG_HIGH =
{
	.r15 = RES_K(4.7),
	.r14 = RES_K(4.7),
	.c5 = CAP_U(0.01),
	.c4 = CAP_U(0.0047),

	.r13 = RES_K(5.6),
	.r18 = RES_K(5.6),
	.c6 = CAP_U(0.01),
	.c7 = CAP_P(560),

	.r19 = RES_K(3.3),
	.r20 = RES_K(3.3),
	.c8 = CAP_U(0.047),
	.c9 = CAP_P(200),
};

// Voice card configurations below reverse R12 and R17 compared to the 1981
// schematics. See comments in dmx_voice_card_config for more on this.

constexpr const dmx_voice_card_config BASS_CONFIG =
{
	.c2 = CAP_U(0.0033),
	.c3 = CAP_U(3.3),
	.r12 = RES_R(750),
	.r17 = RES_R(47),
	.split_rom = false,
	.multi_rom = false,
	.pitch_control = false,
	.decay = dmx_voice_card_config::decay_mode::ENABLED,
	.early_decay = dmx_voice_card_config::early_decay_mode::ENABLED,
	.filter = FILTER_CONFIG_LOW,
};

constexpr const dmx_voice_card_config SNARE_CONFIG =
{
	.c2 = CAP_U(0.0022),
	.c3 = 0,
	.r12 = RES_K(1.5),
	.r17 = RES_R(15),
	.split_rom = false,
	.multi_rom = false,
	.pitch_control = false,
	.decay = dmx_voice_card_config::decay_mode::DISABLED,
	.early_decay = dmx_voice_card_config::early_decay_mode::ENABLED_ON_TR1,
	.filter = FILTER_CONFIG_HIGH,
};

// The 1981 schematic shows 6.8uF for c3. electrongate.com reports 2.2uF. The
// service manual states that c3 can be changed, and offers a useful range of
// 2uF - 10uF.
// Similarly, resistor values in electrongate.com disagree with schematics.
// - electrongate.com: R12 = 6.8K, R17 not installed.
// - schematics: R12 = 1.5K, R17 = 15.
// Using the electrongate values below.
constexpr const dmx_voice_card_config HIHAT_CONFIG =
{
	.c2 = CAP_U(0.0033),
	.c3 = CAP_U(2.2),
	.r12 = RES_K(6.8),
	.r17 = RES_M(100),  // Not connected. Using a huge resistor.
	.split_rom = false,
	.multi_rom = false,
	.pitch_control = false,
	.decay = dmx_voice_card_config::decay_mode::ENABLED_ON_TR12,
	.early_decay = dmx_voice_card_config::early_decay_mode::ENABLED_ON_TR1,
	.filter = FILTER_CONFIG_HIGH,
};

constexpr const dmx_voice_card_config TOM_CONFIG =
{
	.c2 = CAP_U(0.0033),
	.c3 = CAP_U(6.8),
	.r12 = RES_K(33),
	.r17 = RES_K(18),
	.split_rom = false,
	.multi_rom = false,
	.pitch_control = true,
	.decay = dmx_voice_card_config::decay_mode::ENABLED,
	.early_decay = dmx_voice_card_config::early_decay_mode::ENABLED,
	.filter = FILTER_CONFIG_LOW,
};

constexpr dmx_voice_card_config PERC_CONFIG(float c2, bool filter_high)
{
	// The schematic and electrongate.com agree on all components for PERC1,
	// but disagree for PERC2:
	// * C2: 0.0033uF in schematic, but 0.0047uF on electrongate.com.
	// * Filter: "high" configuration in the schematic, but "low" on
	//   electrongate.com.
	return
	{
		.c2 = c2,
		.c3 = 0,
		.r12 = RES_K(1.5),
		.r17 = RES_R(30),
		.split_rom = true,
		.multi_rom = false,
		.pitch_control = false,
		.decay = dmx_voice_card_config::decay_mode::DISABLED,
		.early_decay = dmx_voice_card_config::early_decay_mode::ENABLED_ON_TR1,
		.filter = filter_high ? FILTER_CONFIG_HIGH : FILTER_CONFIG_LOW,
	};
}

constexpr const dmx_voice_card_config CYMBAL_CONFIG =
{
	.c2 = CAP_U(0.0033),
	.c3 = 0,
	.r12 = RES_R(680),
	.r17 = RES_R(100),
	.split_rom = true,
	.multi_rom = true,
	.pitch_control = false,
	.decay = dmx_voice_card_config::decay_mode::DISABLED,
	.early_decay = dmx_voice_card_config::early_decay_mode::ENABLED_ON_TR1,
	.filter = FILTER_CONFIG_HIGH,
};


class dmx_state : public driver_device
{
public:
	enum voice_card_indices
	{
		VC_BASS = 0,
		VC_SNARE,
		VC_HIHAT,
		VC_SMALL_TOMS,
		VC_LARGE_TOMS,
		VC_PERC1,
		VC_PERC2,
		VC_CYMBAL,
		NUM_VOICE_CARDS
	};
	static constexpr const int METRONOME_INDEX = NUM_VOICE_CARDS;

	static constexpr feature_type unemulated_features() { return feature::TAPE; }

	dmx_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MAINCPU_TAG)
		, m_digit_device(*this, "dl1414_%d", 0)
		, m_digit_output(*this, "digit_%d", 0U)
		, m_metronome(*this, "metronome_sound")
		, m_metronome_out(*this, "METRONOME_OUT")
		, m_metronome_timer(*this, "metronome_timer")
		, m_buttons(*this, "buttons_%d", 0)
		, m_faders(*this, "fader_p%d", 1)
		, m_switches(*this, "switches")
		, m_external_triggers(*this, "external_triggers")
		, m_output_select(*this, "output_select")
		, m_clk_in(*this, "clk_in")
		, m_clk_out_tip(*this, "CLK_OUT_tip")
		, m_voices(*this, "voice_%d", 0)
		, m_voice_rc(*this, "voice_rc_filter_%d", 0)
		, m_left_mixer(*this, "left_mixer")
		, m_right_mixer(*this, "right_mixer")
		, m_left_speaker(*this, "lspeaker")
		, m_right_speaker(*this, "rspeaker")
		, m_samples(*this, "sample_%d", 0)
	{
	}

	void dmx(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(clk_in_changed);
	DECLARE_INPUT_CHANGED_MEMBER(selected_output_changed);
	DECLARE_INPUT_CHANGED_MEMBER(voice_volume_changed);
	DECLARE_INPUT_CHANGED_MEMBER(master_volume_changed);
	DECLARE_INPUT_CHANGED_MEMBER(pitch_adj_changed);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	void refresh_int_flipflop();
	void int_timer_preset_w(u8 data);
	void int_timer_enable_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(int_timer_tick);

	void update_metronome();
	void metronome_mix_w(u8 data);
	void metronome_level_w(int state);
	void metronome_trigger_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(metronome_timer_tick);

	void display_w(offs_t offset, u8 data);
	template<int DISPLAY> void display_output_w(offs_t offset, u16 data);

	u8 buttons_r(offs_t offset);
	u8 external_triggers_r();
	u8 cassette_r();
	template<int GROUP> void gen_trigger_w(u8 data);

	void update_output();
	void update_mix_level(int voice);

	void memory_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device_array<dl1414_device, 4> m_digit_device;
	output_finder<16> m_digit_output;
	required_device<speaker_sound_device> m_metronome;
	output_finder<> m_metronome_out;
	required_device<timer_device> m_metronome_timer;
	required_ioport_array<6> m_buttons;
	required_ioport_array<10> m_faders;
	required_ioport m_switches;  // Includes foot switches.
	required_ioport m_external_triggers;
	required_ioport m_output_select;
	required_ioport m_clk_in;
	output_finder<> m_clk_out_tip;  // Tip conncetion of "CLK OUT" TRS jack.

	required_device_array<dmx_voice_card_device, 8> m_voices;
	required_device_array<filter_rc_device, 9> m_voice_rc;
	required_device<mixer_device> m_left_mixer;
	required_device<mixer_device> m_right_mixer;
	required_device<speaker_device> m_left_speaker;
	required_device<speaker_device> m_right_speaker;
	required_memory_region_array<8> m_samples;

	// 40103 timer (U11 in Processor Board)
	u8 m_int_timer_preset = 0xff;
	bool m_int_timer_enabled = true;
	s16 m_int_timer_value = 0xff;
	u8 m_int_timer_out = 1;
	u8 m_int_flipflop_clock = 0;

	// Metronome state.
	bool m_metronome_on = false;
	bool m_metronome_mix = false;
	bool m_metronome_level_high = false;

	// The service manual states that the crystal is 4.912MHz, in the section
	// that describes the clock circuitry. However, the parts list specifies
	// this as a 4.9152 MHz crystal. The parts list value is likely the correct
	// one.
	// Divided by U35, 74LS74.
	static constexpr const XTAL CPU_CLOCK = 4.9152_MHz_XTAL / 2;
	// Divided by U34, 74LS393.
	static constexpr const XTAL INT_TIMER_CLOCK = CPU_CLOCK / 256;

	// Configuration for each voice needs to appear in the index specified by
	// the voice_card_indices enum for that voice.
	static constexpr const dmx_voice_card_config VOICE_CONFIGS[NUM_VOICE_CARDS] =
	{
		BASS_CONFIG,    // VC_BASS
		SNARE_CONFIG,   // VC_SNARE
		HIHAT_CONFIG,   // VC_HIHAT
		TOM_CONFIG,     // VC_SMALL_TOMS
		TOM_CONFIG,     // VC_LARGE_TOMS
		PERC_CONFIG(CAP_U(0.0033), true),   // VC_PERC1
		PERC_CONFIG(CAP_U(0.0047), false),  // VC_PERC2
		CYMBAL_CONFIG,  // VC_CYMBAL
	};

	// The loud click is a ~2.5V pulse, while the quiet one is a ~1.25V pulse.
	// See update_metronome().
	static constexpr double METRONOME_LEVELS[3] = { 0.0, 0.5, 1.0 };

	// The mixer's inputs are the 8 voices and the metronome.
	static constexpr const int NUM_MIXED_VOICES = NUM_VOICE_CARDS + 1;

	// The left and right channel mixing resistors for each voice. The voice
	// will be panned towards the side with lower resistance. Furthermore, these
	// resistors control the relative volume of the voices, where lower
	// resistance means louder.
	// Each entry needs to appear in the index specified by voice_card_indices,
	// except for the metronome entry, which appears last.
	// All resistors are located on the switchboard.
	static constexpr const std::tuple<float, float> MIX_RESISTORS[NUM_MIXED_VOICES] =
	{
		{ RES_K(10),  RES_K(10)  },  // R10, R9  - VC_BASS
		{ RES_K(8.2), RES_K(20)  },  // R12, R11 - VC_SNARE
		{ RES_K(20),  RES_K(8.2) },  // R14, R13 - VC_HIHAT
		{ RES_K(6.8), RES_K(100) },  // R16, R15 - VC_SMALL_TOMS
		{ RES_K(100), RES_K(6.8) },  // R18, R17 - VC_LARGE_TOMS
		{ RES_K(6.8), RES_K(100) },  // R22, R21 - VC_PERC1
		{ RES_K(100), RES_K(6.8) },  // R24, R23 - VC_PERC2
		{ RES_K(8.2), RES_K(20)  },  // R20, R19 - VC_CYMBAL
		{ RES_K(10),  RES_K(10)  },  // R26, R25 - METRONOME_INDEX
									 // ECO 304 values (see update_metronome()).
	};

	static constexpr const int VOICE_TO_FADER_MAP[NUM_MIXED_VOICES] =
	{
		0, 1, 2, 3, 4, 6, 7, 5, 8
	};
};

void dmx_state::refresh_int_flipflop()
{
	// All component designations refer to the Processor Board.

	// Interrups are used for maintaining tempo. They are generated by an SR
	// flipflop (U23B, 4013), with /Q connected to /INT. D is always 1.
	// If CLK IN is not connected, the flipflop is clocked by the timer's output
	// (U11, 40103).
	// If CLK IN is connected, it will clock the flipflop, and the timer's
	// output will be ignored.
	// The Z80's interrupt acknowledge cycle clears the flipfop (flipflop
	// "clear" connected to NOR(/M1, /IORQ) (U32, 4001)).
	// CSYN (cassette sync) will also trigger interrupts, via the flipflop's
	// "set" input.
	// The circuit ensures that either the timer is enabled, or CSYN signals
	// are generated, but not both. This prevents potentially conflicting
	// interrupts.

	u8 new_int_flipflop_clock = 0;
	if (m_clk_in->read() & 0x01)  // Is CLK IN plugged?
	{
		new_int_flipflop_clock = (m_clk_in->read() & 0x02) ? 1 : 0;
	}
	else
	{
		// 40103 (U11) timer output is inverted by Q1, R20 and R43.
		new_int_flipflop_clock = m_int_timer_out ? 0 : 1;
	}

	if (!m_int_flipflop_clock && new_int_flipflop_clock)  // 0->1 transition.
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);

	m_int_flipflop_clock = new_int_flipflop_clock;
	m_clk_out_tip = m_int_flipflop_clock;
}

void dmx_state::int_timer_preset_w(u8 data)
{
	if (m_int_timer_preset == data)
		return;
	m_int_timer_preset = data;
	LOGMASKED(LOG_INT_TIMER, "INT timer preset: %02x\n", m_int_timer_preset);
}

void dmx_state::int_timer_enable_w(int state)
{
	// When the INT timer is enabled, the cassette sync INT generator is
	// disabled, and vice versa. Cassette functionality (including sync) is
	// not currently emulated.
	if (m_int_timer_enabled == bool(state))
		return;

	// The CLEN signal is inverted by U28 (4011 NAND), but it is active low
	// since it is connected on the /CE input of a 40103. So, logically,
	// "1" == "enabled".
	m_int_timer_enabled = bool(state);
	LOGMASKED(LOG_INT_TIMER, "INT timer enabled: %d\n", m_int_timer_enabled);
}

TIMER_DEVICE_CALLBACK_MEMBER(dmx_state::int_timer_tick)
{
	if (!m_int_timer_enabled)
		return;

	if (m_int_timer_value <= 0)  // We reached 0 in the previous clock cycle.
	{
		// The 40103 timer is configured for synchronous loading of the preset
		// value. So it starts over in the next clock cycle after reaching 0.
		// TODO: Actually verify that a preset of 0 means 256. Not clear in
		// the datasheet.
		if (m_int_timer_preset == 0)
			m_int_timer_value = 256;
		else
			m_int_timer_value = m_int_timer_preset;

		// The timer's output transitions to 1 in the clock cycle after reaching
		// 0 (the one we are handling here).
		m_int_timer_out = 1;
		refresh_int_flipflop();
		return;
	}

	--m_int_timer_value;

	if (m_int_timer_value <= 0)
	{
		// The timer's output goes low when its counter reaches 0.
		m_int_timer_out = 0;
		refresh_int_flipflop();
	}
}

void dmx_state::update_metronome()
{
	// The metronome "click" is a 1ms pulse. The metronome is active during
	// playback and recording, and the pulses appear on the "metronome out"
	// connection. The metronome can also be mixed with the rest of the voices,
	// but this only happens during recording, not playback. The loudness of
	// the "click" can be controlled by the firmware (2 distinct levels).
	// The metronome circuit is also used to generate "beep" tones.

	// The emulated circuitry is that of Engineering Change Order 304 (ECO 304,
	// January 1982), which improved the metronome.

	m_metronome_out = m_metronome_on ? 1 : 0;  // ~10 Volt pulse on "metronome out".

	int level = 0;
	if (m_metronome_on && m_metronome_mix)
		level = m_metronome_level_high ? 2 : 1;
	m_metronome->level_w(level);

	LOGMASKED(LOG_METRONOME, "Metronome update - on:%d, mix:%d, level:%d\n",
			m_metronome_on, m_metronome_mix, level);
}

void dmx_state::metronome_mix_w(u8 data)
{
	// D0 connected to D of U35 74LS74 (D-flipflop), which is clocked by
	// CLICK* from the address decoder (U7, 74LS42), acting as a 1-bit latch.
	const bool new_value = BIT(data, 0);
	if (m_metronome_mix == new_value)
		return;
	m_metronome_mix = new_value;
	update_metronome();
}

void dmx_state::metronome_level_w(int state)
{
	// LV1 signal. D3 of U20 (74C174 latch, processor board).
	const bool new_value = state;
	if (m_metronome_level_high == new_value)
		return;
	m_metronome_level_high = new_value;
	update_metronome();
}

void dmx_state::metronome_trigger_w(u8 /*data*/)
{
	// Writing to this port clocks U33A (D-flipflop). R62 and C32 form an RC
	// network that resets the flipflop after ~1ms. This becomes a pulse (via
	// Q8, R55, R54) on the "metronome out" connection, and in some cases also
	// mixed with the other voices (see update_metronome()).
	m_metronome_timer->adjust(attotime::from_msec(1));
	m_metronome_on = true;
	update_metronome();
}

TIMER_DEVICE_CALLBACK_MEMBER(dmx_state::metronome_timer_tick)
{
	m_metronome_on = false;
	update_metronome();
}

void dmx_state::display_w(offs_t offset, u8 data)
{
	// Decoding between different displays is done by U2 (74LS42). The
	// m_digit_device array is ordered from right to left (U6-U3). All
	// components are on the Switch Board.
	m_digit_device[offset >> 2]->bus_w(offset & 0x03, data);
}

template<int DISPLAY> void dmx_state::display_output_w(offs_t offset, u16 data)
{
	// The displays and the digits within each display are addressed from right
	// to left with increassing offset. Reversing that order in m_digit_output,
	// since left-to-right is more convenient for building the layout.
	static_assert(DISPLAY >= 0 && DISPLAY < 4);
	const int display_in_layout = 3 - DISPLAY;
	const int digit_in_layout = 3 - (offset & 0x03);
	m_digit_output[4 * display_in_layout + digit_in_layout] = data;
}

u8 dmx_state::buttons_r(offs_t offset)
{
	// Switches are inverted and buffered by 2 x 80C98 (inverting 3-state
	// buffers), U7 and U8 on Switch Board.
	return ~m_buttons[offset]->read();
}

u8 dmx_state::external_triggers_r()
{
	// External triggers are inverted by 2 X CA3086 NPN arrays (U4 for D0-D3 and
	// U5 for D4-D7) and buffered by U6 (74LS244). All components are on the
	// Processor Board.
	return ~m_external_triggers->read();
}

u8 dmx_state::cassette_r()
{
	// All components are on the Processor Board.
	// Inputs buffered by U19 (75LS244). This is a mix of cassette-related
	// inputs and the states of foot-switches and toggle-switches.

	const u8 data = m_switches->read();

	const u8 d0 = 0;  // CD0: not yet implemented.
	const u8 d1 = 0;  // CD1: not yet implemented.
	const u8 d2 = 0;  // CD2: not yet implemented.

	const u8 d3 = BIT(data, 3);  // CASEN* (cassette enable switch. Active low).

	// START footswitch input is pulled high (R41) and ANDed with 1 (U22,
	// 74LS08, with the other input tied high via R42). So U22 is logically a
	// no-op. However, the NEXT footswitch is tied directly to U19.
	const u8 d4 = BIT(data, 4);  // START* (output of U22).
	const u8 d5 = BIT(data, 5);  // NEXT (pulled high by R58).
	const u8 d6 = BIT(data, 6);  // TSYN* (pulled high by R23).
	const u8 d7 = BIT(data, 7);  // PROT* (memory protect switch. Active low).

	return (d7 << 7) | (d6 << 6) | (d5 << 5) | (d4 << 4) |
		   (d3 << 3) | (d2 << 2) | (d1 << 1) | d0;
}

template<int GROUP> void dmx_state::gen_trigger_w(u8 data)
{
	// Triggers are active-high and only last for the duration of the write.
	// They are implemented by enabling an 74LS244 (U8 and U9) whose outputs are
	// normally pulled low.

	static_assert(GROUP >= 0 && GROUP <= 1);
	static constexpr const int TRIGGER_MAPPINGS[2][NUM_VOICE_CARDS] =
	{
		{0, 2, 4, 6, 1, 3, 5, 7},
		{8, 14, 10, 12, 9, 15, 11, 13},
	};

	if (data == 0)
		return;

	std::vector<bool> triggers(2 * NUM_VOICE_CARDS, false);
	for (int i = 0; i < NUM_VOICE_CARDS; ++i)
	{
		if (BIT(data, i))
		{
			const int trig_index = TRIGGER_MAPPINGS[GROUP][i];
			triggers[trig_index] = true;
			LOGMASKED(LOG_TRIGGERS, "Trigerred TR%d\n", trig_index);
		}
	}

	for (int i = 0; i < NUM_VOICE_CARDS; ++i)
	{
		const bool tr0 = triggers[2 * i];
		const bool tr1 = triggers[2 * i + 1];
		if (tr0 || tr1)
			m_voices[i]->trigger(tr0, tr1);
	}
}

void dmx_state::update_output()
{
	const float stereo_gain = (m_output_select->read() & 0x01) ? 1 : 0;
	m_left_speaker->set_input_gain(0, stereo_gain); // left
	m_left_speaker->set_input_gain(1, 1 - stereo_gain);  // mono
	m_right_speaker->set_input_gain(0, stereo_gain);  // right
	m_right_speaker->set_input_gain(1, 1 - stereo_gain);  // mono
	LOGMASKED(LOG_FADERS, "Output changed to: %d\n", m_output_select->read());
}

void dmx_state::update_mix_level(int voice)
{
	// Computes the gain of a voice for the left and right channels, taking
	// into account the fader position and the loading from the mixing
	// resistors. Each voice has a hardcoded pan and relative mixing ratio based
	// on the resistors in MIX_RESISTORS.
	// Also reconfigures the voice's output RC filter (high-pass), since its `R`
	// changes when the volume fader moves. This won't be audible, since the
	// cutoff frequency is mostly <10 HZ, except for low volumes.
	assert(voice >= 0 && voice < NUM_MIXED_VOICES);

	static constexpr const float P_MAX = RES_K(10);  // Volume potentiometer.
	static constexpr const float VC_R21 = RES_R(4.7);  // R21 on voice cards.
	static constexpr const float VC_C10 = CAP_U(33);  // C10 on voice cards.
	static constexpr const float PB_C24 = CAP_U(6.8);  // C24 on processor board.

	// Feedback resistors on the left and right summing op-amps (U1B, U1C).
	static constexpr const float R_FEEDBACK_LEFT = RES_K(4.7);  // R30.
	static constexpr const float R_FEEDBACK_RIGHT = RES_K(4.7);  // R29.

	const s32 pot_percent = m_faders[VOICE_TO_FADER_MAP[voice]]->read();
	const float r_pot_bottom = P_MAX * pot_percent / 100.0F;
	const float r_pot_top = P_MAX - r_pot_bottom;
	const float r_mix_left = std::get<0>(MIX_RESISTORS[voice]);
	const float r_mix_right = std::get<1>(MIX_RESISTORS[voice]);
	const float r_gnd = RES_3_PARALLEL(r_pot_bottom, r_mix_left, r_mix_right);
	const float r_top_extra = (voice == METRONOME_INDEX) ? 0 : VC_R21;
	const float v_pot = RES_VOLTAGE_DIVIDER(r_pot_top + r_top_extra, r_gnd);

	// -v_pot because the summing opamp mixer is inverting.
	const float gain_left = -v_pot * R_FEEDBACK_LEFT / r_mix_left;
	const float gain_right = -v_pot * R_FEEDBACK_RIGHT / r_mix_right;
	const float rc_c = (voice == METRONOME_INDEX) ? PB_C24 : VC_C10;

	m_voice_rc[voice]->filter_rc_set_RC(filter_rc_device::HIGHPASS, r_gnd, 0, 0, rc_c);
	m_left_mixer->set_input_gain(voice, gain_left);
	m_right_mixer->set_input_gain(voice, gain_right);

	LOGMASKED(LOG_FADERS, "Voice %d volume changed to: %d (gain L:%f, R:%f), HPF cutoff: %.2f Hz\n",
			voice, pot_percent, gain_left, gain_right, 1.0F / (2 * float(M_PI) * r_gnd * rc_c));
}

void dmx_state::memory_map(address_map &map)
{
	// Component designations refer to the Processor Board.
	// Memory decoding done by U2 (74LS42) and U22 (74LS08).
	map.global_mask(0x3fff);  // A14 and A15 not used.
	map(0x0000, 0x1fff).rom();  // 2 x 2732 (4K x 8bit) ROMs, U18-U17.
	// 4 x 6116 (2K x 8bit) RAMs, U16-U13. U3 (4071, OR-gate) will only allow
	// RAM select signals to go through if fully powered up.
	map(0x2000, 0x3fff).ram().share(NVRAM_TAG);
}

void dmx_state::io_map(address_map &map)
{
	// Signal names (e.g. GEN0*) match those in the schematics.

	map.global_mask(0x7f);  // A7-A15 not used for port decoding or I/O.

	// Write decoding done by U7 (74LS42) on the Processor Board.
	map(0x00, 0x0f).w(FUNC(dmx_state::display_w));  // ID*.
	map(0x10, 0x10).mirror(0x0f).w(FUNC(dmx_state::gen_trigger_w<0>));  // GEN0*
	map(0x20, 0x20).mirror(0x0f).w(FUNC(dmx_state::gen_trigger_w<1>));  // GEN1*
	map(0x30, 0x30).mirror(0x0f).w(FUNC(dmx_state::int_timer_preset_w));  // STIM*
	map(0x40, 0x40).mirror(0x0f).w(FUNC(dmx_state::metronome_trigger_w));  // METRONOME OUT
	map(0x50, 0x50).mirror(0x0f).w("cas_latch", FUNC(output_latch_device::write));  // CAS0*
	map(0x60, 0x60).mirror(0x0f).w(FUNC(dmx_state::metronome_mix_w));  // CLICK*

	// Read decoding done by U9 (74LS42) on the Switch Board.
	map(0x00, 0x05).mirror(0x78).r(FUNC(dmx_state::buttons_r));  // IORD*
	map(0x06, 0x06).mirror(0x78).r(FUNC(dmx_state::external_triggers_r));  // SW6*
	map(0x07, 0x07).mirror(0x78).r(FUNC(dmx_state::cassette_r));  // CASI*
}

void dmx_state::machine_start()
{
	m_clk_out_tip.resolve();
	m_metronome_out.resolve();
	m_digit_output.resolve();

	save_item(NAME(m_int_timer_preset));
	save_item(NAME(m_int_timer_enabled));
	save_item(NAME(m_int_timer_value));
	save_item(NAME(m_int_timer_out));
	save_item(NAME(m_int_flipflop_clock));

	save_item(NAME(m_metronome_on));
	save_item(NAME(m_metronome_mix));
	save_item(NAME(m_metronome_level_high));
}

void dmx_state::machine_reset()
{
	update_output();
	for (int i = 0; i < NUM_MIXED_VOICES; ++i)
		update_mix_level(i);
}

void dmx_state::dmx(machine_config &config)
{
	Z80(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &dmx_state::memory_map);
	m_maincpu->set_addrmap(AS_IO, &dmx_state::io_map);

	NVRAM(config, NVRAM_TAG, nvram_device::DEFAULT_ALL_0);

	// 40103 timer (U11 on Processor Board). See refresh_int_flipflop() and
	// int_timer_tick().
	TIMER(config, "int_timer_u11").configure_periodic(
		FUNC(dmx_state::int_timer_tick), attotime::from_hz(INT_TIMER_CLOCK));

	// A one-shot timer based on an RC-cicruit. See metronome_trigger_w().
	TIMER(config, m_metronome_timer).configure_generic(
		FUNC(dmx_state::metronome_timer_tick));

	DL1414T(config, m_digit_device[0], 0U).update().set(FUNC(dmx_state::display_output_w<0>));
	DL1414T(config, m_digit_device[1], 0U).update().set(FUNC(dmx_state::display_output_w<1>));
	DL1414T(config, m_digit_device[2], 0U).update().set(FUNC(dmx_state::display_output_w<2>));
	DL1414T(config, m_digit_device[3], 0U).update().set(FUNC(dmx_state::display_output_w<3>));

	config.set_default_layout(layout_oberheim_dmx);

	// 74C174 (U20, on Processor Board), 6-bit latch.
	output_latch_device &cas_latch(OUTPUT_LATCH(config, "cas_latch"));
	cas_latch.bit_handler<0>().set_output("CDATO");
	cas_latch.bit_handler<1>().set_output("ARM");
	cas_latch.bit_handler<2>().set(FUNC(dmx_state::int_timer_enable_w));
	// Bit 3 is an open-collector connection to the Ring of the "CLK OUT" TRS jack.
	cas_latch.bit_handler<3>().set_output("CLK_OUT_ring").invert();
	cas_latch.bit_handler<4>().set(FUNC(dmx_state::metronome_level_w));  // LV1.
	// Bit 5 not connected.

	MIXER(config, m_left_mixer);
	MIXER(config, m_right_mixer);
	for (int i = 0; i < NUM_VOICE_CARDS; ++i)
	{
		DMX_VOICE_CARD(config, m_voices[i], VOICE_CONFIGS[i], &m_samples[i]);
		FILTER_RC(config, m_voice_rc[i]);
		// The RC filters are initialized in machine_reset() (specifically:
		// update_mix_level()).
		m_voices[i]->add_route(ALL_OUTPUTS, m_voice_rc[i], 1.0);
		m_voice_rc[i]->add_route(ALL_OUTPUTS, m_left_mixer, 1.0);
		m_voice_rc[i]->add_route(ALL_OUTPUTS, m_right_mixer, 1.0);
	}

	SPEAKER_SOUND(config, m_metronome);
	FILTER_RC(config, m_voice_rc[METRONOME_INDEX]);
	m_metronome->set_levels(3, METRONOME_LEVELS);
	m_metronome->add_route(ALL_OUTPUTS, m_voice_rc[METRONOME_INDEX], 1.0);
	m_voice_rc[METRONOME_INDEX]->add_route(ALL_OUTPUTS, m_left_mixer, 1.0);
	m_voice_rc[METRONOME_INDEX]->add_route(ALL_OUTPUTS, m_right_mixer, 1.0);

	// Passive mixer using 1K resistors (R33 and R34).
	mixer_device &mono_mixer = MIXER(config, "mono_mixer");
	m_left_mixer->add_route(ALL_OUTPUTS, mono_mixer, 0.5);
	m_right_mixer->add_route(ALL_OUTPUTS, mono_mixer, 0.5);

	// Only one of the left (right) or mono will be active for each speaker at
	// runtime. Controlled by a config setting (see update_output()).

	SPEAKER(config, m_left_speaker).front_left();
	m_left_mixer->add_route(ALL_OUTPUTS, m_left_speaker, 1.0);
	mono_mixer.add_route(ALL_OUTPUTS, m_left_speaker, 1.0);

	SPEAKER(config, m_right_speaker).front_right();
	m_right_mixer->add_route(ALL_OUTPUTS, m_right_speaker, 1.0);
	mono_mixer.add_route(ALL_OUTPUTS, m_right_speaker, 1.0);
}

DECLARE_INPUT_CHANGED_MEMBER(dmx_state::clk_in_changed)
{
	refresh_int_flipflop();
}

DECLARE_INPUT_CHANGED_MEMBER(dmx_state::selected_output_changed)
{
	update_output();
}

DECLARE_INPUT_CHANGED_MEMBER(dmx_state::voice_volume_changed)
{
	update_mix_level(param);
}

DECLARE_INPUT_CHANGED_MEMBER(dmx_state::master_volume_changed)
{
	const float gain = newval / 100.0F;
	m_left_mixer->set_output_gain(0, gain);
	m_right_mixer->set_output_gain(0, gain);
	LOGMASKED(LOG_FADERS, "Master volume changed: %d\n", newval);
}

DECLARE_INPUT_CHANGED_MEMBER(dmx_state::pitch_adj_changed)
{
	// Using "100 -" so that larger values increase pitch.
	m_voices[param]->set_pitch_adj(100 - newval);
	LOGMASKED(LOG_PITCH, "Voice %d pitch adjustment changed: %d\n", param, newval);
}

INPUT_PORTS_START(dmx)
	PORT_START("buttons_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("buttons_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("<") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME(">") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SWING") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SONG") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EDIT") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("STEP") PORT_CODE(KEYCODE_PGUP)

	PORT_START("buttons_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RECORD") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ERASE") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("COPY") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LENGTH") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PLAY") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TEMPO") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("QUANT") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SIGN") PORT_CODE(KEYCODE_COLON)

	PORT_START("buttons_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BASS 3") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SNARE 3") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("HI-HAT OPEN") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TOM 3") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TOM 6") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CYMB 3") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RIMSHOT") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CLAPS") PORT_CODE(KEYCODE_COMMA)

	PORT_START("buttons_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BASS 2") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SNARE 2") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("HI-HAT ACC") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TOM 2") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TOM 5") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CYMB 2") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TAMP 2") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SHAKE 2") PORT_CODE(KEYCODE_K)

	PORT_START("buttons_5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BASS 1") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SNARE 1") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("HI-HAT CLOSED") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TOM 1") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TOM 4") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CYMB 1") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TAMB 1") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SHAKE 1") PORT_CODE(KEYCODE_I)

	PORT_START("switches")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CASSETTE ENABLE") PORT_TOGGLE
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("START")  // Footswitch.
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("NEXT")  // Footswitch.
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SYNC IN middle")  // Plug input.
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MEMORY PROTECT") PORT_TOGGLE

	PORT_START("external_triggers")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("BASS TRIG")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("PERC1 TRIG")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("CYMBAL TRIG")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TOM2 TRIG")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TOM1 TRIG")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("HI-HAT TRIG")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SNARE TRIG")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("PERC2 TRIG")

	PORT_START("clk_in")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("CLK IN plugged") PORT_CODE(KEYCODE_QUOTE) PORT_TOGGLE
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("CLK IN") PORT_CODE(KEYCODE_SLASH)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::clk_in_changed), 0)

	PORT_START("output_select")
	PORT_CONFNAME(0x01, 0x01, "Output")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::selected_output_changed), 0)
	PORT_CONFSETTING(   0x00, "Mono")
	PORT_CONFSETTING(   0x01, "Stereo")

	// Fader potentiometers. P1-P10 on the Switch Board.

	PORT_START("fader_p1")
	PORT_ADJUSTER(100, "BASS volume")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::voice_volume_changed), dmx_state::VC_BASS)

	PORT_START("fader_p2")
	PORT_ADJUSTER(100, "SNARE volume")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::voice_volume_changed), dmx_state::VC_SNARE)

	PORT_START("fader_p3")
	PORT_ADJUSTER(100, "HI-HAT volume")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::voice_volume_changed), dmx_state::VC_HIHAT)

	PORT_START("fader_p4")
	PORT_ADJUSTER(100, "TOM1 volume")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::voice_volume_changed), dmx_state::VC_SMALL_TOMS)

	PORT_START("fader_p5")
	PORT_ADJUSTER(100, "TOM2 volume")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::voice_volume_changed), dmx_state::VC_LARGE_TOMS)

	PORT_START("fader_p6")
	PORT_ADJUSTER(100, "CYMBAL volume")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::voice_volume_changed), dmx_state::VC_CYMBAL)

	PORT_START("fader_p7")
	PORT_ADJUSTER(100, "PERC1 volume")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::voice_volume_changed), dmx_state::VC_PERC1)

	PORT_START("fader_p8")
	PORT_ADJUSTER(100, "PERC2 volume")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::voice_volume_changed), dmx_state::VC_PERC2)

	PORT_START("fader_p9")
	PORT_ADJUSTER(100, "MET volume")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::voice_volume_changed), dmx_state::METRONOME_INDEX)

	PORT_START("fader_p10")
	PORT_ADJUSTER(100, "VOLUME")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::master_volume_changed), 0)

	// Tunning potentiomenters. One per voice card, designated as T1 and labeled
	// as "PITCH ADJ."

	PORT_START("pitch_adj_1")
	PORT_ADJUSTER(dmx_voice_card_device::T1_DEFAULT_PERCENT, "BASS pitch")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::pitch_adj_changed), dmx_state::VC_BASS)

	PORT_START("pitch_adj_2")
	PORT_ADJUSTER(dmx_voice_card_device::T1_DEFAULT_PERCENT, "SNARE pitch")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::pitch_adj_changed), dmx_state::VC_SNARE)

	PORT_START("pitch_adj_3")
	PORT_ADJUSTER(dmx_voice_card_device::T1_DEFAULT_PERCENT, "HI-HAT pitch")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::pitch_adj_changed), dmx_state::VC_HIHAT)

	PORT_START("pitch_adj_4")
	PORT_ADJUSTER(dmx_voice_card_device::T1_DEFAULT_PERCENT, "TOM1 pitch")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::pitch_adj_changed), dmx_state::VC_SMALL_TOMS)

	PORT_START("pitch_adj_5")
	PORT_ADJUSTER(dmx_voice_card_device::T1_DEFAULT_PERCENT, "TOM2 pitch")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::pitch_adj_changed), dmx_state::VC_LARGE_TOMS)

	PORT_START("pitch_adj_6")
	PORT_ADJUSTER(dmx_voice_card_device::T1_DEFAULT_PERCENT, "CYMBAL pitch")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::pitch_adj_changed), dmx_state::VC_CYMBAL)

	PORT_START("pitch_adj_7")
	PORT_ADJUSTER(dmx_voice_card_device::T1_DEFAULT_PERCENT, "PERC1 pitch")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::pitch_adj_changed), dmx_state::VC_PERC1)

	PORT_START("pitch_adj_8")
	PORT_ADJUSTER(dmx_voice_card_device::T1_DEFAULT_PERCENT, "PERC2 pitch")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::pitch_adj_changed), dmx_state::VC_PERC2)
INPUT_PORTS_END

ROM_START(obdmx)
	ROM_REGION(0x2000, MAINCPU_TAG, 0)  // 2 x 2732 (4K x 8bit) ROMs, U18 and U17.
	ROM_DEFAULT_BIOS("rev_f")

	ROM_SYSTEM_BIOS(0, "debug", "DMX debug firmware")
	ROMX_LOAD("dmx_dbg.u18", 0x000000, 0x001000, CRC(b3a24d64) SHA1(99a64b92f1520aaf2e7117b2fc75bdbd847f3a7f), ROM_BIOS(0))
	ROMX_FILL(0x001000, 0x001000, 0xff, ROM_BIOS(0))  // No ROM needed in U17.

	ROM_SYSTEM_BIOS(1, "rev_b", "DMX 1.00 (Revision B), 1981")
	ROMX_LOAD("dmx_b0.u18", 0x000000, 0x001000, CRC(69116c5b) SHA1(29448b60d7f864b0a9df2a786b877feb7b019c05), ROM_BIOS(1))
	ROMX_LOAD("dmx_b1.u17", 0x001000, 0x001000, CRC(49084dee) SHA1(fffe4e6592dc586c0a0f03bed18308df38b991da), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "rev_f", "DMX 2.21 (Revision F), 1982")
	ROMX_LOAD("dmx_f0.u18", 0x000000, 0x001000, CRC(a01e5e7d) SHA1(74faf9c056c7e3eb89ed7657d8b64b1d3fe0ec22), ROM_BIOS(2))
	ROMX_LOAD("dmx_f1.u17", 0x001000, 0x001000, CRC(b24d4040) SHA1(74e7ae87cdcf9442cf76b696970c0efe52a30262), ROM_BIOS(2))

	// Sample ROMs. The number appended to "sample_" needs to match the
	// corresponding voice in the dmx_state::voice_card_indices enum.

	ROM_REGION(0x1000, "sample_0", 0)  // Bass (dmx_state::VC_BASS).
	ROM_LOAD("21kick.u7", 0x000000, 0x001000, CRC(3ab63275) SHA1(83b0e61e42c9d277f205f4ff5dfcb796c05084b8))

	ROM_REGION(0x1000, "sample_1", 0)  // Snare (dmx_state::VC_SNARE).
	ROM_LOAD("snare6.u7", 0x000000, 0x001000, CRC(eca57ad7) SHA1(f65dcae905e0f7db9bf9324153ddd16a33dbdc8b))

	ROM_REGION(0x1000, "sample_2", 0)  // Hi-Hat (dmx_state::VC_HIHAT).
	ROM_LOAD("hat1a.u7", 0x000000, 0x001000, CRC(36e934f9) SHA1(e97d65bfb2d98cbf463a92d12e2ab91938757d37))

	ROM_REGION(0x1000, "sample_3", 0)  // Small Toms (dmx_state::VC_SMALL_TOMS).
	ROM_LOAD("stom5.u7", 0x000000, 0x001000, CRC(e8022d72) SHA1(61a14da4d96db12b040ab9529208b165212b3693))

	ROM_REGION(0x1000, "sample_4", 0)  // Large Toms (dmx_state::VC_LARGE_TOMS).
	ROM_LOAD("sfltom2.u7", 0x000000, 0x001000, CRC(f5b48db0) SHA1(a85e72a9f87838c26538e5873f6938e51a517c90))

	ROM_REGION(0x1000, "sample_5", 0)  // Perc 1: Tambourine / Rimshot (dmx_state::VC_PERC1).
	ROM_LOAD("stik.u7", 0x000000, 0x001000, CRC(61af39e3) SHA1(5648674854a8db80656bf729c4f353b75d101d7b))

	ROM_REGION(0x1000, "sample_6", 0)  // Perc 2: Shaker / Claps (dmx_state::VC_PERC2).
	ROM_LOAD("shake6.u7", 0x000000, 0x001000, CRC(00549f07) SHA1(d41c3357d486c398f882774130789c9756dfb9af))

	ROM_REGION(0x8000, "sample_7", 0)  // Cymbal: Ride / Crash (dmx_state::VC_CYMBAL).
	ROM_LOAD("crash1.u12", 0x000000, 0x001000, CRC(40d584af) SHA1(98702946e80892eb834433268164404fab3b0e28))
	ROM_LOAD("crash2.u11", 0x001000, 0x001000, CRC(8daf2447) SHA1(59c0af6c46c04ccd0eff90070de4fe08c5a47b42))
	ROM_LOAD("crash3.u10", 0x002000, 0x001000, CRC(4a41c86d) SHA1(baffb30659bb628d4fd9a382dc851683aad92b88))
	ROM_LOAD("crash4.u9", 0x003000, 0x001000, CRC(b30876a9) SHA1(fe9f906f38ad9354d043794d6c22d07c78672261))
	ROM_LOAD("ride2a1.u8", 0x004000, 0x001000, CRC(17c960e8) SHA1(5f4681979c6339ff1c38dc90257d14a0a7516c52))
	ROM_LOAD("ride2a2.u7", 0x005000, 0x001000, CRC(5b2198d0) SHA1(5e008c76f949a6357c1160e00d43631585702d45))
	ROM_LOAD("ride2a3.u6", 0x006000, 0x001000, CRC(4f67cacb) SHA1(8abc2c6769bd315c489969486de02b567348ea94))
	ROM_LOAD("ride2a4.u5", 0x007000, 0x001000, CRC(25656751) SHA1(46e5f70da82f8845dd820cbcf942bd0e7e328d56))
ROM_END

}  // anonymous namespace

SYST(1980, obdmx, 0, 0, dmx, dmx, dmx_state, empty_init, "Oberheim", "DMX", MACHINE_SUPPORTS_SAVE)
