// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland TR-707/727 drum machines.

    From the Service Notes: “The differences between two models [TR-707 and
    TR-727] are sound data, component values in several audio stages and a
    couple of pin connections at IC30 of Voice board. Both models derive all
    rhythm sounds from PCM-encoded samples of real sounds stored in ROM.”

    The TR-707 and TR-727 have 10 voices, each of which can be played independently.
    Some of the voices have two sample variations, and the hi hat voice has two
    decay variations (open and closed). Only one variation per voice can play at
    a time. This brings the total number of sounds to 15. While there are 16
    drum pads, the two "closed hi hat / short whistle" pads produce the same
    sound.

    There are 2 voice architectures. The "multiplex sound" section uses a single
    8-bit DAC with time-multiplexing for 8 voices. The "single sound" section
    consists of two independent ROMs and 6-bit DACs. More information in:
    tr707_audio_device::device_add_mconfig().

    The TR-707/727 come with the following sounds:
     Pad       TR-707             TR-727
    * 1       Bass Drum 1        Hi Bongo
    * 2       Bass Drum 2        Low Bongo
    * 3       Snare Drum 1       Mute Hi Conga
    * 4       Snare Drum 2       Open Hi Conga
    * 5       Low Tom            Low Conga
    * 6       Mid Tom            Hi Timbale
    * 7       Hi Tom             Low Timbale
    * 8       Rimshot            Hi Agogo
    * 9       Cowbell            Low Agogo
    * 10      Handclap           Cabasa
    * 11      Tambourine         Maracas
    * 12, 13  Closed Hi Hat      Short Whistle
    * 14      Open Hi Hat        Long Whistle
    * 15      Crash Cymbal       Quijada
    * 16      Ride Cymbal        Star Chime

    Documentation and code use the TR-707 sound names.

    Reasons for MACHINE_IMPERFECT_SOUND, and other subtle imperfections:
    * The single-transistor VCAs for the cymbals and hi-hat are modeled as
      linear, which they aren't.
    * No time-multiplexing of the DAC across 8 voices. Using 8 DACs for now.
    * BPFs for bass, snare and toms change their response curve based on
      signal amplitude. The way the circuit is designed might also add distortion.
      This effect is not modeled.
    * Diodes in some of the EGs might shape the decay curve. These are not modeled.
    * Some uncertainty about the hat EG circuit. See update_hat_eg().

    This driver is based on the TR-707 service notes, and is intended as an
    educational tool.
****************************************************************************/

#include "emu.h"
#include "mb63h114.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/m6800/m6801.h"
#include "machine/7474.h"
#include "machine/nvram.h"
#include "machine/rescap.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/flt_biquad.h"
#include "sound/flt_rc.h"
#include "sound/flt_vol.h"
#include "sound/mixer.h"
#include "sound/va_eg.h"
#include "sound/va_vca.h"
#include "video/hd61602.h"
#include "video/pwm.h"
#include "speaker.h"

#include "roland_tr707.lh"

#define LOG_TRIGGER (1U << 1)
#define LOG_SYNC    (1U << 2)
#define LOG_TEMPO   (1U << 3)
#define LOG_ACCENT  (1U << 4)
#define LOG_CART    (1U << 5)
#define LOG_MIX     (1U << 6)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

namespace {

enum mux_voice
{
	MV_BASS = 0,
	MV_SNARE,
	MV_LOW_TOM,
	MV_MID_TOM,
	MV_HI_TOM,
	MV_HI_HAT,
	MV_RIMSHOT,
	MV_HANDCLAP,
	MV_COUNT
};

enum cymbal_voice
{
	CV_CRASH = 0,
	CV_RIDE,
	CV_COUNT
};

enum mix_channel
{
	MC_BASS = 0,
	MC_SNARE,
	MC_LOW_TOM,
	MC_MID_TOM,
	MC_HI_TOM,
	MC_RIMSHOT,
	MC_HANDCLAP,
	MC_HI_HAT,
	MC_CRASH,
	MC_RIDE,
	MC_COUNT
};

// Values for components that differ between the TR707 and TR727.
struct component_config
{
	// IC30 configuration.
	const u8 xck2_input;  // 0: A, 1: B, 2: C, 3: D.

	// Envelope generator discharge resistors.
	const double R95;
	const double R102;
	const double R82;
	const double R91;
	const double R73;

	// Mixer channel pan resistors.
	const double R203;
	const double R205;
	const double R206;
	const double R207;
	const double R208;
	const double R211;
	const double R214;

	// Pre-fader BPF resistors and capacitors.
	const double R255;
	const double C213;
	const double R256;
	const double R233;
	const double C216;
	const double R237;
	const double C220;
	const double R242;
	const double C221;
	const double C222;
	const double R246;
	const double C226;
	const double C230;
	const double R249;
	const double C231;
};

constexpr const component_config TR707_COMPONENTS =
{
	.xck2_input = 3,

	.R95  = RES_M(4.7),
	.R102 = RES_M(2.2),
	.R82  = RES_M(2.2),
	.R91  = RES_M(4.7),
	.R73  = RES_M(2.2),

	.R203 = RES_K(22),
	.R205 = RES_K(33),
	.R206 = RES_K(33),
	.R207 = RES_K(47),
	.R208 = RES_K(22),
	.R211 = RES_K(33),
	.R214 = RES_K(47),

	.R255 = RES_K(10),
	.C213 = CAP_U(0.01),
	.R256 = RES_K(2.2),
	.R233 = RES_K(33),
	.C216 = CAP_P(470),
	.R237 = RES_K(47),
	.C220 = CAP_U(0.01),
	.R242 = RES_K(47),
	.C221 = CAP_U(0.01),
	.C222 = CAP_U(1),
	.R246 = RES_K(33),
	.C226 = CAP_U(0.01),
	.C230 = CAP_U(0.01),
	.R249 = RES_K(22),
	.C231 = CAP_U(0.01),
};

constexpr const component_config TR727_COMPONENTS =
{
	.xck2_input = 2,

	.R95  = RES_M(1),
	.R102 = RES_M(1),  // Not sure. Barely legible.
	.R82  = RES_M(4.7),
	.R91  = RES_M(2.2),
	.R73  = RES_M(4.7),

	.R203 = RES_K(47),
	.R205 = RES_K(47),
	.R206 = RES_K(22),
	.R207 = RES_K(22),
	.R208 = RES_K(47),
	.R211 = RES_K(22),  // Not sure. Barely legible.
	.R214 = RES_K(33),

	.R255 = RES_K(22),
	.C213 = CAP_U(0.0047),
	.R256 = RES_K(47),
	.R233 = RES_K(47),
	.C216 = CAP_U(0.0047),
	.R237 = RES_K(22),
	.C220 = CAP_U(0.012),
	.R242 = RES_K(33),
	.C221 = CAP_U(0.012),
	.C222 = CAP_U(10),
	.R246 = RES_K(47),
	.C226 = CAP_U(0.0022),
	.C230 = CAP_U(0.01),
	.R249 = RES_K(10),
	.C231 = CAP_U(0.0022),
};

}  // anonymous namespace


class tr707_audio_device : public device_t
{
public:
	tr707_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, const component_config &components) ATTR_COLD;
	tr707_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) ATTR_COLD;

	void accent_level_w(u8 data);
	void voice_select_w(u8 data);
	void voice_trigger_w(u16 data);

	DECLARE_INPUT_CHANGED_MEMBER(mix_adjusted);
	DECLARE_INPUT_CHANGED_MEMBER(master_volume_adjusted);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr const double VCC = 5;  // Volts.
	static constexpr const double VBE = 0.6;  // BJT base-emitter voltage drop.
	static constexpr const u16 MAX_CYMBAL_COUNTER = 0x8000;

	static filter_biquad_device::biquad_params rc_bpf(double r1, double r2, double c1, double c2, bool crrc);
	static double mux_dac_v(double v_eg, u8 data);

	void advance_sample_w(offs_t offset, u16 data);

	void update_hat_eg();
	void update_master_volume();
	void update_mix(int channel);


	const component_config m_comps;

	required_memory_region m_mux_samples;  // IC34, IC35
	required_device<mb63h114_device> m_mac;  // IC30
	required_device_array<dac08_device, MV_COUNT> m_mux_dac;  // uPC624C (IC37)
	required_device_array<va_rc_eg_device, MV_COUNT> m_mux_eg;
	required_device_array<va_vca_device, MV_COUNT> m_mux_vca;
	required_device<va_rc_eg_device> m_hat_eg;

	required_memory_region_array<CV_COUNT> m_cymbal_samples;
	required_device_array<dac_6bit_r2r_device, CV_COUNT> m_cymbal_dac;
	required_device_array<filter_rc_device, CV_COUNT> m_cymbal_hpf;
	required_device_array<va_rc_eg_device, CV_COUNT> m_cymbal_eg;
	required_device_array<va_vca_device, CV_COUNT> m_cymbal_vca;

	required_ioport_array<MC_COUNT> m_level_sliders;
	required_device_array<filter_volume_device, MC_COUNT> m_level;
	// BPFs before the voice volume faders.
	required_device_array<filter_biquad_device, MC_COUNT> m_voice_bpf;
	// LPFs after the voice volume faders.
	required_device_array<filter_rc_device, MC_COUNT> m_voice_lpf;

	required_device<mixer_device> m_left_mixer;
	required_device<mixer_device> m_right_mixer;
	required_ioport m_master_volume;

	u16 m_triggers = 0x3ff;
	double m_accent_level = 0;
	u8 m_bass_variation = 0;
	u8 m_snare_variation = 0;
	u8 m_rimshot_cowbell = 0;  // 0: rimshot, 1: cowbell.
	u8 m_handclap_tambourine = 0;  // 0: handclap, 1: tambourine.
	bool m_hat_is_closed = false;
	bool m_hat_triggering = false;
	u8 m_mac_c = 0;  // Last value of IC30, output C (pin 7).
	std::array<u16, CV_COUNT> m_cymbal_counter = {MAX_CYMBAL_COUNTER, MAX_CYMBAL_COUNTER};  // TC404 (IC18, IC23), TC4520 (IC20a, IC20b).
};

