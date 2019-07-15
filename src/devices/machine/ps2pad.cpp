// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony DualShock 2 device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "ps2pad.h"

DEFINE_DEVICE_TYPE(SONYPS2_PAD, ps2_pad_device, "ps2pad", "Sony DualShock 2")

/*static*/ const size_t ps2_pad_device::BUFFER_SIZE = 64; // Total guess

ps2_pad_device::ps2_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYPS2_PAD, tag, owner, clock)
{
}

ps2_pad_device::~ps2_pad_device()
{
}

void ps2_pad_device::device_start()
{
	save_item(NAME(m_recv_buf));
	save_item(NAME(m_xmit_buf));
	save_item(NAME(m_curr_recv));
	save_item(NAME(m_curr_xmit));
	save_item(NAME(m_end_recv));
	save_item(NAME(m_end_xmit));
	save_item(NAME(m_cmd));
	save_item(NAME(m_cmd_size));
	save_item(NAME(m_configuring));
}

void ps2_pad_device::device_reset()
{
	memset(m_recv_buf, 0, BUFFER_SIZE);
	memset(m_xmit_buf, 0, BUFFER_SIZE);

	m_curr_recv = 0;
	m_curr_xmit = 0;

	m_end_recv = 0;
	m_end_xmit = 0;

	m_cmd = 0;
	m_cmd_size = 0;
	m_configuring = false;
}

void ps2_pad_device::recv_fifo_push(uint8_t data)
{
	if (m_end_recv >= BUFFER_SIZE)
	{
		return;
	}

	//logerror("%s: Receiving %02x from SIO2\n", machine().describe_context(), data);

	m_recv_buf[m_end_recv++] = data;
}

void ps2_pad_device::xmit_fifo_push(uint8_t data)
{
	if (m_end_xmit >= BUFFER_SIZE)
	{
		return;
	}

	//logerror("%s: Pushing %02x onto transmit FIFO\n", machine().describe_context(), data);

	m_xmit_buf[m_end_xmit++] = data;
}

uint8_t ps2_pad_device::xmit_fifo_pop()
{
	uint8_t ret = 0;
	if (m_curr_xmit < m_end_xmit)
	{
		ret = m_xmit_buf[m_curr_xmit++];
	}
	if (m_curr_xmit == m_end_xmit)
	{
		m_curr_xmit = 0;
		m_end_xmit = 0;
	}
	return ret;
}

uint8_t ps2_pad_device::recv_fifo_pop()
{
	uint8_t ret = 0;
	if (m_curr_recv < m_end_recv)
	{
		ret = m_recv_buf[m_curr_recv++];
	}
	if (m_curr_recv == m_end_recv)
	{
		m_curr_recv = 0;
		m_end_recv = 0;
	}
	return ret;
}

