// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 memory card device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "ps2mc.h"

DEFINE_DEVICE_TYPE(SONYPS2_MC, ps2_mc_device, "ps2mc", "PlayStation 2 Memory Card")

/*static*/ const size_t ps2_mc_device::BUFFER_SIZE = 512; // Total guess
/*static*/ const uint8_t ps2_mc_device::DEFAULT_TERMINATOR = 0x55;

ps2_mc_device::ps2_mc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYPS2_MC, tag, owner, clock)
{
}

ps2_mc_device::~ps2_mc_device()
{
}

void ps2_mc_device::device_start()
{
	save_item(NAME(m_recv_buf));
	save_item(NAME(m_xmit_buf));
	save_item(NAME(m_curr_recv));
	save_item(NAME(m_curr_xmit));
	save_item(NAME(m_end_recv));
	save_item(NAME(m_end_xmit));
	save_item(NAME(m_cmd));
	save_item(NAME(m_cmd_size));
	save_item(NAME(m_terminator));
	save_item(NAME(m_status));
}

void ps2_mc_device::device_reset()
{
	memset(m_recv_buf, 0, BUFFER_SIZE);
	memset(m_xmit_buf, 0, BUFFER_SIZE);

	m_curr_recv = 0;
	m_curr_xmit = 0;

	m_end_recv = 0;
	m_end_xmit = 0;

	m_cmd = 0;
	m_cmd_size = 0;
	m_terminator = DEFAULT_TERMINATOR;
	m_status = 0;
}

void ps2_mc_device::recv_fifo_push(uint8_t data)
{
	if (m_end_recv >= BUFFER_SIZE)
	{
		return;
	}

	m_recv_buf[m_end_recv++] = data;

	//logerror("%s: Receiving %02x from SIO2, new top: %d\n", machine().describe_context(), data, m_end_recv);
}

void ps2_mc_device::xmit_fifo_push(uint8_t data)
{
	if (m_end_xmit >= BUFFER_SIZE)
	{
		return;
	}

	//logerror("%s: Pushing %02x onto transmit FIFO\n", machine().describe_context(), data);

	m_xmit_buf[m_end_xmit++] = data;
}

uint8_t ps2_mc_device::xmit_fifo_pop()
{
	uint8_t ret = 0;
	if (m_curr_xmit < m_end_xmit)
	{
		ret = m_xmit_buf[m_curr_xmit++];
	}
	if (m_curr_xmit >= m_end_xmit)
	{
		m_curr_xmit = 0;
		m_end_xmit = 0;
	}
	return ret;
}

uint8_t ps2_mc_device::recv_fifo_pop()
{
	uint8_t ret = 0;
	if (m_curr_recv < m_end_recv)
	{
		ret = m_recv_buf[m_curr_recv++];
	}
	if (m_curr_recv >= m_end_recv)
	{
		m_curr_recv = 0;
		m_end_recv = 0;
	}
	//logerror("recv_fifo_pop %02x, %d/%d\n", ret, m_curr_recv, m_end_recv);
	return ret;
}

void ps2_mc_device::process_fifos()
{
	m_status = 0;
	m_cmd_size = 0;

	while (recv_fifo_depth())
	{
		const uint8_t data = recv_fifo_pop();
		m_cmd_size++;

		if (m_cmd_size == 1)
		{
			if (data == SIO_DEVICE_ID)
			{
				xmit_fifo_push(0x2b);
			}
			else
			{
				logerror("%s: Unknkown command byte 1: %02x\n", machine().describe_context(), data);
			}
		}
		else if (m_cmd_size == 2)
		{
			process_command(data);
		}
	}

	m_curr_recv = 0;
	m_end_recv = 0;
}

void ps2_mc_device::process_command(uint8_t data)
{
	m_cmd = data;

	switch (m_cmd)
	{
		case CMD_UNKNOWN_F3:
			//logerror("%s: MC command: Unknown 0xf3\n", machine().describe_context());
			cmd_init();
			m_status = 0;
			break;
		case CMD_INIT:
			//logerror("%s: MC command: Init (%02x)\n", machine().describe_context(), m_cmd);
			cmd_init();
			break;
		case CMD_GET_TERM:
			//logerror("%s: MC command: Get Terminator\n", machine().describe_context());
			cmd_get_term();
			break;
		default:
			logerror("%s: Unknown MC command: %02x\n", machine().describe_context(), m_cmd);
			break;
	}
}

void ps2_mc_device::cmd_init()
{
	while (recv_fifo_depth())
	{
		recv_fifo_pop();

		if (m_cmd_size == 2)
		{
			m_status = 0x8c;
			xmit_fifo_push(m_terminator);
		}

		m_cmd_size++;
	}
}

void ps2_mc_device::cmd_get_term()
{
	while (recv_fifo_depth())
	{
		recv_fifo_pop();

		if (m_cmd_size == 2)
		{
			m_status = 0x8b;
			xmit_fifo_push(m_terminator);
			xmit_fifo_push(0x55);
		}

		m_cmd_size++;
	}
}
