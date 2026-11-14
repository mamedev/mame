// license:BSD-3-Clause
// copyright-holders:Devin Acker
/*
    Casio DH-100 wind controller

    This electronic wind instrument is both a MIDI controller and a simple monophonic synth,
    which works by passing the uPD78C11's timer/counter output through switchable RC filters
    and a VCA/VCF section, which is controlled by either the breath sensor or a fixed envelope
    (if the breath mode switch is turned off).

    A test mode (not detailed in the service manual) can be enabled by holding keys 3, 4, and 6
    (counted from the top of the instrument, not including the portamento or sharp keys) while
    turning the instrument on. This performs a RAM test, then plays a short test tone.
    After that, pressing the note keys (with breath mode disabled) plays a volume/filter sweep,
    other buttons play another test tone, and pressing the transpose key turns the power off.

    TODO:
    - dump and add the other models
    - layouts?
    - netlist sound accuracy/quality issues (see notes in nl_dh100.cpp)
*/

#include "emu.h"

#include "nl_dh100.h"
#include "bus/midi/midioutport.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/netlist.h"
#include "sound.h"
#include "speaker.h"

namespace {

class dh100_state : public driver_device
{
public:
	dh100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_netlist(*this, "sound_nl")
		, m_dac(*this, "sound_nl:dac")
		, m_vcf(*this, "sound_nl:vcf%u", 1)
		, m_led_power(*this, "led_power")
	{ }

	void dh100(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void envelope_w(u8 data);
	void apo_w(int state);

	required_device<upd78c11_device> m_maincpu;
	required_device<netlist_mame_sound_device> m_netlist;
	required_device<netlist_mame_int_input_device> m_dac;
	required_device_array<netlist_mame_analog_input_device, 4> m_vcf;
	output_finder<> m_led_power;
};


/**************************************************************************/
static INPUT_PORTS_START(dh100)
	PORT_START("PB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Key 7")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Key 6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Key 5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Key 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Sharp")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Key 3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Key 2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Key 1")

	PORT_START("PC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_OTHER  ) PORT_NAME("Octave Up")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) // APO
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER  ) PORT_NAME("Breath Mode") PORT_TOGGLE
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("AN0")
	// minimum level is checked on boot for calibration, but normally sits at around 1V
	// maximum recognized by firmware is (min + 0x64), higher values are clamped
	PORT_BIT( 0xff, 0x30, IPT_PEDAL ) PORT_NAME("Breath Sensor") PORT_MINMAX(0x30, 0x94) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("AN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Transpose")

	PORT_START("AN5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Tone Select")

	PORT_START("AN7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Portamento")

	// AN6 is used as a vibrato switch, but is normally disconnected
	PORT_START("EXTRA")
	PORT_CONFNAME( 0x1, 0x0, "Enable Vibrato Switch" )
	PORT_CONFSETTING(   0x0, DEF_STR(No) )
	PORT_CONFSETTING(   0x1, DEF_STR(Yes) )

