// license:BSD-3-Clause
// copyright-holders:Devin Acker
// thanks-to:BCM

/***************************************************************************
    Casiotone 8000 / "Symphonytron" system

    The Symphonytron was a modular electronic organ produced by Casio in 1983.
    The full system consists of:

    - up to two Casiotone 8000 keyboards (8049 CPU, 2x uPD931 "vowel-consonant synthesis")
    - RC-1 accompaniment unit (uPD7801 CPU, uPD930 rhythm generator, analog percussion)
    - MB-1 memory unit (8049 CPU, 2x uPD931, RAM cartridge slot)
    - FK-1 pedal keyboard (8049 CPU, single uPD931)
    - CS-100 or CS-200 keyboard stand with built-in mixer

    The keyboards and memory unit all connect to the RC-1 via 14-pin DIN connectors.
    Although the RAM cart slot is located on the MB-1, all actual access to the cart is controlled
    remotely by the RC-1, which uses the cart to record and play back both rhythm/chord and melody
    data. The MB-1's sound hardware is then used to play back recorded melody data independently of
    the keyboards. The RC-1 also has a "tone mix" feature, where note data received from one keyboard
    is automatically forwarded to the other.

    The individual units can also be used on their own; the MB-1 will also respond to notes and tone
    selection commands via the DIN connector, but it needs the RC-1 present to do much else.
    It's marked as "not working" for this reason.

    This driver also features MIDI in/thru support via an "adapter" device which translates a subset
    of MIDI messages into the protocol used with the original connectors.

    TODO:
    - volume/expression pedal (for all systems)
    - fix aliasing in BBD output for some presets

***************************************************************************/

#include "emu.h"

#include "ct8000_midi.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8243.h"
#include "machine/rescap.h"
#include "sound/bbd.h"
#include "sound/flt_biquad.h"
#include "sound/flt_rc.h"
#include "sound/flt_vol.h"
#include "sound/mixer.h"
#include "sound/upd931.h"

#include "speaker.h"

#define LOG_VCO (1<<1)

// #define VERBOSE (LOG_GENERAL | LOG_VCO)

#include "logmacro.h"

#include "ct8000.lh"
#include "ctfk1.lh"

namespace {

class ct8000_state : public driver_device
{
public:
	ct8000_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_io(*this, "ioport%c", 'a')
		, m_midi(*this, "midi")
		, m_931a(*this, "upd931a")
		, m_931b(*this, "upd931b")
		, m_mixer(*this, "mixer")
		, m_filter_rc(*this, "filter_rc%u", 0)
		, m_filter_bq(*this, "filter_bq%u", 0)
		, m_bbd(*this, "bbd")
		, m_bank(*this, "bank")
		, m_inputs(*this, "KC%X", 0) // schematic uses KC0-9, A-C
	{
	}

	void config_base(machine_config &config) ATTR_COLD;
	void ctmb1(machine_config &config) ATTR_COLD;
	void ct8000(machine_config &config) ATTR_COLD;
	void ctfk1(machine_config &config) ATTR_COLD;

	ioport_value switch_r() { return m_switch; }
	DECLARE_INPUT_CHANGED_MEMBER(switch_w);
	DECLARE_INPUT_CHANGED_MEMBER(switch_clear_w);

protected:
	void ct8000_map(address_map &map) ATTR_COLD;
	void ct8000_io_map(address_map &map) ATTR_COLD;

	virtual void driver_start() override ATTR_COLD;
	virtual void driver_reset() override ATTR_COLD;

	void p1_w(u8 data);
	u8 p1_r();
	void p2_w(u8 data);
	u8 p2_r();

	void p4a_w(u8 data);
	void p5a_w(u8 data);
	void p6a_w(u8 data);
	void p7a_w(u8 data);

	void p4b_w(u8 data);
	void p7b_w(u8 data);

	u8 keys_r();

	void filter_main_w(u8 data);
	void filter_sub_w(u8 data);
	void filter_bass_w(u8 data);

