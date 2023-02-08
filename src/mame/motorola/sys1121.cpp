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

#include "bus/vme/tp881v.h"
#include "bus/vme/vme_mvme120.h"
#include "bus/vme/vme_mvme180.h"
#include "bus/vme/vme_mvme181.h"
#include "bus/vme/vme_mvme327a.h"
#include "bus/vme/vme_smvme2000.h"


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

void sys1121_vme_cards(device_slot_interface &device)
{
	device.option_add("mvme120",   VME_MVME120);
	device.option_add("mvme121",   VME_MVME121);
	device.option_add("mvme122",   VME_MVME122);
	device.option_add("mvme123",   VME_MVME123);
	device.option_add("smvme2000", VME_SMVME2000);
	device.option_add("mvme180",   VME_MVME180);
	device.option_add("mvme181",   VME_MVME181);
	device.option_add("mvme327a",  VME_MVME327A);
	device.option_add("tp881v",    TP881V);
}

void sys1121_state::sys1121(machine_config &config)
{
	VME(config, "vme", 0);
	VME_SLOT(config, "slot1", sys1121_vme_cards, "mvme120", 1, "vme");
	VME_SLOT(config, "slot2", sys1121_vme_cards, nullptr, 2, "vme");
	VME_SLOT(config, "slot3", sys1121_vme_cards, nullptr, 3, "vme");
	VME_SLOT(config, "slot4", sys1121_vme_cards, nullptr, 4, "vme");
	VME_SLOT(config, "slot5", sys1121_vme_cards, nullptr, 5, "vme");
	VME_SLOT(config, "slot6", sys1121_vme_cards, nullptr, 6, "vme");
	VME_SLOT(config, "slot7", sys1121_vme_cards, nullptr, 7, "vme");
	VME_SLOT(config, "slot8", sys1121_vme_cards, nullptr, 8, "vme");
}

// This is a VME chassis so any ROMs are contained in the cards.
ROM_START(sys1121)
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY     FULLNAME    FLAGS
COMP( 1984, sys1121, 0,       0,      sys1121, sys1121, sys1121_state, empty_init, "Motorola", "SYS1121",  MACHINE_IS_SKELETON )
