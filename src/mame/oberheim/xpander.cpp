// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
The Oberheim Xpander is a 6-voice digitally-controlled analog synthesizer, with
digital modulation sources (LFOs, EGs). Each voice can be configured
independently, which provides a multitimbrality of 6. The Xpander does not have
a keyboard. It is controlled by MIDI, and can accept CV/gate in.

The user interface consists of multiple buttons, 6 rotary encoders, and 3 vacuum
fluorescent displays. There are individual outputs for each voice, along with
left, right and mono outs. Other than MIDI, inputs consist of 6 CV and 2 pedal
inputs, 6 gate and 1 trigger inputs (user-configurable as active high or active
low), and a control input ("chain advance", active low).

The Xpander has two 6809-based computers. The main computer scans for button
presses, drives the VFDs, interprets the digital and analog inputs (MIDI, CVs,
triggers, etc.), and sends parameters to the voice computer.

The voice board consists of 6 analog voices controlled by the voice computer.
The main computer communicates with the voice board by accessing its RAM after
halting its CPU.

Each voice is built around a CEM3374 (dual oscillator) and a CEM3372 (VCF and
VCA). VCO2 can modulate either VCO1 or the VCF to produce FM effects. There is
also circuitry to generate pulse waves out of the saw ones, and to mix in noise.
The noise source is shared for all voices. The CEM3372 is combined with a 4051
MUX and other support circuitry to implement 16 different filter modes.

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
mode which surpases 14 bits (see voice_dac_enable_w()). This is used for pitch
CVs.

The voice computer also selects VCO waveforms, switches between different VCF
modes, etc., by controlling switch and multiplexer ICs. Finally, it also
controls the routing and amount of FM by configuring an MDAC (AD7523, one per
voice).

PCBoards:
- Processor board: main computer.
- Pot board: buttons, rotary encoders, inputs, outputs.
- Display board: control of VFDS.
- Voice board: 6 analog voices, voice computer.
- Power supply.

This driver is based on the Xpander's service manual and schematics, and is
intended as an educational tool. There is no attempt to emulate audio.
*/

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/m6809/m6809.h"
#include "machine/6850acia.h"
#include "machine/adc0804.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/output_latch.h"
#include "machine/pit8253.h"
#include "machine/rescap.h"
#include "machine/timer.h"
#include "video/pwm.h"

#include <algorithm>
#include <array>

#include "oberheim_xpander.lh"

#define LOG_CV_IN       (1U << 1)
#define LOG_SWITCHES    (1U << 2)
#define LOG_ENCODERS    (1U << 3)
#define LOG_CPU_COMMS   (1U << 4)
#define LOG_FIRQ_TIMER  (1U << 5)
#define LOG_VOICE_TIMER (1U << 6)
#define LOG_DAC         (1U << 7)
#define LOG_DAC_VERBOSE (1U << 8)
#define LOG_MEM_PROTECT (1U << 9)

#define VERBOSE (LOG_GENERAL | LOG_ENCODERS | LOG_FIRQ_TIMER | LOG_DAC | LOG_MEM_PROTECT)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


namespace {

// A single voice on the xpander voice board.
// Note: the X in component designations refers to the voice number. For example,
// UX03 refers to U103 for voice 1, U203 for voice 2, and so on. All component
// designations apply to the voice board unless otherwise stated.
class xpander_voice_device : public device_t
{
public:
	xpander_voice_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;

	double cem3374_tempco_v() const;

	void latch0_w(u8 data);
	void latch1_w(u8 data);
	void latch2_w(u8 data);

	void set_cv(u8 cv_index, double cv, bool fast);
	void set_res_cv(double cv);

protected:
	void device_start() override ATTR_COLD;

private:
	static inline constexpr int NUM_CVS = 9;
	static inline constexpr int RES_CV_INDEX = 8;
	static inline constexpr const char *CV_NAMES[NUM_CVS] =
	{
		"VCA", "PW1", "VOLA", "VCOF1", "VOLB", "VCFF", "PW2", "VCOF2", "RES"
	};

	output_finder<> m_fm_mdac;  // Latch connected to AD7523/MP7523.
	output_finder<> m_filter_mode;
	output_finder<> m_noise;
	output_finder<> m_pan;
	output_finder<> m_saw1;
	output_finder<> m_saw2;
	output_finder<> m_tri1;
	output_finder<> m_tri2;
	output_finder<> m_vcofm;
	output_finder<> m_sync;

	std::array<double, NUM_CVS> m_cv;
	std::array<bool, NUM_CVS - 1> m_fast;  // `-1` because RES does not support fast updates.
};

}  // anonymous namespace

DEFINE_DEVICE_TYPE(XPANDER_VOICE, xpander_voice_device, "xpander_voice", "Xpander voice")

xpander_voice_device::xpander_voice_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XPANDER_VOICE, tag, owner, 0)
	, m_fm_mdac(*this, "fm_mdac")
	, m_filter_mode(*this, "filter_mode")
	, m_noise(*this, "noise")
	, m_pan(*this, "pan")
	, m_saw1(*this, "saw1")
	, m_saw2(*this, "saw2")
	, m_tri1(*this, "tri1")
	, m_tri2(*this, "tri2")
	, m_vcofm(*this, "vcofm")
	, m_sync(*this, "sync")
{
	std::fill(m_cv.begin(), m_cv.end(), 0.0);
	std::fill(m_fast.begin(), m_fast.end(), false);
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
	m_filter_mode = BIT(data, 0, 4);
	m_noise = BIT(data, 4);
	m_pan = BIT(data, 5, 3);
}

void xpander_voice_device::set_cv(u8 cv_index, double cv, bool fast)
{

	if (cv == m_cv[cv_index] && fast == m_fast[cv_index])
		return;

	m_cv[cv_index] = cv;
	m_fast[cv_index] = fast;
	LOGMASKED(LOG_DAC, "Voice %s - CV %s: %f, fast: %d\n", basetag(), CV_NAMES[cv_index], cv, fast);
}

void xpander_voice_device::set_res_cv(double cv)
{
	if (cv == m_cv[RES_CV_INDEX])
		return;

	m_cv[RES_CV_INDEX] = cv;
	LOGMASKED(LOG_DAC, "Voice %s - CV %s: %f\n", basetag(), CV_NAMES[RES_CV_INDEX], cv);
}