	void pll_w(offs_t offset, u8 data);
	virtual void update_clocks();

	required_device<i8049_device> m_maincpu;
	required_device_array<i8243_device, 2> m_io;

	required_device<ct8000_midi_device> m_midi;

	required_device<upd931_device> m_931a;
	optional_device<upd931_device> m_931b;
	required_device<mixer_device> m_mixer;
	optional_device_array<filter_rc_device, 3> m_filter_rc;
	optional_device_array<filter_biquad_device, 3> m_filter_bq;
	optional_device<mn3207_device> m_bbd;

	required_memory_bank m_bank;

	optional_ioport_array<13> m_inputs;

	TIMER_CALLBACK_MEMBER(bbd_tick);
	void bbd_setup_next_tick();
	
	emu_timer *m_bbd_timer;

	u16 m_key_select;
	u8 m_key_enable;

	ioport_value m_switch;

	u16 m_pll_counter[2];
	u16 m_pll_ref[2];

	u8 m_clock_select;
	u8 m_clock_div;
};

void ct8000_state::ct8000_map(address_map &map)
{
	map(0x800, 0xfff).bankr("bank");
}

void ct8000_state::ct8000_io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(ct8000_state::keys_r), FUNC(ct8000_state::pll_w));
}

//**************************************************************************
void ct8000_state::config_base(machine_config &config)
{
	I8049(config, m_maincpu, 4.946864_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ct8000_state::ct8000_map);
	m_maincpu->set_addrmap(AS_IO, &ct8000_state::ct8000_io_map);
	m_maincpu->p1_out_cb().set(FUNC(ct8000_state::p1_w));
	m_maincpu->p1_in_cb().set(FUNC(ct8000_state::p1_r));
	m_maincpu->p2_out_cb().set(FUNC(ct8000_state::p2_w));
	m_maincpu->p2_in_cb().set(FUNC(ct8000_state::p2_r));

	CT8000_MIDI(config, m_midi);
	m_midi->int_cb().set_inputline(m_maincpu, MCS48_INPUT_IRQ);
	m_maincpu->t0_in_cb().set(m_midi, FUNC(ct8000_midi_device::ack_r));

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_midi, FUNC(ct8000_midi_device::rx_w));

	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");
	mdin.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	I8243(config, m_io[0]);
	m_maincpu->prog_out_cb().set(m_io[0], FUNC(i8243_device::prog_w));
	m_io[0]->p4_out_cb().set(FUNC(ct8000_state::p4a_w));
	m_io[0]->p5_out_cb().set(FUNC(ct8000_state::p5a_w));
	m_io[0]->p6_out_cb().set(FUNC(ct8000_state::p6a_w));
	m_io[0]->p7_out_cb().set(FUNC(ct8000_state::p7a_w));

	I8243(config, m_io[1]);
	m_maincpu->prog_out_cb().append(m_io[1], FUNC(i8243_device::prog_w));
	m_io[1]->p4_out_cb().set(FUNC(ct8000_state::p4b_w));
	m_io[1]->p5_in_cb().set(m_midi, FUNC(ct8000_midi_device::data_r));
	m_io[1]->p5_out_cb().set_nop();
	m_io[1]->p6_out_cb().set(m_midi, FUNC(ct8000_midi_device::data_w));
	m_io[1]->p7_out_cb().set(FUNC(ct8000_state::p7b_w));
}

