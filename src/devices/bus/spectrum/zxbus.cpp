// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/*******************************************************************************************

        ZXBUS device

*******************************************************************************************/

#include "emu.h"
#include "zxbus.h"

DEFINE_DEVICE_TYPE(ZXBUS_SLOT, zxbus_slot_device, "zxbus_slot", "ZXBUS slot")

zxbus_slot_device::zxbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	zxbus_slot_device(mconfig, ZXBUS_SLOT, tag, owner, clock)
{
}

zxbus_slot_device::zxbus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_zxbus_bus(*this, finder_base::DUMMY_TAG)
{
}

void zxbus_slot_device::device_start()
{
	device_zxbus_card_interface *const dev = dynamic_cast<device_zxbus_card_interface *>(get_card_device());
	if (dev) dev->set_zxbusbus(m_zxbus_bus);

	// tell zxbus bus that there is one slot with the specified tag
	downcast<zxbus_device &>(*m_zxbus_bus).add_slot(tag());
}


DEFINE_DEVICE_TYPE(ZXBUS, zxbus_device, "zxbus", "ZXBUS bus")

zxbus_device::zxbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	zxbus_device(mconfig, ZXBUS, tag, owner, clock)
{
}

zxbus_device::zxbus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_iospace(*this, finder_base::DUMMY_TAG, -1)
{
}

void zxbus_device::add_slot(const char *tag)
{
	device_t *dev = subdevice(tag);
	add_slot(dynamic_cast<device_slot_interface *>(dev));
}

void zxbus_device::add_slot(device_slot_interface *slot)
{
	m_slot_list.push_front(slot);
}

device_zxbus_card_interface::device_zxbus_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "zxbus"),
		m_zxbus(nullptr), m_zxbus_dev(nullptr)
{
}

void device_zxbus_card_interface::set_zxbus_device()
{
	m_zxbus = dynamic_cast<zxbus_device *>(m_zxbus_dev);
}

#include "neogs.h"

void zxbus_cards(device_slot_interface &device)
{
	device.option_add("neogs", NEOGS);
}