void xpander_voice_device::device_start()
{
	save_item(NAME(m_cv));
	save_item(NAME(m_fast));
}


namespace {

// The xpander's voice board. Consists of 6 voices and a Z80-based computer for
// controlling them.
// All component designations apply to the voice board unless otherwise stated.
class xpander_voiceboard_device : public device_t
{
public:
	xpander_voiceboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;

	void reset_w(int state);
	void haltreq_w(int state);

	int haltack_r() const { return m_haltack; }

	auto haltack_cb() { return m_haltack_cb.bind(); }

protected:
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void device_start() override ATTR_COLD;

private:
	void voicecpu_map(address_map &map) ATTR_COLD;

	u8 datain_r();
	void dataout_w(u8 data);
	void latch0_w(offs_t offset, u8 data);
	void latch1_w(offs_t offset, u8 data);
	void latch2_w(offs_t offset, u8 data);

	double get_dac_v() const;
	void dac_enable_w(offs_t offset, u8 data);
	void dac_clear_w(u8 data);
	void dac_w(offs_t offset, u8 data);

	void pit_out0_changed(int state);
	void pit_out2_changed(int state);

	void update_line_halt();

	static inline constexpr u16 LOW7_MASK = 0x7f;
	static inline constexpr u16 HIGH7_MASK = LOW7_MASK << 7;

	// U815A (TL084), R851 and R852 scale the voltage reference selected by U805 (CD4051).
	static inline constexpr double DAC_VREF_SCALER = 1.0 + RES_K(2.43) / RES_K(1);

	// A 4V reference generated by D805, R856, R857, R854, U815C is then scaled
	// by R853 and R855. This is the voltage reference used when generating CVs
	// other than those for oscillator frequencies.
	static inline constexpr double DEFAULT_DAC_VREF = 4.0 * RES_VOLTAGE_DIVIDER(RES_K(18.2), RES_K(10)) * DAC_VREF_SCALER;

	required_device<mc6809_device> m_voicecpu;
	required_device<pit8253_device> m_pit;
	required_device_array<xpander_voice_device, 6> m_voices;
	devcb_write_line m_haltack_cb;

	// Many bool variables represent signals in the schematic (e.g. HALTREQ).
	// Some of those signals are active low in the schematic (e.g. HALTREQ*).
	// But the variables here are always active-high (active == true).

	bool m_haltdis;  // Halt disable (HALTDS).
	bool m_haltreq;  // Halt request (HALTREQ).
	bool m_haltack;  // Halt acknowledge (HALTAKN).
	bool m_autodone;  // Autotune done (AUTODNE).
	u16 m_dac_data;
	double m_dac_fine_v;
	double m_dac_vref;  // Sampled in C806 and buffered and scaled by U815.
	bool m_fast;  // Fast CV sampling enabled (FAST).
};

}  // anonymous namespace

DEFINE_DEVICE_TYPE(XPANDER_VOICEBOARD, xpander_voiceboard_device, "xpander_voiceboard", "Xpander voice board")

xpander_voiceboard_device::xpander_voiceboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XPANDER_VOICEBOARD, tag, owner, 0)
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
{
}

void xpander_voiceboard_device::device_add_mconfig(machine_config &config)
{
	MC6809(config, m_voicecpu, 16_MHz_XTAL / 2);  // 8 MHz
	m_voicecpu->set_addrmap(AS_PROGRAM, &xpander_voiceboard_device::voicecpu_map);

	PIT8253(config, m_pit);
	// TODO: Set clk<0>. Connected to "OSC".
	// Clock inputs 1 and 2 connected to CPU's Q signal, which is 4 times slower
	// than the XTAL input to the CPU.
	m_pit->set_clk<1>(16_MHz_XTAL / 2 / 4);  // 2 MHz
	m_pit->set_clk<2>(16_MHz_XTAL / 2 / 4);  // 2 MHz
	m_pit->out_handler<0>().set(FUNC(xpander_voiceboard_device::pit_out0_changed));
	// Output 1 not connected.
	m_pit->out_handler<2>().set(FUNC(xpander_voiceboard_device::pit_out2_changed));

	for (int i = 0; i < m_voices.size(); ++i)
		XPANDER_VOICE(config, m_voices[i]);
}

void xpander_voiceboard_device::device_start()
{
	save_item(NAME(m_haltdis));
	save_item(NAME(m_haltreq));
	save_item(NAME(m_haltack));
	save_item(NAME(m_autodone));
	save_item(NAME(m_dac_data));
	save_item(NAME(m_dac_fine_v));
	save_item(NAME(m_dac_vref));
	save_item(NAME(m_fast));
}

void xpander_voiceboard_device::voicecpu_map(address_map &map)
{
	// Signal names (e.g. LATCH0*) are from the schematic.

	map.unmap_value_high();  // Data bus pulled high by resistors in Voice Board.

	// RAM's /CS connected to A15.
	map(0x0000, 0x1fff).mirror(0x6000).ram().share("voiceram");  // 1 x 6264 (U917).

	// ROM and port/peripheral decoding done by 1/2 LS139 (U915A).

	// Ports / peripherals enabled when U915 O0 is low AND one of
	// READ*, WRITE*, TIMER* is low. Additional decoding done by 74LS42 (U918).
	map(0x8000, 0x8007).mirror(0x03f8).w(FUNC(xpander_voiceboard_device::latch0_w));  // LATCH0*
	map(0x8400, 0x8407).mirror(0x03f8).w(FUNC(xpander_voiceboard_device::latch1_w));  // LATCH1*
	map(0x8800, 0x8807).mirror(0x03f8).w(FUNC(xpander_voiceboard_device::latch2_w));  // LATCH2*
	map(0x8c00, 0x8dff).mirror(0x0200).w(FUNC(xpander_voiceboard_device::dac_w));  // SDAC (inverted by LS04).
	map(0x9000, 0x9000).mirror(0x03ff).w(FUNC(xpander_voiceboard_device::dataout_w));  // DATAOUT*
	map(0x9400, 0x9400).mirror(0x03ff).r(FUNC(xpander_voiceboard_device::datain_r));  // DATAIN*
	map(0x9800, 0x9803).mirror(0x03fc).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));  // TIMER*
	map(0x9c00, 0x9c00).mirror(0x03ff).unmaprw();  // Unused. U918 output 7 not connected.

	// ROMs only get enabled if BA* is high. I.e. disabled when main CPU has
	// control of the voice cpu's bus.
	map(0xa000, 0xffff).rom().region(":voicecpu", 0);  // 3 x 6264 (U909, U912, U914)
}

