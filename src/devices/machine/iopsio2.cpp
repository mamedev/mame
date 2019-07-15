// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 IOP SIO2 device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "iopsio2.h"

DEFINE_DEVICE_TYPE(SONYIOP_SIO2, iop_sio2_device, "iopsio2", "PlayStation 2 IOP SIO2")

/*static*/ const size_t iop_sio2_device::BUFFER_SIZE = 512; // total guess based on memcard block size

iop_sio2_device::iop_sio2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYIOP_SIO2, tag, owner, clock)
	, m_intc(*this, finder_base::DUMMY_TAG)
	, m_pad0(*this, finder_base::DUMMY_TAG)
	, m_pad1(*this, finder_base::DUMMY_TAG)
	, m_mc0(*this, finder_base::DUMMY_TAG)
{
}

iop_sio2_device::~iop_sio2_device()
{
}

void iop_sio2_device::device_start()
{
	if (!m_response_timer)
		m_response_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(iop_sio2_device::response_timer), this));

	save_item(NAME(m_buffer));
	save_item(NAME(m_curr_byte));
	save_item(NAME(m_end_byte));
	save_item(NAME(m_ctrl));

	save_item(NAME(m_unknown_0x6c));
	save_item(NAME(m_unknown_0x70));
	save_item(NAME(m_unknown_0x74));

	save_item(NAME(m_cmdbuf));
	save_item(NAME(m_curr_port));
	save_item(NAME(m_cmd_size));
	save_item(NAME(m_cmd_length));
	save_item(NAME(m_databuf[0]));
	save_item(NAME(m_databuf[1]));

	save_item(NAME(m_target_device));
}

void iop_sio2_device::device_reset()
{
	memset(m_buffer, 0, BUFFER_SIZE);
	m_curr_byte = 0;
	m_end_byte = 0;
	m_ctrl = 0;

	m_unknown_0x6c = 0;
	m_unknown_0x70 = 0xf;
	m_unknown_0x74 = 0;

	memset(m_cmdbuf, 0, sizeof(uint32_t) * 16);
	m_curr_port = 0;
	m_cmd_size = 0;
	m_cmd_length = 0;
	memset(m_databuf, 0, sizeof(uint32_t) * 4 * 2);

	m_target_device = 0;
}

READ32_MEMBER(iop_sio2_device::read)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0x64/4:
			ret = receive();
			//logerror("%s: read: FIFO RECV %08x & %08x\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x68/4: // Control?
			ret = m_ctrl;
			//logerror("%s: read: CTRL %08x & %08x\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x6c/4:
			ret = m_unknown_0x6c;
			//logerror("%s: read: Unknown 0x6c (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x70/4:
			ret = m_unknown_0x70;
			//logerror("%s: read: Unknown 0x70 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x74/4:
			ret = m_unknown_0x74;
			//logerror("%s: read: Unknown 0x74 (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		case 0x80/4:
			// Unknown. Case is here to prevent log spam.
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
		case 0x00/4: case 0x04/4: case 0x08/4: case 0x0c/4: case 0x10/4: case 0x14/4: case 0x18/4: case 0x1c/4:
		case 0x20/4: case 0x24/4: case 0x28/4: case 0x2c/4: case 0x30/4: case 0x34/4: case 0x38/4: case 0x3c/4:
			//logerror("%s: write: CMDBUF[%d] = %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
			COMBINE_DATA(&m_cmdbuf[offset]);
			break;
		case 0x40/4: case 0x44/4:
			//logerror("%s: write: DATABUF[0][%d] = %08x & %08x\n", machine().describe_context(), offset - 0x40/4, data, mem_mask);
			COMBINE_DATA(&m_databuf[0][offset - 0x40/4]);
			break;
		case 0x48/4: case 0x4c/4:
			//logerror("%s: write: DATABUF[1][%d] = %08x & %08x\n", machine().describe_context(), offset - 0x48/4, data, mem_mask);
			COMBINE_DATA(&m_databuf[1][offset - 0x48/4]);
			break;
		case 0x50/4: case 0x54/4:
			//logerror("%s: write: DATABUF[2][%d] = %08x & %08x\n", machine().describe_context(), offset - 0x50/4, data, mem_mask);
			COMBINE_DATA(&m_databuf[2][offset - 0x50/4]);
			break;
		case 0x58/4: case 0x5c/4:
			//logerror("%s: write: DATABUF[3][%d] = %08x & %08x\n", machine().describe_context(), offset - 0x58/4, data, mem_mask);
			COMBINE_DATA(&m_databuf[3][offset - 0x58/4]);
			break;
		case 0x60/4:
			//logerror("%s: write: XMIT %08x & %08x\n", machine().describe_context(), data, mem_mask);
			transmit_to_device_hack((uint8_t)data);
			break;
		case 0x68/4: // Control?
		{
			//logerror("%s: write: Control? %08x & %08x\n", machine().describe_context(), data, mem_mask);
			//uint32_t old_ctrl = m_ctrl;
			m_ctrl = data & ~CTRL_IRQ;
			if (data & CTRL_IRQ)
			{
				m_intc->raise_interrupt(iop_intc_device::INT_SIO2);
			}
			else
			{
				m_curr_port = 0;
				m_cmd_size = 0;
				m_cmd_length = 0;
				m_end_byte = 0;
				m_curr_byte = 0;
				m_target_device = 0;
			}
			break;
		}
		case 0x80/4:
			// Unknown. Case is here to prevent log spam.
			break;
		default:
			logerror("%s: write: Unknown offset %08x = %08x & %08x\n", machine().describe_context(), 0x1f808200 + (offset << 2), data, mem_mask);
			break;
	}
}

