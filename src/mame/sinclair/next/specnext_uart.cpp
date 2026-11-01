// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next UART
**********************************************************************/

#include "emu.h"
#include "specnext_uart.h"


#define LOG_DAT_OUT (1U << 1)
#define LOG_DAT_IN  (1U << 2)
#define LOG_RX_OVR  (1U << 3)

//#define VERBOSE ( LOG_DAT_OUT | LOG_DAT_IN | LOG_RX_OVR | LOG_GENERAL )
#include "logmacro.h"

#define LOGDOUT(...) LOGMASKED(LOG_DAT_OUT, __VA_ARGS__)
#define LOGDIN(...)  LOGMASKED(LOG_DAT_IN,  __VA_ARGS__)
#define LOGOVR(...)  LOGMASKED(LOG_RX_OVR,  __VA_ARGS__)


DEFINE_DEVICE_TYPE(SPECNEXT_UART, specnext_uart_device, "specnext_uart", "Spectrum Next UART");

specnext_uart_device::specnext_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_UART, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, m_out_txd_cb(*this)
	, m_out_rx_full_near_cb(*this)
	, m_out_tx_empty_cb(*this)
	, m_out_rts_cb(*this)
	, m_rx_head(0), m_rx_tail(0)
	, m_rx_empty(true)
	, m_rx_full_near(false)
	, m_rx_hold(0)
	, m_rx_hold_valid(false)
	, m_rx_overflow(false)
	, m_cts_n(false)
	, m_tx_bit_sent(false)
{
}

void specnext_uart_device::tra_callback()
{
	// frame register can hold Tx at the start bit (flow control, break, sticky reset)
	if (tx_blocked() && !m_tx_bit_sent)
	{
		if (BIT(m_framing, 6))
			m_out_txd_cb(0);
		return;
	}

	m_tx_bit_sent = true;
	m_out_txd_cb(transmit_register_get_data_bit());
	if (is_transmit_register_empty())
		m_tx_bit_sent = false;
}

void specnext_uart_device::tra_complete()
{
	if (!tx_blocked())
	{
		device_buffered_serial_interface<64>::tra_complete();
		if (!is_transmit_register_empty())
			m_tx_bit_sent = false;
	}

	txd_idle_w();
}

void specnext_uart_device::received_byte(u8 byte)
{
	if (BIT(m_framing, 7))
		return;

	if (m_rx_hold_valid)
	{
		m_rx_overflow = true;
		LOGOVR("RX overflow, dropped %02x\n", byte);
		return;
	}

	m_rx_hold = (m_rx_overflow << 8) | byte;
	m_rx_hold_valid = true;
	rx_fifo_push();
}

void specnext_uart_device::rx_fifo_push()
{
	if (!m_rx_hold_valid || (!m_rx_empty && (m_rx_head == m_rx_tail)))
		return;

	m_rx_fifo[m_rx_tail] = m_rx_hold;
	m_rx_tail = (m_rx_tail + 1) % RX_FIFO_SIZE;
	m_rx_empty = false;
	m_rx_hold_valid = false;

	if (!m_rx_full_near)
	{
		m_rx_full_near = ((m_rx_tail + RX_FIFO_SIZE - m_rx_head) % RX_FIFO_SIZE) >= (RX_FIFO_SIZE * 3 / 4);
		if (m_rx_full_near)
			m_out_rx_full_near_cb(1);
	}

	update_rts();
}

u8 specnext_uart_device::dat_r()
{
	u8 data = 0x00;

	if (!m_rx_empty)
	{
		data = m_rx_fifo[m_rx_head] & 0xff;
		if (!machine().side_effects_disabled())
		{
			m_rx_head = (m_rx_head + 1) % RX_FIFO_SIZE;
			m_rx_empty = (m_rx_head == m_rx_tail) ? true : false;
			m_rx_full_near = ((m_rx_tail + RX_FIFO_SIZE - m_rx_head) % RX_FIFO_SIZE) >= (RX_FIFO_SIZE * 3 / 4);
			rx_fifo_push();
			update_rts();
		}
		LOGDIN("DR = %02x(%c)\n", data, data >= 0x20 ? data : '?');
	}

	return data;
}

u8 specnext_uart_device::status_reg_r()
{
	// TODO:
	//o_cpu_d <= uart0_status_rx_err_break & uart0_status_rx_err_framing & (uart0_rx_o(8) and uart0_status_rx_avail) & uart0_status_tx_empty &
	//          uart0_status_rx_near_full & uart0_status_rx_err_overflow & uart0_status_tx_full & uart0_status_rx_avail;

	const bool rx_err = !m_rx_empty && BIT(m_rx_fifo[m_rx_head], 8);
	const bool tx_empty = fifo_empty() && is_transmit_register_empty() && !tx_blocked();
	const u8 status = (rx_err << 5)
			| (tx_empty << 4)
			| (m_rx_full_near << 3)
			| (m_rx_overflow << 2)
			| (fifo_full() << 1)
			| (!m_rx_empty << 0);

	if (!machine().side_effects_disabled())
		m_rx_overflow = false;

	return status;
}