void ps2_pad_device::process_fifos()
{
	m_cmd_size = 0;

	while (recv_fifo_depth())
	{
		const uint8_t data = recv_fifo_pop();
		m_cmd_size++;

		if (m_cmd_size == 1)
		{
			if (data == SIO_DEVICE_ID)
			{
				xmit_fifo_push(0xff);
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

void ps2_pad_device::process_command(uint8_t data)
{
	m_cmd = data;

	switch (m_cmd)
	{
		case CMD_READ_BUTTONS:
			//logerror("%s: Pad command: Read Buttons\n", machine().describe_context());
			cmd_read_buttons();
			break;
		case CMD_CONFIG:
			//logerror("%s: Pad command: Config\n", machine().describe_context());
			cmd_config();
			break;
		case CMD_GET_MODEL:
			//logerror("%s: Pad command: Get Model\n", machine().describe_context());
			cmd_get_model();
			break;
		case CMD_GET_ACT:
			//logerror("%s: Pad command: Get Act\n", machine().describe_context());
			cmd_get_act();
			break;
		case CMD_GET_COMB:
			//logerror("%s: Pad command: Get Comb\n", machine().describe_context());
			cmd_get_comb();
			break;
		case CMD_GET_MODE:
			//logerror("%s: Pad command: Get Mode\n", machine().describe_context());
			cmd_get_comb();
			break;
		default:
			logerror("%s: Unknown pad command: %02x, forcing idle\n", machine().describe_context(), m_cmd);
			break;
	}
}

void ps2_pad_device::cmd_read_buttons()
{
	while (recv_fifo_depth())
	{
		recv_fifo_pop();

		if (m_cmd_size == 2)
		{
			xmit_fifo_push(0x41);
			xmit_fifo_push(0x5a);
			xmit_fifo_push(0x00);
			xmit_fifo_push(0x00);
		}

		m_cmd_size++;
	}
}

void ps2_pad_device::cmd_config()
{
	if (m_configuring)
	{
		xmit_fifo_push(0xf3);
		xmit_fifo_push(0x5a);
		xmit_fifo_push(0x00);
		xmit_fifo_push(0x00);
		xmit_fifo_push(0x00);
		xmit_fifo_push(0x00);
		xmit_fifo_push(0x00);
		xmit_fifo_push(0x00);
		return;
	}

	while (recv_fifo_depth())
	{
		const uint8_t data = recv_fifo_pop();

		if (m_cmd_size == 2)
		{
			xmit_fifo_push(0x41);
			xmit_fifo_push(0x5a);
			xmit_fifo_push(0x00);
			xmit_fifo_push(0x00);
		}
		else if (m_cmd_size == 3)
		{
			m_configuring = BIT(data, 0);
			//logerror("%s: Entering config mode? %c\n", machine().describe_context(), m_configuring ? 'Y' : 'N');
		}

		m_cmd_size++;
	}
}

void ps2_pad_device::cmd_get_model()
{
	while (recv_fifo_depth())
	{
		recv_fifo_pop();

		if (m_cmd_size == 2)
		{
			xmit_fifo_push(0xf3);
			xmit_fifo_push(0x5a);
			xmit_fifo_push(0x01);
			xmit_fifo_push(0x02);
			xmit_fifo_push(0x00);
			xmit_fifo_push(0x02);
			xmit_fifo_push(0x01);
			xmit_fifo_push(0x00);
		}

		m_cmd_size++;
	}
}

void ps2_pad_device::cmd_get_act()
{
	while (recv_fifo_depth())
	{
		const uint8_t data = recv_fifo_pop();

		if (m_cmd_size == 3)
		{
			xmit_fifo_push(0xf3);
			xmit_fifo_push(0x5a);
			xmit_fifo_push(0x00);
			xmit_fifo_push(0x00);
			xmit_fifo_push(0x01);
			if (data < 2)
			{
				xmit_fifo_push(0x01);
				xmit_fifo_push(0x01);
				xmit_fifo_push(0x14);
			}
			else
			{
				xmit_fifo_push(0x02);
				xmit_fifo_push(0x00);
				xmit_fifo_push(0x0a);
			}
		}

		m_cmd_size++;
	}
}

void ps2_pad_device::cmd_get_comb()
{
	while (recv_fifo_depth())
	{
		recv_fifo_pop();

		if (m_cmd_size == 2)
		{
			xmit_fifo_push(0xf3);
			xmit_fifo_push(0x5a);
			xmit_fifo_push(0x00);
			xmit_fifo_push(0x00);
			xmit_fifo_push(0x02);
			xmit_fifo_push(0x00);
			xmit_fifo_push(0x01);
			xmit_fifo_push(0x00);
		}

		m_cmd_size++;
	}
}

void ps2_pad_device::cmd_get_mode()
{
	while (recv_fifo_depth())
	{
		const uint8_t data = recv_fifo_pop();

		if (m_cmd_size == 3)
		{
			xmit_fifo_push(0xf3);
			xmit_fifo_push(0x5a);
			xmit_fifo_push(0x00);
			xmit_fifo_push(0x00);
			xmit_fifo_push(0x00);
			if (data < 2)
			{
				xmit_fifo_push(4 + data * 3);
			}
			else
			{
				xmit_fifo_push(0x00);
			}
			xmit_fifo_push(0x00);
			xmit_fifo_push(0x00);
		}

		m_cmd_size++;
	}
}
