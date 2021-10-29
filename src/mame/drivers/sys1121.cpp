// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/*
 *	Motorola MVME120 CPU board.
 *  This is an early MVME system from 1984, using standard Motorola parts
 *  instead of the many ASICs of later boards.
 *
 *	The following configurations were available:
 *  MVME120 - 10MHz 68010, 128KB RAM, 4KB SRAM cache, 68451 MMU
 *  MVME121 - 10MHz 68010, 512KB RAM, 4KB SRAM cache, 68451 MMU
 *  MVME122 - 12.5MHz 68010, 128KB RAM, no cache, no MMU
 *  MVME123 - 12.5MHz 68010, 512KB RAM, 4KB SRAM cache, no MMU
 *
 *  Current state, it crashes at $F058D8 while testing CPU exception handling.
 *  If you skip over that address (pc=F058E2 in the debugger) it continues
 *  through the self-test.
 */
#include "emu.h"
#include "bus/vme/vme.h"
#include "bus/vme/vme_mvme120.h"
#include "logmacro.h"

class sys1121_state : public driver_device
{
public:
sys1121_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device (mconfig, type, tag)
	{
	}
	virtual void machine_start () override { }
//  virtual void machine_reset () override;

	void init_sys1121()      { }
	void sys1121(machine_config &config);
};

/* Input ports */
static INPUT_PORTS_START (sys1121)
INPUT_PORTS_END

static void mvme120_vme_cards(device_slot_interface &device)
{
	device.option_add("mvme120", VME_MVME120);
	device.option_add("mvme121", VME_MVME121);
	device.option_add("mvme122", VME_MVME122);
	device.option_add("mvme123", VME_MVME123);
}

void sys1121_state::sys1121(machine_config &config)
{
	VME(config, "vme", 0);
	VME_SLOT(config, "slot1", mvme120_vme_cards, "mvme120", 1, "vme");
}

/* ROM definitions */
ROM_START (sys1121) ROM_END

/* Driver */
//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY     FULLNAME    FLAGS
COMP( 1984, sys1121, 0,       0,      sys1121, sys1121, sys1121_state, empty_init, "Motorola", "SYS1121",  MACHINE_IS_SKELETON )
