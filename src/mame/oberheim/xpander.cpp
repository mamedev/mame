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
See xpander_vb.cpp for more info. The main computer communicates with the voice
board by accessing its RAM after halting its CPU.

The Matrix-12 is basically an Xpander with a second voice board, a keyboard
and two pitch/mod levers. It also got rid of the GATE and CV inputs, and the
individual voice outputs. Its main processor board differs somewhat: it has more
ROM and RAM, and additional circuitry for scanning the keyboard and levers. Its
peripherals are otherwise mostly the same, but the memory map is different.
The rest of the boards are identical or nearly identical. For details, compare
xpander() with matrix12(), and the two maincpu_map() functions with each other.

PCBoards:
- Processor board: main computer.
- Pot board: buttons, rotary encoders, inputs, outputs.
- Display board: control of VFDS.
- Voice board: 6 analog voices, voice computer (x2 for the Matrix-12).
- Power supply.

This driver is based on the Xpander's service manual and schematics, and is
intended as an educational tool. There is no attempt to emulate audio.

A note on variable "polarity": many bool variables represent signals in the
schematic (e.g. HALTREQ). Some of those signals are active low (e.g. HALTREQ*).
But the variables and functions here (e.g. haltreq_r()) are always
active-high (active == true).
*/

#include "emu.h"

#include "matrixsynth_kbd.h"
#include "xpander_vb.h"

#include "bus/midi/midi.h"
#include "cpu/m6809/m6809.h"
#include "machine/6850acia.h"
#include "machine/7474.h"
#include "machine/adc0804.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/output_latch.h"
#include "machine/pit8253.h"
#include "machine/quadmouse.h"
#include "machine/rescap.h"
#include "machine/timer.h"
#include "video/pwm.h"
#include "corefloat.h"

#include <algorithm>
#include <array>

#include "oberheim_matrix12.lh"
#include "oberheim_xpander.lh"

#define LOG_CV_IN       (1U << 1)
#define LOG_SWITCHES    (1U << 2)
#define LOG_ENCODERS    (1U << 3)
#define LOG_FIRQ_TIMER  (1U << 4)
#define LOG_BANKING     (1U << 5)
#define LOG_GATE_IN     (1U << 6)
#define LOG_LEVER       (1U << 7)

#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


namespace {

constexpr double VPLUS = 12;
constexpr double VMINUS = -12;

// Maps the port's current value to the specified range.
// It is OK for `new_end` to be smaller than `new_start`.
double map_port_range(const required_ioport &port, double new_start, double new_end)
{
	return fmaprange<double>(
		port->read(), port->field(1)->minval(), port->field(1)->maxval(), new_start, new_end);
}

// Converts a 0-5V double to the byte produced by the ADC.
u8 adc_byte(double v)
{
	const double clamped_v = std::clamp(v, 0.0, 5.0);  // ADC input has clamping diodes.
	const double mapped = std::round(fmaprange(clamped_v, 0.0, 5.0, 0.0, 255.0));
	return u8(std::clamp(mapped, 0.0, 255.0));
}


// Hardware and functionality that is common to the Xpander and Matrix-12.
class xpander_state_base : public driver_device
{
public:
	static constexpr feature_type unemulated_features() { return feature::TAPE; }

