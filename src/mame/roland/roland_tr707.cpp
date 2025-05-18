// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland TR-707/727 drum machines.

    From the Service Notes: “The differences between two models [TR-707 and
    TR-727] are sound data, component values in several audio stages and a
    couple of pin connections at IC30 of Voice board. Both models derive all
    rhythm sounds from PCM-encoded samples of real sounds stored in ROM.”

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
#include "sound/va_eg.h"
//#include "video/hd61603.h"
#include "video/pwm.h"

#define LOG_TRIGGER (1U << 1)
#define LOG_SYNC    (1U << 2)
#define LOG_TEMPO   (1U << 3)
#define LOG_ACCENT  (1U << 4)
#define LOG_CART    (1U << 5)

#define VERBOSE (LOG_GENERAL | LOG_TRIGGER | LOG_TEMPO | LOG_CART)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

namespace {

class roland_tr707_state : public driver_device
{
public:
	roland_tr707_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cartslot(*this, "cartslot")
		, m_mac(*this, "mac")
		, m_led_matrix(*this, "led_matrix")
		, m_key_switches(*this, "KEY%u", 0U)
		, m_cart_led(*this, "led_cart")
		, m_leds(4)
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
		, m_accent_level(*this, "ACCENT")
		, m_cart_bank(0)
		, m_key_led_row(0xff)
		, m_tempo_source(0xff)
		, m_midi_rxd_bit(true)  // Initial value is high, for serial "idle".
	{
		constexpr const char *LED_NAME_SUFFIXES[4][6] =
		{
			{"1", "2", "3", "4", "scale_1", "scale_2"},
			{"5", "6", "7", "8", "scale_3", "scale_4"},
			{"9", "10", "11", "12", "group_a", "group_b"},
			{"13", "14", "15", "16", "group_c", "group_d"},
		};
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 6; ++j)
				m_leds[i].push_back(output_finder<>(*this, std::string("led_") + LED_NAME_SUFFIXES[i][j]));
	}

	void tr707(machine_config &config);

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

	static double discharge_t(double r, double c, double v);

	u8 key_scan_r();
	void key_led_row_w(u8 data);
	void leds_w(u8 data);
	void led_outputs_w(offs_t offset, u8 data);
	void accent_level_w(u8 data);
	u8 ga_trigger_r(offs_t offset);
	void ga_trigger_w(offs_t offset, u8 data);
	void voice_select_w(u8 data);

	int cart_connected_r() const;
	u8 cart_r(offs_t offset);
	void cart_w(offs_t offset, u8 data);

	int midi_rxd_r() const;
	void midi_rxd_w(int state);

	template<enum dinsync_index Which> int dinsync_r() const;
	template<enum dinsync_index Which> void dinsync_w(int state);