uint8_t iop_sio2_device::receive()
{
	if (m_curr_byte >= m_end_byte)
	{
		//logerror("D:%d == E:%d, returning 0\n", m_curr_byte, m_end_byte);
		return 0;
	}
	const uint8_t ret = m_buffer[m_curr_byte];
	//logerror("buf[%d] = %02x\n", m_curr_byte, ret);
	m_curr_byte++;
	if (m_curr_byte >= m_end_byte)
	{
		//logerror("Reached end of buffer, resetting to 0\n");
		m_curr_byte = 0;
		m_end_byte = 0;
	}
	return ret;
}

void iop_sio2_device::transmit(uint8_t data)
{
	//logerror("%s: Transmitting: %02x\n", machine().describe_context(), data);
	if (!m_cmd_size && m_cmdbuf[m_curr_port])
	{
		m_cmd_size = (m_cmdbuf[m_curr_port++] & 0x0001ff00) >> 8;
		m_cmd_length = m_cmd_size;
	}

	// TODO: Transmit serially to actual slot device at 250kHz-2MHz as needed
	transmit_to_device_hack(data);
}

ps2_pad_device* iop_sio2_device::pad(uint8_t port)
{
	return (port == 0) ? m_pad0.target() : m_pad1.target();
}

TIMER_CALLBACK_MEMBER(iop_sio2_device::response_timer)
{
	if (param < 2)
	{
		pad(param)->process_fifos();
		while (pad(param)->xmit_fifo_depth() && m_end_byte < BUFFER_SIZE)
		{
			receive_from_device_hack(pad(param)->xmit_fifo_pop());
		}
	}
	else
	{
	}
}

void iop_sio2_device::transmit_to_device_hack(uint8_t data)
{
	if (m_target_device == 0)
	{
		select_device_hack(data);
	}

	//bool cmd_complete = false;
	if (m_cmd_size)
	{
		m_cmd_size--;
		if (!m_cmd_size)
		{
			//cmd_complete = true;
		}
	}

	switch (m_target_device)
	{
		case ps2_pad_device::SIO_DEVICE_ID:
			m_unknown_0x6c = 0x0001d100; // TODO: As above
			receive_from_device_hack(0);

			/*m_unknown_0x6c = 0x00001100; // TODO: What do these bits mean and why does the code expect them?
			pad(m_curr_port)->recv_fifo_push(data);

			if (!m_cmd_size)
			{
			    m_response_timer->adjust(attotime::from_ticks(8*pad(m_curr_port)->xmit_fifo_depth(), 250'000), m_curr_port);
			    m_curr_port++;
			}*/
			break;

		case ps2_mc_device::SIO_DEVICE_ID:
			/*if (m_cmd_size || cmd_complete)
			{
			    logerror("Pushing %02x to memory card, m_cmd_size is %d\n", data, m_cmd_size);
			    m_mc0->recv_fifo_push(data);
			}

			if (cmd_complete)
			{
			    logerror("Command is complete, processing fifos\n");
			    m_unknown_0x6c = 0x00001100;
			    m_mc0->process_fifos();
			    while (m_mc0->xmit_fifo_depth() && m_end_byte < BUFFER_SIZE)
			    {
			        logerror("xmit fifo is %d, end byte is %d\n", m_mc0->xmit_fifo_depth(), m_end_byte);
			        receive_from_device_hack(m_mc0->xmit_fifo_pop());
			    }
			    m_unknown_0x74 = m_cmd_length;
			}*/
			m_unknown_0x6c = 0x0001d100; // TODO: As above
			receive_from_device_hack(0);
			break;

		default:
			m_unknown_0x6c = 0x0001d100; // TODO: As above
			receive_from_device_hack(0);
			logerror("%s: Unknown device selected, can't write data %02x\n", machine().describe_context(), data);
			break;
	}
}

void iop_sio2_device::select_device_hack(uint8_t data)
{
	switch (data)
	{
		case ps2_pad_device::SIO_DEVICE_ID:
			m_target_device = data;
			// TODO: Select transmit frequency, etc.
			//logerror("%s: Selecting transmit device: Pad\n", machine().describe_context());
			break;
		case ps2_mc_device::SIO_DEVICE_ID:
			m_target_device = data;
			//logerror("%s: Selecting transmit device: Memcard\n", machine().describe_context());
			break;

		default:
			m_target_device = 0xff;
			//logerror("%s: Selecting transmit device: Unknown\n", machine().describe_context());
			break;
	}
}

void iop_sio2_device::receive_from_device_hack(uint8_t data)
{
	if (m_end_byte == BUFFER_SIZE)
	{
		return;
	}
	m_buffer[m_end_byte++] = data;
	//logerror("%s: Receiving byte from device (new top: %d)\n", machine().describe_context(), m_end_byte);
}
