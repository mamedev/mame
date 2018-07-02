// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 DMAC device skeleton
*
*   To Do:
*     Everything
*
*/

#include "ps2sif.h"
#include "ps2dma.h"
#include "cpu/mips/mips3.h"

DEFINE_DEVICE_TYPE(SONYPS2_DMAC, ps2_dmac_device, "ps2dmac", "EE Core DMAC")

ps2_dmac_device::ps2_dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYPS2_DMAC, tag, owner, clock)
	, device_execute_interface(mconfig, *this)
	, m_ee(*this, finder_base::DUMMY_TAG)
	, m_ram(*this, finder_base::DUMMY_TAG)
	, m_sif(*this, finder_base::DUMMY_TAG)
	, m_icount(0)
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
		save_item(NAME(m_channels[channel].m_qwc), channel);
		save_item(NAME(m_channels[channel].m_addr), channel);
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
				case SIF0:
					transfer_sif0();
					break;
			}
		}
		if (m_last_serviced == 10)
			m_last_serviced = 0;
	}
}

void ps2_dmac_device::transfer_sif0()
{
	channel_t &channel = m_channels[SIF0];
	const uint32_t count = channel.quadword_count();
	if (count)
	{
		printf("count\n");
		if (m_sif->fifo_depth(0) > 4)
		{
			const uint32_t addr = channel.addr();
			for (uint32_t word = 0; word < 4; word++)
			{
				m_ee->space(AS_PROGRAM).write_dword(addr + word, m_sif->fifo_pop(0));
			}
			channel.set_addr(addr + 0x10);
			channel.set_quadword_count(count - 1);
		}
	}
	else
	{
		if (channel.end_tag())
		{
			printf("end\n");
			transfer_finish(SIF0);
		}
		else if (m_sif->fifo_depth(0) >= 2)
        {
			printf("follow\n");
		}
		else
		{
		}
	}
}

void ps2_dmac_device::follow_tag(uint32_t channel)
{
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
		case 0x9000/8: /* D1_CHCR */
			logerror("%s: dmac_channel_r: D1_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x9010/8: /* D1_MADR */
			logerror("%s: dmac_channel_r: D1_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x9020/8: /* D1_QWC */
			logerror("%s: dmac_channel_r: D1_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x9030/8: /* D1_TADR */
			logerror("%s: dmac_channel_r: D1_TADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x9040/8: /* D1_ASR0 */
			logerror("%s: dmac_channel_r: D1_ASR0 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x9050/8: /* D1_ASR1 */
			logerror("%s: dmac_channel_r: D1_ASR1 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xa000/8: /* D2_CHCR */
			logerror("%s: dmac_channel_r: D2_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xa010/8: /* D2_MADR */
			logerror("%s: dmac_channel_r: D2_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xa020/8: /* D2_QWC */
			logerror("%s: dmac_channel_r: D2_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xa030/8: /* D2_TADR */
			logerror("%s: dmac_channel_r: D2_TADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xa040/8: /* D2_ASR0 */
			logerror("%s: dmac_channel_r: D2_ASR0 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xa050/8: /* D2_ASR1 */
			logerror("%s: dmac_channel_r: D2_ASR1 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
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
			logerror("%s: dmac_channel_r: SIF0_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc010/8: /* SIF0_MADR */
			ret = m_channels[SIF0].addr();
			logerror("%s: dmac_channel_r: SIF0_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc020/8: /* SIF0_QWC */
			ret = m_channels[SIF0].quadword_count();
			logerror("%s: dmac_channel_r: SIF0_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc400/8: /* D6_CHCR */
			logerror("%s: dmac_channel_r: D6_CHCR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc410/8: /* D6_MADR */
			logerror("%s: dmac_channel_r: D6_MADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc420/8: /* D6_QWC */
			logerror("%s: dmac_channel_r: D6_QWC (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0xc430/8: /* D6_TADR */
			logerror("%s: dmac_channel_r: D6_TADR (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
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
	static const char* mode_strings[4] = { "Normal", "Chain", "Interleave", "Undefined" };

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
		case 0x9000/8: /* D1_CHCR */
			logerror("%s: dmac_channel_w: D1_CHCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x9010/8: /* D1_MADR */
			logerror("%s: dmac_channel_w: D1_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x9020/8: /* D1_QWC */
			logerror("%s: dmac_channel_w: D1_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x9030/8: /* D1_TADR */
			logerror("%s: dmac_channel_w: D1_TADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x9040/8: /* D1_ASR0 */
			logerror("%s: dmac_channel_w: D1_ASR0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x9050/8: /* D1_ASR1 */
			logerror("%s: dmac_channel_w: D1_ASR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xa000/8: /* D2_CHCR */
			logerror("%s: dmac_channel_w: D2_CHCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xa010/8: /* D2_MADR */
			logerror("%s: dmac_channel_w: D2_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xa020/8: /* D2_QWC */
			logerror("%s: dmac_channel_w: D2_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xa030/8: /* D2_TADR */
			logerror("%s: dmac_channel_w: D2_TADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xa040/8: /* D2_ASR0 */
			logerror("%s: dmac_channel_w: D2_ASR0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xa050/8: /* D2_ASR1 */
			logerror("%s: dmac_channel_w: D2_ASR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
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
			logerror("%s: dmac_channel_w: SIF0_CHCR = %08x & %08x (DIR=%s Memory, MOD=%s, ASP=%d, TTE=%s DMAtag, \n", machine().describe_context(), data, mem_mask, BIT(data, 0) ? "From" : "To", mode_strings[(data >> 2) & 3], (data >> 4) & 3, BIT(data, 6) ? "Transfers" : "Does not transfer");
			logerror("%s:                                   TIE=%d, START=%d, TAG=%04x\n", machine().describe_context(), BIT(data, 7), BIT(data, 8), data >> 16);
			m_channels[SIF0].set_chcr(data);
			break;
		case 0xc010/8: /* SIF0_MADR */
			logerror("%s: dmac_channel_w: SIF0_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_channels[SIF0].set_addr(data);
			break;
		case 0xc020/8: /* SIF0_QWC */
			logerror("%s: dmac_channel_w: SIF0_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_channels[SIF0].set_quadword_count(data);
			break;
		case 0xc400/8: /* D6_CHCR */
			logerror("%s: dmac_channel_w: D6_CHCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xc410/8: /* D6_MADR */
			logerror("%s: dmac_channel_w: D6_MADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xc420/8: /* D6_QWC */
			logerror("%s: dmac_channel_w: D6_QWC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0xc430/8: /* D6_TADR */
			logerror("%s: dmac_channel_w: D6_TADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
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
	m_enabled = BIT(m_chcr, 8);
	if (!was_enabled && m_enabled)
	{
		m_end_tag = m_mode == 0;
	}
}