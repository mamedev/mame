// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 IOP DMAC device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "iopdma.h"

#include "cpu/mips/ps2vu.h"


DEFINE_DEVICE_TYPE(SONYIOP_DMA, iop_dma_device, "iopdma", "PlayStation 2 IOP DMAC")

iop_dma_device::iop_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYIOP_DMA, tag, owner, clock)
	, device_execute_interface(mconfig, *this)
	, m_intc(*this, finder_base::DUMMY_TAG)
	, m_ram(*this, finder_base::DUMMY_TAG)
	, m_sif(*this, finder_base::DUMMY_TAG)
	, m_spu(*this, finder_base::DUMMY_TAG)
	, m_sio2(*this, finder_base::DUMMY_TAG)
	, m_icount(0)
{
}

iop_dma_device::~iop_dma_device()
{
}

void iop_dma_device::device_start()
{
	set_icountptr(m_icount);

	for (uint32_t channel = 0; channel < 16; channel++)
	{
		save_item(NAME(m_channels[channel].m_priority), channel);
		save_item(NAME(m_channels[channel].m_enabled), channel);
		save_item(NAME(m_channels[channel].m_busy), channel);
		save_item(NAME(m_channels[channel].m_end), channel);

		save_item(NAME(m_channels[channel].m_addr), channel);
		save_item(NAME(m_channels[channel].m_ctrl), channel);
		save_item(NAME(m_channels[channel].m_tag_addr), channel);

		save_item(NAME(m_channels[channel].m_block), channel);
		save_item(NAME(m_channels[channel].m_block_count), channel);
		save_item(NAME(m_channels[channel].m_word_count), channel);
		save_item(NAME(m_channels[channel].m_count), channel);
	}

	save_item(NAME(m_running_mask));
	save_item(NAME(m_icount));
	save_item(NAME(m_dpcr[0]));
	save_item(NAME(m_dpcr[1]));
	save_item(NAME(m_dicr[0]));
	save_item(NAME(m_dicr[1]));
	save_item(NAME(m_int_ctrl[0].m_mask));
	save_item(NAME(m_int_ctrl[0].m_status));
	save_item(NAME(m_int_ctrl[0].m_enabled));
	save_item(NAME(m_int_ctrl[1].m_mask));
	save_item(NAME(m_int_ctrl[1].m_status));
	save_item(NAME(m_int_ctrl[1].m_enabled));

	save_item(NAME(m_last_serviced));
}

void iop_dma_device::device_reset()
{
	memset(m_channels, 0, sizeof(channel_t) * 16);
	memset(m_int_ctrl, 0, sizeof(intctrl_t) * 2);
	m_dpcr[0] = 0;
	m_dpcr[1] = 0;
	m_dicr[0] = 0;
	m_dicr[1] = 0;
	m_last_serviced = 0;
	m_running_mask = 0;
}

void iop_dma_device::execute_run()
{
	if (!m_running_mask)
	{
		m_icount = 0;
		return;
	}

	while (m_icount > 0)
	{
		// TODO: Is this right? This doesn't seem right.
		for (; m_last_serviced < 16; m_last_serviced++)
		{
			const uint32_t channel = m_last_serviced;
			if (m_channels[channel].enabled() && m_channels[channel].busy())
			{
				switch (channel)
				{
					case SPU_BANK1:
					case SPU_BANK2:
						transfer_spu(channel);
						break;
					case SIF0:
						transfer_sif0(channel);
						break;
					case SIF1:
						transfer_sif1(channel);
						break;
					case SIO2_IN:
						transfer_to_sio2(channel);
						break;
					case SIO2_OUT:
						transfer_from_sio2(channel);
						break;
					default:
						logerror("%s: Attempting to transfer an unimplemented DMA channel (%d)\n", machine().describe_context(), channel);
						break;
				}
			}
		}
		if (m_last_serviced == 16)
			m_last_serviced = 0;
		m_icount--;
	}
}