DEFINE_DEVICE_TYPE(TR707_AUDIO, tr707_audio_device, "tr707_audio_device", "TR-707 audio circuits");

tr707_audio_device::tr707_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, const component_config &components)
	: device_t(mconfig, TR707_AUDIO, tag, owner, 0)
	, m_comps(components)
	, m_mux_samples(*this, ":voices")
	, m_mac(*this, "mac")
	, m_mux_dac(*this, "mux_dac_%u", 1)
	, m_mux_eg(*this, "mux_eg_%u", 1)
	, m_mux_vca(*this, "mux_vca_%u", 1)
	, m_hat_eg(*this, "hat_eg")
	, m_cymbal_samples(*this, ":cymbal%u", 1)
	, m_cymbal_dac(*this, "cymbal_dac_%u", 1)
	, m_cymbal_hpf(*this, "cymbal_hpf_%u", 1)
	, m_cymbal_eg(*this, "cymbal_eg_%u", 1)
	, m_cymbal_vca(*this, "cymbal_vca_%u", 1)
	, m_level_sliders(*this, ":SLIDER_%u", 2)  // Level sliders start at 2.
	, m_level(*this, "level_%u", 1)
	, m_voice_bpf(*this, "voice_bpf_%u", 1)
	, m_voice_lpf(*this, "voice_lpf_%u", 1)
	, m_left_mixer(*this, "lmixer")
	, m_right_mixer(*this, "rmixer")
	, m_master_volume(*this, ":VOLUME")
{
}

tr707_audio_device::tr707_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tr707_audio_device(mconfig, tag, owner, TR707_COMPONENTS)
{
}

void tr707_audio_device::accent_level_w(u8 data)
{
	// D0-D5 are converted to a voltage by a 6-bit resistor array DAC (RA5,
	// RKM7LW502), and buferred by emitter follower Q19, R77, R78. Could not
	// find any information about that DAC. Assuming it is linear.
	m_accent_level = VCC * (data & 0x3f) / double(0x3f) - VBE;
	LOGMASKED(LOG_TRIGGER, "Accent level: %02x - %f\n", data, m_accent_level);
}

void tr707_audio_device::voice_select_w(u8 data)
{
	m_bass_variation = BIT(data, 0);
	m_snare_variation = BIT(data, 1);
	m_rimshot_cowbell = BIT(data, 6);
	m_handclap_tambourine = BIT(data, 7);

	m_hat_is_closed = BIT(data, 5);
	update_hat_eg();

	LOGMASKED(LOG_TRIGGER, "Voice selected: %02x. Bass: %d, snare: %d, rim/cow: %d, hcp/tamb: %d, hat closed: %d\n",
			  data, m_bass_variation, m_snare_variation, m_rimshot_cowbell, m_handclap_tambourine, m_hat_is_closed);
}

void tr707_audio_device::voice_trigger_w(u16 data)
{
	if ((data & 0x03ff) == m_triggers)
		return;
	m_triggers = data & 0x03ff;

	m_mac->xst_w(m_triggers & 0xff);
	for (int i = 0; i < MV_COUNT; ++i)
	{
		if (BIT(m_triggers, i))
			m_mux_eg[i]->set_target_v(0);
		else
			m_mux_eg[i]->set_instant_v(m_accent_level);
	}

	// In addition to the EG for the DAC reference current (handled above), the
	// hi-hat voice has an additional EG and VCA that postrpocesses the DAC
	// output. This is used to create the open and closed hat variations.
	m_hat_triggering = !BIT(m_triggers, MV_HI_HAT);  // Active low.
	update_hat_eg();

	for (int i = 0; i < CV_COUNT; ++i)
	{
		if (!BIT(m_triggers, 8 + i))
		{
			m_cymbal_counter[i] = 0;
			m_cymbal_eg[i]->set_instant_v(m_accent_level);
		}
		else
		{
			m_cymbal_eg[i]->set_target_v(0);
		}
	}

	if (m_triggers != 0x3ff)
		LOGMASKED(LOG_TRIGGER, "Trigger: %03x\n", m_triggers);
}

DECLARE_INPUT_CHANGED_MEMBER(tr707_audio_device::mix_adjusted)
{
	update_mix(param);
}

DECLARE_INPUT_CHANGED_MEMBER(tr707_audio_device::master_volume_adjusted)
{
	update_master_volume();
}

