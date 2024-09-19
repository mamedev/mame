// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Labtam International 3000 Systems
 *
 * Sources:
 *  -
 *
 * TODO:
 *  - additional cards
 */

/*
3000 1983
???

3003 1983
Desktop
Board:
one - Z80 SBC
one - 8086 VDU/COMM
one - WD1002-HDO (Like Kaypro-10) https://retrocmp.de/kaypro/kay-p2_hrdw.htm#hdc
one - FDD 5'25 - Connect to WD1002 board
one - MFM HDD 5'25 NEC D5124 10MB - Connect to WD1002 board

3006 1983
Monoblock
Board:
one - Z80 SBC
one - 8086 VDU/COMM
two - FDD 8" - Connect to Z80 SBC board (to WD2793A on board)
one - FDD 5"25 - Connect to Z80 SBC board (to WD2793A on board)

3015-V32 1985
Tower
Board:
one - Z80 SBC
four - 8086 VDU/COMM (one board with main ROM and others slave ROM)
one - V32 (main processor board on CPU - NS32032, MMU - NS32082, FPU - NS32081, 2 Megabyte RAM), Optional SCSI contoler on board.
one or two - Additional one or two RAM board with 6 Megabyte.
one - INTERPHASE SMD 2190 (Controller SMD HDD)
one - Xylogics 472 (Controller 1/2 inch TAPE with PERTEC interface)
 */

#include "emu.h"

#include "bus/multibus/multibus.h"

#include "bus/multibus/labtam_3232.h"
#include "bus/multibus/labtam_vducom.h"
#include "bus/multibus/labtam_z80sbc.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class labtam_state : public driver_device
{
public:
	labtam_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_bus(*this, "slot")
	{
	}

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

public:
	// machine config
	void labtam(machine_config &config);

private:
	required_device<multibus_device> m_bus;
};

void labtam_state::machine_start()
{
}

void labtam_state::machine_reset()
{
}

static void labtam_cards(device_slot_interface &device)
{
	device.option_add("labtam_3232",    LABTAM_3232);
	device.option_add("labtam_8086cpu", LABTAM_8086CPU);
	device.option_add("labtam_vducom",  LABTAM_VDUCOM);
	device.option_add("labtam_z80sbc",  LABTAM_Z80SBC);
}

void labtam_state::labtam(machine_config &config)
{
	MULTIBUS(config, m_bus, 10_MHz_XTAL); // FIXME: clock driven by bus master

	MULTIBUS_SLOT(config, "slot:0", m_bus, labtam_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot:1", m_bus, labtam_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot:2", m_bus, labtam_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot:3", m_bus, labtam_cards, "labtam_8086cpu", false);
	MULTIBUS_SLOT(config, "slot:4", m_bus, labtam_cards, "labtam_vducom", false);
	MULTIBUS_SLOT(config, "slot:5", m_bus, labtam_cards, "labtam_z80sbc", false);
}

ROM_START(labtam)
ROM_END

}

/*   YEAR  NAME    PARENT COMPAT MACHINE  INPUT  CLASS          INIT        COMPANY                 FULLNAME    FLAGS */
COMP(1983, labtam, 0,     0,     labtam,  0,     labtam_state,  empty_init, "Labtam International", "3006", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
