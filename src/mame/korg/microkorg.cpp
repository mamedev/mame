// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Korg microKORG compact synthesizer/vocoder.

    The MS2000 Analog Modeling Synthesizer runs on similar hardware.

****************************************************************************/

#include "emu.h"
#include "bus/midi/midi.h"
#include "cpu/h8/h8s2329.h"
#include "cpu/dsp563xx/dsp56362.h"
#include "machine/intelfsh.h"
#include "machine/timer.h"
#include "microkorg.lh"


namespace {

class microkorg_state : public driver_device
{
public:
	microkorg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dsp(*this, "dsp")
		, m_ioport_p7(*this, "P7")
		, m_ioport_p6(*this, "P6")
		, m_ioport_p5(*this, "P5")
		, m_ioport_p4(*this, "P4")
		, m_ioport_p3(*this, "P3")
		, m_disp(*this, "disp_%u", 0U)
		, m_led_timbre_1(*this, "led_timbre_1")
		, m_led_timbre_2(*this, "led_timbre_2")
		, m_led_edit_1_synth(*this, "led_edit_1_synth")
		, m_led_edit_2_synth(*this, "led_edit_2_synth")
		, m_led_edit_1_vocoder(*this, "led_edit_1_vocoder")
		, m_led_edit_2_vocoder(*this, "led_edit_2_vocoder")
		, m_led_tempo(*this, "led_tempo")
		, m_led_arpeggiator(*this, "led_arpeggiator")
		, m_led_side(*this, "led_side")
		, m_led_write(*this, "led_write")
		, m_led_shift(*this, "led_shift")
		, m_led_original_value(*this, "led_original_value")
		, m_led_digit(*this, "led_digit_%u", 1U)
		, m_led_octave_up_red(*this, "led_octave_up_red")
		, m_led_octave_up_green(*this, "led_octave_up_green")
		, m_led_octave_down_red(*this, "led_octave_down_red")
		, m_led_octave_down_green(*this, "led_octave_down_green")
	{
	}

	void microkorg(machine_config &config);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void port1_w(u8 data);
	void port2_w(u8 data);
	u8 port6_r();
	u8 port5_r();
	void portf_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(commit_leds);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h8s2320_device> m_maincpu;
	required_device<dsp56362_device> m_dsp;

	required_ioport m_ioport_p7;
	required_ioport m_ioport_p6;
	required_ioport m_ioport_p5;
	required_ioport m_ioport_p4;
	required_ioport m_ioport_p3;

	u8 m_port1, m_portf;
	u8 m_led_state[8];

	output_finder<3> m_disp;    // 3-digit+dot display, 8-bit segment masks (bit 7 = dot)

	// single-color LEDs (0 = off, 1 = on)
	output_finder<> m_led_timbre_1;
	output_finder<> m_led_timbre_2;
	output_finder<> m_led_edit_1_synth;
	output_finder<> m_led_edit_2_synth;
	output_finder<> m_led_edit_1_vocoder;
	output_finder<> m_led_edit_2_vocoder;
	output_finder<> m_led_tempo;
	output_finder<> m_led_arpeggiator;
	output_finder<> m_led_side;
	output_finder<> m_led_write;
	output_finder<> m_led_shift;
	output_finder<> m_led_original_value;
	output_finder<8> m_led_digit;

	// bicolor LEDs, MSGB39W 3-pin common-cathode: one output per die (0 = off, 1 = on)
	output_finder<> m_led_octave_up_red;      // pin 3 (+red/orange)
	output_finder<> m_led_octave_up_green;    // pin 1 (+green)
	output_finder<> m_led_octave_down_red;    // pin 3 (+red/orange)
	output_finder<> m_led_octave_down_green;  // pin 1 (+green)