	PORT_START("AN6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_UNUSED ) PORT_CONDITION("EXTRA", 0x1, EQUALS, 0x0)
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("Vibrato") PORT_TOGGLE PORT_CONDITION("EXTRA", 0x1, EQUALS, 0x1)

	// TODO: add the filter offset trimmer (VR3) when filter JFETs are modeled in the netlist
INPUT_PORTS_END


/**************************************************************************/
void dh100_state::dh100(machine_config &config)
{
	UPD78C11(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->pa_out_cb().set(FUNC(dh100_state::envelope_w));
	m_maincpu->pb_in_cb().set_ioport("PB");
	m_maincpu->pc_in_cb().set_ioport("PC");
	m_maincpu->pc_out_cb().set(FUNC(dh100_state::apo_w)).bit(4);
	m_maincpu->an0_func().set_ioport("AN0");
	m_maincpu->an3_func().set_ioport("AN0"); // AN0/AN3 are wired together and averaged
	m_maincpu->an4_func().set_ioport("AN4");
	m_maincpu->an5_func().set_ioport("AN5");
	m_maincpu->an6_func().set_ioport("AN6");
	m_maincpu->an7_func().set_ioport("AN7");

	auto &mdout(MIDI_PORT(config, "mdout", midiout_slot, "midiout"));
	m_maincpu->txd_func().set(mdout, FUNC(midi_port_device::write_txd));

	SPEAKER(config, "speaker", 1).front_center();

	NETLIST_SOUND(config, m_netlist, 48000).set_source(NETLIST_NAME(dh100))
		.add_route(ALL_OUTPUTS, "speaker", 1.0);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:output", 0, "OUTPUT");

	// R-2R DAC for VCA and VCF
	NETLIST_INT_INPUT(config, m_dac, "DAC.VAL", 0, 255);

	// VCF filter stages currently modeled using variable resistors instead of JFETs
	NETLIST_ANALOG_INPUT(config, m_vcf[0], "T1.DIAL").set_mult_offset(1.0 / 255, 0);
	NETLIST_ANALOG_INPUT(config, m_vcf[1], "T2.DIAL").set_mult_offset(1.0 / 255, 0);
	NETLIST_ANALOG_INPUT(config, m_vcf[2], "T3.DIAL").set_mult_offset(1.0 / 255, 0);
	NETLIST_ANALOG_INPUT(config, m_vcf[3], "T4.DIAL").set_mult_offset(1.0 / 255, 0);

	// square wave outputs
	auto &pc6(NETLIST_LOGIC_INPUT(config, "sound_nl:pc6", "PC6.IN", 0));
	auto &pc7(NETLIST_LOGIC_INPUT(config, "sound_nl:pc7", "PC7.IN", 0));
	m_maincpu->co0_func().set(pc6, FUNC(netlist_mame_logic_input_device::write));
	m_maincpu->co1_func().set(pc7, FUNC(netlist_mame_logic_input_device::write));

	// tone switches
	auto &pf4(NETLIST_LOGIC_INPUT(config, "sound_nl:pf4", "PF4.IN", 0));
	auto &pf5(NETLIST_LOGIC_INPUT(config, "sound_nl:pf5", "PF5.IN", 0));
	auto &pf6(NETLIST_LOGIC_INPUT(config, "sound_nl:pf6", "PF6.IN", 0));
	auto &pf7(NETLIST_LOGIC_INPUT(config, "sound_nl:pf7", "PF7.IN", 0));
	m_maincpu->pf_out_cb().set(pf4, FUNC(netlist_mame_logic_input_device::write)).bit(4);
	m_maincpu->pf_out_cb().append(pf5, FUNC(netlist_mame_logic_input_device::write)).bit(5);
	m_maincpu->pf_out_cb().append(pf6, FUNC(netlist_mame_logic_input_device::write)).bit(6);
	m_maincpu->pf_out_cb().append(pf7, FUNC(netlist_mame_logic_input_device::write)).bit(7);
}

/**************************************************************************/
void dh100_state::machine_start()
{
	apo_w(0);
	// keep CPU in reset for long enough to avoid turning on the amp before the netlist output voltage settles
	m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::from_msec(250));
}

/**************************************************************************/
void dh100_state::envelope_w(u8 data)
{
	m_dac->write8(data);
	for (auto &vcf : m_vcf)
		vcf->write8(data);
}

/**************************************************************************/
void dh100_state::apo_w(int state)
{
	m_led_power = state;
	m_netlist->set_output_gain(0, state ? 1.0 : 0.0);
}



/**************************************************************************/
ROM_START( dh100 )
	ROM_REGION(0x1000, "maincpu", 0)
	// another CPU revision labeled CM511 also exists, see https://www.polynominal.com/repair/casio-dh100/
	ROM_LOAD( "cm505.ic1", 0x0000, 0x1000, CRC(70248b45) SHA1(cbef1f797e5df71507ba6002e305299890468aa7) )
ROM_END

} // anonymous namespace

SYST( 1988, dh100, 0,    0, dh100, dh100, dh100_state, empty_init, "Casio", "DH-100 Digital Horn", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