//**************************************************************************
void ct8000_state::ctmb1(machine_config &config)
{
	config_base(config);

	SPEAKER(config, "speaker", 2).front();

	MIXER(config, m_mixer);
	m_mixer->add_route(0, "speaker", 1.0, 0);
	m_mixer->add_route(0, "speaker", 1.0, 1);

	// 931 A - sub (consonant) waveform
	UPD931(config, m_931a, m_maincpu->clock());
	m_931a->set_master(false);
	m_931a->filter_cb().set(FUNC(ct8000_state::filter_sub_w));
	/*
	Both of the uPD931s have a fairly large amount of headroom in their outputs,
	which is partly compensated for via adjustable gain (see filter_main_w and filter_sub_w),
	but the base output gain should be boosted as well to provide a decent volume.
	*/
	m_931a->add_route(0, m_filter_rc[0], 2.0);

	// sub HPF
	FILTER_RC(config, m_filter_rc[0]).add_route(0, m_filter_bq[0], 1.0);
	FILTER_BIQUAD(config, m_filter_bq[0]).add_route(0, m_filter_rc[1], 1.0);

	// sub LPF
	FILTER_RC(config, m_filter_rc[1]).add_route(0, m_filter_bq[1], 1.0);
	FILTER_BIQUAD(config, m_filter_bq[1]).add_route(0, "filter_ac0", 1.0);

	FILTER_RC(config, "filter_ac0").set_ac().add_route(0, m_mixer, 1.0);

	// 931 B - main (vowel) waveform
	UPD931(config, m_931b, m_maincpu->clock());
	m_931b->sync_cb().set(m_931a, FUNC(upd931_device::sync_w));
	m_931b->filter_cb().set(FUNC(ct8000_state::filter_main_w));
	m_931b->add_route(0, m_filter_rc[2], 2.0);

	// main LPF
	FILTER_RC(config, m_filter_rc[2]).add_route(0, m_filter_bq[2], 1.0);
	FILTER_BIQUAD(config, m_filter_bq[2]).add_route(0, "filter_ac1", 1.0);

	FILTER_RC(config, "filter_ac1").set_ac().add_route(0, m_mixer, 1.0);

	MN3207(config, m_bbd);
	m_mixer->add_route(0, m_bbd, 1.0);
	m_bbd->add_route(ALL_OUTPUTS, "chorus", 0.5);

	auto &bbd_mixer = MIXER(config, "chorus");
	bbd_mixer.add_route(0, "speaker", 0.4, 0);
	bbd_mixer.add_route(0, "speaker", -0.4, 1);
}

//**************************************************************************
void ct8000_state::ct8000(machine_config &config)
{
	ctmb1(config);
	config.set_default_layout(layout_ct8000);
}

//**************************************************************************
void ct8000_state::ctfk1(machine_config &config)
{
	config_base(config);
	m_io[0]->p4_out_cb().set_nop(); // no switchable clock

	// valid program/patch numbers on FK-1 start at 1 instead of 0 for some reason
	m_midi->set_base_program(1);

	SPEAKER(config, "speaker").front_center();

	MIXER(config, m_mixer).add_route(0, "speaker", 1.0);

	UPD931(config, m_931a, m_maincpu->clock());
	m_931a->filter_cb().set(FUNC(ct8000_state::filter_bass_w));
	m_931a->add_route(0, m_filter_rc[0], 4.0); // boost volume even more since the FK-1 is monophonic

	FILTER_RC(config, m_filter_rc[0]).add_route(0, m_filter_bq[0], 1.0);
	FILTER_BIQUAD(config, m_filter_bq[0]).add_route(0, "filter_ac", 1.0);

	FILTER_RC(config, "filter_ac").set_ac().add_route(0, m_mixer, 1.0);

	config.set_default_layout(layout_ctfk1);
}

//**************************************************************************
void ct8000_state::driver_start()
{
	m_bbd_timer = timer_alloc(FUNC(ct8000_state::bbd_tick), this);

	m_bank->configure_entries(0, 2, memregion("bankrom")->base(), 0x800);

	m_key_select = 0xffff;
	m_key_enable = 0;
	m_switch = 0;

	m_pll_counter[0] = m_pll_counter[1] = 0;
	m_pll_ref[0] = m_pll_ref[1] = 0;

	m_clock_select = m_clock_div = 0;

	save_item(NAME(m_key_select));
	save_item(NAME(m_key_enable));
	save_item(NAME(m_switch));

	save_item(NAME(m_pll_counter));
	save_item(NAME(m_pll_ref));

	save_item(NAME(m_clock_select));
	save_item(NAME(m_clock_div));
}

