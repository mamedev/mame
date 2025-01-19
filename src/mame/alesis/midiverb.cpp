// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
The Midiverb is a digital delay & reverb unit.

The computer portion of the device is very simple. The firmware runs on a
80C31 microcontroller. It reads the 4 buttons, drives the two 7-segment
displays, and listens to MIDI for program changes. It also controls which
program (effect) is running on the DSP.

An interesting aspect of the Midiverb is its DSP, which is built out of discrete
logic components. This runs custom microcode consisting of 4 instructions.

The UI is very simple. The user can choose one of 63 effects by using the
"up" and "down" buttons on the unit. The effect can also be set via MIDI
program changes, and the MIDI channel is configurable ("channel" button). The
"defeat" button will run the 64th effect, which is just silence. That same
silence program is also enabled temporarily when switching between effects.
Finally, there is a wet/dry control knob. For more information on the audio
hardware, see midiverb_state::configure_audio().

This driver is based on https://www.youtube.com/watch?v=JNPpU08YZjk
and https://www.youtube.com/watch?v=5DYbirWuBaU, and is intended as an
educational tool.

TODO: DSP emulation (coming soon).

Usage notes:

The driver comes with an interactive layout.

MIDI is optional, and can be configured as follows:
./mame -listmidi  # List MIDI devices, physical or virtual (e.g. DAWs).
./mame -window midiverb -midiin "{midi device}"

Audio inputs are emulated using MAME's sample playback mechanism.
- Create a new directory `midiverb` under the `samples` MAME directory.
- Copy the .wav files to be used as audio inputs into that directory. Use names:
  left.wav and right.wav for the left and right input respectively. Note that
  MAME does not support stereo .wav files, so they need to be separate. It is
  also fine to just include one of the two files.
- When the emulation is running, press Space to trigger the processing of those
  files.
- Look out for any errors, such as unsupported file format.
- If there is distortion, adjust INPUT LEVEL (in the Slider Controls menu).
- Use the "DRY/WET MIX" Slider Control to adjust the wet/dry ratio.

- At the moment, DSP emulation is mostly a passthrough. It just does a
  12-bit quantization and causes a resampling at its lowish sample rate. But it
  should still be possible to hear the difference between the dry and wet
  signals, in part due to all the filtering stages.
*/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "machine/rescap.h"
#include "sound/flt_biquad.h"
#include "sound/flt_rc.h"
#include "sound/mixer.h"
#include "sound/samples.h"
#include "video/pwm.h"
#include "speaker.h"

#include "alesis_midiverb.lh"

#define LOG_PROGRAM_CHANGE (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_PROGRAM_CHANGE)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

// Emulation of the MIDIverb DSP circuit, built out of discrete logic
// components.
class midiverb_dsp : public device_t, public device_sound_interface
{
public:
	midiverb_dsp(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	void program_select_w(u8 data);

protected:
	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream, const std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs) override;

private:
	sound_stream *m_stream = nullptr;
	u8 m_program = 0;

	static constexpr const int CLOCKS_PER_INSTRUCTION = 2;
	static constexpr const int INSTRUCTIONS_PER_SAMPLE = 128;
	static constexpr const float DAC_MAX_V = 4.8F;
};

DEFINE_DEVICE_TYPE(MIDIVERB_DSP, midiverb_dsp, "midiverb_dsp", "MIDIverb discrete DSP");

midiverb_dsp::midiverb_dsp(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MIDIVERB_DSP, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
}

void midiverb_dsp::program_select_w(u8 data)
{
	const u8 new_program = data & 0x3f;
	if (m_program == new_program)
		return;

	m_stream->update();
	m_program = new_program;
	LOGMASKED(LOG_PROGRAM_CHANGE, "Program changed to: %d\n", m_program);
}

void midiverb_dsp::device_start()
{
	// The actual sample rate works out to 23,437.5 KHz. But stream_alloc takes
	// a u32, and .value() will round it down to 23,437 KHz.
	const XTAL sample_clock = 6_MHz_XTAL / CLOCKS_PER_INSTRUCTION / INSTRUCTIONS_PER_SAMPLE;
	m_stream = stream_alloc(1, 2, sample_clock.value());

	save_item(NAME(m_program));
}

