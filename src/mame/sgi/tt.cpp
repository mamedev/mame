// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics Professional IRIS/PowerSeries "Twin Tower" systems.
 *
 *   Year  Model  Board  CPU    Clock    I/D Cache    Code Name
 *   1987  4D/50  IP4    R2000  8MHz     64KiB/32KiB  Twin Tower
 *   1987  4D/70  IP4    R2000  12.5MHz  64KiB/32KiB  Twin Tower
 *
 * Sources:
 *   - VME-Eclipse CPU (VIP10) Specification, Silicon Graphics, Inc.
 *
 * TODO:
 *  - graphics
 *  - IP4.5, IP5, IP7
 *  - 15 slot version (IP5, IP7)
 *
 */

#include "emu.h"

#include "bus/vme/vme.h"
#include "bus/vme/enp10.h"

#include "ip4.h"

// graphics cards
#include "gm1.h"

//#define VERBOSE (0)
#include "logmacro.h"

namespace {

class ip4_state : public driver_device
{
public:
	ip4_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_vme(*this, "vme")
		, m_slot(*this, "vme:slot%u", 1U)
	{
	}

	void pi4d50(machine_config &config);
	void pi4d70(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void tt12(machine_config &config, XTAL clock);

private:
	required_device<vme_bus_device> m_vme;
	required_device_array<vme_slot_device, 12> m_slot;
};

void ip4_state::pi4d50(machine_config &config)
{
	tt12(config, 16_MHz_XTAL);
}

void ip4_state::pi4d70(machine_config &config)
{
	tt12(config, 25_MHz_XTAL);
}

static void vme_cards(device_slot_interface &device)
{
	device.option_add("enp10", VME_ENP10);
}

static void slot8_cards(device_slot_interface &device)
{
	vme_cards(device);

	//device.option_add("ge4", SGI_GE4); // GT
}
static void slot9_cards(device_slot_interface &device)
{
	//device.option_add("de3", SGI_DE3); // B,G
	device.option_add("gm1", SGI_GM1); // GT
}
static void slot10_cards(device_slot_interface &device)
{
	//device.option_add("gf3", SGI_GF3); // B,G
	//device.option_add("rm1", SGI_RM1); // GT
}
static void slot11_cards(device_slot_interface &device)
{
	//device.option_add("tb2", SGI_TB2); // B,G
	//device.option_add("rv1", SGI_RV1); // GT
}
static void slot12_cards(device_slot_interface &device)
{
	//device.option_add("zb2", SGI_ZB2); // G
	//device.option_add("rm1", SGI_RM1); // GT
}

void ip4_state::tt12(machine_config &config, XTAL clock)
{
	VME(config, m_vme);

	VME_SLOT(config,  m_slot[0]).option_set("ip4", SGI_IP4).clock(clock);
	VME_SLOT(config,  m_slot[1], vme_cards, "enp10", false);
	VME_SLOT(config,  m_slot[2], vme_cards, nullptr, false);
	VME_SLOT(config,  m_slot[3], vme_cards, nullptr, false);

	VME_SLOT(config,  m_slot[4], vme_cards, nullptr, false);
	VME_SLOT(config,  m_slot[5], vme_cards, nullptr, false);
	VME_SLOT(config,  m_slot[6], vme_cards, nullptr, false);
	VME_SLOT(config,  m_slot[7], slot8_cards, nullptr, false);

	VME_SLOT(config,  m_slot[8], slot9_cards, nullptr, false);
	VME_SLOT(config,  m_slot[9], slot10_cards, nullptr, false);
	VME_SLOT(config, m_slot[10], slot11_cards, nullptr, false);
	VME_SLOT(config, m_slot[11], slot12_cards, nullptr, false);
}

void ip4_state::machine_start()
{
}

void ip4_state::machine_reset()
{
}

ROM_START(pi4d50)
ROM_END

#define rom_pi4d70 rom_pi4d50

} // anonymous namespace

//   YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY             FULLNAME                   FLAGS
COMP(1987, pi4d50, 0,      0,      pi4d50,  0,     ip4_state, empty_init, "Silicon Graphics", "Professional IRIS 4D/50", 0)
COMP(1987, pi4d70, 0,      0,      pi4d70,  0,     ip4_state, empty_init, "Silicon Graphics", "Professional IRIS 4D/70", 0)
