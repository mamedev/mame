// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
The voice board consists of 6 analog voices controlled by the voice computer.

Each voice is built around a CEM3374 (dual oscillator) and a CEM3372 (VCF and
VCA). VCO2 can modulate either VCO1 or the VCF to produce FM effects. There is
also circuitry to generate pulse waves out of the saw ones, and to mix in noise.
The noise source is located on the synth's processor board and shared by all
voice boards and voices. The CEM3372 is combined with a 4051 MUX and other
support circuitry to implement 15 different filter modes.

The voice computer generates 9 control voltages (CVs) for each voice:
- VCO pitch (1 and 2).
- VCO pulse-width (1 and 2).
- VCO volume (1 and 2).
- VCA amplitude.
- VCF cutoff frequency.
- VCF resonance.

There are no analog LFOs or EGs. Those are implemented digitally, and their
effect is incorporated in the CV outputs. All 54 (6 x 9) CVs are generated using
a single 14-bit DAC, whose output is time-multiplexed to 54 Sample and Hold
(S&H) circuits. The CV generation circuit can operate in a "high resolution"
mode which surpases 14 bits (see dac_enable_w()). This is used for pitch CVs.

The voice computer also selects VCO waveforms, switches between different VCF
modes, etc., by controlling switch and multiplexer ICs. Finally, it also
controls the routing and amount of FM by configuring an MDAC (AD7523, one per
voice).

A note on variable "polarity": many bool variables represent signals in the
schematic (e.g. HALTREQ). Some of those signals are active low (e.g. HALTREQ*).
But the variables and functions here (e.g. m_haltreq, haltreq_r()) are always
active-high (active == true).
*/

#include "emu.h"
#include "xpander_vb.h"
#include "machine/rescap.h"

#include <numbers>
#include <utility>

#define LOG_CPU_COMMS   (1U << 1)
#define LOG_VOICE_TIMER (1U << 2)
#define LOG_DAC         (1U << 3)
#define LOG_DAC_VERBOSE (1U << 4)
#define LOG_OSC_MIX     (1U << 5)
#define LOG_FILTER_MODE (1U << 6)
#define LOG_PAN         (1U << 7)

#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


// *** xpander_voice_device ***

// Note: the X in component designations refers to the voice number. For example,
// UX03 refers to U103 for voice 1, U203 for voice 2, and so on. Component
// designations apply to the voice board unless otherwise noted.

const char *const xpander_voice_device::CV_NAMES[xpander_voice_device::NUM_CVS] =
{
	// For Revision D of the voice board.
	// Previous revisions had "PW1" and "VCA" swapped.
	"PW1", "VCA", "VOLA", "VCOF1", "VOLB", "VCFF", "PW2", "VCOF2", "RES"
};

// VCF input B DC block.
const double xpander_voice_device::RX40 = RES_R(680);
const double xpander_voice_device::CX33 = CAP_U(2.2);

xpander_voice_device::xpander_voice_device(const machine_config &mconfig, const char *tag, device_t *owner, device_sound_interface *noise)
	: device_t(mconfig, XPANDER_VOICE, tag, owner, 0)
	, device_sound_interface(mconfig, *this)
	, m_noise(noise)
	, m_sh(*this, "sample_and_hold_%u", 0U)
	, m_osc2_mixer(*this, "osc2_mix")
	, m_vcf(*this, "vcf")
	, m_vcf_out4(*this, "vcf_out4_dcblock")
	, m_pole_mixer(*this, "pole_mixer")
	, m_vca(*this, "vca")
	, m_out(*this, "out")
	, m_fm_mdac(*this, "fm_mdac")
	, m_saw1(*this, "saw1")
	, m_saw2(*this, "saw2")
	, m_tri1(*this, "tri1")
	, m_tri2(*this, "tri2")
	, m_vcofm(*this, "vcofm")
	, m_sync(*this, "sync")
	, m_noi(false)
	, m_filter_mode(0)
	, m_pan(3)
{
	std::fill(m_cv.begin(), m_cv.end(), 0.0);
	std::fill(m_fast.begin(), m_fast.end(), false);
}

xpander_voice_device::xpander_voice_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xpander_voice_device(mconfig, tag, owner, nullptr)
{
}