void ct8000_state::driver_reset()
{
	bbd_setup_next_tick();	
}

//**************************************************************************
void ct8000_state::p1_w(u8 data)
{
	// bit 0-3: 931 data
	// bit 4: 931 strobe 1 (I1)
	// bit 5: 931 strobe 2 (I2)
	// bit 6: 931 B select (ct8000) / 931 select (fk1)
	// bit 7: 931 A select (ct8000) / unused (fk1)
	m_931a->db_w(data & 0xf);
	m_931a->i1_w(BIT(data, 4));
	m_931a->i2_w(BIT(data, 5));

	if (m_931b)
	{
		m_931b->db_w(data & 0xf);
		m_931b->i1_w(BIT(data, 4));
		m_931b->i2_w(BIT(data, 5));
		m_931b->i3_w(BIT(data, 6));
		m_931a->i3_w(BIT(data, 7));
	}
	else
	{
		m_931a->i3_w(BIT(data, 6));
	}
}

//**************************************************************************
u8 ct8000_state::p1_r()
{
	u8 status = m_931a->db_r();
	if (m_931b)
		status &= m_931b->db_r();

	return 0xf0 | status;
}

//**************************************************************************
void ct8000_state::p2_w(u8 data)
{
	m_io[0]->p2_w(data & 0xf);
	m_io[1]->p2_w(data & 0xf);
	m_io[0]->cs_w(BIT(data, 4));
	m_io[1]->cs_w(BIT(data, 6));
	m_bank->set_entry(BIT(data, 7));
}

//**************************************************************************
u8 ct8000_state::p2_r()
{
	return 0xf0 | (m_io[0]->p2_r() & m_io[1]->p2_r());
}

//**************************************************************************
void ct8000_state::p4a_w(u8 data)
{
	// bit 0: master 931 clock select (1 = master VCO, 0 = slave VCO)
	// bit 1-2: master 931 clock divider (3, 6, 12, 4)
	// bit 3: 931 reset
	m_clock_select = BIT(~data, 0);
	m_clock_div = BIT(data, 1, 2);

	update_clocks();
}

//**************************************************************************
void ct8000_state::p5a_w(u8 data)
{
	// key matrix out bits 4-7
	m_key_select &= 0xff0f;
	m_key_select |= (data << 4);
}

//**************************************************************************
void ct8000_state::p6a_w(u8 data)
{
	// bit 0: unused
	// bit 1: key matrix in enable (active low)
	// bit 2-3: key matrix out bits 8-9
	m_key_enable = BIT(~data, 1);
	m_key_select &= 0xfcff;
	m_key_select |= (BIT(data, 2, 2) << 8);
}

//**************************************************************************
void ct8000_state::p7a_w(u8 data)
{
	// key matrix out bits 0-3
	m_key_select &= 0xfff0;
	m_key_select |= data;
}

//**************************************************************************
void ct8000_state::p4b_w(u8 data)
{
	// bit 0: data out strobe to RC-1
	// bit 1: data in ack to RC-1 (active low)
	// bit 2: stereo chorus enable (active low)
	// bit 3: stereo chorus enable for line in (active low)
	m_midi->strobe_w(BIT(data, 0));
	m_midi->ack_w(BIT(data, 1));

	if (m_bbd)
		m_bbd->set_input_gain(0, BIT(data, 2) ? 0 : 1.0);
}

//**************************************************************************
void ct8000_state::p7b_w(u8 data)
{
	// bit 0-2: key matrix out bits 10-12
	// bit 3: mute
	m_key_select &= 0xe3ff;
	m_key_select |= ((data & 0x7) << 10);

	// mute is applied to mixed/filtered 931 output before the stereo chorus
	const double gain = BIT(data, 3) ? 1.0 : 0.0;
	m_mixer->set_input_gain(0, gain);
	m_mixer->set_input_gain(1, gain);
}

