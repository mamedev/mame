// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/*******************************************************************************************

        ZXBUS device

*******************************************************************************************/

#include "emu.h"
#include "bus.h"

DEFINE_DEVICE_TYPE(ZXBUS_SLOT, zxbus_slot_device, "zxbus_slot", "ZXBUS slot")

zxbus_slot_device::zxbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: zxbus_slot_device(mconfig, ZXBUS_SLOT, tag, owner, clock)
{
}

zxbus_slot_device::zxbus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_single_card_slot_interface<device_zxbus_card_interface>(mconfig, *this)
	, m_zxbus_bus(*this, finder_base::DUMMY_TAG)
{
}

void zxbus_slot_device::device_start()
{
	device_zxbus_card_interface *const card(get_card_device());
	if (card) card->set_zxbusbus(*m_zxbus_bus);

	m_zxbus_bus->add_slot(*this);
}

DEFINE_DEVICE_TYPE(ZXBUS, zxbus_device, "zxbus", "ZXBUS bus")

zxbus_device::zxbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: zxbus_device(mconfig, ZXBUS, tag, owner, clock)
{
}

zxbus_device::zxbus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_io(nullptr)
	, m_shadow_io(nullptr)
{
}

void zxbus_device::device_start()
{
}

void zxbus_device::add_slot(zxbus_slot_device &slot)
{
	m_slot_list.push_front(&slot);

	device_zxbus_card_interface *card = slot.get_card_device();
	if (card)
	{
		if (m_io)
			m_io->install_device(0x0000, 0xffff, *card, &device_zxbus_card_interface::io_map);
		if (m_shadow_io)
			m_shadow_io->install_device(0x0000, 0xffff, *card, &device_zxbus_card_interface::shadow_io_map);
	}
}

device_zxbus_card_interface::device_zxbus_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "zxbus")
	, m_zxbus(nullptr)
{
}

void device_zxbus_card_interface::interface_pre_start()
{
	if (!m_zxbus)
		throw device_missing_dependencies();
}


#include "nemoide.h"
#include "neogs.h"
#include "smuc.h"

void zxbus_cards(device_slot_interface &device)
{
	device.option_add("nemoide", ZXBUS_NEMOIDE);
	device.option_add("neogs", ZXBUS_NEOGS);
}

void zxbus_gmx_cards(device_slot_interface &device)
{
	zxbus_cards(device);
	device.option_add("smuc", ZXBUS_SMUC);
}
