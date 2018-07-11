// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 SIO2 device skeleton
*
*   To Do:
*     Everything
*
*/

#include "iopsio2.h"

/*static*/ const size_t iop_sio2_device::BUFFER_SIZE = 64; // total guess

DEFINE_DEVICE_TYPE(SONYIOP_SIO2, iop_sio2_device, "iopsio2", "Playstation 2 SIO2")

iop_sio2_device::iop_sio2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYIOP_SIO2, tag, owner, clock)
	, m_intc(*this, finder_base::DUMMY_TAG)
{
}

void iop_sio2_device::device_start()
{
	save_item(NAME(m_ctrl));
	save_item(NAME(m_receive_buf));
	save_item(NAME(m_receive_curr));
	save_item(NAME(m_receive_end));
	save_item(NAME(m_transmit_buf));
	save_item(NAME(m_transmit_curr));
	save_item(NAME(m_transmit_end));
}

void iop_sio2_device::device_reset()
{
	m_ctrl = 0;

	memset(m_receive_buf, 0, BUFFER_SIZE);
	m_receive_curr = 0;
	m_receive_end = 0;

	memset(m_transmit_buf, 0, BUFFER_SIZE);
	m_transmit_curr = 0;
	m_transmit_end = 0;
}

READ32_MEMBER(iop_sio2_device::read)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0x68/4: // Control?
			logerror("%s: read: CTRL %08x & %08x\n", machine().describe_context(), ret, mem_mask);
			break;
		default:
			logerror("%s: read: Unknown offset %08x & %08x\n", machine().describe_context(), 0x1f808200 + (offset << 2), mem_mask);
			break;
	}
	return ret;
}

WRITE32_MEMBER(iop_sio2_device::write)
{
	switch (offset)
	{
		case 0x68/4: // Control?
		{
			uint32_t old_ctrl = m_ctrl;
			m_ctrl = data & CTRL_IRQ;
			if (old_ctrl != m_ctrl && (m_ctrl & CTRL_IRQ))
			{
				m_intc->raise_interrupt(iop_intc_device::INT_SIO2);
			}
			break;
		}
		default:
			logerror("%s: write: Unknown offset %08x = %08x & %08x\n", machine().describe_context(), 0x1f808200 + (offset << 2), data, mem_mask);
			break;
	}
}

uint8_t iop_sio2_device::transmit()
{
	if (m_transmit_curr == m_transmit_end)
	{
		return 0;
	}
	uint8_t ret = m_transmit_buf[m_transmit_curr];
	m_transmit_curr = (m_transmit_curr + 1) & (BUFFER_SIZE-1);
	return ret;
}

void iop_sio2_device::receive(uint8_t data)
{
	if (m_receive_end >= BUFFER_SIZE)
	{
		return;
	}
	m_receive_buf[m_receive_end] = data;
	m_receive_end++;
}
