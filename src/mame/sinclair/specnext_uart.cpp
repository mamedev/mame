// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next UART
**********************************************************************/

#include "emu.h"
#include "specnext_uart.h"


#define LOG_DAT_OUT (1U << 1)
#define LOG_DAT_IN  (1U << 2)

//#define VERBOSE ( LOG_DAT_OUT | LOG_DAT_IN | LOG_GENERAL )
#include "logmacro.h"

#define LOGDOUT(...) LOGMASKED(LOG_DAT_OUT, __VA_ARGS__)
#define LOGDIN(...)  LOGMASKED(LOG_DAT_IN,  __VA_ARGS__)


DEFINE_DEVICE_TYPE(SPECNEXT_UART, specnext_uart_device, "specnext_uart", "Spectrum Next UART")

specnext_uart_device::specnext_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_UART, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, m_out_txd_cb(*this)
	, m_out_rx_full_near_cb(*this)
	, m_out_tx_empty_cb(*this)
{
}


void specnext_uart_device::tra_callback()
{
	const int txd = transmit_register_get_data_bit();
	m_out_txd_cb(txd);
}

void specnext_uart_device::received_byte(u8 byte)
{
	m_rx_fifo[m_rx_tail] = byte;
	m_rx_tail = (m_rx_tail + 1) % RX_FIFO_SIZE;
	m_rx_empty = false;

	if (!m_rx_full_near)
	{
		m_rx_full_near = ((m_rx_tail + RX_FIFO_SIZE - m_rx_head) % RX_FIFO_SIZE) >= (RX_FIFO_SIZE * 3 / 4);
		if (m_rx_full_near)
			m_out_rx_full_near_cb(1);
	}
}

u8 specnext_uart_device::dat_r()
{
	u8 data = 0x00;

	if (!m_rx_empty)
	{
		data = m_rx_fifo[m_rx_head];
		if (!machine().side_effects_disabled())
		{
			m_rx_head = (m_rx_head + 1) % RX_FIFO_SIZE;
			m_rx_empty = (m_rx_head == m_rx_tail) ? true : false;
			m_rx_full_near = ((m_rx_tail + RX_FIFO_SIZE - m_rx_head) % RX_FIFO_SIZE) >= (RX_FIFO_SIZE * 3 / 4);
		}
		LOGDIN("DR = %02x(%c)\n", data, data >= 0x20 ? data : '?');
	}

	return data;
}

u8 specnext_uart_device::status_reg_r()
{
	// TODO:
	//o_cpu_d <= uart0_status_rx_err_break & uart0_status_rx_err_framing & (uart0_rx_o(8) and uart0_status_rx_avail) & uart0_status_tx_empty &
	//			uart0_status_rx_near_full & uart0_status_rx_err_overflow & uart0_status_tx_full & uart0_status_rx_avail;

	const bool rx_overflow = !m_rx_empty && (m_rx_head == m_rx_tail);
	const u8 status = (fifo_empty() << 4)
			| (m_rx_full_near << 3)
			| (rx_overflow << 2)
			| (fifo_full() << 1)
			| (!m_rx_empty << 0);

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
			break;

		case 0b01:
			if (BIT(data, 4))
			{
				m_prescalar_msb = data & 0x07;
				update_serial();
			}
			break;

		case 0b10:
			if (BIT(data, 7))
			{
				clear_fifo();
				clear_rx_fifo();
			}
			m_framing = data;
			update_serial();
			break;

		case 0b11:
		default:
			transmit_byte(data);
			LOGDOUT("DW: %02x(%c)\n", data, data >= 0x20 ? data : '?');
			break;
	}

}

void specnext_uart_device::clear_rx_fifo()
{
	m_rx_head = m_rx_tail = 0;
	m_rx_empty = true;
	m_rx_full_near = false;
}

void specnext_uart_device::update_serial()
{
	const u8 bits = 0b100 | BIT(m_framing, 3, 2);
	const parity_t parity = BIT(m_framing, 2) ? (BIT(m_framing, 1) ? PARITY_ODD : PARITY_EVEN) : PARITY_NONE;
	set_data_frame(1, bits + 1, parity, STOP_BITS_1);

	const int rate = clock() / ((m_prescalar_msb << 14) | m_prescalar_lsb);
	set_tra_rate(rate);
	set_rcv_rate(rate);
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
}
