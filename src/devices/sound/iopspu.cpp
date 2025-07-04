// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 IOP SPU device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "iopspu.h"

DEFINE_DEVICE_TYPE(SONYIOP_SPU, iop_spu_device, "iopspu", "PlayStation 2 IOP SPU")

iop_spu_device::iop_spu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYIOP_SPU, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_iop(*this, finder_base::DUMMY_TAG)
	, m_intc(*this, finder_base::DUMMY_TAG)
{
}

iop_spu_device::~iop_spu_device()
{
}

void iop_spu_device::device_start()
{
	m_ram = std::make_unique<uint16_t[]>(2 * 1024 * 1024); // ?

	if (!m_core[0].m_autodma_done_timer_hack)
		m_core[0].m_autodma_done_timer_hack = timer_alloc(FUNC(iop_spu_device::autodma_done_timer_hack), this);

	if (!m_core[1].m_autodma_done_timer_hack)
		m_core[1].m_autodma_done_timer_hack = timer_alloc(FUNC(iop_spu_device::autodma_done_timer_hack), this);

	save_item(NAME(m_core[0].m_status));
	save_item(NAME(m_core[0].m_start_port_addr));
	save_item(NAME(m_core[0].m_curr_port_addr));
	save_item(NAME(m_core[0].m_unknown_0x19a));
	save_item(NAME(m_core[0].m_autodma_ctrl));
	save_item(NAME(m_core[1].m_status));
	save_item(NAME(m_core[1].m_start_port_addr));
	save_item(NAME(m_core[1].m_curr_port_addr));
	save_item(NAME(m_core[1].m_unknown_0x19a));
	save_item(NAME(m_core[1].m_autodma_ctrl));
}

void iop_spu_device::device_reset()
{
	m_core[0].m_status = 0;
	m_core[0].m_start_port_addr = 0;
	m_core[0].m_curr_port_addr = 0;
	m_core[0].m_unknown_0x19a = 0;
	m_core[0].m_autodma_ctrl = 0;
	m_core[1].m_status = 0;
	m_core[1].m_start_port_addr = 0;
	m_core[1].m_curr_port_addr = 0;
	m_core[1].m_unknown_0x19a = 0;
	m_core[1].m_autodma_ctrl = 0;
}

void iop_spu_device::dma_write(int bank, uint32_t data)
{
	iop_spu_core_t &core = m_core[bank];
	//m_ram[core.m_curr_port_addr] = (uint16_t)(data >> 16);
	core.m_curr_port_addr++;
	//m_ram[core.m_curr_port_addr] = (uint16_t)data;
	core.m_curr_port_addr++;

	if (core.m_curr_port_addr >= 0xfffff)
		core.m_curr_port_addr = 0x2800 >> 1;

	core.m_status &= ~STATUS_DMA_DONE;
	core.m_status |= STATUS_DMA_ACTIVE;

	if ((core.m_autodma_ctrl & (1 << bank)) && core.m_autodma_done_timer_hack->expire().is_never())
	{
		core.m_autodma_done_timer_hack->adjust(attotime::from_ticks(256, 48000), bank); // 256 halfwords * assumed 48kHz sampling rate
	}
}

void iop_spu_device::dma_done(int bank)
{
	iop_spu_core_t &core = m_core[bank];
	core.m_status |= STATUS_DMA_DONE;
	core.m_status &= ~STATUS_DMA_ACTIVE;
}

void iop_spu_device::sound_stream_update(sound_stream &stream)
{
	// TODO
}

TIMER_CALLBACK_MEMBER(iop_spu_device::autodma_done_timer_hack)
{
	m_core[param].m_autodma_done_timer_hack->adjust(attotime::never);
	m_intc->raise_interrupt(iop_intc_device::INT_SPU);
}

uint16_t iop_spu_device::port_read(int bank)
{
	iop_spu_core_t &core = m_core[bank];
	const uint16_t ret = m_ram[core.m_curr_port_addr];
	core.m_curr_port_addr++;
	if (core.m_curr_port_addr >= 0xfffff)
		core.m_curr_port_addr = 0x2800 >> 1;
	return ret;
}

void iop_spu_device::port_write(int bank, uint16_t data)
{
	iop_spu_core_t &core = m_core[bank];
	m_ram[core.m_curr_port_addr] = data;
	core.m_curr_port_addr++;
	if (core.m_curr_port_addr >= 0xfffff)
		core.m_curr_port_addr = 0x2800 >> 1;
}

