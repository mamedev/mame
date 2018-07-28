// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 disc controller device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "iopcdvd.h"

DEFINE_DEVICE_TYPE(SONYIOP_CDVD, iop_cdvd_device, "iopcdvd", "PlayStation 2 disc controller")

/*static*/ const size_t iop_cdvd_device::BUFFER_SIZE = 16; // Total guess

iop_cdvd_device::iop_cdvd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYIOP_CDVD, tag, owner, clock)
	, m_intc(*this, finder_base::DUMMY_TAG)
{
}

iop_cdvd_device::~iop_cdvd_device()
{
}

void iop_cdvd_device::device_start()
{
	save_item(NAME(m_channel[0].m_buffer));
	save_item(NAME(m_channel[0].m_curr));
	save_item(NAME(m_channel[0].m_end));
	save_item(NAME(m_channel[0].m_status));
	save_item(NAME(m_channel[0].m_command));
	save_item(NAME(m_channel[1].m_buffer));
	save_item(NAME(m_channel[1].m_curr));
	save_item(NAME(m_channel[1].m_end));
	save_item(NAME(m_channel[1].m_status));
	save_item(NAME(m_channel[1].m_command));
}

void iop_cdvd_device::device_reset()
{
	for (offs_t i = 0; i < 2; i++)
	{
		memset(m_channel[i].m_buffer, 0, BUFFER_SIZE);
		m_channel[i].m_curr = 0;
		m_channel[i].m_end = 0;
		m_channel[i].m_status = 0;
		m_channel[i].m_command = 0;
	}

	m_channel[0].m_status |= CDVD_STATUS_IDLE;
	m_channel[1].m_status |= CDVD_STATUS_IDLE;
}

READ8_MEMBER(iop_cdvd_device::read)
{
	uint8_t ret = 0;
	switch (offset)
	{
		case 0x04:
			ret = m_channel[CHAN_SERVO].m_command;
			logerror("%s: cdvd_r: Servo Command (%02x)\n", machine().describe_context(), ret);
			break;
		case 0x05:
			ret = m_channel[CHAN_SERVO].m_status;
			logerror("%s: cdvd_r: Servo Status (%02x)\n", machine().describe_context(), ret);
			break;
		case 0x16:
			ret = m_channel[CHAN_DATA].m_command;
			logerror("%s: cdvd_r: Data Command (%02x)\n", machine().describe_context(), ret);
			break;
		case 0x17:
			ret = m_channel[CHAN_DATA].m_status;
			logerror("%s: cdvd_r: Data Status (%02x)\n", machine().describe_context(), ret);
			break;
		case 0x18:
			ret = data_fifo_pop();
			logerror("%s: cdvd_r: Data FIFO Out (%02x) (%d left)\n", machine().describe_context(), ret, m_channel[CHAN_DATA].m_end - m_channel[CHAN_DATA].m_curr);
			break;
		default:
			logerror("%s: cdvd_r: Unknown read: %08x\n", machine().describe_context(), 0x1f402000 + offset);
			break;
	}
	return ret;
}

void iop_cdvd_device::data_fifo_push(uint8_t data)
{
	drive_channel_t &channel = m_channel[CHAN_DATA];
	if (channel.m_end >= BUFFER_SIZE)
	{
		return;
	}

	channel.m_buffer[channel.m_end++] = data;
}

uint8_t iop_cdvd_device::data_fifo_pop()
{
	drive_channel_t &channel = m_channel[CHAN_DATA];
	if (channel.m_end == channel.m_curr)
	{
		channel.m_status |= CDVD_STATUS_IDLE;
		return 0;
	}

	const uint8_t ret = channel.m_buffer[channel.m_curr++];
	if (channel.m_end == channel.m_curr)
	{
		channel.m_status |= CDVD_STATUS_IDLE;
		channel.m_curr = 0;
		channel.m_end = 0;
	}
	return ret;
}

WRITE8_MEMBER(iop_cdvd_device::write)
{
	switch (offset)
	{
		case 0x16:
			handle_data_command(data);
			break;
		case 0x17:
			data_fifo_push(data);
			logerror("%s: cdvd_w: Data FIFO push? %02x\n", machine().describe_context(), data);
			break;
		default:
			logerror("%s: cdvd_w: Unknown write: %08x = %02x\n", machine().describe_context(), 0x1f402000 + offset, data);
			break;
	}
}

void iop_cdvd_device::handle_data_command(uint8_t data)
{
	drive_channel_t &channel = m_channel[CHAN_DATA];
	channel.m_command = data;
	channel.m_status &= ~CDVD_STATUS_IDLE;
	if (channel.m_end)
		logerror("%s: cdvd_w: Data Command warning: Issuing command without reading previous results\n", machine().describe_context());

	switch (data)
	{
		case 0x08:
			logerror("%s: cdvd_w: Command 0x08?\n", machine().describe_context());
			for (size_t i = 0; i < 8; i++)
				data_fifo_push(0);
			break;
		case 0x15:
			logerror("%s: cdvd_w: Command 0x15?\n", machine().describe_context());
			data_fifo_push(1);
			break;
		case 0x40:
			logerror("%s: cdvd_w: Command 0x40?\n", machine().describe_context());
			data_fifo_push(0);
			break;
		case 0x41:
			logerror("%s: cdvd_w: Command 0x41?\n", machine().describe_context());
			for (size_t i = 0; i < 0x10; i++)
				data_fifo_push(0);
			break;
		case 0x43:
			logerror("%s: cdvd_w: Command 0x43?\n", machine().describe_context());
			data_fifo_push(0);
			break;
		default:
			logerror("%s: cdvd_w: Unknown command(?) %02x\n", machine().describe_context(), data);
			break;
	}
}