void tr707_audio_device::device_add_mconfig(machine_config &config)
{
	// *** Mulitpex sound section ***

	// Encompasses all voices other than crash and ride cymbal. The 8 voices
	// time-multiplex a single 8-bit DAC. A MUX (IC40, 4051) selects the
	// amplitude envelope generator to act as the DAC reference, and a DMUX
	// (IC41, 4051) routes the output of the DAC to the appropriate sample &
	// hold circuit. The MB63H114 generates sample ROM addresses for the 8
	// voices, and the addressing and timing signals for the MUX, DMUX and DAC
	// input.

	MB63H114(config, m_mac, 1.6_MHz_XTAL);
	m_mac->counter_cb().set(FUNC(tr707_audio_device::advance_sample_w));

	// Decay resistors for the envelope generators.
	const std::array<double, MV_COUNT> MUX_EG_R =
	{
		m_comps.R95,
		m_comps.R102,
		RES_M(4.7),  // R92
		RES_M(4.7),  // R93
		RES_M(4.7),  // R85
		RES_M(4.7),  // R104
		m_comps.R82,
		m_comps.R91,
	};

	// Larger DAC data values result in more negative voltages. So the maximum
	// voltage is produced when data = 0, and the minimum one when data = 0xff.
	constexpr const double MAX_MUX_EG_V = VCC;
	const double mux_dac_vpp = mux_dac_v(MAX_MUX_EG_V, 0) - mux_dac_v(MAX_MUX_EG_V, 0xff);
	const double mux_dac_scale = -(mux_dac_vpp / 2.0) / MAX_MUX_EG_V;

	// Time multiplexing is not emulated at the moment. Using 8 "virtual" DACs
	// and corresponding VCAs instead. The DAC circuit is somewhat involved. See
	// mux_dac_v() for its interpretation.
	for (int i = 0; i < MV_COUNT; ++i)
	{
		VA_RC_EG(config, m_mux_eg[i]).set_r(MUX_EG_R[i]).set_c(CAP_U(0.047));  // [C48, C52-55, C57-58]
		DAC08(config, m_mux_dac[i]);
		VA_VCA(config, m_mux_vca[i]);
		m_mux_dac[i]->add_route(0, m_mux_vca[i], 1.0, 0);
		m_mux_eg[i]->add_route(0, m_mux_vca[i], mux_dac_scale, 1);
	}
	m_mux_eg[MV_HI_HAT]->set_c(CAP_U(1));  // C47


	// *** Hi-hat VCA section ***

	// The hi-hat is one of multiplex voices, but it is post-processed by a
	// high-pass filter (HPF) and a single-transistor VCA with its own envelope
	// generator (EG). The EG has a slow and fast decay mode, which are used for
	// the open and closed hi-hat sounds, respectively. For more info on the hat
	// EG, see update_hat_eg().

	constexpr const double HAT_HPF_SCALE = RES_VOLTAGE_DIVIDER(RES_K(10), RES_R(220));  // R125, R121
	constexpr const double HAT_VCA_V2I_SCALE = 0.0084;  // Converts from input voltage to output current.
	constexpr const double HAT_VCA_SCALE = -HAT_VCA_V2I_SCALE * RES_K(4.7);  // R120, inverting op-amp.

	auto &hat_hpf = FILTER_RC(config, "hat_hpf");
	hat_hpf.set_rc(filter_rc_device::HIGHPASS, RES_R(220), 0, 0, CAP_U(1));  // ~723 Hz, R121, C69
	m_mux_vca[MV_HI_HAT]->add_route(0, hat_hpf, HAT_HPF_SCALE, 0);

	VA_RC_EG(config, m_hat_eg).set_c(CAP_U(1));  // C71
	auto &hat_vca = VA_VCA(config, "hat_vca");  // 2SD1469R, Q32
	hat_hpf.add_route(0, hat_vca, HAT_VCA_SCALE, 0);
	m_hat_eg->add_route(0, hat_vca, 1.0 / VCC, 1);


	// *** Single sound section ***

	// Two voices, each with their own ROM, address counter, 6-bit DAC, HPF,
	// single-transistor VCA, and amplitude EG.

	constexpr const double CYMBAL_HPF_SCALE = RES_VOLTAGE_DIVIDER(RES_K(22), RES_R(470));  // [R59, R60], [R63, R64]
	constexpr const double CYMBAL_VCA_V2I_SCALE = 0.0043;  // Converts from input voltage to output current.
	constexpr const double CYMBAL_VCA_SCALE = -CYMBAL_VCA_V2I_SCALE * RES_K(10);  // [R62, R65], inverting op-amp.

	const std::array<double, CV_COUNT> CYMBAL_EG_R =
	{
		// R58 is 47K in the schematic, but that causes a very quick decay.
		// Using 470K, which matches R61 used in the ride cymbal voice below.
		RES_K(470),  // R58
		RES_2_PARALLEL(/*R61*/RES_K(470), m_comps.R73),
	};

	for (int i = 0; i < CV_COUNT; ++i)
	{
		DAC_6BIT_R2R(config, m_cymbal_dac[i]).set_output_range(0, VCC);
		FILTER_RC(config, m_cymbal_hpf[i]);
		m_cymbal_hpf[i]->set_rc(filter_rc_device::HIGHPASS, RES_R(470), 0, 0, CAP_U(1));  // ~339 Hz, [R63, R64], [C35, C34]
		m_cymbal_dac[i]->add_route(0, m_cymbal_hpf[i], CYMBAL_HPF_SCALE);

		VA_RC_EG(config, m_cymbal_eg[i]).set_r(CYMBAL_EG_R[i]).set_c(CAP_U(1));  // [C50, C49]
		VA_VCA(config, m_cymbal_vca[i]);  // 2SD1469R [Q14, Q15]
		// V2I converter is based on an op-amp in inverting configuration.
		m_cymbal_hpf[i]->add_route(0, m_cymbal_vca[i], CYMBAL_VCA_SCALE, 0);
		m_cymbal_eg[i]->add_route(0, m_cymbal_vca[i], 1.0 / VCC, 1);
	}


	/*** Mixer section ***/

	// Each voice is processed by a band-pass filter (BPF) with a custom tuning
	// per voice, an inverting op-amp with the volume slider as the feedback
	// resistor, and a ~15.9 KHz RC low-pass filter (LPF). Following the LPF,
	// each voice is mixed by dedicated resistors into the left and right
	// channels, for a fixed pan setting per voice.
	// Each voice has its own output, which also follows the LPF. If the voice
	// output is connected, the voice won't be mixed into the left and right
	// channels.

	constexpr const double R_MAX_MASTER_VOLUME = RES_K(50);  // VR212a, VR212b
	constexpr const double R_MAX_CHANNEL_VOLUME = RES_K(50);  // VR202-VR211

	const std::array<double, MC_COUNT> r_mix_left =
	{
		RES_K(22),  // R202
		m_comps.R205,
		m_comps.R208,
		m_comps.R211,
		m_comps.R214,
		RES_K(33),  // R217
		RES_K(33),  // R221
		RES_K(47),  // R224
		RES_K(33),  // R227
		RES_K(22),  // R229
	};

	const std::array<double, MC_COUNT> r_mix_right =
	{
		m_comps.R203,
		m_comps.R206,
		m_comps.R207,
		RES_K(33),  // R212
		RES_K(22),  // R215
		RES_K(33),  // R218
		RES_K(33),  // R220
		RES_K(22),  // R223
		RES_K(22),  // R226
		RES_K(47),  // R230
	};

	// Band-pass filter (BPF) components for each voice.
	// The BPFs for the bass, snare and toms have a parallel path to R1, with
	// a separate resistor and diodes in both directions. This is probably meant
	// to change the filter's response depending on the signal's amplitude.
	// This change in response is not emulated at the moment. The filter is
	// modeled for a high amplitude signal.
	// The rest of the BPFs are typical RC-based BPFs.
	const std::array<std::array<double, 4>, MC_COUNT> bpf_comps =
	{{
		// {R1, R2, C1, C2}

		{RES_2_PARALLEL(m_comps.R255,       /*R231*/RES_K(22)), /*R234*/RES_K(33), m_comps.C213,        /*C214*/CAP_U(1)},
		{RES_2_PARALLEL(m_comps.R256,       /*R232*/RES_K(22)), m_comps.R233,      m_comps.C216,        /*C215*/CAP_U(1)},
		{RES_2_PARALLEL(/*R257*/RES_K(4.7), /*R235*/RES_K(22)), /*R238*/RES_K(47), /*C217*/CAP_U(0.01), /*C218*/CAP_U(1)},
		{RES_2_PARALLEL(/*R258*/RES_K(4.7), /*R236*/RES_K(22)), m_comps.R237,      m_comps.C220,        /*c219*/CAP_U(1)},
		{RES_2_PARALLEL(/*R259*/RES_K(4.7), /*R239*/RES_K(22)), m_comps.R242,      m_comps.C221,        m_comps.C222},

		{/*R240*/RES_K(22), /*R241*/RES_K(47), /*C224*/CAP_P(470), /*C223*/CAP_U(0.01)},
		{/*R243*/RES_K(22), m_comps.R246,      /*C225*/CAP_P(470), m_comps.C226},
		{/*R224*/RES_K(22), /*R245*/RES_K(22), /*C228*/CAP_P(470), /*C227*/CAP_U(0.01)},
		{/*R247*/RES_K(10), /*R250*/RES_K(22), /*C229*/CAP_P(470), m_comps.C230},
		{/*R248*/RES_K(10), m_comps.R249,      /*C232*/CAP_P(470), m_comps.C231},
	}};

	const std::array<device_sound_interface *, MC_COUNT> voices =
	{
		m_mux_vca[MV_BASS],
		m_mux_vca[MV_SNARE],
		m_mux_vca[MV_LOW_TOM],
		m_mux_vca[MV_MID_TOM],
		m_mux_vca[MV_HI_TOM],
		m_mux_vca[MV_RIMSHOT],
		m_mux_vca[MV_HANDCLAP],
		&hat_vca,
		m_cymbal_vca[CV_CRASH],
		m_cymbal_vca[CV_RIDE],
	};

	MIXER(config, m_left_mixer);
	MIXER(config, m_right_mixer);
	for (int i = 0; i < MC_COUNT; ++i)
	{
		FILTER_BIQUAD(config, m_voice_bpf[i]);
		m_voice_bpf[i]->setup(rc_bpf(bpf_comps[i][0], bpf_comps[i][1], bpf_comps[i][2], bpf_comps[i][3], false));
		voices[i]->add_route(0, m_voice_bpf[i], 1.0);

		FILTER_VOLUME(config, m_level[i]);
		m_voice_bpf[i]->add_route(0, m_level[i], -R_MAX_CHANNEL_VOLUME / bpf_comps[i][1]);  // Inverting op-amp.

		FILTER_RC(config, m_voice_lpf[i]).set_lowpass(RES_K(1), CAP_U(0.01));  // ~15.9 KHz.
		m_level[i]->add_route(0, m_voice_lpf[i], 1.0);
		m_voice_lpf[i]->add_route(0, m_left_mixer, -R_MAX_MASTER_VOLUME / r_mix_left[i]);  // Inverting op-amp.
		m_voice_lpf[i]->add_route(0, m_right_mixer,  -R_MAX_MASTER_VOLUME / r_mix_right[i]);  // Same.
	}


	/*** Output section ***/

	// The outputs of the left and right summing op-amps are processed by BPFs
	// with a flat response, and -3dB points at ~0.35 Hz and ~12.4 KHz, before
	// making it to the left and right output sockets.
	auto &left_bpf = FILTER_BIQUAD(config, "left_out_bpf");
	auto &right_bpf = FILTER_BIQUAD(config, "right_out_bpf");
	left_bpf.setup(rc_bpf(RES_K(1), RES_K(47), CAP_U(10), CAP_U(0.01), true));  // R114, R112, C79, C76
	right_bpf.setup(rc_bpf(RES_K(1), RES_K(47), CAP_U(10), CAP_U(0.01), true));  // R113, R111, C80, C77
	m_left_mixer->add_route(0, left_bpf, 1.0);
	m_right_mixer->add_route(0, right_bpf, 1.0);

	constexpr const double VOLTAGE_TO_AUDIO_SCALE = 0.2;
	SPEAKER(config, "speaker", 2).front();
	left_bpf.add_route(0, "speaker", VOLTAGE_TO_AUDIO_SCALE, 0);
	right_bpf.add_route(0, "speaker", VOLTAGE_TO_AUDIO_SCALE, 1);
}

void tr707_audio_device::device_start()
{
	save_item(NAME(m_triggers));
	save_item(NAME(m_accent_level));
	save_item(NAME(m_bass_variation));
	save_item(NAME(m_snare_variation));
	save_item(NAME(m_rimshot_cowbell));
	save_item(NAME(m_handclap_tambourine));
	save_item(NAME(m_hat_is_closed));
	save_item(NAME(m_hat_triggering));
	save_item(NAME(m_mac_c));
	save_item(NAME(m_cymbal_counter));
}

void tr707_audio_device::device_reset()
{
	update_hat_eg();
	update_master_volume();
	for (int channel = 0; channel < MC_COUNT; ++channel)
		update_mix(channel);
}

// Biquad parameters for an RC-based bandpass filter.
// crrc == true:
//   Vin -- C1 -- R1 -+---+- Vout
//                    |   |
//                    R2  C2
//                    |   |
//                   GND  GND
// crrc == false:
//   Vin -- R1 -+- C2 -+- Vout
//              |      |
//              C1     R2
//              |      |
//             GND    GND
// TODO: move to sound/flt_biquad.
filter_biquad_device::biquad_params tr707_audio_device::rc_bpf(double r1, double r2, double c1, double c2, bool crrc)
{
	// BPF transfer function: H(s) = (A * s) / (s ^ 2 + B * s + C)
	// In the C-R-R-C configuration, we have:
	//   A = 1 / (R1 * C2)
	//   B = (R1 * C1 + R2 * C2 + R2 * C1) / (R1 * R2 * C1 * C2)
	//   C = 1 / (R1 * R2 * C1 * C2)
	// In the R-C-C-R configuration, we have:
	//   A = 1 / (R1 * C1)
	//   B = (R1 * C1 + R2 * C2 + R1 * C2) / (R1 * R2 * C1 * C2)
	//   C = 1 / (R1 * R2 * C1 * C2)
	// From the standard transfer function for BPFs, we have:
	//   A = gain * (w / Q)
	//   B = w / Q
	//   C = w ^ 2
	// The calculations of Fc, Q and gain below are derived from the
	// equations above, with some algebra.

	const double x = sqrt(r1 * r2 * c1 * c2);
	const double y = crrc ? (r1 * c1 + r2 * c2 + r2 * c1) : (r1 * c1 + r2 * c2 + r1 * c2);
	const double z = crrc ? (r2 * c1) : (r2 * c2);
	filter_biquad_device::biquad_params params;
	params.type = filter_biquad_device::biquad_type::BANDPASS;
	params.fc = 1.0 / (2.0 * M_PI * x);
	params.q = x / y;
	params.gain = z / y;
	return params;
}

