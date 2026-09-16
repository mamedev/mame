// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 *  Motorola M8120 system.
 */

#include "emu.h"

#include "bus/vme/vme.h"
#include "bus/vme/mvme187.h"

namespace {

class m8120_state : public driver_device
{
public:
	m8120_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void m8120(machine_config &config);
};

void m8120_state::m8120(machine_config &config)
{
	VME(config, "vme");

	VME_SLOT(config, "vme:slot1").option_set("mvme187", VME_MVME187);
}

ROM_START(m8120)
ROM_END

} // anonymous namespace

//    YEAR   NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME  FLAGS
COMP( 1991,  m8120, 0,      0,      m8120,   0,     m8120_state, empty_init, "Motorola", "M8120",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
