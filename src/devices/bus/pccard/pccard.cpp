// license:GPL-2.0+
// copyright-holders:smf
#include "emu.h"
#include "pccard.h"

//#define VERBOSE 1
#include "logmacro.h"

device_pccard_interface::device_pccard_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "pccard"),
	m_cd1_cb(*this),
	m_cd2_cb(*this),
	m_bvd1_cb(*this),
	m_bvd2_cb(*this),
	m_wp_cb(*this)
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

pccard_slot_device::pccard_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PCCARD_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_pccard_interface>(mconfig, *this),
	device_pccard_interface(mconfig, *this)
{
}

void pccard_slot_device::device_config_complete()
{
	m_dev = get_card_device();
	if (m_dev)
	{
		m_dev->cd1().set(*this, FUNC(pccard_slot_device::update_cd1));
		m_dev->cd2().set(*this, FUNC(pccard_slot_device::update_cd2));
		m_dev->bvd1().set(*this, FUNC(pccard_slot_device::update_bvd1));
		m_dev->bvd2().set(*this, FUNC(pccard_slot_device::update_bvd2));
		m_dev->wp().set(*this, FUNC(pccard_slot_device::update_wp));
	}
}

void pccard_slot_device::device_start()
{
}

uint16_t pccard_slot_device::read_memory(offs_t offset, uint16_t mem_mask)
{
	if (m_dev)
		return m_dev->read_memory(offset, mem_mask);
	else
		return 0xffff;
}

void pccard_slot_device::write_memory(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_dev)
		m_dev->write_memory(offset, data, mem_mask);
}

uint16_t pccard_slot_device::read_reg(offs_t offset, uint16_t mem_mask)
{
	if (m_dev)
		return m_dev->read_reg(offset, mem_mask);
	else
		return 0xffff;
}

void pccard_slot_device::write_reg(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_dev)
		m_dev->write_reg(offset, data, mem_mask);
}

void pccard_slot_device::update_cd1(int state)
{
	m_cd1_cb(state);
}

void pccard_slot_device::update_cd2(int state)
{
	m_cd2_cb(state);
}

void pccard_slot_device::update_bvd1(int state)
{
	m_bvd1_cb(state);
}

void pccard_slot_device::update_bvd2(int state)
{
	m_bvd2_cb(state);
}

void pccard_slot_device::update_wp(int state)
{
	m_wp_cb(state);
}

DEFINE_DEVICE_TYPE(PCCARD_SLOT, pccard_slot_device, "pccard", "PC Card Slot")