void xpander_voice_device::device_add_mconfig(machine_config &config)
{
	// Schematic: xpander and matrix-12 voice board, sheet 3 of 4.

	// A set of smoothing sample & hold (S&H) circuits provide control voltages
	// to the CEM3372 and CEM3374. 8 (out of 9) of those have the ability to
	// bypass smoothing.
	for (auto &sh : m_sh)  // UX06, UX07, UX08 and surrounding passive components.
		VA_SMOOTH_SH(config, sh, RES_M(1), CAP_U(0.033), CAP_U(0.0068));

	// TODO: Route VCO control S&Hs once VCOs are implemented. For now, just
	// route them to the output, with 0 gain, so that they have a sink.
	for (int unimplemented : {CV_VCOF1, CV_PW1, CV_VCOF2, CV_PW2})
		m_sh[unimplemented]->add_route(0, *this, 0.0);

	// TODO: Add oscillators 1 and 2.

	if (m_noise)
		m_noise->add_route(0, m_osc2_mixer, 1.0);  // UX14 (CD4053) pin 13

	// The 3 outputs of oscillator 2 and the noise source (common to all voices)
	// are mixed passively (see update_osc2_mix()), dc-blocked, and fed to
	// channel B of the CEM3372.
	FILTER_RC(config, m_osc2_mixer)  // R will be adjusted in update_osc2_mix().
		.set_rc(filter_rc_device::HIGHPASS, RX40, 0, 0, CX33);

	// Filtering is done by the CEM3372, a 4-pole resonant low-pass filter. The
	// Xpander implements 15 filter modes out of that by mixing signals from the
	// different filter stages (aka "pole mixing") and by switching the value of
	// the first capacitor. See update_filter_mode().

	constexpr double RX63 = RES_K(1.2);
	constexpr double RX64 = RES_K(20);

	cem3372_vcf_device::stream_inputs vcf_inputs =
	{
		.channel_a = nullptr,  // TODO: implement.
		.channel_b = m_osc2_mixer,
		.level_a = m_sh[CV_VOLA],
		.level_b = m_sh[CV_VOLB],
		.freq_cv = m_sh[CV_VCFF],
		.res_cv = m_sh[CV_RES],
	};

	CEM3372_VCF(config, m_vcf, vcf_inputs)  // UX12
		.configure_res_in_divider(RX64, RX63)
		.set_c1(CAP_U(0.033))  // CX46 (5%) - Can be modified in update_filter_mode().
		.set_c2(CAP_U(0.033))  // CX41 (5%)
		.set_c3(CAP_U(0.033))  // CX42 (5%)
		.set_c4(CAP_P(330))    // CX43 (MICA)
		// Route gains for OUTPUT_CAP{1-3} computed in update_filter_mode().
		.add_route(cem3372_vcf_device::OUTPUT_CAP1, m_pole_mixer, 0.0)
		.add_route(cem3372_vcf_device::OUTPUT_CAP2, m_pole_mixer, 0.0)
		.add_route(cem3372_vcf_device::OUTPUT_CAP3, m_pole_mixer, 0.0)
		.add_route(cem3372_vcf_device::OUTPUT_VCFOUT, m_vcf_out4, 1.0);

	// The filter's final output is dc-blocked, routed to the resonance input
	// of the CEM3372 (this routing is implicit in the CEM3372 implementation)
	// and to the pole mixer (see update_filter_mode()).
	constexpr double R_VCFOUT = RES_2_PARALLEL(RES_K(100), RX64 + RX63);  // RX68 (1%)
	FILTER_RC(config, m_vcf_out4)  // ~4.1 Hz
		.set_rc(filter_rc_device::HIGHPASS, R_VCFOUT, 0, 0, CAP_U(2.2))  // CX48
		.add_route(0, m_pole_mixer, 0.0);  // Gain computed in update_filter_mode().

	MIXER(config, m_pole_mixer)  // UX17 (CD4051) and resistors to its left.
		.add_route(0, "vca_in_dcblock", -RES_K(68));  // RX66

	// The voltage output of the pole mixer is dc-blocked and converted to a
	// current, and fed to the VCA section of the CEM3372.

	constexpr double RX62 = RES_K(33);
	FILTER_RC(config, "vca_in_dcblock")  // ~2.2 Hz
		.set_rc(filter_rc_device::HIGHPASS, RX62, 0, 0, CAP_U(2.2))  // CX47
		.add_route(0, m_vca, 1.0 / RX62, cem3372_vca_device::INPUT_AUDIO);

	// The VCA level CV is further smoothed by a low-pass filter, before being
	// fed to the corresponding input of the CEM3372.
	m_sh[CV_VCA]->add_route(0, "vca_cv_lpf", 1.0);
	FILTER_RC(config, "vca_cv_lpf")  // ~159 Hz
		.set_lowpass(RES_K(10), CAP_U(0.1))  // RX51, CX40
		.add_route(0, m_vca, 1.0, cem3372_vca_device::INPUT_GAIN);

	// Finally, the current output of the VCA will be converted to a voltage,
	// dc-blocked, and fed to the pan circuit.

	CEM3372_VCA(config, m_vca)  // UX12
		.configure_inv_opamp_voltage_output(RES_K(33))  // UX16 (TL081), RX70
		.add_route(0, m_out, 1.0);

	FILTER_RC(config, m_out)  // 7.3-9.8 Hz
		// Route gains and filter R will be computed in update_pan().
		.set_rc(filter_rc_device::HIGHPASS, RES_K(10), 0, 0, CAP_U(2.2))  // CX49
		.add_route(0, *this, 0.0, 0)
		.add_route(0, *this, 0.0, 1);
}

