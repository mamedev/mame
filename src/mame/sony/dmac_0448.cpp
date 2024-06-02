// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Sony NEWS DMAC 0448 device.
 *
 * Sources:
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/dev/dmac_0448.h
 *
 * TODO:
 *  - 16 and 32 bit transfers
 *  - terminal count handling
 *  - save state
 */

#include "emu.h"
#include "dmac_0448.h"

#define LOG_REGISTER (1U << 1)
#define LOG_DRQ (1U << 2)
#define LOG_IRQ (1U << 3)

#define VERBOSE 0 // (LOG_GENERAL | LOG_REGISTER | LOG_DRQ | LOG_IRQ)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(DMAC_0448, dmac_0448_device, "dmac_0448", "Sony DMA Controller 0448")

dmac_0448_device::dmac_0448_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DMAC_0448, tag, owner, clock)
	, m_bus(*this, finder_base::DUMMY_TAG, -1, 32)
	, m_out_int(*this)
	, m_dma_r(*this, 0)
	, m_dma_w(*this)
{
}

void dmac_0448_device::map(address_map &map)
{
	map(0x2, 0x2).r(FUNC(dmac_0448_device::cstat_r));
	map(0x3, 0x3).w(FUNC(dmac_0448_device::cctl_w));
	map(0x4, 0x4).rw(FUNC(dmac_0448_device::ctrcl_r), FUNC(dmac_0448_device::ctrcl_w));
	map(0x5, 0x5).rw(FUNC(dmac_0448_device::ctrcm_r), FUNC(dmac_0448_device::ctrcm_w));
	map(0x6, 0x6).rw(FUNC(dmac_0448_device::ctrch_r), FUNC(dmac_0448_device::ctrch_w));
	map(0x7, 0x7).rw(FUNC(dmac_0448_device::ctag_r), FUNC(dmac_0448_device::ctag_w));
	map(0x8, 0x8).rw(FUNC(dmac_0448_device::cwid_r), FUNC(dmac_0448_device::cwid_w));
	map(0x9, 0x9).rw(FUNC(dmac_0448_device::cofsl_r), FUNC(dmac_0448_device::cofsl_w));
	map(0xa, 0xa).rw(FUNC(dmac_0448_device::cofsh_r), FUNC(dmac_0448_device::cofsh_w));

	map(0xc, 0xd).rw(FUNC(dmac_0448_device::cmap_r), FUNC(dmac_0448_device::cmap_w));
	map(0xe, 0xe).w(FUNC(dmac_0448_device::gsel_w));
	map(0xf, 0xf).r(FUNC(dmac_0448_device::gstat_r));
}

void dmac_0448_device::device_start()
{
	m_irq_check = timer_alloc(FUNC(dmac_0448_device::irq_check), this);
	m_dma_check = timer_alloc(FUNC(dmac_0448_device::dma_check), this);

	m_out_int_state = false;
	m_gsel = 0;
	m_gstat = 0;
}

void dmac_0448_device::device_reset()
{
	m_irq_check->adjust(attotime::zero);
}

void dmac_0448_device::set_irq_line(int number, int state)
{
	LOGMASKED(LOG_IRQ, "set irq %d to %d\n", number, state);
	u8 const mask = 1U << (number * 2);

	if (state)
	{
		m_gstat |= mask;
		// If the channel is enabled and has autopad enabled, the transaction must be terminated at this time
		m_dma_check->adjust(attotime::zero);
	}
	else
		m_gstat &= ~mask;

	m_irq_check->adjust(attotime::zero);
}

void dmac_0448_device::irq_check(s32 param)
{
	bool const out_int_stat = bool(m_gstat & 0x55);

	if (out_int_stat != m_out_int_state)
	{
		LOGMASKED(LOG_IRQ, "update irq to %d\n", out_int_stat);
		m_out_int_state = out_int_stat;
		m_out_int(m_out_int_state);
	}
}

