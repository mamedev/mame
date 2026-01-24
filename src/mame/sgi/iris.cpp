// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics IRIS systems.
 *
 *  Model  Type         CPU  Graphics      Disk    Chassis
 *  -----  -----------  ---  -------------  ------  -------
 *  1400   Workstation  PM2  IRIS           ST-506  20-slot
 *  1500   Workstation  PM2  IRIS           SMD     rack
 *  2000   Terminal     PM2  IRIS           none    10-slot
 *  2200   Terminal     PM2  IRIS           none    10-slot
 *  2300   Workstation  PM2  IRIS           ST-506  20-slot
 *  2400   Workstation  PM2  IRIS           ST-506  20-slot
 *  2500   Workstation  PM2  IRIS           SMD     rack
 *
 *  2300T  Workstation  IP2  IRIS           ST-506  20-slot
 *  2400T  Workstation  IP2  IRIS           ST-506  20-slot
 *  2500T  Workstation  IP2  IRIS           SMD     rack
 *  3010   Terminal     IP2  Enhanced IRIS  ST-506
 *  3020   Workstation  IP2  Enhanced IRIS  ST-506
 *  3030   Workstation  IP2  Enhanced IRIS  ESDI
 *  3110   Workstation  IP2  Enhanced IRIS  ST-506  20-slot
 *  3115   Workstation  IP2  Enhanced IRIS  ST-506  20-slot
 *  3120   Workstation  IP2  Enhanced IRIS  ESDI    20-slot
 *  3130   Workstation  IP2  Enhanced IRIS  ESDI    20-slot
 *
 * TODO:
 *  - storage cards (Data Systems Design DSD-5217)
 *  - network cards (Excelan EXOS/101 "nx")
 *  - graphics cards (GF1+UC3+DC3+BP2?)
 *  - Sky floating-point card
 *
 */

#include "emu.h"

#include "bus/multibus/multibus.h"
#include "bus/multibus/dsd5217.h"

#include "ip2.h"
#include "pm2.h"

#include "iris1400.lh"

namespace {

class iris_state : public driver_device
{
public:
	iris_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bus(*this, "bus")
		, m_slot(*this, "slot%u", 1U)
	{
	}

	void iris1400(machine_config &config);
	void iris3130(machine_config &config);

private:
	void common(machine_config &config);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<multibus_device> m_bus;
	required_device_array<multibus_slot_device, 20> m_slot;
};

void iris_state::machine_start()
{
}

void iris_state::machine_reset()
{
}

static DEVICE_INPUT_DEFAULTS_START(dsd)
	// wake-up address 0x7f00
	DEVICE_INPUT_DEFAULTS("W7", 0x01, 0x01)
	DEVICE_INPUT_DEFAULTS("W7", 0x02, 0x02)
	DEVICE_INPUT_DEFAULTS("W7", 0x04, 0x04)
	DEVICE_INPUT_DEFAULTS("W7", 0x08, 0x08)
	DEVICE_INPUT_DEFAULTS("W7", 0x10, 0x10)
	DEVICE_INPUT_DEFAULTS("W7", 0x20, 0x20)
	DEVICE_INPUT_DEFAULTS("W7", 0x40, 0x40)
	DEVICE_INPUT_DEFAULTS("W7", 0x80, 0x00)
	DEVICE_INPUT_DEFAULTS("W9", 0x01, 0x00)
	DEVICE_INPUT_DEFAULTS("W9", 0x02, 0x00)
	DEVICE_INPUT_DEFAULTS("W9", 0x04, 0x00)
	DEVICE_INPUT_DEFAULTS("W9", 0x08, 0x00)
	DEVICE_INPUT_DEFAULTS("W9", 0x10, 0x00)
	DEVICE_INPUT_DEFAULTS("W9", 0x20, 0x00)
	DEVICE_INPUT_DEFAULTS("W9", 0x40, 0x00)
	DEVICE_INPUT_DEFAULTS("W9", 0x80, 0x00)