	void tempo_source_w(u8 data);
	void update_tempo_line();
	void internal_tempo_clock_changed(int state);
	void update_internal_tempo_timer(bool cap_reset);
	TIMER_DEVICE_CALLBACK_MEMBER(tempo_timer_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(tempo_restart_timer_tick);

	void update_accent_adc();
	void accent_adc_flipflop_cb(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(accent_adc_timer_tick);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<hd6303x_cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cartslot;
	required_device<mb63h114_device> m_mac;
	required_device<pwm_display_device> m_led_matrix;
	required_ioport_array<4> m_key_switches;
	output_finder<> m_cart_led;  // D325 (GL9NP2) dual LED (red & green).
	std::vector<std::vector<output_finder<>>> m_leds;

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

void roland_tr707_state::machine_start()
{
	save_item(NAME(m_cart_bank));
	save_item(NAME(m_key_led_row));
	save_item(NAME(m_tempo_source));
	save_item(NAME(m_midi_rxd_bit));

	m_dinsync_out.resolve();
	m_cart_led.resolve();
	for (std::vector<output_finder<>> &led_row : m_leds)
		for (output_finder<> &led_output : led_row)
			led_output.resolve();
}

void roland_tr707_state::machine_reset()
{
	update_internal_tempo_timer(true);
	update_accent_adc();
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

void roland_tr707_state::accent_level_w(u8 data)
{
	// Bits D0-D5 converted to a CV by a DAC resistor network: RA5, RKM7LW502 in
	// the schematic, RKM7LM502 in the parts list.
	LOGMASKED(LOG_TRIGGER, "Accent level: %d\n", data);
}

u8 roland_tr707_state::ga_trigger_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		ga_trigger_w(offset, 0);
	return 0x00;  // Data bus pulled low.
}

void roland_tr707_state::ga_trigger_w(offs_t offset, u8 data)
{
	// TODO: A0-A9 also trigger the envelope generators for each voice.
	m_mac->xst_w(offset & 0xff);

	m_cart_bank = BIT(offset, 10);
	m_cart_led = (offset >> 10) & 0x03; // Bit 11: green, bit 10: red.

	if ((offset & 0x3ff) != 0x3ff)
		LOGMASKED(LOG_TRIGGER, "GA trigger: %03x\n", offset);
}

void roland_tr707_state::voice_select_w(u8 data)
{
	// Bits D0-D5.
	LOGMASKED(LOG_TRIGGER, "Voice selected: %02x\n", data);
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
	// 4A <- Cartridge LED, red (see ga_trigger_w()).
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
	const double knob_r = KNOB_MAX * (100.0 - m_tempo_knob->read()) / 100.0;
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

void roland_tr707_state::internal_tempo_clock_changed(int state)
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
	//map(0x1000, 0x1000).mirror(0xfff).rw("lcdd", FUNC(hd61602_device::ready_r), FUNC(hd61602_device::write));
	map(0x2000, 0x27ff).ram().share("nvram1");
	map(0x2800, 0x2fff).ram().share("nvram2");
	map(0x3000, 0x3fff).rw(FUNC(roland_tr707_state::cart_r), FUNC(roland_tr707_state::cart_w));
	map(0x4000, 0x4000).mirror(0xfff).w(FUNC(roland_tr707_state::leds_w));
	map(0x5000, 0x5000).mirror(0xfff).w(FUNC(roland_tr707_state::accent_level_w));
	map(0x6000, 0x6fff).r(FUNC(roland_tr707_state::ga_trigger_r)).w(FUNC(roland_tr707_state::ga_trigger_w));
	map(0x7000, 0x7000).mirror(0xfff).w(FUNC(roland_tr707_state::voice_select_w));
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

	PORT_START("TEMPO")  // Tempo knob, VR301, 1M (EWH-LNAF20C16).
	PORT_ADJUSTER(50, "Tempo") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(roland_tr707_state::tempo_pots_adjusted), 0)

	PORT_START("ACCENT")  // Accent level slider, VR201, 50K(B) (S2018).
	PORT_ADJUSTER(50, "Accent") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(roland_tr707_state::accent_pots_adjusted), 0)

	// Trimmer defaults based on calibration instructions in the service notes.
	PORT_START("TM1")  // Tempo trimmer, TM-1, 200K(B) (RVF8P01-204).
	PORT_ADJUSTER(62, "TRIMMER: tempo")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(roland_tr707_state::tempo_pots_adjusted), 0)
	PORT_START("TM2")  // Accent series trimmer, TM-2, 50K(B) (RVF8P01-503).
	PORT_ADJUSTER(45, "TRIMMER: accent, series (TM2)")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(roland_tr707_state::accent_pots_adjusted), 0)
	PORT_START("TM3")  // Accent parallel trimmer, TM-3, 200K(B) (RVF8P01-204).
	PORT_ADJUSTER(15, "TRIMMER: accent, parallel (TM3)")
		 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(roland_tr707_state::accent_pots_adjusted), 0)

	PORT_START("DINSYNC_CONFIG")
	PORT_CONFNAME(0x01, 0x00, "DIN sync input cable connected")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x01, DEF_STR(Yes))

	PORT_START("DINSYNC")  // SYNC socket (J4). Bit numbers map to `enum dinsync_index`.
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SYNC start/stop")  // Pin 1.
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SYNC tempo")  // Pin 3. 24 cloks per quarter note.
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(roland_tr707_state::sync_input_changed), 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SYNC Continue")  // Pin 5.

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

void roland_tr707_state::tr707(machine_config &config)
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
	// P52 and P53 are inputs connected to eact other. Assuming they evaluate
	// to 1, but not sure.
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
	m_tempo_ff->comp_output_cb().set(FUNC(roland_tr707_state::internal_tempo_clock_changed));

	VA_RC_EG(config, m_accent_adc_rc).set_c(CAP_U(0.27));  // C15.
	TIMER(config, m_accent_adc_timer).configure_generic(FUNC(roland_tr707_state::accent_adc_timer_tick));
	TTL7474(config, m_accent_adc_ff, 0);  // 4013, IC4b.
	m_accent_adc_ff->d_w(1);  // D tied to VCC.
	m_accent_adc_ff->comp_output_cb().set(FUNC(roland_tr707_state::accent_adc_flipflop_cb));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(FUNC(roland_tr707_state::midi_rxd_w));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	//HD61602(config, "lcdd");

	GENERIC_CARTSLOT(config, m_cartslot, generic_plain_slot, nullptr, "tr707_cart");

	PWM_DISPLAY(config, m_led_matrix).set_size(4, 6);
	m_led_matrix->output_x().set(FUNC(roland_tr707_state::led_outputs_w));

	MB63H114(config, m_mac, 1.6_MHz_XTAL);
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


SYST(1985, tr707, 0, 0, tr707, tr707, roland_tr707_state, empty_init, "Roland", "TR-707 Rhythm Composer", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(1985, tr727, 0, 0, tr707, tr707, roland_tr707_state, empty_init, "Roland", "TR-727 Rhythm Composer", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
