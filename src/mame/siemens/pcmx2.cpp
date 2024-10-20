// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Siemens PC-MX2
 *
 * Sources:
 *  - http://www.cpu-ns32k.net/Siemens.html
 *  - https://oldcomputers-ddns.org/public/pub/rechner/siemens/mx-rm/pc-mx2/manuals/pc-mx2_betriebsanleitung_1987_(v2-1a).pdf
 *  - https://oldcomputers-ddns.org/public/pub/rechner/siemens/mx-rm/pc-mx2/manuals/pc-mx2_pc2000_9780_logik.pdf
 *
 * TODO:
 *  - front panel
 *  - storager
 */
/*
 * WIP notes:
 *  - currently there's no disk controller, so only possible to boot into the monitor
 *  - start with a terminal connected to SERAD port 0, i.e.: mame pcmx2 -slot:3:serad:port0 terminal
 *  - configure terminal for 38400 bps, 7 data bits, odd parity
 *  - set CPUAP board Boot Option to Monitor
 *  - press Enter at the "*" prompt to display monitor help
 */

#include "emu.h"

#include "bus/multibus/multibus.h"
#include "bus/multibus/cpuap.h"
//#include "bus/multibus/dueai.h"
#include "bus/multibus/serad.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class pcmx2_state : public driver_device
{
public:
	pcmx2_state(machine_config const &mconfig, device_type type, char const *tag)
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
	void pcmx2(machine_config &config);

private:
	required_device<multibus_device> m_bus;
};

void pcmx2_state::machine_start()
{
}

void pcmx2_state::machine_reset()
{
}

static void pcmx2_cards(device_slot_interface &device)
{
	device.option_add("cpuap", CPUAP);
	//device.option_add("dueai", DUEAI);
	device.option_add("serad", SERAD);
}

void pcmx2_state::pcmx2(machine_config &config)
{
	MULTIBUS(config, m_bus, 20_MHz_XTAL / 2);

	MULTIBUS_SLOT(config, "slot:1", m_bus, pcmx2_cards, nullptr, false); // DTC 86-1 or Storager
	MULTIBUS_SLOT(config, "slot:2", m_bus, pcmx2_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot:3", m_bus, pcmx2_cards, "serad", false);
	MULTIBUS_SLOT(config, "slot:4", m_bus, pcmx2_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot:5", m_bus, pcmx2_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot:6", m_bus, pcmx2_cards, "cpuap", false);
	MULTIBUS_SLOT(config, "slot:7", m_bus, pcmx2_cards, nullptr, false); // MEM
	MULTIBUS_SLOT(config, "slot:8", m_bus, pcmx2_cards, nullptr, false);
}

ROM_START(pcmx2)
ROM_END

}

/*   YEAR   NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY    FULLNAME  FLAGS */
COMP(1985,  pcmx2, 0,      0,      pcmx2,   0,     pcmx2_state, empty_init, "Siemens", "PC-MX2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
