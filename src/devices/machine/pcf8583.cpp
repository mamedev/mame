// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen, Ryan Holtz
/*********************************************************************

    Philips PCF8583 Clock and Calendar with 240 x 8-bit RAM

    TODO:
        - Alarm mode
        - Event-counter mode
        - Clock select
        - Clock output
        - Interrupts

*********************************************************************/

#include "emu.h"
#include "pcf8583.h"

DEFINE_DEVICE_TYPE(PCF8583, pcf8583_device, "pcf8583", "PCF8583 RTC with 240x8 RAM")

pcf8583_device::pcf8583_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PCF8583, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, device_nvram_interface(mconfig, *this)
	, m_irq_callback(*this)
{
}

void pcf8583_device::device_start()
{
	std::fill(std::begin(m_register), std::end(m_register), 0);

	m_timer = timer_alloc(TIMER_TICK);
	m_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));

	save_item(NAME(m_scl));
	save_item(NAME(m_sda));
	save_item(NAME(m_inp));
	save_item(NAME(m_transfer_active));
	save_item(NAME(m_bit_index));
	save_item(NAME(m_irq));
	save_item(NAME(m_data_recv_index));
	save_item(NAME(m_data_recv));
	save_item(NAME(m_mode));
	save_item(NAME(m_pos));
	save_item(NAME(m_write_address));
	save_item(NAME(m_read_address));

	m_irq_callback.resolve_safe();
}

void pcf8583_device::device_reset()
{
	m_scl = 1;
	m_sda = 1;
	m_transfer_active = false;
	m_inp = 0;
	m_mode = RTC_MODE_RECV;
	m_bit_index = 0;
	m_irq = false;
	m_pos = 0;
	clear_rx_buffer();
	set_time(true, get_date_year(), get_date_month(), get_date_day(), 0, get_time_hour(), get_time_minute(), get_time_second());
}

void pcf8583_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_TICK:
			if (!BIT(m_data[REG_CONTROL], CONTROL_STOP_BIT))
				advance_hundredths();
			break;
	}
}

void pcf8583_device::advance_hundredths()
{
	uint8_t hundredths = bcd_to_integer(m_data[REG_HUNDREDTHS]);
	hundredths++;
	if (hundredths >= 100)
	{
		hundredths = 0;
		advance_seconds();
		m_irq = !m_irq;
		printf("Toggling IRQ: %d\n", m_irq ? 1 : 0);
		m_irq_callback(m_irq);
	}
	m_data[REG_HUNDREDTHS] = convert_to_bcd(hundredths);
}


void pcf8583_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	set_time_second(second);
	set_time_minute(minute);
	set_time_hour(hour);
	set_date_day(day);
	set_date_month(month);
	set_date_year(year);
}

void pcf8583_device::nvram_default()
{
	std::fill(std::begin(m_data), std::end(m_data), 0);
}

void pcf8583_device::nvram_read(emu_file &file)
{
	file.read(m_data, sizeof(m_data));
}

void pcf8583_device::nvram_write(emu_file &file)
{
	file.write(m_data, sizeof(m_data));
}

void pcf8583_device::write_register(uint8_t offset, uint8_t data)
{
	logerror("%s: write_register: address %02x = %02x\n", machine().describe_context(), offset, data);
	m_data[offset] = data;
}

WRITE_LINE_MEMBER(pcf8583_device::scl_w)
{
	if (m_transfer_active && !m_scl && state)
	{
		switch (m_mode)
		{
			case RTC_MODE_RECV:
			{
				logerror("%s: scl_w: Receiving bit %d in receive mode\n", machine().describe_context(), m_sda ? 1 : 0);
				if (m_sda)
					m_data_recv |= (0x80 >> m_bit_index);
				m_bit_index++;

				if (m_bit_index > 8) // ignore ACK bit
				{
					if (m_data_recv_index == 0)
					{
						if (m_data_recv == m_read_address)
						{
							logerror("%s: scl_w: Received byte 0 (%02x), matches read address, entering read/send mode\n", machine().describe_context(), m_data_recv);
							m_mode = RTC_MODE_SEND;
						}
						else if (m_data_recv == m_write_address)
						{
							logerror("%s: scl_w: Received byte 0 (%02x), matches write address, entering read/send mode\n", machine().describe_context(), m_data_recv);
							m_mode = RTC_MODE_RECV;
						}
						else
						{
							logerror("%s: scl_w: Received byte 0 (%02x), unknown address, going idle\n", machine().describe_context(), m_data_recv);
							m_mode = RTC_MODE_NONE;
						}
					}
					else if (m_data_recv_index == 1)
					{
						logerror("%s: scl_w: Received byte 1 (%02x), setting current read/write pos\n", machine().describe_context(), m_data_recv);
						m_pos = m_data_recv;
					}
					else if (m_data_recv_index >= 2)
					{
						logerror("%s: scl_w: Received byte 2+ (%d: %02x), storing to memory\n", machine().describe_context(), m_data_recv_index, m_data_recv);
						write_register(m_pos, m_data_recv);
						m_pos++;
					}

					m_bit_index = 0;
					m_data_recv = 0;
					m_data_recv_index++;
				}
			}
			break;

			case RTC_MODE_SEND:
			{
				if (m_bit_index < 8)
				{
					m_inp = BIT(m_data[m_pos], 7 - m_bit_index);
					logerror("%s: scl_w: In send mode, reading bit %d from ram[0x%02x]=%02x (%d)\n", machine().describe_context(), m_bit_index, m_pos, m_data[m_pos], m_inp);
				}
				m_bit_index++;

				if (m_bit_index > 8)
				{
					m_bit_index = 0;
					m_pos++;
				}
			}
			break;
		}
	}

	m_scl = state;
}

WRITE_LINE_MEMBER(pcf8583_device::sda_w)
{
	if (m_scl)
	{
		if (!state && m_sda)
		{
			// start condition (high to low when clock is high)
			m_transfer_active = true;
			m_bit_index = 0;
			m_data_recv_index = 0;
			clear_rx_buffer();
		}
		else if (state && !m_sda)
		{
			// stop condition (low to high when clock is high)
			m_transfer_active = false;
		}
	}

	m_sda = state;
}

READ_LINE_MEMBER(pcf8583_device::sda_r)
{
	return m_inp;
}

void pcf8583_device::clear_rx_buffer()
{
	m_data_recv = 0;
	m_data_recv_index = 0;
}

void pcf8583_device::set_a0(uint8_t a0)
{
	m_read_address = READ_ADDRESS_BASE | (a0 << 1);
	m_write_address = WRITE_ADDRESS_BASE | (a0 << 1);
}