void xpander_voice_device::device_start()
{
	stream_alloc(2, 2, machine().sample_rate());

	save_item(NAME(m_noi));
	save_item(NAME(m_filter_mode));
	save_item(NAME(m_pan));
	save_item(NAME(m_cv));
	save_item(NAME(m_fast));
}

void xpander_voice_device::device_reset()
{
	update_osc2_mix();
	update_filter_mode();
	update_pan();
}

void xpander_voice_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < 2; ++i)
		stream.copy(i, i);
}

double xpander_voice_device::cem3374_tempco_v() const
{
	// The CEM3374 has an on-chip temperature sensor which generates a voltage
	// on pin 10. This can be used for temperature compensation of the
	// oscillator frequency CVs. For now, just returning the nominal value
	// according to the datasheet.
	return 2.5;
}

void xpander_voice_device::latch0_w(u8 data)  // UX03, 74LS374.
{
	m_fm_mdac = data;
}

void xpander_voice_device::latch1_w(u8 data)  // UX02, 74HC174.
{
	m_vcofm = BIT(data, 0);
	m_saw2 = BIT(data, 1);
	m_tri1 = BIT(data, 2);
	m_sync = BIT(data, 3);
	m_tri2 = BIT(data, 4);
	m_saw2 = BIT(data, 5);
}

void xpander_voice_device::latch2_w(u8 data)  // UX01, 74HC374.
{
	const u8 new_filter_mode = BIT(data, 0, 4);
	if (new_filter_mode != m_filter_mode)
	{
		m_filter_mode = new_filter_mode;
		update_filter_mode();
	}

	const bool new_noi = BIT(data, 4);
	if (new_noi != m_noi)
	{
		m_noi = new_noi;
		update_osc2_mix();
	}

	const u8 new_pan = BIT(data, 5, 3);
	if (new_pan != m_pan)
	{
		m_pan = new_pan;
		update_pan();
	}
}

void xpander_voice_device::set_cv(u8 cv_index, double cv, bool fast)
{
	if (m_cv[cv_index] != cv || m_fast[cv_index] != fast)
		LOGMASKED(LOG_DAC, "%s: CV %s: %f, fast: %d\n", tag(), CV_NAMES[cv_index], cv, fast);

	m_cv[cv_index] = cv;
	m_fast[cv_index] = fast;

	if (cv_index == CV_VCFF)
		cv = vcf_freq_cv(cv);

	if (fast)
		m_sh[cv_index]->sample_fast(cv);
	else
		m_sh[cv_index]->sample(cv);
}

void xpander_voice_device::set_res_cv(double cv)
{
	if (m_cv[CV_RES] != cv)
		LOGMASKED(LOG_DAC, "%s: CV %s: %f\n", tag(), CV_NAMES[CV_RES], cv);

	m_cv[CV_RES] = cv;
	m_sh[CV_RES]->sample(cv);
}

void xpander_voice_device::sh_hold()
{
	for (auto &sh : m_sh)
		sh->hold();
}

double xpander_voice_device::vcf_freq_cv(double vcff)
{
	// Schematic: xpander and matrix-12 voice board, sheet 3 of 4, middle.

	// The sampled VCF frequency CV is converted to the range required by the
	// CEM3372 frequency control input.

	constexpr double RX52 = RES_K(196);  // 1%
	constexpr double RX53 = RES_K(38.3);  // 1%
	constexpr double RX54 = RES_K(1);  // 1%

	return vcff * RES_VOLTAGE_DIVIDER(RX53, RES_2_PARALLEL(RX52, RX54)) +
		   -5.0 * RES_VOLTAGE_DIVIDER(RX52, RES_2_PARALLEL(RX53, RX54));
}

void xpander_voice_device::update_osc2_mix()  // Channel B input to the VCF.
{
	// The effective R of the dcblock filter will depend on which channel B
	// inputs are enabled.
	double r_dcblock = RX40;

	double noise_gain = 0;
	if (m_noi)
	{
		// Resistance of noise circuit on the processor board.
		constexpr double R_NOISE_SOURCE = RES_2_PARALLEL(RES_K(10), RES_R(47));

		constexpr double RX61 = RES_K(3.3);
		r_dcblock += RX61 + R_NOISE_SOURCE;
		noise_gain = RES_VOLTAGE_DIVIDER(RX61, RX40);
	}
	m_noise->set_route_gain(0, m_osc2_mixer, 0, noise_gain);

	m_osc2_mixer->filter_rc_set_RC(filter_rc_device::HIGHPASS, r_dcblock, 0, 0, CX33);

	LOGMASKED(LOG_OSC_MIX, "%s: Osc 2 - noise: %f, dcblock: %f Hz\n",
			  tag(), noise_gain, 1.0 / (2.0 * std::numbers::pi_v<double> * r_dcblock * CX33));
}