uint16_t iop_spu_device::read(offs_t offset, uint16_t mem_mask)
{
	return reg_read(BIT(offset, 9), (offset << 1) & 0x3ff, mem_mask);
}

uint16_t iop_spu_device::reg_read(int bank, uint32_t offset, uint16_t mem_mask)
{
	iop_spu_core_t &core = m_core[bank];
	uint16_t ret = 0;

	if (offset < 0x180)
	{
		const uint32_t reg = offset & 0xf;
		ret = core.m_voices[offset >> 4].read(reg);
		logerror("%s %d: Voice read: Unknown offset %d (%04x & %04x)\n", machine().describe_context(), bank, reg, ret, mem_mask);
		return ret;
	}

	switch (offset)
	{
		case 0x19a:
			ret = core.m_unknown_0x19a;
			logerror("%s %d: read: Unknown 0x19a (%04x & %04x)\n", machine().describe_context(), bank, ret, mem_mask);
			break;

		case 0x1a8:
			ret = (uint16_t)(core.m_start_port_addr >> 16);
			logerror("%s %d: read: PORT_ADDR_HI: %04x & %04x\n", machine().describe_context(), bank, ret, mem_mask);
			break;

		case 0x1aa:
			ret = (uint16_t)core.m_start_port_addr;
			logerror("%s %d: read: PORT_ADDR_LO: %04x & %04x\n", machine().describe_context(), bank, ret, mem_mask);
			break;

		case 0x1b0:
			ret = core.m_autodma_ctrl;
			logerror("%s %d: read: Auto DMA control: %04x & %04x\n", machine().describe_context(), bank, ret, mem_mask);
			break;

		case 0x344:
			ret = core.m_status;
			logerror("%s %d: read: STATUS: %04x & %04x\n", machine().describe_context(), bank, ret, mem_mask);
			break;

		default:
			logerror("%s %d: read: Unknown %08x (%04x)\n", machine().describe_context(), bank, 0x1f900000 + (offset << 1) + (bank << 10), mem_mask);
			break;
	}
	return ret;
}

void iop_spu_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	reg_write(BIT(offset, 9), (offset << 1) & 0x3ff, data, mem_mask);
}

void iop_spu_device::reg_write(int bank, uint32_t offset, uint16_t data, uint16_t mem_mask)
{
	iop_spu_core_t &core = m_core[bank];

	if (offset < 0x180)
	{
		const uint32_t reg = offset & 0xf;
		logerror("%s %d: Voice write: Unknown offset %d = %04x & %04x\n", machine().describe_context(), bank, reg, data, mem_mask);
		core.m_voices[offset >> 4].write(reg, data);
		return;
	}

	switch (offset)
	{
		case 0x19a:
			logerror("%s %d: write: Unknown 0x19a = %04x & %04x\n", machine().describe_context(), bank, data, mem_mask);
			if (BIT(data, 15)) // Reset?
			{
				data &= 0x7fff;
				core.m_status = 0;
				core.m_autodma_ctrl = 0;
			}
			core.m_unknown_0x19a = data;
			break;

		case 0x1a8:
			logerror("%s %d: write: PORT_ADDR_HI: %04x & %04x\n", machine().describe_context(), bank, data, mem_mask);
			core.m_start_port_addr &= ~0x000f0000;
			core.m_start_port_addr |= ((uint32_t)data << 16) & 0x000f0000;
			core.m_curr_port_addr = core.m_start_port_addr;
			break;

		case 0x1aa:
			logerror("%s %d: write: PORT_ADDR_LO: %04x & %04x\n", machine().describe_context(), bank, data, mem_mask);
			core.m_start_port_addr &= ~0x0000ffff;
			core.m_start_port_addr |= data;
			core.m_curr_port_addr = core.m_start_port_addr;
			break;

		case 0x1b0:
			logerror("%s %d: write: Auto DMA control: %04x & %04x\n", machine().describe_context(), bank, data, mem_mask);
			core.m_autodma_ctrl = data;
			break;

		default:
			logerror("%s %d: write: Unknown %08x = %04x & %04x\n", machine().describe_context(), bank, 0x1f900000 + (offset << 1) + (bank << 10), data, mem_mask);
			break;
	}
}

void iop_spu_device::voice_t::write(uint32_t offset, uint16_t data)
{
	m_unknown[(offset >> 1) & 7] = data;
}

uint16_t iop_spu_device::voice_t::read(uint32_t offset)
{
	return m_unknown[(offset >> 1) & 7];
}
