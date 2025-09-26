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
  CEM3320 VCF with a hardcoded envelope, the output of which is routed to the
  mixer and an individual output jack. The rest of the voices are sent to the
  mixer unfiltered, but their individual outputs have a ~15.9KHz RC LPF. The hat
  is post-processed by a CEM3360 VCA, which can operate in 2 variations: slow
  decay (open hat) and faster, user-controlled decay (closed hat). There's a
  single clock for all voices, which is tuned by a trimmer on the circuit board.

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

Reasons for MACHINE_IMPERFECT_SOUND:
* Missing a few sample checksums.
* Missing bass drum LPF and filter envelope.
* Missing tom / conga LPF and filter envelope.
* Linear, instead of tanh response for hi-hat VCA.

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
#include "sound/flt_biquad.h"
#include "sound/flt_rc.h"
#include "sound/flt_vol.h"
#include "sound/mixer.h"
#include "sound/spkrdev.h"
#include "sound/va_eg.h"
#include "sound/va_vca.h"
#include "speaker.h"

#include "linn_linndrum.lh"

#define LOG_KEYBOARD         (1U << 1)
#define LOG_DEBOUNCE         (1U << 2)
#define LOG_TEMPO            (1U << 3)
#define LOG_TEMPO_CHANGE     (1U << 4)
#define LOG_STROBES          (1U << 5)
#define LOG_TAPE_SYNC_ENABLE (1U << 6)
#define LOG_MIX              (1U << 7)
#define LOG_PITCH            (1U << 8)
#define LOG_HAT_EG           (1U << 9)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

namespace {

enum mux_voices
{
	MV_TAMBOURINE = 0,
	MV_CABASA,
	MV_CLAP,
	MV_COWBELL,
	MV_BASS,
	MV_HAT,
	MV_RIDE,
	MV_CRASH,
	NUM_MUX_VOICES
};

// Names (excluding the TV_ prefix) match those in the schematics.
enum tom_voices
{
	TV_LOW_CONGA = 0,
	TV_HI_CONGA,
	TV_LOW_TOMS,
	TV_MID_TOMS,
	TV_HI_TOMS,
	NUM_TOM_VOICES
};

// Names (excluding the TK_ prefix) match those on the tuning UI.
enum tuning_knobs
{
	TK_SNARE = 0,
	TK_HI_TOMS,
	TK_MID_TOMS,
	TK_LO_TOMS,
	TK_HI_CONGAS,
	TK_LO_CONGAS,
	NUM_TUNING_KNOBS
};

// Names (excluding the MIX_ prefix) match those on the mixer UI.
enum mixer_channels
{
	MIX_BASS = 0,
	MIX_SNARE,
	MIX_SIDESTICK,
	MIX_HIHAT,
	MIX_HITOMS,
	MIX_MIDTOMS,
	MIX_LOTOMS,
	MIX_RIDE,
	MIX_CRASH,
	MIX_CABASA,
	MIX_TAMB,
	MIX_HICONGAS,
	MIX_LOCONGAS,
	MIX_COWBELL,
	MIX_CLAPS,
	MIX_CLICK,
	NUM_MIXER_CHANNELS
};

}  // anonymous namespace


class linndrum_audio_device : public device_t
{
public:
	linndrum_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;

	void mux_drum_w(int voice, u8 data, bool is_strobe = true);
	void snare_w(u8 data);  // Snare and sidestick.
	void tom_w(u8 data);  // Tom and conga.
	void strobe_click_w(u8 data);
	void beep_w(int state);

	DECLARE_INPUT_CHANGED_MEMBER(mix_changed);
	DECLARE_INPUT_CHANGED_MEMBER(master_volume_changed);
	DECLARE_INPUT_CHANGED_MEMBER(mux_drum_tuning_changed);
	DECLARE_INPUT_CHANGED_MEMBER(snare_tuning_changed);
	DECLARE_INPUT_CHANGED_MEMBER(tom_tuning_changed);
	DECLARE_INPUT_CHANGED_MEMBER(hat_decay_changed);

protected:
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	static void write_dac(dac76_device& dac, u8 sample);
	static s32 get_ls267_freq(const std::array<s32, 2>& freq_range_hz, float cv);
	static float get_snare_tom_pitch_cv(float v);

