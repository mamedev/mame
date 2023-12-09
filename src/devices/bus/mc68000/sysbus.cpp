// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    mc-68000-Computer System Bus

***************************************************************************/

#include "emu.h"
#include "sysbus.h"


//**************************************************************************
//  BUS DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(MC68000_SYSBUS, mc68000_sysbus_device, "mc68000_sysbus", "mc-68000 System Bus")

mc68000_sysbus_device::mc68000_sysbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MC68000_SYSBUS, tag, owner, clock),
	m_irq1_cb(*this),
	m_irq2_cb(*this),
	m_irq3_cb(*this),
	m_irq4_cb(*this),
	m_irq5_cb(*this),
	m_irq6_cb(*this),
	m_irq7_cb(*this)
{
}

mc68000_sysbus_device::~mc68000_sysbus_device()
{
}

void mc68000_sysbus_device::add_card(int slot, device_mc68000_sysbus_card_interface *card)
{
	m_device_list[slot] = card;
}

void mc68000_sysbus_device::device_start()
{

	// clear slots
	std::fill(std::begin(m_device_list), std::end(m_device_list), nullptr);
}

// callbacks from slot device to the host
void mc68000_sysbus_device::irq1_w(int state) { m_irq1_cb(state); }
void mc68000_sysbus_device::irq2_w(int state) { m_irq2_cb(state); }
void mc68000_sysbus_device::irq3_w(int state) { m_irq3_cb(state); }
void mc68000_sysbus_device::irq4_w(int state) { m_irq4_cb(state); }
void mc68000_sysbus_device::irq5_w(int state) { m_irq5_cb(state); }
void mc68000_sysbus_device::irq6_w(int state) { m_irq6_cb(state); }
void mc68000_sysbus_device::irq7_w(int state) { m_irq7_cb(state); }

uint16_t mc68000_sysbus_device::slot_r(int slot, offs_t offset, uint16_t mem_mask)
{
	if (m_device_list[slot])
		return m_device_list[slot]->slot_r(offset, mem_mask);
	else
	{
		logerror("Read from unused slot %d: %06x = ffff & %04x\n", slot, offset, mem_mask);
		return 0xffff;
	}
}

void mc68000_sysbus_device::slot_w(int slot, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_device_list[slot])
		m_device_list[slot]->slot_w(offset, data, mem_mask);
	else
		logerror("Write to unused slot %d: %06x = %04x & %04x\n", slot, offset, data, mem_mask);
}

uint16_t mc68000_sysbus_device::floppy_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	for (int i = 0; i < 8; i++)
		if (m_device_list[i])
			data &= m_device_list[i]->floppy_r(offset, mem_mask);

	return data;
}

void mc68000_sysbus_device::floppy_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	for (int i = 0; i < 8; i++)
		if (m_device_list[i])
			m_device_list[i]->floppy_w(offset, data, mem_mask);
}


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(MC68000_SYSBUS_SLOT, mc68000_sysbus_slot_device, "mc68000_sysbus_slot", "mc-68000 System Bus Slot")

mc68000_sysbus_slot_device::mc68000_sysbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mc68000_sysbus_slot_device(mconfig, MC68000_SYSBUS_SLOT, tag, owner, clock)
{
}

mc68000_sysbus_slot_device::mc68000_sysbus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_single_card_slot_interface<device_mc68000_sysbus_card_interface>(mconfig, *this)
{
}

void mc68000_sysbus_slot_device::device_resolve_objects()
{
	device_mc68000_sysbus_card_interface *const card = get_card_device();

	if (card)
	{
		mc68000_sysbus_device *bus = downcast<mc68000_sysbus_device *>(m_owner);
		card->set_bus(bus, tag());
	}
}

void mc68000_sysbus_slot_device::device_start()
{
}


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

device_mc68000_sysbus_card_interface::device_mc68000_sysbus_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "mc68000sysbus"),
	m_bus(nullptr),
	m_next(nullptr),
	m_slot_tag(nullptr),
	m_slot(-1)
{
}

device_mc68000_sysbus_card_interface::~device_mc68000_sysbus_card_interface()
{
}

void device_mc68000_sysbus_card_interface::set_bus(mc68000_sysbus_device *bus, const char *slot_tag)
{
	m_slot_tag = slot_tag;
	m_bus = bus;
}

void device_mc68000_sysbus_card_interface::interface_pre_start()
{
	if (!m_bus)
		fatalerror("mc-68000 System Bus device undefined\n");

	if (m_slot < 0)
	{
		if (!m_bus->started())
			throw device_missing_dependencies();

		// extract the slot number from the last digit of the slot tag
		size_t const len = strlen(m_slot_tag);

		m_slot = (m_slot_tag[len - 1] - '0');
		if (m_slot < 0 || m_slot > 7)
			fatalerror("Slot %x out of range for mc-68000 System Bus\n", m_slot);

		m_bus->add_card(m_slot, this);
	}
}
