// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 EE DMA controller device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "ps2dma.h"

#include "cpu/mips/mips3.h"
#include "cpu/mips/ps2vif1.h"
#include "video/ps2gif.h"

DEFINE_DEVICE_TYPE(SONYPS2_DMAC, ps2_dmac_device, "ps2dmac", "PlayStation 2 EE DMAC")

ps2_dmac_device::ps2_dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYPS2_DMAC, tag, owner, clock)
	, device_execute_interface(mconfig, *this)
	, m_ee(*this, finder_base::DUMMY_TAG)
	, m_ram(*this, finder_base::DUMMY_TAG)
	, m_sif(*this, finder_base::DUMMY_TAG)
	, m_gs(*this, finder_base::DUMMY_TAG)
	, m_vu1(*this, finder_base::DUMMY_TAG)
	, m_icount(0)
{
}

ps2_dmac_device::~ps2_dmac_device()
{
}

void ps2_dmac_device::device_start()
{
	set_icountptr(m_icount);

	for (uint32_t channel = 0; channel < 10; channel++)
	{
		//save_item(NAME(m_channels[channel].m_priority), channel);
	}

	save_item(NAME(m_icount));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_mem_drain));
	save_item(NAME(m_enabled));

	save_item(NAME(m_disable_mask));

	save_item(NAME(m_istat));
	save_item(NAME(m_imask));

	save_item(NAME(m_last_serviced));

	for (uint32_t channel = 0; channel < 10; channel++)
	{
		save_item(NAME(m_channels[channel].m_chcr), channel);
		save_item(NAME(m_channels[channel].m_mode), channel);
		save_item(NAME(m_channels[channel].m_enabled), channel);
		save_item(NAME(m_channels[channel].m_end_tag), channel);
		save_item(NAME(m_channels[channel].m_end_irq), channel);
		save_item(NAME(m_channels[channel].m_ienable), channel);
		save_item(NAME(m_channels[channel].m_qwc), channel);
		save_item(NAME(m_channels[channel].m_addr), channel);
		save_item(NAME(m_channels[channel].m_tag_addr), channel);
	}
}

void ps2_dmac_device::device_reset()
{
	m_ctrl = 0;
	m_mem_drain = 0;
	m_enabled = false;

	m_disable_mask = 0x1201;

	m_istat = 0;
	m_imask = 0;

	m_last_serviced = 0;
}

void ps2_dmac_device::execute_run()
{
	if (!m_enabled || (m_disable_mask & 0x10000))
	{
		m_icount = 0;
		return;
	}

	while (m_icount > 0)
	{
		for (; m_last_serviced < 10 && m_icount > 0; m_last_serviced++, m_icount--)
		{
			if (!m_channels[m_last_serviced].enabled())
				continue;

			switch (m_last_serviced)
			{
				case VIF1:
					transfer_vif1();
					break;
				case GIF:
					transfer_gif();
					break;
				case SIF0:
					transfer_sif0();
					break;
				case SIF1:
					transfer_sif1();
					break;
				default:
					logerror("%s: Attempting to service unimplemented DMA channel %d\n", machine().describe_context(), m_last_serviced);
					break;
			}
		}
		if (m_last_serviced == 10)
			m_last_serviced = 0;
	}
}

