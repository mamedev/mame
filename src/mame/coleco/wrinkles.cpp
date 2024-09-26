// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:David Viens
/*******************************************************************************

Talking Wrinkles (model 6006), a dog hand puppet

Published by Lakeside (a Coleco subsidiary at that time, after Coleco purchased
Leisure Dynamics in 1985). Programming by Stephen Beck. The speech technology is
by Electronic Speech Systems. The plushie itself is licensed from Ganz Bros.

Hardware notes:

PCB 1:
- PCB label: REV 4.1 DIGITAL, 201239C, (C) COLECO 1986
- P80C31BH, 11MHz XTAL
- 32KB EPROM
- cartridge slot (no known cartridges were released)

PCB 2:
- PCB label: ANALOG REV 6.2, 201238D, (C) COLECO 1986
- button, motion sensor, microphone

Known sensors:
- 0x02: bellybutton, literally a button
- 0x04: detect violent motion (drop Wrinkles and he will cry)
- 0x10: detect light motion
- 0x40: detect open mouth (use as handpuppet to make it 'talk')
- 0x80: detect magnet in mouth (the toy came with a 'bone' that has a magnet in it)

TODO:
- where is the microphone? or are they the same inputs as the motion sensors?
- power-on by pressing button

*******************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "sound/dac.h"

#include "speaker.h"


namespace {

class wrinkles_state : public driver_device
{
public:
	wrinkles_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void wrinkles(machine_config &config);

private:
	required_device<mcs51_cpu_device> m_maincpu;

	void main_map(address_map &map) ATTR_COLD;
};



/*******************************************************************************
    Address Maps
*******************************************************************************/

void wrinkles_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x7fff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( wrinkles )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("Tickle Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("Impact Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Motion Sensor")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("Mouth Open")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("Mouth Magnet")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void wrinkles_state::wrinkles(machine_config &config)
{
	// basic machine hardware
	I80C31(config, m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &wrinkles_state::main_map);
	m_maincpu->port_in_cb<1>().set_ioport("INPUTS");
	m_maincpu->port_out_cb<3>().set("dac", FUNC(dac_8bit_r2r_device::write));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac").add_route(ALL_OUTPUTS, "speaker", 0.5);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( wrinkles )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("umua117_wrkl_dif4.u3", 0x0000, 0x8000, CRC(4ec8ddbf) SHA1(beb165d933659859a4f966168ca121843cd6642b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1986, wrinkles,  0,      0,      wrinkles, wrinkles, wrinkles_state, empty_init, "Lakeside / Coleco / Ganz Bros", "Talking Wrinkles", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_CONTROLS | MACHINE_NOT_WORKING )
