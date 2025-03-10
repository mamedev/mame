// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
The LinnDrum (unofficially also known as LM-2) is a digital drum machine. It
stores and plays digital samples (recordings of acoustic dumps), some of which
are processed by analog filters or other analog circuits.

The firmware runs on a Z80. It controls the UI (reads buttons, drives displays
and LEDs), synchronization with other devices, cassette I/O, and voice
triggering.

The LinnDrum has 10 voice cores, not including the "click" (metronome) and
"beep" sounds. Many of the voices have variations (loudness, pitch, sample
selection, decay time), bringing the number of possible sounds to 28. However,
end-users can only trigger 23 sounds, and can control mixing and panning
for 15 of them (some are grouped together). Each voice core can run
independently, for a max polyphony of 10. Only one variation per core can be
active at a time. The "click" and "beep" sounds can be played independently of
each other and the voice cores.

There are multiple voice architectures:

* The "mux drums" section consists of 8 voice cores. Bass, cabasa, tambourine,
  clap and cowbell (4K samples each), hi-hat (16K) and ride and crash cymbals
  (32K each). Voices other than the clap and cowbell can be attenuated, to
  create 2 loudness variations. The clap and cowbell always play at full volume.
  There is a single, time-multiplexed DAC used for all voices, and each voice
  has its own output. All 8 voices can be played simultaneously, but only a
  single loudness variation of each. The bass drum is post-processed by a
  CEM3320 VCF with a hardcoded envelope, but all other voices lack a
  reconstruction filter. The hat is post-processed by a CEM3360 VCA, which can
  operate in 2 variations: slow decay (open hat) and faster, user-controlled
  decay (closed hat). There's a single clock for all voices, which is tuned by
  a trimmer on the circuit board.

* The "snare / sidestick" section consists of a single voice core (and single
  DAC) which can either play the sidestick, or the snare voice (4K samples
  each). The two voices have individual outputs, and their volume and pan can be
  set independently. There are 4 possible loudness variations for each voice,
  though the end-user only has access to 3 for the snare and 1 for the
  sidestick. The voice core output is post-processed by a single-pole lowpass RC
  filter. The end-user can control the sample rate via a tuning knob.
  This section also includes the circuit for the "click" sound (metronome),
  which triggers pulses of fixed length, and the circuit for  the "beep" sound,
  which allows the firmware to generate pulses of arbitrary length.

* The "tom / conga" section is similar to the "snare / sidestick" one. It
  consists of a single voice core (and single DAC) which can either play the tom
  or the conga voice (8K samples each). There are 3 pitch variations for the
  tom, and 2 pitch variations for the conga. The end-user can set the 5 pitches
  using the corresponding tuning knobs. Each of those 5 variations has its own
  output, and can be mixed and panned independently, even though only a single
  one can be active at a time. All variations are post-processed by a CEM3320
  VCF with a hardcoded envelope.

The driver is based on the LinnDrum's service manual and schematics, and is
intended as an educational tool.

Most of the digital functionality is emulated. Audio is partially emulated.

Reasons for MACHINE_IMPERFECT_SOUND:
* Missing a few sample checksums.
* Volume and pan sliders don't work yet.
* Tuning knobs and trimmers don't work yet (no tom and conga variations).
* Missing bass drum LPF and filter envelope.
* Missing snare / sidestick volume envelope.
* Missing hi-hat volume envelope (open and closed hats will sound the same.
  Decay knob is inoperative).
* Missing tom / conga LPF and filter envelope.

PCBoards:
* CPU board. 2 sections in schematics:
  * Computer.
  * Input/Output.
* DRM board. 4 sections in schematics:
  * MUX drums.
  * MUX drum output stage.
  * Snare/Sidestick.
  * Tom/Cga.
* MXR board.
* 5 VSR board (power supply).

Usage:

The driver includes an interactive layout. Make sure to enable plugins so that
sliders and knobs can be manipulated with the mouse. It is recommended to run
with a high sample rate.

Example:
./mame -window -samplerate 96000 -plugins -plugin layout linndrum
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/output_latch.h"
#include "machine/rescap.h"
#include "machine/timer.h"
#include "sound/dac76.h"
#include "sound/flt_rc.h"
#include "sound/flt_vol.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "linn_linndrum.lh"

#define LOG_KEYBOARD         (1U << 1)
#define LOG_DEBOUNCE         (1U << 2)
#define LOG_TEMPO            (1U << 3)
#define LOG_TEMPO_CHANGE     (1U << 4)
#define LOG_STROBES          (1U << 5)
#define LOG_TAPE_SYNC_ENABLE (1U << 6)
#define LOG_VOLUME           (1U << 7)

#define VERBOSE (LOG_GENERAL | LOG_STROBES)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

namespace {

enum mux_voices
{
	TAMBOURINE = 0,
	CABASA,
	CLAP,
	COWBELL,
	BASS,
	HAT,
	RIDE,
	CRASH,
	NUM_MUX_VOICES
};

}  // anonymous namespace


class linndrum_audio_device : public device_t
{
public:
	linndrum_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;

	void voice_data_enable_w(int state);
	template <int Voice> void strobe_mux_drum_w(u8 data);
	void strobe_snare_w(u8 data);  // Snare and sidestick.
	void strobe_tom_w(u8 data);  // Tom and conga.
	void strobe_click_w(u8 data);
	void beep_w(int state);

protected:
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void device_start() override ATTR_COLD;

private:
	static void write_dac(dac76_device& dac, u8 sample);

	void set_mux_drum_volume(int voice, bool d1);
	void set_snare_volume(bool d3, bool d2);
	u8 get_voice_data(u8 data) const;

