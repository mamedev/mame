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

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(DMAC3, dmac3_device, "dmac3", "Sony DMA Controller 3")

dmac3_device::dmac3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DMAC3, tag, owner, clock)
	, m_bus(*this, finder_base::DUMMY_TAG, -1, 64)
	, m_out_int(*this)
	, m_dma_r(*this)
	, m_dma_w(*this)
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
