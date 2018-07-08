// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 IOP SPU device skeleton
*
*   To Do:
*     Everything
*
*/

#include "iopspu.h"

DEFINE_DEVICE_TYPE(SONYIOP_SPU, iop_spu_device, "iopspu", "Playstation 2 IOP SPU")

iop_spu_device::iop_spu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYIOP_SPU, tag, owner, clock)
	, m_iop(*this, finder_base::DUMMY_TAG)
{
}

void iop_spu_device::device_start()
{
	m_ram = std::make_unique<uint16_t[]>(2 * 1024 * 1024); // ?
}

void iop_spu_device::device_reset()
{
	m_status = 0;
	m_start_port_addr = 0;
	m_curr_port_addr = 0;
	m_unknown_0x19a = 0;
}

void iop_spu_device::dma_write(uint32_t data)
{
	//m_ram[m_curr_port_addr] = (uint16_t)(data >> 16);
	m_curr_port_addr++;
	//m_ram[m_curr_port_addr] = (uint16_t)data;
	m_curr_port_addr++;

	m_status &= ~STATUS_DMA_DONE;
	m_status |= STATUS_DMA_ACTIVE;
}

void iop_spu_device::dma_done()
{
	m_status |= STATUS_DMA_DONE;
	m_status &= ~STATUS_DMA_ACTIVE;
}

uint16_t iop_spu_device::port_read()
{
	uint16_t ret = m_ram[m_curr_port_addr];
	m_curr_port_addr++;
	return ret;
}

void iop_spu_device::port_write(uint16_t data)
{
	m_ram[m_curr_port_addr] = data;
	m_curr_port_addr++;
}

READ16_MEMBER(iop_spu_device::read)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0x19a/2:
			ret = m_unknown_0x19a;
			logerror("%s: read: Unknown 0x19a (%04x & %04x)\n", machine().describe_context(), ret, mem_mask);
			break;

        case 0x1a8/2:
            ret = (uint16_t)(m_start_port_addr >> 16);
			logerror("%s: read: PORT_ADDR_HI: %04x & %04x\n", machine().describe_context(), ret, mem_mask);
            break;

        case 0x1aa/2:
            ret = (uint16_t)m_start_port_addr;
			logerror("%s: read: PORT_ADDR_LO: %04x & %04x\n", machine().describe_context(), ret, mem_mask);
            break;

		case 0x344/2:
			ret = m_status;
			logerror("%s: read: STATUS: %04x & %04x\n", machine().describe_context(), ret, mem_mask);
			break;

		default:
			logerror("%s: read: Unknown %08x (%04x)\n", machine().describe_context(), 0x1f900000 + (offset << 1), mem_mask);
			break;
	}
	return ret;
}

WRITE16_MEMBER(iop_spu_device::write)
{
	switch (offset)
	{
		case 0x19a/2:
			logerror("%s: write: Unknown 0x19a = %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_unknown_0x19a = data;
			break;

        case 0x1a8/2:
			logerror("%s: write: PORT_ADDR_HI: %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_start_port_addr &= ~0x000f0000;
			m_start_port_addr |= ((uint32_t)data << 16) & 0x000f0000;
			m_curr_port_addr = m_start_port_addr;
            break;

        case 0x1aa/2:
			logerror("%s: write: PORT_ADDR_LO: %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_start_port_addr &= ~0x0000ffff;
			m_start_port_addr |= data;
			m_curr_port_addr = m_start_port_addr;
            break;

		default:
			logerror("%s: write: Unknown %08x = %04x & %04x\n", machine().describe_context(), 0x1f900000 + (offset << 1), data, mem_mask);
			break;
	}
}
