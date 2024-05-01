// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "pccard.h"

//#define VERBOSE 1
#include "logmacro.h"


device_pccard_interface::device_pccard_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "pccard"),
	m_slot(dynamic_cast<pccard_slot_device *>(device.owner()))
{
}

uint16_t device_pccard_interface::read_memory(offs_t offset, uint16_t mem_mask)
{
	if (VERBOSE & LOG_GENERAL)
		device().logerror("unhandled memory read %08x %04x\n", offset, mem_mask);
	return 0xffff;
}

void device_pccard_interface::write_memory(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (VERBOSE & LOG_GENERAL)
		device().logerror("unhandled memory write %08x %04x %04x\n", offset, data, mem_mask);
}

uint16_t device_pccard_interface::read_reg(offs_t offset, uint16_t mem_mask)
{
	if (VERBOSE & LOG_GENERAL)
		device().logerror("unhandled register read %08x %04x\n", offset, mem_mask);
	return 0xffff;
}

void device_pccard_interface::write_reg(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (VERBOSE & LOG_GENERAL)
		device().logerror("unhandled register write %08x %04x %04x\n", offset, data, mem_mask);
}

DEFINE_DEVICE_TYPE(PCCARD_SLOT, pccard_slot_device, "pccard", "PC Card Slot")

pccard_slot_device::pccard_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PCCARD_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_pccard_interface>(mconfig, *this),
	m_card_detect_cb(*this),
	m_battery_voltage_1_cb(*this),
	m_battery_voltage_2_cb(*this),
	m_write_protect_cb(*this),
	m_pccard(nullptr)
{
}

void pccard_slot_device::device_start()
{
	m_pccard = get_card_device();
}

uint16_t pccard_slot_device::read_memory(offs_t offset, uint16_t mem_mask)
{
	if (m_pccard)
		return m_pccard->read_memory(offset, mem_mask);
	else
		return 0xffff;
}

void pccard_slot_device::write_memory(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_pccard)
		m_pccard->write_memory(offset, data, mem_mask);
}

uint16_t pccard_slot_device::read_reg(offs_t offset, uint16_t mem_mask)
{
	if (m_pccard)
		return m_pccard->read_reg(offset, mem_mask);
	else
		return 0xffff;
}

void pccard_slot_device::write_reg(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_pccard)
		m_pccard->write_reg(offset, data, mem_mask);
}

uint8_t pccard_slot_device::read_memory_byte(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_pccard)
	{
		if (BIT(offset, 0))
			data = m_pccard->read_memory(offset / 2, 0xff00) >> 8;
		else
			data = m_pccard->read_memory(offset / 2, 0x00ff) >> 0;
	}

	return data;
}

uint8_t pccard_slot_device::read_reg_byte(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_pccard)
	{
		if (BIT(offset, 0))
			data = m_pccard->read_reg(offset / 2, 0xff00) >> 8;
		else
			data = m_pccard->read_reg(offset / 2, 0x00ff) >> 0;
	}

	return data;
}

void pccard_slot_device::write_memory_byte(offs_t offset, uint8_t data)
{
	if (m_pccard)
	{
		if (BIT(offset, 0))
			m_pccard->write_memory(offset / 2, data << 8, 0xff00);
		else
			m_pccard->write_memory(offset / 2, data << 0, 0x00ff);
	}
}

void pccard_slot_device::write_reg_byte(offs_t offset, uint8_t data)
{
	if (m_pccard)
	{
		if (BIT(offset, 0))
			m_pccard->write_reg(offset / 2, data << 8, 0xff00);
		else
			m_pccard->write_reg(offset / 2, data << 0, 0x00ff);
	}
}