	xpander_state_base(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD;

	void xpander_common(machine_config &config) ATTR_COLD;

	virtual DECLARE_INPUT_CHANGED_MEMBER(memory_protect_changed) { }

protected:
	void machine_start() override ATTR_COLD;

	u8 gate_r();
	u8 switch_r(offs_t offset);
	u8 encoder_dir_r();
	u8 encoder_sw_r();
	bool memory_protect_r() const;

	u8 adc_r(offs_t offset);
	void adc_w(offs_t offset, u8 data);

	void display_w(offs_t offset, u8 data);
	void display_clear();

	u8 pedal_cv(int pedal) const;
	bool pedal_connected(int pedal) const;

	u8 selected_cv_in() const { return m_selected_cv_in; }
	bool inhibit_cv_in() const { return m_inhibit_cv_in; }
	mc6809_device *maincpu() { return m_maincpu.target(); }

private:
	void encoder_moved(int encoder);
	void display_output_w(int display, offs_t offset, u32 data);

	virtual std::pair<double, double> pedal_pulldown(int pedal) const = 0;
	virtual u8 gate_inputs() const = 0;
	virtual u8 cv_in_r() = 0;

	virtual void maincpu_map(address_map &map) ATTR_COLD = 0;

	required_device<mc6809_device> m_maincpu;
	required_device<adc0804_device> m_adc;
	required_device_array<quadencoder_device, 6> m_encoder;
	required_device_array<ttl7474_device, 6> m_encoder_dir_ff;
	required_device_array<ttl7474_device, 6> m_encoder_changed_ff;
	required_ioport_array<8> m_switch_io;
	required_ioport m_memory_protect_io;
	required_ioport_array<2> m_pedal_io;
	required_ioport_array<2> m_pedal_type;
	required_device_array<pwm_display_device, 3> m_vfd_devices;
	output_finder<3, 40> m_vfd_chars;
	output_finder<3, 40> m_vfd_lines;

	u8 m_selected_cv_in;  // MUX A-C inputs.
	bool m_inhibit_cv_in;  // MUX INHibit input.
	std::array<u64, 3> m_vfd_anode_masks;
};

xpander_state_base::xpander_state_base(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_maincpu(*this, "maincpu")
	, m_adc(*this, "adc")
	, m_encoder(*this, "encoder_%u", 1U)
	, m_encoder_dir_ff(*this, "encoder_dir_flipflop_%u", 1U)
	, m_encoder_changed_ff(*this, "encoder_changed_flipflop_%u", 1U)
	, m_switch_io(*this, "switches_%u", 0U)
	, m_memory_protect_io(*this, "memory_protect")
	, m_pedal_io(*this, "pedal_%u", 1U)
	, m_pedal_type(*this, "pedal_%u_type", 1U)
	, m_vfd_devices(*this, "vfd_%u", 1U)
	, m_vfd_chars(*this, "vfd_%u_char_%u", 1U, 1U)
	, m_vfd_lines(*this, "vfd_%u_line_%u", 1U, 1U)
	, m_selected_cv_in(0x07)  // Pulled high.
	, m_inhibit_cv_in(true)  // Pulled high.
	, m_vfd_anode_masks({0, 0, 0})
{
}

u8 xpander_state_base::gate_r()  // U23 (74LS244, pot board)
{
	const u8 input = gate_inputs();
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_GATE_IN, "Gate: %02x\n", input);

	// All signals inverted by U22 (CA3081, D1-D7) and Q1 (NPN, D0).
	// TRIGGER (D7) is smoothed by C17 and inverted again by U2 (74LS02).
	return ~input ^ 0x80;
}

u8 xpander_state_base::switch_r(offs_t offset)  // U16 (74LS42, pot board)
{
	// A0-A2 used as ABC inputs of a 74LS42 (U16, pot board), which in turn
	// controls which column of the switch matrix is active.
	// Input D is connected to the SWITCH* signal, and outputs 8 and 9 (active
	// when SWITCH* is high) are not connected.
	const u8 column = offset & 0x07;
	const u8 data = m_switch_io[column]->read();
	if (data != 0xff && !machine().side_effects_disabled())
		LOGMASKED(LOG_SWITCHES, "Pressed: %02x - %02x\n", column, data);
	return data;
}

void xpander_state_base::encoder_moved(int encoder)
{
	const bool sw1 = m_encoder[encoder]->mn_r();
	const bool sw2 = m_encoder[encoder]->pl_r();

	m_encoder_dir_ff[encoder]->d_w(sw1 ? 1 : 0);
	m_encoder_dir_ff[encoder]->clock_w(sw2 ? 1 : 0);
	m_encoder_changed_ff[encoder]->clock_w((sw1 || sw2) ? 0 : 1);  // NOR(sw1, sw2), 74LS02.

	LOGMASKED(LOG_ENCODERS, "Encoder %d moved: %d %d\n", encoder, sw1, sw2);
}

static u8 byte_from_flipflops(const required_device_array<ttl7474_device, 6> &ff)
{
	u8 data = 0;
	for (int i = 0; i < ff.size(); ++i)
		data |= ff[i]->output_r() << i;
	return data;
}

u8 xpander_state_base::encoder_dir_r()  // U5 (74LS367, pot board)
{
	const u8 data = byte_from_flipflops(m_encoder_dir_ff);

	if (!machine().side_effects_disabled())
	{
		// The DIR* signal also resets the encoder change detection flipflops.
		for (int i = 0; i < m_encoder_changed_ff.size(); ++i)
		{
			m_encoder_changed_ff[i]->clear_w(0);
			m_encoder_changed_ff[i]->clear_w(1);
		}

		LOGMASKED(LOG_ENCODERS, "Encoder dir_r: %02x\n", data);
	}

	return data;
}

u8 xpander_state_base::encoder_sw_r()  // U9 (74LS367, pot board)
{
	const u8 data = byte_from_flipflops(m_encoder_changed_ff);
	if (data != 0 && !machine().side_effects_disabled())
		LOGMASKED(LOG_ENCODERS, "Encoder sw_r: %02x\n", data);
	return data;
}

bool xpander_state_base::memory_protect_r() const
{
	// The memory protect DPDT switch grounds two signals when enabled:
	// One is read by the firmware, and the other disables (via logic gates) the
	// /WR signal for a subset of RAM chips.
	return !BIT(m_memory_protect_io->read(), 0);
}

u8 xpander_state_base::adc_r(offs_t offset)
{
	// CV* signal mapped to:

	// a) U20 on Pot Board (ADC0804).
	const u8 data = m_adc->read();

	// b) U18 latch on pot board, controls U17 (4051) mux (
	//    A0-A2 -> A-C, A3 -> INH).
	if (!machine().side_effects_disabled())
	{
		m_selected_cv_in = offset & 0x07;
		m_inhibit_cv_in = offset & 0x08;

		LOGMASKED(LOG_CV_IN, "ADC Read: %02x - %02x\n", offset, data);
	}

	return data;
}

void xpander_state_base::adc_w(offs_t offset, u8 data)
{
	// CV* signal mapped to:
	// a) U18 latch on pot board, controls U17 (4051) mux (A0-A2 -> A-C, A3 -> INH).
	m_selected_cv_in = offset & 0x07;
	m_inhibit_cv_in = offset & 0x08;
	// b) U20 on Pot Board (ADC0804).
	m_adc->write(data);
	LOGMASKED(LOG_CV_IN, "ADC Write: %02x - %02x\n", offset, data);
}

void xpander_state_base::display_w(offs_t offset, u8 data)
{
	// There are 3 vacuum fluorescent displays (VFDs). These are controlled in
	// a similar way to multi-segment LED displays, and can be time-multiplexed
	// in the same way, though they run at a higher voltage (55V in this case).

	// Each of the 3 VFDs has 40 16-segment characters. The segments in each
	// display share a common anode, for a total of 3 x 16 = 48 anode signals.
	// There is a gate signal for each of the 40 characters, and those are
	// shared between the 3 displays, for a total of 40 gate signals.

	// All components are located on the display board.

	// Writing to the display will first assert the DISPLCR* signal, followed
	// by the DISP* signal after a logic gate propagation delay.

	// Handle the DISPLCR* signal.
	display_clear();

	// Handle the DISP* signal.

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

void xpander_state_base::display_clear()
{
	// A /CLR on 74LS259 (U12, display board) will prepare the VFD anode latches
	// for the next update (lowers their clock input, no effect on emulation).
	// This will also clear the grid mask as a side effect.
	for (int i = 0; i < m_vfd_devices.size(); ++i)
		m_vfd_devices[i]->write_my(0);
}

void xpander_state_base::display_output_w(int display, offs_t offset, u32 data)
{
	// The FG405A2 is a non-standard 16-segment display. It includes a
	// 14-segment character, a period, and a line under the character.
	// Map the 16 segments of the FG405A2 to a `led14seg` and a line.
	m_vfd_chars[display][offset] = bitswap<15>(data, 1, 3, 6, 5, 4, 2, 7, 12, 11, 10, 13, 15, 14, 9, 8);
	m_vfd_lines[display][offset] = BIT(data, 0);
}

// The Xpander and Matrix-12 support multiple types of pedals.
enum pedal_type
{
	PEDAL_TYPE_NC = 0,            // Not connected.

	// Continuous (aka analog) pedals:
	PEDAL_TYPE_CONT_ACTIVE_NEG,   // Voltage source, negative polarity (0V is max).
	PEDAL_TYPE_CONT_ACTIVE_POS,   // Voltage source, positive polarity (+5V is max).
	PEDAL_TYPE_CONT_PASSIVE_NEG,  // 47KOhm potentiometer, negative polarity (pressing decreases resistance).
	PEDAL_TYPE_CONT_PASSIVE_POS,  // 47KOhm Potentiometer, positive polarity (pressing increase resistance).

	// Switch pedals:
	PEDAL_TYPE_SW_NEG,            // Powered or unpowered switch, active low. 0/5V if powered.
	PEDAL_TYPE_SW_ACTIVE_POS,     // Powered switch, active high, 0/5V.
};

u8 xpander_state_base::pedal_cv(int pedal) const
{
	// According to the owner's manual, active pedals should output voltages
	// between 0 and 5V.
	constexpr double VMAX_PEDAL = 5;
	// According to the owner's manual, passive (resistive) pedals should be
	// 47 KOhm linear potentiometers.
	constexpr double R_PEDAL = RES_K(47);
	constexpr double R_VPLUS = RES_K(62);  // R78, R77

	const u8 pedal_type = m_pedal_type[pedal]->read();
	const double value = map_port_range(m_pedal_io[pedal], 0, 1);

	double cv = 0;
	switch (pedal_type)
	{
		case PEDAL_TYPE_NC:
		{
			const auto [v_pulldown, r_pulldown] = pedal_pulldown(pedal);
			cv = v_pulldown + (VPLUS - v_pulldown) * RES_VOLTAGE_DIVIDER(R_VPLUS, r_pulldown);
			break;
		}
		case PEDAL_TYPE_CONT_ACTIVE_NEG:
		{
			cv = VMAX_PEDAL * (1.0 - value);
			break;
		}
		case PEDAL_TYPE_CONT_ACTIVE_POS:
		{
			cv = VMAX_PEDAL * value;
			break;
		}
		case PEDAL_TYPE_CONT_PASSIVE_NEG:
		{
			cv = VPLUS * RES_VOLTAGE_DIVIDER(R_VPLUS, R_PEDAL * (1.0 - value));
			break;
		}
		case PEDAL_TYPE_CONT_PASSIVE_POS:
		{
			cv = VPLUS * RES_VOLTAGE_DIVIDER(R_VPLUS, R_PEDAL * value);
			break;
		}
		case PEDAL_TYPE_SW_NEG:
		{
			cv = (value >= 0.5) ? 0.0 : VMAX_PEDAL;
			break;
		}
		case PEDAL_TYPE_SW_ACTIVE_POS:
		{
			cv = (value >= 0.5) ? VMAX_PEDAL : 0.0;
			break;
		}
		default:
		{
			assert(false);
			break;
		}
	}

	const u8 adc_value = adc_byte(cv);
	LOGMASKED(LOG_CV_IN, "Pedal %d, type: %d, input: %f, V: %f, ADC value: %u\n",
			  pedal, pedal_type, value, cv, adc_value);
	return adc_value;
}

bool xpander_state_base::pedal_connected(int pedal) const
{
	return m_pedal_type[pedal]->read() != PEDAL_TYPE_NC;
}

void xpander_state_base::machine_start()
{
	save_item(NAME(m_selected_cv_in));
	save_item(NAME(m_inhibit_cv_in));
	save_item(NAME(m_vfd_anode_masks));
}

void xpander_state_base::xpander_common(machine_config &config)
{
	// Component designations refer to the processor board unless otherwise noted.

	MC6809(config, m_maincpu, 16_MHz_XTAL / 2);  // U9 (U10 on the Matrix-12), 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &xpander_state_base::maincpu_map);

	auto &midiacia = ACIA6850(config, "midiacia");  // U26 (U30 on the Matrix-12)
	midiacia.txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	midiacia.irq_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	auto &acia_clock = CLOCK(config, "aciaclock", 16_MHz_XTAL / 32);  // 500 KHz.
	acia_clock.signal_handler().set("midiacia", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("midiacia", FUNC(acia6850_device::write_rxc));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");
	auto &midi_in = MIDI_PORT(config, "mdin", midiin_slot, "midiin");
	midi_in.rxd_handler().set("midiacia", FUNC(acia6850_device::write_rxd));
	midi_in.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	ADC0804(config, m_adc, 16_MHz_XTAL / 32);  // U20 (pot board)
	m_adc->vin_callback().set(FUNC(xpander_state_base::cv_in_r));

	// U24 (74LS374, U32 on the Matrix-12), /CLK <- LED2*
	// All outputs active low (connected to LED cathodes).
	auto &led2 = OUTPUT_LATCH(config, "latch_led2");
	led2.bit_handler<0>().set_output("led_misc").invert();
	led2.bit_handler<1>().set_output("led_ramp_x").invert();
	led2.bit_handler<2>().set_output("led_lfo_x").invert();
	led2.bit_handler<3>().set_output("led_env_x").invert();
	led2.bit_handler<4>().set_output("led_vcf_vca").invert();
	led2.bit_handler<5>().set_output("led_vco1").invert();  // On pot board.
	led2.bit_handler<6>().set_output("led_vco2").invert();  // On pot board.
	led2.bit_handler<7>().set_output("led_fm_lag").invert();  // On pot board.

	// U14 (74LS374, pot board), /CLK <- LED1*
	// All outputs active low (connected to LED cathodes).
	auto &led1 = OUTPUT_LATCH(config, "latch_led1");
	led1.bit_handler<0>().set_output("led_x_sel").invert();
	led1.bit_handler<1>().set_output("led_mod_sorc").invert();
	led1.bit_handler<2>().set_output("led_mod").invert();
	led1.bit_handler<3>().set_output("led_on_off_a").invert();
	led1.bit_handler<4>().set_output("led_track_x").invert();
	led1.bit_handler<5>().set_output("led_page_2").invert();
	led1.bit_handler<6>().set_output("led_on_off_b").invert();
	led1.bit_handler<7>().set_output("led_value").invert();

	for (int i = 0; i < m_encoder.size(); ++i)
	{
		QUADENCODER(config, m_encoder[i]);
		m_encoder[i]->write_mn().set([this, i] (int state) { encoder_moved(i); });
		m_encoder[i]->write_pl().set([this, i] (int state) { encoder_moved(i); });
		TTL7474(config, m_encoder_dir_ff[i]);
		TTL7474(config, m_encoder_changed_ff[i]).d_w(1);  // D tied to +5V.
	}

	for (int i = 0; i < m_vfd_devices.size(); ++i)
	{
		PWM_DISPLAY(config, m_vfd_devices[i]).set_size(40, 16);  // 40 x 16-segment display.
		m_vfd_devices[i]->set_segmask(0xffffffffff, 0xffff);
		m_vfd_devices[i]->output_digit().set([this, i] (offs_t offset, u32 data) { display_output_w(i, offset, data); });
	}
}


// *** Xpander ***
class xpander_state : public xpander_state_base
{
public:
	xpander_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD;

	void xpander(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(memory_protect_changed) override { update_banking(); }

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(firq_timer_elapsed);
	void firq_timer_preset_w(u8 data);

	std::pair<double, double> pedal_pulldown(int pedal) const override;
	u8 gate_inputs() const override;

	u8 cv_in_r() override;
	u8 datain_r();

	void pull_w(u8 data);

	void update_banking();

	void maincpu_map(address_map &map) override ATTR_COLD;

	required_device<xpandervb_device> m_voiceboard;
	required_shared_ptr<u8> m_voiceram;
	required_device<timer_device> m_firq_timer;
	memory_view m_ram01_view;
	memory_view m_rom0_view;
	required_ioport m_gate_io;
	required_ioport m_gate_config;
	required_ioport_array<6> m_cv_io;

	u8 m_firq_timer_preset;
	u8 m_pull;
};

xpander_state::xpander_state(const machine_config &mconfig, device_type type, const char *tag)
	: xpander_state_base(mconfig, type, tag)
	, m_voiceboard(*this, "voiceboard")
	, m_voiceram(*this, "voiceboard:voiceram")
	, m_firq_timer(*this, "firq_timer")
	, m_ram01_view(*this, "ram01_view")
	, m_rom0_view(*this, "rom0_view")
	, m_gate_io(*this, "gate_inputs")
	, m_gate_config(*this, "gate_config")
	, m_cv_io(*this, "cv_in_%u", 1U)
	, m_firq_timer_preset(0xff)  // Pulled high.
	, m_pull(0x00)
{
}

TIMER_DEVICE_CALLBACK_MEMBER(xpander_state::firq_timer_elapsed)
{
	// All components are located on the processor board.

	// CPU clock divided by U10
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
		maincpu()->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);

		// Schedule for the specified number of cycles. param = -1 ensure the
		// logic in `if (param < 0)` above is activated when the timer elapses.
		m_firq_timer->adjust(param * attotime::from_hz(TIMER_CLOCK), -1);

		// When FIRQ* activates, it asserts the DISPCLR* signal.
		display_clear();
	}
}

void xpander_state::firq_timer_preset_w(u8 data)
{
	if (m_firq_timer_preset == data)
		return;
	m_firq_timer_preset = data;
	LOGMASKED(LOG_FIRQ_TIMER, "FIRQ Timer Preset: %02x\n", data);
}

std::pair<double, double> xpander_state::pedal_pulldown(int pedal) const
{
	assert(pedal == 0 || pedal == 1);
	constexpr double R_PULL[2] = { RES_K(47), RES_K(47) };  // R75, R76
	return std::make_pair(/*GND*/0, R_PULL[pedal]);
}

u8 xpander_state::gate_inputs() const
{
	const u8 connected = (m_gate_config->read() >> 8) & 0xff;
	const u8 polarity = m_gate_config->read() & 0xff;
	const u8 active = m_gate_io->read();

	// Connected inputs need their polarity taken into account.
	// Disconnected inputs will assume the corresponding values in m_pull, which
	// is configured by the firmware.
	return (connected & (active ^ ~polarity)) | (~connected & m_pull);
}

u8 xpander_state::cv_in_r()  // U17 (CD4051, pot board) + U18 (74LS174, pot board)
{
	const u8 cv_index = selected_cv_in();
	assert(cv_index >= 0 && cv_index < 8);

	if (inhibit_cv_in())
		return 0;  // Floating ADC input.

	u8 cv = 0;
	if (cv_index < 6)
		cv = adc_byte(map_port_range(m_cv_io[cv_index], 0, 5));
	else
		cv = pedal_cv(cv_index - 6);

	LOGMASKED(LOG_CV_IN, "CV in: %02x - %02x\n", cv_index, cv);
	return cv;
}

u8 xpander_state::datain_r()  // U15 (74LS367, processor board)
{
	// D0 - HALTAKN*
	const u8 d0 = m_voiceboard->haltack_r() ? 0 : 1;

	// D1 - Memory write protect (low when protected)
	const u8 d1 = memory_protect_r() ? 0 : 1;

	// D2 - Cassette DATA.
	const u8 d2 = 1;  // TODO: Implement.

	// Buffer inputs for D3-D5 not connected. Will likely resolve to 1.
	// D6-D7 not connected to buffer. Data bus pulled high.
	const u8 d_unused = 0xf8;

	return d_unused | (d2 << 2) | (d1 << 1) | (d0 << 0);
}

void xpander_state::pull_w(u8 data)  // U21 (74HC374, pot board), CLK <- PULL*
{
	// Chain Advance input (bit 6) is always pulled high.
	m_pull = (BIT(data, 6) << 7) | (1 << 6) | BIT(data, 0, 6);
}

void xpander_state::update_banking()
{
	m_ram01_view.select(memory_protect_r() ? 1 : 0);
	m_rom0_view.select(m_voiceboard->haltack_r() ? 1 : 0);

	LOGMASKED(LOG_BANKING, "Banking - NVRAM A: %d, ROM0: %d\n",
			  *m_ram01_view.entry(), *m_rom0_view.entry());
}

void xpander_state::machine_start()
{
	xpander_state_base::machine_start();
	firq_timer_elapsed(*m_firq_timer, m_firq_timer_preset);  // Reset the timer.
	save_item(NAME(m_firq_timer_preset));
	save_item(NAME(m_pull));
}

void xpander_state::machine_reset()
{
	xpander_state_base::machine_reset();
	update_banking();
}

void xpander_state::maincpu_map(address_map &map)
{
	// Component designations refer to the processor board, unless otherwise
	// noted. The signal names below (e.g. DISP*, HALTSET*) match those in the
	// schematics.

	map.unmap_value_high();  // Data bus pulled high by resistors on pot board.

	// Address decoding when A15=0 done by U22B (74LS139), which controls access
	// to RAM and I/O ports.
	// RAM write and select signals can only go active when there is power (as
	// determined by the PUP circuit).
	// RAM0 and RAM1 (but not RAM2) can be write-protected in hardware by a
	// memory-protect switch (SW6, active-closed, disables /WR signal).
	map(0x000, 0x3fff).view(m_ram01_view);  // U22B-O[0-1]: RAM0*, RAM1*
	m_ram01_view[0](0x000, 0x3fff).ram().share("ram01");
	m_ram01_view[1](0x000, 0x3fff).readonly().share("ram01");
	map(0x4000, 0x5fff).ram().share("ram2");  // U22B-O2: RAM2*

	// U22B-O3 -> U23 (74LS42)
	map(0x6000, 0x61ff).mirror(0x0200).w(FUNC(xpander_state::display_w)); // U23-O0: DISP* and DISPCLR*
	map(0x6400, 0x6400).mirror(0x03ff).w(FUNC(xpander_state::firq_timer_preset_w));  // U23-O1: INTSET*
	map(0x6800, 0x6800).mirror(0x03ff).w("latch_haltset", FUNC(output_latch_device::write));  // U23-O2: HALTSET*
	// ACIA addressing: RS <- A0, CS1 <- A1, CS0 <- A2, /CS2 <- UART*.
	map(0x6c06, 0x6c07).mirror(0x03f8).rw("midiacia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));  // U23-O3: UART*
	map(0x7000, 0x7000).mirror(0x03ff).r(FUNC(xpander_state::datain_r));  // U23-O4: DATAIN*
	map(0x7400, 0x7400).mirror(0x03ff).w("latch_led2", FUNC(output_latch_device::write));  // U23-O5: LED2*
	map(0x7800, 0x7800).mirror(0x03ff).unmaprw();  // U23-O6: not connected

	// U23-O7: BEN* -> U15 (74LS42, pot board)
	map(0x7c00, 0x7c00).mirror(0x023f).r(FUNC(xpander_state::encoder_dir_r));  // U15-O0: DIR*
	map(0x7c40, 0x7c40).mirror(0x023f).r(FUNC(xpander_state::encoder_sw_r));  // U15-O1: SW*
	map(0x7c80, 0x7c80).mirror(0x023f).w("latch_led1", FUNC(output_latch_device::write));  // U15-O2: LED1*
	map(0x7cc0, 0x7cc0).mirror(0x023f).r(FUNC(xpander_state::gate_r));  // U15-O3: GATE*
	map(0x7d00, 0x7d00).mirror(0x023f).w(FUNC(xpander_state::pull_w));  // U15-O4: PULL*
	map(0x7d40, 0x7d47).mirror(0x0238).r(FUNC(xpander_state::switch_r));  // U15-O5: SWITCH*
	map(0x7d80, 0x7d8f).mirror(0x0230).rw(FUNC(xpander_state::adc_r), FUNC(xpander_state::adc_w));  // U15-O6: CV*
	map(0x7dc0, 0x7dc0).mirror(0x023f).unmaprw();  // U15-O7: not connected

	// Address decoding when A15=1 done by U22A (74LS139).
	map(0x8000, 0x9fff).view(m_rom0_view);  // U22A-O0
	m_rom0_view[0](0x8000, 0x9fff).rom().region("maincpu", 0x0000);  // ROM0*
	m_rom0_view[1](0x8000, 0x9fff).ram().share(m_voiceram);  // VOICERAM*
	map(0xa000, 0xffff).rom().region("maincpu", 0x2000);  // U22A-O[1-3]: ROM[1-3]*
}

void xpander_state::xpander(machine_config &config)
{
	// Component designations refer to the Xpander processor board, unless
	// otherwise noted.

	xpander_common(config);

	NVRAM(config, "ram01", nvram_device::DEFAULT_ALL_0);  // U2, U3 (6264), /WR <- WRITEA*
	NVRAM(config, "ram2", nvram_device::DEFAULT_ALL_0);  // U4 (6264), /WR <- WRITEB*

	TIMER(config, m_firq_timer).configure_generic(FUNC(xpander_state::firq_timer_elapsed));  // U12 (40103)

	XPANDER_VOICEBOARD(config, m_voiceboard);
	m_voiceboard->haltack_cb().set([this] (int state) { update_banking(); });

	// U25 (74LS374), CLK <- HALTSET*
	auto &haltset = OUTPUT_LATCH(config, "latch_haltset");
	haltset.bit_handler<0>().set(m_voiceboard, FUNC(xpandervb_device::haltreq_w)).invert();  // HALTREQ0*
	// Bits 1-3: HALTREQ1*-HALTREQ3*, not connected.
	// HALTREQ[1-3]* are not used in the production Xpander. They are probably
	// there to support different voice board configurations.
	haltset.bit_handler<4>().set(m_voiceboard, FUNC(xpandervb_device::reset_w)).invert();  // RES*
	haltset.bit_handler<5>().set_output("cassmute").invert();  // CASSMUTE*
	// Bit 6 not connected.
	haltset.bit_handler<7>().set_output("cassout");  // CASSOUT

	config.set_default_layout(layout_oberheim_xpander);
}


// *** Matrix-12 ***
class matrix12_state : public xpander_state_base
{
public:
	matrix12_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD;

	void matrix12(machine_config &config) ATTR_COLD;

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	std::pair<double, double> pedal_pulldown(int pedal) const override;
	u8 gate_inputs() const override;
	u8 kbd_pressure_cv() const;

	u8 cv_in_r() override;
	u8 datain_r();

	void voiceboard_reset_w(int state);
	void banksel_w(int state);

	void lset_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(lever_1_timer_elapsed);
	TIMER_DEVICE_CALLBACK_MEMBER(lever_2_timer_elapsed);

	void pit_out2_changed(int state);
	void update_banking();

	void maincpu_map(address_map &map) override ATTR_COLD;

	required_device_array<xpandervb_device, 2> m_voiceboard;
	required_shared_ptr_array<u8, 2> m_voiceram;
	memory_view m_mem_view;
	memory_view m_voiceram_view;
	required_device<matrix12_kbd_device> m_kbd;
	required_device<pit8253_device> m_pit;
	required_device_array<timer_device, 2> m_lever_timer;
	required_ioport_array<2> m_lever;
	required_ioport m_gate_io;
	required_ioport m_gate_config;
	required_ioport m_pressure;
	required_ioport m_pressure_scale;
	required_ioport m_pressure_offset;

	bool m_bsel;  // Bank select.
	bool m_pull_trigger;
	std::array<bool, 2> m_pull_pedal;
};

matrix12_state::matrix12_state(const machine_config &mconfig, device_type type, const char *tag)
	: xpander_state_base(mconfig, type, tag)
	, m_voiceboard(*this, "voiceboard_%u", 0U)
	, m_voiceram(*this, "voiceboard_%u:voiceram", 0U)
	, m_mem_view(*this, "mem_view")
	, m_voiceram_view(*this, "voiceram_view")
	, m_kbd(*this, "kbd")
	, m_pit(*this, "pit")
	, m_lever_timer(*this, "lever_%u_timer", 1U)
	, m_lever(*this, "lever_%u", 1U)
	, m_gate_io(*this, "gate_inputs")
	, m_gate_config(*this, "gate_config")
	, m_pressure(*this, "pressure")
	, m_pressure_scale(*this, "trimmer_pressure_scale")
	, m_pressure_offset(*this, "trimmer_pressure_offset")
	, m_bsel(false)
	, m_pull_trigger(false)
	, m_pull_pedal({false, false})
{
}

std::pair<double, double> matrix12_state::pedal_pulldown(int pedal) const
{
	assert(pedal == 0 || pedal == 1);
	const double v_pull = m_pull_pedal[pedal] ? 5.0 : 0.0;
	constexpr double R_PULL[2] = { RES_K(100), RES_K(100) };  // R61, R62
	return std::make_pair(v_pull, R_PULL[pedal]);
}

u8 matrix12_state::gate_inputs() const
{
	const u8 config = m_gate_config->read();
	const u8 value = m_gate_io->read();

	// When a pedal is connected, pedalXin will get disconnected from the
	// ADC input circuit and assume the value of the firmware-configured pull.
	// When a pedal is disconnected, pedalXin will be connected to the ADC
	// input circuit, and it will evaluate to a logical 1 regardless of the pull
	// state.
	const u8 pedal1in = pedal_connected(0) ? (m_pull_pedal[0] ? 1 : 0) : 1;
	const u8 pedal2in = pedal_connected(1) ? (m_pull_pedal[1] ? 1 : 0) : 1;

	const u8 chain_adv = BIT(config, 0) ? BIT(value, 0) : 1;

	u8 trigger = m_pull_trigger ? 1 : 0;
	if (BIT(config, 2))  // Trigger input connected?
	{
		trigger = BIT(value, 1);
		if (!BIT(config, 1))  // Inverse polarity?
			trigger ^= 1;
	}

	return (trigger << 7) | (chain_adv << 6) | (pedal2in << 5) | (pedal1in << 4);
}

u8 matrix12_state::kbd_pressure_cv() const
{
	// Schematic: Matrix-12 processor board, sheet 2 of 4.

	// Keyboard pressure (aka aftertouch) is measured by a force-sensitive
	// resistor (FSR).

	constexpr double T1_MAX = RES_K(1);  // scale trimmer
	constexpr double T2_MAX = RES_K(100);  // offset trimmer

	const double r_scale = map_port_range(m_pressure_scale, 0, T1_MAX);
	const double r_offset = map_port_range(m_pressure_offset, 0, T2_MAX);

	// The exact min and max values of the FSR are not known. The values below
	// were chosen to make the trimmers useful, so hopefully they are not too
	// far off the real ones.
	constexpr double R_PRESS_MIN = RES_K(5);
	constexpr double R_PRESS_MAX = RES_K(500);

	// Applying a bit of pressure should rapidly reduce resistance. Modeling that
	// with an audio pot response.
	const double press_response = RES_AUDIO_POT_LAW(map_port_range(m_pressure, 1, 0));
	const double r_press = fmaprange(press_response, 0.0, 1.0, R_PRESS_MIN, R_PRESS_MAX);

	// See circuit surrounding U23 (TL081).
	const double i_in = VPLUS / (r_offset + RES_K(470)) + VMINUS / (r_press + RES_R(470));  // R20, R21
	const double v_out = -(RES_K(1.5) + r_scale) * i_in;  // R17
	const u8 cv = adc_byte(v_out);

	LOGMASKED(LOG_CV_IN, "Pressure %f, %f, %u\n", r_press, v_out, cv);
	return cv;
}

u8 matrix12_state::cv_in_r()  // U17 (CD4051, pot board) + U18 (74LS174, pot board)
{
	const u8 cv_index = selected_cv_in();
	assert(cv_index >= 0 && cv_index < 8);
	if (inhibit_cv_in())
		return 0;  // Floating ADC input.

	u8 cv = 0;
	switch (cv_index)
	{
		case 0: cv = kbd_pressure_cv(); break;
		case 6: cv = pedal_cv(0); break;
		case 7: cv = pedal_cv(1); break;
		default: cv = 0; break;  // Floating ADC input.
	}

	LOGMASKED(LOG_CV_IN, "CV in: %02x - %02x\n", cv_index, cv);
	return cv;
}

u8 matrix12_state::datain_r()  // U50 (74LS367, processor board)
{
	// D0 - HALTAKN*
	const u8 d0 = (m_voiceboard[0]->haltack_r() || m_voiceboard[1]->haltack_r()) ? 0 : 1;

	// D1 - Memory write protect (low when protected)
	const u8 d1 = memory_protect_r() ? 0 : 1;

	// D2 - CDATA (cassette data)
	const u8 d2 = 1;  // TODO: implement.

	// D7 - DOR*
	const u8 d7 = m_kbd->dor_neg_r();

	// Buffer inputs for D3 and D4 not connected. Will likely resolve to 1.
	// D5 and D6 not connected to buffer. Data bus pulled high.
	const u8 d_unused = 0x78;

	return (d7 << 7) | d_unused | (d2 << 2) | (d1 << 1) | (d0 << 0);
}

void matrix12_state::voiceboard_reset_w(int state)
{
	for (auto &vb : m_voiceboard)
		vb->reset_w(state);
}

void matrix12_state::banksel_w(int state)
{
	m_bsel = state;
	update_banking();
}

void matrix12_state::lset_w(u8 data)
{
	// Each resistive lever is part of a one-shot RC circuit (74LS221). Strobing
	// LSET* triggers the two one-shots. Their output pulse time, measured by
	// an 8253, will depend on the position of the levers.

	for (int i = 0; i < 2; ++i)
	{
		if (m_lever_timer[i]->remaining().is_never())  // No timing is in progress.
		{
			// The 2 levers are 10KOhm potentiometers, centered at 5K. According
			// to repair info online, the motion range of the lever is ~2K, so
			// it takes values of 4K-6K.
			const double r_lever = map_port_range(m_lever[i], RES_K(6), RES_K(4));

			constexpr double c_ext = CAP_U(0.22);  // C46, C45
			const double r_ext = r_lever + RES_R(220);  // R39, R40
			const double t = 0.7 * c_ext * r_ext;  // 74LS221 timing formula.
			m_lever_timer[i]->adjust(attotime::from_double(t));

			if (i == 0)
				m_pit->write_gate0(1);
			else
				m_pit->write_gate1(1);

			LOGMASKED(LOG_LEVER, "Lever %d: %f, %f\n", i, r_lever, t);
		}
		else
		{
			// If a timing is in progress, additional triggers are ignored.
			LOGMASKED(LOG_LEVER, "Lever %d: timing still in progress\n");
		}
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(matrix12_state::lever_1_timer_elapsed)
{
	m_pit->write_gate0(0);
}

TIMER_DEVICE_CALLBACK_MEMBER(matrix12_state::lever_2_timer_elapsed)
{
	m_pit->write_gate1(0);
}

void matrix12_state::pit_out2_changed(int state)
{
	if (state)  // FIRQ triggered on positive-going transition.
	{
		// Using HOLD_LINE because there is circuitry that clears the FIRQ* line
		// when the CPU acks the interrupt (U48 - 74LS74 and  U47A - 74LS32 on
		// processor board, using BA BS* and A3 as inputs).
		maincpu()->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);

		// When FIRQ* is asserted, DISPCLR* will also get asserted.
		display_clear();
	}
}

void matrix12_state::update_banking()
{
	const bool haltack0 = m_voiceboard[0]->haltack_r();
	const bool haltack1 = m_voiceboard[1]->haltack_r();

	if (haltack0 && haltack1)
		// In this state, voice RAM reads will cause bus conflicts. This state
		// is entered during a voiceboard reset, but no voice RAM access is
		// attempted, so it is OK. Disabling voice RAM in this case, to ensure
		// "unmapped memory" errors are logged if it does get accessed.
		m_voiceram_view.select(2);
	else if (haltack0)
		m_voiceram_view.select(0);  // Arm voiceboard 0 RAM.
	else if (haltack1)
		m_voiceram_view.select(1);  // Arm voiceboard 1 RAM.
	else
		m_voiceram_view.select(2);  // Disable voiceboard RAMs.

	m_mem_view.select((haltack0 || haltack1 || m_bsel) ? 1 : 0);

	LOGMASKED(LOG_BANKING, "Banking - MEM: %d (bsel: %d haltack0: %d, haltack1: %d), Voiceram: %d\n",
			  *m_mem_view.entry(), m_bsel, haltack0, haltack1, *m_voiceram_view.entry());
}

void matrix12_state::machine_start()
{
	xpander_state_base::machine_start();
	save_item(NAME(m_bsel));
	save_item(NAME(m_pull_trigger));
	save_item(NAME(m_pull_pedal));
}

void matrix12_state::machine_reset()
{
	xpander_state_base::machine_reset();
	update_banking();
}

void matrix12_state::maincpu_map(address_map &map)
{
	// Component designations refer to the Matrix-12 processor board, unless
	// otherwise noted. The signal names below (e.g. DISP*, HALTSET*) refer to
	// those in the schematics.

	map.unmap_value_high();  // Data bus pulled high by resistors on pot board.

	// RAM select signals can only go active when there is power (as determined
	// by the PUP circuit).

	// Top-level address decoding when A15=0 is done by U40A (74LS139).
	map(0x0000, 0x3fff).ram().share("ram01");  // U40A-O[0-1]: BRAM0*, BRAM1*

	// U40A-02: I/O* -> U40B (74LS139) when A12=0.
	map(0x4000, 0x4001).mirror(0x0ffc).r(m_kbd, FUNC(matrix12_kbd_device::kbd0_r));  // U40B-O0: KBD0*
	map(0x4002, 0x4003).mirror(0x0ffc).r(m_kbd, FUNC(matrix12_kbd_device::kbd1_r));  // U40B-O1: KBD1*

	// U40A-02: I/O* -> U41 (74LS138) when A12=1.
	map(0x5000, 0x51ff).w(FUNC(matrix12_state::display_w)); // U41-O0: DISP* and DISPCLR*
	map(0x5200, 0x5203).mirror(0x01fc).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));  // U41-O1: INTSET*
	map(0x5400, 0x5400).mirror(0x01ff).w("latch_haltset", FUNC(output_latch_device::write));  // U41-O2: HALTSET*
	// ACIA addressing: RS <- A0, CS1 <- A1, CS0 <- A2, /CS2 <- UART*.
	map(0x5606, 0x5607).mirror(0x01f8).rw("midiacia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));  // U41-O3: UART*
	map(0x5800, 0x5800).mirror(0x01ff).r(FUNC(matrix12_state::datain_r));  // U41-O4: DATAIN*
	map(0x5a00, 0x5a00).mirror(0x01ff).w("latch_led2", FUNC(output_latch_device::write));  // U41-O5: LED2*
	map(0x5c00, 0x5c00).mirror(0x01ff).w(FUNC(matrix12_state::lset_w));  // U41-O6: LSET*

	// U41-O7: BEN* -> U15 (74LS42, on pot board)
	map(0x5e00, 0x5e00).mirror(0x003f).r(FUNC(matrix12_state::encoder_dir_r));  // U15-O0: DIR*
	map(0x5e40, 0x5e40).mirror(0x003f).r(FUNC(matrix12_state::encoder_sw_r));  // U15-O1: SW*
	map(0x5e80, 0x5e80).mirror(0x003f).w("latch_led1", FUNC(output_latch_device::write));  // U15-O2: LED1*
	map(0x5ec0, 0x5ec0).mirror(0x003f).r(FUNC(matrix12_state::gate_r));  // U15-O3: GATE*
	map(0x5f00, 0x5f00).mirror(0x003f).w("latch_pull", FUNC(output_latch_device::write));  // U15-O4: PULL*
	map(0x5f40, 0x5f47).mirror(0x0038).r(FUNC(matrix12_state::switch_r));  // U15-O5: SWITCH*
	map(0x5f80, 0x5f8f).mirror(0x0030).rw(FUNC(matrix12_state::adc_r), FUNC(matrix12_state::adc_w));  // U15-O6: CV*
	map(0x5fc0, 0x5fc0).mirror(0x003f).unmaprw();  // U15-O7: not connected.

	map(0x6000, 0x7fff).rom().region("maincpu", 0x0000);  // U40A-O3: ROM0*

	// Top-level address decoding when A15=1 is done by U39 (74LS138). Input C
	// is driven by a "bank select" signal, which is either activated by the
	// firmware (BSEL*) or by one of the voice CPUs releasing its bus to the
	// main CPU (HALTAKN*). Note that ROM4 is addressable in both bank
	// configurations.
	map(0x8000, 0xffff).view(m_mem_view);
	m_mem_view[0](0x8000, 0xffff).rom().region("maincpu", 0x2000);  // U39-O[0-3]: ROM1*-ROM4*
	m_mem_view[1](0x8000, 0x9fff).view(m_voiceram_view);  // U39-O4: VOICERAM*
	m_voiceram_view[0](0x8000, 0x9fff).ram().share(m_voiceram[0]);
	m_voiceram_view[1](0x8000, 0x9fff).ram().share(m_voiceram[1]);
	m_voiceram_view[2](0x8000, 0x9fff).unmaprw();
	m_mem_view[1](0xa000, 0xdfff).ram().share("ram23");  // U39-O[5-6]: BRAM2*, BRAM3*
	m_mem_view[1](0xe000, 0xffff).rom().region("maincpu", 0x8000);  // U39-O7: ROM4*
}

void matrix12_state::matrix12(machine_config &config)
{
	// Component designations refer to the processor board, unless otherwise noted.

	xpander_common(config);

	// Jumper wires and optional decoder ICs (U24, U25, 74HC139) can be used to
	// configure RAM as either 4 x 6264 or 16 x 6116. One subtle difference is
	// that the "memory protect" switch will not block the /WR signal in the
	// former configuration. /WR is only blocked for the "upper" 6116 sockets (
	// those that store patches in the 6116 configuration), but those sockets
	// are not used in the 6264 configuration. In both cases, the firmware reads
	// the "memory protect" signal and won't issue patch writes when asserted,
	// but the electrical /WR protection is missing from the 6264 configuration.
	// We currently emulate the 6264 configuration.
	NVRAM(config, "ram01", nvram_device::DEFAULT_ALL_0);  // U4, U3 (6264)
	NVRAM(config, "ram23", nvram_device::DEFAULT_ALL_0);  // U2, U1 (6264)

	TIMER(config, m_lever_timer[0])  // U49A (74LS221)
		.configure_generic(FUNC(matrix12_state::lever_1_timer_elapsed));
	TIMER(config, m_lever_timer[1])  // U49B (74LS221)
		.configure_generic(FUNC(matrix12_state::lever_2_timer_elapsed));

	PIT8253(config, m_pit);  // U31
	m_pit->set_clk<0>(16_MHz_XTAL / 8);  // 2 MHz
	m_pit->set_clk<1>(16_MHz_XTAL / 8);  // 2 MHz
	m_pit->set_clk<2>(16_MHz_XTAL / 8);  // 2 MHz
	m_pit->out_handler<2>().set(FUNC(matrix12_state::pit_out2_changed));

	MATRIX12_KBD(config, m_kbd, 16_MHz_XTAL / 16);  // 1 MHz

	for (int i = 0; i < m_voiceboard.size(); ++i)
	{
		XPANDER_VOICEBOARD(config, m_voiceboard[i]);
		m_voiceboard[i]->haltack_cb().set([this] (int state) { update_banking(); });
	}

	// U33 (74LS374), /CLK <- HALTSET*
	auto &haltset = OUTPUT_LATCH(config, "latch_haltset");
	haltset.bit_handler<0>().set(m_voiceboard[0], FUNC(xpandervb_device::haltreq_w)).invert();  // HALTREQ0*
	haltset.bit_handler<1>().set(m_voiceboard[1], FUNC(xpandervb_device::haltreq_w)).invert();  // HALTREQ1*
	// Bit 2 not connected.
	haltset.bit_handler<3>().set(m_kbd, FUNC(matrix12_kbd_device::kbdclr_w));  // KBDCLR
	haltset.bit_handler<4>().set(FUNC(matrix12_state::voiceboard_reset_w)).invert();  // RES*
	haltset.bit_handler<5>().set_output("cassmute").invert();  // CASSMUTE*
	haltset.bit_handler<6>().set(FUNC(matrix12_state::banksel_w)).invert();  // BSEL*
	haltset.bit_handler<7>().set_output("cassout");  // CASSOUT

	// U21 (74LS374, pot board) <- /CLK <- PULL*
	auto &pull = OUTPUT_LATCH(config, "latch_pull");
	pull.bit_handler<0>().set_output("led_voices_1_6").invert();
	pull.bit_handler<1>().set_output("led_voices_7_12").invert();
	// Bits 2 and 3 not connected.
	pull.bit_handler<4>().set([this] (int state) { m_pull_pedal[0] = state; });  // PULL5
	pull.bit_handler<5>().set([this] (int state) { m_pull_pedal[1] = state; });  // PULL6
	pull.bit_handler<6>().set([this] (int state) { m_pull_trigger = state; });  // PULL7
	// Bit 7 not connected.

	config.set_default_layout(layout_oberheim_matrix12);
}

INPUT_PORTS_START(xpander_base)
	PORT_START("switches_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)

	PORT_START("switches_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("+") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("STORE") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PAGE2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TUNE")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MASTER")

	PORT_START("switches_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SINGLE")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MULTI")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VOICE 1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VOICE 2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VOICE 3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VOICE 4")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VOICE 5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VOICE 6")

	PORT_START("switches_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1 5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1 6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VCO1") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VCO2") PORT_CODE(KEYCODE_H)

	PORT_START("switches_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2 5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2 6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("FM/LAG") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TRACK X") PORT_CODE(KEYCODE_COMMA)

	PORT_START("switches_5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LEVER 1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LEVER 2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PEDAL 1") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PEDAL 2") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VIB") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("KEYBOARD") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LAG") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VELOCITY") PORT_CODE(KEYCODE_8)