void iop_dma_device::transfer_sif0(uint32_t chan)
{
	channel_t &channel = m_channels[chan];
	const uint32_t count = channel.count();
	if (count)
	{
		if (m_sif->fifo_depth(0) < ps2_sif_device::MAX_FIFO_DEPTH)
		{
			const uint32_t addr = channel.addr();
			m_sif->fifo_push(0, m_ram[addr >> 2]);
			channel.set_addr(addr + 4);
			channel.set_count(count - 1);
		}
	}
	else
	{
		if (channel.end())
		{
			//logerror("sif0.end\n");
			transfer_finish(SIF0);
		}
		else if (m_sif->fifo_depth(0) < ps2_sif_device::MAX_FIFO_DEPTH - 2)
		{
			const uint32_t tag_addr_bytes = channel.tag_addr();
			const uint32_t tag_addr = tag_addr_bytes >> 2;
			const uint32_t iop_hi = m_ram[tag_addr];
			const uint32_t iop_lo = m_ram[tag_addr + 1];
			const uint32_t ee_hi = m_ram[tag_addr + 2];
			const uint32_t ee_lo = m_ram[tag_addr + 3];

			//logerror("%s: following sif0 iop tag, full tag is %08x %08x %08x %08x\n", machine().describe_context(), iop_hi, iop_lo, ee_hi, ee_lo);
			channel.set_addr(iop_hi & 0x00ffffff);
			channel.set_count((iop_lo + 3) & ~3);
			channel.set_tag_addr(tag_addr_bytes + 0x10);

			m_sif->fifo_push(0, ee_hi);
			m_sif->fifo_push(0, ee_lo);

			if (iop_hi & 0xc0000000)
			{
				//logerror("%s: sif0 iop tag end\n", machine().describe_context());
				channel.m_end = true;
			}
		}
	}
}

void iop_dma_device::transfer_sif1(uint32_t chan)
{
	channel_t &channel = m_channels[chan];
	const uint32_t count = channel.count();
	if (count)
	{
		if (m_sif->fifo_depth(1))
		{
			const uint32_t data = m_sif->fifo_pop(1);
			const uint32_t addr = channel.addr();
			//logerror("%s: sif1 pop value: %08x\n", machine().describe_context(), (uint32_t)data);
			m_ram[addr >> 2] = data;

			channel.set_addr(addr + 4);
			channel.set_count(count - 1);
		}
	}
	else
	{
		if (channel.end())
		{
			//logerror("sif1.end\n");
			transfer_finish(SIF1);
		}
		else if (m_sif->fifo_depth(1) >= 4)
		{
			const uint32_t iop_hi = m_sif->fifo_pop(1);
			const uint32_t iop_lo = m_sif->fifo_pop(1);
			m_sif->fifo_pop(1); // ee_hi - ignored
			m_sif->fifo_pop(1); // ee_lo - ignored
			//logerror("%s: following sif1 iop tag, tag is %08x %08x\n", machine().describe_context(), iop_hi, iop_lo);
			channel.set_addr(iop_hi & 0x00ffffff);
			channel.set_count(iop_lo);

			if (iop_hi & 0xc0000000)
			{
				//logerror("%s: sif1 iop tag end\n", machine().describe_context());
				channel.m_end = true;
			}
		}
	}
}

void iop_dma_device::transfer_spu(uint32_t chan)
{
	channel_t &channel = m_channels[chan];
	const uint32_t count = channel.count();
	const bool first_bank = chan == SPU_BANK1;
	const int bank = first_bank ? 0 : 1;
	if (count)
	{
		const uint32_t addr = channel.addr();

		m_spu->dma_write(bank, m_ram[addr >> 2]);

		channel.set_count(count - 1);
		channel.set_addr(addr + 4);
	}
	else if (channel.busy())
	{
		channel.set_word_count(0);
		channel.set_block_count(0);

		m_spu->dma_done(bank);

		//if (first_bank)
			//m_intc->raise_interrupt(iop_intc_device::INT_SPU);

		transfer_finish(chan);
	}
}

void iop_dma_device::transfer_to_sio2(uint32_t chan)
{
	channel_t &channel = m_channels[chan];
	const uint32_t count = channel.count();
	// TODO: Clock out at correct serial rate
	if (count)
	{
		const uint32_t addr = channel.addr();
		const uint32_t data = m_ram[addr >> 2];
		m_sio2->transmit(data & 0xff);
		m_sio2->transmit((data >> 8) & 0xff);
		m_sio2->transmit((data >> 16) & 0xff);
		m_sio2->transmit((data >> 24) & 0xff);
		channel.set_count(count - 1);
		channel.set_addr(addr + 4);
	}
	else
	{
		channel.set_block_count(0);
		channel.set_word_count(0);
		transfer_finish(SIO2_IN);
	}
}