void ps2_dmac_device::transfer_vif1()
{
	if (m_mem_drain)
	{
		logerror("%s: transfer_vif1: Not yet implemented: memory drain bit. HACK: Disabling channel.\n", machine().describe_context());
		transfer_finish(VIF1);
		return;
	}

	channel_t &channel = m_channels[VIF1];
	const uint32_t count = channel.quadword_count();
	if (count)
	{
		//logerror("%s: DMAC VIF1 quadword count: %08x\n", machine().describe_context(), count);
		if (m_vu1->interface()->fifo_available(4))
		{
			uint32_t addr = channel.addr();
			if (BIT(addr, 31))
				addr -= 0x10000000;
			address_space &space = m_ee->space(AS_PROGRAM);
			uint32_t words[4] = { 0, 0, 0, 0 };
			for (int word = 0; word < 4; word++)
			{
				words[word] = space.read_dword(addr);
				addr += 4;
			}
			m_vu1->interface()->dma_write(((uint64_t)words[3] << 32) | words[2], ((uint64_t)words[1] << 32) | words[0]);
			channel.set_addr(channel.addr() + 0x10);
			channel.set_quadword_count(count - 1);
		}
	}
	else if (channel.end_tag())
	{
		logerror("%s: DMAC VIF1 end tag\n", machine().describe_context());
		transfer_finish(VIF1);
	}
	else if (m_vu1->interface()->fifo_available(2))
	{
		logerror("%s: DMAC VIF1 following source tag\n", machine().describe_context());
		uint32_t tag_addr = channel.tag_addr();
		if (BIT(tag_addr, 31))
			tag_addr -= 0x10000000;
		address_space &space = m_ee->space(AS_PROGRAM);
		uint32_t words[4] = { 0, 0, 0, 0 };
		for (int word = 0; word < 4; word++)
		{
			words[word] = space.read_dword(tag_addr);
			tag_addr += 4;
		}
		m_vu1->interface()->tag_write(words);
		follow_source_tag(VIF1);
	}
}

void ps2_dmac_device::transfer_gif()
{
	if (m_mem_drain)
	{
		logerror("%s: Not yet implemented: memory drain bit. HACK: Disabling channel.\n", machine().describe_context());
		transfer_finish(GIF);
		return;
	}

	channel_t &channel = m_channels[GIF];
	const uint32_t count = channel.quadword_count();
	if (count)
	{
		//logerror("%s: DMAC GIF quadword count: %08x\n", machine().describe_context(), count);
		uint32_t addr = channel.addr();
		if (BIT(addr, 31))
			addr -= 0x10000000;
		address_space &space = m_ee->space(AS_PROGRAM);
		uint32_t words[4] = { 0, 0, 0, 0 };
		for (int word = 0; word < 4; word++)
		{
			words[word] = space.read_dword(addr);
			addr += 4;
		}
		m_gs->interface()->write_path3(((uint64_t)words[3] << 32) | words[2], ((uint64_t)words[1] << 32) | words[0]);
		channel.set_addr(channel.addr() + 0x10);
		channel.set_quadword_count(count - 1);
	}
	else if (channel.end_tag())
	{
		logerror("%s: DMAC GIF end tag\n", machine().describe_context());
		transfer_finish(GIF);
	}
	else
	{
		logerror("%s: DMAC GIF following source tag\n", machine().describe_context());
		follow_source_tag(GIF);
	}
}

void ps2_dmac_device::transfer_sif0()
{
	channel_t &channel = m_channels[SIF0];
	const uint32_t count = channel.quadword_count();
	if (count)
	{
		if (m_sif->fifo_depth(0) >= 4)
		{
			//logerror("%s: SIF0 depth is %d\n", machine().describe_context(), m_sif->fifo_depth(0));
			uint32_t addr = channel.addr();
			if (BIT(addr, 31))
				addr -= 0x10000000;
			for (int word = 0; word < 4; word++)
			{
				const uint32_t data = m_sif->fifo_pop(0);
				//logerror("%s: SIF0 popping %08x -> %08x\n", machine().describe_context(), data, addr);
				m_ee->space(AS_PROGRAM).write_dword(addr, data);
				addr += 4;
			}
			channel.set_addr(channel.addr() + 0x10);
			channel.set_quadword_count(count - 1);
			//logerror("%s: SIF0 remaining count: %08x\n", machine().describe_context(), channel.quadword_count());
		}
	}
	else if (channel.end_tag())
	{
		//logerror("%s: SIF0 end tag, finishing transfer\n", machine().describe_context());
		transfer_finish(SIF0);
	}
	else if (m_sif->fifo_depth(0) >= 2)
	{
		const uint32_t hi = m_sif->fifo_pop(0);
		const uint32_t lo = m_sif->fifo_pop(0);
		const uint32_t tag = hi;
		//logerror("%s: SIF0 chaining tag, tag %08x %08x\n", machine().describe_context(), hi, lo);
		channel.set_addr(lo);
		channel.set_tag_addr(channel.tag_addr() + 0x10);
		channel.set_quadword_count(tag & 0x0000ffff);
		channel.set_chcr((channel.chcr() & 0x0000ffff) | (tag & 0xffff0000));

		const uint8_t mode = (tag >> 28) & 7;
		if ((channel.irq_enable() && BIT(tag, 31)) || mode == ID_END)
		{
			channel.set_end_tag(true);
		}
	}
}