void dmac_0448_device::set_drq_line(int channel, int state)
{
	u8 const mask = 1U << ((channel * 2) + 1);

	if (state) {
		LOGMASKED(LOG_DRQ, "set DRQ 0x%x\n", mask);
		m_gstat |= mask;
	} else {
		LOGMASKED(LOG_DRQ, "clear DRQ 0x%x\n", mask);
		m_gstat &= ~mask;
	}

	if (state)
		m_dma_check->adjust(attotime::zero);
}

void dmac_0448_device::cctl_w(u8 data)
{
	LOGMASKED(LOG_REGISTER, "cctl_w 0x%x (%s)\n", data, machine().describe_context());
	if ((data & CS_ENABLE) && !(m_channel[m_gsel].cctl & CS_ENABLE))
	{
		LOG("transfer started address 0x%08x count 0x%x\n",
			u32(m_channel[m_gsel].cmap[m_channel[m_gsel].ctag]) << 12 | m_channel[m_gsel].cofs, m_channel[m_gsel].ctrc);
	}
	m_channel[m_gsel].cctl = data;

	m_dma_check->adjust(attotime::zero);
}

void dmac_0448_device::gsel_w(u8 data)
{
	LOGMASKED(LOG_REGISTER, "gsel_w 0x%x (%s)\n", data, machine().describe_context());
	m_gsel = data;
}

void dmac_0448_device::dma_check(s32 param)
{
	bool active = false;

	for (unsigned channel = 0; channel < 4; channel++)
	{
		// check drq active
		dma_channel &dma = m_channel[channel];
		if (channel == 0)
		{
			LOG("dma_check cctl = 0x%x gstat = 0x%x ctrc = 0x%x\n", dma.cctl, m_gstat, dma.ctrc); 
		}

		// if (!BIT(m_gstat, (channel * 2) + 1) && !((dma.cctl & 0x10) && (m_gstat & (1 << (2* channel)))) && !((m_gstat & 0x1) && (dma.cctl & 0x10))) // TODO: don't hardcode channel
		// 	continue;
		
		if (!BIT(m_gstat, (channel * 2) + 1))
			continue;

		// check channel enabled
		if (!(dma.cctl & CS_ENABLE))
			return;

		// check transfer count
		if (!dma.ctrc && !(dma.cctl & 0x10))
			return;

		// TODO: confirm if this is correct
		u32 const address = u32(dma.cmap[dma.ctag]) << 12 | dma.cofs;

		if ((m_gstat & (1 << (2* channel))) && (dma.cctl & 0x10)) // autopad
		{
			if (dma.cctl & CS_MODE)
			{
				LOG("dma_r data autopad address 0x%08x\n", address);
				m_bus->write_byte(address, 0x00);
			}
			else
			{
				LOG("dma_w data autopad address 0x%08x\n", address);
				m_dma_w[channel](0x00);
			}
		}
		else
		{
			// perform dma transfer
			if (dma.cctl & CS_MODE)
			{
				// device to memory
				u8 const data = m_dma_r[channel]();

				// LOG("dma_r data 0x%02x address 0x%08x\n", data, address);

				m_bus->write_byte(address, data);
			}
			else
			{
				// memory to device
				u8 const data = m_bus->read_byte(address);

				// LOG("dma_w data 0x%02x address 0x%08x\n", data, address);

				m_dma_w[channel](data);
			}
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
		if (dma.ctrc > 0) {
			dma.ctrc--;
		}
		LOG("ctrc now 0x%x\n", dma.ctrc);

		// set terminal count flag
		if (!dma.ctrc)
		{
			LOG("transfer complete\n");
			dma.cstat |= CS_TCZ;
			LOG("dma_check channel %d cstat = 0x%x\n", channel, dma.cstat); 
			// TODO: terminal count interrupt?
		}
		else
		{
			// LOG("Not done, clearing TCZ for ch %d\n", channel);
			dma.cstat &= ~CS_TCZ;
		}

		if (BIT(m_gstat, (channel * 2) + 1) || (dma.ctrc && (dma.cctl & 0x10)))
			active = true;
	}

	if (active)
		m_dma_check->adjust(attotime::zero);
}