void iop_dma_device::transfer_from_sio2(uint32_t chan)
{
	channel_t &channel = m_channels[chan];
	const uint32_t count = channel.count();
	// TODO: Clock in at correct serial rate
	if (count)
	{
		const uint32_t addr = channel.addr();
		uint32_t data = m_sio2->receive();
		data |= m_sio2->receive() << 8;
		data |= m_sio2->receive() << 16;
		data |= m_sio2->receive() << 24;
		m_ram[addr >> 2] = data;
		channel.set_count(count - 1);
		channel.set_addr(addr + 4);
	}
	else
	{
		channel.set_block_count(0);
		channel.set_word_count(0);
		transfer_finish(SIO2_OUT);
	}
}

void iop_dma_device::transfer_finish(uint32_t chan)
{
	channel_t &channel = m_channels[chan];
	channel.m_ctrl &= ~0x1000000;
	channel.m_busy = false;
	channel.m_end = false;
	const uint32_t index = BIT(chan, 3);
	const uint8_t subchan = chan & 7;
	m_int_ctrl[index].m_status |= (1 << subchan);
	m_dicr[index] |= 1 << (subchan + 24);

	if (m_int_ctrl[index].m_status & m_int_ctrl[index].m_mask)
	{
		m_intc->raise_interrupt(iop_intc_device::INT_DMA);
	}
}

READ32_MEMBER(iop_dma_device::bank0_r)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0x00/4: case 0x10/4: case 0x20/4: case 0x30/4: case 0x40/4: case 0x50/4: case 0x60/4:
			ret = m_channels[offset >> 2].addr();
			//logerror("%s: bank0_r: channel[%d].addr (%08x & %08x)\n", machine().describe_context(), offset >> 2, ret, mem_mask);
			break;
		case 0x04/4: case 0x14/4: case 0x24/4: case 0x34/4: case 0x44/4: case 0x54/4: case 0x64/4:
			ret = m_channels[offset >> 2].block();
			//logerror("%s: bank0_r: channel[%d].block  (%08x & %08x)\n", machine().describe_context(), offset >> 2, ret, mem_mask);
			break;
		case 0x08/4: case 0x18/4: case 0x28/4: case 0x38/4: case 0x48/4: case 0x58/4: case 0x68/4:
			ret = m_channels[offset >> 2].ctrl();
			//logerror("%s: bank0_r: channel[%d].ctrl (%08x & %08x)\n", machine().describe_context(), offset >> 2, ret, mem_mask);
			break;
		case 0x0c/4: case 0x1c/4: case 0x2c/4: case 0x3c/4: case 0x4c/4: case 0x5c/4: case 0x6c/4:
			ret = m_channels[offset >> 2].tag_addr();
			//logerror("%s: bank0_r: channel[%d].tag_addr (%08x & %08x)\n", machine().describe_context(), offset >> 2, ret, mem_mask);
			break;
		case 0x70/4: // 0x1f8010f0, DPCR
			ret = m_dpcr[0];
			//logerror("%s: bank0_r: DPCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x74/4: // 0x1f8010f4, DICR
			ret = m_dicr[0];
			if ((m_int_ctrl[0].m_status & m_int_ctrl[0].m_mask) && m_int_ctrl[0].m_enabled)
				ret |= 0x80000000;
			//logerror("%s: bank0_r: DICR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		default:
			logerror("%s: bank0_r: Unknown %08x & %08x\n", machine().describe_context(), 0x1f801080 + (offset << 2), mem_mask);
			break;
	}
	return ret;
}