	// interrupt level 1
	DEVICE_INPUT_DEFAULTS("W10", 0xff, 0xfd)
DEVICE_INPUT_DEFAULTS_END

static void iris_p_cards(device_slot_interface &device)
{
	// processor side cards
	device.option_add("ip2", SGI_IP2);
	device.option_add("pm2", SGI_PM2);

	device.option_add("dsd", MULTIBUS_DSD5217).input_device_defaults(DEVICE_INPUT_DEFAULTS_NAME(dsd));
}
static void iris_g_cards(device_slot_interface &device)
{
	// graphics side cards
}

// 20-slot chassis, card positions assumed from 2xxx
void iris_state::common(machine_config &config)
{
	MULTIBUS(config, m_bus, 10'000'000);

	// processor side slots
	MULTIBUS_SLOT(config, m_slot[0], m_bus, iris_p_cards, nullptr, false); // memory
	MULTIBUS_SLOT(config, m_slot[1], m_bus, iris_p_cards, nullptr, false); // processor
	MULTIBUS_SLOT(config, m_slot[2], m_bus, iris_p_cards, nullptr, false); // ethernet
	MULTIBUS_SLOT(config, m_slot[3], m_bus, iris_p_cards, nullptr, false); // disk/tape controller
	MULTIBUS_SLOT(config, m_slot[4], m_bus, iris_p_cards, nullptr, false);
	MULTIBUS_SLOT(config, m_slot[5], m_bus, iris_p_cards, nullptr, false);
	MULTIBUS_SLOT(config, m_slot[6], m_bus, iris_p_cards, nullptr, false);
	MULTIBUS_SLOT(config, m_slot[7], m_bus, iris_p_cards, nullptr, false);
	MULTIBUS_SLOT(config, m_slot[8], m_bus, iris_p_cards, nullptr, false);
	MULTIBUS_SLOT(config, m_slot[9], m_bus, iris_p_cards, nullptr, false);

	// graphics side slots
	MULTIBUS_SLOT(config, m_slot[10], m_bus, iris_g_cards, nullptr, false);
	MULTIBUS_SLOT(config, m_slot[11], m_bus, iris_g_cards, nullptr, false);
	MULTIBUS_SLOT(config, m_slot[12], m_bus, iris_g_cards, nullptr, false);
	MULTIBUS_SLOT(config, m_slot[13], m_bus, iris_g_cards, nullptr, false);
	MULTIBUS_SLOT(config, m_slot[14], m_bus, iris_g_cards, nullptr, false);
	MULTIBUS_SLOT(config, m_slot[15], m_bus, iris_g_cards, nullptr, false); // bit plane
	MULTIBUS_SLOT(config, m_slot[16], m_bus, iris_g_cards, nullptr, false); // bit plane
	MULTIBUS_SLOT(config, m_slot[17], m_bus, iris_g_cards, nullptr, false); // display controller
	MULTIBUS_SLOT(config, m_slot[18], m_bus, iris_g_cards, nullptr, false); // update controller
	MULTIBUS_SLOT(config, m_slot[19], m_bus, iris_g_cards, nullptr, false); // frame buffer
}

void iris_state::iris1400(machine_config &config)
{
	common(config);

	m_slot[1]->set_default_option("pm2");
	m_slot[3]->set_default_option("dsd");

	config.set_default_layout(layout_iris1400);
}

void iris_state::iris3130(machine_config &config)
{
	common(config);

	m_slot[1]->set_default_option("ip2");
	m_slot[3]->set_default_option("dsd");
}

ROM_START(iris1400)
ROM_END
ROM_START(iris3130)
ROM_END

} // anonymous namespace

//   YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT  CLASS       INIT        COMPANY             FULLNAME      FLAGS
COMP(1984, iris1400, 0,      0,      iris1400, 0,     iris_state, empty_init, "Silicon Graphics", "IRIS 1400",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP(1985, iris3130, 0,      0,      iris3130, 0,     iris_state, empty_init, "Silicon Graphics", "IRIS 3130",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
