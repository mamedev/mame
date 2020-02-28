// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Coleco Talking Wrinkles, a plushie dog handpuppet toy

Hardware is a P80C31BH @ 11MHz and a 32KB ROM, RAM is in the MCU.
It also has a cartridge slot, but no known cartridges were released.
The speech technology is by Electronic Speech Systems.

TODO:
- add sensors

******************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
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

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<mcs51_cpu_device> m_maincpu;

	void main_map(address_map &map);

	// I/O handlers
	DECLARE_WRITE8_MEMBER(sensor_w);
	DECLARE_READ8_MEMBER(sensor_r);
};

void wrinkles_state::machine_start()
{
}



/******************************************************************************
    I/O
******************************************************************************/

WRITE8_MEMBER(wrinkles_state::sensor_w)
{
}

READ8_MEMBER(wrinkles_state::sensor_r)
{
	// sensors here
	// d1: hold down for power-off?
	// d7: mouth sensor?
	// other: ?
	return 0xff;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void wrinkles_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x7fff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( wrinkles )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void wrinkles_state::wrinkles(machine_config &config)
{
	/* basic machine hardware */
	I80C31(config, m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &wrinkles_state::main_map);
	m_maincpu->port_in_cb<1>().set(FUNC(wrinkles_state::sensor_r));
	m_maincpu->port_out_cb<1>().set(FUNC(wrinkles_state::sensor_w));
	m_maincpu->port_out_cb<3>().set("dac", FUNC(dac_8bit_r2r_device::write));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac").add_route(ALL_OUTPUTS, "speaker", 0.5);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( wrinkles )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("umua117_wrkl_dif4.u3", 0x0000, 0x8000, CRC(4ec8ddbf) SHA1(beb165d933659859a4f966168ca121843cd6642b) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT CMP MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1986, wrinkles,  0,      0, wrinkles, wrinkles, wrinkles_state, empty_init, "Coleco / Ganz", "Talking Wrinkles", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