void xpander_voice_device::update_filter_mode()
{
	// Schematic: xpander and matrix-12 voice board, sheet 3 of 4, middle- and
	// top-right.

	// The output stages of the 4-pole low-pass filter (CEM3372) can be mixed in
	// different proportions to produce 8 filter variations (aka "pole mixing").
	// Furthermore, the capacitor of the first stage can be switched with a much
	// smaller one, essentially making the first stage a passthrough. The
	// combination of the above produces 2 x 8 = 16 filter modes, 15 of which
	// are unique.
	//
	// Mode   Name in UI        Description
	// ----   ----------        -----------
	// 0x0 - 4 POLE BAND      - 4-pole bandpass
	// 0x1 - 3 HIGH + 1 LOW   - 3-pole highpass + 1-pole lowpass
	// 0x2 - 3 PPHASE + 1 LOW - 3-pole allpass + 1-pole lowpass
	// 0x3 - 2 NOTCH + 1 LOW  - 2-pole notch + 1-pole lowpass
	// 0x4 - 2 POLE LOW       - 2-pole lowpass
	// 0x5 - 2 HIGH + 1 LOW   - 2-pole highpass + 1-pole lowpass
	// 0x6 - 4 POLE LOW       - 4-pole lowpass
	// 0x7 - 2 POLE BAND      - 2-pole bandpass
	// 0x8 - [unused [*]]     - 2-pole highpass + 1-pole lowpass
	// 0x9 - 3 POLE HIGH      - 3-pole highpass
	// 0xa - 3 POLE PHASE     - 3-pole allpass
	// 0xb - 2 POLE NOTCH     - 2-pole notch
	// 0xc - 1 POLE LOW       - 1-pole lowpass
	// 0xd - 2 POLE HIGH      - 2-pole highpass
	// 0xe - 3 POLE LOW       - 3-pole lowpass
	// 0xf - 1 POLE HIGH      - 1-pole highpass
	//
	// [*] Modes 5 and 8 result in the same filter type. The synth uses mode 5
	// for "2-pole highpass + 1-pole lowpass".

	constexpr int STAGE_COUNT = 4;  // Number of filter stages.

	// Each filter stage's output is scaled and/or inverted.
	constexpr double VCF_OUT_GAIN[STAGE_COUNT] =
	{
		// UX13B (TL074 op-amp) in non-inverting configuration.
		1.0 + RES_K(100) / RES_K(1),  // RX80 (1%), RX79 (1%)

		// UX13D (TL074 op-amp) in non-inverting configuration, followed by
		// UX13C in inverting configuration.
		(1.0 + RES_K(100) / RES_K(1)) * (-RES_K(10) / RES_K(10)),  // RX60, RX59, RX78, RX77 (all 1%)

		// UX13A (TL074 op-amp) in non-inverting configuration.
		1.0 + RES_K(100) / RES_K(1),  // RX57 (1%), RX58 (1%)

		// UX15B (TL082 op-amp) in inverting configuration.
		-RES_K(100) / RES_K(100),  // RX67 (1%), RX68 (1%)
	};

	// The scaled stage outputs are mixed in different proportions, depending
	// on the filter mode.
	constexpr double R_MIX[8][STAGE_COUNT] =
	{
		// Tolerance of all resistors except RX75 and RX76: 1%.
		// NC = Not Connected.

		// stage 1    stage 2      stage 3      stage 4
		{ -1,         RES_K(100),  RES_K(49.9), RES_K(100)  },  // NC,   RX89, RX90. RX91
		{ RES_K(100), RES_K(33.2), RES_K(33.2), RES_K(100)  },  // RX88, RX87, RX86, RX85
		{ RES_K(100), RES_K(33.2), RES_K(16.5), RES_K(24.9) },  // RX81, RX82, RX83, RX84
		{ RES_K(100), RES_K(49.9), RES_K(49.9), -1          },  // RX93, RX92, RX94, NC
		{ -1,         RES_K(62),   -1,          -1          },  // NC,   RX75, NC,   NC
		{ RES_K(100), RES_K(49.9), RES_K(100),  -1          },  // RX73, RX74, RX72, NC
		{ -1,         -1,         -1,           RES_K(56)   },  // NC,   NC,   NC,   RX76
		{ RES_K(100), RES_K(100), -1,           -1,         },  // RX95, RX71, NC,   NC
	};

	const std::pair<device_sound_interface *, int> mix_inputs[4] =
	{
		{ m_vcf.target(), cem3372_vcf_device::OUTPUT_CAP1 },
		{ m_vcf.target(), cem3372_vcf_device::OUTPUT_CAP2 },
		{ m_vcf.target(), cem3372_vcf_device::OUTPUT_CAP3 },
		{ m_vcf_out4.target(), 0 },
	};

	// A MUX (UX17, CD4051) selects between the 8 mix configurations.

	const u8 mix_sel = BIT(m_filter_mode, 0, 3);
	double gain[STAGE_COUNT] = { 0.0, 0.0, 0.0, 0.0 };

	for (int i = 0; i < STAGE_COUNT; ++i)
	{
		const double r = R_MIX[mix_sel][i];
		if (r > 0.0)
			gain[i] = VCF_OUT_GAIN[i] / r;

		auto [in, chan] = mix_inputs[i];
		in->set_route_gain(chan, m_pole_mixer, 0, gain[i]);
	}

	// UX14B (CD4053) switches the capacitor of the first filter stage.
	const double c1 = BIT(m_filter_mode, 3) ? CAP_P(100) : CAP_U(0.033);  // CX45, CX46 (5%)
	m_vcf->set_c1(c1);

	LOGMASKED(LOG_FILTER_MODE, "%s: Filter mode: %d, C1: %e, pole gains: %f %f %f %f\n",
			  tag(), m_filter_mode, c1, gain[0], gain[1], gain[2], gain[3]);
}