u8 specnext_uart_device::reg_r(offs_t reg)
{
	switch (reg)
	{
		case 0b00: return dat_r();
		case 0b01: return m_prescalar_msb;
		case 0b10: return m_framing;
		case 0b11: default:
			return status_reg_r();
	}
}

void specnext_uart_device::reg_w(offs_t reg, u8 data)
{
	switch (reg)
	{
		case 0b00:
			m_prescalar_lsb = BIT(data, 7)
				? ((m_prescalar_lsb & 0x007f) | ((data & 0x7f) << 7))
				: ((m_prescalar_lsb & 0x3f80) | (data & 0x7f));
			update_serial();
			tx_resume();
			break;

		case 0b01:
			if (BIT(data, 4))
			{
				m_prescalar_msb = data & 0x07;
				update_serial();
				tx_resume();
			}
			break;

		case 0b10:
			if (BIT(data, 7))
			{
				// sticky reset: Tx and Rx modules to idle, fifos empty
				clear_fifo();
				clear_rx_fifo();
				transmit_register_reset();
				receive_register_reset();
				m_tx_bit_sent = false;
			}
			m_framing = data;
			update_serial();
			update_rts();
			tx_resume();
			txd_idle_w();
			break;

		case 0b11:
		default:
			if (BIT(m_framing, 7))
				return;

			{
				const bool tra_was_empty = is_transmit_register_empty();
				transmit_byte(data);
				if (tra_was_empty && !is_transmit_register_empty())
					m_tx_bit_sent = false;
			}
			tx_resume();
			LOGDOUT("DW: %02x(%c)\n", data, data >= 0x20 ? data : '?');
			break;
	}

}

void specnext_uart_device::clear_rx_fifo()
{
	m_rx_head = m_rx_tail = 0;
	m_rx_empty = true;
	m_rx_full_near = false;
	m_rx_hold_valid = false;
	m_rx_overflow = false;
	update_rts();
}

void specnext_uart_device::update_serial()
{
	const u8 bits = 0b100 | BIT(m_framing, 3, 2);
	const parity_t parity = BIT(m_framing, 2) ? (BIT(m_framing, 1) ? PARITY_ODD : PARITY_EVEN) : PARITY_NONE;
	set_data_frame(1, bits + 1, parity, BIT(m_framing, 0) ? STOP_BITS_2 : STOP_BITS_1);

	const int rate = clock() / ((m_prescalar_msb << 14) | m_prescalar_lsb);
	set_tra_rate(rate);
	set_rcv_rate(rate);
}

bool specnext_uart_device::tx_blocked() const
{
	return BIT(m_framing, 7) || BIT(m_framing, 6) || (BIT(m_framing, 5) && m_cts_n);
}

void specnext_uart_device::tx_resume()
{
	if (!tx_blocked() && is_transmit_register_empty() && !fifo_empty())
		tra_complete();
}

void specnext_uart_device::txd_idle_w()
{
	if (is_transmit_register_empty())
		m_out_txd_cb(BIT(m_framing, 6) ? 0 : 1);
}

void specnext_uart_device::update_rts()
{
	// rtr_n: 0 = ready to receive; deasserts near full only with flow control enabled
	const bool flow = BIT(m_framing, 5);
	const bool near_full = BIT(m_framing, 7) || (((m_rx_tail + RX_FIFO_SIZE - m_rx_head) % RX_FIFO_SIZE) >= RX_FIFO_SIZE - 2);
	m_out_rts_cb(flow && near_full);
}

void specnext_uart_device::cts_w(int state)
{
	m_cts_n = state;
	tx_resume();
}

void specnext_uart_device::device_start()
{
	receive_register_reset();
	transmit_register_reset();

	m_prescalar_msb = 0b000;
	m_prescalar_lsb = 0b00000011110011;
	m_framing = 0x18;
	clear_fifo();
	clear_rx_fifo();

	update_serial();
	m_out_txd_cb(1);

	save_item(NAME(m_prescalar_msb));
	save_item(NAME(m_prescalar_lsb));
	save_item(NAME(m_framing));
	save_item(NAME(m_rx_fifo));
	save_item(NAME(m_rx_head));
	save_item(NAME(m_rx_tail));
	save_item(NAME(m_rx_empty));
	save_item(NAME(m_rx_full_near));
	save_item(NAME(m_rx_hold));
	save_item(NAME(m_rx_hold_valid));
	save_item(NAME(m_rx_overflow));
	save_item(NAME(m_cts_n));
	save_item(NAME(m_tx_bit_sent));
}

void specnext_uart_device::device_reset()
{
	clear_fifo();
	clear_rx_fifo();
	transmit_register_reset();
	receive_register_reset();
	m_tx_bit_sent = false;
	txd_idle_w();
}