WRITE32_MEMBER(iop_dma_device::bank0_w)
{
	switch (offset)
	{
		case 0x00/4: case 0x10/4: case 0x20/4: case 0x30/4: case 0x40/4: case 0x50/4: case 0x60/4:
			//logerror("%s: bank0_w: channel[%d].addr = %08x & %08x\n", machine().describe_context(), offset >> 2, data, mem_mask);
			m_channels[offset >> 2].set_addr(data);
			break;
		case 0x04/4: case 0x14/4: case 0x24/4: case 0x34/4: case 0x44/4: case 0x54/4: case 0x64/4:
			//logerror("%s: bank0_w: channel[%d].block = %08x & %08x\n", machine().describe_context(), offset >> 2, data, mem_mask);
			m_channels[offset >> 2].set_block(data, mem_mask);
			break;
		case 0x08/4: case 0x18/4: case 0x28/4: case 0x38/4: case 0x48/4: case 0x58/4: case 0x68/4:
			//logerror("%s: bank0_w: channel[%d].ctrl = %08x & %08x\n", machine().describe_context(), offset >> 2, data, mem_mask);
			m_channels[offset >> 2].set_ctrl(data);
			m_running_mask |= m_channels[offset >> 2].busy() ? (1 << (offset >> 2)) : 0;
			break;
		case 0x0c/4: case 0x1c/4: case 0x2c/4: case 0x3c/4: case 0x4c/4: case 0x5c/4: case 0x6c/4:
			//logerror("%s: bank0_w: channel[%d].tag_addr = %08x & %08x\n", machine().describe_context(), offset >> 2, data, mem_mask);
			m_channels[offset >> 2].set_tag_addr(data);
			break;
		case 0x70/4: // 0x1f8010f0, DPCR
			//logerror("%s: bank0_w: DPCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			set_dpcr(data, 0);
			break;
		case 0x74/4: // 0x1f8010f4, DICR
			//logerror("%s: bank0_w: DICR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			set_dicr(data, 0);
			break;
		default:
			logerror("%s: bank0_w: Unknown %08x = %08x & %08x\n", machine().describe_context(), 0x1f801080 + (offset << 2), data, mem_mask);
			break;
	}
}

READ32_MEMBER(iop_dma_device::bank1_r)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0x00/4: case 0x10/4: case 0x20/4: case 0x30/4: case 0x40/4: case 0x50/4: case 0x60/4:
			ret = m_channels[(offset >> 2) + 8].addr();
			//logerror("%s: bank1_r: channel[%d].addr (%08x & %08x)\n", machine().describe_context(), (offset >> 2) + 8, ret, mem_mask);
			break;
		case 0x04/4: case 0x14/4: case 0x24/4: case 0x34/4: case 0x44/4: case 0x54/4: case 0x64/4:
			ret = m_channels[(offset >> 2) + 8].block();
			//logerror("%s: bank1_r: channel[%d].block  (%08x & %08x)\n", machine().describe_context(), (offset >> 2) + 8, ret, mem_mask);
			break;
		case 0x08/4: case 0x18/4: case 0x28/4: case 0x38/4: case 0x48/4: case 0x58/4: case 0x68/4:
			ret = m_channels[(offset >> 2) + 8].ctrl();
			//if ((offset >> 2) != 2)
				//logerror("%s: bank1_r: channel[%d].ctrl (%08x & %08x)\n", machine().describe_context(), (offset >> 2) + 8, ret, mem_mask);
			break;
		case 0x0c/4: case 0x1c/4: case 0x2c/4: case 0x3c/4: case 0x4c/4: case 0x5c/4: case 0x6c/4:
			ret = m_channels[(offset >> 2) + 8].tag_addr();
			//logerror("%s: bank1_r: channel[%d].tag_addr (%08x & %08x)\n", machine().describe_context(), (offset >> 2) + 8, ret, mem_mask);
			break;
		case 0x70/4: // 0x1f801570, DPCR2
			ret = m_dpcr[1];
			//logerror("%s: bank1_r: DPCR2 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x74/4: // 0x1f801574, DICR2
			ret = m_dicr[1];
			if ((m_int_ctrl[1].m_status & m_int_ctrl[1].m_mask) && m_int_ctrl[1].m_enabled)
				ret |= 0x80000000;
			//logerror("%s: bank1_r: DICR2 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		default:
			logerror("%s: bank1_r: Unknown %08x & %08x\n", machine().describe_context(), 0x1f801500 + (offset << 2), mem_mask);
			break;
	}
	return ret;
}