void xpander_voiceboard_device::reset_w(int state)
{
	const bool reset_voice = state;
	const bool voice_resetting = (m_voicecpu->input_line_state(INPUT_LINE_RESET) == ASSERT_LINE);
	if (reset_voice != voice_resetting)
	{
		m_voicecpu->set_input_line(INPUT_LINE_RESET, reset_voice ? ASSERT_LINE : CLEAR_LINE);
		LOGMASKED(LOG_CPU_COMMS, "Voice CPU reset line asserted: %d\n", reset_voice);
	}
}

void xpander_voiceboard_device::haltreq_w(int state)
{
	const bool haltreq = state;
	if (haltreq == m_haltreq)
		return;
	m_haltreq = haltreq;
	update_line_halt();
}

u8 xpander_voiceboard_device::datain_r()
{
	// D0 - AUTODNE*
	const u8 d0 = m_autodone ? 0 : 1;
	// D1 - OSC
	const u8 d1 = 1;  // TODO: Implement.
	// D2-D7 - Not connected (pulled up).
	return 0xfc | (d1 << 1) | d0;
}

void xpander_voiceboard_device::dataout_w(u8 data)
{
	// D0 - HALTDS
	const u8 d0 = BIT(data, 0);
	if (m_haltdis != d0)
	{
		m_haltdis = d0;
		LOGMASKED(LOG_CPU_COMMS, "Voice HALTDS: %d\n", m_haltdis);
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

void xpander_voiceboard_device::latch0_w(offs_t offset, u8 data)
{
	m_voices[offset]->latch0_w(data);
}

void xpander_voiceboard_device::latch1_w(offs_t offset, u8 data)
{
	m_voices[offset]->latch1_w(data);
}

void xpander_voiceboard_device::latch2_w(offs_t offset, u8 data)
{
	m_voices[offset]->latch2_w(data);
}

double xpander_voiceboard_device::get_dac_v() const  // Returns output of U812.
{
	constexpr u16 MAX_DAC_DATA = (1U << 14) - 1;
	return -m_dac_data * m_dac_vref / MAX_DAC_DATA;
}

void xpander_voiceboard_device::dac_enable_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_DAC_VERBOSE, "DAC: %04x: %02x\n", offset, data);

	// Updating the 7 LSBits for the DAC. Bit 0 is ignored.
	m_dac_data = (m_dac_data & HIGH7_MASK) | (data  >> 1);

	const bool is_hres = !BIT(offset, 8);
	if (is_hres)
	{
		const u8 ref_mux_address = (offset >> 4) & 0x07;  // A4-A6 (U805).
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
		// In that case, the last value sampled in C22 is used as the reference.
		// So we keep the value of m_dac_vref the same.
	}
	else
	{
		// U805 disabled. But Y input of U014 (4053) enabled.
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

	// FTSH low enables one of the first 8 outputs of U803 (controlled
	// by A5-A7). Each of those outputs enables a 4051 mux, whose
	// address is controlled by A1-A3.

	const u8 selected_sh = (offset >> 4) & 0x07;  // A4-A6.
	if (selected_sh == 0)  // U803 output 0 is not connected.
		return;

	double dac_v = -RES_K(10) / RES_K(10) * get_dac_v();
	if (is_hres)  // Turns on U814.
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
		m_voices[sh_address]->set_res_cv(dac_v);
	}
	else
	{
		// Voice is selected by `selected_sh` (output of U803). Need a -1
		// because the first U803 output is not connected. The CV index for
		// the given voice is selected by `sh_address`.
		m_voices[selected_sh - 1]->set_cv(sh_address, dac_v, m_fast);
	}
}

void xpander_voiceboard_device::dac_clear_w(u8 data)
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
	// None of the above affect the emulation. They will be set to valid
	// values on the next invocation to voice_dac_w().

	// (2) Activates latch for the 7 MSBits of the 14-bit DAC.
	//     D0-D6 used for the 7 MSBits.
	//     If D7 is set, it will enable fast mode on the next invocation to
	//     voice_dac_w().
	m_dac_data = ((u16(data) & LOW7_MASK) << 7) | (m_dac_data & LOW7_MASK);
	m_fast = BIT(data, 7);

	LOGMASKED(LOG_DAC_VERBOSE, "DAC clear %02x: %04x - %d\n", data, m_dac_data, m_fast);
}

void xpander_voiceboard_device::dac_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))  // VOICEN*  (A0 high)
		dac_enable_w(offset, data);
	else  // CLEAR* (A0 low)
		dac_clear_w(data);
}

void xpander_voiceboard_device::pit_out0_changed(int state)
{
	// OUT0 -> AUTODNE* -> 74LS04 (inverted) -> GATE1
	m_autodone = !state;
	m_pit->write_gate1(state ? 0 : 1);
	LOGMASKED(LOG_VOICE_TIMER, "PIT timer 0 out: %d\n", state);
}

void xpander_voiceboard_device::pit_out2_changed(int state)
{
	if (state)  // Interrupt triggered on positive transition of output.
	{
		// Using HOLD_LINE because there is circuitry that clears the IRQ* line
		// when the CPU ACks the IRQ (U919 - 74LS74, U911 - 74LS08, using BS and
		// BA* as inputs).
		m_voicecpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
	}
}

void xpander_voiceboard_device::update_line_halt()
{
	if (m_haltreq && !m_haltdis)
	{
		m_voicecpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_haltack = true;
	}
	else
	{
		m_voicecpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_haltack = false;
	}
	m_haltack_cb(m_haltack ? 1 : 0);
}


namespace {

constexpr int NUM_ENCODER_POSITIONS = 30;

class xpander_state : public driver_device
{
public:
	xpander_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD;