	TIMER_DEVICE_CALLBACK_MEMBER(mux_timer_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(snare_timer_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(click_timer_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(tom_timer_tick);

	// Mux drums.
	required_memory_region_array<NUM_MUX_VOICES> m_mux_samples;
	required_device_array<dac76_device, NUM_MUX_VOICES> m_mux_dac;  // AM6070 (U88).
	required_device_array<filter_volume_device, NUM_MUX_VOICES> m_mux_volume;  // CD4053 (U90), R60, R62.
	std::array<bool, NUM_MUX_VOICES> m_mux_counting = { false, false, false, false, false, false, false, false };
	std::array<u16, NUM_MUX_VOICES> m_mux_counters = { 0, 0, 0, 0, 0, 0, 0, 0 };

	// Snare / sidestick.
	required_memory_region m_snare_samples;  // 2732 ROM (U79).
	required_memory_region m_sidestick_samples;  // 2732 ROMs (U78).
	required_device<dac76_device> m_snare_dac;  // AM6070 (U92).
	required_device<filter_volume_device> m_snare_volume;  // R69, R72, R71, R70.
	required_device<timer_device> m_click_timer;  // 556 (U65A).
	required_device<speaker_sound_device> m_click;
	required_device<speaker_sound_device> m_beep;
	bool m_snare_counting = false;  // /Q1 of U41 (74LS74).
	u16 m_snare_counter = 0;  // 13-bit counter (2 x 74LS393, U61, U62).
	bool m_sidestick_selected = false;  // Chooses between snare and sidestick.

	// Tom / conga.
	required_memory_region m_tom_samples;  // 2 x 2732 ROMs (U68, U69).
	required_memory_region m_conga_samples;  // 2 x 2732 ROMs (U66, U67).
	required_device<timer_device> m_tom_timer;  // 74LS627 (U77B).
	required_device<dac76_device> m_tom_dac;  // AM6070 (U82).
	bool m_tom_counting = false;  // /Q1 of U73 (74LS74).
	u16 m_tom_counter = 0;  // 14-bit counter (2 x 74LS393, U70, U71).
	bool m_tom_selected = false;  // Selects between tom and conga.
	u8 m_tom_variation = 0;  // Tom / conga pitch variation.

	bool m_voice_data_enabled = false;  // Enables/disables U19 (74LS365).

	static constexpr const float VPLUS = 15;  // Volts.
	static constexpr const float VCC = 5;  // Volts.
	static constexpr const char *MUX_VOICE_NAMES[NUM_MUX_VOICES] =
	{
		"TAMBOURINE", "CABASA", "CLAP", "COWBELL", "BASS", "HAT", "RIDE", "CRASH"
	};
};

DEFINE_DEVICE_TYPE(LINNDRUM_AUDIO, linndrum_audio_device, "linndrum_audio_device", "LinnDrum audio circuits");

linndrum_audio_device::linndrum_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LINNDRUM_AUDIO, tag, owner, clock)
	, m_mux_samples(*this, ":sample_mux_drum_%u", 0)
	, m_mux_dac(*this, "mux_drums_virtual_dac_%u", 1)
	, m_mux_volume(*this, "mux_drums_volume_control_%u", 1)
	, m_snare_samples(*this, ":sample_snare")
	, m_sidestick_samples(*this, ":sample_sidestick")
	, m_snare_dac(*this, "snare_sidestick_dac")
	, m_snare_volume(*this, "snare_sidestick_volume")
	, m_click_timer(*this, "click_timer")
	, m_click(*this, "click")
	, m_beep(*this, "beep")
	, m_tom_samples(*this, ":sample_tom")
	, m_conga_samples(*this, ":sample_conga")
	, m_tom_timer(*this, "tom_conga_timer")
	, m_tom_dac(*this, "tom_conga_dac")
{
}

void linndrum_audio_device::voice_data_enable_w(int state)
{
	// Controls whether data (D0-D3) is transmitted to the voice circuits. This
	// is done by U19 (74LS365 hex buffer. Enable inputs are active-low).
	// This is usually disabled to prevent interference from the "noisy" data
	// bus to the voice circuits.
	m_voice_data_enabled = !state;
}

template <int Voice> void linndrum_audio_device::strobe_mux_drum_w(u8 data)
{
	static_assert(Voice >= 0 && Voice < NUM_MUX_VOICES);
	data = get_voice_data(data);

	m_mux_counting[Voice] = BIT(data, 0);
	if (!m_mux_counting[Voice])
		m_mux_counters[Voice] = 0;

	set_mux_drum_volume(Voice, BIT(data, 1));

	LOGMASKED(LOG_STROBES, "Strobed mux drum %s, %02x\n", MUX_VOICE_NAMES[Voice], data);
}

void linndrum_audio_device::strobe_snare_w(u8 data)
{
	data = get_voice_data(data);

	m_snare_counting = BIT(data, 0);
	if (!m_snare_counting)
		m_snare_counter = 0;

	m_sidestick_selected = BIT(data, 1);  // Play sidestick instead of snare.
	set_snare_volume(BIT(data, 3), BIT(data, 2));

	LOGMASKED(LOG_STROBES, "Strobed snare / sidestick: %02x\n", data);
}

void linndrum_audio_device::strobe_tom_w(u8 data)
{
	data = get_voice_data(data);

	m_tom_counting = BIT(data, 0);
	if (!m_tom_counting)
		m_tom_counter = 0;

	m_tom_selected = BIT(data, 1);  // Play tom instead of conga.
	m_tom_variation = (data >> 2) & 0x03;  // D3, D2.

	LOGMASKED(LOG_STROBES, "Strobed tom / conga: %02x\n", data);
}

void linndrum_audio_device::strobe_click_w(u8 /*data*/)
{
	static constexpr const float R10 = RES_K(100);
	static constexpr const float C12 = CAP_U(0.01);
	m_click_timer->adjust(PERIOD_OF_555_MONOSTABLE(R10, C12));
	m_click->level_w(1);
	LOGMASKED(LOG_STROBES, "Strobed click.\n");
}

void linndrum_audio_device::beep_w(int state)
{
	// Beep signal is inverted by U76B (74LS00).
	m_beep->level_w(state ? 0 : 1);
	LOGMASKED(LOG_STROBES, "Beep: %d\n", state);
}

void linndrum_audio_device::device_add_mconfig(machine_config &config)
{
	// Sample rate is controlled by 74LS627 timers. There isn't a formula that
	// computes frequency for these chips. The datasheet just provides graphs.
	// The values below were obtained by eyballing those graphs, and then
	// adjusting by ear to youtube videos, while sticking to round numbers.
	// TODO: Implement control by potentiomenters.
	static constexpr const int MUX_DRUM_SAMPLE_RATE = 30000;
	static constexpr const int SNARE_SAMPLE_RATE = 28000;
	static constexpr const int TOM_SAMPLE_RATE = 32000;

	// *** Mux drums section.

	TIMER(config, "mux_drum_timer").configure_periodic(  // 74LS627 (U77A).
		FUNC(linndrum_audio_device::mux_timer_tick), attotime::from_hz(MUX_DRUM_SAMPLE_RATE));

	// The actual "mux drums" hardware has a single AM6070, which is
	// time-multiplexed across the 8 voices. Implementing it that way is
	// possible, but requires a sample rate of at least 240KHz (8 x ~30K) for
	// reasonable results. It also requires emulating audio sample & hold
	// functionality. So 8 "virtual" DACs and volume-control MUXes are used
	// instead.
	for (int voice = 0; voice < NUM_MUX_VOICES; ++voice)
	{
		DAC76(config, m_mux_dac[voice], 0);  // AM6070 (U88).
		FILTER_VOLUME(config, m_mux_volume[voice]);  // CD4053 (U90), R60, R62 (see set_mux_drum_volume()).
		m_mux_dac[voice]->add_route(ALL_OUTPUTS, m_mux_volume[voice], 1.0);
	}

	// *** Snare / sidestick section.

	TIMER(config, "snare_timer").configure_periodic(  // 74LS627 (U80A).
		FUNC(linndrum_audio_device::snare_timer_tick), attotime::from_hz(SNARE_SAMPLE_RATE));
	DAC76(config, m_snare_dac, 0);  // AM6070 (U92)
	FILTER_VOLUME(config, m_snare_volume);  // See set_snare_volume().
	m_snare_dac->add_route(ALL_OUTPUTS, m_snare_volume, 1.0);

	// The DAC's current outputs are processed by a current-to-voltage converter
	// that embeds an RC filter. This consists of an op-amp (U103), R127 and C65
	// (for positive voltages), and R126 and C31 (for negative voltages). The
	// two resistors and capacitors have the same value.
	filter_rc_device &snare_dac_filter = FILTER_RC(config, "snare_sidestick_dac_filter");
	snare_dac_filter.set_lowpass(RES_K(2.49), CAP_P(2700));  // R127-C65, R126-C31.
	m_snare_volume->add_route(ALL_OUTPUTS, snare_dac_filter, 1.0);

	TIMER(config, m_click_timer).configure_generic(FUNC(linndrum_audio_device::click_timer_tick));  // 556 (U65A).
	SPEAKER_SOUND(config, m_click);
	SPEAKER_SOUND(config, m_beep);

	// *** Tom / conga section.

	TIMER(config, m_tom_timer).configure_periodic(  // 74LS627 (U77B).
		FUNC(linndrum_audio_device::tom_timer_tick), attotime::from_hz(TOM_SAMPLE_RATE));
	DAC76(config, m_tom_dac, 0);  // AM6070 (U82).

	// *** Output.

	// TODO: Implement mixing and panning.
	speaker_device &speaker = SPEAKER(config, "monospeaker").front_center();
	for (int i = 0; i < NUM_MUX_VOICES; ++i)
		m_mux_volume[i]->add_route(ALL_OUTPUTS, speaker, 1.0);
	snare_dac_filter.add_route(ALL_OUTPUTS, speaker, 1.0);
	m_click->add_route(ALL_OUTPUTS, speaker, 1.0);
	m_beep->add_route(ALL_OUTPUTS, speaker, 1.0);
	m_tom_dac->add_route(ALL_OUTPUTS, speaker, 1.0);
}

void linndrum_audio_device::device_start()
{
	save_item(NAME(m_mux_counting));
	save_item(NAME(m_mux_counters));
	save_item(NAME(m_snare_counting));
	save_item(NAME(m_snare_counter));
	save_item(NAME(m_sidestick_selected));
	save_item(NAME(m_tom_counting));
	save_item(NAME(m_tom_counter));
	save_item(NAME(m_tom_selected));
	save_item(NAME(m_tom_variation));
	save_item(NAME(m_voice_data_enabled));
}

void linndrum_audio_device::write_dac(dac76_device& dac, u8 sample)
{
	dac.update();
	dac.sb_w(BIT(sample, 7));
	dac.b1_w(BIT(sample, 6));
	dac.b2_w(BIT(sample, 5));
	dac.b3_w(BIT(sample, 4));
	dac.b4_w(BIT(sample, 3));
	dac.b5_w(BIT(sample, 2));
	dac.b6_w(BIT(sample, 1));
	dac.b7_w(BIT(sample, 0));
}

void linndrum_audio_device::set_mux_drum_volume(int voice, bool d1)
{
	// Volume variations are controlled by a CD4053 MUX (U90B), whose "select"
	// input is connected to the active voice's D1 (via a 74LS151 encoder, U35).
	// Depending on how the mux is configured, the audio signal will either
	// remain unchanged, or it will get attenuated by a voltage divider.
	// The MUX is always configured in the "no attenuation" mode for the clap
	// and cowbell voices.

	static constexpr const float ATTENUATION = RES_VOLTAGE_DIVIDER(RES_K(10), RES_K(3.3));  // R60, R62.

	const bool attenuate = !d1 && voice != CLAP && voice != COWBELL;
	assert(voice >= 0 && voice < NUM_MUX_VOICES);
	m_mux_volume[voice]->set_gain(attenuate ? ATTENUATION : 1.0);

	LOGMASKED(LOG_VOLUME, "Mux drum %s gain: %f\n", MUX_VOICE_NAMES[voice], m_mux_volume[voice]->gain());
}

void linndrum_audio_device::set_snare_volume(bool d3, bool d2)
{
	// Snare and sidestick volume is set by controlling the reference current to
	// the DAC. Bits D2 and D3 from the voice data bus (latched by U42, 74LS74)
	// control the voltage at the end of R72 and R71.

	static constexpr const float R0 = RES_K(3.3);  // R70.
	static constexpr const float R1 = RES_K(380);  // R69.
	static constexpr const float R2 = RES_K(22);   // R72.
	static constexpr const float R3 = RES_K(5.6);  // R71.

	static constexpr const float a = R0 * R2 * R3;
	static constexpr const float b = R0 * R1 * R3;
	static constexpr const float c = R0 * R1 * R2;
	static constexpr const float d = R1 * R2 * R3;

	static constexpr const float MAX_IREF = (a * VPLUS + b * VCC + c * VCC) / (R0 * (a + b + c + d));

	const float v2 = d2 ? VCC : 0;
	const float v3 = d3 ? VCC : 0;
	const float iref = (a * VPLUS + b * v2 + c * v3) / (R0 * (a + b + c + d));
	m_snare_volume->set_gain(iref / MAX_IREF);

	LOGMASKED(LOG_VOLUME, "Snare volume - iref: %f, max_iref: %f, gain: %f\n",
	          iref, MAX_IREF, iref / MAX_IREF);
}

u8 linndrum_audio_device::get_voice_data(u8 data) const
{
	if (m_voice_data_enabled)
	{
		return data & 0x0f;  // Voice data bus is 4 bits wide.
	}
	else
	{
		LOG("Firmware bug: floating voice data bus when strobing a voice.\n");
		return 0x0f;  // Floating TTL inputs. Will likely resolve to 1s.
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(linndrum_audio_device::mux_timer_tick)
{
	// The timer on the actual hardware ticks 4 times per voice. A combination
	// of counters, latches, decoders, encoders and analog multiplexers achieves
	// the following:
	// Tick 0: Selects next voice and enables DAC.
	//         - ROM address counter is incremented, if voice is enabled.
	//           If the counter reaches its max, the voice will be disabled and
	//           the counter will get cleared.
	//         - ROM is enabled, and outputs are written to the DAC.
	//         - Volume variation is configured (for voices that support it).
	//         - If the voice is not enabled, the ROM address will be 0, which
	//           according to the service manual always stores a 0.
	// Tick 1: No-op. Waits for DAC to settle.
	// Tick 2: Sample & Hold (S&H) for the selected voice is enabled.
	// Tick 3: S&H is disabled. DAC is disabled, driving its output to 0.

	// The emulation here does all of the above, for all voices, in a single
	// timer tick. The timer period has been adjusted accordingly.

	for (int voice = 0; voice < NUM_MUX_VOICES; ++voice)
	{
		if (m_mux_counting[voice])
		{
			++m_mux_counters[voice];
			if (m_mux_counters[voice] >= m_mux_samples[voice]->bytes())
			{
				// All bits in the voice's D0 and D1 latch (74LS74) are cleared,
				// resulting in:
				m_mux_counting[voice] = false;
				m_mux_counters[voice] = 0;
				set_mux_drum_volume(voice, false);
			}
		}

		const u8 sample = m_mux_samples[voice]->as_u8(m_mux_counters[voice]);
		write_dac(*m_mux_dac[voice], sample);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(linndrum_audio_device::snare_timer_tick)
{
	if (!m_snare_counting)
		return;

	++m_snare_counter;
	if (BIT(m_snare_counter, 12))  // Counter reached 0x1000 (4096).
	{
		// All outputs of U41 and U42 (74LS74 flip-flops) are cleared, resulting in:
		m_snare_counting = false;
		m_snare_counter = 0;
		m_sidestick_selected = false;
		set_snare_volume(false, false);
		write_dac(*m_snare_dac, 0);  // DAC is disabled. Output goes to 0.
		return;
	}

	u8 sample = 0;
	if (m_sidestick_selected)
		sample = m_sidestick_samples->as_u8(m_snare_counter);
	else
		sample = m_snare_samples->as_u8(m_snare_counter);
	write_dac(*m_snare_dac, sample);
}

TIMER_DEVICE_CALLBACK_MEMBER(linndrum_audio_device::click_timer_tick)
{
	m_click->level_w(0);
}

TIMER_DEVICE_CALLBACK_MEMBER(linndrum_audio_device::tom_timer_tick)
{
	if (!m_tom_counting)
		return;

	++m_tom_counter;
	if (BIT(m_tom_counter, 13))  // Counter reached 0x2000 (8192).
	{
		// All outputs of U42B and U73B (74LS74 flip-flops) are cleared, resulting in:
		m_tom_counting = false;
		m_tom_counter = 0;
		m_tom_selected = false;
		m_tom_variation = 0;
		write_dac(*m_tom_dac, 0);  // DAC is disabled. Output goes to 0.
		return;
	}

	u8 sample = 0;
	if (m_tom_selected)
		sample = m_tom_samples->as_u8(m_tom_counter);
	else
		sample = m_conga_samples->as_u8(m_tom_counter);
	write_dac(*m_tom_dac, sample);
}

namespace {

constexpr const char MAINCPU_TAG[] = "z80";
constexpr const char NVRAM_TAG[] = "nvram";

class linndrum_state : public driver_device
{
public:
	static constexpr feature_type unemulated_features() { return feature::TAPE; }

	linndrum_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MAINCPU_TAG)
		, m_audio(*this, "linndrum_audio")
		, m_tempo_timer(*this, "tempo_timer_556_u30a")
		, m_debounce_timer(*this, "debounce_timer_556_u30b")
		, m_keyboard(*this, "keyboard_col_%d", 0)
		, m_playstop(*this, "play_stop")
		, m_triggers(*this, "trigger_inputs")
		, m_tempo_pot(*this, "pot_tempo")
		, m_step_display(*this, "display_step_%d", 0U)
		, m_pattern_display(*this, "display_pattern_%d", 0U)
		, m_tape_sync_out(*this, "output_tape_sync")
	{
	}

	void linndrum(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(tempo_pot_adjusted);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	u8 keyboard_r(offs_t offset);
	u8 inport_r();
	template<int Display> void display_w(u8 data);

	u8 start_debounce_r();
	void start_debounce_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(debounce_timer_elapsed);

	void update_tempo_timer();
	TIMER_DEVICE_CALLBACK_MEMBER(tempo_timer_tick);

	void tape_sync_enable_w(int state);
	void update_tape_sync_out();

	void memory_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<linndrum_audio_device> m_audio;
	required_device<timer_device> m_tempo_timer;  // 556, U30A.
	required_device<timer_device> m_debounce_timer;  // 556, U30B.
	required_ioport_array<6> m_keyboard;
	required_ioport m_playstop;
	required_ioport m_triggers;
	required_ioport m_tempo_pot;
	output_finder<2> m_step_display;  // 2 x MAN4710.
	output_finder<2> m_pattern_display;  // 2 x MAN4710.
	output_finder<> m_tape_sync_out;

	bool m_debouncing = false;
	bool m_tempo_state = false;
	bool m_tape_sync_enabled = false;

	static constexpr const int DISPLAY_STEP = 0;
	static constexpr const int DISPLAY_PATTERN = 1;
};

u8 linndrum_state::keyboard_r(offs_t offset)
{
	// Columns in the keyboard matrix are selected by A0-A5. Each of these
	// address lines are inverted by U29 (74LS05). A high at Ax enables column
	// x. If more than one of A0-A5 are set, multiple columns will be selected.
	// Rows are pulled up by a resistor array (RP8), buffered by U28 (74LS244)
	// and connected to D0-D5.
	u8 selected = 0x3f;  // D0-D5.
	for (int i = 0; i < 6; ++i)
		if (BIT(offset, i))
			selected &= m_keyboard[i]->read();

	const u8 d6 = m_debouncing ? 1 : 0;

	// The play/stop button is not part of the keyboard matrix. It is always
	// read at D7, regardless of the selected column. The play/stop button and
	// footswitch are wired such that if any of them is grounded (activated) a
	// 0 will be returned in D7.
	const bool play_button_pressed = !(m_playstop->read() & 0x01);
	const bool play_footswitch_pressed = !(m_playstop->read() & 0x02);
	const u8 d7 = (play_button_pressed || play_footswitch_pressed) ? 0 : 1;

	if (selected != 0x3f || d7 == 0)
	{
		LOGMASKED(LOG_KEYBOARD,
				"Offset: %02x, keys: %02x, debounce: %d, play: %d\n",
				offset, selected, d6, d7);
	}

	return (d7 << 7) | (d6 << 6) | selected;
}

u8 linndrum_state::inport_r()
{
	const u8 d0 = m_tempo_state ? 1 : 0;  // Tempo.

	// The cassette input and sync input circuits are identical.
	// TODO: Determine value at rest.
	const u8 d1 = 0;  // Sync input. TODO: implement.
	const u8 d2 = 0;  // Cassette input. TODO: implement.

	const u8 triggers = m_triggers->read() & 0x1f;

	return (triggers << 3) | (d2 << 2) | (d1 << 1) | d0;
}

template<int Display> void linndrum_state::display_w(u8 data)
{
	static constexpr const u8 PATTERNS[16] = // 4 x 74LS47 (U24-U27).
	{
		0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07,
		0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0x00,
	};

	const u8 ms_digit = PATTERNS[(data >> 4) & 0x0f];
	const u8 ls_digit = PATTERNS[data & 0x0f];

	static_assert(Display == DISPLAY_STEP || Display == DISPLAY_PATTERN);
	if (Display == DISPLAY_STEP)
	{
		m_step_display[0] = ms_digit;
		m_step_display[1] = ls_digit;
	}
	else
	{
		m_pattern_display[0] = ms_digit;
		m_pattern_display[1] = ls_digit;
	}
}

u8 linndrum_state::start_debounce_r()
{
	if (!machine().side_effects_disabled())
		start_debounce_w(0);
	return 0x00;  // D0-D7 pulled low by RP10.
}

void linndrum_state::start_debounce_w(u8 /*data*/)
{
	static constexpr const float R3 = RES_K(30);
	static constexpr const float C3 = CAP_U(0.1);
	static constexpr attotime INTERVAL = PERIOD_OF_555_MONOSTABLE(R3, C3);

	m_debouncing = true;
	m_debounce_timer->adjust(INTERVAL);
	LOGMASKED(LOG_DEBOUNCE, "Debounce timer: start\n");
}

TIMER_DEVICE_CALLBACK_MEMBER(linndrum_state::debounce_timer_elapsed)
{
	m_debouncing = false;
	LOGMASKED(LOG_DEBOUNCE, "Debounce timer: elapsed\n");
}

void linndrum_state::update_tempo_timer()
{
	static constexpr const float R1 = RES_K(5.1);
	static constexpr const float R2 = RES_K(18);
	static constexpr const float C2 = CAP_U(0.1);
	static constexpr const float P1_MAX = RES_K(100);

	// Using `100 - pot value` because the higher (the more clockwise) the pot
	// is turned, the lower the resistance and the fastest the tempo.
	const float tempo_r = (100 - m_tempo_pot->read()) * P1_MAX / 100.0F;
	const attotime period = PERIOD_OF_555_ASTABLE(R1, R2 + tempo_r, C2);
	m_tempo_timer->adjust(period, 0, period);
	LOGMASKED(LOG_TEMPO_CHANGE, "Tempo adjusted: %f\n", period.as_double());
}

TIMER_DEVICE_CALLBACK_MEMBER(linndrum_state::tempo_timer_tick)
{
	// The output of the 556 clocks an 74LS74 (U8B) that is wired in a
	// divide-by-2 configuration. Each time the timer elapses, m_tempo_state
	// will get negated.
	m_tempo_state = !m_tempo_state;
	update_tape_sync_out();
	LOGMASKED(LOG_TEMPO, "Tempo timer elapsed: %d\n", m_tempo_state);
}

void linndrum_state::tape_sync_enable_w(int state)
{
	LOGMASKED(LOG_TAPE_SYNC_ENABLE, "Tape sync enable: %d\n", state);
	m_tape_sync_enabled = bool(state);
	update_tape_sync_out();
}

void linndrum_state::update_tape_sync_out()
{
	// tape_sync_enable (U16 latch, O5) is NANDed with the current tempo
	// state (U34, 74LS00). The output is then inverted by Q3/R20/R21.
	if (m_tape_sync_enabled)
		m_tape_sync_out = m_tempo_state ? 1 : 0;
	else
		m_tape_sync_out = 0;
}

void linndrum_state::memory_map(address_map &map)
{
	// Signal names (such as "/READ INPORT") are taken from the schematics.

	map.global_mask(0x3fff);  // A14 and A15 are not used.

	map(0x0000, 0x1f7f).rom();  // 2 x 2732. U14-U13.
	map(0x1f80, 0x1f80).mirror(0x003f).r(FUNC(linndrum_state::inport_r));  // /READ INPORT.

	// Writes to: 0x1f80 - 0x1fbf: /OUT STROBE EN.
	// Outputs are selected by two 74LS138 (U20, U21). U20 is enabled when A3 is
	// 0, U21 when it is 1.
	map(0x1f80, 0x1f80).mirror(0x0030).w(FUNC(linndrum_state::display_w<DISPLAY_PATTERN>));  // U23 latch.
	map(0x1f81, 0x1f81).mirror(0x0030).w(FUNC(linndrum_state::display_w<DISPLAY_STEP>));  // U22 latch.
	map(0x1f82, 0x1f82).mirror(0x0030).w("latch_u18", FUNC(output_latch_device::write));  // LEDs.
	map(0x1f83, 0x1f83).mirror(0x0030).w("latch_u17", FUNC(output_latch_device::write));  // LEDs.
	map(0x1f84, 0x1f84).mirror(0x0030).w("latch_u16", FUNC(output_latch_device::write));  // LEDs & outputs.

	// Voice strobes.
	map(0x1f85, 0x1f85).mirror(0x0030).w(m_audio, FUNC(linndrum_audio_device::strobe_mux_drum_w<BASS>));
	map(0x1f86, 0x1f86).mirror(0x0030).w(m_audio, FUNC(linndrum_audio_device::strobe_snare_w));
	map(0x1f87, 0x1f87).mirror(0x0030).w(m_audio, FUNC(linndrum_audio_device::strobe_mux_drum_w<HAT>));
	map(0x1f88, 0x1f88).mirror(0x0030).w(m_audio, FUNC(linndrum_audio_device::strobe_tom_w));
	map(0x1f89, 0x1f89).mirror(0x0030).w(m_audio, FUNC(linndrum_audio_device::strobe_mux_drum_w<RIDE>));
	map(0x1f8a, 0x1f8a).mirror(0x0030).w(m_audio, FUNC(linndrum_audio_device::strobe_mux_drum_w<CRASH>));
	map(0x1f8b, 0x1f8b).mirror(0x0030).w(m_audio, FUNC(linndrum_audio_device::strobe_mux_drum_w<CABASA>));
	map(0x1f8c, 0x1f8c).mirror(0x0030).w(m_audio, FUNC(linndrum_audio_device::strobe_mux_drum_w<TAMBOURINE>));
	map(0x1f8d, 0x1f8d).mirror(0x0030).w(m_audio, FUNC(linndrum_audio_device::strobe_mux_drum_w<COWBELL>));
	map(0x1f8e, 0x1f8e).mirror(0x0030).w(m_audio, FUNC(linndrum_audio_device::strobe_mux_drum_w<CLAP>));
	map(0x1f8f, 0x1f8f).mirror(0x0030).w(m_audio, FUNC(linndrum_audio_device::strobe_click_w));

	map(0x1fc0, 0x1fff).r(FUNC(linndrum_state::keyboard_r));  // /READ KEYBD.
	map(0x2000, 0x3fff).ram().share(NVRAM_TAG); // 4 x HM6116LP4. U12-U9.
}

void linndrum_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	// The debouncing timer is started by the Z80's /IORQ signal itself. So
	// there is no difference between read and write, and the address and data
	// bits don't matter.
	map(0x00, 0x00).mirror(0xff).r(FUNC(linndrum_state::start_debounce_r)).w(FUNC(linndrum_state::start_debounce_w));
}

void linndrum_state::machine_start()
{
	m_step_display.resolve();
	m_pattern_display.resolve();
	m_tape_sync_out.resolve();

	save_item(NAME(m_debouncing));
	save_item(NAME(m_tempo_state));
	save_item(NAME(m_tape_sync_enabled));
}

void linndrum_state::machine_reset()
{
	update_tempo_timer();
}

void linndrum_state::linndrum(machine_config &config)
{
	// D0-D7 and A0-A11 are pulled low by RP10 and RP9.
	Z80(config, m_maincpu, 4_MHz_XTAL / 2);  // Divided by 74LS74, U8A.
	m_maincpu->set_addrmap(AS_PROGRAM, &linndrum_state::memory_map);
	m_maincpu->set_addrmap(AS_IO, &linndrum_state::io_map);

	NVRAM(config, NVRAM_TAG, nvram_device::DEFAULT_ALL_0);

	TIMER(config, m_tempo_timer).configure_generic(  // 556, U30A.
		FUNC(linndrum_state::tempo_timer_tick));
	TIMER(config, m_debounce_timer).configure_generic(  // 556, U30B.
		FUNC(linndrum_state::debounce_timer_elapsed));

	LINNDRUM_AUDIO(config, m_audio);

	config.set_default_layout(layout_linn_linndrum);

	// Latches connected to cathodes of LEDs (through resistors), so they are
	// active-low.

	output_latch_device &u18(OUTPUT_LATCH(config, "latch_u18"));
	u18.bit_handler<0>().set_output("led_1_8").invert();
	u18.bit_handler<1>().set_output("led_1_8_t").invert();
	u18.bit_handler<2>().set_output("led_1_16").invert();
	u18.bit_handler<3>().set_output("led_1_16_t").invert();
	u18.bit_handler<4>().set_output("led_1_32").invert();
	u18.bit_handler<5>().set_output("led_1_32_t").invert();
	u18.bit_handler<6>().set_output("led_hi").invert();
	u18.bit_handler<7>().set_output("led_play_stop").invert();

	output_latch_device &u17(OUTPUT_LATCH(config, "latch_u17"));
	u17.bit_handler<0>().set_output("led_a").invert();
	u17.bit_handler<1>().set_output("led_b").invert();
	u17.bit_handler<2>().set_output("led_c").invert();
	u17.bit_handler<3>().set_output("led_d").invert();
	u17.bit_handler<4>().set_output("led_e").invert();
	u17.bit_handler<5>().set_output("led_f").invert();
	u17.bit_handler<6>().set_output("led_load").invert();
	u17.bit_handler<7>().set_output("led_store").invert();

	output_latch_device &u16(OUTPUT_LATCH(config, "latch_u16"));
	u16.bit_handler<0>().set_output("led_percussion").invert();
	u16.bit_handler<1>().set_output("led_ext_sync").invert();
	u16.bit_handler<2>().set_output("led_pattern").invert();
	u16.bit_handler<2>().append_output("led_song");  // Inverted by U31A.
	u16.bit_handler<3>().set(m_audio, FUNC(linndrum_audio_device::voice_data_enable_w));
	u16.bit_handler<4>().set(m_audio, FUNC(linndrum_audio_device::beep_w));
	u16.bit_handler<5>().set(FUNC(linndrum_state::tape_sync_enable_w));
	// Output voltage divided by R24/R25 to 0V - 2.5V.
	u16.bit_handler<6>().set_output("output_cassette");
	// Inverted by Q4/R22/R23.
	u16.bit_handler<7>().set_output("output_trigger").invert();
}

DECLARE_INPUT_CHANGED_MEMBER(linndrum_state::tempo_pot_adjusted)
{
	update_tempo_timer();
}

// PORT_NAMEs are based on the annotations in the schematic.
INPUT_PORTS_START(linndrum)
	PORT_START("keyboard_col_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("6") PORT_CODE(KEYCODE_6)

	PORT_START("keyboard_col_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SONG/PAT.") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SONG#") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("END") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DELETE") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("INSERT") PORT_CODE(KEYCODE_T)

	PORT_START("keyboard_col_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("<-") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("->") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ENTER") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("STORE")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LOAD")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EXT.SYNC") PORT_CODE(KEYCODE_L)

	PORT_START("keyboard_col_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BPM/TRIG.") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SIDESTICK") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SNARE 1") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SNARE 2") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SNARE 3") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BASS 1") PORT_CODE(KEYCODE_B)

	PORT_START("keyboard_col_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BASS 2") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CRASH") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PERC.") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CABASA1 / HIHAT1") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CABASA2 / HIHAT2") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TAMP 1 / HIHAT O") PORT_CODE(KEYCODE_D)

	PORT_START("keyboard_col_5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TAMP 2 / HI TOM") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("HI CONGA / MID TOM") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LO CONGA / LO TOM") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("COWBELL / RIDE 1") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CLAPS / RIDE 2") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	// The play-stop button and footswitch input are both connected together
	// in a way that if any of them is low, D6 of the keyboard port will read
	// low.
	PORT_START("play_stop")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PLAY/STOP") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PLAY/STOP FOOTSWITCH")

	// The trigger circuit will detect fast changes in the "trigger" inputs, and
	// output a pulse. Slow changes won't trigger a pulse. So there is some
	// flexibility in the types of input accepted. For now, trigger inputs
	// are implemented as digital inputs (traditional trigger signals).
	// TODO: Determine if active high or active low.
	PORT_START("trigger_inputs")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRIGGER 5")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRIGGER 4")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRIGGER 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRIGGER 2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRIGGER 1")

	PORT_START("pot_tempo")
	PORT_ADJUSTER(50, "TEMPO") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(linndrum_state::tempo_pot_adjusted), 0)

	PORT_START("pot_volume")
	PORT_ADJUSTER(100, "MASTER VOLUME")

	PORT_START("pot_tuning_1")
	PORT_ADJUSTER(50, "SNARE TUNING");
	PORT_START("pot_tuning_2")
	PORT_ADJUSTER(50, "HI TOM TUNING");
	PORT_START("pot_tuning_3")
	PORT_ADJUSTER(50, "MID TOM TUNING");
	PORT_START("pot_tuning_4")
	PORT_ADJUSTER(50, "LO TOM TUNING");
	PORT_START("pot_tuning_5")
	PORT_ADJUSTER(50, "HI CONGAS TUNING");
	PORT_START("pot_tuning_6")
	PORT_ADJUSTER(50, "LO CONGAS TUNING");
	PORT_START("pot_tuning_7")
	PORT_ADJUSTER(50, "HIHAT DECAY");

	PORT_START("pot_pan_1")
	PORT_ADJUSTER(50, "BASS PAN")
	PORT_START("pot_pan_2")
	PORT_ADJUSTER(50, "SNARE PAN")
	PORT_START("pot_pan_3")
	PORT_ADJUSTER(50, "SIDESTICK PAN")
	PORT_START("pot_pan_4")
	PORT_ADJUSTER(50, "HIHAT PAN")
	PORT_START("pot_pan_5")
	PORT_ADJUSTER(50, "HI TOM PAN")
	PORT_START("pot_pan_6")
	PORT_ADJUSTER(50, "MID TOM PAN")
	PORT_START("pot_pan_7")
	PORT_ADJUSTER(50, "LO TOM PAN")
	PORT_START("pot_pan_8")
	PORT_ADJUSTER(50, "RIDE PAN")
	PORT_START("pot_pan_9")
	PORT_ADJUSTER(50, "CRASH PAN")
	PORT_START("pot_pan_10")
	PORT_ADJUSTER(50, "CABASA PAN")
	PORT_START("pot_pan_11")
	PORT_ADJUSTER(50, "TAMB PAN")
	PORT_START("pot_pan_12")
	PORT_ADJUSTER(50, "HI CONGA PAN")
	PORT_START("pot_pan_13")
	PORT_ADJUSTER(50, "LO CONGA PAN")
	PORT_START("pot_pan_14")
	PORT_ADJUSTER(50, "COWBELL PAN")
	PORT_START("pot_pan_15")
	PORT_ADJUSTER(50, "CLAPS PAN")
	PORT_START("pot_pan_16")
	PORT_ADJUSTER(50, "CLICK PAN")

	PORT_START("pot_gain_1")
	PORT_ADJUSTER(100, "BASS GAIN")
	PORT_START("pot_gain_2")
	PORT_ADJUSTER(100, "SNARE GAIN")
	PORT_START("pot_gain_3")
	PORT_ADJUSTER(100, "SIDESTICK GAIN")
	PORT_START("pot_gain_4")
	PORT_ADJUSTER(100, "HIHAT GAIN")
	PORT_START("pot_gain_5")
	PORT_ADJUSTER(100, "HI TOM GAIN")
	PORT_START("pot_gain_6")
	PORT_ADJUSTER(100, "MID TOM GAIN")
	PORT_START("pot_gain_7")
	PORT_ADJUSTER(100, "LO TOM GAIN")
	PORT_START("pot_gain_8")
	PORT_ADJUSTER(100, "RIDE GAIN")
	PORT_START("pot_gain_9")
	PORT_ADJUSTER(100, "CRASH GAIN")
	PORT_START("pot_gain_10")
	PORT_ADJUSTER(100, "CABASA GAIN")
	PORT_START("pot_gain_11")
	PORT_ADJUSTER(100, "TAMB GAIN")
	PORT_START("pot_gain_12")
	PORT_ADJUSTER(100, "HI CONGA GAIN")
	PORT_START("pot_gain_13")
	PORT_ADJUSTER(100, "LO CONGA GAIN")
	PORT_START("pot_gain_14")
	PORT_ADJUSTER(100, "COWBELL GAIN")
	PORT_START("pot_gain_15")
	PORT_ADJUSTER(100, "CLAPS GAIN")
	PORT_START("pot_gain_16")
	PORT_ADJUSTER(100, "CLICK GAIN")