//**************************************************************************
u8 ct8000_state::keys_r()
{
	u8 data = 0xff;

	if (m_key_enable)
	{
		for (int i = 0; i < m_inputs.size(); i++)
			if (!BIT(m_key_select, i))
				data &= m_inputs[i].read_safe(0xff);
	}

	return data;
}

//**************************************************************************
INPUT_CHANGED_MEMBER(ct8000_state::switch_w)
{
	if (!oldval && newval)
		m_switch |= param;
}

//**************************************************************************
INPUT_CHANGED_MEMBER(ct8000_state::switch_clear_w)
{
	if (!oldval && newval)
		m_switch = 0;
}

//**************************************************************************
void ct8000_state::filter_main_w(u8 data)
{
	// bit 0-1 = main LPF cutoff
	// bit 2-3 = absolute gain
	double r, c;
	if (BIT(data, 1))
	{
		r = RES_K(68);
		c = CAP_P(470) + BIT(data, 0) ? CAP_P(330) : 0;
	}
	else
	{
		r = RES_K(43);
		c = CAP_P(390);
	}

	m_filter_rc[2]->filter_rc_set_RC(filter_rc_device::LOWPASS, r, 0, 0, CAP_N(2.2));
	m_filter_bq[2]->opamp_sk_lowpass_modify(r, r, RES_M(999.99), RES_R(0.001), CAP_N(5.6), c);

	static const double gain[] = { 3.79, 2.82, 1.94, 1 };
	m_mixer->set_output_gain(0, gain[BIT(data, 2, 2)]);
}

//**************************************************************************
void ct8000_state::filter_sub_w(u8 data)
{
	// bit 0 = sub HPF enable
	// bit 1 = sub LPF cutoff
	// bit 2 = sub LPF disable
	// bit 3 = consonant gain
	if (BIT(data, 0))
	{
		const double c = CAP_N(2.2);
		m_filter_rc[0]->filter_rc_set_RC(filter_rc_device::HIGHPASS, RES_K(18), 0, 0, c);
		m_filter_bq[0]->opamp_sk_highpass_modify(RES_K(6.8), RES_K(220), RES_M(999.99), RES_R(0.001), c, c);
	}
	else
	{
		m_filter_rc[0]->filter_rc_set_RC(filter_rc_device::HIGHPASS, 0, 0, 0, 0);
		m_filter_bq[0]->modify_raw(0, 0, 1, 0, 0);
	}

	if (BIT(data, 2))
	{
		m_filter_rc[1]->filter_rc_set_RC(filter_rc_device::LOWPASS, 0, 0, 0, 0);
		m_filter_bq[1]->modify_raw(0, 0, 1, 0, 0);
	}
	else
	{
		const double r = BIT(data, 1) ? RES_K(22) : RES_K(56);
		const double c = BIT(data, 1) ? CAP_P(430) : CAP_P(330);

		m_filter_rc[1]->filter_rc_set_RC(filter_rc_device::LOWPASS, r, 0, 0, CAP_N(2.2));
		m_filter_bq[1]->opamp_sk_lowpass_modify(r, r, RES_M(999.99), RES_R(0.001), CAP_N(5.6), c);
	}

	m_filter_bq[1]->set_output_gain(0, BIT(data, 3) ? 0.468 : 1);
}