	void xpander(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(encoder_moved);
	DECLARE_INPUT_CHANGED_MEMBER(memory_protect_changed);

protected:
	void machine_start() override ATTR_COLD;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(firq_timer_elapsed);
	void firq_timer_preset_w(u8 data);

	u8 proc_datain_r();
	u8 gate_r();
	u8 switch_r(offs_t offset);
	u8 encoder_dir_r();
	u8 encoder_sw_r();

	u8 cv_in_r();
	u8 adc_r(offs_t offset);
	void adc_w(offs_t offset, u8 data);

	void haltack_changed(int state);
	void haltset_w(u8 data);
	void display_w(offs_t offset, u8 data);
	void display_output_w(int display, offs_t offset, u32 data);

	void maincpu_map(address_map &map) ATTR_COLD;

	required_device<mc6809_device> m_maincpu;
	required_device<xpander_voiceboard_device> m_voiceboard;
	required_device<adc0804_device> m_adc;
	required_device<timer_device> m_firq_timer;  // 40103 presetable timer (U12).
	memory_view m_nvram_a_view;
	memory_view m_rom_0_view;
	required_ioport_array<8> m_switch_io;
	required_ioport m_memory_protect_io;
	required_ioport m_gate_io;
	required_ioport_array<6> m_cv_io;
	required_ioport_array<2> m_pedal_io;
	required_device_array<pwm_display_device, 3> m_vfd_devices;
	output_finder<3, 40> m_vfd_outputs;
	output_finder<> m_cassmute;
	output_finder<> m_cassout;

	u8 m_firq_timer_preset;  // Preset for 40103 timer.
	u8 m_selected_cv_in;  // MUX A-C inputs.
	bool m_inhibit_cv_in;  // MUX INHibit input.
	std::array<bool, 6> m_encoder_dir;
	std::array<bool, 6> m_encoder_changed;
	std::array<u64, 3> m_vfd_anode_masks;
};

xpander_state::xpander_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_maincpu(*this, "maincpu")
	, m_voiceboard(*this, "voiceboard")
	, m_adc(*this, "adc")
	, m_firq_timer(*this, "firq_timer")
	, m_nvram_a_view(*this, "nvram_a_view")
	, m_rom_0_view(*this, "rom_0_view")
	, m_switch_io(*this, "switches_%d", 0U)
	, m_memory_protect_io(*this, "memory_protect")
	, m_gate_io(*this, "gate_inputs")
	, m_cv_io(*this, "cv_in_%d", 1U)
	, m_pedal_io(*this, "pedal_%d", 1U)
	, m_vfd_devices(*this, "vfd_%d", 0U)
	, m_vfd_outputs(*this, "vfd_%u_char_%u", 1U, 1U)
	, m_cassmute(*this, "cassmute")
	, m_cassout(*this, "cassout")
	, m_firq_timer_preset(0xff)  // Pulled high.
	, m_selected_cv_in(0x07)  // Pulled high.
	, m_inhibit_cv_in(true)  // Pulled high.
{
	std::fill(m_encoder_dir.begin(), m_encoder_dir.end(), false);
	std::fill(m_encoder_changed.begin(), m_encoder_changed.end(), false);
	std::fill(m_vfd_anode_masks.begin(), m_vfd_anode_masks.end(), 0);
}

TIMER_DEVICE_CALLBACK_MEMBER(xpander_state::firq_timer_elapsed)
{
	// CPU clock divided by U10 (LS393,  processor board).
	constexpr XTAL TIMER_CLOCK = 16_MHz_XTAL / 64;  // 250 KHz.

	// The FIRQ timer is a 40103 timer (U12, processor board). It is configured
	// to reset to its preset value when the count reaches 0 (/TC connected to
	// /PE). The FIRQ is triggered on the next clock cycle, once /TC goes high
	// (/TC used as CLK on an 74LS74 that will drive /FIRQ low when clocked).

	if (param < 0)  // param < 0 means the timer counted down to 0.
	{
		// The output is now low. It will go high on the next clock cycle,
		// which will trigger a bunch of stuff (see 'else' below). The timer
		// count needs to be set to the current value of `m_firq_timer_preset`,
		// which could (in theory) change by next cycle. So pass the current
		// value in `param`. The `-1` accounts for this single timer cycle.
		m_firq_timer->adjust(1 * attotime::from_hz(TIMER_CLOCK), m_firq_timer_preset - 1);
	}
	else  // Param >= 0 means counting restarted and output transitioned high.
	{
		// Using HOLD_LINE, because FIRQ will be cleared by circuitry that
		// detects a FIRQ Acknowledge. That circuit consists of:
		// U17 (74LS32), U21 (1/4 74LS04), U16 (74LS74). It clears the
		// FIRQ line on the rising edge of Q when BS = 1, BA = 0 and A3 = 0.
		m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);

		// Schedule for the specified number of cycles. param = -1 ensure the
		// logic in `if (param < 0)` above is activated when the timer elapses.
		m_firq_timer->adjust(param * attotime::from_hz(TIMER_CLOCK), -1);

		// When FIRQ* activates, it issues a /CLR on 74LS259 (U12, display
		// board). This also clears the grid mask as a side effect. See
		// display_w() for more info.
		for (int i = 0; i < m_vfd_devices.size(); ++i)
			m_vfd_devices[i]->write_my(0);
	}
}

void xpander_state::firq_timer_preset_w(u8 data)
{
	if (m_firq_timer_preset == data)
		return;
	m_firq_timer_preset = data;
	LOGMASKED(LOG_FIRQ_TIMER, "FIRQ Timer Preset: %02x\n", data);
}

u8 xpander_state::proc_datain_r()
{
	// LS367, U15 (processor board)
	// D0 - HALTAKN*
	const u8 d0 = m_voiceboard->haltack_r() ? 0 : 1;
	// D1 - Memory write protect (low when protected)
	const u8 d1 = BIT(m_memory_protect_io->read(), 0);
	// D2 - Cassette DATA.
	const u8 d2 = 1;  // TODO: Implement.
	// D3-D5 not connected.
	return 0xf8 | (d2 << 2) | (d1 << 1) | (d0 << 0);
}

u8 xpander_state::gate_r()
{
	// All component designations refer to the Pot board.
	// D0-D5: GATE1-6.
	// D6: CHAIN ADVANCE.
	// D7: TRIGGER.
	// All signals inverted by U22 (CA3081, D1-D7) and Q1 (NPN, D0).
	// TRIGGER (D7) is inverted again by U2 (74LS02). Also connected to C17.
	// Signals other than TRIGGER not connected to such a capacitor.
	const u8 value = ~m_gate_io->read() ^ 0x80;
	LOGMASKED(LOG_CV_IN, "Gate: %02x\n", value);
	return value;
}

u8 xpander_state::switch_r(offs_t offset)
{
	// A0-A2 used as ABC inputs of a 74LS42 (U16, pot board), which in turn
	// controls which column of the switch matrix is active.
	// Input D is connected to the SWITCH* signal, and outputs 8 and 9 (active
	// when SWITCH* is high) are not connected.
	const u8 column = offset & 0x07;
	const u8 data = m_switch_io[column]->read();
	if (data != 0xff)
		LOGMASKED(LOG_SWITCHES, "Pressed: %02x - %02x\n", column, data);
	return data;
}

static u8 byte_from_array(const std::array<bool, 6> &v)
{
	u8 data = 0;
	for (int i = 0; i < 6; ++i)
		if (v[i])
			data |= 1U << i;
	return data;
}

u8 xpander_state::encoder_dir_r()
{
	// The DIR* signal also resets encoder change detection flip-flops when
	// active (low).
	std::fill(m_encoder_changed.begin(), m_encoder_changed.end(), false);

	const u8 data = byte_from_array(m_encoder_dir);
	LOGMASKED(LOG_ENCODERS, "Encoder dir_r: %02x\n", data);
	return data;
}

u8 xpander_state::encoder_sw_r()
{
	const u8 data = byte_from_array(m_encoder_changed);
	if (data != 0)
		LOGMASKED(LOG_ENCODERS, "Encoder sw_r: %02x\n", data);
	return data;
}

u8 xpander_state::cv_in_r()
{
	assert(m_selected_cv_in >= 0 && m_selected_cv_in < 8);
	if (m_inhibit_cv_in)
		return 0;

	u8 cv = 0;
	if (m_selected_cv_in < 6)
		cv = m_cv_io[m_selected_cv_in]->read();
	else
		cv = m_pedal_io[m_selected_cv_in - 6]->read();

	LOGMASKED(LOG_CV_IN, "CV in: %02x - %02x\n", m_selected_cv_in, cv);
	return cv;
}

u8 xpander_state::adc_r(offs_t offset)
{
	// CV* signal mapped to:

	// a) U18 latch on pot board, controls U17 (4051) mux (
	//    A0-A2 -> A-C, A3 -> INH).
	if (!machine().side_effects_disabled())
	{
		m_selected_cv_in = offset & 0x07;
		m_inhibit_cv_in = offset & 0x08;
	}

	// b) U20 on Pot Board (ADC0804).
	const u8 data = m_adc->read();
	LOGMASKED(LOG_CV_IN, "ADC Read: %02x - %02x\n", offset, data);
	return data;
}

void xpander_state::adc_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_CV_IN, "ADC Write: %02x - %02x\n", offset, data);
	// CV* signal mapped to:
	// a) U18 latch on pot board, controls U17 (4051) mux (A0-A2 -> A-C, A3 -> INH).
	m_selected_cv_in = offset & 0x07;
	m_inhibit_cv_in = offset & 0x08;
	// b) U20 on Pot Board (ADC0804).
	m_adc->write(data);
}

