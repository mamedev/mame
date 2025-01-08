// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/*
 *  Motorola SYS1121 VME chassis.
 *
 *  The basic configuration was an MVME12x MPU,
 *  an MVME050 system controller, and an MVME320 disk
 *  controller.
 */

#include "emu.h"

#include "bus/vme/vme.h"
#include "bus/vme/vme_cards.h"


namespace {

class sys1121_state : public driver_device
{
public:
	sys1121_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void sys1121(machine_config &config);
};

// Input ports
INPUT_PORTS_START(sys1121)
INPUT_PORTS_END

void sys1121_state::sys1121(machine_config &config)
{
	VME(config, "vme");
	VME_SLOT(config, "vme:slot1", vme_cards, "mvme120");
	VME_SLOT(config, "vme:slot2", vme_cards, nullptr);
	VME_SLOT(config, "vme:slot3", vme_cards, nullptr);
	VME_SLOT(config, "vme:slot4", vme_cards, nullptr);
	VME_SLOT(config, "vme:slot5", vme_cards, nullptr);
	VME_SLOT(config, "vme:slot6", vme_cards, nullptr);
	VME_SLOT(config, "vme:slot7", vme_cards, nullptr);
	VME_SLOT(config, "vme:slot8", vme_cards, nullptr);
}

// This is a VME chassis so any ROMs are contained in the cards.
ROM_START(sys1121)
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY     FULLNAME    FLAGS
COMP( 1984, sys1121, 0,       0,      sys1121, sys1121, sys1121_state, empty_init, "Motorola", "SYS1121",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