void midiverb_dsp::sound_stream_update(sound_stream &stream, const std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs)
{
	assert(inputs.size() == 1);
	assert(outputs.size() == 2);

	const read_stream_view &in = inputs[0];
	write_stream_view &left = outputs[0];
	write_stream_view &right = outputs[1];
	const int n = in.samples();

	for (int i = 0; i < n; ++i)
	{
		// Analog-to-digital conversion is done with a 12-bit DAC+SAR.
		// Note that samples in the stream are treated as voltages (see
		// configure_audio()). Convert the voltage to the range: -/+1.
		const float sample_in = std::clamp(in.get(i), -DAC_MAX_V, DAC_MAX_V) / DAC_MAX_V;
		// Quantize to 12 bits, keeping in mind that the range is -1 - 1 (reason
		// for "/ 2"), then convert to 13 bits ("* 2").
		const s16 quantized = floorf(sample_in * ((1 << 12) / 2 - 1)) * 2;
		assert(quantized > -4096 && quantized < 4096);

		// TODO: Implement DSP logic (coming soon).

		// Digital-to-analog conversion uses the 12-bit DAC and 1 extra bit
		// (LSB), for a total of 13 bits. The extra bit is implemented by
		// conditionally injecting extra current into the current-to-voltage
		// converter that follows the DAC.
		const float sample_out = DAC_MAX_V * float(quantized) / ((1 << 13) / 2 - 1);
		left.put(i, sample_out);
		right.put(i, sample_out);
	}
}

namespace {

constexpr const char MAINCPU_TAG[] = "80c31";

class midiverb_state : public driver_device
{
public:
	midiverb_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MAINCPU_TAG)
		, m_digit_device(*this, "pwm_digit_device")
		, m_digit_out(*this, "digit_%d", 1U)
		, m_mix(*this, "mix")
		, m_input_level(*this, "audio_input_level")
		, m_audio_in(*this, "audio_input")
		, m_dsp(*this, "discrete_dsp")
		, m_left_out(*this, "left_mixer_out")
		, m_right_out(*this, "right_mixer_out")
	{
	}

