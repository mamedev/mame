// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay, Brice Onken

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

#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(DMAC3, dmac3_device, "dmac3", "Sony CXD8403Q DMA Controller")

dmac3_device::dmac3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DMAC3, tag, owner, clock), m_bus(*this, finder_base::DUMMY_TAG, -1, 64), m_out_int(*this), m_dma_r(*this), m_dma_w(*this)
{
}

void dmac3_device::map_dma_ram(address_map &map)
{
	// Host platform configures the use of RAM at device attach
	map(0x0, map_ram_size - 1).ram();
}

void dmac3_device::device_start()
{
	m_out_int.resolve();

	m_dma_r.resolve_all_safe(0);
	m_dma_w.resolve_all_safe();

	m_irq_check = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dmac3_device::irq_check), this));
	m_dma_check = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dmac3_device::dma_check), this));

	m_out_int_state = false;
}

void dmac3_device::device_reset()
{
	m_irq_check->adjust(attotime::zero);
}

void dmac3_device::set_irq_line(int number, int state)
{
	/*
	u8 const mask = 1U << (number * 2);

	if (state)
		m_gstat |= mask;
	else
		m_gstat &= ~mask;

	m_irq_check->adjust(attotime::zero);
	*/
}

void dmac3_device::irq_check(void *ptr, s32 param)
{
	/*
	bool const out_int_stat = bool(m_gstat & 0x55);

	if (out_int_stat != m_out_int_state)
	{
		m_out_int_state = out_int_stat;
		m_out_int(m_out_int_state);
	}
	*/
}

void dmac3_device::set_drq_line(int channel, int state)
{
	/*
	u8 const mask = 1U << ((channel * 2) + 1);

	if (state)
		m_gstat |= mask;
	else
		m_gstat &= ~mask;

	if (state)
		m_dma_check->adjust(attotime::zero);
	*/
}

void dmac3_device::dma_check(void *ptr, s32 param)
{
	/*
	bool active = false;

	for (unsigned channel = 0; channel < 4; channel++)
	{
		// check drq active
		if (!BIT(m_gstat, (channel * 2) + 1))
			continue;

		dma_channel &dma = m_channel[channel];

		// check channel enabled
		if (!(dma.cctl & CS_ENABLE))
			return;

		// check transfer count
		if (!dma.ctrc)
			return;

		// TODO: confirm if this is correct
		u32 const address = u32(dma.cmap[dma.ctag]) << 12 | dma.cofs;

		// perform dma transfer
		if (dma.cctl & CS_MODE)
		{
			// device to memory
			u8 const data = m_dma_r[channel]();

			LOG("dma_r data 0x%02x address 0x%08x\n", data, address);

			m_bus->write_byte(address, data);
		}
		else
		{
			// memory to device
			u8 const data = m_bus->read_byte(address);

			LOG("dma_w data 0x%02x address 0x%08x\n", data, address);

			m_dma_w[channel](data);
		}

		// increment offset
		if (dma.cofs == 0xfff)
		{
			// advance to next page
			dma.cofs = 0;
			dma.ctag++;
		}
		else
			dma.cofs++;

		// decrement count
		dma.ctrc--;

		// set terminal count flag
		if (!dma.ctrc)
		{
			LOG("transfer complete\n");
			dma.cstat |= CS_TCZ;

			// TODO: terminal count interrupt?
		}

		if (BIT(m_gstat, (channel * 2) + 1))
			active = true;
	}

	if (active)
		m_dma_check->adjust(attotime::zero);
	*/
}

uint32_t dmac3_device::cstat_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].cstat;
	LOG("dmac3-%d cstat_r: 0x%x\n", controller, val);
	return val;
}
uint32_t dmac3_device::ictl_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].ictl;
	// hack
	val |= 0x1;
	LOG("dmac3-%d ictl_r: 0x%x\n", controller, val);
	return val;
}
uint32_t dmac3_device::trc_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].trc;
	LOG("dmac3-%d trc_r: 0x%x\n", controller, val);
	return val;
}
uint32_t dmac3_device::tra_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].tra;
	LOG("dmac3-%d tra_r: 0x%x\n", controller, val);
	return val;
}
uint32_t dmac3_device::cnf_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].cnf;
	LOG("dmac3-%d cnf_r: 0x%x\n", controller, val);
	return val;
}

void dmac3_device::cstat_w(DMAC3_Controller controller, uint32_t data)
{
	LOG("dmac3-%d cstat_w: 0x%x\n", controller, data);
	m_controllers[controller].cstat = data;
}

void dmac3_device::ictl_w(DMAC3_Controller controller, uint32_t data)
{
	LOG("dmac3-%d ictl_w: 0x%x\n", controller, data);
	m_controllers[controller].ictl = data;
}

void dmac3_device::trc_w(DMAC3_Controller controller, uint32_t data)
{
	LOG("dmac3-%d trc_w: 0x%x\n", controller, data);
	m_controllers[controller].trc = data;
}

void dmac3_device::tra_w(DMAC3_Controller controller, uint32_t data)
{
	LOG("dmac3-%d tra_w: 0x%x\n", controller, data);
	m_controllers[controller].tra = data;
}

void dmac3_device::cnf_w(DMAC3_Controller controller, uint32_t data)
{
	// Log is polluted with switching between SPIFI3 and regular mode
	// Will probably remove the if at some point, but we can mostly trust all 3
	// DMAC+SPIFI3 users (MROM, NEWS-OS, and NetBSD) to follow this correctly
	if(data != CNF_FASTACCESS && data != CNF_SLOWACCESS)
	{
		LOG("dmac3-%d cnf_w: 0x%x\n", controller, data);
	}
	m_controllers[controller].cnf = data;
}