//**************************************************************************
void ct8000_state::filter_bass_w(u8 data)
{
	// bit 0 = LPF cutoff
	// bit 1 = LPF enable (seems to be incorrectly inverted in the schematic)
	// bit 2-3 = gain

	if (BIT(data, 1))
	{
		const double r = RES_K(12);
		const double c = CAP_N(3.3) + BIT(data, 0) ? CAP_N(10) : 0;

		m_filter_rc[0]->filter_rc_set_RC(filter_rc_device::LOWPASS, r, 0, 0, CAP_N(22));
		m_filter_bq[0]->opamp_sk_lowpass_modify(r, r, RES_M(999.99), RES_R(0.001), CAP_N(56), c);
	}
	else
	{
		m_filter_rc[0]->filter_rc_set_RC(filter_rc_device::LOWPASS, 0, 0, 0, 0);
		m_filter_bq[0]->modify_raw(0, 0, 1, 0, 0);
	}

	static const double gain[] = { 3.79, 2.82, 1.94, 1 };
	m_mixer->set_output_gain(ALL_OUTPUTS, gain[BIT(data, 2, 2)]);
}

//**************************************************************************
void ct8000_state::pll_w(offs_t offset, u8 data)
{
	data &= 0xf;

	for (int i = 0; i < 2; i++)
	{
		if (!BIT(offset, 3 + i))
		{
			switch (offset & 7)
			{
			case 0:
				m_pll_counter[i] &= 0x3ff0;
				m_pll_counter[i] |= data;
				break;

			case 1:
				m_pll_counter[i] &= 0x3f0f;
				m_pll_counter[i] |= (data << 4);
				break;

			case 2:
				m_pll_counter[i] &= 0x30ff;
				m_pll_counter[i] |= (data << 8);
				break;

			case 3:
				m_pll_counter[i] &= 0x0fff;
				m_pll_counter[i] |= ((data & 3) << 12);
				break;

			case 4:
				m_pll_ref[i] &= 0xff0;
				m_pll_ref[i] |= data;
				break;

			case 5:
				m_pll_ref[i] &= 0xf0f;
				m_pll_ref[i] |= (data << 4);
				break;

			case 6:
				m_pll_ref[i] &= 0x0ff;
				m_pll_ref[i] |= (data << 8);
				break;

			default:
				break;
			}
		}
	}

	update_clocks();
}

//**************************************************************************
void ct8000_state::update_clocks()
{
	double clock_scale[2] = { 1.0, 1.0 };

	// master VCO freq (controls 931 B)
	const int sel = m_clock_select & 1;
	if (m_931b && m_pll_counter[sel] && m_pll_ref[sel])
	{
		// PLLs are tuned so that increasing/decreasing the counter by 1 raises or lowers pitch by ~1.5 cents
		clock_scale[0] = (2.0 * m_pll_counter[sel]) / m_pll_ref[sel];

		switch (m_clock_div & 3)
		{
		case 0: clock_scale[0] /= 3; break;
		case 1: clock_scale[0] /= 6; break;
		case 2: clock_scale[0] /= 12; break;
		case 3: clock_scale[0] /= 4; break;
		}

		m_931b->set_clock_scale(clock_scale[0]);
	}

	// slave VCO freq (controls 931 A and CPU)
	if (m_pll_counter[1] && m_pll_ref[1])
	{
		clock_scale[1] = (2.0 * m_pll_counter[1]) / (3 * m_pll_ref[1]);

		m_maincpu->set_clock_scale(clock_scale[1]);
		m_931a->set_clock_scale(clock_scale[1]);
	}

	LOGMASKED(LOG_VCO, "VCO #1 %.3f MHz, #2 %.3f MHz\n",
		clock_scale[0] * m_maincpu->unscaled_clock() / 1'000'000,
		clock_scale[1] * m_maincpu->unscaled_clock() / 1'000'000);
}

//**************************************************************************
TIMER_CALLBACK_MEMBER(ct8000_state::bbd_tick)
{
	m_bbd->tick();
	bbd_setup_next_tick();
}

void ct8000_state::bbd_setup_next_tick()
{
	// 62.5 to 80 kHz, varies at 0.6666... Hz
	double pos = machine().time().as_double() / 1.5;
	pos -= std::floor(pos);
	pos = (pos < 0.5) ? (2 * pos) : 2 * (1.0 - pos);
	const double bbd_freq = 62500 + (80000 - 62500) * pos;

	m_bbd_timer->adjust(attotime::from_ticks(1, bbd_freq));
}


