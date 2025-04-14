// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "emu.h"
#include "zaccaria_a.h"
#include "nl_zac1b11142.h"

#include "cpu/m6800/m6800.h"
#include "machine/clock.h"
#include "machine/rescap.h"
#include "sound/dac.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ZACCARIA_1B11107, zac1b11107_audio_device, "zac1b11107", "Zaccaria 1B11107 Sound Board")
DEFINE_DEVICE_TYPE(ZACCARIA_1B11142, zac1b11142_audio_device, "zac1b11142", "Zaccaria 1B11142 Sound Board")



//**************************************************************************
//  MEMORY MAPS
//**************************************************************************

/*
    base melody/SFX generator CPU map
    1B11107 and 1B11142 both have a 6802 with internal RAM and a PIA accessed at 0x500c
*/
void zac1b111xx_melody_base::zac1b111xx_melody_base_map(address_map &map)
{
	map.unmap_value_high();
	map(0x400c, 0x400f).mirror(0x1ff0).rw("melodypia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
}


/*
    1B11107 sound CPU, produces music and sound effects
    mapping (from tracing sound program and cross-referencing 1B1142 schematic):
    A15 A14 A13 A12 A11 A10 A09 A08 A07 A06 A05 A04 A03 A02 A01 A00
     0   0   0   0   0   0   0   0   0   *   *   *   *   *   *   *  RW 6802 internal ram
     0   0   0   x   x   x   x   x   x   x   x   x   x   x   x   x  Open bus (for area that doesn't overlap RAM)
     0   0   1   x   x   x   x   x   x   x   x   x   x   x   x   x  Open bus
     0   1   0   x   x   x   x   x   x   x   x   x   0   0   x   x  Open bus
     0   1   0   x   x   x   x   x   x   x   x   x   0   1   x   x  Open bus
     0   1   0   x   x   x   x   x   x   x   x   x   1   0   x   x  Open bus
     0   1   0   x   x   x   x   x   x   x   x   x   1   1   *   *  RW 6821 PIA @ 1G
     0   1   1   x   x   x   x   x   x   x   x   x   x   x   x   x  Open bus
     1   0   x   x   x   x   x   x   x   x   x   x   x   x   x   x  Open bus
     1   1   0   0   *   *   *   *   *   *   *   *   *   *   *   *  R  Enable ROM @ 1F
     1   1   0   1   *   *   *   *   *   *   *   *   *   *   *   *  Open bus
     1   1   1   0   *   *   *   *   *   *   *   *   *   *   *   *  R  Enable ROM @ 1D
     1   1   1   1   *   *   *   *   *   *   *   *   *   *   *   *  R  Enable ROM @ 1E

    6821 PIA:
    * CA1 comes from the SOUND 5 line on the input (which may also be connected to an input on the AY chip at 1H)
    * CB1 comes from the 6802's clock divided by 4096*2 (about 437Hz)
    * PA0-7 connect to the data busses of the AY-3-8910 chips
    * PB0 and PB1 connect to the BC1 and BDIR pins of the AY chip at 1H
    * PB2 and PB3 connect to the BC1 and BDIR pins of the AY chip at 1I
*/
void zac1b11107_audio_device::zac1b11107_melody_map(address_map &map)
{
	zac1b111xx_melody_base_map(map);
	map(0xc000, 0xcfff).rom(); // ROM @ 1F
	map(0xe000, 0xffff).rom(); // ROM @ 1D, 1E
}


/*
    1B11142 slave sound CPU, produces music and sound effects
    mapping:
    A15 A14 A13 A12 A11 A10 A09 A08 A07 A06 A05 A04 A03 A02 A01 A00
     0   0   0   0   0   0   0   0   0   *   *   *   *   *   *   *  RW 6802 internal ram
     0   0   0   x   x   x   x   x   x   x   x   x   x   x   x   x  Open bus (for area that doesn't overlap RAM)
     0   0   1   x   x   x   x   x   x   x   x   x   x   x   x   x  Open bus
     0   1   0   x   x   x   x   x   x   x   x   x   0   0   x   x  Open bus
     0   1   0   x   x   x   x   x   x   x   x   x   0   1   x   x  Open bus
     0   1   0   x   x   x   x   x   x   x   x   x   1   0   x   x  Open bus
     0   1   0   x   x   x   x   x   x   x   x   x   1   1   *   *  RW 6821 PIA @ 4I
     0   1   1   x   x   x   x   x   x   x   x   x   x   x   x   x  Open bus
     1   0   %   %   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS4A: Enable ROM 13
     1   1   %   %   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS5A: Enable ROM 9
     note that the % bits go to pins 2 (6802 A12) and 26 (6802 A13) of the roms
     monymony and jackrabt both use 2764 roms, which use pin 2 as A12 and pin 26 as N/C don't care
     hence for actual chips used, the mem map is:
     1   0   x   *   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS4A: Enable ROM 13
     1   1   x   *   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS5A: Enable ROM 9

    6821 PIA:
    * CA1 comes from the master sound cpu's latch bit 7 (which is also connected to the AY chip at 4G's IOB1)
    * CB1 comes from the 6802's clock divided by 4096*2 (about 437Hz)
    * CA2 and CB2 are not connected
    * PA0-7 connect to the data busses of the AY-3-8910 chips
    * PB0 and PB1 connect to the BC1 and BDIR pins of the AY chip at 4G
    * PB2 and PB3 connect to the BC1 and BDIR pins of the AY chip at 4H
*/
void zac1b11142_audio_device::zac1b11142_melody_map(address_map &map)
{
	zac1b111xx_melody_base_map(map);
	map(0x8000, 0x9fff).mirror(0x2000).rom(); // ROM 13
	map(0xc000, 0xdfff).mirror(0x2000).rom(); // ROM 9
}


/*
   1B11142 master sound CPU, controls DAC and speech directly
   mapping:
   A15 A14 A13 A12 A11 A10 A09 A08 A07 A06 A05 A04 A03 A02 A01 A00
    0   0   0   0   0   0   0   0   0   *   *   *   *   *   *   *  RW 6802 internal ram
    x   0   0   0   x   x   x   x   1   x   x   0   x   x   *   *  Open bus (test mode writes as if there was another PIA here)
    x   0   0   0   x   x   x   x   1   x   x   1   x   x   *   *  RW 6821 PIA @ 1I
    x   0   0   1   0   0   x   x   x   x   x   x   x   x   x   x   W MC1408 DAC
    x   x   0   1   0   1   x   x   x   x   x   x   x   x   x   x   W Command to slave melody cpu
    x   x   0   1   1   0   x   x   x   x   x   x   x   x   x   x  R  Command read latch from z80
    x   x   0   1   1   1   x   x   x   x   x   x   x   x   x   x  Open bus
    %   %   1   0   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS1A: Enable ROM 8
    %   %   1   1   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS0A: Enable ROM 7
    note that the % bits go to pins 2 (6802 A14) and 26 (6802 A15) of the roms
    monymony and jackrabt both use 2764 roms, which use pin 2 as A12 and pin 26 as N/C don't care
    hence for actual chips used, the mem map is:
    x   *   1   0   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS1A: Enable ROM 8
    x   *   1   1   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS0A: Enable ROM 7

   6821 PIA:
   PA0-7, PB0-1, CA2 and CB1 connect to the TMS5200
   CA1 and CB2 are not connected, though the test mode assumes there's something connected to CB2 (possibly another LED like the one connected to PB4)
   PB3 connects to 'ACS' which goes to the Z80
*/
void zac1b11142_audio_device::zac1b11142_audio_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0090, 0x0093).mirror(0x8f6c).rw("pia_1i", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x1000).mirror(0x83ff).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x1400, 0x1400).mirror(0xc3ff).w(FUNC(zac1b11142_audio_device::melody_command_w));
	map(0x1800, 0x1800).mirror(0xc3ff).r(FUNC(zac1b11142_audio_device::host_command_r));
	map(0x2000, 0x2fff).mirror(0x8000).rom(); // ROM 8 with A12 low
	map(0x3000, 0x3fff).mirror(0x8000).rom(); // ROM 7 with A12 low
	map(0x6000, 0x6fff).mirror(0x8000).rom(); // ROM 8 with A12 high
	map(0x7000, 0x7fff).mirror(0x8000).rom(); // ROM 7 with A12 high
}



