// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "pccard.h"

//#define VERBOSE 1
#include "logmacro.h"


device_pccard_interface::device_pccard_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "pccard")
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
	m_pccard(nullptr)
{
}

void pccard_slot_device::device_start()
{
	m_pccard = get_card_device();
}

READ_LINE_MEMBER(pccard_slot_device::read_line_inserted)
{
	return m_pccard ? 1 : 0;
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