INPUT_PORTS_START(ct8000)
	PORT_START("KC0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E2")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F2")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#2")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B2")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C3")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F3")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#3")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B3")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C4")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#4")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F4")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#4")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G4")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B4")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C5")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#5")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D5")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F5")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#5")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G5")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#5")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B5")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C6")
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Sustain Pedal")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tune Up")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tune Down")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Stereo Chorus") PORT_TOGGLE
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KCA")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(ct8000_state::switch_r))
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KCB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tone Memory 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tone Memory 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tone Memory 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tone Memory 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tone Set") PORT_TOGGLE
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KCC")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vibrato") PORT_TOGGLE
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SWITCH")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Effect Cancel") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ct8000_state::switch_clear_w), 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Sustain")       PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ct8000_state::switch_w), 0x1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Reverb")        PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ct8000_state::switch_w), 0x4)
INPUT_PORTS_END

INPUT_PORTS_START(ctfk1)
	PORT_START("KC0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#1")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E1")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F1")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A1")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#1")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B1")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E2")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F2")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tune Up")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tune Down")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bass Select") PORT_TOGGLE
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KCA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Sustain") PORT_TOGGLE
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KCB")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) // KI0->KCB diode
	PORT_BIT(0x0e, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Tone Set") PORT_TOGGLE
	PORT_BIT(0xe0, IP_ACTIVE_LOW,  IPT_UNUSED)
INPUT_PORTS_END

INPUT_PORTS_START(ctmb1)
	PORT_START("KC0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Reverse")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Play")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Forward")
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keyboard Synchro")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Record")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Reset")
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Pause")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Demonstration")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Number Select")
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tune Up")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tune Down")
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


ROM_START(ct8000)
	ROM_REGION(0x800, "maincpu", 0)
	ROM_LOAD("upd8049c-364.bin", 0x000, 0x800, CRC(8c9b3f40) SHA1(5507117671300fe350df39b4ca70239079a1b0e7))

	ROM_REGION(0x1000, "bankrom", 0)
	ROM_LOAD("ct8000_2732.bin", 0x0000, 0x1000, CRC(e6f1f3f9) SHA1(637a9d2b6be2f12240724c8aa5744c5f28ac4af0))
ROM_END

ROM_START(ctfk1)
	ROM_REGION(0x800, "maincpu", 0)
	ROM_LOAD("upd8049c-364.bin", 0x000, 0x800, CRC(8c9b3f40) SHA1(5507117671300fe350df39b4ca70239079a1b0e7))

	ROM_REGION(0x1000, "bankrom", 0)
	ROM_LOAD("fk1_2732.bin", 0x0000, 0x1000, CRC(f7416356) SHA1(6d92aa5f377769327451334324f1f32b52951968))
ROM_END

ROM_START(ctmb1)
	ROM_REGION(0x800, "maincpu", 0)
	ROM_LOAD("upd8049c-364.bin", 0x000, 0x800, CRC(8c9b3f40) SHA1(5507117671300fe350df39b4ca70239079a1b0e7))

	ROM_REGION(0x1000, "bankrom", 0)
	ROM_LOAD("mb1_2732.bin", 0x0000, 0x1000, CRC(fc128110) SHA1(8ff31fdccb8200836cf414bc709d5b9a68279205))
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME           FLAGS
SYST( 1983, ct8000,  0,      0,      ct8000,  ct8000,  ct8000_state,  empty_init, "Casio", "Casiotone 8000",  MACHINE_SUPPORTS_SAVE )
SYST( 1983, ctfk1,   0,      0,      ctfk1,   ctfk1,   ct8000_state,  empty_init, "Casio", "Casiotone FK-1",  MACHINE_SUPPORTS_SAVE )
SYST( 1983, ctmb1,   0,      0,      ctmb1,   ctmb1,   ct8000_state,  empty_init, "Casio", "Casiotone MB-1",  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
