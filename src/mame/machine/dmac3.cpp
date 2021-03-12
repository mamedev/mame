// license:BSD-3-Clause
// copyright-holders:Brice Onken
// thanks-to:Patrick Mackinlay

/*
 * Sony NEWS DMAC3 DMA controller
 *
 * References:
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/dmac3reg.h
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/dmac3var.h
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/dmac3.c
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifi.c
 *
 * TODO:
 *  - Almost everything
 */

#include "emu.h"
#include "dmac3.h"

#include "logmacro.h"

DEFINE_DEVICE_TYPE(DMAC3, dmac3_device, "dmac3", "Sony CXD8403Q DMA Controller")

dmac3_device::dmac3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DMAC3, tag, owner, clock)
{
}

void dmac3_device::map_dma_ram(address_map &map)
{
	// Host platform configures the use of RAM at device attach
	map(0x0, map_ram_size - 1).ram();
}

uint32_t dmac3_device::csr_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].csr;
	LOG("dmac3-%d csr_r: 0x%x\n", controller, val);
	return val;
}
uint32_t dmac3_device::intr_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].intr;
	// hack
	val |= 0x1;
	LOG("dmac3-%d intr_r: 0x%x\n", controller, val);
	return val;
}
uint32_t dmac3_device::length_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].length;
	LOG("dmac3-%d length_r: 0x%x\n", controller, val);
	return val;
}
uint32_t dmac3_device::address_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].address;
	LOG("dmac3-%d address_r: 0x%x\n", controller, val);
	return val;
}
uint32_t dmac3_device::conf_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].conf;
	LOG("dmac3-%d conf_r: 0x%x\n", controller, val);
	return val;
}

void dmac3_device::csr_w(DMAC3_Controller controller, uint32_t data)
{
	LOG("dmac3-%d csr_w: 0x%x\n", controller, data);
	m_controllers[controller].csr = data;
}

void dmac3_device::intr_w(DMAC3_Controller controller, uint32_t data)
{
	LOG("dmac3-%d intr_w: 0x%x\n", controller, data);
	m_controllers[controller].intr = data;
}

void dmac3_device::length_w(DMAC3_Controller controller, uint32_t data)
{
	LOG("dmac3-%d length_w: 0x%x\n", controller, data);
	m_controllers[controller].length = data;
}

void dmac3_device::address_w(DMAC3_Controller controller, uint32_t data)
{
	LOG("dmac3-%d address_w: 0x%x\n", controller, data);
	m_controllers[controller].address = data;
}

void dmac3_device::conf_w(DMAC3_Controller controller, uint32_t data)
{
	// Log is polluted with switching between SPIFI3 and regular mode
	// Will probably remove the if at some point, but we can mostly trust all 3
	// DMAC+SPIFI3 users (MROM, NEWS-OS, and NetBSD) to follow this correctly
	if(data != CONF_FASTACCESS && data != CONF_SLOWACCESS)
	{
		LOG("dmac3-%d conf_w: 0x%x\n", controller, data);
	}
	m_controllers[controller].conf = data;
}
