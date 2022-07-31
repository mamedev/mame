// license:BSD-3-Clause
// copyright-holders:Valley Bell
/* Sega 315-5641 / D77591 / 9442CA010 */

#include "emu.h"
#include "315-5641.h"

DEFINE_DEVICE_TYPE(SEGA_315_5641_PCM, sega_315_5641_pcm_device, "315_5641_pcm", "Sega 315-5641 PCM")

sega_315_5641_pcm_device::sega_315_5641_pcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upd7759_device(mconfig, SEGA_315_5641_PCM, tag, owner, clock)
	, m_fifocallback(*this)
	, m_fifo_read(0)
	, m_fifo_write(0)
{
	m_md = 0;   // enforce "slave" mode
}

void sega_315_5641_pcm_device::device_start()
{
	m_fifocallback.resolve_safe();

	upd7759_device::device_start();

	save_item(NAME(m_fifo_data), 0x40);
	save_item(NAME(m_fifo_read));
	save_item(NAME(m_fifo_write));
}

void sega_315_5641_pcm_device::advance_state()
{
	switch (m_state)
	{
	case STATE_DROP_DRQ:
		if (!m_md)
		{
			// Slave Mode: get data from FIFO buffer
			uint8_t fiforead = (m_fifo_read + 1) & 0x3F;
			if (fiforead != m_fifo_write)
			{
				m_fifo_in = m_fifo_data[fiforead];
				m_fifo_read = fiforead;
				if (get_fifo_space() == 0x3F)
				{
					m_fifo_empty = true;
					m_fifocallback(1);
				}
			}
		}
		break;
	}

	upd7759_device::advance_state();
}


void sega_315_5641_pcm_device::port_w(u8 data)
{
	if (m_md)
	{
		// update the FIFO value
		m_fifo_in = data;
	}
	else
	{
		m_fifo_data[m_fifo_write++] = data;
		m_fifo_write &= 0x3F;
		if (m_fifo_empty)
		{
			m_fifo_empty = false;
			m_fifocallback(0);
		}
	}
}


void sega_315_5641_pcm_device::fifo_reset_w(u8 data)
{
	bool reset = !!(data & 1);
	if (!m_fifo_reset && reset)
	{
		m_fifo_read = 0x3F;
		m_fifo_write = 0x00;
		if (m_fifo_empty)
			m_fifocallback(0);
		m_fifo_empty = false;
	}
	m_fifo_reset = reset;
}

void sega_315_5641_pcm_device::internal_start_w(int state)
{
	uint8_t oldstart = m_start;
	uint8_t newstart = (state != 0);

	if (!m_md && m_reset && !oldstart && newstart)
	{
		// Somewhere between "Reset Off" and the first sample data,
		// we need to send a few commands to make the sample stream work.
		// Doing that when rising the "start" line seems to work fine.
		port_w(0xFF);   // "Last Sample" value (must be >= 0x10)
		port_w(0x00);   // Dummy 1
		port_w(0x00);   // Addr MSB
		port_w(0x00);   // Addr LSB
	}

	upd7759_device::internal_start_w(state);
}

uint8_t sega_315_5641_pcm_device::get_fifo_space()
{
	return (m_fifo_read - m_fifo_write) & 0x3F;
}

void sega_315_5641_pcm_device::device_reset()
{
	upd7759_device::device_reset();

	m_fifo_read          = 0x3F;
	m_fifo_write         = 0x00;
	m_fifo_reset         = false;
	if (m_fifo_empty)
		m_fifocallback(0);
	m_fifo_empty         = false;
}