void xpander_state::haltack_changed(int state)
{
	m_rom_0_view.select(state);
}

void xpander_state::haltset_w(u8 data)
{
	// Bits 0-3: HALTREQ* 0-3. Active low.
	// The voice board is wired (with a rotary switch) to HALTREQ0*. HALTREQ(1-3)*
	// are not used in the xpander. They are provisioned for extending to more
	// voice boards. For example, the Oberheim Matrix 12 uses two of those.
	m_voiceboard->haltreq_w(!BIT(data, 0));
	if (BIT(~data, 1, 3))
		LOGMASKED(LOG_CPU_COMMS, "Unexpected HALTREQ asserted: %x\n", BIT(~data, 1, 3));

	// Bit 4: RES* (voice cpu reset), active low.
	m_voiceboard->reset_w(!BIT(data, 4));

	// Bit 5: CASSMUTE*, active low.
	m_cassmute = !BIT(data, 5);

	// Bit 6 not connected.

	// Bit 7: CASS OUT.
	m_cassout = BIT(data, 7);
}

void xpander_state::display_w(offs_t offset, u8 data)
{
	// There are 3 vacuum fluorescent displays (VFDs). These are controlled in
	// a similar way to multi-segment LED displays, and can be time-multiplexed
	// in the same way, though they run at a higher voltage (55V in this case).

	// Each of the 3 VFDs has 40 16-segment characters. The segments in each
	// display share a common anode, for a total of 3 x 16 = 48 anode signals.
	// There is a gate signal for each of the 40 characters, and those are
	// shared between the 3 displays, for a total of 40 gate signals.

	// All component designations refer to the display board.

	// U12 (74LS239) selects which anode signal latch will be enabled, based on
	// A0-A2. There are 6 latches (74LS374), 2 per display.
	const u8 display = (offset >> 1) & 0x03;
	if (display < 3)  // U12 outputs 6 and 7 are not connected.
	{
		if (offset & 0x01)  // Modifying high-order byte.
			m_vfd_anode_masks[display] = (u16(data) << 8) | (m_vfd_anode_masks[display] & 0x00ff);
		else
			m_vfd_anode_masks[display] = (m_vfd_anode_masks[display] & 0xff00) | data;
		m_vfd_devices[display]->write_mx(m_vfd_anode_masks[display]);
	}

	// An 74LS42 (U9), combined with 5 x 4028 (U2, U6, U8, U14, U18) form a
	// decoder that translates the latched A3-A8 to a single selected grid.
	// However, decoding is only enabled when U12 output 5 is high.
	const bool grid_enabled = (offset & 0x07) == 5;
	const u8 grid_offset = (offset >> 3) & 0x3f;
	u64 grid_mask = 0;
	if (grid_enabled && grid_offset < 40)  // There are 40 grid signals.
		grid_mask = u64(1) << grid_offset;

	// Refresh VFD devices with the latest grid mask.
	for (int i = 0; i < m_vfd_devices.size(); ++i)
		m_vfd_devices[i]->write_my(grid_mask);
}

void xpander_state::display_output_w(int display, offs_t offset, u32 data)
{
	// The FG405A2 is a non-standard 16-segment display. It includes a
	// 14-segment character, a period, and a line under the character.
	// Map the 16 segments of the FG405A2 to a `led14segsc`. The line under the
	// character will be represented with the comma.
	assert(display >= 0 && display < 3);
	m_vfd_outputs[display][offset] =
		bitswap<16>(data, 0, 1, 3, 6, 5, 4, 2, 7, 12, 11, 10, 13, 15, 14, 9, 8);
}

