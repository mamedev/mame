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


namespace {

class microkorg_state : public driver_device
{
public:
	microkorg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void microkorg(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h8s2320_device> m_maincpu;
};


void microkorg_state::mem_map(address_map &map)
{
	map(0x000000, 0x0fffff).rw("flash", FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x400000, 0x47ffff).ram();
}


static INPUT_PORTS_START(microkorg)
INPUT_PORTS_END

void microkorg_state::microkorg(machine_config &config)
{
	H8S2320(config, m_maincpu, 10_MHz_XTAL); // HD6412320VF25 (or HD6412324SVF25)
	m_maincpu->set_addrmap(AS_PROGRAM, &microkorg_state::mem_map);
	//m_maincpu->write_portf().set("dsp", FUNC(dsp56362_device::ss_w)).bit(1);
	//m_maincpu->read_adc<0>().set_ioport("BENDER");
	//m_maincpu->read_adc<1>().set_ioport("MODWHEEL");
	m_maincpu->read_adc<2>().set_constant(0x3ff); // battery voltage detector
	//m_maincpu->read_adc<3>().set_ioport("KNOB1");
	//m_maincpu->read_adc<4>().set_ioport("KNOB2");
	//m_maincpu->read_adc<5>().set_ioport("KNOB3");
	//m_maincpu->read_adc<6>().set_ioport("KNOB4");
	//m_maincpu->read_adc<7>().set_ioport("KNOB5");
	//m_maincpu->write_sci_tx<0>().set("dsp", FUNC(dsp56362_device::mosi_w));
	//m_maincpu->write_sci_clk<0>().set("dsp", FUNC(dsp56362_device::sck_w)).invert();
	m_maincpu->write_sci_tx<1>().set("mdout", FUNC(midi_port_device::write_txd));

	FUJITSU_29LV800B(config, "flash"); // MBM29LV800BA-90PFTN

	dsp56362_device &dsp(DSP56362(config, "dsp", 12.288_MHz_XTAL / 4)); // DSPB56362PV100
	dsp.set_hard_omr(0b0101); // slave SPI bootstrap
	//dsp.miso_callback().set(m_maincpu, FUNC(h8s2320_device::sci_rx_w<0>));
	//dsp.hreq_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ7);

	//AK4522(config, "codec", 12.288_MHz_XTAL); // AK4522VF

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_maincpu, FUNC(h8s2320_device::sci_rx_w<1>));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
}

ROM_START(microkorg)
	ROM_REGION16_LE(0x100000, "flash", 0)
	ROM_LOAD("korg_microkorg_v1.03_29lv800b.ic20", 0x000000, 0x100000, CRC(607ada7e) SHA1(4a6e2f4068cac7493484af2a8c1d1db7d8bd7a17))
ROM_END

} // anonymous namespace


SYST(2002, microkorg, 0, 0, microkorg, microkorg, microkorg_state, empty_init, "Korg", "microKORG Synthesizer/Vocoder", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