	void midiverb(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(mix_changed);
	DECLARE_INPUT_CHANGED_MEMBER(audio_input_play);
	DECLARE_INPUT_CHANGED_MEMBER(audio_input_level);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	u8 midi_rxd_r() const;
	void midi_rxd_w(int state);
	void digit_select_w(u8 data);
	void digit_latch_w(u8 data);
	void digit_out_update_w(offs_t offset, u8 data);

	void update_mix();
	void update_audio_input_level();

	void program_map(address_map &map) ATTR_COLD;
	void external_memory_map(address_map &map) ATTR_COLD;
	void configure_audio(machine_config &config) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_digit_device;
	output_finder<2> m_digit_out;  // 2 x MAN4710A (7-seg display), DS1 & DS2.
	required_ioport m_mix;
	required_ioport m_input_level;
	required_device<samples_device> m_audio_in;
	required_device<midiverb_dsp> m_dsp;
	required_device<mixer_device> m_left_out;
	required_device<mixer_device> m_right_out;

	bool m_midi_rxd_bit = true; // Start high for serial idle.
	u8 m_digit_latch_inv = 0x00;
	u8 m_digit_mask = 0x00;

	enum
	{
		LEFT_CHANNEL = 0,
		RIGHT_CHANNEL
	};
};

u8 midiverb_state::midi_rxd_r() const
{
	return m_midi_rxd_bit ? 1 : 0;
}

void midiverb_state::midi_rxd_w(int state)
{
	m_midi_rxd_bit = state;
}

void midiverb_state::digit_select_w(u8 data)
{
	// The digit select signals (bit 0 and 1) are active-low. They connect to
	// the base of PNP transistors (2N4403, Q4 and Q3 for DS1 and DS2
	// respectively). When low, power is connected to the MAN4710 anode inputs.
	m_digit_mask = ~data & 0x03;
	m_digit_device->matrix(m_digit_mask, m_digit_latch_inv);
}

void midiverb_state::digit_latch_w(u8 data)
{
	// The Data bus is connected to the latch in an unintuitive way. Same goes
	// for the connections from the latch to the 7seg display. Presumably done
	// to save board space, which was limited.
	const u8 descrambled = bitswap<7>(data, 1, 6, 7, 4, 0, 2, 5);

	// Inverting because segment LEDs are active-low, but pwm_display_device
	// expects active-high.
	m_digit_latch_inv = ~descrambled & 0x7f;
	m_digit_device->matrix(m_digit_mask, m_digit_latch_inv);
}

void midiverb_state::digit_out_update_w(offs_t offset, u8 data)
{
	// Digits are ordered from left to right. So offset 0 (corresponding to
	// digit_1) is the most significant digit.
	m_digit_out[offset] = data;
}

void midiverb_state::update_mix()
{
	const float wet = m_mix->read() / 100.0F;
	const float dry = 1.0F - wet;

	m_left_out->set_input_gain(0, dry);
	m_left_out->set_input_gain(1, wet);

	m_right_out->set_input_gain(0, dry);
	m_right_out->set_input_gain(1, wet);
}

void midiverb_state::update_audio_input_level()
{
	const float gain = m_input_level->read() / 100.0F;
	m_audio_in->set_output_gain(LEFT_CHANNEL, gain);
	m_audio_in->set_output_gain(RIGHT_CHANNEL, gain);
}

void midiverb_state::program_map(address_map &map)
{
	// 2764 ROM has A0-A11 connected to the MCU, and A12 tied high. ROM /OE
	// is tied to MCU A15, so it is only active when A15 is 0.
	map(0x0000, 0x0fff).mirror(0x7000).rom().region(MAINCPU_TAG, 0x1000);
}

void midiverb_state::external_memory_map(address_map &map)
{
	// Address lines ignored when writing to external memory.
	map(0x0000, 0x0000).mirror(0xffff).w(FUNC(midiverb_state::digit_latch_w));
}

static const char *const midiverb_sample_names[] =
{
	"left",
	"right",
	nullptr
};

void midiverb_state::configure_audio(machine_config &config)
{
	static constexpr const double SK_R3 = RES_M(999.99);
	static constexpr const double SK_R4 = RES_R(0.001);

	// Audio input. Emulated with a "samples" device.
	SAMPLES(config, m_audio_in);
	m_audio_in->set_samples_names(midiverb_sample_names);
	m_audio_in->set_channels(2);

	// According to the user manual, input levels can be up to +6 dBV peak when
	// a single input is connected, or 0 dBV when both are connected. 0 dBV
	// means the input voltage can peak at +/- 1.414V. The Samples device
	// returns samples in the range +/- 1. So we can just treat those as
	// voltages.

	// Each input goes through a highpass RC filter (~31Hz cutoff).
	filter_rc_device &left_rc_in = FILTER_RC(config, "left_rc_in");
	filter_rc_device &right_rc_in = FILTER_RC(config, "right_rc_in");
	left_rc_in.set_rc(filter_rc_device::HIGHPASS, RES_K(51), 0, 0, CAP_U(0.1));
	right_rc_in.set_rc(filter_rc_device::HIGHPASS, RES_K(51), 0, 0, CAP_U(0.1));
	m_audio_in->add_route(LEFT_CHANNEL, left_rc_in, 1.0);
	m_audio_in->add_route(RIGHT_CHANNEL, right_rc_in, 1.0);

	// Following the RC HPF, the signal is scaled using an opamp in the
	// non-inverting amplifier configuration.
	// Using a MIXER for this as a convenient way to apply gain. This stage is
	// not really a mixer.
	mixer_device &left_amp_in = MIXER(config, "left_amp_in");
	mixer_device &right_amp_in = MIXER(config, "right_amp_in");
	const double input_gain = 1 + RES_K(10) / RES_K(2.4);  // ~5.17x
	left_rc_in.add_route(0, left_amp_in, input_gain);
	right_rc_in.add_route(0, right_amp_in, input_gain);

	// The two channels are mixed in equal proportions to form a mono signal.
	mixer_device &lrmix = MIXER(config, "lrmix");
	left_amp_in.add_route(0, lrmix, 0.5);  // LEFT_CHANNEL.
	right_amp_in.add_route(0, lrmix, 0.5);  // RIGHT_CHANNEL.

	// The mono signal passes through a cascade of 3 Sallen-Key filters, which
	// combined form a 6-pole lowpass filter with a cutoff of ~12 Khz and a
	// boost peaking at 9.7 KHz ("preemphasis").

	// 2-pole LPF, ~6.5 Khz cutoff, flat.
	filter_biquad_device &sk_in1 = FILTER_BIQUAD(config, "sk_in1");
	const double sk_in1_r1 = RES_2_PARALLEL(RES_K(10), RES_K(10));
	sk_in1.opamp_sk_lowpass_setup(sk_in1_r1, RES_K(5.1), SK_R3, SK_R4, CAP_U(0.01), CAP_P(3300));
	lrmix.add_route(0, sk_in1, 1.0);

	// 2-pole LPF, ~13 KHz cutoff, 9-10 dB peak at ~8.4 KHz.
	filter_biquad_device &sk_in2 = FILTER_BIQUAD(config, "sk_in2");
	sk_in2.opamp_sk_lowpass_setup(RES_K(10), RES_K(10), SK_R3, SK_R4, CAP_U(0.01), CAP_P(330));
	sk_in1.add_route(0, sk_in2, 1.0);

	// 2-pole LPF, 16 KHz cutoff, 18 dB peak at ~10.5 KHz nominal.
	// NOTE: This is not a pure SK filter. It seems to be combined with a soft-
	// clipping (?) circuit. The circuit's behavior is not emulated.
	filter_biquad_device &sk_in3 = FILTER_BIQUAD(config, "sk_in3");
	sk_in3.opamp_sk_lowpass_setup(RES_K(4.7), RES_K(4.7), SK_R3, SK_R4, CAP_U(0.047), CAP_P(220));
	sk_in2.add_route(0, sk_in3, 1.0);

	// Next stage is the analog-to-digital conversion. This is done by using a
	// DAC (AM6012) together with a SAR (AM25L04 successive approximation
	// register) and a comparator. This setup does a binary search for the
	// correct digital representation. Since audio in MAME is digital, this
	// process is simplified (no need for a binary search). It is implemented
	// within the DSP.

	// The DSP will read the digital sample, process it, and output left and
	// right samples, using the DAC.
	// A mixer is added here to force a resampling, so that the DSP's low sample
	// rate does not propagate back to the SK filters.
	mixer_device &resampler = MIXER(config, "pre_dsp_resample");
	MIDIVERB_DSP(config, m_dsp);
	sk_in3.add_route(0, resampler, 1.0);
	resampler.add_route(0, m_dsp, 1.0);

	// The left and right DSP outputs are processed by a low-pass reconstruction
	// filter.

	// NOTE: While there is an RC filter followed by an SK filter, there is no
	// buffer between them. Therefore, it might not be very accurate to treat
	// them as distinct filters, but probably a decent approximation. The
	// alternative is to derive and implement a custom filter.

	// NOTE: The cutoff frequency of the RC filter is very high (~106 KHz).
	// Including it here might, in theory, decrease audio accuracy ("frequency
	// warping"), unless MAME is running with a very high sample rate.

	// LPF, 1-pole, ~106 KHz cutoff.
	filter_rc_device &left_rc_out = FILTER_RC(config, "left_rc_out");
	filter_rc_device &right_rc_out = FILTER_RC(config, "right_rc_out");
	left_rc_out.set_lowpass(RES_K(1), CAP_P(1500));
	right_rc_out.set_lowpass(RES_K(1), CAP_P(1500));
	m_dsp->add_route(LEFT_CHANNEL, left_rc_out, 1.0);
	m_dsp->add_route(RIGHT_CHANNEL, right_rc_out, 1.0);

	// LPF, 2-pole, ~11 KHz cutoff.
	filter_biquad_device &left_sk_out = FILTER_BIQUAD(config, "left_sk_out");
	filter_biquad_device &right_sk_out = FILTER_BIQUAD(config, "right_sk_out");
	left_sk_out.opamp_sk_lowpass_setup(RES_K(6.8), RES_K(6.8), SK_R3, SK_R4, CAP_P(3300), CAP_P(1500));
	right_sk_out.opamp_sk_lowpass_setup(RES_K(6.8), RES_K(6.8), SK_R3, SK_R4, CAP_P(3300), CAP_P(1500));
	left_rc_out.add_route(0, left_sk_out, 1.0);
	right_rc_out.add_route(0, right_sk_out, 1.0);

	// After reconstruction, each processed (wet) channel is mixed with the
	// corresponding original (dry) channel, based on the position of a
	// user-accessible, dual-gang potentiometer.
	MIXER(config, m_left_out);
	left_amp_in.add_route(0, m_left_out, 1.0);
	left_sk_out.add_route(0, m_left_out, 1.0);
	MIXER(config, m_right_out);
	right_amp_in.add_route(0, m_right_out, 1.0);
	right_sk_out.add_route(0, m_right_out, 1.0);

	// Finally, the signals are attenuated to line level, undoing the ~5x
	// amplification at the input.
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	const double output_gain = RES_VOLTAGE_DIVIDER(RES_K(2.4), RES_R(500));
	m_left_out->add_route(ALL_OUTPUTS, "lspeaker", output_gain);
	m_right_out->add_route(ALL_OUTPUTS, "rspeaker", output_gain);
}

void midiverb_state::machine_start()
{
	m_digit_out.resolve();
	save_item(NAME(m_midi_rxd_bit));
	save_item(NAME(m_digit_latch_inv));
	save_item(NAME(m_digit_mask));
}

void midiverb_state::machine_reset()
{
	update_mix();
	update_audio_input_level();
}

void midiverb_state::midiverb(machine_config &config)
{
	I80C31(config, m_maincpu, 6_MHz_XTAL);  // U55.
	m_maincpu->set_addrmap(AS_PROGRAM, &midiverb_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &midiverb_state::external_memory_map);

	m_maincpu->port_out_cb<1>().set(FUNC(midiverb_state::digit_select_w)).mask(0x03);  // P1.0-P1.1
	m_maincpu->port_out_cb<1>().append(m_dsp, FUNC(midiverb_dsp::program_select_w)).rshift(2);  // P1.2-P1.7
	m_maincpu->port_in_cb<3>().set(FUNC(midiverb_state::midi_rxd_r)).mask(0x01);  // P3.0
	m_maincpu->port_in_cb<3>().append_ioport("buttons").lshift(2).mask(0x3c);  // P3.2-P3.5

	midi_port_device &midi_in = MIDI_PORT(config, "mdin", midiin_slot, "midiin");
	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");
	midi_in.rxd_handler().set(FUNC(midiverb_state::midi_rxd_w));
	midi_in.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	PWM_DISPLAY(config, m_digit_device).set_size(2, 7);  // 2 x MAN4710.
	m_digit_device->set_segmask(0x03, 0x7f);
	m_digit_device->output_digit().set(FUNC(midiverb_state::digit_out_update_w));

	config.set_default_layout(layout_alesis_midiverb);

	configure_audio(config);
}

DECLARE_INPUT_CHANGED_MEMBER(midiverb_state::mix_changed)
{
	update_mix();
}

DECLARE_INPUT_CHANGED_MEMBER(midiverb_state::audio_input_play)
{
	if (newval == 0)
		return;
	m_audio_in->start(LEFT_CHANNEL, 0);
	m_audio_in->start(RIGHT_CHANNEL, 1);
}

DECLARE_INPUT_CHANGED_MEMBER(midiverb_state::audio_input_level)
{
	update_audio_input_level();
}

INPUT_PORTS_START(midiverb)
	PORT_START("buttons")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MIDI CHANNEL") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UP") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DEFEAT") PORT_CODE(KEYCODE_D)

	PORT_START("mix")  // MIX potentiometer at the back of the unit.
	PORT_ADJUSTER(100, "DRY/WET MIX")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(midiverb_state::mix_changed), 0)

	// The following are not controls on the real unit.
	// They control audio input.

	PORT_START("audio_input_control")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("PLAY") PORT_CODE(KEYCODE_SPACE)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(midiverb_state::audio_input_play), 0)

	PORT_START("audio_input_level")
	PORT_ADJUSTER(50, "INPUT LEVEL")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(midiverb_state::audio_input_level), 0)
INPUT_PORTS_END

ROM_START(midiverb)
	ROM_REGION(0x2000, MAINCPU_TAG, 0)
	// U54. 2764 ROM.
	ROM_LOAD("mvop_4-7-86.u54", 0x000000, 0x002000, CRC(14d6596d) SHA1(c6dc579d8086556b2dd4909c8deb3c7006293816))
ROM_END

}  // anonymous namespace

SYST(1986, midiverb, 0, 0, midiverb, midiverb, midiverb_state, empty_init, "Alesis", "MIDIverb", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