void xpander_state::maincpu_map(address_map &map)
{
	// Component designations refer to the Processor board, unless otherwise
	// noted. The signal names below (e.g. DISP*, HALTSET*) reference those in
	// the schematics.

	map.unmap_value_high();  // Data bus pulled high by resistors in Pot board.

	// 1/2 74LS139 (U22, O0-O2) controls access to RAM and other ports.
	// RAM write and select signals can only go active low when there is power
	// (as determined by the PUP circuit).
	// NVRAM_A can be write-protected in hardware by a memory-protect switch
	// (SW6, active-closed, disables /WR signal).
	map(0x000, 0x3fff).view(m_nvram_a_view);
	m_nvram_a_view[0](0x000, 0x3fff).ram().share("nvram_a");
	m_nvram_a_view[1](0x000, 0x3fff).readonly().share("nvram_a");
	// NVRAM_B is not protected by SW6.
	map(0x4000, 0x5fff).ram().share("nvram_b");  // 1 x 6264 (U4).

	// The 74LS139 above (O3) along with an 74LS42 (U23) control access to all
	// other ports / peripherals.
	// Active when O3 is low and one of READ*, WRITE*, UART* is low.
	map(0x6000, 0x61ff).mirror(0x0200).w(FUNC(xpander_state::display_w)); // DISP* and DISPLCLR*
	map(0x6400, 0x6400).mirror(0x03ff).w(FUNC(xpander_state::firq_timer_preset_w));  // INTSET*
	map(0x6800, 0x6800).mirror(0x03ff).w(FUNC(xpander_state::haltset_w));  // HALTSET*
	// ACIA addressing: RS <- A0, CS1 <- A1, CS0 <- A2, /CS2 <- UART*.
	map(0x6c06, 0x6c07).mirror(0x03f8).rw("midiacia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));  // UART*
	map(0x7000, 0x7000).mirror(0x03ff).r(FUNC(xpander_state::proc_datain_r));  // DATAIN*
	map(0x7400, 0x7400).mirror(0x03ff).w("latch_u24_proc", FUNC(output_latch_device::write));  // LED2*
	map(0x7800, 0x7800).mirror(0x03ff).unmaprw();  // Unused. U23 output 6 not connected.

	// Peripherals on Pot board mapped to 0x7c00-0x7fff (BEN* output of U23).
	// Additional decoding for those is done by U15 on the Pot board (74LS42).
	// A9 and A0-A5 are not used for decoding (mirror: 0x023f)
	map(0x7c00, 0x7c00).mirror(0x023f).r(FUNC(xpander_state::encoder_dir_r));  // DIR*
	map(0x7c40, 0x7c40).mirror(0x023f).r(FUNC(xpander_state::encoder_sw_r));  // SW*
	map(0x7c80, 0x7c80).mirror(0x023f).w("latch_u14_pot", FUNC(output_latch_device::write));  // LED1*
	map(0x7cc0, 0x7cc0).mirror(0x023f).r(FUNC(xpander_state::gate_r));  // GATE*
	map(0x7d00, 0x7d00).mirror(0x023f).w("latch_u21_pot", FUNC(output_latch_device::write));  // PULL*
	map(0x7d40, 0x7d47).mirror(0x0238).r(FUNC(xpander_state::switch_r));  // SWITCH*
	map(0x7d80, 0x7d8f).mirror(0x0230).rw(FUNC(xpander_state::adc_r), FUNC(xpander_state::adc_w));  // CV*
	map(0x7dc0, 0x7dc0).mirror(0x023f).unmaprw();  // Unused. U15 (Pot board) output 7 not connected.

	// ROM and VOICERAM decoding is done by 1/2 LS139 (U22), 2/4 LS04 (U21),
	// 2/4 LS32 (U20).
	// (0x8000, 0x9fff) conditionally accesses VOICERAM, when HALTAKN* active.
	map(0x8000, 0x9fff).view(m_rom_0_view);
	m_rom_0_view[0](0x8000, 0x9fff).rom().region("maincpu", 0x0000);  // 1 x 2764 (U8).
	m_rom_0_view[1](0x8000, 0x9fff).ram().share("voiceboard:voiceram");
	map(0xa000, 0xffff).rom().region("maincpu", 0x2000);  // 3 x 2764 (U7 - U5).
}

void xpander_state::machine_start()
{
	firq_timer_elapsed(*m_firq_timer, m_firq_timer_preset);  // Reset the timer.

	save_item(NAME(m_firq_timer_preset));
	save_item(NAME(m_selected_cv_in));
	save_item(NAME(m_inhibit_cv_in));
	save_item(NAME(m_encoder_dir));
	save_item(NAME(m_encoder_changed));
	save_item(NAME(m_vfd_anode_masks));
}

void xpander_state::xpander(machine_config &config)
{
	MC6809(config, m_maincpu, 16_MHz_XTAL / 2);  // 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &xpander_state::maincpu_map);

	TIMER(config, m_firq_timer).configure_generic(FUNC(xpander_state::firq_timer_elapsed));

	NVRAM(config, "nvram_a", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "nvram_b", nvram_device::DEFAULT_ALL_0);

	auto &midiacia = ACIA6850(config, "midiacia");
	midiacia.txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	midiacia.irq_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	clock_device &acia_clock = CLOCK(config, "aciaclock", 16_MHz_XTAL / 32);  // 500 KHz.
	acia_clock.signal_handler().set("midiacia", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("midiacia", FUNC(acia6850_device::write_rxc));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");
	midi_port_device &midi_in = MIDI_PORT(config, "mdin", midiin_slot, "midiin");
	midi_in.rxd_handler().set("midiacia", FUNC(acia6850_device::write_rxd));
	midi_in.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	ADC0804(config, m_adc, 16_MHz_XTAL / 32);  // 500 KHz.
	m_adc->vin_callback().set(FUNC(xpander_state::cv_in_r));

	for (int i = 0; i < m_vfd_devices.size(); ++i)
	{
		PWM_DISPLAY(config, m_vfd_devices[i]).set_size(40, 16);  // 40 x 16-segment display.
		m_vfd_devices[i]->set_segmask(0xffffffffff, 0xffff);
		m_vfd_devices[i]->output_digit().set([this, i] (offs_t offset, u32 data) { display_output_w(i, offset, data); });
	}

	config.set_default_layout(layout_oberheim_xpander);

	XPANDER_VOICEBOARD(config, m_voiceboard);
	m_voiceboard->haltack_cb().set(FUNC(xpander_state::haltack_changed));

	// LED2*, U24 on Processor board. All outputs active low (connected to LED
	// cathodes).
	auto &u24 = OUTPUT_LATCH(config, "latch_u24_proc");
	u24.bit_handler<0>().set_output("led_misc").invert();
	u24.bit_handler<1>().set_output("led_ramp_x").invert();
	u24.bit_handler<2>().set_output("led_lfo_x").invert();
	u24.bit_handler<3>().set_output("led_env_x").invert();
	u24.bit_handler<4>().set_output("led_vcf_vca").invert();
	u24.bit_handler<5>().set_output("led_vco1").invert();  // On Pot board.
	u24.bit_handler<6>().set_output("led_vco2").invert();  // On Pot board.
	u24.bit_handler<7>().set_output("led_fm_lag").invert();  // On Pot board.

	// LED1*, U14 on Pot board. All outputs active low (connected to LED
	// cathodes).
	auto &u14 = OUTPUT_LATCH(config, "latch_u14_pot");
	u14.bit_handler<0>().set_output("led_x_sel").invert();
	u14.bit_handler<1>().set_output("led_mod_sorc").invert();
	u14.bit_handler<2>().set_output("led_mod").invert();
	u14.bit_handler<3>().set_output("led_on_off_a").invert();
	u14.bit_handler<4>().set_output("led_track").invert();
	u14.bit_handler<5>().set_output("led_page_2").invert();
	u14.bit_handler<6>().set_output("led_on_off_b").invert();
	u14.bit_handler<7>().set_output("led_value").invert();

	// PULL*, U21 on Pot board.
	// Configures the resting value for the gate inputs. Used to support both
	// active low and active high inputs.
	auto &u21 = OUTPUT_LATCH(config, "latch_u21_pot");
	u21.bit_handler<0>().set_output("pull_1");  // Default for GATE 1
	u21.bit_handler<1>().set_output("pull_2");  // Default for GATE 2
	u21.bit_handler<2>().set_output("pull_3");  // Default for GATE 3
	u21.bit_handler<3>().set_output("pull_4");  // Default for GATE 4
	u21.bit_handler<4>().set_output("pull_5");  // Default for GATE 5
	u21.bit_handler<5>().set_output("pull_6");  // Default for GATE 6
	u21.bit_handler<6>().set_output("pull_7");  // Default for TRIGGER
	// Bit 7 not connected.
}

DECLARE_INPUT_CHANGED_MEMBER(xpander_state::encoder_moved)
{
	const int encoder = param;
	if (oldval != newval)
		m_encoder_changed[encoder] = true;

	constexpr int WRAP_BUFFER = 3;
	const bool overflowed = newval <= WRAP_BUFFER &&
			oldval >= NUM_ENCODER_POSITIONS - WRAP_BUFFER;
	const bool underflowed = newval >= NUM_ENCODER_POSITIONS - WRAP_BUFFER &&
			oldval <= WRAP_BUFFER;
	m_encoder_dir[encoder] = ((newval > oldval) || overflowed) && !underflowed;

	LOGMASKED(LOG_ENCODERS, "Encoder %d changed from: %d to: %d (o: %d, u: %d), dir: %d\n",
			encoder, oldval, newval, overflowed, underflowed, m_encoder_dir[encoder]);
}

DECLARE_INPUT_CHANGED_MEMBER(xpander_state::memory_protect_changed)
{
	const bool memory_protected = !BIT(m_memory_protect_io->read(), 0);
	LOGMASKED(LOG_MEM_PROTECT, "NVRAM A protected: %d\n", memory_protected);
	if (memory_protected)
		m_nvram_a_view.select(1);
	else
		m_nvram_a_view.select(0);
}

INPUT_PORTS_START(xpander)
	PORT_START("switches_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_1)

	PORT_START("switches_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("8")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("9")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("+")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("-")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("STORE")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PAGE2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TUNE") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MASTER")

	PORT_START("switches_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SINGLE")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MULTI")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VOICE 1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VOICE 2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VOICE 3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VOICE 4") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VOICE 5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VOICE 6")

	PORT_START("switches_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1 1") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1 5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1 6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VCO1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VCO2")

	PORT_START("switches_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2 1") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2 5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2 6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("FM/LAG")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TRACK X")

	PORT_START("switches_5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LEVER 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LEVER 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PEDAL 1") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PEDAL 2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VIB")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("KEYBOARD")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LAG")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VELOCITY")

	PORT_START("switches_6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RELEASE VELOCITY")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PRESSURE") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ENV")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LFO")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TRACK")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RAMP")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)  // No switch. Pulled up.
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)  // No switch. Pulled up.

	PORT_START("switches_7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VCF/VCA") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ENV X")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LFO X")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RAMP X")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MISC.")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)  // No switch. Pulled up.
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)  // No switch. Pulled up.
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)  // No switch. Pulled up.

	PORT_START("memory_protect")  // SW6 on Processor board.
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Memory Protect") PORT_TOGGLE PORT_CODE(KEYCODE_P)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xpander_state::memory_protect_changed), 0)

	// These inputs (except "chain advance") can be configured by the user as
	// active low or active high. This configuration is manifested in the "PULL"
	// signals (search for "latch_u21_pot"). Modeling as active low for now.
	PORT_START("gate_inputs")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("GATE 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("GATE 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("GATE 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("GATE 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("GATE 5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("GATE 6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CHAIN ADVANCE")  // No corresponding 'PULL' signal.
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TRIGGER")

	// Voltage inputs.
	PORT_START("cv_in_1")
	PORT_BIT(0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(30)
	PORT_START("cv_in_2")
	PORT_BIT(0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(30)
	PORT_START("cv_in_3")
	PORT_BIT(0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(30)
	PORT_START("cv_in_4")
	PORT_BIT(0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(30)
	PORT_START("cv_in_5")
	PORT_BIT(0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(30)
	PORT_START("cv_in_6")
	PORT_BIT(0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(30)
	PORT_START("pedal_1")
	PORT_BIT(0xff, 0x00, IPT_PEDAL1) PORT_SENSITIVITY(30)
	PORT_START("pedal_2")
	PORT_BIT(0xff, 0x00, IPT_PEDAL2) PORT_SENSITIVITY(30)

	// Rotary encoders.
	PORT_START("encoder_0")
	PORT_BIT(0x1f, 0x00, IPT_POSITIONAL) PORT_POSITIONS(NUM_ENCODER_POSITIONS)
		PORT_WRAPS PORT_SENSITIVITY(20) PORT_KEYDELTA(1)
		PORT_CODE_DEC(KEYCODE_Q) PORT_CODE_INC(KEYCODE_W)
		PORT_FULL_TURN_COUNT(NUM_ENCODER_POSITIONS)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xpander_state::encoder_moved), 0)
	PORT_START("encoder_1")
	PORT_BIT(0x1f, 0x00, IPT_POSITIONAL) PORT_POSITIONS(NUM_ENCODER_POSITIONS)
		PORT_WRAPS PORT_SENSITIVITY(20) PORT_KEYDELTA(1)
		PORT_CODE_DEC(KEYCODE_E) PORT_CODE_INC(KEYCODE_R)
		PORT_FULL_TURN_COUNT(NUM_ENCODER_POSITIONS)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xpander_state::encoder_moved), 1)
	PORT_START("encoder_2")
	PORT_BIT(0x1f, 0x00, IPT_POSITIONAL) PORT_POSITIONS(NUM_ENCODER_POSITIONS)
		PORT_WRAPS PORT_SENSITIVITY(20) PORT_KEYDELTA(1)
		PORT_CODE_DEC(KEYCODE_A) PORT_CODE_INC(KEYCODE_S)
		PORT_FULL_TURN_COUNT(NUM_ENCODER_POSITIONS)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xpander_state::encoder_moved), 2)
	PORT_START("encoder_3")
	PORT_BIT(0x1f, 0x00, IPT_POSITIONAL) PORT_POSITIONS(NUM_ENCODER_POSITIONS)
		PORT_WRAPS PORT_SENSITIVITY(20) PORT_KEYDELTA(1)
		PORT_CODE_DEC(KEYCODE_D) PORT_CODE_INC(KEYCODE_F)
		PORT_FULL_TURN_COUNT(NUM_ENCODER_POSITIONS)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xpander_state::encoder_moved), 3)
	PORT_START("encoder_4")
	PORT_BIT(0x1f, 0x00, IPT_POSITIONAL) PORT_POSITIONS(NUM_ENCODER_POSITIONS)
		PORT_WRAPS PORT_SENSITIVITY(20) PORT_KEYDELTA(1)
		PORT_CODE_DEC(KEYCODE_G) PORT_CODE_INC(KEYCODE_H)
		PORT_FULL_TURN_COUNT(NUM_ENCODER_POSITIONS)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xpander_state::encoder_moved), 4)
	PORT_START("encoder_5")
	PORT_BIT(0x1f, 0x00, IPT_POSITIONAL) PORT_POSITIONS(NUM_ENCODER_POSITIONS)
		PORT_WRAPS PORT_SENSITIVITY(20) PORT_KEYDELTA(1)
		PORT_CODE_DEC(KEYCODE_J) PORT_CODE_INC(KEYCODE_K)
		PORT_FULL_TURN_COUNT(NUM_ENCODER_POSITIONS)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xpander_state::encoder_moved), 5)