void ps2_dmac_device::transfer_sif1()
{
	channel_t &channel = m_channels[SIF1];
	const uint32_t count = channel.quadword_count();
	if (count)
	{
		//logerror("%s: DMAC SIF1 quadword count: %08x\n", machine().describe_context(), count);
		if (m_sif->fifo_depth(1) < (ps2_sif_device::MAX_FIFO_DEPTH - 4))
		{
			uint32_t addr = channel.addr();
			if (BIT(addr, 31))
				addr -= 0x10000000;
			address_space &space = m_ee->space(AS_PROGRAM);
			for (int word = 0; word < 4; word++)
			{
				const uint32_t data = space.read_dword(addr);
				//logerror("%s: DMAC SIF1 Pushing %08x\n", machine().describe_context(), data);
				addr += 4;
				m_sif->fifo_push(1, data);
			}
			channel.set_addr(channel.addr() + 0x10);
			channel.set_quadword_count(count - 1);
		}
	}
	else if (channel.end_tag())
	{
		//logerror("%s: DMAC SIF1 end tag\n", machine().describe_context());
		transfer_finish(SIF1);
	}
	else
	{
		//logerror("%s: DMAC SIF1 following source tag\n", machine().describe_context());
		follow_source_tag(SIF1);
	}
}

void ps2_dmac_device::follow_source_tag(uint32_t chan)
{
	channel_t &channel = m_channels[chan];
	uint32_t tag_addr = channel.tag_addr();
	if (BIT(tag_addr, 31))
		tag_addr -= 0x10000000;
	address_space &space = m_ee->space(AS_PROGRAM);
	const uint64_t hi = space.read_qword(tag_addr);
	const uint64_t lo = space.read_dword(tag_addr + 8);
	const uint32_t tag = (uint32_t)hi;
	const uint32_t addr = (uint32_t)(hi >> 32) & ~0xf;
	if (chan != GIF && chan != VIF1)
		logerror("Trying to follow tag: %08x|%08x %08x|%08x\n", (uint32_t)(hi >> 32), (uint32_t)hi, (uint32_t)(lo >> 32), (uint32_t)lo);

	channel.set_chcr((channel.chcr() & 0x0000ffff) | (tag & 0xffff0000));
	channel.set_quadword_count(tag & 0x0000ffff);
	const uint32_t id = (tag >> 28) & 7;
	const bool irq = BIT(tag, 31);

	switch (id)
	{
		case ID_CNT:
			channel.set_addr(channel.tag_addr() + 0x10);
			channel.set_tag_addr(channel.addr() + (channel.quadword_count() << 4));
			break;
		case ID_REFE:
			channel.set_end_tag(true);
			// Intentional fall-through
		case ID_REF:
			channel.set_addr(addr);
			channel.set_tag_addr(channel.tag_addr() + 0x10);
			break;
		case ID_REFS:
			// We don't handle stalls yet, just act the same as REF for now
			channel.set_addr(addr);
			channel.set_tag_addr(channel.tag_addr() + 0x10);
			break;
		case ID_NEXT:
			channel.set_addr(channel.tag_addr() + 0x10);
			channel.set_tag_addr(addr);
			break;
		case ID_END:
			channel.set_addr(channel.tag_addr() + 0x10);
			channel.set_end_tag(true);
			break;
		default:
			logerror("%s: Unknown DMAtag ID: %d\n", machine().describe_context(), id);
			break;
	}

	if (irq && channel.irq_enable())
	{
		channel.set_end_tag(true);
	}
}

void ps2_dmac_device::transfer_finish(uint32_t chan)
{
	channel_t &channel = m_channels[chan];
	channel.set_chcr(channel.chcr() & ~0x100);
	m_istat |= (1 << chan);
	update_interrupts();
}

void ps2_dmac_device::update_interrupts()
{
	logerror("%s: update_interrupts: %d\n", machine().describe_context(), (m_istat & m_imask) ? 1 : 0);
	m_ee->set_input_line(MIPS3_IRQ1, (m_istat & m_imask) ? ASSERT_LINE : CLEAR_LINE);
}