	TIMER_DEVICE_CALLBACK_MEMBER(hat_trigger_timer_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(mux_timer_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(snare_timer_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(click_timer_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(tom_timer_tick);

	void update_volume_and_pan(int channel);
	void update_master_volume();
	void update_mux_drum_pitch();
	void update_snare_pitch();
	void update_tom_pitch();
	void update_hat_decay();

	// Mux drums.
	required_ioport m_mux_tuning_trimmer;
	required_ioport m_hat_decay_pot;
	required_memory_region_array<NUM_MUX_VOICES> m_mux_samples;
	required_device<timer_device> m_mux_timer;  // 74LS627 (U77A).
	required_device_array<dac76_device, NUM_MUX_VOICES> m_mux_dac;  // AM6070 (U88).
	required_device_array<filter_volume_device, NUM_MUX_VOICES> m_mux_volume;  // CD4053 (U90), R60, R62.
	required_device<timer_device> m_hat_trigger_timer;  // U37B (LM556).
	required_device<va_rc_eg_device> m_hat_eg;
	required_device<va_vca_device> m_hat_vca;  // CEM3360 (U91B).
	bool m_hat_open = false;
	bool m_hat_triggered = false;
	std::array<bool, NUM_MUX_VOICES> m_mux_counting = { false, false, false, false, false, false, false, false };
	std::array<u16, NUM_MUX_VOICES> m_mux_counters = { 0, 0, 0, 0, 0, 0, 0, 0 };

	// Snare / sidestick.
	required_memory_region m_snare_samples;  // 2732 ROM (U79).
	required_memory_region m_sidestick_samples;  // 2732 ROMs (U78).
	required_device<timer_device> m_snare_timer;  // 74L627 (U80A).
	required_device<dac76_device> m_snare_dac;  // AM6070 (U92).
	required_device<filter_volume_device> m_snare_out;  // U90A (CD4053) pin 12 (ax).
	required_device<filter_volume_device> m_sidestick_out;  // U90A (CD4053) pin 13 (ay).
	required_device<timer_device> m_click_timer;  // 556 (U65A).
	required_device<speaker_sound_device> m_click;
	required_device<speaker_sound_device> m_beep;
	bool m_snare_counting = false;  // /Q1 of U41 (74LS74).
	u16 m_snare_counter = 0;  // 13-bit counter (2 x 74LS393, U61, U62).
	bool m_sidestick_selected = false;  // Chooses between snare and sidestick.

	// Tom / conga.
	required_ioport_array<NUM_TUNING_KNOBS> m_tuning_knobs;
	required_memory_region m_tom_samples;  // 2 x 2732 ROMs (U68, U69).
	required_memory_region m_conga_samples;  // 2 x 2732 ROMs (U66, U67).
	required_device<timer_device> m_tom_timer;  // 74LS627 (U77B).
	required_device<dac76_device> m_tom_dac;  // AM6070 (U82).
	required_device_array<filter_volume_device, NUM_TOM_VOICES> m_tom_out;  // U87 (CD4051) outputs 0, 1, 4, 5, 6.
	bool m_tom_counting = false;  // /Q1 of U73 (74LS74).
	u16 m_tom_counter = 0;  // 14-bit counter (2 x 74LS393, U70, U71).
	bool m_tom_selected = false;  // Selects between tom and conga.
	s8 m_tom_selected_pitch = 0;

	// Mixer.
	required_ioport_array<NUM_MIXER_CHANNELS> m_volume;
	required_ioport_array<NUM_MIXER_CHANNELS> m_pan;
	required_ioport m_master_volume;
	required_device_array<filter_rc_device, NUM_MIXER_CHANNELS - 1> m_voice_hpf;
	required_device<filter_biquad_device> m_click_bpf;
	required_device<mixer_device> m_left_mixer;  // 4558 op-amp (U1A).
	required_device<mixer_device> m_right_mixer;  // 4558 op-amp (U1B).
	required_device<speaker_device> m_out;        // 4558 op-amp (U2A, U2B).

	static constexpr const float MIXER_R_PRE_FADER[NUM_MIXER_CHANNELS] =
	{
		RES_R(0),    // bass
		RES_R(0),    // snare
		RES_R(0),    // sidestick
		RES_K(5.1),  // hihat
		RES_R(0),    // hi tom
		RES_R(0),    // mid tom
		RES_R(0),    // low tom
		RES_K(10),   // ride
		RES_K(5.1),  // crash
		RES_K(10),   // cabasa
		RES_K(10),   // tambourine
		RES_R(0),    // hi conga
		RES_R(0),    // low conga
		RES_K(5.1),  // cowbell
		RES_K(2.4),  // clap
		RES_R(0),    // click
	};

	static constexpr const float MIXER_R_FEEDBACK = RES_K(33);
	static constexpr const float MIXER_R_BEEP = RES_K(510);  // Same value for left and right.
	static constexpr const float OUTPUT_R_INPUT = RES_K(10);  // Input resistor of output stage opamp.
	static constexpr const float OUTPUT_R_FEEDBACK = RES_K(10);
	static constexpr const float OUTPUT_C_FEEDBACK = CAP_P(1000);
	static constexpr const float R_ON_CD4053 = RES_R(125);  // Typical Ron resistance for 15V supply (-7.5V / 7.5V).

	static constexpr const float VPLUS = 15;  // Volts.
	static constexpr const float VCC = 5;  // Volts.
	static constexpr const float MUX_DAC_IREF = VPLUS / (RES_K(15) + RES_K(15));  // R55 + R57.
	static constexpr const float TOM_DAC_IREF = MUX_DAC_IREF;  // Configured in the same way.
	// All DAC current-to-voltage converter resistors, for both positive and
	// negative values, are 2.49 KOhm.
	static constexpr const float R_DAC_I2V = RES_K(2.49);  // R58, R59, R127, R126, tom DAC I2V (missing designation).

	// Constants for hi hat envelope generator circuit.
	static constexpr const float HAT_C22 = CAP_U(1);
	static constexpr const float HAT_R33 = RES_M(1);
	static constexpr const float HAT_R34 = RES_K(10);
	static constexpr const float HAT_DECAY_POT_R_MAX = RES_K(100);
	static constexpr const float HAT_EG2CV_SCALER = RES_VOLTAGE_DIVIDER(RES_K(8.2), RES_K(10));  // R67, R66.

	// The audio pipeline operates on voltage magnitudes. This scaler normalizes
	// the final output's range to approximately: -1 - 1.
	static constexpr const float VOLTAGE_TO_SOUND_SCALER = 0.2F;

	// These frequency ranges were eyeballed from figure 6 of the 74LS627
	// datasheet: https://www.ti.com/product/SN74LS628 .
	static constexpr const std::array<s32, 2> MUX_TIMER_HZ_RANGE = { 250'000, 2'330'000 };  // C14: 330pF.
	static constexpr const std::array<s32, 2> SNARE_TIMER_HZ_RANGE = { 8'000, 80'000 };  // C111: 0.01uF.
	static constexpr const std::array<s32, 2> TOM_TIMER_HZ_RANGE = { 8'000, 80'000 };  // C?: 0.01uF.

	// CV input impedance. The datasheet provides typical and max currents,
	// given for 1V and 5V. These work out to 100K typical, and
	// 20K minimum impedances. Using the advertised typical value.
	static constexpr const float LS627_CV_INPUT_R = RES_K(100);

	static constexpr const char *MUX_VOICE_NAMES[NUM_MUX_VOICES] =
	{
		"TAMBOURINE", "CABASA", "CLAP", "COWBELL", "BASS", "HAT", "RIDE", "CRASH"
	};
	static constexpr const char *TOM_VOICE_NAMES[NUM_TOM_VOICES] =
	{
		"LOW CONGA", "HI CONGA", "LOW TOMS", "MID TOMS", "HI TOMS"
	};
	static constexpr const char *MIXER_CHANNEL_NAMES[NUM_MIXER_CHANNELS] =
	{
		"BASS", "SNARE", "SIDESTICK", "HIHAT", "HITOMS", "MIDTOMS", "LOTOMS",
		"RIDE", "CRASH", "CABASA", "TAMB", "HICONGAS", "LOCONGAS",
		"COWBELL", "CLAP", "CLICK"
	};
};

DEFINE_DEVICE_TYPE(LINNDRUM_AUDIO, linndrum_audio_device, "linndrum_audio_device", "LinnDrum audio circuits");

linndrum_audio_device::linndrum_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LINNDRUM_AUDIO, tag, owner, clock)
	, m_mux_tuning_trimmer(*this, ":pot_mux_tuning")
	, m_hat_decay_pot(*this, ":pot_hihat_decay")
	, m_mux_samples(*this, ":sample_mux_drum_%u", 0)
	, m_mux_timer(*this, "mux_drum_timer")
	, m_mux_dac(*this, "mux_drums_virtual_dac_%u", 1)
	, m_mux_volume(*this, "mux_drums_volume_control_%u", 1)
	, m_hat_trigger_timer(*this, "hat_trigger_timer")
	, m_hat_eg(*this, "hat_eg")
	, m_hat_vca(*this, "hat_vca")
	, m_snare_samples(*this, ":sample_snare")
	, m_sidestick_samples(*this, ":sample_sidestick")
	, m_snare_timer(*this, "snare_sidestick_timer")
	, m_snare_dac(*this, "snare_sidestick_dac")
	, m_snare_out(*this, "snare_output")
	, m_sidestick_out(*this, "sidestick_output")
	, m_click_timer(*this, "click_timer")
	, m_click(*this, "click")
	, m_beep(*this, "beep")
	, m_tuning_knobs(*this, ":pot_tuning_%u", 1)
	, m_tom_samples(*this, ":sample_tom")
	, m_conga_samples(*this, ":sample_conga")
	, m_tom_timer(*this, "tom_conga_timer")
	, m_tom_dac(*this, "tom_conga_dac")
	, m_tom_out(*this, "tom_conga_out_%u", 0)
	, m_volume(*this, ":pot_gain_%u", 1)
	, m_pan(*this, ":pot_pan_%u", 1)
	, m_master_volume(*this, ":pot_volume")
	, m_voice_hpf(*this, "voice_hpf_%u", 1)
	, m_click_bpf(*this, "click_bpf")
	, m_left_mixer(*this, "lmixer")
	, m_right_mixer(*this, "rmixer")
	, m_out(*this, "speaker")
{
}

void linndrum_audio_device::mux_drum_w(int voice, u8 data, bool is_strobe)
{
	assert(voice >= 0 && voice < NUM_MUX_VOICES);

	m_mux_counting[voice] = BIT(data, 0);
	if (!m_mux_counting[voice])
		m_mux_counters[voice] = 0;

	// Volume variations are controlled by a CD4053 MUX (U90B), whose "select"
	// input is connected to the active voice's D1 (via a 74LS151 encoder, U35).
	// Depending on how the mux is configured, the audio signal will either
	// remain unchanged, or it will get attenuated by a voltage divider.
	// The MUX is always configured in the "no attenuation" mode for the clap
	// and cowbell voices.
	static constexpr const float ATTENUATION = RES_VOLTAGE_DIVIDER(RES_K(10), RES_K(3.3) + R_ON_CD4053);  // R60, R62.
	const bool attenuate = !BIT(data, 1) && voice != MV_CLAP && voice != MV_COWBELL;
	m_mux_volume[voice]->set_gain(attenuate ? ATTENUATION : 1);

	if (voice == MV_HAT)
	{
		m_hat_open = BIT(data, 2);
		m_hat_triggered = is_strobe;
		update_hat_decay();
		if (is_strobe)
			m_hat_trigger_timer->adjust(PERIOD_OF_555_MONOSTABLE(RES_K(510), CAP_U(0.01)));  // R8, C4.
		LOGMASKED(LOG_HAT_EG, "Hat EG write: %x, %d.\n", data, is_strobe);
	}

	LOGMASKED(LOG_STROBES, "Strobed mux drum %s: %02x (gain: %f)\n",
			  MUX_VOICE_NAMES[voice], data, m_mux_volume[voice]->gain());
}

void linndrum_audio_device::snare_w(u8 data)
{
	m_snare_counting = BIT(data, 0);
	if (!m_snare_counting)
	{
		m_snare_counter = 0;
		write_dac(*m_snare_dac, 0);  // DAC is disabled. Output goes to 0.
	}

	m_sidestick_selected = BIT(data, 1);  // Play sidestick instead of snare.
	if (m_sidestick_selected)
	{
		m_snare_out->set_gain(0);
		m_sidestick_out->set_gain(1);
	}
	else
	{
		m_snare_out->set_gain(1);
		m_sidestick_out->set_gain(0);
	}

	// Snare and sidestick volume is set by controlling the reference current to
	// the DAC. Bits D2 and D3 from the voice data bus (latched by U42, 74LS74)
	// control the voltage at the end of R72 and R71.

	// While there is a capacitor (C29, 0.01 UF) attached to the DAC's Iref
	// input, it is too small to have a musical effect. Changes in current will
	// stablizie within 0.1 ms, and such changes only happen at voice trigger.

	static constexpr const float R0 = RES_K(3.3);  // R70.
	static constexpr const float R1 = RES_K(380);  // R69.
	static constexpr const float R2 = RES_K(22);   // R72.
	static constexpr const float R3 = RES_K(5.6);  // R71.

	static constexpr const float a = R0 * R2 * R3;
	static constexpr const float b = R0 * R1 * R3;
	static constexpr const float c = R0 * R1 * R2;
	static constexpr const float d = R1 * R2 * R3;

	// Compute DAC reference current.
	const float v2 = BIT(data, 2) ? VCC : 0;
	const float v3 = BIT(data, 3) ? VCC : 0;
	const float iref = (a * VPLUS + b * v2 + c * v3) / (R0 * (a + b + c + d));
	m_snare_dac->set_fixed_iref(iref);

	LOGMASKED(LOG_STROBES, "Strobed snare / sidestick: %02x (iref: %f)\n", data, iref);
}

void linndrum_audio_device::tom_w(u8 data)
{
	m_tom_counting = BIT(data, 0);
	if (!m_tom_counting)
	{
		m_tom_counter = 0;
		write_dac(*m_tom_dac, 0);  // DAC is disabled. Output goes to 0.
	}

	m_tom_selected = BIT(data, 1);  // Play tom instead of conga.
	// It is possible for neither the tom nor the conga ROM to be selected. This
	// can only happen when the voice is disabled, so it does not affect the
	// emulation.

	// Address for the pitch control MUX (U81) and output selection MUX (U87).
	// Both are CD4051s.
	const u8 variation = bitswap<3>(data, 1, 3, 2);  // MUX: C, B, A.

	m_tom_selected_pitch = -1;
	switch (variation)
	{
		case 0: m_tom_selected_pitch = TV_LOW_CONGA; break;
		case 1: m_tom_selected_pitch = TV_HI_CONGA; break;
		case 4: m_tom_selected_pitch = TV_LOW_TOMS; break;
		case 5: m_tom_selected_pitch = TV_MID_TOMS; break;
		case 6: m_tom_selected_pitch = TV_HI_TOMS; break;
		default: LOG("Firmware bug: invalid pitch variation for tom/conga.\n");
	}
	update_tom_pitch();

	// If the tom/conga voice is disabled, The INH input of the output MUX (U87)
	// will be high, and all outputs will be disconnected. That's in contrast to
	// the pitch MUX (U81), which is always enabled.
	// Disconnected outputs have their voltage pulled to 0 by a 10K resistor.
	const s8 selected_output = m_tom_counting ? m_tom_selected_pitch : -1;
	for (int i = 0; i < NUM_TOM_VOICES; ++i)
		m_tom_out[i]->set_gain((i == selected_output) ? 1 : 0);

	LOGMASKED(LOG_STROBES, "Strobed tom / conga: %02x (is_tom: %d, pitch:%d, output: %d, %s)\n",
			  data, m_tom_selected, m_tom_selected_pitch, selected_output,
			  (selected_output >= 0) ? TOM_VOICE_NAMES[selected_output] : "none");
}

void linndrum_audio_device::strobe_click_w(u8 /*data*/)
{
	m_click_timer->adjust(PERIOD_OF_555_MONOSTABLE(RES_K(100), CAP_U(0.01)));  // R10, C12.
	m_click->level_w(1);
	LOGMASKED(LOG_STROBES, "Strobed click.\n");
}

void linndrum_audio_device::beep_w(int state)
{
	// Beep signal is inverted by U76B (74LS00).
	m_beep->level_w(state ? 0 : 1);
	LOGMASKED(LOG_STROBES, "Beep: %d\n", state);
}

DECLARE_INPUT_CHANGED_MEMBER(linndrum_audio_device::mix_changed)
{
	update_volume_and_pan(param);
}

DECLARE_INPUT_CHANGED_MEMBER(linndrum_audio_device::master_volume_changed)
{
	update_master_volume();
}

DECLARE_INPUT_CHANGED_MEMBER(linndrum_audio_device::mux_drum_tuning_changed)
{
	update_mux_drum_pitch();
}

DECLARE_INPUT_CHANGED_MEMBER(linndrum_audio_device::snare_tuning_changed)
{
	update_snare_pitch();
}

DECLARE_INPUT_CHANGED_MEMBER(linndrum_audio_device::tom_tuning_changed)
{
	update_tom_pitch();
}

DECLARE_INPUT_CHANGED_MEMBER(linndrum_audio_device::hat_decay_changed)
{
	update_hat_decay();
}

void linndrum_audio_device::device_add_mconfig(machine_config &config)
{
	// *** Mux drums section.

	TIMER(config, m_mux_timer).configure_generic(FUNC(linndrum_audio_device::mux_timer_tick));  // 74LS627 (U77A).

	// The actual "mux drums" hardware has a single AM6070, which is
	// time-multiplexed across the 8 voices. Implementing it that way is
	// possible, but requires a sample rate of at least 240KHz (8 x ~30K) for
	// reasonable results. It also requires emulating audio sample & hold
	// functionality. So 8 "virtual" DACs and volume-control MUXes are used
	// instead.
	for (int voice = 0; voice < NUM_MUX_VOICES; ++voice)
	{
		DAC76(config, m_mux_dac[voice], 0);  // AM6070 (U88).
		m_mux_dac[voice]->configure_voltage_output(R_DAC_I2V, R_DAC_I2V);  // R58, R59.
		m_mux_dac[voice]->set_fixed_iref(MUX_DAC_IREF);
		FILTER_VOLUME(config, m_mux_volume[voice]);  // CD4053 (U90), R60, R62 (see mux_drum_w()).
		m_mux_dac[voice]->add_route(0, m_mux_volume[voice], 1.0);
	}

	TIMER(config, m_hat_trigger_timer).configure_generic(FUNC(linndrum_audio_device::hat_trigger_timer_tick));  // LM556 (U37B).
	VA_RC_EG(config, m_hat_eg).set_c(HAT_C22);
	VA_VCA(config, m_hat_vca).configure_cem3360_linear_cv();
	m_mux_volume[MV_HAT]->add_route(0, m_hat_vca, 1.0, 0);
	m_hat_eg->add_route(0, m_hat_vca, HAT_EG2CV_SCALER, 1);

	// *** Snare / sidestick section.

	TIMER(config, m_snare_timer).configure_generic(FUNC(linndrum_audio_device::snare_timer_tick));  // 74LS627 (U80A).
	DAC76(config, m_snare_dac, 0);  // AM6070 (U92)
	m_snare_dac->configure_voltage_output(R_DAC_I2V, R_DAC_I2V);  // R127, R126.

	// The DAC's current outputs are processed by a current-to-voltage converter
	// that embeds an RC filter. This consists of an op-amp (U103), R127 and C65
	// (for positive voltages), and R126 and C31 (for negative voltages). The
	// two resistors and capacitors have the same value.
	auto &snare_dac_filter = FILTER_RC(config, "snare_sidestick_dac_filter");
	snare_dac_filter.set_lowpass(R_DAC_I2V, CAP_P(2700));  // R127-C65, R126-C31. Cutoff: ~23.7KHz.
	m_snare_dac->add_route(0, snare_dac_filter, 1.0);

	FILTER_VOLUME(config, m_snare_out);
	FILTER_VOLUME(config, m_sidestick_out);
	snare_dac_filter.add_route(0, m_snare_out, 1.0);
	snare_dac_filter.add_route(0, m_sidestick_out, 1.0);

	TIMER(config, m_click_timer).configure_generic(FUNC(linndrum_audio_device::click_timer_tick));  // 556 (U65A).
	static const double LEVELS[2] = { 0, VCC };
	SPEAKER_SOUND(config, m_click).set_levels(2, LEVELS);
	SPEAKER_SOUND(config, m_beep).set_levels(2, LEVELS);

	// *** Tom / conga section.

	TIMER(config, m_tom_timer).configure_generic(FUNC(linndrum_audio_device::tom_timer_tick));  // 74LS627 (U77B).
	DAC76(config, m_tom_dac, 0);  // AM6070 (U82).
	// Schematic is missing the second resistor, but that's almost certainly an error.
	// It is also missing component designations.
	m_tom_dac->configure_voltage_output(R_DAC_I2V, R_DAC_I2V);
	m_tom_dac->set_fixed_iref(TOM_DAC_IREF);
	for (int i = 0; i < NUM_TOM_VOICES; ++i)
	{
		FILTER_VOLUME(config, m_tom_out[i]);  // One of U87'S (CD4051) outputs.
		m_tom_dac->add_route(0, m_tom_out[i], 1.0);
	}

	// *** Mixer.
	const std::array<device_sound_interface *, NUM_MIXER_CHANNELS> voice_outputs =
	{
		m_mux_volume[MV_BASS],
		m_snare_out,
		m_sidestick_out,
		m_hat_vca,
		m_tom_out[TV_HI_TOMS],
		m_tom_out[TV_MID_TOMS],
		m_tom_out[TV_LOW_TOMS],
		m_mux_volume[MV_RIDE],
		m_mux_volume[MV_CRASH],
		m_mux_volume[MV_CABASA],
		m_mux_volume[MV_TAMBOURINE],
		m_tom_out[TV_HI_CONGA],
		m_tom_out[TV_LOW_CONGA],
		m_mux_volume[MV_COWBELL],
		m_mux_volume[MV_CLAP],
		m_click,
	};

	MIXER(config, m_left_mixer);  // U1A
	MIXER(config, m_right_mixer);  // U1B

	assert(voice_outputs.size() - 1 == MIX_CLICK);
	for (int i = 0; i < voice_outputs.size() - 1; ++i)  // Skip "click".
	{
		// The filter and gain will be configured in update_volume_and_pan().
		FILTER_RC(config, m_voice_hpf[i]);
		voice_outputs[i]->add_route(0, m_voice_hpf[i], 1.0);
		m_voice_hpf[i]->add_route(0, m_left_mixer, 1.0);
		m_voice_hpf[i]->add_route(0, m_right_mixer, 1.0);
	}

	FILTER_BIQUAD(config, m_click_bpf);  // Configured in update_volume_and_pan().
	voice_outputs[MIX_CLICK]->add_route(0, m_click_bpf, 1.0);
	m_click_bpf->add_route(0, m_left_mixer, 1.0);
	m_click_bpf->add_route(0, m_right_mixer, 1.0);

	auto &beep_hpf = FILTER_RC(config, "beep_hpf");
	const float rc_r = RES_2_PARALLEL(MIXER_R_BEEP, MIXER_R_BEEP);
	beep_hpf.set_rc(filter_rc_device::HIGHPASS, rc_r, 0, 0, CAP_U(0.1));  // C17. Cutoff: ~6.24 Hz.
	m_beep->add_route(0, beep_hpf, 1.0);
	// The mixers are inverting.
	beep_hpf.add_route(0, m_left_mixer, -MIXER_R_FEEDBACK / MIXER_R_BEEP);
	beep_hpf.add_route(0, m_right_mixer, -MIXER_R_FEEDBACK / MIXER_R_BEEP);

	// The left/right output op-amps (U2A, U2B) also have a capacitor in their
	// feedback loop, which turns them into LPFs.
	auto &left_rc = FILTER_RC(config, "left_output_lpf");
	auto &right_rc = FILTER_RC(config, "right_output_lpf");
	left_rc.set_lowpass(OUTPUT_R_FEEDBACK, OUTPUT_C_FEEDBACK);  // Cutoff: ~15.9KHz.
	right_rc.set_lowpass(OUTPUT_R_FEEDBACK, OUTPUT_C_FEEDBACK);
	m_left_mixer->add_route(0, left_rc, 1.0);
	m_right_mixer->add_route(0, right_rc, 1.0);

	SPEAKER(config, m_out, 2).front();
	// Gain will be set in update_master_volume().
	left_rc.add_route(0, m_out, 1.0, 0);
	right_rc.add_route(0, m_out, 1.0, 1);
}

void linndrum_audio_device::device_start()
{
	save_item(NAME(m_hat_open));
	save_item(NAME(m_hat_triggered));
	save_item(NAME(m_mux_counting));
	save_item(NAME(m_mux_counters));
	save_item(NAME(m_snare_counting));
	save_item(NAME(m_snare_counter));
	save_item(NAME(m_sidestick_selected));
	save_item(NAME(m_tom_counting));
	save_item(NAME(m_tom_counter));
	save_item(NAME(m_tom_selected));
	save_item(NAME(m_tom_selected_pitch));
}

void linndrum_audio_device::device_reset()
{
	for (int i = 0; i < NUM_MIXER_CHANNELS; ++i)
		update_volume_and_pan(i);
	update_master_volume();
	update_mux_drum_pitch();
	update_snare_pitch();
	update_tom_pitch();
	update_hat_decay();
}

void linndrum_audio_device::write_dac(dac76_device &dac, u8 sample)
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

s32 linndrum_audio_device::get_ls267_freq(const std::array<s32, 2>& freq_range, float cv)
{
	// The relationship between CV and frequency is approximately linear. The
	// frequency range is set by an external capacitor.
	static constexpr const float MIN_CV = 0;
	static constexpr const float MAX_CV = VCC;
	const float alpha = (freq_range[1] - freq_range[0]) / (MAX_CV - MIN_CV);
	return s32(roundf(freq_range[0] + alpha * cv));
}

float linndrum_audio_device::get_snare_tom_pitch_cv(float v_tune)
{
	// The tom/conga and snare tuning knobs are combined with resistors to
	// produce a tuning voltage (v_tune). The tom/conga and snare networks are
	// different. But v_tune for both is processed by identical circuits.
	// Component designations below are for the snare circuit, but values are
	// the same for the tom one.

	static constexpr const float R96 = RES_K(30);
	static constexpr const float R97 = RES_K(47);
	static constexpr const float R100 = RES_K(10);
	static constexpr const float EXTERNAL_CV = 0;  // External CV not yet emulated.

	// U95B (4558 opamp) mixes v_tune with external CV. v_tune is applied to
	// the + input, while the (inverted and scaled) external CV is applied to
	// the - input, via R97.
	const float opamp_out = v_tune + (v_tune - EXTERNAL_CV) * R96 / R97;

	// The output is sent to the timer's CV input via a 10K resistor. That CV is
	// loaded by the timer input's impedance.
	const float cv = opamp_out * RES_VOLTAGE_DIVIDER(R100, LS627_CV_INPUT_R);

	// There are clamping diodes attached to the CV input.
	return std::clamp<float>(cv, 0, VCC);
}

TIMER_DEVICE_CALLBACK_MEMBER(linndrum_audio_device::hat_trigger_timer_tick)
{
	m_hat_triggered = false;
	update_hat_decay();
	LOGMASKED(LOG_HAT_EG, "Hat EG started decay.\n");
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
				// All outputs in the voice's data latch (74LS74) are cleared.
				mux_drum_w(voice, 0, false);
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
		// All outputs of U41 and U42 (74LS74 flip-flops) are cleared.
		snare_w(0);
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
		// All outputs of U42B and U73B (74LS74 flip-flops) are cleared.
		tom_w(0);
		return;
	}

	u8 sample = 0;
	if (m_tom_selected)
		sample = m_tom_samples->as_u8(m_tom_counter);
	else
		sample = m_conga_samples->as_u8(m_tom_counter);
	write_dac(*m_tom_dac, sample);
}

void linndrum_audio_device::update_volume_and_pan(int channel)
{
	assert(channel >= 0 && channel < NUM_MIXER_CHANNELS);

	static constexpr const float R_VOL_MAX = RES_K(5);
	static constexpr const float R_PAN_MAX = RES_K(10);
	static constexpr const float R1 = RES_K(5.6);
	static constexpr const float R2 = RES_K(5.6);
	static constexpr const float R3 = RES_K(15);
	static constexpr const float R4 = RES_K(15);

	// Since we are interested in voltage gain, rather than actual voltage,
	// use 1V as the voice's output.
	static constexpr const float V_VOICE = 1;
	// DC-blocking capacitor. Same value for all voice outputs except for the
	// "click" and "beep" sounds.
	static constexpr const float C_VOICE = CAP_U(10);

	const s32 volume = m_volume[channel]->read();
	const float r_vol_bottom = R_VOL_MAX * RES_AUDIO_POT_LAW(volume / 100.0F);
	const float r_vol_top = R_VOL_MAX - r_vol_bottom;
	const float r_pan_left = m_pan[channel]->read() * R_PAN_MAX / 100.0F;
	const float r_pan_right = R_PAN_MAX - r_pan_left;

	const float r0 = MIXER_R_PRE_FADER[channel] + r_vol_top;
	const float r_right_gnd = R1 + RES_2_PARALLEL(r_pan_right, R3);
	const float r_left_gnd = R2 + RES_2_PARALLEL(r_pan_left, R4);

	// Resistance to ground as seen from the volume pot's wiper and the voice's output.
	const float r_wiper_gnd = RES_3_PARALLEL(r_vol_bottom, r_left_gnd, r_right_gnd);
	const float r_voice_gnd = r0 + r_wiper_gnd;

	float gain_left = 0;
	float gain_right = 0;
	if (volume > 0)
	{
		// Calculate voltage scale factor at the wiper of the volume pot.
		const float i0 = V_VOICE / r_voice_gnd;
		const float v_pot = V_VOICE - i0 * r0;

		// Calculate voltage at the input resistor (R3) of the U1A summing
		// op-amp, and use it to compute amp gain.
		const float i1 = v_pot / r_right_gnd;
		const float v_input_right = v_pot - i1 * R1;
		gain_right = v_input_right * MIXER_R_FEEDBACK / R3;

		// Calculate voltage at the input resistor (R4) of the U1B summing
		// op-amp, and use it to compute amp gain.
		const float i2 = v_pot / r_left_gnd;
		const float v_input_left = v_pot - i2 * R2;
		gain_left = v_input_left * MIXER_R_FEEDBACK / R4;
	}

	device_sound_interface *mixer_input = nullptr;
	if (channel == MIX_CLICK)
		mixer_input = m_click_bpf;
	else
		mixer_input = m_voice_hpf[channel];

	// Using -gain_*, because the summing op-amps are inverting.
	mixer_input->set_route_gain(0, m_left_mixer, 0, -gain_left);
	mixer_input->set_route_gain(0, m_right_mixer, 0, -gain_right);
	LOGMASKED(LOG_MIX, "Gain update for %s - left: %f, right: %f\n",
			  MIXER_CHANNEL_NAMES[channel], gain_left, gain_right);

	if (channel == MIX_CLICK)
	{
		// Compared to the other voices, the click uses a different value for
		// the DC blocking capacitor. Furthermore, there is a capacitor to ground
		// at the volume pot wiper. The resulting filter acts as an HPF (cutoff:
		// ~1.15 KHz) when the click volume is at max, and becomes a BPF whose
		// characteristics change as the volume decreases.

		// Capacitor at the output of the 556 timer.
		static constexpr const float C_CLICK_DCBLOCK = CAP_U(0.01);  // C37
		// Capacitor to ground, at the wiper of the "click" volume fader.
		static constexpr const float C_CLICK_WIPER = CAP_U(0.047);  // No designation.

		// All "click" filter configurations result in singificant attenutation
		// (< 0.2x), which makes the click very quiet. The filter characteristics
		// (gain and Fc) have been verified with simulations. So it is possible
		// there is an error in the schematic. Different capacitor values, or a
		// supply of 15V (instead of the stated 5V) for the 556 timer generating
		// the click could explain this.
		// For now, scale by some number to match the volume of other voices.
		static constexpr const float CLICK_GAIN_CORRECTION = 5;

		if (volume >= 100)
		{
			// HPF transfer function: H(s) = (g * s) / (s + w) where
			//   g = C1 / (C1 + C2)
			//   w = 1 / (R * (C1 + C2))
			const float fc = 1.0F / (2 * float(M_PI) * r_voice_gnd * (C_CLICK_DCBLOCK + C_CLICK_WIPER));
			const float gain = C_CLICK_DCBLOCK / (C_CLICK_DCBLOCK + C_CLICK_WIPER);
			m_click_bpf->modify(filter_biquad_device::biquad_type::HIGHPASS1P1Z, fc, 1, gain * CLICK_GAIN_CORRECTION);
			LOGMASKED(LOG_MIX, "- HPF cutoff: %.2f Hz, Gain: %.3f\n", fc, gain);
		}
		else if (volume > 0)
		{
			filter_biquad_device::biquad_params p = m_click_bpf->rc_rr_bandpass_calc(r0, r_wiper_gnd, C_CLICK_DCBLOCK, C_CLICK_WIPER);
			// The filter params above include the effect of the volume
			// potentiometer. But `gain_left` and `gain_right` already incorporate
			// that effect. So it needs to be undone from the filter's gain.
			p.gain /= RES_VOLTAGE_DIVIDER(r0, r_wiper_gnd);
			// See comments for CLICK_GAIN_CORRECTION.
			p.gain *= CLICK_GAIN_CORRECTION;
			m_click_bpf->modify(p);
			LOGMASKED(LOG_MIX, "- BPF cutoff: %.2f Hz, Q: %.3f, Gain: %.3f\n", p.fc, p.q, p.gain);
		}
		// Else, if the volume is 0, don't change the BPF's configuration to avoid divisions by 0.
	}
	else
	{
		// The rest of the voices just have a DC-blocking filter. Its exact cutoff
		// will depend on the volume and pan settings, but it won't be audible.
		m_voice_hpf[channel]->filter_rc_set_RC(filter_rc_device::HIGHPASS, r_voice_gnd, 0, 0, C_VOICE);
		LOGMASKED(LOG_MIX, "- HPF cutoff: %.2f Hz\n", 1.0F / (2 * float(M_PI) * r_voice_gnd * C_VOICE));
	}
}

void linndrum_audio_device::update_master_volume()
{
	static constexpr const float R_MASTER_VOLUME_MAX = RES_K(10);

	const float r_pot_bottom = R_MASTER_VOLUME_MAX * RES_AUDIO_POT_LAW(m_master_volume->read() / 100.0F);
	const float r_pot_top = R_MASTER_VOLUME_MAX - r_pot_bottom;
	const float v_input = RES_VOLTAGE_DIVIDER(r_pot_top, RES_2_PARALLEL(r_pot_bottom, OUTPUT_R_INPUT));

	const float gain = v_input * OUTPUT_R_FEEDBACK / OUTPUT_R_INPUT;
	const float final_gain = gain * VOLTAGE_TO_SOUND_SCALER;

	// Using -final_gain, because the output opamps (U2A, U2B) are inverting.
	m_out->set_input_gain(0, -final_gain);
	m_out->set_input_gain(1, -final_gain);

	LOGMASKED(LOG_MIX, "Master volume updated. Gain: %f, final gain: %f\n", gain, final_gain);
}

void linndrum_audio_device::update_mux_drum_pitch()
{
	static constexpr const int TIMER_TICKS_PER_VOICE = 4;
	static constexpr const float POT_MAX = RES_K(10);
	static constexpr const float R19A = RES_K(18);

	const float pot_bottom = m_mux_tuning_trimmer->read() * POT_MAX / 100.0F;
	const float pot_top = POT_MAX - pot_bottom;
	const float cv = VPLUS * RES_VOLTAGE_DIVIDER(R19A + pot_top, RES_2_PARALLEL(pot_bottom, LS627_CV_INPUT_R));
	const s32 freq = get_ls267_freq(MUX_TIMER_HZ_RANGE, cv);

	// See comments in mux_timer_tick() for the reason for this adjustment.
	const s32 adjusted_freq = float(freq) / (NUM_MUX_VOICES * TIMER_TICKS_PER_VOICE);
	const attotime period = attotime::from_hz(s32(roundf(adjusted_freq)));
	m_mux_timer->adjust(period, 0, period);

	LOGMASKED(LOG_PITCH, "Updated mux drum pitch. CV: %f, freq: %d, adjusted: %d\n",
			  cv, freq, adjusted_freq);
}

void linndrum_audio_device::update_snare_pitch()
{
	static constexpr const float POT_MAX = RES_K(100);
	static constexpr const float R101 = RES_K(75);
	static constexpr const float R98 = RES_K(22);

	const float pot_bottom = m_tuning_knobs[TK_SNARE]->read() * POT_MAX / 100.0F;
	const float pot_top = POT_MAX - pot_bottom;
	const float v_pot = VPLUS * RES_VOLTAGE_DIVIDER(pot_top, RES_2_PARALLEL(pot_bottom, R101 + R98));
	const float cv = get_snare_tom_pitch_cv(v_pot * RES_VOLTAGE_DIVIDER(R101, R98));

	const s32 freq = get_ls267_freq(SNARE_TIMER_HZ_RANGE, cv);
	const attotime period = attotime::from_hz(freq);
	m_snare_timer->adjust(period, 0, period);

	LOGMASKED(LOG_PITCH, "Updated snare pitch. CV: %f, freq: %d Hz\n", cv, freq);
}

void linndrum_audio_device::update_tom_pitch()
{
	static constexpr const float R_POT_MAX = RES_K(100);

	// A map from `enum tom_voices` to `enum tuning_knobs`.
	static constexpr const int VOICE_TO_KNOB_MAP[NUM_TOM_VOICES] =
	{
		TK_LO_CONGAS,
		TK_HI_CONGAS,
		TK_LO_TOMS,
		TK_MID_TOMS,
		TK_HI_TOMS
	};

	if (m_tom_selected_pitch < 0)
	{
		LOG("Firmware or driver bug: floating input to pitch CV op-amp.\n");
		return;
	}

	// The pitch CV for each variation appears on an input of MUX U81 (CD4051).
	// The CV of the currently selected variation is routed to the timer (74LS627, U77B).
	// Compute the CV of the selected variation.
	const int knob_index = VOICE_TO_KNOB_MAP[m_tom_selected_pitch];
	const s32 knob_value = m_tuning_knobs[knob_index]->read();
	const float r_pot_bottom = knob_value * R_POT_MAX / 100.0F;
	const float r_pot_top = R_POT_MAX - r_pot_bottom;
	const float v_mux_out = VPLUS * RES_VOLTAGE_DIVIDER(RES_K(390) + r_pot_top, r_pot_bottom);
	const float cv = get_snare_tom_pitch_cv(v_mux_out);

	const s32 freq = get_ls267_freq(SNARE_TIMER_HZ_RANGE, cv);
	const attotime period = attotime::from_hz(freq);

	// Only restart the timer if there is a change. Many calls to this function
	// will not result in a frequency change.
	if (m_tom_timer->period() != period)
		m_tom_timer->adjust(period, 0, period);

	LOGMASKED(LOG_PITCH, "Updated tom pitch: %d, %d. CV: %f, freq: %d\n",
			  knob_index, knob_value, cv, freq);
}

void linndrum_audio_device::update_hat_decay()
{
	// When the hat is configed as "open", the EG will discharge through a 1M
	// resistor. When the hat is "closed", a CD4053 will add a parallel
	// discharge path through the "hihat decay" pot.
	float eg_r = HAT_R33;
	if (!m_hat_open)
	{
		const float r_decay_pot = HAT_DECAY_POT_R_MAX * m_hat_decay_pot->read() / 100.0F;
		eg_r = RES_2_PARALLEL(HAT_R33, HAT_R34 + R_ON_CD4053 + r_decay_pot);
	}
	m_hat_eg->set_r(eg_r);

	// When the hat is strobed, it will trigger a 556 (U37A) timer in monostable
	// mode (see mux_drum_w()). The timer's output pin is connected to the EG
	// capacitor via a diode, and will maintain that capacitor at VCC until the
	// timer expires. At that point, the EG capacitor will discharge through eg_r.
	if (m_hat_triggered)
		m_hat_eg->set_instant_v(VCC);
	else
		m_hat_eg->set_target_v(0);

	LOGMASKED(LOG_HAT_EG, "Hat decay. Open: %d, EG R: %f\n", m_hat_open, eg_r);
}

namespace {

constexpr const char MAINCPU_TAG[] = "z80";
constexpr const char NVRAM_TAG[] = "nvram";
constexpr const char AUDIO_TAG[] = "linndrum_audio";

class linndrum_state : public driver_device
{
public:
	static constexpr feature_type unemulated_features() { return feature::TAPE; }

	linndrum_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MAINCPU_TAG)
		, m_audio(*this, AUDIO_TAG)
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

	void voice_data_enable_w(int state);
	u8 get_voice_data(u8 data) const;

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
	bool m_voice_data_enabled = false;  // Enables/disables U19 (74LS365).

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

void linndrum_state::voice_data_enable_w(int state)
{
	// Controls whether data (D0-D3) is transmitted to the voice circuits. This
	// is done by U19 (74LS365 hex buffer. Enable inputs are active-low).
	// This is usually disabled to prevent interference from the "noisy" data
	// bus to the voice circuits.
	m_voice_data_enabled = !state;
}

u8 linndrum_state::get_voice_data(u8 data) const
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
	map(0x1f85, 0x1f85).mirror(0x0030).lw8(NAME([this] (u8 data) { m_audio->mux_drum_w(MV_BASS, get_voice_data(data)); }));
	map(0x1f86, 0x1f86).mirror(0x0030).lw8(NAME([this] (u8 data) { m_audio->snare_w(get_voice_data(data)); }));
	map(0x1f87, 0x1f87).mirror(0x0030).lw8(NAME([this] (u8 data) { m_audio->mux_drum_w(MV_HAT, get_voice_data(data)); }));
	map(0x1f88, 0x1f88).mirror(0x0030).lw8(NAME([this] (u8 data) { m_audio->tom_w(get_voice_data(data)); }));
	map(0x1f89, 0x1f89).mirror(0x0030).lw8(NAME([this] (u8 data) { m_audio->mux_drum_w(MV_RIDE, get_voice_data(data)); }));
	map(0x1f8a, 0x1f8a).mirror(0x0030).lw8(NAME([this] (u8 data) { m_audio->mux_drum_w(MV_CRASH, get_voice_data(data)); }));
	map(0x1f8b, 0x1f8b).mirror(0x0030).lw8(NAME([this] (u8 data) { m_audio->mux_drum_w(MV_CABASA, get_voice_data(data)); }));
	map(0x1f8c, 0x1f8c).mirror(0x0030).lw8(NAME([this] (u8 data) { m_audio->mux_drum_w(MV_TAMBOURINE, get_voice_data(data)); }));
	map(0x1f8d, 0x1f8d).mirror(0x0030).lw8(NAME([this] (u8 data) { m_audio->mux_drum_w(MV_COWBELL, get_voice_data(data)); }));
	map(0x1f8e, 0x1f8e).mirror(0x0030).lw8(NAME([this] (u8 data) { m_audio->mux_drum_w(MV_CLAP, get_voice_data(data)); }));
	map(0x1f8f, 0x1f8f).mirror(0x0030).w(m_audio, FUNC(linndrum_audio_device::strobe_click_w));  // No voice data sent.

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
	save_item(NAME(m_voice_data_enabled));
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
	u16.bit_handler<3>().set(FUNC(linndrum_state::voice_data_enable_w));
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
	PORT_ADJUSTER(80, "TEMPO") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(linndrum_state::tempo_pot_adjusted), 0)

	PORT_START("pot_volume")
	PORT_ADJUSTER(90, "MASTER VOLUME") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::master_volume_changed), 0)

