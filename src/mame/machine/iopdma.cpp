// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 IOP DMA device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "iopdma.h"

DEFINE_DEVICE_TYPE(SONYIOP_DMA, iop_dma_device, "iopdma", "Sony IOP DMA")

iop_dma_device::iop_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYIOP_DMA, tag, owner, clock)
{
}

void iop_dma_device::device_start()
{
}

void iop_dma_device::device_reset()
{
	m_dpcr[0] = 0;
	m_dpcr[1] = 0;
	m_dicr[0] = 0;
	m_dicr[1] = 0;
}

READ32_MEMBER(iop_dma_device::ctrl0_r)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0: // 0x1f8010f0, DPCR
			ret = m_dpcr[0];
			logerror("%s: ctrl0_r: DPCR (%08x)\n", machine().describe_context(), ret);
			break;
		case 1: // 0x1f8010f4, DICR
			ret = m_dicr[0];
			logerror("%s: ctrl0_r: DICR (%08x)\n", machine().describe_context(), ret);
			break;
		default:
			// Can't happen
			break;
	}
	return ret;
}

WRITE32_MEMBER(iop_dma_device::ctrl0_w)
{
	switch (offset)
	{
		case 0: // 0x1f8010f0, DPCR
			logerror("%s: ctrl0_w: DPCR = %08x\n", machine().describe_context(), data);
			set_dpcr(data, 0);
			break;
		case 1: // 0x1f8010f4, DICR
			logerror("%s: ctrl0_w: DICR = %08x\n", machine().describe_context(), data);
			set_dicr(data, 0);
			break;
		default:
			// Can't happen
			break;
	}
}

READ32_MEMBER(iop_dma_device::ctrl1_r)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0: // 0x1f801570, DPCR2
			ret = m_dpcr[1];
			logerror("%s: ctrl1_r: DPCR2 (%08x)\n", machine().describe_context(), ret);
			break;
		case 1: // 0x1f801574, DICR2
			ret = m_dicr[1];
			logerror("%s: ctrl1_r: DICR2 (%08x)\n", machine().describe_context(), ret);
			break;
		default:
			// Can't happen
			break;
	}
	return ret;
}

WRITE32_MEMBER(iop_dma_device::ctrl1_w)
{
	switch (offset)
	{
		case 0: // 0x1f8010f0, DPCR
			logerror("%s: ctrl1_w: DPCR2 = %08x\n", machine().describe_context(), data);
			set_dpcr(data, 1);
			break;
		case 1: // 0x1f8010f4, DICR
			logerror("%s: ctrl1_w: DICR2 = %08x\n", machine().describe_context(), data);
			set_dicr(data, 1);
			break;
		default:
			// Can't happen
			break;
	}
}

void iop_dma_device::update_interrupts()
{
	// TODO
}

void iop_dma_device::set_dpcr(uint32_t data, uint32_t index)
{
	m_dpcr[index] = data;
	for (uint32_t channel = index*8, bit = 0; channel < index*8 + 8; channel++, bit += 4)
	{
		const uint8_t field = (data >> bit) & 0xf;
		const bool was_enabled = m_channels[channel].m_enabled;
		const bool is_enabled = BIT(field, 3);
		m_channels[channel].m_enabled = is_enabled;
		m_channels[channel].m_priority = field & 7;
		if (!was_enabled && is_enabled)
		{
			// Check for running status?
		}
	}
}

void iop_dma_device::set_dicr(uint32_t data, uint32_t index)
{
	m_dicr[index] = data;
	m_int_ctrl[index].m_mask = (data >> 16) & 0x7f;
	m_int_ctrl[index].m_status &= ~((data >> 24) & 0x7f);
	m_int_ctrl[index].m_enabled = BIT(data, 23);
	update_interrupts();
}