WRITE32_MEMBER(iop_dma_device::bank1_w)
{
	switch (offset & 0x1f)
	{
		case 0x00/4: case 0x10/4: case 0x20/4: case 0x30/4: case 0x40/4: case 0x50/4: case 0x60/4:
			//logerror("%s: bank1_w: channel[%d].addr = %08x & %08x\n", machine().describe_context(), (offset >> 2) + 8, data, mem_mask);
			m_channels[(offset >> 2) + 8].set_addr(data);
			break;
		case 0x04/4: case 0x14/4: case 0x24/4: case 0x34/4: case 0x44/4: case 0x54/4: case 0x64/4:
			//logerror("%s: bank1_w: channel[%d].block = %08x & %08x\n", machine().describe_context(), (offset >> 2) + 8, data, mem_mask);
			m_channels[(offset >> 2) + 8].set_block(data, mem_mask);
			break;
		case 0x08/4: case 0x18/4: case 0x28/4: case 0x38/4: case 0x48/4: case 0x58/4: case 0x68/4:
			//logerror("%s: bank1_w: channel[%d].ctrl = %08x & %08x\n", machine().describe_context(), (offset >> 2) + 8, data, mem_mask);
			m_channels[(offset >> 2) + 8].set_ctrl(data);
			m_running_mask |= m_channels[(offset >> 2) + 8].busy() ? (1 << ((offset >> 2) + 8)) : 0;
			break;
		case 0x0c/4: case 0x1c/4: case 0x2c/4: case 0x3c/4: case 0x4c/4: case 0x5c/4: case 0x6c/4:
			//logerror("%s: bank1_w: channel[%d].tag_addr = %08x & %08x\n", machine().describe_context(), (offset >> 2) + 8, data, mem_mask);
			m_channels[(offset >> 2) + 8].set_tag_addr(data);
			break;
		case 0x70/4: // 0x1f801570, DPCR2
			//logerror("%s: bank1_w: DPCR2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			set_dpcr(data, 1);
			break;
		case 0x74/4: // 0x1f801574, DICR2
			//logerror("%s: bank1_w: DICR2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			set_dicr(data, 1);
			break;
		default:
			logerror("%s: bank1_w: Unknown %08x = %08x & %08x\n", machine().describe_context(), 0x1f801500 + (offset << 2), data, mem_mask);
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
		m_channels[channel].set_pri_ctrl(field);
		if (BIT(field, 3))
			m_running_mask |= 0x10000 << channel;
		else
			m_running_mask &= ~(0x10000 << channel);
	}
}

void iop_dma_device::set_dicr(uint32_t data, uint32_t index)
{
	m_dicr[index] = (m_dicr[index] & 0x7f000000) | (data & ~0x7f007fff);
	m_dicr[index] &= ~(data & 0x7f000000);
	m_int_ctrl[index].m_mask = (data >> 16) & 0x7f;
	m_int_ctrl[index].m_status &= ~((data >> 24) & 0x7f);
	m_int_ctrl[index].m_enabled = BIT(data, 23);
	update_interrupts();
}

void iop_dma_device::channel_t::set_pri_ctrl(uint32_t pri_ctrl)
{
	bool was_enabled = m_enabled;
	m_enabled = BIT(pri_ctrl, 3);
	m_priority = pri_ctrl & 7;
	if (!was_enabled && m_enabled)
		m_end = false;
}

void iop_dma_device::channel_t::set_block(uint32_t block, uint32_t mem_mask)
{
	m_block = block;
	if (mem_mask & 0xffff)
		set_block_count(block);
	if (mem_mask & 0xffff0000)
		set_word_count(block >> 16);
}

void iop_dma_device::channel_t::set_block_count(uint32_t block_count)
{
	m_block_count = (uint16_t)block_count;
	m_count = m_block_count * m_word_count;
}

void iop_dma_device::channel_t::set_word_count(uint32_t word_count)
{
	m_word_count = (uint16_t)word_count;
	m_count = m_block_count * m_word_count;
}

void iop_dma_device::channel_t::set_ctrl(uint32_t ctrl)
{
	m_ctrl = ctrl;
	m_busy = BIT(ctrl, 24);
}