	// switches_6 is defined in the `xpander` and `matrix12` specializations
	// below. The two differ by 1 button.

	PORT_START("switches_7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VCF/VCA") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ENV X") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LFO X") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RAMP X") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MISC.") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)  // No switch. Pulled up.
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)  // No switch. Pulled up.
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)  // No switch. Pulled up.

	PORT_START("memory_protect")  // Memory Protect switch on processor board.
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MEMORY PROTECT") PORT_TOGGLE
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xpander_state_base::memory_protect_changed), 0)

	PORT_START("volume_knob")  // 50K dual-gang potentiometer.
	PORT_ADJUSTER(90, "VOLUME")

	PORT_START("pedal_1")
	PORT_BIT(0xff, 0x00, IPT_PEDAL1) PORT_SENSITIVITY(30) PORT_KEYDELTA(30)

	PORT_START("pedal_2")
	PORT_BIT(0xff, 0x00, IPT_PEDAL2) PORT_SENSITIVITY(30) PORT_KEYDELTA(30)

	// The Xpander and Matrix-12 support multiple pedal types. Look for
	// "enum pedal_type" for info on the different types. As with the real synth,
	// the polarity of the connected pedal should match the polarity configured
	// in the synth's menu, for things to work correctly.

	PORT_START("pedal_1_type")  // Pedal 1 is normally the volume pedal (continuous).
	PORT_CONFNAME(0x07, PEDAL_TYPE_CONT_PASSIVE_POS, "PEDAL 1 TYPE")
	PORT_CONFSETTING(PEDAL_TYPE_NC, "NOT CONNECTED")
	PORT_CONFSETTING(PEDAL_TYPE_CONT_ACTIVE_NEG, "VOLTAGE -")
	PORT_CONFSETTING(PEDAL_TYPE_CONT_ACTIVE_POS, "VOLTAGE +")
	PORT_CONFSETTING(PEDAL_TYPE_CONT_PASSIVE_NEG, "RESISTANCE -")
	PORT_CONFSETTING(PEDAL_TYPE_CONT_PASSIVE_POS, "RESISTANCE +")
	PORT_CONFSETTING(PEDAL_TYPE_SW_NEG, "SWITCH -")
	PORT_CONFSETTING(PEDAL_TYPE_SW_ACTIVE_POS, "SWITCH +")

	PORT_START("pedal_2_type")  // Pedal 2 is normally the sustain pedal (switch).
	PORT_CONFNAME(0x07, PEDAL_TYPE_SW_NEG, "PEDAL 2 TYPE")
	PORT_CONFSETTING(PEDAL_TYPE_NC, "NOT CONNECTED")
	PORT_CONFSETTING(PEDAL_TYPE_CONT_ACTIVE_NEG, "VOLTAGE -")
	PORT_CONFSETTING(PEDAL_TYPE_CONT_ACTIVE_POS, "VOLTAGE +")
	PORT_CONFSETTING(PEDAL_TYPE_CONT_PASSIVE_NEG, "RESISTANCE -")
	PORT_CONFSETTING(PEDAL_TYPE_CONT_PASSIVE_POS, "RESISTANCE +")
	PORT_CONFSETTING(PEDAL_TYPE_SW_NEG, "SWITCH -")
	PORT_CONFSETTING(PEDAL_TYPE_SW_ACTIVE_POS, "SWITCH +")

	// All rotary encoders are LA226.

	// 30 teeth (according to the service manual) x 4 events per tooth.
	constexpr int NUM_ENCODER_POSITIONS = 30 * 4;

	PORT_START("encoder_knob_1")
	PORT_BIT(0xff, 0x00, IPT_POSITIONAL) PORT_POSITIONS(NUM_ENCODER_POSITIONS - 1)
		PORT_WRAPS PORT_SENSITIVITY(50) PORT_KEYDELTA(4)
		PORT_CODE_DEC(KEYCODE_Q) PORT_CODE_INC(KEYCODE_W)
		PORT_FULL_TURN_COUNT(NUM_ENCODER_POSITIONS)
		PORT_CHANGED_MEMBER("encoder_1", FUNC(quadencoder_device::changed), 0)

	PORT_START("encoder_knob_2")
	PORT_BIT(0xff, 0x00, IPT_POSITIONAL) PORT_POSITIONS(NUM_ENCODER_POSITIONS - 1)
		PORT_WRAPS PORT_SENSITIVITY(50) PORT_KEYDELTA(4)
		PORT_CODE_DEC(KEYCODE_E) PORT_CODE_INC(KEYCODE_R)
		PORT_FULL_TURN_COUNT(NUM_ENCODER_POSITIONS)
		PORT_CHANGED_MEMBER("encoder_2", FUNC(quadencoder_device::changed), 0)

	PORT_START("encoder_knob_3")
	PORT_BIT(0xff, 0x00, IPT_POSITIONAL) PORT_POSITIONS(NUM_ENCODER_POSITIONS - 1)
		PORT_WRAPS PORT_SENSITIVITY(50) PORT_KEYDELTA(4)
		PORT_CODE_DEC(KEYCODE_A) PORT_CODE_INC(KEYCODE_S)
		PORT_FULL_TURN_COUNT(NUM_ENCODER_POSITIONS)
		PORT_CHANGED_MEMBER("encoder_3", FUNC(quadencoder_device::changed), 0)

	PORT_START("encoder_knob_4")
	PORT_BIT(0xff, 0x00, IPT_POSITIONAL) PORT_POSITIONS(NUM_ENCODER_POSITIONS - 1)
		PORT_WRAPS PORT_SENSITIVITY(50) PORT_KEYDELTA(4)
		PORT_CODE_DEC(KEYCODE_D) PORT_CODE_INC(KEYCODE_F)
		PORT_FULL_TURN_COUNT(NUM_ENCODER_POSITIONS)
		PORT_CHANGED_MEMBER("encoder_4", FUNC(quadencoder_device::changed), 0)

	PORT_START("encoder_knob_5")
	PORT_BIT(0xff, 0x00, IPT_POSITIONAL) PORT_POSITIONS(NUM_ENCODER_POSITIONS - 1)
		PORT_WRAPS PORT_SENSITIVITY(50) PORT_KEYDELTA(4)
		PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X)
		PORT_FULL_TURN_COUNT(NUM_ENCODER_POSITIONS)
		PORT_CHANGED_MEMBER("encoder_5", FUNC(quadencoder_device::changed), 0)

	PORT_START("encoder_knob_6")
	PORT_BIT(0xff, 0x00, IPT_POSITIONAL) PORT_POSITIONS(NUM_ENCODER_POSITIONS - 1)
		PORT_WRAPS PORT_SENSITIVITY(50) PORT_KEYDELTA(4)
		PORT_CODE_DEC(KEYCODE_C) PORT_CODE_INC(KEYCODE_V)
		PORT_FULL_TURN_COUNT(NUM_ENCODER_POSITIONS)
		PORT_CHANGED_MEMBER("encoder_6", FUNC(quadencoder_device::changed), 0)