//**************************************************************************
//  I/O PORT DEFINITIONS
//**************************************************************************

INPUT_PORTS_START(zac1b11142_ioports)
	PORT_START("1B11142")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("P1") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(zac1b11142_audio_device::p1_changed), 0) // test button?  generates NMI on master CPU

	PORT_START("VP1")
	PORT_ADJUSTER( 50, "VP1 - Music volume" )   NETLIST_ANALOG_PORT_CHANGED("sound_nl", "pot1")
	PORT_START("VP2")
	PORT_ADJUSTER( 50, "VP2 - Speech volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "pot2")
	PORT_START("VP3")
	PORT_ADJUSTER( 50, "VP3 - DAC volume" )     NETLIST_ANALOG_PORT_CHANGED("sound_nl", "pot3")

INPUT_PORTS_END



//**************************************************************************
//  BASE MELODY GENERATOR DEVICE CLASS
//**************************************************************************

zac1b111xx_melody_base::zac1b111xx_melody_base(
		machine_config const &mconfig,
		device_type devtype,
		char const *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, devtype, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_melodycpu(*this, "melodycpu")
	, m_melodypia(*this, "melodypia")
	, m_melodypsg1(*this, "melodypsg1")
	, m_melodypsg2(*this, "melodypsg2")
	, m_melody_command(0)
{
}

u8 zac1b111xx_melody_base::melodypia_porta_r()
{
	u8 const control = m_melodypia->b_output();
	u8 data = 0xff;

	if (0x01 == (control & 0x03))
		data &= m_melodypsg1->data_r();

	if (0x04 == (control & 0x0c))
		data &= m_melodypsg2->data_r();

	return data;
}

void zac1b111xx_melody_base::melodypia_porta_w(u8 data)
{
	u8 const control = m_melodypia->b_output();

	if (control & 0x02)
		m_melodypsg1->data_address_w((control >> 0) & 0x01, data);

	if (control & 0x08)
		m_melodypsg2->data_address_w((control >> 2) & 0x01, data);
}

void zac1b111xx_melody_base::melodypia_portb_w(u8 data)
{
	if (data & 0x02)
		m_melodypsg1->data_address_w((data >> 0) & 0x01, m_melodypia->a_output());

	if (data & 0x08)
		m_melodypsg2->data_address_w((data >> 2) & 0x01, m_melodypia->a_output());
}

u8 zac1b111xx_melody_base::melodypsg1_portb_r()
{
	return m_melody_command;
}

void zac1b111xx_melody_base::device_add_mconfig(machine_config &config)
{
	M6802(config, m_melodycpu, XTAL(3'579'545)); // verified on pcb
	m_melodycpu->set_addrmap(AS_PROGRAM, &zac1b111xx_melody_base::zac1b111xx_melody_base_map);

	clock_device &timebase(CLOCK(config, "timebase", XTAL(3'579'545)/4096/2)); // CPU clock divided using 4040 and half of 74LS74
	timebase.signal_handler().set(m_melodypia, FUNC(pia6821_device::cb1_w));

	PIA6821(config, m_melodypia);
	m_melodypia->readpa_handler().set(FUNC(zac1b111xx_melody_base::melodypia_porta_r));
	m_melodypia->writepa_handler().set(FUNC(zac1b111xx_melody_base::melodypia_porta_w));
	m_melodypia->writepb_handler().set(FUNC(zac1b111xx_melody_base::melodypia_portb_w));
	m_melodypia->irqa_handler().set_inputline("melodycpu", INPUT_LINE_NMI);
	m_melodypia->irqb_handler().set_inputline("melodycpu", M6802_IRQ_LINE);

	AY8910(config, m_melodypsg1, XTAL(3'579'545)/2); // CPU clock divided using 4040
	m_melodypsg1->port_b_read_callback().set(FUNC(zac1b111xx_melody_base::melodypsg1_portb_r));

	AY8910(config, m_melodypsg2, XTAL(3'579'545)/2); // CPU clock divided using 4040
}

void zac1b111xx_melody_base::device_start()
{
	save_item(NAME(m_melody_command));
}

void zac1b111xx_melody_base::device_reset()
{
	m_melody_command = 0;
}



//**************************************************************************
//  1B11107-SPECIFIC IMPLEMENTATION
//**************************************************************************

zac1b11107_audio_device::zac1b11107_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: zac1b111xx_melody_base(mconfig, ZACCARIA_1B11107, tag, owner, clock)
{
}

void zac1b11107_audio_device::sound_w(u8 data)
{
	// the sound program masks out the three most significant bits
	// assume the top two bits are not connected and read high from the internal pull-ups
	m_melodypia->ca1_w((data >> 5) & 0x01);
	m_melody_command = data | 0xc0;
}

void zac1b11107_audio_device::reset_w(int state)
{
	// TODO: there is a pulse-stretching network attached that should be simulated
	m_melodycpu->set_input_line(INPUT_LINE_RESET, state);
	// TODO: holds the reset line of m_melodypia - can't implement this in MAME at this time
	// TODO: holds the reset line of m_melodypsg1 - can't implement this in MAME at this time
	// TODO: holds the reset line of m_melodypsg2 - can't implement this in MAME at this time
}

void zac1b11107_audio_device::melodypsg1_porta_w(u8 data)
{
	// similar to 1B11142
	// TODO: move this to netlist audio where it belongs, along with the rest of the filtering
	static double const table[8] = {
			RES_K(8.2),
			RES_R(820),
			RES_K(3.3),
			RES_R(150),
			RES_K(5.6),
			RES_R(390),
			RES_K(1.5),
			RES_R(47) };
	m_melodypsg2->set_volume(1, 150 * RES_VOLTAGE_DIVIDER(RES_K(4.7), table[data & 0x07]));
}

void zac1b11107_audio_device::melodypsg2_porta_w(u8 data)
{
	// TODO: assume LEVELT is controlled here as is the case for 1B11142?
}

void zac1b11107_audio_device::device_add_mconfig(machine_config &config)
{
	zac1b111xx_melody_base::device_add_mconfig(config);

	m_melodycpu->set_addrmap(AS_PROGRAM, &zac1b11107_audio_device::zac1b11107_melody_map);

	m_melodypsg1->port_a_write_callback().set(FUNC(zac1b11107_audio_device::melodypsg1_porta_w));
	m_melodypsg1->add_route(ALL_OUTPUTS, *this, 0.5, 0);

	m_melodypsg2->port_a_write_callback().set(FUNC(zac1b11107_audio_device::melodypsg2_porta_w));
	m_melodypsg2->add_route(ALL_OUTPUTS, *this, 0.5, 0);
}



//**************************************************************************
//  1B11142-SPECIFIC IMPLEMENTATION
//**************************************************************************

zac1b11142_audio_device::zac1b11142_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: zac1b111xx_melody_base(mconfig, ZACCARIA_1B11142, tag, owner, clock)
	, m_acs_cb(*this)
	, m_audiocpu(*this, "audiocpu")
	, m_pia_1i(*this, "pia_1i")
	, m_speech(*this, "speech")
	, m_ioa(*this, "sound_nl:ioa%u", 0)
	, m_level(*this, "sound_nl:level")
	, m_levelt(*this, "sound_nl:levelt")
	, m_sw1(*this, "sound_nl:sw1")
	, m_inputs(*this, "1B11142")
	, m_host_command(0)
{
}

void zac1b11142_audio_device::hs_w(u8 data)
{
	m_host_command = data;
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}

int zac1b11142_audio_device::acs_r()
{
	return (~m_pia_1i->b_output() >> 3) & 0x01;
}

void zac1b11142_audio_device::ressound_w(int state)
{
	// TODO: there is a pulse-stretching network attached that should be simulated
	m_melodycpu->set_input_line(INPUT_LINE_RESET, state);
	// TODO: holds the reset line of m_melodypia - can't implement this in MAME at this time
	// TODO: holds the reset line of m_melodypsg1 - can't implement this in MAME at this time
	// TODO: holds the reset line of m_melodypsg2 - can't implement this in MAME at this time
	m_audiocpu->set_input_line(INPUT_LINE_RESET, state);
	// TODO: holds the reset line of m_pia_1i - can't implement this in MAME at this time
	// TODO: does some funky stuff with the VDD and VSS lines on the speech chip
}

void zac1b11142_audio_device::ay_4g_porta_w(u8 data)
{
	m_ioa[0]->write_line(BIT(data, 0)); // tromba mix volume
	m_ioa[1]->write_line(BIT(data, 1)); //   "     "    "
	m_ioa[2]->write_line(BIT(data, 2)); //   "     "    "
	m_ioa[3]->write_line(BIT(data, 3)); // cassa gate
	m_ioa[4]->write_line(BIT(data, 4)); // rullante gate
}

void zac1b11142_audio_device::ay_4h_porta_w(u8 data)
{
	m_level->write_line(BIT(data, 0)); // output level
	m_levelt->write_line(BIT(data, 1)); // tromba level
}

void zac1b11142_audio_device::ay_4h_portb_w(u8 data)
{
	m_sw1->write_line(BIT(data, 0)); // ANAL3 filter
}

u8 zac1b11142_audio_device::host_command_r()
{
	return m_host_command;
}

void zac1b11142_audio_device::melody_command_w(u8 data)
{
	m_melodypia->ca1_w((data >> 7) & 0x01);
	m_melody_command = data;
}

INPUT_CHANGED_MEMBER(zac1b11142_audio_device::p1_changed)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, (m_inputs->read() & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}

void zac1b11142_audio_device::pia_1i_portb_w(u8 data)
{
	m_speech->rsq_w(BIT(data, 0));
	m_speech->wsq_w(BIT(data, 1));
	m_acs_cb(BIT(~data, 3));
	// TODO: a LED output().set_led_value(0, BIT(data, 4));
}

void zac1b11142_audio_device::device_add_mconfig(machine_config &config)
{
	zac1b111xx_melody_base::device_add_mconfig(config);

	m_melodycpu->set_addrmap(AS_PROGRAM, &zac1b11142_audio_device::zac1b11142_melody_map);

	m_melodypsg1->port_a_write_callback().set(FUNC(zac1b11142_audio_device::ay_4g_porta_w));
	m_melodypsg1->add_route(0, "sound_nl", 1.0, 0);
	m_melodypsg1->add_route(1, "sound_nl", 1.0, 1);
	m_melodypsg1->add_route(2, "sound_nl", 1.0, 2);
	m_melodypsg1->set_flags(AY8910_RESISTOR_OUTPUT);

	m_melodypsg2->port_a_write_callback().set(FUNC(zac1b11142_audio_device::ay_4h_porta_w));
	m_melodypsg2->port_b_write_callback().set(FUNC(zac1b11142_audio_device::ay_4h_portb_w));
	m_melodypsg2->add_route(0, "sound_nl", 1.0, 3);
	m_melodypsg2->add_route(1, "sound_nl", 1.0, 4);
	m_melodypsg2->add_route(2, "sound_nl", 1.0, 5);
	m_melodypsg2->set_flags(AY8910_RESISTOR_OUTPUT);

	M6802(config, m_audiocpu, XTAL(3'579'545)); // verified on pcb
	m_audiocpu->set_addrmap(AS_PROGRAM, &zac1b11142_audio_device::zac1b11142_audio_map);

	PIA6821(config, m_pia_1i);
	m_pia_1i->readpa_handler().set(m_speech, FUNC(tms5220_device::status_r));
	m_pia_1i->writepa_handler().set(m_speech, FUNC(tms5220_device::data_w));
	m_pia_1i->writepb_handler().set(FUNC(zac1b11142_audio_device::pia_1i_portb_w));

	//MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.30, 0); // mc1408.1f
	MC1408(config, "dac").add_route(ALL_OUTPUTS, "sound_nl", 1.0, 7); // mc1408.1f

	// There is no xtal, the clock is obtained from a RC oscillator as shown in the TMS5220 datasheet (R=100kOhm C=22pF)
	// 162kHz measured on pin 3 20 minutes after power on, clock would then be 162.3*4=649.2kHz
	TMS5200(config, m_speech, 649200); // ROMCLK pin measured at 162.3Khz, OSC is exactly *4 of that)
	m_speech->irq_cb().set(m_pia_1i, FUNC(pia6821_device::cb1_w));
	m_speech->ready_cb().set(m_pia_1i, FUNC(pia6821_device::ca2_w));
	m_speech->add_route(0, "sound_nl", 1.0, 6);

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(netlist_zac1b11142)
		.add_route(ALL_OUTPUTS, *this, 1.0, 0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:ioa0",   "I_IOA0.IN",   0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:ioa1",   "I_IOA1.IN",   0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:ioa2",   "I_IOA2.IN",   0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:ioa3",   "I_IOA3.IN",   0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:ioa4",   "I_IOA4.IN",   0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:level",  "I_LEVEL.IN",  0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:levelt", "I_LEVELT.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:sw1",    "I_SW1.IN",    0);

	NETLIST_STREAM_INPUT(config, "sound_nl:cin0", 0, "R_AY4G_A.R");
	NETLIST_STREAM_INPUT(config, "sound_nl:cin1", 1, "R_AY4G_B.R");
	NETLIST_STREAM_INPUT(config, "sound_nl:cin2", 2, "R_AY4G_C.R");
	NETLIST_STREAM_INPUT(config, "sound_nl:cin3", 3, "R_AY4H_A.R");
	NETLIST_STREAM_INPUT(config, "sound_nl:cin4", 4, "R_AY4H_B.R");
	NETLIST_STREAM_INPUT(config, "sound_nl:cin5", 5, "R_AY4H_C.R");
	/* Speech
	 * The 5200 output is a current source providing between 0 and 1.5mA.
	 * This is explained in detail in the datasheet.
	 */
	NETLIST_STREAM_INPUT(config, "sound_nl:cin6", 6, "I_SP.I").set_mult_offset(2.0 * 750e-6, 750e-6);
	/* DAC
	 * The 1408 output is a current sink providing between 0 and 2.0mA.
	 * This is explained in detail in the datasheet.
	 */
	NETLIST_STREAM_INPUT(config, "sound_nl:cin7", 7, "I_DAC.I").set_mult_offset(1e-3, 1e-3);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "C7.2").set_mult_offset(3000.0 * 10.0 / 32768.0, 0.0); // FIXME: no clue what numbers to use here

	// Potentiometers
	NETLIST_ANALOG_INPUT(config, "sound_nl:pot1", "P1.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:pot2", "P2.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:pot3", "P3.DIAL");

}

ioport_constructor zac1b11142_audio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(zac1b11142_ioports);
}

void zac1b11142_audio_device::device_start()
{
	zac1b111xx_melody_base::device_start();

	save_item(NAME(m_host_command));
}

void zac1b11142_audio_device::device_reset()
{
	zac1b111xx_melody_base::device_reset();

	m_host_command = 0;
}