void xpander_voice_device::update_pan()
{
	// Schematic: xpander and matrix-12 voice board, sheet 4 of 4.

	assert(m_pan >= 0 && m_pan < 8);

	constexpr double R_F = RES_K(10);  // Op-amp feedback resistors: R101 (left) and R103 (right).
	constexpr double R_PAN[7] =
	{
		RES_K(10),
		RES_K(12),
		RES_K(15),
		RES_K(20),
		RES_K(30),
		RES_K(56),
		-1,  // not connected
	};

	double gain_left = 0.0;
	double gain_right = 0.0;
	double r_dcblock = RES_K(10);  // Resistor at the individual voice outputs.

	if (m_pan < 7)
	{
		double r_inv = 0.0;

		const double r_left = R_PAN[m_pan];
		if (r_left > 0.0)
		{
			gain_left = -R_F / r_left;
			r_inv += 1.0 / r_left;
		}

		const double r_right = R_PAN[6 - m_pan];
		if (r_right > 0.0)
		{
			gain_right = -R_F / r_right;
			r_inv += 1.0 / r_right;
		}

		assert(r_inv > 0.0);
		r_dcblock = 1.0 / r_inv;
	}
	// m_pan == 7 will route the voice to a dedicated output (not yet
	// emulated), and not route it to the left and right channels.

	m_out->set_route_gain(0, this, 0, gain_left);
	m_out->set_route_gain(0, this, 1, gain_right);
	m_out->filter_rc_set_RC(filter_rc_device::HIGHPASS, r_dcblock, 0, 0, CAP_U(2.2));  // CX49

	LOGMASKED(LOG_PAN, "%s: Pan %d, L: %f, R: %f, Rdcblock: %f\n",
			  tag(), m_pan, gain_left, gain_right, r_dcblock);
}


// *** xpandervb_device ***

namespace {

ROM_START(xpandervb)
	// The only known difference between firmware versions 1.4 and 1.6 is that
	// 1.4 tries to detect the revision of the voice board, whereas v1.6 skips
	// that check and assumes it is revision D, apparently because the check
	// became unreliable as components aged.

	ROM_DEFAULT_BIOS("1.6")
	ROM_REGION(0x6000, "voicecpu", 0)  // 3 x 2764 8Kx8bit ROMs
	ROM_FILL(0x000000, 0x002000, 0xff)  // U909. Spare 2764 ROM slot. Not populated. Data bus pulled high.

	ROM_SYSTEM_BIOS(0, "1.6", "Xpander voice board, revision 1.6")
	ROMX_LOAD("ca1.6-0.u912", 0x002000, 0x002000, CRC(4e98f114) SHA1(a7babb3d5211b5ef0d03f50540401ae4ae7170f3), ROM_BIOS(0))
	ROMX_LOAD("ca1.6-1.u914", 0x004000, 0x002000, CRC(9b7a7914) SHA1(951bbc197a0140a93bb7621a7aedbe0f4037990c), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "1.4", "Xpander voice board, revision 1.4")
	ROMX_LOAD("ca1.4-0.u912", 0x002000, 0x002000, CRC(79f4490a) SHA1(5ab13ccc3b75df313a447520ca54a492102b7e3b), ROM_BIOS(1))
	ROMX_LOAD("ca1.4-1.u914", 0x004000, 0x002000, CRC(9b7a7914) SHA1(951bbc197a0140a93bb7621a7aedbe0f4037990c), ROM_BIOS(1))
ROM_END

}  // anonymous namespace

// U815A (TL084), R851 and R852 scale the voltage reference selected by U805 (CD4051).
double xpandervb_device::DAC_VREF_SCALER = 1.0 + RES_K(2.43) / RES_K(1);