INPUT_PORTS_END

INPUT_PORTS_START(xpander)
	PORT_INCLUDE(xpander_base)

	PORT_START("switches_6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RELEASE VELOCITY") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PRESSURE") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ENV") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LFO") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TRACK") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RAMP") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)  // No switch. Pulled up.
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)  // No switch. Pulled up.

	PORT_START("gate_inputs")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("GATE 1") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("GATE 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("GATE 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("GATE 4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("GATE 5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("GATE 6")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("CHAIN ADVANCE")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRIGGER") PORT_CODE(KEYCODE_T)

	// The polarity of the Gate inputs (except "chain advance") can be specified
	// via the "master" menu on the synth. The configuration below controls the
	// polarity of what is actually connected to those inputs. As with the real
	// synth, the settings on the synth need to match what's actually connected
	// for things to work correctly.
	PORT_START("gate_config")
	PORT_CONFNAME(0x0101, 0x0101, "GATE 1")
	PORT_CONFSETTING(0x0000, "NOT CONNECTED")
	PORT_CONFSETTING(0x0100, "- POLARITY")  // active low
	PORT_CONFSETTING(0x0101, "+ POLARITY")  // active high
	PORT_CONFNAME(0x0202, 0x0202, "GATE 2")
	PORT_CONFSETTING(0x0000, "NOT CONNECTED")
	PORT_CONFSETTING(0x0200, "- POLARITY")
	PORT_CONFSETTING(0x0202, "+ POLARITY")
	PORT_CONFNAME(0x0404, 0x0404, "GATE 3")
	PORT_CONFSETTING(0x0000, "NOT CONNECTED")
	PORT_CONFSETTING(0x0400, "- POLARITY")
	PORT_CONFSETTING(0x0404, "+ POLARITY")
	PORT_CONFNAME(0x0808, 0x0808, "GATE 4")
	PORT_CONFSETTING(0x0000, "NOT CONNECTED")
	PORT_CONFSETTING(0x0800, "- POLARITY")
	PORT_CONFSETTING(0x0808, "+ POLARITY")
	PORT_CONFNAME(0x1010, 0x1010, "GATE 5")
	PORT_CONFSETTING(0x0000, "NOT CONNECTED")
	PORT_CONFSETTING(0x1000, "- POLARITY")
	PORT_CONFSETTING(0x1010, "+ POLARITY")
	PORT_CONFNAME(0x2020, 0x2020, "GATE 6")
	PORT_CONFSETTING(0x0000, "NOT CONNECTED")
	PORT_CONFSETTING(0x2000, "- POLARITY")
	PORT_CONFSETTING(0x2020, "+ POLARITY")
	PORT_CONFNAME(0x4040, 0x4000, "CHAIN ADVANCE")
	PORT_CONFSETTING(0x0000, "NOT CONNECTED")
	PORT_CONFSETTING(0x4000, "CONNECTED")  // Chain Advance switch is always active low (- polarity).
	PORT_CONFNAME(0x8080, 0x8080, "TRIGGER")
	PORT_CONFSETTING(0x0000, "NOT CONNECTED")
	PORT_CONFSETTING(0x8000, "- POLARITY")
	PORT_CONFSETTING(0x8080, "+ POLARITY")

	PORT_START("cv_in_1")
	PORT_ADJUSTER(0, "CV 1")

	PORT_START("cv_in_2")
	PORT_ADJUSTER(0, "CV 2")

	PORT_START("cv_in_3")
	PORT_ADJUSTER(0, "CV 3")

	PORT_START("cv_in_4")
	PORT_ADJUSTER(0, "CV 4")

	PORT_START("cv_in_5")
	PORT_ADJUSTER(0, "CV 5")

	PORT_START("cv_in_6")
	PORT_ADJUSTER(0, "CV 6")
INPUT_PORTS_END

INPUT_PORTS_START(matrix12)
	PORT_INCLUDE(xpander_base)

	PORT_START("switches_6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RELEASE VELOCITY") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PRESSURE") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ENV") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LFO") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TRACK") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RAMP") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)  // No switch. Pulled up.
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VOICES 1-6, 7-12")

	PORT_START("gate_inputs")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CHAIN ADVANCE")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRIGGER") PORT_CODE(KEYCODE_T)

	// The polarity of the trigger input can be specified via the "master" menu
	// on the synth. The configuration below controls the polarity of the actual
	// connected input. As with the real synth, the two polarities need to match
	// for things to work correctly.
	PORT_START("gate_config")
	PORT_CONFNAME(0x01, 0x01, "CHAIN ADVANCE")
	PORT_CONFSETTING(0x00, "NOT CONNECTED")
	PORT_CONFSETTING(0x01, "CONNECTED")
	PORT_CONFNAME(0x06, 0x06, "TRIGGER")
	PORT_CONFSETTING(0x00, "NOT CONNECTED")
	PORT_CONFSETTING(0x04, "- POLARITY")
	PORT_CONFSETTING(0x06, "+ POLARITY")

	PORT_START("lever_1")  // 10KOhm pot, center at 5K, usable range: 4K-6K.
	PORT_BIT(0xff, 50, IPT_PADDLE_V) PORT_NAME("LEVER 1") PORT_MINMAX(0, 100)
		PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_CENTERDELTA(30)
		PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT)

	PORT_START("lever_2")  // Same specs as lever 1.
	PORT_BIT(0xff, 50, IPT_PADDLE_V) PORT_NAME("LEVER 1") PORT_MINMAX(0, 100)
		PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_CENTERDELTA(30)
		PORT_CODE_DEC(KEYCODE_DOWN) PORT_CODE_INC(KEYCODE_UP)

	PORT_START("pressure")
	PORT_BIT(0xff, 0x00, IPT_PEDAL3) PORT_SENSITIVITY(30) PORT_KEYDELTA(30)

	PORT_START("trimmer_pressure_scale")  // T1, 1KOhm
	PORT_ADJUSTER(80, "TRIMMER: PRESSURE SCALE")

	PORT_START("trimmer_pressure_offset")  // T2, 100KOhm
	PORT_ADJUSTER(30, "TRIMMER: PRESSURE OFFSET")
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
ROM_END

