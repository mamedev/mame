// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "znmcu.h"

DEFINE_DEVICE_TYPE(ZNMCU, znmcu_device, "znmcu", "Sony ZN MCU")

znmcu_device::znmcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ZNMCU, tag, owner, clock),
	m_dsw_handler(*this),
	m_analog1_handler(*this),
	m_analog2_handler(*this),
	m_dataout_handler(*this),
	m_dsr_handler(*this),
	m_select(1),
	m_clk(1),
	m_bit(0),
	m_byte(0),
	m_databytes(0)
{
}

void znmcu_device::device_start()
{
	m_dsw_handler.resolve_safe(0xff);
	m_analog1_handler.resolve_safe(0xff);
	m_analog2_handler.resolve_safe(0xff);
	m_dataout_handler.resolve_safe();
	m_dsr_handler.resolve_safe();

	m_mcu_timer = timer_alloc(0);

	m_dataout_handler(1);
	m_dsr_handler(1);

	save_item(NAME(m_select));
	save_item(NAME(m_clk));
	save_item(NAME(m_bit));
	save_item(NAME(m_byte));
	save_item(NAME(m_databytes));
	save_item(NAME(m_send));

	memset(m_send, 0, sizeof(m_send));
}

WRITE_LINE_MEMBER(znmcu_device::write_select)
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
			m_dataout_handler(1);
			m_dsr_handler(1);
			m_mcu_timer->adjust(attotime::never);
		}

		m_select = state;
	}
}

WRITE_LINE_MEMBER(znmcu_device::write_clock)
{
	if (m_clk != state)
	{
		if (!state && !m_select)
		{
			uint8_t data = 0;
			if (m_byte <= m_databytes && m_byte < MaxBytes)
			{
				data = m_send[m_byte];
			}

			int dataout = ((data >> m_bit) & 1);
			m_dataout_handler(dataout);

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

		m_clk = state;
	}
}

void znmcu_device::device_timer(emu_timer &timer, device_timer_id tid, int param)
{
	m_dsr_handler(param);

	if (!param)
	{
		if (m_byte == 0)
		{
			m_databytes = 2;
			m_send[0] = (m_databytes << 4) | (m_dsw_handler() & 0xf);
			m_send[1] = m_analog1_handler();
			m_send[2] = m_analog2_handler();
		}

		m_mcu_timer->adjust(attotime::from_usec(5), 1);
	}
}