READ32_MEMBER(ps2_dmac_device::read)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0: /* D_CTRL */
			ret = m_ctrl;
			logerror("%s: dmac_r: D_CTRL (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 2: /* D_STAT */
			ret = m_istat | (m_imask << 16);
			logerror("%s: dmac_r: D_STAT (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 4: /* D_PCR */
			logerror("%s: dmac_r: D_PCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 6: /* D_SQWC */
			logerror("%s: dmac_r: D_SQWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 8: /* D_RBSR */
			logerror("%s: dmac_r: D_RBSR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 10: /* D_RBOR */
			logerror("%s: dmac_r: D_RBOR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 12: /* D_STADR */
			logerror("%s: dmac_r: D_STADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		default:
			logerror("%s: dmac_r: Unknown offset %08x & %08x\n", machine().describe_context(), 0x1000e000 + (offset << 3), mem_mask);
			break;
	}
	return ret;
}

WRITE32_MEMBER(ps2_dmac_device::write)
{
	switch (offset)
	{
		case 0: /* D_CTRL */
			logerror("%s: dmac_w: D_CTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			logerror("%s:         ENABLE=%d MEM_DRAIN=%d\n", machine().describe_context(), BIT(data, 0), (data >> 2) & 3);
			m_ctrl = data;
			m_enabled = BIT(data, 0);
			m_mem_drain = (data >> 2) & 3;
			break;
		case 2: /* D_STAT */
			logerror("%s: dmac_w: D_STAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_istat &= ~(data & 0x7fff);
			m_imask ^= (data >> 16) & 0x7fff;
			update_interrupts();
			break;
		case 4: /* D_PCR */
			logerror("%s: dmac_w: D_PCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 6: /* D_SQWC */
			logerror("%s: dmac_w: D_SQWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 8: /* D_RBSR */
			logerror("%s: dmac_w: D_RBSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 10: /* D_RBOR */
			logerror("%s: dmac_w: D_RBOR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 12: /* D_STADR */
			logerror("%s: dmac_w: D_STADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		default:
			logerror("%s: dmac_w: Unknown offset %08x = %08X & %08x\n", machine().describe_context(), 0x1000e000 + (offset << 3), data, mem_mask);
			break;
	}
}

READ32_MEMBER(ps2_dmac_device::channel_r)
{
	uint32_t ret = 0;
	switch (offset + 0x8000/8)
	{
		case 0x8000/8: /* D0_CHCR */
			logerror("%s: dmac_channel_r: D0_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x8010/8: /* D0_MADR */
			logerror("%s: dmac_channel_r: D0_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x8020/8: /* D0_QWC */
			logerror("%s: dmac_channel_r: D0_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x8030/8: /* D0_TADR */
			logerror("%s: dmac_channel_r: D0_TADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x8040/8: /* D0_ASR0 */
			logerror("%s: dmac_channel_r: D0_ASR0 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x8050/8: /* D0_ASR1 */
			logerror("%s: dmac_channel_r: D0_ASR1 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x9000/8: /* VIF1_CHCR */
			ret = m_channels[VIF1].chcr();
			//logerror("%s: dmac_channel_r: VIF1_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x9010/8: /* VIF1_MADR */
			ret = m_channels[VIF1].addr();
			logerror("%s: dmac_channel_r: VIF1_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x9020/8: /* VIF1_QWC */
			ret = m_channels[VIF1].quadword_count();
			logerror("%s: dmac_channel_r: VIF1_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x9030/8: /* VIF1_TADR */
			ret = m_channels[VIF1].tag_addr();
			logerror("%s: dmac_channel_r: VIF1_TADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x9040/8: /* VIF1_ASR0 */
			logerror("%s: dmac_channel_r: VIF1_ASR0 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x9050/8: /* VIF1_ASR1 */
			logerror("%s: dmac_channel_r: VIF1_ASR1 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xa000/8: /* GIF_CHCR */
			ret = m_channels[GIF].chcr();
			//logerror("%s: dmac_channel_r: GIF_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xa010/8: /* GIF_MADR */
			ret = m_channels[GIF].addr();
			logerror("%s: dmac_channel_r: GIF_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xa020/8: /* GIF_QWC */
			ret = m_channels[GIF].quadword_count();
			logerror("%s: dmac_channel_r: GIF_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xa030/8: /* GIF_TADR */
			ret = m_channels[GIF].tag_addr();
			logerror("%s: dmac_channel_r: GIF_TADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xa040/8: /* GIF_ASR0 */
			logerror("%s: dmac_channel_r: GIF_ASR0 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xa050/8: /* GIF_ASR1 */
			logerror("%s: dmac_channel_r: GIF_ASR1 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xb000/8: /* D3_CHCR */
			logerror("%s: dmac_channel_r: D3_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xb010/8: /* D3_MADR */
			logerror("%s: dmac_channel_r: D3_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xb020/8: /* D3_QWC */
			logerror("%s: dmac_channel_r: D3_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xb400/8: /* D4_CHCR */
			logerror("%s: dmac_channel_r: D4_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xb410/8: /* D4_MADR */
			logerror("%s: dmac_channel_r: D4_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xb420/8: /* D4_QWC */
			logerror("%s: dmac_channel_r: D4_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xb430/8: /* D4_TADR */
			logerror("%s: dmac_channel_r: D4_TADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc000/8: /* SIF0_CHCR */
			ret = m_channels[SIF0].chcr();
			//logerror("%s: dmac_channel_r: SIF0_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc010/8: /* SIF0_MADR */
			ret = m_channels[SIF0].addr();
			//logerror("%s: dmac_channel_r: SIF0_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc020/8: /* SIF0_QWC */
			ret = m_channels[SIF0].quadword_count();
			//logerror("%s: dmac_channel_r: SIF0_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc400/8: /* SIF1_CHCR */
			ret = m_channels[SIF1].chcr();
			//logerror("%s: dmac_channel_r: SIF1_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc410/8: /* SIF1_MADR */
			ret = m_channels[SIF1].addr();
			//logerror("%s: dmac_channel_r: SIF1_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc420/8: /* SIF1_QWC */
			ret = m_channels[SIF1].quadword_count();
			//logerror("%s: dmac_channel_r: SIF1_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc430/8: /* SIF1_TADR */
			ret = m_channels[SIF1].tag_addr();
			//logerror("%s: dmac_channel_r: SIF1_TADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc800/8: /* D7_CHCR */
			logerror("%s: dmac_channel_r: D7_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc810/8: /* D7_MADR */
			logerror("%s: dmac_channel_r: D7_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc820/8: /* D7_QWC */
			logerror("%s: dmac_channel_r: D7_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xd000/8: /* D8_CHCR */
			logerror("%s: dmac_channel_r: D8_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xd010/8: /* D8_MADR */
			logerror("%s: dmac_channel_r: D8_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xd020/8: /* D8_QWC */
			logerror("%s: dmac_channel_r: D8_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xd080/8: /* D8_SADR */
			logerror("%s: dmac_channel_r: D8_SADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xd400/8: /* D9_CHCR */
			logerror("%s: dmac_channel_r: D9_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xd410/8: /* D9_MADR */
			logerror("%s: dmac_channel_r: D9_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xd420/8: /* D9_QWC */
			logerror("%s: dmac_channel_r: D9_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xd430/8: /* D9_TADR */
			logerror("%s: dmac_channel_r: D9_TADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xd480/8: /* D9_SADR */
			logerror("%s: dmac_channel_r: D9_SADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		default:
			logerror("%s: dmac_channel_r: Unknown offset %08x & %08x\n", machine().describe_context(), 0x10008000 + (offset << 3), mem_mask);
			break;
	}
	return ret;
}

WRITE32_MEMBER(ps2_dmac_device::channel_w)
{
	static char const *const mode_strings[4] = { "Normal", "Chain", "Interleave", "Undefined" };

	switch (offset + 0x8000/8)
	{
		case 0x8000/8: /* D0_CHCR */
			logerror("%s: dmac_channel_w: D0_CHCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x8010/8: /* D0_MADR */
			logerror("%s: dmac_channel_w: D0_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x8020/8: /* D0_QWC */
			logerror("%s: dmac_channel_w: D0_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x8030/8: /* D0_TADR */
			logerror("%s: dmac_channel_w: D0_TADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x8040/8: /* D0_ASR0 */
			logerror("%s: dmac_channel_w: D0_ASR0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x8050/8: /* D0_ASR1 */
			logerror("%s: dmac_channel_w: D0_ASR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x9000/8: /* VIF1_CHCR */
			logerror("%s: dmac_channel_w: VIF1_CHCR = %08x & %08x (DIR=%s Memory, MOD=%s, ASP=%d, TTE=%s DMAtag, \n", machine().describe_context(), data, mem_mask, BIT(data, 0) ? "From" : "To", mode_strings[(data >> 2) & 3], (data >> 4) & 3, BIT(data, 6) ? "Transfers" : "Does not transfer");
			logerror("%s:                                   TIE=%d, START=%d, TAG=%04x\n", machine().describe_context(), BIT(data, 7), BIT(data, 8), data >> 16);
			m_channels[VIF1].set_chcr(data);
			break;
		case 0x9010/8: /* VIF1_MADR */
			logerror("%s: dmac_channel_w: VIF1_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_channels[VIF1].set_addr(data);
			break;
		case 0x9020/8: /* VIF1_QWC */
			logerror("%s: dmac_channel_w: VIF1_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_channels[VIF1].set_quadword_count(data);
			break;
		case 0x9030/8: /* VIF1_TADR */
			logerror("%s: dmac_channel_w: VIF1_TADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_channels[VIF1].set_tag_addr(data);
			break;
		case 0x9040/8: /* VIF1_ASR0 */
			logerror("%s: dmac_channel_w: VIF1_ASR0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x9050/8: /* VIF1_ASR1 */
			logerror("%s: dmac_channel_w: VIF1_ASR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xa000/8: /* GIF_CHCR */
			logerror("%s: dmac_channel_w: GIF_CHCR = %08x & %08x (DIR=%s Memory, MOD=%s, ASP=%d, TTE=%s DMAtag, \n", machine().describe_context(), data, mem_mask, BIT(data, 0) ? "From" : "To", mode_strings[(data >> 2) & 3], (data >> 4) & 3, BIT(data, 6) ? "Transfers" : "Does not transfer");
			logerror("%s:                                   TIE=%d, START=%d, TAG=%04x\n", machine().describe_context(), BIT(data, 7), BIT(data, 8), data >> 16);
			m_channels[GIF].set_chcr(data);
			break;
		case 0xa010/8: /* GIF_MADR */
			logerror("%s: dmac_channel_w: GIF_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_channels[GIF].set_addr(data);
			break;
		case 0xa020/8: /* GIF_QWC */
			logerror("%s: dmac_channel_w: GIF_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_channels[GIF].set_quadword_count(data);
			break;
		case 0xa030/8: /* GIF_TADR */
			logerror("%s: dmac_channel_w: GIF_TADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_channels[GIF].set_tag_addr(data);
			break;
		case 0xa040/8: /* GIF_ASR0 */
			logerror("%s: dmac_channel_w: GIF_ASR0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xa050/8: /* GIF_ASR1 */
			logerror("%s: dmac_channel_w: GIF_ASR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xb000/8: /* D3_CHCR */
			logerror("%s: dmac_channel_w: D3_CHCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xb010/8: /* D3_MADR */
			logerror("%s: dmac_channel_w: D3_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xb020/8: /* D3_QWC */
			logerror("%s: dmac_channel_w: D3_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xb400/8: /* D4_CHCR */
			logerror("%s: dmac_channel_w: D4_CHCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xb410/8: /* D4_MADR */
			logerror("%s: dmac_channel_w: D4_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xb420/8: /* D4_QWC */
			logerror("%s: dmac_channel_w: D4_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xb430/8: /* D4_TADR */
			logerror("%s: dmac_channel_w: D4_TADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xc000/8: /* SIF0_CHCR */
			//logerror("%s: dmac_channel_w: SIF0_CHCR = %08x & %08x (DIR=%s Memory, MOD=%s, ASP=%d, TTE=%s DMAtag, \n", machine().describe_context(), data, mem_mask, BIT(data, 0) ? "From" : "To", mode_strings[(data >> 2) & 3], (data >> 4) & 3, BIT(data, 6) ? "Transfers" : "Does not transfer");
			//logerror("%s:                                   TIE=%d, START=%d, TAG=%04x\n", machine().describe_context(), BIT(data, 7), BIT(data, 8), data >> 16);
			m_channels[SIF0].set_chcr(data);
			break;
		case 0xc010/8: /* SIF0_MADR */
			//logerror("%s: dmac_channel_w: SIF0_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_channels[SIF0].set_addr(data);
			break;
		case 0xc020/8: /* SIF0_QWC */
			//logerror("%s: dmac_channel_w: SIF0_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_channels[SIF0].set_quadword_count(data);
			break;
		case 0xc400/8: /* D6_CHCR */
			//logerror("%s: dmac_channel_w: SIF1_CHCR = %08x & %08x (DIR=%s Memory, MOD=%s, ASP=%d, TTE=%s DMAtag, \n", machine().describe_context(), data, mem_mask, BIT(data, 0) ? "From" : "To", mode_strings[(data >> 2) & 3], (data >> 4) & 3, BIT(data, 6) ? "Transfers" : "Does not transfer");
			//logerror("%s:                                   TIE=%d, START=%d, TAG=%04x\n", machine().describe_context(), BIT(data, 7), BIT(data, 8), data >> 16);
			m_channels[SIF1].set_chcr(data);
			break;
		case 0xc410/8: /* D6_MADR */
			//logerror("%s: dmac_channel_w: SIF1_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_channels[SIF1].set_addr(data);
			break;
		case 0xc420/8: /* D6_QWC */
			//logerror("%s: dmac_channel_w: SIF1_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_channels[SIF1].set_quadword_count(data);
			break;
		case 0xc430/8: /* D6_TADR */
			//logerror("%s: dmac_channel_w: SIF1_TADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_channels[SIF1].set_tag_addr(data);
			break;
		case 0xc800/8: /* D7_CHCR */
			logerror("%s: dmac_channel_w: D7_CHCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xc810/8: /* D7_MADR */
			logerror("%s: dmac_channel_w: D7_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xc820/8: /* D7_QWC */
			logerror("%s: dmac_channel_w: D7_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xd000/8: /* D8_CHCR */
			logerror("%s: dmac_channel_w: D8_CHCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xd010/8: /* D8_MADR */
			logerror("%s: dmac_channel_w: D8_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xd020/8: /* D8_QWC */
			logerror("%s: dmac_channel_w: D8_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xd080/8: /* D8_SADR */
			logerror("%s: dmac_channel_w: D8_SADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xd400/8: /* D9_CHCR */
			logerror("%s: dmac_channel_w: D9_CHCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xd410/8: /* D9_MADR */
			logerror("%s: dmac_channel_w: D9_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xd420/8: /* D9_QWC */
			logerror("%s: dmac_channel_w: D9_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xd430/8: /* D9_TADR */
			logerror("%s: dmac_channel_w: D9_TADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xd480/8: /* D9_SADR */
			logerror("%s: dmac_channel_w: D9_SADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		default:
			logerror("%s: dmac_channel_w: Unknown offset %08x = %08x & %08x\n", machine().describe_context(), 0x10008000 + (offset << 3), data, mem_mask);
			break;
	}
}

READ32_MEMBER(ps2_dmac_device::disable_mask_r)
{
	uint32_t ret = m_disable_mask;
	logerror("%s: m_disable_mask = %08x & %08x\n", machine().describe_context(), ret, mem_mask);
	return ret;
}

WRITE32_MEMBER(ps2_dmac_device::disable_mask_w)
{
	logerror("%s: m_disable_mask = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_disable_mask = data;
}

void ps2_dmac_device::channel_t::set_chcr(uint32_t data)
{
	m_chcr = data;
	m_mode = (data >> 2) & 3;
	bool was_enabled = m_enabled;
	m_ienable = BIT(m_chcr, 7);
	m_enabled = BIT(m_chcr, 8);
	if (!was_enabled && m_enabled)
	{
		m_end_tag = m_mode == 0;
	}
	else
	{
		m_end_tag = false;
	}
}
