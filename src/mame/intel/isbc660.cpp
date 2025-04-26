// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Intel iSBC 660 System Chassis
 *
 * This is a bare system chassis with an 8-slot backplane into which a variety
 * of Multibus boards may be installed.
 *
 * Sources:
 *  - http://www.nj7p.org/Manuals/PDFs/Intel/AFN-00285A.pdf
 *
 * TODO:
 *  - additional cards
 */

#include "emu.h"

#include "bus/multibus/multibus.h"
#include "bus/multibus/isbc8024.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class isbc660_state : public driver_device
{
public:
	isbc660_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_bus(*this, "bus")
	{
	}

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

public:
	// machine config
	void isbc660(machine_config &config);

private:
	required_device<multibus_device> m_bus;
};

void isbc660_state::machine_start()
{
}

void isbc660_state::machine_reset()
{
}

static void isbc660_cards(device_slot_interface &device)
{
	device.option_add("isbc8024", ISBC8024);
}

void isbc660_state::isbc660(machine_config &config)
{
	MULTIBUS(config, m_bus, 10_MHz_XTAL); // FIXME: clock driven by bus master

	MULTIBUS_SLOT(config, "slot1", m_bus, isbc660_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot2", m_bus, isbc660_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot3", m_bus, isbc660_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot4", m_bus, isbc660_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot5", m_bus, isbc660_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot6", m_bus, isbc660_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot7", m_bus, isbc660_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot8", m_bus, isbc660_cards, nullptr, false);
}

ROM_START(isbc660)
ROM_END

}

/*   YEAR  NAME     PARENT COMPAT MACHINE  INPUT CLASS          INIT        COMPANY  FULLNAME    FLAGS */
COMP(1985, isbc660, 0,     0,     isbc660, 0,    isbc660_state, empty_init, "Intel", "iSBC 660", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
