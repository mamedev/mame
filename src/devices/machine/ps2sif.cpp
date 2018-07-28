// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 SIF device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "ps2sif.h"

DEFINE_DEVICE_TYPE(SONYPS2_SIF, ps2_sif_device, "ps2sif", "PlayStation 2 SIF")

/*static*/ const size_t ps2_sif_device::MAX_FIFO_DEPTH = 0x20;

ps2_sif_device::ps2_sif_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYPS2_SIF, tag, owner, clock)
	, m_intc(*this, finder_base::DUMMY_TAG)
{
}

ps2_sif_device::~ps2_sif_device()
{
}

void ps2_sif_device::device_start()
{
	save_item(NAME(m_ms_mailbox));
	save_item(NAME(m_sm_mailbox));
	save_item(NAME(m_ms_flag));
	save_item(NAME(m_sm_flag));
	save_item(NAME(m_ctrl));

	m_fifo[0] = std::make_unique<uint32_t[]>(MAX_FIFO_DEPTH);
	m_fifo[1] = std::make_unique<uint32_t[]>(MAX_FIFO_DEPTH);
}

void ps2_sif_device::device_reset()
{
	m_ms_mailbox = 0;
	m_sm_mailbox = 0;
	m_ms_flag = 0;
	m_sm_flag = 0;
	m_ctrl = 0;

	memset(m_fifo[0].get(), 0, sizeof(uint32_t) * MAX_FIFO_DEPTH);
	memset(m_fifo[1].get(), 0, sizeof(uint32_t) * MAX_FIFO_DEPTH);
	m_fifo_curr[0] = 0;
	m_fifo_curr[1] = 0;
}

READ32_MEMBER(ps2_sif_device::ee_r)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0:
			ret = m_ms_mailbox;
			//logerror("%s: ee_r: SIF master->slave mailbox (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 2:
			ret = m_sm_mailbox;
			//logerror("%s: ee_r: SIF slave->master mailbox (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 4:
			ret = m_ms_flag;
			//logerror("%s: ee_r: SIF master->slave flag (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 6:
			ret = m_sm_flag;
			//if (ret != 0)
				//logerror("%s: ee_r: SIF slave->master flag (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 8:
			ret = m_ctrl | 0xF0000102;
			//logerror("%s: ee_r: SIF control (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		default:
			logerror("%s: ee_r: Unknown (%08x & %08x)\n", machine().describe_context(), 0x1000f200 + (offset << 3), mem_mask);
			break;
	}
	return ret;
}

WRITE32_MEMBER(ps2_sif_device::ee_w)
{
	switch (offset)
	{
		case 0:
			//logerror("%s: ee_w: SIF master->slave mailbox (%08x & %08x)\n", machine().describe_context(), data, mem_mask);
			m_ms_mailbox = data;
			break;
		case 4:
			//logerror("%s: ee_w: SIF set master->slave flag (%08x & %08x)\n", machine().describe_context(), data, mem_mask);
			m_ms_flag |= data;
			break;
		case 6:
			//logerror("%s: ee_w: SIF clear slave->master flag (%08x & %08x)\n", machine().describe_context(), data, mem_mask);
			m_sm_flag &= ~data;
			break;
		case 8:
			//logerror("%s: ee_w: SIF set control = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			if (BIT(data, 8))
				m_ctrl |= (1 << 8);
			else
				m_ctrl &= ~(1 << 8);
			// ??
			break;
		default:
			logerror("%s: ee_w: Unknown %08x = %08x & %08x\n", machine().describe_context(), 0x1000f200 + (offset << 3), data, mem_mask);
			break;
	}
}

READ32_MEMBER(ps2_sif_device::iop_r)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0:
			ret = m_ms_mailbox;
			//logerror("%s: iop_r: SIF master->slave mailbox (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 4:
			ret = m_sm_mailbox;
			//logerror("%s: iop_r: SIF slave->master mailbox (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 8:
			ret = m_ms_flag;
			//if (ret != 0)
				//logerror("%s: iop_r: SIF master->slave flag (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 12:
			ret = m_sm_flag;
			//logerror("%s: iop_r: SIF slave->master flag (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 16:
			ret = m_ctrl | 0xf0000002;
			//logerror("%s: iop_r: SIF control register (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		default:
			logerror("%s: iop_r: Unknown read (%08x & %08x)\n", machine().describe_context(), 0x1d000000 + (offset << 2), mem_mask);
			break;
	}
	return ret;
}

WRITE32_MEMBER(ps2_sif_device::iop_w)
{
	switch (offset)
	{
		case 4:
			//logerror("%s: iop_w: SIF slave->master mailbox (%08x & %08x)\n", machine().describe_context(), data, mem_mask);
			m_sm_mailbox = data;
			break;
		case 8:
			//logerror("%s: iop_w: SIF clear master->slave flag (%08x & %08x)\n", machine().describe_context(), data, mem_mask);
			m_ms_flag &= ~data;
			break;
		case 12:
			//logerror("%s: iop_w: SIF set slave->master flag (%08x & %08x)\n", machine().describe_context(), data, mem_mask);
			m_sm_flag |= data;
			break;
		case 16:
			//logerror("%s: iop_w: SIF set control (%08x & %08x)\n", machine().describe_context(), data, mem_mask);
			m_ctrl ^= data & 0xf0;
			if (data & 0x80 || data & 0x20)
			{
				m_ctrl &= ~0xf000;
				m_ctrl |= 0x2000;
			}
			break;
		default:
			logerror("%s: iop_w: Unknown write %08x = %08x & %08x\n", machine().describe_context(), 0x1d000000 + (offset << 2), data, mem_mask);
			break;
	}
}

uint32_t ps2_sif_device::fifo_depth(uint32_t channel)
{
	assert(channel < 2);
	return m_fifo_curr[channel];
}

uint32_t ps2_sif_device::fifo_pop(uint32_t channel)
{
	assert(channel < 2);
	assert(m_fifo_curr[channel] > 0);
	uint32_t ret = m_fifo[channel][0];
	m_fifo_curr[channel]--;
	memcpy(&m_fifo[channel][0], &m_fifo[channel][1], sizeof(uint32_t) * m_fifo_curr[channel]);
	return ret;
}

void ps2_sif_device::fifo_push(uint32_t channel, uint32_t value)
{
	assert(m_fifo_curr[channel] < MAX_FIFO_DEPTH);
	m_fifo[channel][m_fifo_curr[channel]] = value;
	m_fifo_curr[channel]++;
}