ROM_START(matrix12)
	ROM_REGION(0xa000, "maincpu", 0)  // 5 x 2764 8Kx8bit ROMs (U9 - U5, processor board). Rev 1.1.
	ROM_LOAD("mat1.1-0.u9", 0x000000, 0x002000, CRC(2a241ea1) SHA1(d459c40e3f72c261f23187d5f7a36df3d5ab6f4b))
	ROM_LOAD("mat1.1-1.u8", 0x002000, 0x002000, CRC(4a871fec) SHA1(b9f73f4ff4e3be1d1371478ccce089bcdd2f38dd))
	ROM_LOAD("mat1.1-2.u7", 0x004000, 0x002000, CRC(cb3d0359) SHA1(cd98f0ee46812dfad2aed6f43fcaab0d37679fa8))
	ROM_LOAD("mat1.1-3.u6", 0x006000, 0x002000, CRC(ce8d0e4d) SHA1(a465fcd1f432a81ee30af91de0fd517aac194726))
	ROM_LOAD("mat1.1-4.u5", 0x008000, 0x002000, CRC(1eb21a15) SHA1(218f837d500a2c1c19614bc07084a5c85aac7007))
ROM_END

}  // anonymous namespace

SYST(1984, xpander, 0, 0, xpander, xpander, xpander_state, empty_init, "Oberheim", "Xpander", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)  // 1984-1988
SYST(1985, matrix12, 0, 0, matrix12, matrix12, matrix12_state, empty_init, "Oberheim", "Matrix-12", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)  // 1985-1988