// Computes the output voltage of the mux DAC circuit, given the voltage at the
// output of the envelope generator and the DAC data value.
double tr707_audio_device::mux_dac_v(double v_eg, u8 data)
{
	// These equations use a simplified model of BJTs: constant Vbe, no current
	// flowing into the base. These simplifications do not make e meaningful
	// difference in this particular case.
	constexpr const double R156 = RES_K(12);
	constexpr const double R153 = RES_K(2.2);
	constexpr const double R147 = RES_K(2.2);
	constexpr const double R148 = RES_K(2.2);

	// Compute reference current into the DAC.
	const double v_in = v_eg - VBE;  // VBE of Q44.
	const double i_ref = VCC / R156 + v_in / R153;

	// Compute output current, using the formula in the DAC08 datasheet. The
	// formula specifies "/ 256", rather than the more intuitive "/ 255".
	const double i_out = double(data) / 256.0 * i_ref;

	// Compute voltage at the DAC output. There is a "feed-forward" loop from
	// v_in to the DAC's i_out, via 2 resistors.
	const double v_dac_out = v_in - (R147 + R148) * i_out;

	// Compute the final output of the circuit.
	return v_dac_out - /*Q43*/VBE - /*Q41*/VBE;
}

void tr707_audio_device::advance_sample_w(offs_t offset, u16 data)
{
	// Update MB63H114 clock inputs.
	const u8 b = BIT(offset, 1);
	const u8 c = BIT(offset, 2);
	const u8 d = BIT(offset, 3);
	const u8 xck2 = BIT(offset, m_comps.xck2_input);
	m_mac->xck_w((b << 7) | (b << 6) | (c << 5) | (d << 4) | (d << 3) | (xck2 << 2) | (b << 1) | (b << 0));

	// Update multiplex voice DAC.
	u16 counter = 0;
	const u8 voice = offset & 0x07;
	switch (voice)
	{
		case 0: counter = (data & 0x1ffe) | m_bass_variation; break;
		case 1: counter = (data & 0x1ffe) | m_snare_variation; break;
		case 6: counter = (data & 0x1ffe) | m_rimshot_cowbell; break;
		case 7: counter = (data & 0x1ffe) | m_handclap_tambourine; break;
		default: counter = data; break;
	}
	const u8 sample = m_mux_samples->as_u8((voice << 13) | counter);
	m_mux_dac[voice]->data_w(sample);

	// Update single sound (cymbal) DACs.
	if (!m_mac_c && c)  // Positive edge of C output.
	{
		// Cymbal timing is actually controlled by output B, which is divided by
		// 2 by a flipflop. But C is also B divided by 2, so using C here for
		// convenience.
		for (int i = 0; i < CV_COUNT; ++i)
		{
			if (m_cymbal_counter[i] < MAX_CYMBAL_COUNTER)
			{
				++m_cymbal_counter[i];
				const u8 cymbal_sample = m_cymbal_samples[i]->as_u8(m_cymbal_counter[i] & 0x7fff);
				m_cymbal_dac[i]->data_w(cymbal_sample >> 2);  // Bits D2-D7.
			}
		}
	}
	m_mac_c = c;
}

void tr707_audio_device::update_hat_eg()
{
	// Departures from the hat EG schematic:
	//
	// 1) The schematic shows Q20 connected to the accent level. But this looks
	// wrong. It would keep the EG constantly triggered. This emulation assumes
	// it is connected to the hat trigger signal.
	//
	// 2) The schematic shows two capacitors (C70 and C71) separated by a
	// resistor (R127). This setup would result in a slower attack, the EG would
	// only reach a fraction of its max value, and the hat volume would be very
	// low. Furthermore, C70 isn't really functional in this setup.
	// This emulation ignores C71, which leaves C70 as the only capacitor, and
	// makes the EG behave similar to the other ones in this drum machine.
	//
	// 3) Diode D12 (engaged for the closed hat sound) will probably slow down
	// the EG decay as it reaches lower levels. This effect is not emulated.

	double r_discharge = RES_2_PARALLEL(RES_K(220), RES_M(1));  // R124, R126
	if (m_hat_is_closed)
		r_discharge = RES_2_PARALLEL(r_discharge, RES_K(10));  // R123
	r_discharge += RES_K(4.7);  // R127

	if (m_hat_triggering)
	{
		const double r_charge = RES_R(100);  // R128
		m_hat_eg->set_r(RES_2_PARALLEL(r_charge, r_discharge));
		m_hat_eg->set_target_v(VCC * RES_VOLTAGE_DIVIDER(r_charge, r_discharge));
	}
	else
	{
		m_hat_eg->set_r(r_discharge);
		m_hat_eg->set_target_v(0);
	}

}

void tr707_audio_device::update_master_volume()
{
	const double gain = m_master_volume->read() / 100.0;
	m_left_mixer->set_output_gain(0, gain);
	m_right_mixer->set_output_gain(0, gain);
	LOGMASKED(LOG_MIX, "Master volume adjusted %d %f\n", m_master_volume->read(), gain);
}

void tr707_audio_device::update_mix(int channel)
{
	assert(channel >= 0 && channel < MC_COUNT);
	const double gain = m_level_sliders[channel]->read() / 100.0;
	m_level[channel]->set_gain(gain);
	LOGMASKED(LOG_MIX, "Mix slider adjusted %d: %f\n", channel, gain);
}


namespace {

constexpr const char AUDIO_TAG[] = "tr707_audio";

class roland_tr707_state : public driver_device
{
public:
	static constexpr feature_type unemulated_features() { return feature::TAPE; }

	roland_tr707_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD;