INPUT_PORTS_END

ROM_START(xpander)
	ROM_REGION(0x8000, "maincpu", 0)  // 4 x 2764 8Kx8bit ROMs (U8 - U5, processor board).
	ROM_DEFAULT_BIOS("1.2")

	ROM_SYSTEM_BIOS(0, "1.2", "Xpander Main Processor Revision 1.2")
	ROMX_LOAD("exp1.2-0.u8", 0x000000, 0x002000, CRC(dc3801b1) SHA1(4064edc2fa0bea62684c28e8e004feb8229604fe), ROM_BIOS(0))
	ROMX_LOAD("exp1.2-1.u7", 0x002000, 0x002000, CRC(b57f8482) SHA1(233efc3da3a96777b10a4d6fe7e80864e9231f04), ROM_BIOS(0))
	ROMX_LOAD("exp1.2-2.u6", 0x004000, 0x002000, CRC(30f28710) SHA1(798c1b28fb59819baadbf0142ecd1348e759fa70), ROM_BIOS(0))
	ROMX_LOAD("exp1.2-3.u5", 0x006000, 0x002000, CRC(0bc8335e) SHA1(5897ad0cd66cf37b8e9b277654aab4eb83d1c8ab), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "1.0", "Xpander Main Processor Revision 1.0")
	ROMX_LOAD("exp1.0-0.u8", 0x000000, 0x002000, CRC(d93fb34c) SHA1(e470589562f6544507c276ccfc95632a6288eb18), ROM_BIOS(1))
	ROMX_LOAD("exp1.0-1.u7", 0x002000, 0x002000, CRC(80912a48) SHA1(064e9fff26a4ebf8902c3e085fa631bb5579a160), ROM_BIOS(1))
	ROMX_LOAD("exp1.0-2.u6", 0x004000, 0x002000, CRC(89e14d6e) SHA1(7aef67db9d78523777af6f1edf04fd6b0d11229b), ROM_BIOS(1))
	ROMX_LOAD("exp1.0-3.u5", 0x006000, 0x002000, CRC(21794b92) SHA1(e544375c3fc29931497cb6429db7a2812f77c553), ROM_BIOS(1))

	ROM_REGION(0x6000, "voicecpu", 0)  // 3 x 2764 8Kx8bit ROMs. Rev 1.4.
	ROM_FILL(0x000000, 0x002000, 0xff)  // U909. Spare 2764 ROM slot. Not populated. Data bus pulled high.
	ROM_LOAD("ca1.4-0.u912", 0x002000, 0x002000, CRC(79f4490a) SHA1(5ab13ccc3b75df313a447520ca54a492102b7e3b))
	ROM_LOAD("ca1.4-1.u914", 0x004000, 0x002000, CRC(9b7a7914) SHA1(951bbc197a0140a93bb7621a7aedbe0f4037990c))
ROM_END

}  // anonymous namespace

// In production from 1984 to 1988.
SYST(1984, xpander, 0, 0, xpander, xpander, xpander_state, empty_init, "Oberheim", "Xpander", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