// A 4V reference generated by D805, R856, R857, R854, U815C is then scaled by
// R853 and R855. This is the DAC voltage reference used when generating CVs
// other than those for oscillator frequencies.
double xpandervb_device::DEFAULT_DAC_VREF = 4.0 * RES_VOLTAGE_DIVIDER(RES_K(18.2), RES_K(10)) * DAC_VREF_SCALER;

xpandervb_device::xpandervb_device(const machine_config &mconfig, const char *tag, device_t *owner, device_sound_interface *noise)
	: device_t(mconfig, XPANDER_VOICEBOARD, tag, owner, 0)
	, device_sound_interface(mconfig, *this)
	, m_noise(noise)
	, m_voicecpu(*this, "voicecpu")
	, m_pit(*this, "pit")
	, m_voices(*this, "voice_%u", 1U)
	, m_haltack_cb(*this)
	, m_haltdis(false)
	, m_haltreq(false)
	, m_haltack(false)
	, m_autodone(true)
	, m_dac_data(0.0)
	, m_dac_fine_v(0.0)
	, m_dac_vref(DEFAULT_DAC_VREF)
	, m_fast(false)
	, m_dac_voice(-1)
{
}

xpandervb_device::xpandervb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xpandervb_device(mconfig, tag, owner, nullptr)
{
}

const tiny_rom_entry *xpandervb_device::device_rom_region() const
{
	return ROM_NAME(xpandervb);
}

void xpandervb_device::device_add_mconfig(machine_config &config)
{
	MC6809(config, m_voicecpu, 16_MHz_XTAL / 2);  // 8 MHz
	m_voicecpu->set_addrmap(AS_PROGRAM, &xpandervb_device::voicecpu_map);

	PIT8253(config, m_pit);
	// TODO: Set clk<0>. Connected to "OSC".
	// Clock inputs 1 and 2 connected to CPU's Q signal, which is 4 times slower
	// than the XTAL input to the CPU.
	m_pit->set_clk<1>(16_MHz_XTAL / 2 / 4);  // 2 MHz
	m_pit->set_clk<2>(16_MHz_XTAL / 2 / 4);  // 2 MHz
	m_pit->out_handler<0>().set(FUNC(xpandervb_device::pit_out0_changed));
	// Output 1 not connected.
	m_pit->out_handler<2>().set(FUNC(xpandervb_device::pit_out2_changed));

	for (int i = 0; i < m_voices.size(); ++i)
	{
		XPANDER_VOICE(config, m_voices[i], m_noise)
			.add_route(0, *this, 1.0, OUTPUT_LEFT)
			.add_route(1, *this, 1.0, OUTPUT_RIGHT);
	}
}

void xpandervb_device::device_start()
{
	stream_alloc(2, 2, machine().sample_rate());

	save_item(NAME(m_haltdis));
	save_item(NAME(m_haltreq));
	save_item(NAME(m_haltack));
	save_item(NAME(m_autodone));
	save_item(NAME(m_dac_data));
	save_item(NAME(m_dac_fine_v));
	save_item(NAME(m_dac_vref));
	save_item(NAME(m_fast));
	save_item(NAME(m_dac_voice));
}

void xpandervb_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < 2; ++i)
		stream.copy(i, i);
}

void xpandervb_device::voicecpu_map(address_map &map)
{
	// Signal names (e.g. LATCH0*) are from the schematic.

	map.unmap_value_high();  // Data bus pulled high by resistors in Voice Board.

	// RAM's /CS connected to A15.
	map(0x0000, 0x1fff).mirror(0x6000).ram().share("voiceram");  // 1 x 6264 (U917).

	// ROM and port/peripheral decoding done by 1/2 LS139 (U915A).

	// Ports / peripherals enabled when U915 O0 is low AND one of
	// READ*, WRITE*, TIMER* is low. Additional decoding done by 74LS42 (U918).
	map(0x8000, 0x8007).mirror(0x03f8).w(FUNC(xpandervb_device::latch0_w));  // LATCH0*
	map(0x8400, 0x8407).mirror(0x03f8).w(FUNC(xpandervb_device::latch1_w));  // LATCH1*
	map(0x8800, 0x8807).mirror(0x03f8).w(FUNC(xpandervb_device::latch2_w));  // LATCH2*
	map(0x8c00, 0x8dff).mirror(0x0200).w(FUNC(xpandervb_device::dac_w));  // SDAC (inverted by LS04).
	map(0x9000, 0x9000).mirror(0x03ff).w(FUNC(xpandervb_device::dataout_w));  // DATAOUT*
	map(0x9400, 0x9400).mirror(0x03ff).r(FUNC(xpandervb_device::datain_r));  // DATAIN*
	map(0x9800, 0x9803).mirror(0x03fc).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));  // TIMER*
	map(0x9c00, 0x9c00).mirror(0x03ff).unmaprw();  // Unused. U918 output 7 not connected.

	// ROMs only get enabled if BA* is high. I.e. disabled when main CPU has
	// control of the voice cpu's bus.
	map(0xa000, 0xffff).rom().region("voicecpu", 0);  // 3 x 6264 (U909, U912, U914)
}