	void tr707(machine_config &config);
	void tr727(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(sync_input_changed);
	DECLARE_INPUT_CHANGED_MEMBER(tempo_pots_adjusted);
	DECLARE_INPUT_CHANGED_MEMBER(accent_pots_adjusted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	enum dinsync_index
	{
		DINSYNC_STARTSTOP = 0,
		DINSYNC_TEMPO,
		DINSYNC_CONTINUE
	};

	enum accent_adc_timer_param
	{
		ACCENT_FLIPFLOP_CLR_CLEAR = 0,
		ACCENT_FLIPFLOP_CLR_ASSERT,
	};

	struct seg_output  // Reference to an LCD segment output.
	{
		enum seg_type
		{
			NONE = 0,
			TRIANGLE,
			TRACK,
			TEXT,
			DOT,
			DIGIT,
		};

		seg_output() : seg_output(NONE, 0, 0) {}
		seg_output(enum seg_type _type, offs_t _x, offs_t _y = 0) : type(_type), x(_x), y(_y) {}

		enum seg_type type;
		offs_t x;
		offs_t y;
	};

	static double discharge_t(double r, double c, double v);

	u8 key_scan_r();
	void key_led_row_w(u8 data);
	void leds_w(u8 data);
	void led_outputs_w(offs_t offset, u8 data);
	u8 trigger_r(offs_t offset);
	void trigger_w(offs_t offset, u8 data);

	u8 lcd_reset_r();
	void lcd_seg_w(offs_t offset, u64 data);
	void lcd_seg_outputs_w(offs_t offset, u8 data);

	int cart_connected_r() const;
	u8 cart_r(offs_t offset);
	void cart_w(offs_t offset, u8 data);

	int midi_rxd_r() const;
	void midi_rxd_w(int state);

	template<enum dinsync_index Which> int dinsync_r() const;
	template<enum dinsync_index Which> void dinsync_w(int state);

	void tempo_source_w(u8 data);
	void update_tempo_line();
	void internal_tempo_clock_cb(int state);
	void update_internal_tempo_timer(bool cap_reset);
	TIMER_DEVICE_CALLBACK_MEMBER(tempo_timer_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(tempo_restart_timer_tick);

	void update_accent_adc();
	void accent_adc_flipflop_cb(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(accent_adc_timer_tick);

	void mem_map(address_map &map) ATTR_COLD;
	void tr_707_727_common(machine_config &config) ATTR_COLD;

	required_device<tr707_audio_device> m_audio;
	required_device<hd6303x_cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cartslot;
	required_device<pwm_display_device> m_led_matrix;
	required_ioport_array<4> m_key_switches;
	output_finder<> m_cart_led;  // D325 (GL9NP2) dual LED (red & green).
	std::vector<std::vector<output_finder<>>> m_leds;

	required_device<hd61602_device> m_lcdc;
	required_device<pwm_display_device> m_lcd_pwm;
	output_finder<10> m_seg_triangle;
	output_finder<4> m_seg_track;
	output_finder<7> m_seg_text;
	output_finder<16, 10> m_seg_dot;
	output_finder<3> m_seg_digit;

	required_ioport m_tapesync_in;
	required_ioport m_dinsync_in;
	required_ioport m_dinsync_config;
	output_finder<3> m_dinsync_out;

	required_device<timer_device> m_tempo_timer;
	required_device<timer_device> m_tempo_restart_timer;
	required_device<ttl7474_device> m_tempo_ff;  // Actually a 4013, IC4a.
	required_ioport m_tempo_trimmer;  // TM-1, 200K(B).
	required_ioport m_tempo_knob;  // VR301, 1M(B).

	required_device<va_rc_eg_device> m_accent_adc_rc;
	required_device<timer_device> m_accent_adc_timer;
	required_device<ttl7474_device> m_accent_adc_ff;  // 4013, IC4b.
	required_ioport m_accent_trimmer_series;  // TM-2, 50K(B).
	required_ioport m_accent_trimmer_parallel;  // TM-3, 200K(B).
	required_ioport m_accent_level;  // VR201, 50K(B).

	output_finder<> m_layout_727;
	bool m_is_727;  // Configuration. Not needed in save state.
	std::vector<std::vector<seg_output>> m_seg_map;  // Configuration.

	u8 m_cart_bank;  // IC27 (40H174), Q5.
	u8 m_key_led_row;  // P60-P63.
	u8 m_tempo_source;  // P64-P66.
	bool m_midi_rxd_bit;

	static constexpr const double VCC = 5.0;  // Volts.
	// Typical negative- and positive-going thresholds for a 4584 Schmitt
	// trigger with a 5V supply.
	static constexpr const double NEG_THRESH_4584 = 2.1;
	static constexpr const double POS_THRESH_4584 = 2.7;
};

roland_tr707_state::roland_tr707_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_audio(*this, AUDIO_TAG)
	, m_maincpu(*this, "maincpu")
	, m_cartslot(*this, "cartslot")
	, m_led_matrix(*this, "led_matrix")
	, m_key_switches(*this, "KEY%u", 0U)
	, m_cart_led(*this, "led_cart")
	, m_leds(4)
	, m_lcdc(*this, "lcdc")
	, m_lcd_pwm(*this, "lcd_pwm")
	, m_seg_triangle(*this, "seg_triangle_%u", 1U)
	, m_seg_track(*this, "seg_track_%u", 1U)
	, m_seg_text(*this, "seg_text_%u", 1U)
	, m_seg_dot(*this, "seg_dot_%u_%u", 1U, 1U)
	, m_seg_digit(*this, "seg_digit_%u", 1U)
	, m_tapesync_in(*this, "TAPESYNC")
	, m_dinsync_in(*this, "DINSYNC")
	, m_dinsync_config(*this, "DINSYNC_CONFIG")
	, m_dinsync_out(*this, "DINSYNC_OUT_%u", 0U)
	, m_tempo_timer(*this, "tempo_clock")
	, m_tempo_restart_timer(*this, "tempo_restart_timer")
	, m_tempo_ff(*this, "tempo_flipflop")
	, m_tempo_trimmer(*this, "TM1")
	, m_tempo_knob(*this, "TEMPO")
	, m_accent_adc_rc(*this, "accent_adc_rc_network")
	, m_accent_adc_timer(*this, "accent_adc_timer")
	, m_accent_adc_ff(*this, "accent_adc_flipflop")
	, m_accent_trimmer_series(*this, "TM2")
	, m_accent_trimmer_parallel(*this, "TM3")
	, m_accent_level(*this, "SLIDER_1")
	, m_layout_727(*this, "is727")
	, m_is_727(false)
	, m_seg_map(hd61602_device::NCOM, std::vector<seg_output>(hd61602_device::NSEG))  // 4x51 LCD segments.
	, m_cart_bank(0)
	, m_key_led_row(0xff)
	, m_tempo_source(0xff)
	, m_midi_rxd_bit(true)  // Initial value is high, for serial "idle".
{
	// Initalize LED outputs.

	constexpr const char *LED_NAME_SUFFIXES[4][6] =
	{
		{"1", "2", "3", "4", "scale_1", "scale_2"},
		{"5", "6", "7", "8", "scale_3", "scale_4"},
		{"9", "10", "11", "12", "group_1", "group_2"},
		{"13", "14", "15", "16", "group_3", "group_4"},
	};
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 6; ++j)
			m_leds[i].push_back(output_finder<>(*this, std::string("led_") + LED_NAME_SUFFIXES[i][j]));


	// Build a mapping (m_seg_map) from the LCD controller's rows and columns (
	// "com" and "seg" in hd61602 parlance) to the corresponding outputs.

	for (int i = 0; i < 4; ++i)
		m_seg_map[3 - i][50] = seg_output(seg_output::TRACK, i);

	constexpr const std::tuple<int, int> SEG_TRIANGLES[10] =
	{
		{0, 48},  // cymbal
		{1, 48},  // hi hat
		{2, 48},  // hcp / tamb
		{3, 48},  // rim / cowbell
		{3, 47},  // hi tom
		{2, 47},  // mid tom
		{1, 47},  // low tom
		{0, 47},  // snare drum
		{0, 46},  // bass drum
		{1, 46},  // accent
	};
	for (int i = 0; i < 10; ++i)
	{
		const std::tuple<int, int> &seg = SEG_TRIANGLES[i];
		m_seg_map[std::get<0>(seg)][std::get<1>(seg)] = seg_output(seg_output::TRIANGLE, i);
	}

	constexpr const std::tuple<int, int> SEG_TEXT[7] =
	{
		{0, 44},  // tempo
		{0, 40},  // measure
		{3, 46},  // track play
		{2, 46},  // track write
		{3, 49},  // pattern play
		{2, 49},  // step write
		{1, 49},  // tap write
	};
	for (int i = 0; i < 7; ++i)
	{
		const std::tuple<int, int> &seg = SEG_TEXT[i];
		m_seg_map[std::get<0>(seg)][std::get<1>(seg)] = seg_output(seg_output::TEXT, i);
	}

	constexpr const std::tuple<int, int> SEG_DIGIT[3][7] =
	{
		{ {0, 45}, {1, 44}, {2, 44}, {3, 44}, {3, 45}, {1, 45}, {2, 45} },
		{ {0, 43}, {1, 42}, {2, 42}, {3, 42}, {3, 43}, {1, 43}, {2, 43} },
		{ {0, 41}, {1, 40}, {2, 40}, {3, 40}, {3, 41}, {1, 41}, {2, 41} },
	};
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 7; ++j)
		{
			const std::tuple<int, int> &seg = SEG_DIGIT[i][j];
			m_seg_map[std::get<0>(seg)][std::get<1>(seg)] = seg_output(seg_output::DIGIT, i, j);
		}
	}

	// Segment addresses (row.col aka com.seg) for the 10x16 matrix of dots.
	// "0.0 .. 3.0" means "0.0 1.0 2.0 3.0" and similar for the decreasing
	// cases susch as "3.1 .. 0.1".

	//  cymbal   0.0  .. 3.0  3.1  .. 0.1  0.20 .. 3.20 3.21 .. 0.21
	//  hat:     0.2  .. 3.2  3.3  .. 0.3  0.22 .. 3.22 3.23 .. 0.23
	//  hcp:     0.4  .. 3.4  3.5  .. 0.5  0.24 .. 3.24 3.25 .. 0.25
	//  rim:     0.6  .. 3.6  3.7  .. 0.7  0.26 .. 3.26 3.27 .. 0.27
	//  hi tom:  0.8  .. 3.8  3.9  .. 0.9  0.28 .. 3.28 3.29 .. 0.29
	//  mid tom: 0.10 .. 3.10 3.11 .. 0.11 0.30 .. 3.30 3.31 .. 0.31
	//  low tom: 0.12 .. 3.12 3.13 .. 0.13 0.32 .. 3.32 3.33 .. 0.33
	//  snare:   0.14 .. 3.14 3.15 .. 0.15 0.34 .. 3.34 3.35 .. 0.35
	//  bass:    0.16 .. 3.16 3.17 .. 0.17 0.36 .. 3.36 3.37 .. 0.37
	//  accent:  0.18 .. 3.18 3.19 .. 0.19 0.38 .. 3.38 3.39 .. 0.39

	// Map each row.col to an x.y output position, based on the table above.
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			m_seg_map[i][2 * j] = seg_output(seg_output::DOT, i, j);
			m_seg_map[3 - i][2 * j + 1] = seg_output(seg_output::DOT, i + 4, j);
			m_seg_map[i][2 * j + 20] = seg_output(seg_output::DOT, i + 8, j);
			m_seg_map[3 - i][2 * j + 21] = seg_output(seg_output::DOT, i + 12, j);
		}
	}
}

void roland_tr707_state::machine_start()
{
	save_item(NAME(m_cart_bank));
	save_item(NAME(m_key_led_row));
	save_item(NAME(m_tempo_source));
	save_item(NAME(m_midi_rxd_bit));

	m_layout_727.resolve();
	m_dinsync_out.resolve();
	m_cart_led.resolve();
	for (std::vector<output_finder<>> &led_row : m_leds)
		for (output_finder<> &led_output : led_row)
			led_output.resolve();

	m_seg_triangle.resolve();
	m_seg_track.resolve();
	m_seg_text.resolve();
	m_seg_dot.resolve();
	m_seg_digit.resolve();
}

void roland_tr707_state::machine_reset()
{
	update_internal_tempo_timer(true);
	update_accent_adc();
	m_layout_727 = m_is_727;
}


double roland_tr707_state::discharge_t(double r, double c, double v)
{
	// RC (dis)charge time to reach V:
	//   dt = -R * C * log( (Vend - V) / (Vend - Vstart) )
	// In this case, Vstart = 5V, Vend = 0V.
	return -r * c * log(v / VCC);
}

