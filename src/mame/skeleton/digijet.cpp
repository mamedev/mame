// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*************************************************************************

    drivers/digijet.cpp

    Skeleton driver for the Volkswagen Digijet series of automotive ECUs

    The Digijet Engine Control Unit (ECU) was used in Volkswagen vehicles
    from the early 1980s.

    Currently, the only dump is from a 1985 Volkswagen Vanagon (USA CA).

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

private:
	required_device<cpu_device> m_maincpu;

	virtual void machine_start() override { }
	virtual void machine_reset() override { }
	void io_map(address_map &map) ATTR_COLD;
};

void digijet_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START( digijet )
INPUT_PORTS_END

void digijet_state::digijet(machine_config &config)
{
	/* basic machine hardware */
	I8049(config, m_maincpu, XTAL(11'000'000));
	m_maincpu->set_addrmap(AS_IO, &digijet_state::io_map);
}

ROM_START( digijet )
	ROM_REGION( 0x800, I8049_TAG, 0 )
	ROM_LOAD( "vanagon_85_usa_ca.bin", 0x000, 0x800, CRC(2ed7c4c5) SHA1(ae48d8892b44fe76b48bcefd293c15cd47af3fba) ) // Volkswagen Vanagon, 1985, USA, California
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY       FULLNAME   FLAGS
CONS( 1985, digijet, 0,      0,      digijet, digijet, digijet_state, empty_init, "Volkswagen", "Digijet", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