INPUT_PORTS_END

ROM_START(linndrum)
	ROM_REGION(0x2000, MAINCPU_TAG, 0)
	ROM_LOAD("ld_2.1_a.u14", 0x000000, 0x001000, CRC(566d720e) SHA1(91b7a515e3d18a28b7f5428765ed79114a5a00fb))
	ROM_LOAD("ld_2.1_b.u13", 0x001000, 0x001000, CRC(9c9a5520) SHA1(6e4573051254051c75f9071fd8dfcd5a9184f9cc))

	// All sample ROMs are 2732.
	// ROM file name format: sticker_label.silscreen_label.component_designation

	ROM_REGION(0x1000, "sample_mux_drum_0", 0)  // Tambourine.
	ROM_LOAD("tamb1.tamb.u25", 0x000000, 0x001000, CRC(0309eba3) SHA1(89a1910b5224a1db91c31100cfe81ebd36610027))

	ROM_REGION(0x1000, "sample_mux_drum_1", 0)  // Cabasa.
	ROM_LOAD("cbsa.u26", 0x000000, 0x001000, NO_DUMP)
	ROM_FILL(0x000000, 0x001000, 0x00)  // Silence. Remove if checksum becomes available.

	ROM_REGION(0x1000, "sample_mux_drum_2", 0)  // Claps.
	ROM_LOAD("clps.u27", 0x000000, 0x001000, NO_DUMP)
	ROM_FILL(0x000000, 0x001000, 0x00)  // Silence. Remove if checksum becomes available.

	ROM_REGION(0x1000, "sample_mux_drum_3", 0)  // Cowbell.
	ROM_LOAD("cwbl1.cwbl.u28", 0x000000, 0x001000, CRC(819d4a2c) SHA1(04d6fb88dd8751336617e50ce840fa63e6002942))

	ROM_REGION(0x1000, "sample_mux_drum_4", 0)  // Bass.
	ROM_LOAD("bass.u29", 0x000000, 0x001000, NO_DUMP)
	ROM_FILL(0x000000, 0x001000, 0x00)  // Silence. Remove if checksum becomes available.

	ROM_REGION(0x4000, "sample_mux_drum_5", 0)  // Hi hat.
	ROM_LOAD("hat1a.hat1.u30", 0x000000, 0x001000, CRC(20b35416) SHA1(9aed28369b9b3f4a088c3f2ee9c88ed4b029a3ae))
	ROM_LOAD("hat1b.hat2.u31", 0x001000, 0x001000, CRC(fb4b3bac) SHA1(99a6cada1e741a7294b9aa08df36489644253acb))
	ROM_LOAD("hat1c.hat3.u32", 0x002000, 0x001000, CRC(62d1e667) SHA1(516df46a6e1050151f4b1696568a4ffbde0eba7c))
	ROM_LOAD("hat1d.hat4.u33", 0x003000, 0x001000, CRC(01819ab1) SHA1(b0cfece5568375340eab16f8523ddb8599013310))

	ROM_REGION(0x8000, "sample_mux_drum_6", 0)  // Ride cymbal.
	ROM_LOAD("ride1a.rid1.u9", 0x000000, 0x001000, CRC(3d0a852f) SHA1(58ea6cda2ad1a8b6506ad89ba3f5c47584013ef0))
	ROM_LOAD("ride1b.rid2.u10", 0x001000, 0x001000, CRC(5bb0e082) SHA1(6f4535d08ac013d804cc2d9fd9fedca2b8515c99))
	ROM_LOAD("ride1c.rid3.u8", 0x002000, 0x001000, CRC(fa48f0e3) SHA1(a455b6d8d8dde9a7903919c87263b392f0510c8a))
	ROM_LOAD("ride1d.rid4.u7", 0x003000, 0x001000, CRC(3a8b4133) SHA1(f0a2f7a0db1024e7e263000f74d127f53bf6df94))
	ROM_LOAD("ride1e.rid5.u6", 0x004000, 0x001000, CRC(e30dc9a2) SHA1(933d2fa23a371831e779315f61e9a8f281ffab7c))
	ROM_LOAD("ride1f.rid6.u5", 0x005000, 0x001000, CRC(ba63cc66) SHA1(ba4f2fd13d89c37eab30301b96338c6a9a69ad9d))
	ROM_LOAD("ride1g.rid7.u4", 0x006000, 0x001000, CRC(d5e24b3e) SHA1(fba0fa24e8afd7686b480ace1838587fe2a6691c))
	ROM_LOAD("ride1h.rid8.u3", 0x007000, 0x001000, CRC(4ac922bb) SHA1(f86b3fbd079ef59b5e260250b95ec75829a1c31d))

	ROM_REGION(0x8000, "sample_mux_drum_7", 0)  // Crash cymbal.
	ROM_LOAD("crsh1a.crs1.u14", 0x000000, 0x001000, CRC(85ce8dc5) SHA1(a1fb4064f7d02df21ef898dab1bd4df9754f2420))
	ROM_LOAD("crsh1b.crs2.u13", 0x001000, 0x001000, CRC(3681d6f0) SHA1(1ad7e202eb9a82af03949bcf3ba281932bf83f5b))
	ROM_LOAD("crsh1c.crs3.u12", 0x002000, 0x001000, CRC(ff1a4d87) SHA1(6e10d141f9a8dbbf951d72fbdeda4e8386fd5bc8))
	ROM_LOAD("crsh1d.crs4.u15", 0x003000, 0x001000, CRC(7f623944) SHA1(03c1294df1e057442aacdb004d3ebd8e94628b15))
	ROM_LOAD("crsh1e.crs5.u16", 0x004000, 0x001000, CRC(851c306a) SHA1(4f41a31e2e8273df7664a65c5e5a3528b312627a))
	ROM_LOAD("crsh1f.crs6.u17", 0x005000, 0x001000, CRC(2fee35fe) SHA1(53b68ffe940beee4dcf568af0ed129a02a2948b9))
	ROM_LOAD("crsh1g.crs7.u18", 0x006000, 0x001000, CRC(53f939bb) SHA1(980368e122793d1318e3643b87e06e053690f0de))
	ROM_LOAD("crsh1h.crs8.u11", 0x007000, 0x001000, CRC(7b9f55c2) SHA1(dd0eb89ad89e56a28d8dccb78a979acb954968a1))

	ROM_REGION(0x1000, "sample_sidestick", 0)
	ROM_LOAD("sstk1.sstk.u78", 0x000000, 0x001000, CRC(61af39e3) SHA1(5648674854a8db80656bf729c4f353b75d101d7b))

	ROM_REGION(0x1000, "sample_snare", 0)
	ROM_LOAD("snar9.snar.u79", 0x000000, 0x001000, CRC(83478583) SHA1(bac791208270eff1f2f362511d6418873c47827c))

	ROM_REGION(0x2000, "sample_conga", 0)
	ROM_LOAD("cnga1a.cga1.u66", 0x000000, 0x001000, CRC(1a579539) SHA1(169741786c44026f2b6ef4052cccb2a27ba41e19))
	ROM_LOAD("cnga1b.cga2.u67", 0x001000, 0x001000, CRC(02434d69) SHA1(451398fbf9ac94a1773f4f40ef4ca32d3c857537))

	ROM_REGION(0x2000, "sample_tom", 0)
	ROM_LOAD("tom6a.tom1.u68", 0x000000, 0x001000, CRC(75f83e43) SHA1(386aa53311e6f8cea56e8021b19855a5ba586f52))
	ROM_LOAD("tom6b.tom2.u69", 0x001000, 0x001000, CRC(c7633ca4) SHA1(60ab77bf21897b55cc8d2844ce1cc0c65958c939))
ROM_END

}  // anonymous namespace

SYST(1982, linndrum, 0, 0, linndrum, linndrum, linndrum_state, empty_init, "Linn Electronics", "LinnDrum", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND)