u8 roland_tr707_state::key_scan_r()
{
	u8 data = 0x00;

	for (int n = 0; n < 4; n++)
		if (!BIT(m_key_led_row, n))
			data |= m_key_switches[n]->read();

	return data;
}

void roland_tr707_state::key_led_row_w(u8 data)
{
	m_key_led_row = data;

	// key/led selection will enable positive supply to LED anodes (via
	// Q301-304) when low.
	m_led_matrix->write_my(~m_key_led_row & 0x0f);
}

void roland_tr707_state::leds_w(u8 data)
{
	// Data bits D0-D5 (IC301, 40H174) are inverted by IC302 (M54517 transistor
	// array) and connected to LED cathodes (low = on). So D0-D5 are inverted
	// twice.
	m_led_matrix->write_mx(data & 0x3f);
}

void roland_tr707_state::led_outputs_w(offs_t offset, u8 data)
{
	m_leds[offset & 0x3f][offset >> 6] = data;
}

u8 roland_tr707_state::trigger_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		trigger_w(offset, 0);
	return 0x00;  // Data bus pulled low.
}

void roland_tr707_state::trigger_w(offs_t offset, u8 data)
{
	m_cart_bank = BIT(offset, 10);
	m_cart_led = (offset >> 10) & 0x03; // Bit 11: green, bit 10: red.
	m_audio->voice_trigger_w(offset & 0x3ff);
}

u8 roland_tr707_state::lcd_reset_r()
{
	if (!machine().side_effects_disabled())
		m_lcdc->reset_counter_strobe();
	return 0x00;  // Data bus pulled low.
}

void roland_tr707_state::lcd_seg_w(offs_t offset, u64 data)
{
	m_lcd_pwm->matrix(1 << offset, data);
}

void roland_tr707_state::lcd_seg_outputs_w(offs_t offset, u8 data)
{
	const seg_output &seg = m_seg_map[offset & 0x3f][offset >> 6];
	switch (seg.type)
	{
		case seg_output::TRIANGLE: m_seg_triangle[seg.x] = data;   break;
		case seg_output::TRACK:    m_seg_track[seg.x] = data;      break;
		case seg_output::TEXT:     m_seg_text[seg.x] = data;       break;
		case seg_output::DOT:      m_seg_dot[seg.x][seg.y] = data; break;
		case seg_output::DIGIT:
		{
			const u8 bit = BIT(data, 0) << seg.y;
			const u8 mask = 0x7f ^ (1 << seg.y);
			m_seg_digit[seg.x] = (m_seg_digit[seg.x] & mask) | bit;
			break;
		}
		default: break;
	}
}

int roland_tr707_state::cart_connected_r() const
{
	// P54 is pulled up by R24 and connected to pin 1 ("sens") of the cartridge,
	// connection. When the cartridge is connected, P54 will be grounded.
	return m_cartslot->exists() ? 0 : 1;

	// That same signal is also connected to the /G1 and /G2 inputs of IC7 (
	// 40H367 buffer) that routes control signals to the cartridge.
	// 1A <- /RD
	// 2A <- /WR
	// 3A <- Cartslot select, from IC11:Y3 (memory address decoder).
	// 4A <- Cartridge LED, red (see roland_tr707_state::trigger_w()).
	// 5A <- A11
	// 6A <- A10
}

u8 roland_tr707_state::cart_r(offs_t offset)
{
	return m_cartslot->read_ram((m_cart_bank << 12) | offset);
}

void roland_tr707_state::cart_w(offs_t offset, u8 data)
{
	m_cartslot->write_ram((m_cart_bank << 12) | offset, data);
	LOGMASKED(LOG_CART, "Cart write: %d - %04x - %02x\n", m_cart_bank, offset, data);
}

int roland_tr707_state::midi_rxd_r() const
{
	return m_midi_rxd_bit ? 1 : 0;
}

void roland_tr707_state::midi_rxd_w(int state)
{
	m_midi_rxd_bit = bool(state);
}

template<enum roland_tr707_state::dinsync_index Which> int roland_tr707_state::dinsync_r() const
{
	// There is a single DIN-sync socket that can act as either an input or an
	// output. MCU outputs P27, P26 and P21 (start/stop, continue, tempo) are
	// inverted by Q9, Q11 and Q10 respectively (output stage). The output stage
	// is connected to the socket pins. Those pins are also connected to (and
	// inverted by) Q7, Q12 and Q9 (inpug stage). The wiring is such that the
	// input stage will either sense the DIN-sync pins if they are being driven,
	// or the output stage if not.

	if (BIT(m_dinsync_config->read(), 0))  // DIN-sync cable connected and serving as input.
		return BIT(m_dinsync_in->read(), Which) ? 0 : 1;
	else
		return m_dinsync_out[Which] ? 0 : 1;
}

template<enum roland_tr707_state::dinsync_index Which> void roland_tr707_state::dinsync_w(int state)
{
	// See comments in dinsync_r().
	const int new_value = state ? 0 : 1;
	if (new_value == m_dinsync_out[Which])
		return;
	m_dinsync_out[Which] = new_value;
	LOGMASKED(LOG_SYNC, "Set dinsync out %d: %d\n", Which, new_value);

	if (Which == DINSYNC_STARTSTOP && !state)  // Resets tempo on a "start".
	{
		// 1->0 transition on the input of IC3d. This resets the tempo timing
		// capacitor, does an immediate flipflop clear, and does a flipflop
		// preset ~9ms later. That timing matches the DIN-sync requirement to
		// start sending clock pulses 9ms after the start signal.
		m_tempo_timer->reset();
		m_tempo_ff->clear_w(0);  // See comment regarding polarity in:
		m_tempo_ff->clear_w(1);  // tempo_restart_timer_tick().

		const double dt = discharge_t(RES_M(1), CAP_U(0.01), NEG_THRESH_4584);  // R5, C11.
		m_tempo_restart_timer->adjust(attotime::from_double(dt));
		LOGMASKED(LOG_TEMPO, "Reset tempo - dt: %f\n", dt);
	}
}

void roland_tr707_state::tempo_source_w(u8 data)
{
	if (m_tempo_source == data)
		return;
	m_tempo_source = data;
	update_tempo_line();
	LOGMASKED(LOG_TEMPO, "Selected tempo source: %02x\n", data);
}

void roland_tr707_state::update_tempo_line()
{
	// A set of NAND gates (IC2) and a discrete AND circuit (D4, D5, R9)
	// determine which of the tempo clock sources makes it to P20 (TIN).
	const bool tempo = !(BIT(m_tempo_source, 0) && m_tempo_ff->output_comp_r());  // IC2b
	const bool din_sync = !(BIT(m_tempo_source, 1) && dinsync_r<DINSYNC_TEMPO>());  // IC2c
	const bool tape_sync = !(BIT(m_tempo_source, 2) && BIT(m_tapesync_in->read(), 0));  // IC2a
	const bool selected_clock = !(tempo && din_sync && tape_sync);  // IC2d w/ discrente AND.
	m_maincpu->set_input_line(M6801_TIN_LINE, selected_clock ? ASSERT_LINE : CLEAR_LINE);
}

DECLARE_INPUT_CHANGED_MEMBER(roland_tr707_state::sync_input_changed)
{
	update_tempo_line();
}

void roland_tr707_state::update_internal_tempo_timer(bool cap_reset)
{
	// The tempo clock circuit's timing capacitor (C10) starts at 5V and
	// discharges through the tempo trimmer, resistor R318, and tempo knob. When
	// it reaches a Schmitt trigger's (IC3b) negative-going threshold, it will
	// reset the timing capacitor to 5V (via Q3), and clock a flipflop (IC4a)
	// wired as a divide-by-two circuit. The inverted output of IC4a is the
	// tempo clock (24 cycles per quarter note). The firmware can also reset the
	// tempo clock.

	constexpr const double C10 = CAP_U(0.047);
	constexpr const double R318 = RES_K(47);
	constexpr const double TRIMMER_MAX = RES_K(200);  // TM-1.
	constexpr const double KNOB_MAX = RES_M(1);  // VR301.

	// Using 100.0 - x, so that larger values result in higher tempo.
	// The tempo potentiometer has a "C" (inverse audio) taper, wired such that
	// turning it clockwise reduces the resistance. Treating the input as a
	// position (a value increase corresponds to a resistance decrease),
	// results in an "A" (audio) taper wrt the position.
	const double knob_r = KNOB_MAX * RES_AUDIO_POT_LAW((100.0 - m_tempo_knob->read()) / 100.0);
	const double trimmer_r = TRIMMER_MAX * (100.0 - m_tempo_trimmer->read()) / 100.0;
	const double r = R318 + trimmer_r + knob_r;
	const double period = discharge_t(r, C10, NEG_THRESH_4584);

	// If not invoked after a capacitor reset, continue from the current
	// position in the cycle.
	double remaining = period;
	if (!cap_reset)
		remaining *= m_tempo_timer->remaining().as_double() / m_tempo_timer->period().as_double();

	m_tempo_timer->adjust(attotime::from_double(remaining), 0, attotime::from_double(period));
	LOGMASKED(LOG_TEMPO, "Update tempo timer. R: %f, T: %f, Rem: %f, F: %f, BPM: %f\n",
			  r, period, remaining, 1.0 / period, 60.0 / period / 24 / 2);
}

void roland_tr707_state::internal_tempo_clock_cb(int state)
{
	// Divide-by-two configuration: output /Q connected to input D.
	m_tempo_ff->d_w(state);
	// /Q connected to IC2b (4011), part of the tempo source select circuit.
	update_tempo_line();
}

