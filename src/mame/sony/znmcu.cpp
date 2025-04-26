// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "znmcu.h"

DEFINE_DEVICE_TYPE(ZNMCU, znmcu_device, "znmcu", "Sony ZN MCU")

znmcu_device::znmcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ZNMCU, tag, owner, clock),
	m_analog_cb(*this, 0xff),
	m_trackball_cb(*this, 0),
	m_dsw_cb(*this, 0xff),
	m_dsr_cb(*this),
	m_txd_cb(*this),
	m_sck(1),
	m_select(1),
	m_bit(0),
	m_byte(0),
	m_databytes(0)
{
}

void znmcu_device::device_start()
{
	m_mcu_timer = timer_alloc(FUNC(znmcu_device::mcu_tick), this);

	m_dsr_cb(1);
	m_txd_cb(1);

	save_item(NAME(m_sck));
	save_item(NAME(m_select));
	save_item(NAME(m_bit));
	save_item(NAME(m_byte));
	save_item(NAME(m_databytes));
	save_item(NAME(m_analog_read));
	save_item(NAME(m_trackball_read));
	save_item(NAME(m_send));
	save_item(NAME(m_trackball));

	m_send.fill(0);
	m_trackball.fill(0);
}

void znmcu_device::select(int state)
{
	if (m_select != state)
	{
		if (!state)
		{
			m_bit = 0;
			m_byte = 0;
			m_mcu_timer->adjust(attotime::from_usec(50), 0);
		}
		else
		{
			m_txd_cb(1);
			m_dsr_cb(1);
			m_mcu_timer->adjust(attotime::never);
		}

		m_select = state;
	}
}

void znmcu_device::sck(int state)
{
	if (m_sck != state)
	{
		if (!state && !m_select)
		{
			uint8_t data = 0;
			if (m_byte <= m_databytes && m_byte < m_send.size())
			{
				data = m_send[m_byte];
			}

			int dataout = ((data >> m_bit) & 1);
			m_txd_cb(dataout);

			m_bit++;
			if (m_bit == 8)
			{
				if (m_byte < m_databytes)
				{
					m_mcu_timer->adjust(attotime::from_usec(50), 0);
				}

				m_bit = 0;
				m_byte++;
			}
		}

		m_sck = state;
	}
}

void znmcu_device::analog_read(int state)
{
	m_analog_read = state;
}

void znmcu_device::trackball_read(int state)
{
	m_trackball_read = state;
}

TIMER_CALLBACK_MEMBER(znmcu_device::mcu_tick)
{
	m_dsr_cb(param);

	if (!param)
	{
		if (m_byte == 0)
		{
			m_databytes = m_analog_read ? 8 : m_trackball_read ? 6 : 1;
			m_send[0] = (m_databytes << 4) | (m_dsw_cb() & 0xf);

			if (m_analog_read)
				for (int i = 0; i < 8; i++)
					m_send[1 + i] = m_analog_cb[i]();
			else if (m_trackball_read)
			{
				uint16_t d[4];
				for (int i = 0; i < 4; i++)
				{
					uint16_t c = m_trackball_cb[i]();
					d[i] = c - m_trackball[i];
					m_trackball[i] = c;
				}

				m_send[1] = d[0];
				m_send[2] = ((d[0] & 0xf00) >> 8) | ((d[1] & 0xf00) >> 4);
				m_send[3] = d[1];
				m_send[4] = d[2];
				m_send[5] = ((d[2] & 0xf00) >> 8) | ((d[3] & 0xf00) >> 4);
				m_send[6] = d[3];
			}
		}

		m_mcu_timer->adjust(attotime::from_usec(5), 1);
	}
}
