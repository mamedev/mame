// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics IRIS 1400 workstation
 *
 * TODO:
 *  - storage cards (Data Systems Design DSD-5217)
 *  - network card (Excelan EXOS/101 "nx")
 *  - graphics cards (GF1+UC3+DC3+BP2?)
 *  - Sky floating-point card
 *
 */
 /*
  * Notes
  * IRIS 1400/1500 - standalone workstations; PM2 (10 MHz 68010), SGI
  * PM2M1 multibus memory, Excelan EXOS101 ethernet. The processor is SGI-
  * designed. The 1400 uses ST-506 hard drives on a DSD 5215 (also used in
  * the 1200 to control the optional floppy).
  *
  *
  * The IRIS 1400 comes standard with 1.5 MB of CPU  memory,  8  bit-
  * planes of 1024x1024 image memory, and a 72 MB Winchester disk and
  * controller.  Options include additional CPU and image  memory,  a
  * floating point accelerator, and large capacity disk and tape sys-
  * tems.
  */
#include "emu.h"

#include "bus/multibus/multibus.h"

#include "pm2.h"

namespace {

class iris1400_state : public driver_device
{
public:
	iris1400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bus(*this, "bus")
	{
	}

	void iris1400(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<multibus_device> m_bus;
};

void iris1400_state::machine_start()
{
}

void iris1400_state::machine_reset()
{
}

static void iris1400_p_cards(device_slot_interface &device)
{
	// processor side cards
	device.option_add("pm2",  SGI_PM2);
}
static void iris1400_g_cards(device_slot_interface &device)
{
	// graphics side cards
}

void iris1400_state::iris1400(machine_config &config)
{
	MULTIBUS(config, m_bus, 10'000'000);

	// 20-slot chassis, card positions assumed from 2xxx
	MULTIBUS_SLOT(config,  "slot1", m_bus, iris1400_p_cards, nullptr, false); // memory
	MULTIBUS_SLOT(config,  "slot2", m_bus, iris1400_p_cards, "pm2",   false);
	MULTIBUS_SLOT(config,  "slot3", m_bus, iris1400_p_cards, nullptr, false); // ethernet
	MULTIBUS_SLOT(config,  "slot4", m_bus, iris1400_p_cards, nullptr, false); // disk/tape controller
	MULTIBUS_SLOT(config,  "slot5", m_bus, iris1400_p_cards, nullptr, false);
	MULTIBUS_SLOT(config,  "slot6", m_bus, iris1400_p_cards, nullptr, false);
	MULTIBUS_SLOT(config,  "slot7", m_bus, iris1400_p_cards, nullptr, false);
	MULTIBUS_SLOT(config,  "slot8", m_bus, iris1400_p_cards, nullptr, false);
	MULTIBUS_SLOT(config,  "slot9", m_bus, iris1400_p_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot10", m_bus, iris1400_p_cards, nullptr, false);

	MULTIBUS_SLOT(config, "slot11", m_bus, iris1400_g_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot12", m_bus, iris1400_g_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot13", m_bus, iris1400_g_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot14", m_bus, iris1400_g_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot15", m_bus, iris1400_g_cards, nullptr, false);
	MULTIBUS_SLOT(config, "slot16", m_bus, iris1400_g_cards, nullptr, false); // bit plane
	MULTIBUS_SLOT(config, "slot17", m_bus, iris1400_g_cards, nullptr, false); // bit plane
	MULTIBUS_SLOT(config, "slot18", m_bus, iris1400_g_cards, nullptr, false); // display controller
	MULTIBUS_SLOT(config, "slot19", m_bus, iris1400_g_cards, nullptr, false); // update controller
	MULTIBUS_SLOT(config, "slot20", m_bus, iris1400_g_cards, nullptr, false); // frame buffer
}

ROM_START(iris1400)
ROM_END

} // anonymous namespace

//   YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT  CLASS           INIT        COMPANY             FULLNAME      FLAGS
COMP(1984, iris1400, 0,      0,      iris1400, 0,     iris1400_state, empty_init, "Silicon Graphics", "IRIS 1400",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