void xpandervb_device::reset_w(int state)
{
	const bool reset_voice = state;
	const bool voice_resetting = (m_voicecpu->input_line_state(INPUT_LINE_RESET) == ASSERT_LINE);
	if (reset_voice == voice_resetting)
		return;

	auto func = timer_expired_delegate(FUNC(xpandervb_device::deferred_reset_w), this);
	machine().scheduler().synchronize(std::move(func), reset_voice ? 1 : 0);
}

TIMER_CALLBACK_MEMBER(xpandervb_device::deferred_reset_w)
{
	m_voicecpu->set_input_line(INPUT_LINE_RESET, param ? ASSERT_LINE : CLEAR_LINE);
	LOGMASKED(LOG_CPU_COMMS, "Voice CPU reset line asserted: %d\n", param);
}

void xpandervb_device::haltreq_w(int state)
{
	const bool haltreq = state;
	if (haltreq == m_haltreq)
		return;

	const auto func = timer_expired_delegate(FUNC(xpandervb_device::deferred_haltreq_w), this);
	machine().scheduler().synchronize(func, haltreq ? 1 : 0);
}

TIMER_CALLBACK_MEMBER(xpandervb_device::deferred_haltreq_w)
{
	m_haltreq = param;
	update_line_halt();
	LOGMASKED(LOG_CPU_COMMS, "HALTREQ: %d, HALTDIS: %d\n", m_haltreq, m_haltdis);
}

u8 xpandervb_device::datain_r()
{
	// D0 - AUTODNE*
	const u8 d0 = m_autodone ? 0 : 1;
	// D1 - OSC
	const u8 d1 = 1;  // TODO: Implement.
	// D2-D7 - Not connected (pulled up).
	return 0xfc | (d1 << 1) | d0;
}

void xpandervb_device::dataout_w(u8 data)
{
	// D0 - HALTDS
	const u8 d0 = BIT(data, 0);
	if (m_haltdis != d0)
	{
		m_haltdis = d0;
		update_line_halt();
	}
	// D1 - AUTOST
	m_pit->write_gate0(BIT(data, 1));
	// D2 - PW*
	// TODO: pit gate2 is actually: PW* | OSC. For now, setting to PW*.
	m_pit->write_gate2(BIT(data, 2));
	// TODO: D3 - AUTO*
	// D4-D5 - Not Connected.
}

void xpandervb_device::latch0_w(offs_t offset, u8 data)
{
	// Address decoding done by U804 (74LS42). Only outputs 0-5 are connected
	// to latch enable signals. Similarly for the latchX_w handlers below.
	if (offset <= 5)
		m_voices[5 - offset]->latch0_w(data);
}

void xpandervb_device::latch1_w(offs_t offset, u8 data)
{
	// Decoding by U802.
	if (offset <= 5)
		m_voices[5 - offset]->latch1_w(data);
}

void xpandervb_device::latch2_w(offs_t offset, u8 data)
{
	// Decoding by U801.
	if (offset <= 5)
		m_voices[5 - offset]->latch2_w(data);
}

double xpandervb_device::get_dac_v() const  // Returns output of U812.
{
	constexpr u16 MAX_DAC_DATA = (1U << 14) - 1;
	return -m_dac_data * m_dac_vref / MAX_DAC_DATA;
}

