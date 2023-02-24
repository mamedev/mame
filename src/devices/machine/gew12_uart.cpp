// license:BSD-3-Clause
// copyright-holders:Devin Acker
/*********************************************************************

	gew12_uart.cpp

	Yamaha GEW12 UART

*********************************************************************/

#include "emu.h"
#include "gew12_uart.h"

DEFINE_DEVICE_TYPE(GEW12_UART, gew12_uart_device, "gew12_uart", "Yamaha GEW12 UART")

gew12_uart_device::gew12_uart_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: device_t(mconfig, GEW12_UART, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_tx_handler(*this)
	, m_tx_irq_handler(*this)
	, m_rx_irq_handler(*this)
{
}

void gew12_uart_device::device_start()
{
	m_tx_handler.resolve_safe();
	m_tx_irq_handler.resolve_safe();
	m_rx_irq_handler.resolve_safe();

	const attotime rate = clocks_to_attotime(448); // 14 MHz -> 31250 bps
	m_timer = timer_alloc(FUNC(gew12_uart_device::timer_tick), this);
	m_timer->adjust(rate, 0, rate);

	save_item(NAME(m_data_out));
	save_item(NAME(m_data_in));
	save_item(NAME(m_status));
	save_item(NAME(m_rx));
}

void gew12_uart_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	transmit_register_reset();
	receive_register_reset();

	m_data_out = m_data_in = 0;
	m_rx = 1;
	m_status = STATUS_TX_READY;
}

TIMER_CALLBACK_MEMBER(gew12_uart_device::timer_tick)
{
	receive_register_update_bit(m_rx);

	if (is_receive_register_full())
	{
		receive_register_extract();
		m_data_in = get_received_char();

		if (is_receive_framing_error())
		{
			m_status |= STATUS_FRAMING_ERROR;
			logerror("MIDI RX framing error\n");
		}

		if (m_status & STATUS_RX_READY)
		{
			m_status |= STATUS_OVERRUN_ERROR;
			logerror("MIDI RX overrun error\n");
		}
		else
		{
			m_status |= STATUS_RX_READY;
		}
	}

	if (!(m_status & STATUS_TX_READY) && is_transmit_register_empty())
	{
		transmit_register_setup(m_data_out);
		m_status |= STATUS_TX_READY;
	}

	if (!is_transmit_register_empty())
	{
		m_tx_handler(transmit_register_get_data_bit());
	}

	irq_update();
}

void gew12_uart_device::irq_update()
{
	m_rx_irq_handler((m_status & STATUS_RX_BITS) ? ASSERT_LINE : CLEAR_LINE);
	m_tx_irq_handler((m_status & STATUS_TX_READY) ? ASSERT_LINE : CLEAR_LINE);
}

u8 gew12_uart_device::status_r()
{
	if (!machine().side_effects_disabled())
	{
		m_status &= ~STATUS_ERROR_BITS;
		irq_update();
	}

	return m_status;
}

u8 gew12_uart_device::data_r()
{
	if (!machine().side_effects_disabled())
	{
		m_status &= ~STATUS_RX_READY;
		irq_update();
	}

	return m_data_in;
}

void gew12_uart_device::data_w(u8 data)
{
	m_data_out = data;
	m_status &= ~STATUS_TX_READY;
	irq_update();
}

WRITE_LINE_MEMBER(gew12_uart_device::rx_w)
{
	m_rx = state;
}
