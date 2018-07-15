// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 disc controller device skeleton
*
*   To Do:
*     Everything
*
*/

#include "iopcdvd.h"

/*static*/ const size_t iop_cdvd_device::BUFFER_SIZE = 2048; // Total guess

DEFINE_DEVICE_TYPE(SONYIOP_CDVD, iop_cdvd_device, "iopcdvd", "Playstation 2 disc controller")

iop_cdvd_device::iop_cdvd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYIOP_CDVD, tag, owner, clock)
	, m_intc(*this, finder_base::DUMMY_TAG)
{
}

void iop_cdvd_device::device_start()
{
	m_out_buf = std::make_unique<uint8_t[]>(BUFFER_SIZE);

	save_item(NAME(m_status_0x05));
	save_item(NAME(m_status_0x17));
	save_item(NAME(m_command));
	save_item(NAME(m_out_count));
	save_item(NAME(m_out_curr));
}

void iop_cdvd_device::device_reset()
{
	m_status_0x05 = CDVD_STATUS_BOOT;
	m_status_0x17 = CDVD_STATUS_IDLE;
	m_command = 0;
	memset(&m_out_buf[0], 0, BUFFER_SIZE);
	m_out_count = 0;
	m_out_curr = 0;
}

READ8_MEMBER(iop_cdvd_device::read)
{
	uint8_t ret = 0;
	switch (offset)
	{
		case 0x05:
			ret = m_status_0x05;
			logerror("%s: cdvd_r: Status 0x05? (%02x)\n", machine().describe_context(), ret);
			break;
		case 0x16:
			ret = m_command;
			logerror("%s: cdvd_r: Command? (%02x)\n", machine().describe_context(), ret);
			break;
		case 0x17:
			ret = m_status_0x17;
			logerror("%s: cdvd_r: Status 0x17? (%02x)\n", machine().describe_context(), ret);
			break;
		case 0x18:
			ret = m_out_buf[m_out_curr];
			if (m_out_curr < m_out_count)
			{
				m_out_curr++;
				if (m_out_curr == m_out_count)
				{
					m_status_0x17 |= CDVD_STATUS_IDLE;
					m_out_count = 0;
					m_out_curr = 0;
				}
			}
			logerror("%s: cdvd_r: Command Output (%02x) (%d left)\n", machine().describe_context(), ret, m_out_count - m_out_curr);
			break;
		default:
			logerror("%s: cdvd_r: Unknown read: %08x\n", machine().describe_context(), 0x1f402000 + offset);
			break;
	}
	return ret;
}

WRITE8_MEMBER(iop_cdvd_device::write)
{
	switch (offset)
	{
		case 0x16:
			m_command = data;
			m_status_0x17 &= ~(CDVD_STATUS_IDLE | CDVD_STATUS_BOOT);
			m_status_0x05 &= ~CDVD_STATUS_BOOT;
			if (m_out_count > 0)
				logerror("%s: cdvd_w: Command warning: Issuing command without reading previous results\n", machine().describe_context());
			switch (data)
			{
				case 0x15:
					logerror("%s: cdvd_w: Command 0x15?\n", machine().describe_context());
					m_out_buf[m_out_count++] = 1;
					m_intc->raise_interrupt(5);
					break;
				case 0x40:
					logerror("%s: cdvd_w: Command 0x40?\n", machine().describe_context());
					m_out_buf[m_out_count++] = 0;
					break;
				case 0x41:
					logerror("%s: cdvd_w: Command 0x41?\n", machine().describe_context());
					m_out_count = 0x10;
					memset(&m_out_buf[0], 0, 0x10);
					break;
				case 0x43:
					logerror("%s: cdvd_w: Command 0x43?\n", machine().describe_context());
					m_out_buf[m_out_count++] = 0;
					break;
				default:
					logerror("%s: cdvd_r: Unknown command(?) %02x\n", machine().describe_context(), data);
					break;
			}
			break;
		default:
			logerror("%s: cdvd_w: Unknown write: %08x = %02x\n", machine().describe_context(), 0x1f402000 + offset, data);
			break;
	}
}