void xpandervb_device::dac_enable_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_DAC_VERBOSE, "DAC: %04x: %02x\n", offset, data);

	// Updating the 7 LSBits for the DAC. Bit 0 is ignored.
	m_dac_data = (m_dac_data & 0x3f80) | (data  >> 1);

	// Determine the DAC reference voltage. The U805 (CD4051) and U814C (CD4053)
	// MUXes determine the voltage sampled in C806, which is then buffered,
	// scaled and routed to the DAC VREF input (m_dac_vref).
	const bool hres = !BIT(offset, 8);  // HRES*
	if (hres)
	{
		// When in high resolution mode (HRES* signal active), the reference
		// voltage is determined by U805 ("ref mux").
		const u8 ref_mux_address = (offset >> 4) & 0x07;  // A4-A6.
		if (ref_mux_address >= 1 && ref_mux_address <= 6)
		{
			// Selects the voiceX temperature compensation voltage.
			m_dac_vref = m_voices[ref_mux_address - 1]->cem3374_tempco_v() * DAC_VREF_SCALER;
		}
		else if (ref_mux_address == 7)
		{
			m_dac_vref = DEFAULT_DAC_VREF;
		}
		// else: ref_mux_address == 0 disconnects the reference voltage.
		// In that case, the last value sampled in C806 is maintained. So
		// maintain the value of m_dac_vref.
	}
	else
	{
		// When HRES* is inactive, U814C (CD4053) will route a fixed reference
		// voltage to the DAC.
		m_dac_vref = DEFAULT_DAC_VREF;
	}

	if (BIT(offset, 7))  // FTSH (fine-tune sample & hold) active.
	{
		// No S&H MUXes are selected, because input D on U803 is high.
		// U814 is activated by FTSH, and samples the DAC voltage in C805/U815 (
		// m_dac_fine_v). This "fine" voltage will be added to the coarser
		// voltage in a future DAC write. This mode is used for generating
		// voltages at a resolution greater than 14 bits.
		m_dac_fine_v = get_dac_v();
		return;
	}

	// FTSH low enables U803 which controls the CV sample & hold circuit that
	// will get activated (via A1-A6).

	const u8 selected_sh = (offset >> 4) & 0x07;  // A4-A6.
	if (selected_sh == 0)  // U803 output 0 is not connected.
		return;

	double dac_v = -RES_K(10) / RES_K(10) * get_dac_v();
	if (hres)  // Turns on U814.
	{
		dac_v += -RES_K(10) / RES_K(30.1) * m_dac_fine_v
				 -RES_K(10) / RES_K(17.4) * m_dac_vref;
	}

	const u8 sh_address = (offset >> 1) & 0x07;  // A1-A3.
	if (selected_sh == 7)
	{
		// Updates resonance (RES). In this case, the voice is selected by
		// `sh_address` (U816). Note that RES is not affected by the FAST*
		// signal.
		m_dac_voice = sh_address;
		m_voices[m_dac_voice]->set_res_cv(dac_v);
	}
	else
	{
		// Voice is selected by `selected_sh` (output of U803). Need a -1
		// because the first U803 output is not connected. The CV index for
		// the given voice is selected by `sh_address`.
		m_dac_voice = selected_sh - 1;
		m_voices[m_dac_voice]->set_cv(sh_address, dac_v, m_fast);
	}
}

void xpandervb_device::dac_clear_w(u8 data)
{
	// (1) Clears latch U806. This results in:
	// - No S&H mux selected (activates output 0 of U803, which is not
	//   connected to anything)
	// - S&H address 0 selected, but this is a no-op since no S&H mux is
	//   selected (see above).
	// - FTSH (active high) deactivated (set low).
	// - HRES* (active low) activated (set low).
	// - Mux U805 activated (by HRES* low), but address 0 selected.
	//   input 0 is not connected.
	//   - No active reference voltage to the DAC. Previously used reference
	//     sampled in C806.
	// Other than disabling the S&Hs, none of the above affect the emulation.
	// Relevant state will be initialized on the next invocation of dac_enable_w().
	if (m_dac_voice >= 0)
	{
		m_voices[m_dac_voice]->sh_hold();
		m_dac_voice = -1;
	}

	// (2) Activates latch for the 7 MSBits of the 14-bit DAC.
	//     D0-D6 used for the 7 MSBits.
	//     If D7 is set, it will enable fast mode on the next invocation to
	//     dac_enable_w().
	m_dac_data = ((u16(data) & 0x7f) << 7) | (m_dac_data & 0x7f);
	m_fast = BIT(data, 7);

	LOGMASKED(LOG_DAC_VERBOSE, "DAC clear %02x: %04x - %d\n", data, m_dac_data, m_fast);
}

void xpandervb_device::dac_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))  // VOICEN*  (A0 high)
		dac_enable_w(offset, data);
	else  // CLEAR* (A0 low)
		dac_clear_w(data);
}

void xpandervb_device::pit_out0_changed(int state)
{
	// OUT0 -> AUTODNE* -> 74LS04 (inverted) -> GATE1
	m_autodone = !state;
	m_pit->write_gate1(state ? 0 : 1);
	LOGMASKED(LOG_VOICE_TIMER, "PIT timer 0 out: %d\n", state);
}

void xpandervb_device::pit_out2_changed(int state)
{
	if (state)  // Interrupt triggered on positive transition of output.
	{
		// Using HOLD_LINE because there is circuitry that clears the IRQ* line
		// when the CPU ACks the IRQ (U919 - 74LS74, U911 - 74LS08, using BS and
		// BA* as inputs).
		m_voicecpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
	}
}

void xpandervb_device::update_line_halt()
{
	bool haltack = false;
	if (m_haltreq && !m_haltdis)
	{
		m_voicecpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		haltack = true;
	}
	else
	{
		m_voicecpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		haltack = false;
	}

	if (haltack != m_haltack)
	{
		m_haltack = haltack;
		m_haltack_cb(m_haltack ? 1 : 0);
	}
}


DEFINE_DEVICE_TYPE(XPANDER_VOICE, xpander_voice_device, "xpander_voice", "Xpander voice")
DEFINE_DEVICE_TYPE(XPANDER_VOICEBOARD, xpandervb_device, "xpandervb", "Xpander voice board")
