// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

namespace {

class kisssite_state : public driver_device

{
public:
	kisssite_state(machine_config const &mconfig, device_type type, char const *tag) :
		driver_device(mconfig, type, tag)
	{
	}

	void kisssite(machine_config &config) ATTR_COLD;

private:
};


void kisssite_state::kisssite(machine_config &config)
{
}

INPUT_PORTS_START(kisssite)
INPUT_PORTS_END

ROM_START(kisssite)
	ROM_REGION(0x08'0000, "maincpu", 0 )
	ROM_LOAD("ht27c020.u10", 0x000000, 0x040000, CRC(ccedce2b) SHA1(28dd3dfd0b8de0c5aa1c37d193ffc479d46563a1) )
ROM_END

} // anonymous namespace

SYST(200?, kisssite, 0,         0,      kisssite, kisssite, kisssite_state, empty_init, "Tomy", "Kiss-Site", MACHINE_IS_SKELETON)