TIMER_DEVICE_CALLBACK_MEMBER(roland_tr707_state::tempo_timer_tick)
{
	m_tempo_ff->clock_w(1);
	m_tempo_ff->clock_w(0);
}

TIMER_DEVICE_CALLBACK_MEMBER(roland_tr707_state::tempo_restart_timer_tick)
{
	// If comparing to the actual circuit, note the inverted writes. The
	// 4013 used in the circuit has active-high 'preset' and 'clear' inputs,
	// in contrast to the 7474 used here to emulate it.
	LOGMASKED(LOG_TEMPO, "Tempo reset timer invoked\n");
	m_tempo_ff->preset_w(0);
	m_tempo_ff->preset_w(1);
	update_internal_tempo_timer(true);
}

DECLARE_INPUT_CHANGED_MEMBER(roland_tr707_state::tempo_pots_adjusted)
{
	update_internal_tempo_timer(false);
}

void roland_tr707_state::update_accent_adc()
{
	// The position of the Accent slider is determined by an ADC circuit
	// consisting of a 4013 flipflop (IC4b), a 4584 Schmitt trigger inverter
	// (IC3e), and passive components. It works by measuring the time it takes
	// for a capacitor (C15) to discharge via the Accent slider, two trimmers
	// and a resistor. The firmware initiates the discharge. Once the voltage
	// reaches the negative-going threshold of IC3e, IRQ2 will be asserted and
	// the capacitor will start charging.

	constexpr const double ACCENT_MAX = RES_K(50);
	constexpr const double TM2_MAX = RES_K(50);
	constexpr const double TM3_MAX = RES_K(200);
	constexpr const double R159 = RES_K(1);

	const double accent = ACCENT_MAX * m_accent_level->read() / 100.0;
	const double tm2 = TM2_MAX * m_accent_trimmer_series->read() / 100.0;
	const double tm3 = TM3_MAX * m_accent_trimmer_parallel->read() / 100.0;

	const double parallel_r = (tm3 > 0 && accent > 0) ? RES_2_PARALLEL(tm3, accent) : 0;
	const double r = R159 + tm2 + parallel_r;
	m_accent_adc_rc->set_r(r);

	const bool charging = m_accent_adc_ff->output_comp_r();
	const double target_v = charging ? VCC : 0.0;
	m_accent_adc_rc->set_target_v(target_v);

	attotime dt = attotime::never;
	const double current_v = m_accent_adc_rc->get_v();
	if (charging && current_v < POS_THRESH_4584)
	{
		dt = m_accent_adc_rc->get_dt(POS_THRESH_4584);
		assert(dt != attotime::never);
		m_accent_adc_timer->adjust(dt, ACCENT_FLIPFLOP_CLR_CLEAR);
	}
	else if (!charging && current_v > NEG_THRESH_4584)
	{
		dt = m_accent_adc_rc->get_dt(NEG_THRESH_4584);
		assert(dt != attotime::never);
		m_accent_adc_timer->adjust(dt, ACCENT_FLIPFLOP_CLR_ASSERT);
	}
	else
	{
		m_accent_adc_timer->reset();
	}

	// fliflop Q connected to P51 (IRQ2).
	const enum line_state irq2 = m_accent_adc_ff->output_r() ? CLEAR_LINE : ASSERT_LINE;
	m_maincpu->set_input_line(HD6301_IRQ2_LINE, irq2);

	LOGMASKED(LOG_ACCENT, "Update accent ADC - R: %f, V: %f, dt: %f, irq2: %d\n",
			  r, target_v, dt.as_double(), irq2);
}

TIMER_DEVICE_CALLBACK_MEMBER(roland_tr707_state::accent_adc_timer_tick)
{
	// If comparing to the actual circuit, note the inverted 'clear' signal. The
	// 4013 used in the circuit has an active-high 'clear' input, in contrast to
	// the 7474 used here to emulate it.
	LOGMASKED(LOG_ACCENT, "Accent ADC timer elapsed: %d\n", param);
	switch (param)
	{
		case ACCENT_FLIPFLOP_CLR_ASSERT:
			m_accent_adc_ff->clear_w(0);  // Active low.
			break;
		case ACCENT_FLIPFLOP_CLR_CLEAR:
			m_accent_adc_ff->clear_w(1);
			break;
	}
}

void roland_tr707_state::accent_adc_flipflop_cb(int state)
{
	LOGMASKED(LOG_ACCENT, "Accent flipflop - charging: %d\n", state);
	update_accent_adc();
}

DECLARE_INPUT_CHANGED_MEMBER(roland_tr707_state::accent_pots_adjusted)
{
	LOGMASKED(LOG_ACCENT, "Accent slider or trimmers changed\n");
	update_accent_adc();
}

void roland_tr707_state::mem_map(address_map &map)
{
	// Address bus A0-A11 pulled high by RA1 and RA2.
	map.unmap_value_low();  // Data bus pulled low by RA301.
	map(0x0000, 0x0000).mirror(0x7ff).unmaprw();
	map(0x0800, 0x0800).mirror(0x7ff).r(FUNC(roland_tr707_state::key_scan_r));
	map(0x1000, 0x1000).mirror(0xfff).r(FUNC(roland_tr707_state::lcd_reset_r)).w(m_lcdc, FUNC(hd61602_device::data_w));
	map(0x2000, 0x27ff).ram().share("nvram1");
	map(0x2800, 0x2fff).ram().share("nvram2");
	map(0x3000, 0x3fff).rw(FUNC(roland_tr707_state::cart_r), FUNC(roland_tr707_state::cart_w));
	map(0x4000, 0x4000).mirror(0xfff).w(FUNC(roland_tr707_state::leds_w));
	map(0x5000, 0x5000).mirror(0xfff).w(m_audio, FUNC(tr707_audio_device::accent_level_w));
	map(0x6000, 0x6fff).rw(FUNC(roland_tr707_state::trigger_r), FUNC(roland_tr707_state::trigger_w));
	map(0x7000, 0x7000).mirror(0xfff).w(m_audio, FUNC(tr707_audio_device::voice_select_w));
	map(0x8000, 0xbfff).mirror(0x4000).rom().region("program", 0);
}


static INPUT_PORTS_START(tr707)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("8") PORT_CODE(KEYCODE_8)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("10") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("11") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("12") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("13") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("14") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("15") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("16") PORT_CODE(KEYCODE_Y)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Start") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Stop/Cont") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Pattern Clear") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Scale") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Last Step") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Inst Select") PORT_CODE(KEYCODE_F)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tempo") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Track") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Pattern") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Shuffle") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Group A") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Group B") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Group C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Group D") PORT_CODE(KEYCODE_V)

	PORT_START("TEMPO")  // Tempo knob, VR301, 1M(C) (EWH-LNAF20C16, inverse audio taper).
	PORT_ADJUSTER(50, "Tempo") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(roland_tr707_state::tempo_pots_adjusted), 0)

	PORT_START("VOLUME")  // Master volume slider, VR212, 50K(B) (S3028, dual-gang).
	PORT_ADJUSTER(50, "Volume Level") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(tr707_audio_device::master_volume_adjusted), 0)

	// SLIDER_* are all 50K B-curve (linear) potentiomenters (part# S2018).
	PORT_START("SLIDER_1")  // VR201
	PORT_ADJUSTER(50, "Accent") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(roland_tr707_state::accent_pots_adjusted), 0)
	PORT_START("SLIDER_2")  // VR202
	PORT_ADJUSTER(100, "Bass Drum Level") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(tr707_audio_device::mix_adjusted), MC_BASS)
	PORT_START("SLIDER_3")  // VR203
	PORT_ADJUSTER(100, "Snare Drum Level") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(tr707_audio_device::mix_adjusted), MC_SNARE)
	PORT_START("SLIDER_4")  // VR204
	PORT_ADJUSTER(100, "Low Tom Level") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(tr707_audio_device::mix_adjusted), MC_LOW_TOM)
	PORT_START("SLIDER_5")  // VR205
	PORT_ADJUSTER(100, "Mid Tom Level") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(tr707_audio_device::mix_adjusted), MC_MID_TOM)
	PORT_START("SLIDER_6")  // VR206
	PORT_ADJUSTER(100, "Hi Tom Level") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(tr707_audio_device::mix_adjusted), MC_HI_TOM)
	PORT_START("SLIDER_7")  // VR207
	PORT_ADJUSTER(100, "Rimshot / Cowbell Level") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(tr707_audio_device::mix_adjusted), MC_RIMSHOT)
	PORT_START("SLIDER_8")  // VR208
	PORT_ADJUSTER(100, "Handclap / Tambourine Level") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(tr707_audio_device::mix_adjusted), MC_HANDCLAP)
	PORT_START("SLIDER_9")  // VR209
	PORT_ADJUSTER(100, "Hi Hat Level") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(tr707_audio_device::mix_adjusted), MC_HI_HAT)
	PORT_START("SLIDER_10")  // VR210
	PORT_ADJUSTER(100, "Crash Level") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(tr707_audio_device::mix_adjusted), MC_CRASH)
	PORT_START("SLIDER_11")  // VR211
	PORT_ADJUSTER(100, "Ride Level") PORT_CHANGED_MEMBER(AUDIO_TAG, FUNC(tr707_audio_device::mix_adjusted), MC_RIDE)

	// Trimmer defaults based on calibration instructions in the service notes.
	PORT_START("TM1")  // Tempo trimmer, TM-1, 200K(B) (RVF8P01-204).
	PORT_ADJUSTER(62, "TRIMMER: tempo (TM1)")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(roland_tr707_state::tempo_pots_adjusted), 0)
	PORT_START("TM2")  // Accent series trimmer, TM-2, 50K(B) (RVF8P01-503).
	PORT_ADJUSTER(45, "TRIMMER: accent, series (TM2)")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(roland_tr707_state::accent_pots_adjusted), 0)
	PORT_START("TM3")  // Accent parallel trimmer, TM-3, 200K(B) (RVF8P01-204).
	PORT_ADJUSTER(15, "TRIMMER: accent, parallel (TM3)")
		 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(roland_tr707_state::accent_pots_adjusted), 0)

	PORT_START("DINSYNC_CONFIG")
	PORT_CONFNAME(0x01, 0x01, "DIN sync input cable connected")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x01, DEF_STR(Yes))

	PORT_START("DINSYNC")  // SYNC socket (J4). Bit numbers map to `enum dinsync_index`.
	// SYNC start needs to be active for SYNC tempo to have an effect.
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SYNC start/stop") PORT_CODE(KEYCODE_B)  // Pin 1.
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SYNC tempo") PORT_CODE(KEYCODE_M)  // Pin 3. 24 cloks per quarter note.
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(roland_tr707_state::sync_input_changed), 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SYNC Continue") PORT_CODE(KEYCODE_N)  // Pin 5.

	PORT_START("TAPESYNC")  // TAPE LOAD / SYNC IN socket (J5).
	// Input connected to a filter (C30, R43, C31, R47), followed by a
	// comparator (IC17b) referenced to 3V.
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TAPE LOAD / SYNC IN")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(roland_tr707_state::sync_input_changed), 0)

	PORT_START("START_STOP_IN")  // START/STOP socket (J8).
	// This physical input is active low (pulled high by R66), but gets inverted
	// by Q16, which is what MCU reads. So treat as active high.
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("START / STOP IN") PORT_CODE(KEYCODE_BACKSLASH)
INPUT_PORTS_END

