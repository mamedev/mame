// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Nokia MikroMikko 2 expansion bus

***************************************************************************/

#include "emu.h"
#include "exp.h"
#include "mmc186.h"
#include "crtc186.h"

DEFINE_DEVICE_TYPE(MIKROMIKKO2_EXPANSION_BUS_SLOT, mikromikko2_expansion_bus_slot_device, "mikromikko2_expansion_bus_slot", "Nokia MikroMikko 2 expansion bus slot")

mikromikko2_expansion_bus_slot_device::mikromikko2_expansion_bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mikromikko2_expansion_bus_slot_device(mconfig, MIKROMIKKO2_EXPANSION_BUS_SLOT, tag, owner, clock)
{
}

mikromikko2_expansion_bus_slot_device::mikromikko2_expansion_bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_bus(*this, finder_base::DUMMY_TAG)
{
}

void mikromikko2_expansion_bus_slot_device::device_start()
{
	auto *const dev = dynamic_cast<device_mikromikko2_expansion_bus_card_interface *>(get_card_device());

	if (dev)
		dev->set_bus(m_bus);
}

DEFINE_DEVICE_TYPE(MIKROMIKKO2_EXPANSION_BUS, mikromikko2_expansion_bus_device, "mikromikko2_expansion_bus", "Nokia MikroMikko 2 expansion bus")

mikromikko2_expansion_bus_device::mikromikko2_expansion_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mikromikko2_expansion_bus_device(mconfig, MIKROMIKKO2_EXPANSION_BUS, tag, owner, clock)
{
}

mikromikko2_expansion_bus_device::mikromikko2_expansion_bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_memspace(*this, finder_base::DUMMY_TAG, -1),
	m_iospace(*this, finder_base::DUMMY_TAG, -1),
	m_out_nmi_cb(*this),
	m_out_ir2_cb(*this),
	m_out_ir3_cb(*this),
	m_out_ir4_cb(*this),
	m_out_ir5_cb(*this),
	m_out_ir6_cb(*this),
	m_out_hold1_cb(*this),
	m_out_hold2_cb(*this),
	m_out_hold3_cb(*this),
	m_out_hold4_cb(*this),
	m_out_hold5_cb(*this)
{
}

void mikromikko2_expansion_bus_device::device_start()
{
}

device_mikromikko2_expansion_bus_card_interface::device_mikromikko2_expansion_bus_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "mikromikko2_expansion_bus"),
	m_bus(nullptr),
	m_card(&device)
{
}

void mikromikko2_expansion_bus_cards(device_slot_interface &device)
{
	device.option_add("crtc186", NOKIA_CRTC186);
	device.option_add("mmc186", NOKIA_MMC186);
}