	PORT_START("pot_mux_tuning")  // Internal trimmer. Not accessible by the end-user.
	PORT_ADJUSTER(25, "TRIMMER: MUX DRUM TUNING") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mux_drum_tuning_changed), 0)

	PORT_START("pot_tuning_1")
	PORT_ADJUSTER(25, "SNARE TUNING") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::snare_tuning_changed), 0)
	PORT_START("pot_tuning_2")
	PORT_ADJUSTER(60, "HI TOM TUNING") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::tom_tuning_changed), 0)
	PORT_START("pot_tuning_3")
	PORT_ADJUSTER(50, "MID TOM TUNING") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::tom_tuning_changed), 0)
	PORT_START("pot_tuning_4")
	PORT_ADJUSTER(40, "LO TOM TUNING") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::tom_tuning_changed), 0)
	PORT_START("pot_tuning_5")
	PORT_ADJUSTER(52, "HI CONGAS TUNING") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::tom_tuning_changed), 0)
	PORT_START("pot_tuning_6")
	PORT_ADJUSTER(40, "LO CONGAS TUNING") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::tom_tuning_changed), 0)
	PORT_START("pot_hihat_decay")
	PORT_ADJUSTER(50, "HIHAT DECAY") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::hat_decay_changed), 0)

	PORT_START("pot_pan_1")
	PORT_ADJUSTER(50, "BASS PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_BASS)
	PORT_START("pot_pan_2")
	PORT_ADJUSTER(50, "SNARE PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_SNARE)
	PORT_START("pot_pan_3")
	PORT_ADJUSTER(50, "SIDESTICK PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_SIDESTICK)
	PORT_START("pot_pan_4")
	PORT_ADJUSTER(50, "HIHAT PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_HIHAT)
	PORT_START("pot_pan_5")
	PORT_ADJUSTER(50, "HI TOM PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_HITOMS)
	PORT_START("pot_pan_6")
	PORT_ADJUSTER(50, "MID TOM PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_MIDTOMS)
	PORT_START("pot_pan_7")
	PORT_ADJUSTER(50, "LO TOM PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_LOTOMS)
	PORT_START("pot_pan_8")
	PORT_ADJUSTER(50, "RIDE PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_RIDE)
	PORT_START("pot_pan_9")
	PORT_ADJUSTER(50, "CRASH PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_CRASH)
	PORT_START("pot_pan_10")
	PORT_ADJUSTER(50, "CABASA PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_CABASA)
	PORT_START("pot_pan_11")
	PORT_ADJUSTER(50, "TAMB PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_TAMB)
	PORT_START("pot_pan_12")
	PORT_ADJUSTER(50, "HI CONGA PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_HICONGAS)
	PORT_START("pot_pan_13")
	PORT_ADJUSTER(50, "LO CONGA PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_LOCONGAS)
	PORT_START("pot_pan_14")
	PORT_ADJUSTER(50, "COWBELL PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_COWBELL)
	PORT_START("pot_pan_15")
	PORT_ADJUSTER(50, "CLAPS PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_CLAPS)
	PORT_START("pot_pan_16")
	PORT_ADJUSTER(50, "CLICK PAN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_CLICK)

	PORT_START("pot_gain_1")
	PORT_ADJUSTER(100, "BASS GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_BASS)
	PORT_START("pot_gain_2")
	PORT_ADJUSTER(100, "SNARE GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_SNARE)
	PORT_START("pot_gain_3")
	PORT_ADJUSTER(100, "SIDESTICK GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_SIDESTICK)
	PORT_START("pot_gain_4")
	PORT_ADJUSTER(100, "HIHAT GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_HIHAT)
	PORT_START("pot_gain_5")
	PORT_ADJUSTER(100, "HI TOM GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_HITOMS)
	PORT_START("pot_gain_6")
	PORT_ADJUSTER(100, "MID TOM GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_MIDTOMS)
	PORT_START("pot_gain_7")
	PORT_ADJUSTER(100, "LO TOM GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_LOTOMS)
	PORT_START("pot_gain_8")
	PORT_ADJUSTER(100, "RIDE GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_RIDE)
	PORT_START("pot_gain_9")
	PORT_ADJUSTER(100, "CRASH GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_CRASH)
	PORT_START("pot_gain_10")
	PORT_ADJUSTER(100, "CABASA GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_CABASA)
	PORT_START("pot_gain_11")
	PORT_ADJUSTER(100, "TAMB GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_TAMB)
	PORT_START("pot_gain_12")
	PORT_ADJUSTER(100, "HI CONGA GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_HICONGAS)
	PORT_START("pot_gain_13")
	PORT_ADJUSTER(100, "LO CONGA GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_LOCONGAS)
	PORT_START("pot_gain_14")
	PORT_ADJUSTER(100, "COWBELL GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_COWBELL)
	PORT_START("pot_gain_15")
	PORT_ADJUSTER(100, "CLAPS GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_CLAPS)
	PORT_START("pot_gain_16")
	PORT_ADJUSTER(100, "CLICK GAIN") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(linndrum_audio_device::mix_changed), MIX_CLICK)
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