void roland_tr707_state::tr_707_727_common(machine_config &config)
{
	HD6303X(config, m_maincpu, 4_MHz_XTAL); // HD6303XF
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_tr707_state::mem_map);

	// P20 (TIN) connected to IC2d (4011), which outputs the selected tempo
	// source. Not configured here, look for M6801_TIN_LINE.
	m_maincpu->in_p2_cb().set(FUNC(roland_tr707_state::midi_rxd_r)).mask(0x01).lshift(3);

	m_maincpu->out_p2_cb().set(FUNC(roland_tr707_state::dinsync_w<DINSYNC_TEMPO>)).bit(1);
	m_maincpu->out_p2_cb().append(m_accent_adc_ff, FUNC(ttl7474_device::clock_w)).bit(2);
	m_maincpu->out_p2_cb().append("mdout", FUNC(midi_port_device::write_txd)).bit(4);
	m_maincpu->out_p2_cb().append_output("TAPESYNC_OUT").bit(5);  // TAPE SAVE / SYNC OUT socket (J6).
	m_maincpu->out_p2_cb().append(FUNC(roland_tr707_state::dinsync_w<DINSYNC_CONTINUE>)).bit(6);
	m_maincpu->out_p2_cb().append(FUNC(roland_tr707_state::dinsync_w<DINSYNC_STARTSTOP>)).bit(7);

	m_maincpu->in_p5_cb().set_constant(0).mask(0x01);  // Connected to gnd.
	// P51 (IRQ2) connected to IC4b (m_accent_adc_ff), pin Q. Not configured
	// here, look for HD6301_IRQ2_LINE.
	// A table in the service notes describes P52 and P53 as "unused, pulled up".
	m_maincpu->in_p5_cb().append_constant(0x0c).mask(0x0c);
	m_maincpu->in_p5_cb().append(FUNC(roland_tr707_state::cart_connected_r)).mask(0x01).lshift(4);
	m_maincpu->in_p5_cb().append(FUNC(roland_tr707_state::dinsync_r<DINSYNC_CONTINUE>)).mask(0x01).lshift(5);
	m_maincpu->in_p5_cb().append(FUNC(roland_tr707_state::dinsync_r<DINSYNC_STARTSTOP>)).mask(0x01).lshift(6);
	m_maincpu->in_p5_cb().append_ioport("START_STOP_IN").mask(0x01).lshift(7);

	m_maincpu->out_p6_cb().set(FUNC(roland_tr707_state::key_led_row_w)).mask(0x0f);
	m_maincpu->out_p6_cb().append(FUNC(roland_tr707_state::tempo_source_w)).rshift(4).mask(0x07);
	m_maincpu->out_p6_cb().append_output("RIMTRIG_OUT").bit(7).invert();  // Inverted by Q13.

	NVRAM(config, "nvram1", nvram_device::DEFAULT_ALL_0); // HM6116LP-4 + battery
	NVRAM(config, "nvram2", nvram_device::DEFAULT_ALL_0); // HM6116LP-4 + battery

	TIMER(config, m_tempo_timer).configure_generic(FUNC(roland_tr707_state::tempo_timer_tick));
	TIMER(config, m_tempo_restart_timer).configure_generic(FUNC(roland_tr707_state::tempo_restart_timer_tick));
	TTL7474(config, m_tempo_ff, 0);  // 4013, IC4a.
	m_tempo_ff->comp_output_cb().set(FUNC(roland_tr707_state::internal_tempo_clock_cb));

	VA_RC_EG(config, m_accent_adc_rc).set_c(CAP_U(0.27));  // C15.
	TIMER(config, m_accent_adc_timer).configure_generic(FUNC(roland_tr707_state::accent_adc_timer_tick));
	TTL7474(config, m_accent_adc_ff, 0);  // 4013, IC4b.
	m_accent_adc_ff->d_w(1);  // D tied to VCC.
	m_accent_adc_ff->comp_output_cb().set(FUNC(roland_tr707_state::accent_adc_flipflop_cb));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(FUNC(roland_tr707_state::midi_rxd_w));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	GENERIC_CARTSLOT(config, m_cartslot, generic_plain_slot, nullptr, "tr707_cart");

	HD61602(config, m_lcdc);
	m_lcdc->write_segs().set(FUNC(roland_tr707_state::lcd_seg_w));
	PWM_DISPLAY(config, m_lcd_pwm).set_size(hd61602_device::NCOM, hd61602_device::NSEG);
	m_lcd_pwm->output_x().set(FUNC(roland_tr707_state::lcd_seg_outputs_w));

	PWM_DISPLAY(config, m_led_matrix).set_size(4, 6);
	m_led_matrix->output_x().set(FUNC(roland_tr707_state::led_outputs_w));

	config.set_default_layout(layout_roland_tr707);
}

void roland_tr707_state::tr707(machine_config &config)
{
	tr_707_727_common(config);
	TR707_AUDIO(config, m_audio, TR707_COMPONENTS);
	m_is_727 = false;
}

void roland_tr707_state::tr727(machine_config &config)
{
	tr_707_727_common(config);
	TR707_AUDIO(config, m_audio, TR727_COMPONENTS);
	m_is_727 = true;
}

ROM_START(tr707)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("os_rom_firmware.ic13", 0x0000, 0x4000, CRC(3517ea00) SHA1(f5d57a79abf49131bd9832ae4e2dbced914ea523)) // 27128

	ROM_REGION(0x10000, "voices", 0) // "BD-MT" (IC34) and "HT-TAMB" (IC35)
	ROM_LOAD("hn61256p_c71_15179694.ic34", 0x0000, 0x8000, CRC(a196489b) SHA1(fd2bfe67d4d03d2b2134aa7feebe9167c44b1f8d))
	ROM_LOAD("hn61256p_c72_15179695.ic35", 0x8000, 0x8000, CRC(b05302e5) SHA1(5cc866f345906d817147ae2a61bc36d7be926511))

	ROM_REGION(0x8000, "cymbal1", 0) // "Crash Cymbal"
	ROM_LOAD("hn61256p_c73_15179696.ic19", 0x0000, 0x8000, CRC(b0bea07f) SHA1(965e23ad71e1f95d56307fa67272725dff46ba67))

	ROM_REGION(0x8000, "cymbal2", 0) // "Ride Cymbal"
	ROM_LOAD("hn61256p_c74_15179697.ic22", 0x0000, 0x8000, CRC(9411943a) SHA1(6c7c0f002ed66e4ccf182a4538d9bb239623ac43))
ROM_END

ROM_START(tr727)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("osv_1.0_hd4827128.ic13", 0x0000, 0x4000, CRC(49954161) SHA1(8eb033d9729aa84cc3c33b8ce30925ff3c35e70a))

	ROM_REGION(0x10000, "voices", 0) // "BNG-HTB" (IC34) and "LTB-MC" (IC35)
	ROM_LOAD("hn61256p_15179694.ic34", 0x0000, 0x8000, NO_DUMP)
	ROM_LOAD("hn61256p_15179695.ic35", 0x8000, 0x8000, NO_DUMP)

	ROM_REGION(0x8000, "cymbal1", 0) // "Quijada"
	ROM_LOAD("hn61256p_15179696.ic19", 0x0000, 0x8000, NO_DUMP)

	ROM_REGION(0x8000, "cymbal2", 0) // "Star Chime"
	ROM_LOAD("hn61256p_15179697.ic22", 0x0000, 0x8000, NO_DUMP)
ROM_END

} // anonymous namespace


SYST(1985, tr707, 0, 0, tr707, tr707, roland_tr707_state, empty_init, "Roland", "TR-707 Rhythm Composer", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
SYST(1985, tr727, 0, 0, tr727, tr707, roland_tr707_state, empty_init, "Roland", "TR-727 Rhythm Composer", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
