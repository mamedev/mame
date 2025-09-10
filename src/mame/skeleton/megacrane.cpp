// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/****************************************************************************

    Skeleton driver for Elaut Megacrane

****************************************************************************/

#include "emu.h"
#include "speaker.h"

#include "cpu/mc68hc11/mc68hc11.h"
#include "sound/ay8910.h"

namespace {

class megacrane_state : public driver_device
{
public:
	megacrane_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void megacrane(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<mc68hc11_cpu_device> m_maincpu;
};


void megacrane_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
}


static INPUT_PORTS_START(megacrane)
	// all unknown
INPUT_PORTS_END


void megacrane_state::megacrane(machine_config &config)
{
	MC68HC11F1(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &megacrane_state::mem_map);

	SPEAKER(config, "mono").front_center();

	YMZ284(config, "ymz", 4000000).add_route(ALL_OUTPUTS, "mono", 1.0);

}


ROM_START(megacrane)
    ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("euro_2002_rossi_leisure_02.39.07.u5", 0x0000, 0x8000, CRC(00000000) SHA1(0000000000000000000000000000000000000000))
	ROM_REGION(0x8000, "sound", 0)
	ROM_LOAD("elaut_98_sound_megacrane.u", 0x0000, 0x8000, CRC(00000000) SHA1(0000000000000000000000000000000000000000))
ROM_END

} // anonymous namespace


SYST(1997, megacrane, 0, 0, megacrane, megacrane, megacrane_state, empty_init, "Elaut", "Megacrane", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
