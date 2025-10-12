// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*************************************************************************

    drivers/digijet.cpp

    Skeleton driver for the Volkswagen Digijet series of automotive ECUs

    The Digijet Engine Control Unit (ECU) was used in Volkswagen vehicles
    from the 1980s.

    CPU: MAF 80A39HL
	ROM: ST M2764AF6
	ADC0809CCN
	LM2901
	SN74LS373N
	SN74LS00N
	XTAL 7.372 MHz

	_________________________
	|          7   A 0       |
	|          4   D 8       |
	C          3   C 0    7  |
	O          7     9    4  |
	N  L2      3          0  |
	N  M0    M2764AF6  X  0  |
	|  91          80A39HL   |
	|________________________|

	Connector
	1  RPM
	2  Coolant temperture
	3  GND
	4  Throttle switch
	5  Lambda sensor
	6  GND
	7  GND
	9  GND
	10 GND
	12 Injectors
	13 Ignition
	14 Air in temperature
	15 Air amount
	16 GND
	17 GND
	18 GND
	19 Air sensor power
	20 Fuel pump
	22 GND

**************************************************************************/

/*
    TODO:

    - Everything
*/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"


namespace {

#define I8049_TAG   "i8049"

class digijet_state : public driver_device
{
public:
	digijet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, I8049_TAG)
	{
	}

	void digijet(machine_config &config);
	void digijet90(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	virtual void machine_start() override { }
	virtual void machine_reset() override { }
	void io_map(address_map &map) ATTR_COLD;
	void io_map90(address_map &map) ATTR_COLD;
};

void digijet_state::io_map(address_map &map)
{
	map(0x32,0x32).noprw();;
	map(0x33,0x33).nopr();;
	map(0x3f,0x3f).noprw();;
}

void digijet_state::io_map90(address_map &map)
{
	map(0x37,0x37).noprw();;
	map(0x38,0x38).nopr();;
}

static INPUT_PORTS_START( digijet )
INPUT_PORTS_END

void digijet_state::digijet(machine_config &config)
{
	/* basic machine hardware */
	I8049(config, m_maincpu, XTAL(11'000'000));
	m_maincpu->set_addrmap(AS_IO, &digijet_state::io_map);
}

void digijet_state::digijet90(machine_config &config)
{
	/* basic machine hardware */
	I8049(config, m_maincpu, XTAL(7'372'800));
	m_maincpu->set_addrmap(AS_IO, &digijet_state::io_map90);
}

ROM_START( digijet )
	ROM_REGION( 0x800, I8049_TAG, 0 )
	ROM_LOAD( "vanagon_85_usa_ca.bin", 0x000, 0x800, CRC(2ed7c4c5) SHA1(ae48d8892b44fe76b48bcefd293c15cd47af3fba) ) // Volkswagen Vanagon, 1985, USA, California
ROM_END

ROM_START( digijet90 )
	ROM_REGION( 0x2000, I8049_TAG, 0 )
	ROM_LOAD( "fabb05_03_03.bin", 0x0000, 0x2000, CRC(8c96bcdf) SHA1(73b26914cd15ca3a5e0d7427de9ce4b4e311fb00) ) // Volkswagen 1990, Germany
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT     COMPAT  MACHINE    INPUT    CLASS          INIT        COMPANY       FULLNAME          FLAGS
CONS( 1985, digijet,   digijet90, 0,      digijet,   digijet, digijet_state, empty_init, "Volkswagen", "Digijet",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
CONS( 1990, digijet90, 0,         0,      digijet90, digijet, digijet_state, empty_init, "Volkswagen", "Digijet (1990)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