	u16 get_input() const;
};

void microkorg_state::machine_start()
{
	save_item(NAME(m_port1));
	save_item(NAME(m_portf));
	save_item(NAME(m_led_state));
}

void microkorg_state::machine_reset()
{
	m_port1 = 0;
	m_portf = 0;
	memset(m_led_state, 0, sizeof(m_led_state));
}

void microkorg_state::mem_map(address_map &map)
{
	map(0x000000, 0x0fffff).rw("flash", FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x400000, 0x47ffff).ram();
}

void microkorg_state::port1_w(u8 data)
{
	// bit 0-3 are selection bit a-d for user interface
	// bit 4 is mute
	// bit 5-7 are adsel
	m_port1 = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(microkorg_state::commit_leds)
{
	for(int idx=0; idx != 7; idx++) {
		u8 n = m_led_state[idx];
		m_led_state[idx] = 0;
		switch(idx) {
		case 0:
			for(int i=0; i != 8; i++)
				m_led_digit[i] = BIT(n, i);
			break;
		case 1:
			m_led_tempo          = BIT(n, 0);
			m_led_arpeggiator    = BIT(n, 1);
			m_led_side           = BIT(n, 2);
			m_led_write          = BIT(n, 4);
			m_led_shift          = BIT(n, 5);
			m_led_original_value = BIT(n, 7);
			break;
		case 2:
			m_disp[2] = bitswap(n, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 3:
			m_disp[1] = bitswap(n, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 4:
			m_disp[0] = bitswap(n, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 5:
			m_led_timbre_1        = BIT(n, 0);
			m_led_timbre_2        = BIT(n, 1);
			m_led_edit_1_synth    = BIT(n, 2);
			m_led_edit_2_synth    = BIT(n, 3);
			m_led_edit_1_vocoder  = BIT(n, 4);
			m_led_edit_2_vocoder  = BIT(n, 5);
			m_led_octave_up_red   = BIT(n, 6);
			m_led_octave_down_red = BIT(n, 7);
			break;
		case 6:
			m_led_octave_up_green   = BIT(n, 6);
			m_led_octave_down_green = BIT(n, 7);
			break;
		case 7:
			break;
		}
	}
}

void microkorg_state::port2_w(u8 data)
{
	// Technically, the write itself is not detected.  Plus, it's
	// weird.  But we get the correct result by pseudo-filtering a tad
	m_led_state[BIT(m_port1, 5, 3)] |= data;
}

void microkorg_state::portf_w(u8 data)
{
	// bit 0 is selection bit e for user interface
	// bit 2 is connected to dsp 'ss' pin
	m_portf = data;

	m_dsp->ss_w(BIT(data, 1));
}

u8 microkorg_state::port6_r()
{
	// Return information from the user interface, bits 0-7
	return get_input();
}

u8 microkorg_state::port5_r()
{
	// Return information from the user interface, bits 8-10
	return (get_input() >> 8) | 0xf8;
}

u16 microkorg_state::get_input() const
{
	if(BIT(m_portf, 0)) {
		// Panel inputs
		switch(BIT(m_port1, 0, 3)) {
		case 0: case 1: case 2:
			return 0x7ff;
		case 3:
			return m_ioport_p3->read();
		case 4:
			return m_ioport_p4->read();
		case 5:
			return m_ioport_p5->read();
		case 6:
			return m_ioport_p6->read();
		case 7:
			return m_ioport_p7->read();
		}
	}
	return 0x7ff;
}

static INPUT_PORTS_START(microkorg)
	PORT_START("P7")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x700, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P6")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Shift")                          PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank selection")                 PORT_CODE(KEYCODE_B)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Timbre selection / Format hold") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Write")                          PORT_CODE(KEYCODE_W)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Octave up")                      PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Octave down")                    PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Arpeggiator")                    PORT_CODE(KEYCODE_A)
	PORT_BIT(0x780, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P5")
	PORT_CONFNAME(0x0ff, 0x001, "Bank select")
	PORT_CONFSETTING(    0x001, "Trance")
	PORT_CONFSETTING(    0x002, "Techno/House")
	PORT_CONFSETTING(    0x004, "Electronica")
	PORT_CONFSETTING(    0x008, "D'n'B/Breaks")
	PORT_CONFSETTING(    0x010, "Hiphop/Vintage")
	PORT_CONFSETTING(    0x020, "Retro")
	PORT_CONFSETTING(    0x040, "S.E./Hit")
	PORT_CONFSETTING(    0x080, "Vocoder")
	PORT_BIT(0x700, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P4")
	PORT_CONFNAME(0x7ff, 0x001, "Edit 1")
	PORT_CONFSETTING(    0x001, "Voice")
	PORT_CONFSETTING(    0x002, "Pitch")
	PORT_CONFSETTING(    0x004, "OSC 1")
	PORT_CONFSETTING(    0x008, "OSC 2/Audio in 1")
	PORT_CONFSETTING(    0x010, "Mixer")
	PORT_CONFSETTING(    0x020, "Filter")
	PORT_CONFSETTING(    0x040, "Filter EG/FC mod")
	PORT_CONFSETTING(    0x080, "Amp")
	PORT_CONFSETTING(    0x100, "Amp EG")
	PORT_CONFSETTING(    0x200, "LFO 1")
	PORT_CONFSETTING(    0x400, "LFO 2")

	PORT_START("P3")
	PORT_CONFNAME(0x7ff, 0x001, "Edit 2")
	PORT_CONFSETTING(    0x001, "Patch 1/Ch level A")
	PORT_CONFSETTING(    0x002, "Patch 2/Ch level B")
	PORT_CONFSETTING(    0x004, "Patch 3/Ch Pan A")
	PORT_CONFSETTING(    0x008, "Patch 4/Ch Pan B")
	PORT_CONFSETTING(    0x010, "Mod FX")
	PORT_CONFSETTING(    0x020, "Delay")
	PORT_CONFSETTING(    0x040, "EQ")
	PORT_CONFSETTING(    0x080, "Arpeg A")
	PORT_CONFSETTING(    0x100, "Arpeg B")
	PORT_CONFSETTING(    0x200, "GLobal")
	PORT_CONFSETTING(    0x400, "Midi")
INPUT_PORTS_END

void microkorg_state::microkorg(machine_config &config)
{
	H8S2320(config, m_maincpu, 10_MHz_XTAL); // HD6412320VF25 (or HD6412324SVF25)
	m_maincpu->set_addrmap(AS_PROGRAM, &microkorg_state::mem_map);
	//m_maincpu->read_adc<0>().set_ioport("BENDER");
	//m_maincpu->read_adc<1>().set_ioport("MODWHEEL");
	m_maincpu->read_adc<2>().set_constant(0x3ff); // battery voltage detector
	//m_maincpu->read_adc<3>().set_ioport("KNOB1");
	//m_maincpu->read_adc<4>().set_ioport("KNOB2");
	//m_maincpu->read_adc<5>().set_ioport("KNOB3");
	//m_maincpu->read_adc<6>().set_ioport("KNOB4");
	//m_maincpu->read_adc<7>().set_ioport("KNOB5");
	m_maincpu->write_sci_tx<0>().set("dsp", FUNC(dsp56362_device::mosi_w));
	m_maincpu->write_sci_clk<0>().set("dsp", FUNC(dsp56362_device::sck_w)).invert();
	m_maincpu->write_sci_tx<1>().set("mdout", FUNC(midi_port_device::write_txd));
	m_maincpu->write_port1().set(FUNC(microkorg_state::port1_w));
	m_maincpu->write_port2().set(FUNC(microkorg_state::port2_w));
	m_maincpu->read_port5().set(FUNC(microkorg_state::port5_r));
	m_maincpu->read_port6().set(FUNC(microkorg_state::port6_r));
	m_maincpu->write_portf().set(FUNC(microkorg_state::portf_w));

	FUJITSU_29LV800B(config, "flash"); // MBM29LV800BA-90PFTN

	dsp56362_device &dsp(DSP56362(config, "dsp", 12.288_MHz_XTAL / 4)); // DSPB56362PV100
	dsp.set_hard_omr(0b0101); // slave SPI bootstrap
	dsp.miso_callback().set(m_maincpu, FUNC(h8s2320_device::sci_rx_w<0>));
	dsp.hreq_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ7);

	//AK4522(config, "codec", 12.288_MHz_XTAL); // AK4522VF

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_maincpu, FUNC(h8s2320_device::sci_rx_w<1>));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	TIMER(config, "leds").configure_periodic(FUNC(microkorg_state::commit_leds), attotime::from_hz(30));

	config.set_default_layout(layout_microkorg);
}

ROM_START(microkorg)
	ROM_REGION16_LE(0x100000, "flash", 0)
	ROM_LOAD("korg_microkorg_v1.03_29lv800b.ic20", 0x000000, 0x100000, CRC(607ada7e) SHA1(4a6e2f4068cac7493484af2a8c1d1db7d8bd7a17))
ROM_END

} // anonymous namespace


SYST(2002, microkorg, 0, 0, microkorg, microkorg, microkorg_state, empty_init, "Korg", "microKORG Synthesizer/Vocoder", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
