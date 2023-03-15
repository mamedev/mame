// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Intel XScale SA1110 peripheral emulation

***************************************************************************/

#include "emu.h"
#include "sa1110.h"

#define LOG_UNKNOWN     (1 << 1)
#define LOG_ICP         (1 << 2)
#define LOG_UART3       (1 << 3)
#define LOG_UART3_HF    (1 << 4)
#define LOG_MCP         (1 << 5)
#define LOG_SSP         (1 << 6)
#define LOG_OSTIMER     (1 << 7)
#define LOG_OSTIMER_HF  (1 << 8)
#define LOG_RTC         (1 << 9)
#define LOG_RTC_HF      (1 << 10)
#define LOG_POWER       (1 << 11)
#define LOG_POWER_HF    (1 << 12)
#define LOG_RESET       (1 << 13)
#define LOG_GPIO        (1 << 14)
#define LOG_GPIO_HF     (1 << 15)
#define LOG_INTC        (1 << 16)
#define LOG_PPC         (1 << 17)
#define LOG_DMA         (1 << 18)
#define LOG_UDC         (1 << 19)
#define LOG_ALL         (LOG_UNKNOWN | LOG_ICP | LOG_UART3 | LOG_MCP | LOG_OSTIMER | LOG_RTC | LOG_POWER | LOG_RESET | LOG_GPIO | LOG_INTC | LOG_PPC | LOG_DMA | LOG_UDC)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SA1110_PERIPHERALS, sa1110_periphs_device, "sa1110_periphs", "Intel XScale SA1110 Peripherals")

sa1110_periphs_device::sa1110_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SA1110_PERIPHERALS, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_uart3_irqs(*this, "uart3irq")
	, m_mcp_irqs(*this, "mcpirq")
	, m_codec(*this, finder_base::DUMMY_TAG)
	, m_gpio_out(*this)
	, m_ssp_out(*this)
	, m_uart3_tx_out(*this)
{
}

/*

  Intel SA-1110 UDC - USB Device Controller

  pg. 235 to 258 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

uint32_t sa1110_periphs_device::udc_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_UDCCR:
		LOGMASKED(LOG_UDC, "%s: udc_r: UDC Control Register: %08x & %08x\n", machine().describe_context(), m_udc_regs.udccr, mem_mask);
		return m_udc_regs.udccr;
	case REG_UDCAR:
		LOGMASKED(LOG_UDC, "%s: udc_r: UDC Address Register: %08x & %08x\n", machine().describe_context(), m_udc_regs.udcar, mem_mask);
		return m_udc_regs.udcar;
	case REG_UDCOMP:
		LOGMASKED(LOG_UDC, "%s: udc_r: UDC OUT Max Packet Register: %08x & %08x\n", machine().describe_context(), m_udc_regs.udcomp, mem_mask);
		return m_udc_regs.udcomp;
	case REG_UDCIMP:
		LOGMASKED(LOG_UDC, "%s: udc_r: UDC IN Max Packet Register: %08x & %08x\n", machine().describe_context(), m_udc_regs.udcimp, mem_mask);
		return m_udc_regs.udcimp;
	case REG_UDCCS0:
		LOGMASKED(LOG_UDC, "%s: udc_r: UDC Endpoint 0 Control/Status Register: %08x & %08x\n", machine().describe_context(), m_udc_regs.udccs0, mem_mask);
		return m_udc_regs.udccs0;
	case REG_UDCCS1:
		LOGMASKED(LOG_UDC, "%s: udc_r: UDC Endpoint 1 (OUT) Control/Status Register: %08x & %08x\n", machine().describe_context(), m_udc_regs.udccs1, mem_mask);
		return m_udc_regs.udccs1;
	case REG_UDCCS2:
		LOGMASKED(LOG_UDC, "%s: udc_r: UDC Endpoint 2 (IN) Control/Status Register: %08x & %08x\n", machine().describe_context(), m_udc_regs.udccs2, mem_mask);
		return m_udc_regs.udccs2;
	case REG_UDCD0:
		LOGMASKED(LOG_UDC, "%s: udc_r: UDC Endpoint 0 Data Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case REG_UDCWC:
		LOGMASKED(LOG_UDC, "%s: udc_r: UDC Endpoint 0 Write Count Register: %08x & %08x\n", machine().describe_context(), m_udc_regs.udcwc, mem_mask);
		return m_udc_regs.udcwc;
	case REG_UDCDR:
		//const uint32_t data = udc_rx_fifo_pop();
		LOGMASKED(LOG_UDC, "%s: udc_r: UDC Data Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case REG_UDCSR:
		LOGMASKED(LOG_UDC, "%s: udc_r: UDC Status/Interrupt Register: %08x & %08x\n", machine().describe_context(), m_udc_regs.udcsr, mem_mask);
		return m_udc_regs.udcsr;
	default:
		LOGMASKED(LOG_UDC | LOG_UNKNOWN, "%s: udc_r: Unknown address: %08x & %08x\n", machine().describe_context(), UDC_BASE_ADDR | (offset << 2), mem_mask);
		return 0;
	}
}

void sa1110_periphs_device::udc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_UDCCR:
		LOGMASKED(LOG_UDC, "%s: udc_w: UDC Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_udc_regs.udccr);
		break;
	case REG_UDCAR:
		LOGMASKED(LOG_UDC, "%s: udc_w: UDC Address Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_udc_regs.udcar);
		break;
	case REG_UDCOMP:
		LOGMASKED(LOG_UDC, "%s: udc_w: UDC OUT Max Packet Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_udc_regs.udcomp);
		break;
	case REG_UDCIMP:
		LOGMASKED(LOG_UDC, "%s: udc_w: UDC IN Max Packet Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_udc_regs.udcimp);
		break;
	case REG_UDCCS0:
		LOGMASKED(LOG_UDC, "%s: udc_w: UDC Endpoint 0 Control/Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_udc_regs.udccs0);
		break;
	case REG_UDCCS1:
		LOGMASKED(LOG_UDC, "%s: udc_w: UDC Endpoint 1 (OUT) Control/Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_udc_regs.udccs1);
		break;
	case REG_UDCCS2:
		LOGMASKED(LOG_UDC, "%s: udc_w: UDC Endpoint 2 (IN) Control/Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_udc_regs.udccs2);
		break;
	case REG_UDCD0:
		LOGMASKED(LOG_UDC, "%s: udc_w: UDC Endpoint 0 Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_UDCWC:
		LOGMASKED(LOG_UDC, "%s: udc_w: UDC Endpoint 0 Write Count Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_udc_regs.udcwc);
		break;
	case REG_UDCDR:
		LOGMASKED(LOG_UDC, "%s: udc_w: UDC Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		return;
	case REG_UDCSR:
		LOGMASKED(LOG_UDC, "%s: udc_w: UDC Status/Interrupt Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	default:
		LOGMASKED(LOG_UDC | LOG_UNKNOWN, "%s: udc_w: Unknown address: %08x = %08x & %08x\n", machine().describe_context(), UDC_BASE_ADDR | (offset << 2), data, mem_mask);
		break;
	}
}

/*

  Intel SA-1110 ICP - Serial Port 2

  pg. 264 to 288 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::icp_rx_callback)
{
}

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::icp_tx_callback)
{
}

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::hssp_rx_callback)
{
}

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::hssp_tx_callback)
{
}

void sa1110_periphs_device::icp_uart_set_receiver_enabled(bool enabled)
{
}

void sa1110_periphs_device::icp_uart_set_transmitter_enabled(bool enabled)
{
}

void sa1110_periphs_device::icp_uart_set_receive_irq_enabled(bool enabled)
{
}

void sa1110_periphs_device::icp_uart_set_transmit_irq_enabled(bool enabled)
{
}

uint8_t sa1110_periphs_device::icp_uart_read_receive_fifo()
{
	return 0;
}

void sa1110_periphs_device::icp_uart_write_transmit_fifo(uint8_t data)
{
}

uint16_t sa1110_periphs_device::icp_hssp_read_receive_fifo()
{
	return 0;
}

void sa1110_periphs_device::icp_hssp_write_transmit_fifo(uint8_t data)
{
}

void sa1110_periphs_device::icp_uart_set_receiver_idle()
{
}

void sa1110_periphs_device::icp_uart_begin_of_break()
{
}

void sa1110_periphs_device::icp_uart_end_of_break()
{
}


uint32_t sa1110_periphs_device::icp_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_UTCR0:
		LOGMASKED(LOG_ICP, "%s: icp_r: UART Control Register 0: %08x & %08x\n", machine().describe_context(), m_icp_regs.uart.utcr[0], mem_mask);
		return m_icp_regs.uart.utcr[0];
	case REG_UTCR1:
		LOGMASKED(LOG_ICP, "%s: icp_r: UART Control Register 1: %08x & %08x\n", machine().describe_context(), m_icp_regs.uart.utcr[1], mem_mask);
		return m_icp_regs.uart.utcr[1];
	case REG_UTCR2:
		LOGMASKED(LOG_ICP, "%s: icp_r: UART Control Register 2: %08x & %08x\n", machine().describe_context(), m_icp_regs.uart.utcr[2], mem_mask);
		return m_icp_regs.uart.utcr[2];
	case REG_UTCR3:
		LOGMASKED(LOG_ICP, "%s: icp_r: UART Control Register 3: %08x & %08x\n", machine().describe_context(), m_icp_regs.uart.utcr[3], mem_mask);
		return m_icp_regs.uart.utcr[3];
	case REG_UTCR4:
		LOGMASKED(LOG_ICP, "%s: icp_r: UART Control Register 4: %08x & %08x\n", machine().describe_context(), m_icp_regs.utcr4, mem_mask);
		return m_icp_regs.utcr4;
	case REG_UTDR:
	{
		const uint8_t data = icp_uart_read_receive_fifo();
		LOGMASKED(LOG_ICP, "%s: icp_r: UART Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case REG_UTSR0:
		LOGMASKED(LOG_ICP, "%s: icp_r: UART Status Register 0: %08x & %08x\n", machine().describe_context(), m_icp_regs.uart.utsr0, mem_mask);
		return m_icp_regs.uart.utsr0;
	case REG_UTSR1:
		LOGMASKED(LOG_ICP, "%s: icp_r: UART Status Register 1: %08x & %08x\n", machine().describe_context(), m_icp_regs.uart.utsr1, mem_mask);
		return m_icp_regs.uart.utsr1;
	case REG_HSCR0:
		LOGMASKED(LOG_ICP, "%s: icp_r: HSSP Control Register 0: %08x & %08x\n", machine().describe_context(), m_icp_regs.hssp.hscr0, mem_mask);
		return m_icp_regs.hssp.hscr0;
	case REG_HSCR1:
		LOGMASKED(LOG_ICP, "%s: icp_r: HSSP Control Register 1: %08x & %08x\n", machine().describe_context(), m_icp_regs.hssp.hscr1, mem_mask);
		return m_icp_regs.hssp.hscr1;
	case REG_HSDR:
	{
		const uint16_t data = icp_hssp_read_receive_fifo();
		LOGMASKED(LOG_ICP, "%s: icp_r: HSSP Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case REG_HSSR0:
		LOGMASKED(LOG_ICP, "%s: icp_r: HSSP Status Register 0: %08x & %08x\n", machine().describe_context(), m_icp_regs.hssp.hssr0, mem_mask);
		return m_icp_regs.hssp.hssr0;
	case REG_HSSR1:
		LOGMASKED(LOG_ICP, "%s: icp_r: HSSP Status Register 1: %08x & %08x\n", machine().describe_context(), m_icp_regs.hssp.hssr1, mem_mask);
		return m_icp_regs.hssp.hssr1;
	default:
		LOGMASKED(LOG_ICP | LOG_UNKNOWN, "%s: icp_r: Unknown address: %08x & %08x\n", machine().describe_context(), ICP_BASE_ADDR | (offset << 2), mem_mask);
		return 0;
	}
}

void sa1110_periphs_device::icp_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_UTCR0:
	{
		LOGMASKED(LOG_ICP, "%s: icp_w: UART Control Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_ICP, "%s:        Parity Enable: %d\n", machine().describe_context(), BIT(data, 0));
		LOGMASKED(LOG_ICP, "%s:        Parity Mode: %s\n", machine().describe_context(), BIT(data, 1) ? "Even" : "Odd");
		LOGMASKED(LOG_ICP, "%s:        Stop Bits: %d\n", machine().describe_context(), BIT(data, 2) + 1);
		LOGMASKED(LOG_ICP, "%s:        Data Size: %d\n", machine().describe_context(), BIT(data, 3) ? 8 : 7);
		LOGMASKED(LOG_ICP, "%s:        Sample Clock: %s\n", machine().describe_context(), BIT(data, 4) ? "External" : "Internal");
		LOGMASKED(LOG_ICP, "%s:        Receive Edge: %s\n", machine().describe_context(), BIT(data, 5) ? "Falling" : "Rising");
		LOGMASKED(LOG_ICP, "%s:        Transmit Edge: %s\n", machine().describe_context(), BIT(data, 6) ? "Falling" : "Rising");

		//stop_bits_t stop_bits = (BIT(data, 2) ? STOP_BITS_2 : STOP_BITS_1);

		//parity_t parity = PARITY_NONE;
		//if (BIT(data, 0))
		//{
		//  parity = (BIT(data, 1) ? PARITY_EVEN : PARITY_ODD);
		//}

		//set_data_frame(1, BIT(data, 3) ? 8 : 7, parity, stop_bits);
		//receive_register_reset();
		//transmit_register_reset();

		COMBINE_DATA(&m_icp_regs.uart.utcr[0]);
		break;
	}
	case REG_UTCR1:
	{
		LOGMASKED(LOG_ICP, "%s: icp_w: UART Control Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_ICP, "%s:        Baud Rate Divisor MSB: %02x\n", machine().describe_context(), data & 0x0f);
		//const uint8_t old = m_uart_regs.utcr[1] & 0x0f;
		COMBINE_DATA(&m_icp_regs.uart.utcr[1]);
		//if ((m_uart_regs.utcr[1] & 0x0f) != old)
		//  icp_uart_recalculate_divisor();
		break;
	}
	case REG_UTCR2:
	{
		LOGMASKED(LOG_ICP, "%s: icp_w: UART Control Register 2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_ICP, "%s:        Baud Rate Divisor LSB: %02x\n", machine().describe_context(), (uint8_t)data);
		//const uint8_t old = m_uart_regs.utcr[2] & 0xff;
		COMBINE_DATA(&m_icp_regs.uart.utcr[2]);
		//if ((m_uart_regs.utcr[2] & 0xff) != old)
		//  icp_uart_recalculate_divisor();
		break;
	}
	case REG_UTCR3:
	{
		LOGMASKED(LOG_ICP, "%s: icp_w: UART Control Register 3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_ICP, "%s:        Receive Enable: %d\n", machine().describe_context(), BIT(data, 0));
		LOGMASKED(LOG_ICP, "%s:        Transmit Enable: %d\n", machine().describe_context(), BIT(data, 1));
		LOGMASKED(LOG_ICP, "%s:        Send Break: %d\n", machine().describe_context(), BIT(data, 2));
		LOGMASKED(LOG_ICP, "%s:        Receive FIFO IRQ Enable: %d\n", machine().describe_context(), BIT(data, 3));
		LOGMASKED(LOG_ICP, "%s:        Transmit FIFO IRQ Enable: %d\n", machine().describe_context(), BIT(data, 4));
		LOGMASKED(LOG_ICP, "%s:        Loopback Enable: %d\n", machine().describe_context(), BIT(data, 5));
		const uint32_t old = m_icp_regs.uart.utcr[3];
		COMBINE_DATA(&m_icp_regs.uart.utcr[3]);
		const uint32_t changed = old ^ m_icp_regs.uart.utcr[3];
		if (BIT(changed, 0))
			icp_uart_set_receiver_enabled(BIT(data, 0));
		if (BIT(changed, 1))
			icp_uart_set_transmitter_enabled(BIT(data, 1));
		if (BIT(changed, 3))
			icp_uart_set_receive_irq_enabled(BIT(data, 3));
		if (BIT(changed, 4))
			icp_uart_set_transmit_irq_enabled(BIT(data, 4));
		break;
	}
	case REG_UTCR4:
		LOGMASKED(LOG_ICP, "%s: icp_w: UART Control Register 4 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_ICP, "%s:        HP-SIR enable: %d\n", machine().describe_context(), BIT(data, UTCR4_HSE_BIT), mem_mask);
		LOGMASKED(LOG_ICP, "%s:        Low-Power enable: %d\n", machine().describe_context(), BIT(data, UTCR4_LPM_BIT), mem_mask);
		COMBINE_DATA(&m_icp_regs.utcr4);
		break;
	case REG_UTDR:
		LOGMASKED(LOG_ICP, "%s: icp_w: UART Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (data == 0x0d || data == 0x0a || (data >= 0x20 && data < 0x7f))
		{
			printf("%c", (char)data);
		}
		icp_uart_write_transmit_fifo((uint8_t)data);
		break;
	case REG_UTSR0:
		LOGMASKED(LOG_ICP, "%s: icp_w: UART Status Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_ICP, "%s:        Receiver Idle Status: %d\n", machine().describe_context(), BIT(data, 2));
		LOGMASKED(LOG_ICP, "%s:        Receiver Begin of Break Status: %d\n", machine().describe_context(), BIT(data, 3));
		LOGMASKED(LOG_ICP, "%s:        Receiver End of Break Status: %d\n", machine().describe_context(), BIT(data, 4));
		if (BIT(data, 2))
			icp_uart_set_receiver_idle();
		if (BIT(data, 3))
			icp_uart_begin_of_break();
		if (BIT(data, 4))
			icp_uart_end_of_break();
		break;
	case REG_HSCR0:
		LOGMASKED(LOG_ICP, "%s: icp_w: HSSP Control Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_HSCR1:
		LOGMASKED(LOG_ICP, "%s: icp_w: HSSP Control Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_HSDR:
		LOGMASKED(LOG_ICP, "%s: icp_w: HSSP Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		icp_hssp_write_transmit_fifo((uint8_t)data);
		break;
	case REG_HSSR0:
		LOGMASKED(LOG_ICP, "%s: icp_w: HSSP Status Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_HSSR1:
		LOGMASKED(LOG_ICP, "%s: icp_w: HSSP Status Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	default:
		LOGMASKED(LOG_ICP | LOG_UNKNOWN, "%s: icp_w: Unknown address: %08x = %08x & %08x\n", machine().describe_context(), ICP_BASE_ADDR | (offset << 2), data, mem_mask);
		break;
	}
}

/*

  Intel SA-1110 Serial Port 3 - UART

  pg. 289 to 306 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

WRITE_LINE_MEMBER(sa1110_periphs_device::uart3_irq_callback)
{
	set_irq_line(INT_UART3, state);
}

// Rx completed receiving byte
void sa1110_periphs_device::rcv_complete()
{
	receive_register_extract();

	uint16_t data_and_flags = 0;
	if (is_receive_framing_error())
		data_and_flags |= 0x200;
	if (is_receive_parity_error())
		data_and_flags |= 0x100;
	data_and_flags |= get_received_char();

	uart_write_receive_fifo(data_and_flags);
}

// Tx completed sending byte
void sa1110_periphs_device::tra_complete()
{
	m_uart_regs.tx_fifo_count--;
	m_uart_regs.tx_fifo_read_idx = (m_uart_regs.tx_fifo_read_idx + 1) % std::size(m_uart_regs.tx_fifo);
	m_uart_regs.utsr1 |= (1 << UTSR1_TNF_BIT);

	if (m_uart_regs.tx_fifo_count)
		transmit_register_setup(m_uart_regs.tx_fifo[m_uart_regs.tx_fifo_read_idx]);
	else
		m_uart_regs.utsr1 &= ~(1 << UTSR1_TBY_BIT);

	uart_check_tx_fifo_service();
}

// Tx send bit
void sa1110_periphs_device::tra_callback()
{
	// TODO: Handle loopback mode
	m_uart3_tx_out(transmit_register_get_data_bit());
}

void sa1110_periphs_device::uart_recalculate_divisor()
{
	// TODO: Handle external UART clocking
	const int multiplier = (((m_uart_regs.utcr[1] & 0x0f) << 8) | (m_uart_regs.utcr[2] & 0xff)) + 1;
	set_rcv_rate(INTERNAL_OSC, multiplier * 16);
	set_tra_rate(INTERNAL_OSC, multiplier * 16);

	receive_register_reset();
	transmit_register_reset();
}

void sa1110_periphs_device::uart_update_eif_status()
{
	bool has_error = false;
	for (int i = 0; i < 4 && i < m_uart_regs.rx_fifo_count; i++)
	{
		const int read_idx = (m_uart_regs.rx_fifo_read_idx + i) % std::size(m_uart_regs.rx_fifo);
		if (m_uart_regs.rx_fifo[read_idx] & 0x700)
		{
			has_error = true;
			break;
		}
	}

	if (has_error)
	{
		m_uart_regs.utsr0 |= (1 << UTSR0_EIF_BIT);
		m_uart3_irqs->in_w<UART3_EIF>(1);
	}
	else
	{
		m_uart_regs.utsr0 &= ~(1 << UTSR0_EIF_BIT);
		m_uart3_irqs->in_w<UART3_EIF>(0);
	}
}

void sa1110_periphs_device::uart_write_receive_fifo(uint16_t data_and_flags)
{
	if (m_uart_regs.rx_fifo_count >= std::size(m_uart_regs.rx_fifo))
		return;
	if (!BIT(m_uart_regs.utcr[3], UTCR3_RXE_BIT))
		return;

	// fill FIFO entry
	m_uart_regs.rx_fifo[m_uart_regs.rx_fifo_write_idx] = data_and_flags;
	m_uart_regs.rx_fifo_count++;
	m_uart_regs.rx_fifo_write_idx = (m_uart_regs.rx_fifo_write_idx + 1) % std::size(m_uart_regs.rx_fifo);

	// update error flags
	uart_update_eif_status();

	// update FIFO-service interrupt
	uart_check_rx_fifo_service();
}

uint8_t sa1110_periphs_device::uart_read_receive_fifo()
{
	const uint8_t data = m_uart_regs.rx_fifo[m_uart_regs.rx_fifo_read_idx];
	if (m_uart_regs.rx_fifo_count)
	{
		m_uart_regs.rx_fifo_read_idx = (m_uart_regs.rx_fifo_read_idx + 1) % std::size(m_uart_regs.rx_fifo);
		m_uart_regs.rx_fifo_count--;
		if (m_uart_regs.rx_fifo_count)
		{
			const uint16_t fifo_bottom_flags = ((m_uart_regs.rx_fifo[m_uart_regs.rx_fifo_read_idx]) >> 8) & 7;
			m_uart_regs.utsr1 &= ~((1 << UTSR1_PRE_BIT) | (1 << UTSR1_FRE_BIT) | (1 << UTSR1_ROR_BIT));
			m_uart_regs.utsr1 |= fifo_bottom_flags << UTSR1_PRE_BIT;
		}
		uart_update_eif_status();
	}
	uart_check_rx_fifo_service();
	return data;
}

void sa1110_periphs_device::uart_check_rx_fifo_service()
{
	if (m_uart_regs.rx_fifo_count != 0)
		m_uart_regs.utsr1 |= (1 << UTSR1_RNE_BIT);
	else
		m_uart_regs.utsr1 &= ~(1 << UTSR1_RNE_BIT);

	if (m_uart_regs.rx_fifo_count > 4)
	{
		m_uart_regs.utsr0 |= (1 << UTSR0_RFS_BIT);
		if (BIT(m_uart_regs.utcr[3], UTCR3_RIE_BIT))
		{
			m_uart3_irqs->in_w<UART3_RFS>(1);
		}
	}
	else
	{
		m_uart_regs.utsr0 &= ~(1 << UTSR0_RFS_BIT);
		m_uart3_irqs->in_w<UART3_RFS>(0);
	}
}

void sa1110_periphs_device::uart_write_transmit_fifo(uint8_t data)
{
	if (m_uart_regs.tx_fifo_count >= std::size(m_uart_regs.tx_fifo))
		return;
	if (!BIT(m_uart_regs.utcr[3], UTCR3_TXE_BIT))
		return;

	// immediately start transmitting if FIFO is empty
	if (m_uart_regs.tx_fifo_count == 0)
	{
		m_uart_regs.utsr1 |= (1 << UTSR1_TBY_BIT);
		transmit_register_setup(data);
	}

	// fill FIFO entry
	m_uart_regs.tx_fifo[m_uart_regs.tx_fifo_write_idx] = data;
	m_uart_regs.tx_fifo_count++;
	m_uart_regs.tx_fifo_write_idx = (m_uart_regs.tx_fifo_write_idx + 1) % std::size(m_uart_regs.tx_fifo);

	// update FIFO-service interrupt
	uart_check_tx_fifo_service();
}

void sa1110_periphs_device::uart_check_tx_fifo_service()
{
	if (m_uart_regs.tx_fifo_count < std::size(m_uart_regs.tx_fifo))
		m_uart_regs.utsr1 |= (1 << UTSR1_TNF_BIT);
	else
		m_uart_regs.utsr1 &= ~(1 << UTSR1_TNF_BIT);

	if (m_uart_regs.tx_fifo_count <= 4)
	{
		m_uart_regs.utsr0 |= (1 << UTSR0_TFS_BIT);
		if (BIT(m_uart_regs.utcr[3], UTCR3_TIE_BIT))
		{
			m_uart3_irqs->in_w<UART3_TFS>(1);
		}
	}
	else
	{
		m_uart_regs.utsr0 &= ~(1 << UTSR0_TFS_BIT);
		m_uart3_irqs->in_w<UART3_TFS>(0);
	}
}

void sa1110_periphs_device::uart_set_receiver_idle()
{
}

void sa1110_periphs_device::uart_begin_of_break()
{
}

void sa1110_periphs_device::uart_end_of_break()
{
}

void sa1110_periphs_device::uart_set_receiver_enabled(bool enabled)
{
	if (!enabled)
	{
		m_uart_regs.utsr0 &= ~(1 << UTSR0_RFS_BIT);
		m_uart3_irqs->in_w<UART3_RFS>(0);

		m_uart_regs.utsr1 &= ~(1 << UTSR1_RNE_BIT);

		m_uart_regs.rx_fifo_count = 0;
		m_uart_regs.rx_fifo_read_idx = 0;
		m_uart_regs.rx_fifo_write_idx = 0;

		receive_register_reset();
	}
}

void sa1110_periphs_device::uart_set_transmitter_enabled(bool enabled)
{
	if (enabled)
	{
		//m_uart_regs.utsr0 |= (1 << UTSR0_TFS_BIT);
		//m_uart3_irqs->in_w<UART3_TFS>(1);

		//m_uart_regs.utsr1 |= (1 << UTSR1_TNF_BIT);
	}
	else
	{
		//m_uart_regs.utsr0 &= ~(1 << UTSR0_TFS_BIT);
		//m_uart3_irqs->in_w<UART3_TFS>(0);

		//m_uart_regs.utsr1 &= ~(1 << UTSR1_TBY_BIT);
		//m_uart_regs.utsr1 &= ~(1 << UTSR1_TNF_BIT);

		m_uart_regs.tx_fifo_count = 0;
		m_uart_regs.tx_fifo_read_idx = 0;
		m_uart_regs.tx_fifo_write_idx = 0;

		transmit_register_reset();
	}

	uart_check_tx_fifo_service();
}

void sa1110_periphs_device::uart_set_receive_irq_enabled(bool enabled)
{
}

void sa1110_periphs_device::uart_set_transmit_irq_enabled(bool enabled)
{
}

uint32_t sa1110_periphs_device::uart3_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_UTCR0:
		LOGMASKED(LOG_UART3, "%s: uart3_r: UART Control Register 0: %08x & %08x\n", machine().describe_context(), m_uart_regs.utcr[0], mem_mask);
		return m_uart_regs.utcr[0];
	case REG_UTCR1:
		LOGMASKED(LOG_UART3, "%s: uart3_r: UART Control Register 1: %08x & %08x\n", machine().describe_context(), m_uart_regs.utcr[1], mem_mask);
		return m_uart_regs.utcr[1];
	case REG_UTCR2:
		LOGMASKED(LOG_UART3, "%s: uart3_r: UART Control Register 2: %08x & %08x\n", machine().describe_context(), m_uart_regs.utcr[2], mem_mask);
		return m_uart_regs.utcr[2];
	case REG_UTCR3:
		LOGMASKED(LOG_UART3, "%s: uart3_r: UART Control Register 3: %08x & %08x\n", machine().describe_context(), m_uart_regs.utcr[3], mem_mask);
		return m_uart_regs.utcr[3];
	case REG_UTDR:
	{
		const uint8_t data = uart_read_receive_fifo();
		LOGMASKED(LOG_UART3, "%s: uart3_r: UART Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case REG_UTSR0:
		LOGMASKED(LOG_UART3, "%s: uart3_r: UART Status Register 0: %08x & %08x\n", machine().describe_context(), m_uart_regs.utsr0, mem_mask);
		return m_uart_regs.utsr0;
	case REG_UTSR1:
		LOGMASKED(LOG_UART3, "%s: uart3_r: UART Status Register 1: %08x & %08x\n", machine().describe_context(), m_uart_regs.utsr1, mem_mask);
		return m_uart_regs.utsr1;
	default:
		LOGMASKED(LOG_UART3 | LOG_UNKNOWN, "%s: uart3_r: Unknown address: %08x & %08x\n", machine().describe_context(), UART_BASE_ADDR | (offset << 2), mem_mask);
		return 0;
	}
}

void sa1110_periphs_device::uart3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_UTCR0:
	{
		LOGMASKED(LOG_UART3, "%s: uart3_w: UART Control Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_UART3, "%s:          Parity Enable: %d\n", machine().describe_context(), BIT(data, 0));
		LOGMASKED(LOG_UART3, "%s:          Parity Mode: %s\n", machine().describe_context(), BIT(data, 1) ? "Even" : "Odd");
		LOGMASKED(LOG_UART3, "%s:          Stop Bits: %d\n", machine().describe_context(), BIT(data, 2) + 1);
		LOGMASKED(LOG_UART3, "%s:          Data Size: %d\n", machine().describe_context(), BIT(data, 3) ? 8 : 7);
		LOGMASKED(LOG_UART3, "%s:          Sample Clock: %s\n", machine().describe_context(), BIT(data, 4) ? "External" : "Internal");
		LOGMASKED(LOG_UART3, "%s:          Receive Edge: %s\n", machine().describe_context(), BIT(data, 5) ? "Falling" : "Rising");
		LOGMASKED(LOG_UART3, "%s:          Transmit Edge: %s\n", machine().describe_context(), BIT(data, 6) ? "Falling" : "Rising");

		stop_bits_t stop_bits = (BIT(data, 2) ? STOP_BITS_2 : STOP_BITS_1);

		parity_t parity = PARITY_NONE;
		if (BIT(data, 0))
		{
			parity = (BIT(data, 1) ? PARITY_EVEN : PARITY_ODD);
		}

		set_data_frame(1, BIT(data, 3) ? 8 : 7, parity, stop_bits);
		receive_register_reset();
		transmit_register_reset();

		COMBINE_DATA(&m_uart_regs.utcr[0]);
		break;
	}
	case REG_UTCR1:
	{
		LOGMASKED(LOG_UART3, "%s: uart3_w: UART Control Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_UART3, "%s:          Baud Rate Divisor MSB: %02x\n", machine().describe_context(), data & 0x0f);
		const uint8_t old = m_uart_regs.utcr[1] & 0x0f;
		COMBINE_DATA(&m_uart_regs.utcr[1]);
		if ((m_uart_regs.utcr[1] & 0x0f) != old)
			uart_recalculate_divisor();
		break;
	}
	case REG_UTCR2:
	{
		LOGMASKED(LOG_UART3, "%s: uart3_w: UART Control Register 2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_UART3, "%s:          Baud Rate Divisor LSB: %02x\n", machine().describe_context(), (uint8_t)data);
		const uint8_t old = m_uart_regs.utcr[2] & 0xff;
		COMBINE_DATA(&m_uart_regs.utcr[2]);
		if ((m_uart_regs.utcr[2] & 0xff) != old)
			uart_recalculate_divisor();
		break;
	}
	case REG_UTCR3:
	{
		LOGMASKED(LOG_UART3, "%s: uart3_w: UART Control Register 3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_UART3, "%s:          Receive Enable: %d\n", machine().describe_context(), BIT(data, 0));
		LOGMASKED(LOG_UART3, "%s:          Transmit Enable: %d\n", machine().describe_context(), BIT(data, 1));
		LOGMASKED(LOG_UART3, "%s:          Send Break: %d\n", machine().describe_context(), BIT(data, 2));
		LOGMASKED(LOG_UART3, "%s:          Receive FIFO IRQ Enable: %d\n", machine().describe_context(), BIT(data, 3));
		LOGMASKED(LOG_UART3, "%s:          Transmit FIFO IRQ Enable: %d\n", machine().describe_context(), BIT(data, 4));
		LOGMASKED(LOG_UART3, "%s:          Loopback Enable: %d\n", machine().describe_context(), BIT(data, 5));
		const uint32_t old = m_uart_regs.utcr[3];
		COMBINE_DATA(&m_uart_regs.utcr[3]);
		const uint32_t changed = old ^ m_uart_regs.utcr[3];
		if (BIT(changed, 0))
			uart_set_receiver_enabled(BIT(data, 0));
		if (BIT(changed, 1))
			uart_set_transmitter_enabled(BIT(data, 1));
		if (BIT(changed, 3))
			uart_set_receive_irq_enabled(BIT(data, 3));
		if (BIT(changed, 4))
			uart_set_transmit_irq_enabled(BIT(data, 4));
		break;
	}
	case REG_UTDR:
		LOGMASKED(LOG_UART3, "%s: uart3_w: UART Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (data == 0x0d || data == 0x0a || (data >= 0x20 && data < 0x7f))
		{
			printf("%c", (char)data);
		}
		uart_write_transmit_fifo((uint8_t)data);
		break;
	case REG_UTSR0:
		LOGMASKED(LOG_UART3, "%s: uart3_w: UART Status Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_UART3, "%s:          Receiver Idle Status: %d\n", machine().describe_context(), BIT(data, 2));
		LOGMASKED(LOG_UART3, "%s:          Receiver Begin of Break Status: %d\n", machine().describe_context(), BIT(data, 3));
		LOGMASKED(LOG_UART3, "%s:          Receiver End of Break Status: %d\n", machine().describe_context(), BIT(data, 4));
		if (BIT(data, 2))
			uart_set_receiver_idle();
		if (BIT(data, 3))
			uart_begin_of_break();
		if (BIT(data, 4))
			uart_end_of_break();
		break;
	default:
		LOGMASKED(LOG_UART3 | LOG_UNKNOWN, "%s: uart3_w: Unknown address: %08x = %08x & %08x\n", machine().describe_context(), UART_BASE_ADDR | (offset << 2), data, mem_mask);
		break;
	}
}

/*

  Intel SA-1110 MCP - Serial Port 4

  pg. 306 to 346 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

WRITE_LINE_MEMBER(sa1110_periphs_device::mcp_irq_callback)
{
	set_irq_line(INT_MCP, state);
}

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::mcp_audio_tx_callback)
{
	if (!m_codec)
		return;

	const uint16_t sample = m_mcp_regs.audio_tx_fifo[m_mcp_regs.audio_tx_fifo_read_idx];
	m_codec->audio_sample_in(sample);

	if (m_mcp_regs.audio_tx_fifo_count)
	{
		m_mcp_regs.audio_tx_fifo_count--;
		m_mcp_regs.audio_tx_fifo_read_idx = (m_mcp_regs.audio_tx_fifo_read_idx + 1) % std::size(m_mcp_regs.audio_tx_fifo);

		m_mcp_regs.mcsr &= ~(1 << MCSR_ATU_BIT);
		m_mcp_irqs->in_w<MCP_AUDIO_UNDERRUN>(0);
	}
	else
	{
		m_mcp_regs.mcsr |= (1 << MCSR_ATU_BIT);
		m_mcp_irqs->in_w<MCP_AUDIO_UNDERRUN>(1);
	}

	m_mcp_regs.mcsr |= (1 << MCSR_ANF_BIT);
}

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::mcp_telecom_tx_callback)
{
	if (!m_codec)
		return;

	const uint16_t sample = m_mcp_regs.telecom_tx_fifo[m_mcp_regs.telecom_tx_fifo_read_idx];
	m_codec->telecom_sample_in(sample);

	if (m_mcp_regs.telecom_tx_fifo_count)
	{
		m_mcp_regs.telecom_tx_fifo_count--;
		m_mcp_regs.telecom_tx_fifo_read_idx = (m_mcp_regs.telecom_tx_fifo_read_idx + 1) % std::size(m_mcp_regs.telecom_tx_fifo);

		m_mcp_regs.mcsr &= ~(1 << MCSR_TTU_BIT);
		m_mcp_irqs->in_w<MCP_TELECOM_UNDERRUN>(0);
	}
	else
	{
		m_mcp_regs.mcsr |= (1 << MCSR_TTU_BIT);
		m_mcp_irqs->in_w<MCP_TELECOM_UNDERRUN>(1);
	}

	m_mcp_regs.mcsr |= (1 << MCSR_TNF_BIT);
}

uint16_t sa1110_periphs_device::mcp_read_audio_fifo()
{
	const uint16_t data = m_mcp_regs.audio_rx_fifo[m_mcp_regs.audio_rx_fifo_read_idx];
	if (m_mcp_regs.audio_rx_fifo_count)
	{
		m_mcp_regs.audio_rx_fifo_count--;
		m_mcp_regs.audio_rx_fifo_read_idx = (m_mcp_regs.audio_rx_fifo_read_idx + 1) % std::size(m_mcp_regs.audio_rx_fifo);

		const bool half_full = m_mcp_regs.audio_rx_fifo_count >= 4;
		m_mcp_regs.mcsr &= ~(1 << MCSR_ARS_BIT);
		if (half_full)
		{
			m_mcp_regs.mcsr |= (1 << MCSR_ARS_BIT);
		}
		bool fifo_interrupt = BIT(m_mcp_regs.mccr0, MCCR0_ARE_BIT) && half_full;
		m_mcp_irqs->in_w<MCP_AUDIO_RX>((int)fifo_interrupt);

		if (m_mcp_regs.audio_rx_fifo_count)
			m_mcp_regs.mcsr &= ~(1 << MCSR_ANE_BIT);
		else
			m_mcp_regs.mcsr |= (1 << MCSR_ANE_BIT);
	}
	return data;
}

uint16_t sa1110_periphs_device::mcp_read_telecom_fifo()
{
	const uint16_t data = m_mcp_regs.telecom_rx_fifo[m_mcp_regs.telecom_rx_fifo_read_idx];
	if (m_mcp_regs.telecom_rx_fifo_count)
	{
		m_mcp_regs.telecom_rx_fifo_count--;
		m_mcp_regs.telecom_rx_fifo_read_idx = (m_mcp_regs.telecom_rx_fifo_read_idx + 1) % std::size(m_mcp_regs.telecom_rx_fifo);

		const bool half_full = m_mcp_regs.telecom_rx_fifo_count >= 4;
		m_mcp_regs.mcsr &= ~(1 << MCSR_TRS_BIT);
		if (half_full)
		{
			m_mcp_regs.mcsr |= (1 << MCSR_TRS_BIT);
		}
		bool fifo_interrupt = BIT(m_mcp_regs.mccr0, MCCR0_TRE_BIT) && half_full;
		m_mcp_irqs->in_w<MCP_TELECOM_RX>((int)fifo_interrupt);

		if (m_mcp_regs.telecom_rx_fifo_count)
			m_mcp_regs.mcsr &= ~(1 << MCSR_TNE_BIT);
		else
			m_mcp_regs.mcsr |= (1 << MCSR_TNE_BIT);
	}
	return data;
}

attotime sa1110_periphs_device::mcp_get_audio_frame_rate()
{
	const uint32_t bit_rate = BIT(m_mcp_regs.mccr1, MCCR1_CFS_BIT) ? 9585000 : 11981000;
	const uint64_t ticks = 32 * ((m_mcp_regs.mccr0 & MCCR0_ASD_MASK) >> MCCR0_ASD_BIT);
	return attotime::from_ticks(ticks, bit_rate);
}

attotime sa1110_periphs_device::mcp_get_telecom_frame_rate()
{
	const uint32_t bit_rate = BIT(m_mcp_regs.mccr1, MCCR1_CFS_BIT) ? 9585000 : 11981000;
	const uint64_t ticks = 32 * ((m_mcp_regs.mccr0 & MCCR0_TSD_MASK) >> MCCR0_TSD_BIT);
	return attotime::from_ticks(ticks, bit_rate);
}

void sa1110_periphs_device::mcp_update_sample_rate()
{
	const attotime audio_rate = mcp_get_audio_frame_rate();
	m_mcp_regs.audio_tx_timer->adjust(audio_rate, 0, audio_rate);

	const attotime telecom_rate = mcp_get_telecom_frame_rate();
	m_mcp_regs.telecom_tx_timer->adjust(telecom_rate, 0, telecom_rate);
}

void sa1110_periphs_device::mcp_set_enabled(bool enabled)
{
	if (enabled)
	{
		mcp_update_sample_rate();
	}
	else
	{
		m_mcp_regs.audio_tx_timer->adjust(attotime::never);
		m_mcp_regs.telecom_tx_timer->adjust(attotime::never);
	}
}

void sa1110_periphs_device::mcp_audio_tx_fifo_push(const uint16_t value)
{
	if (m_mcp_regs.audio_rx_fifo_count == std::size(m_mcp_regs.audio_tx_fifo))
		return;

	m_mcp_regs.audio_tx_fifo[m_mcp_regs.audio_tx_fifo_write_idx] = value;
	m_mcp_regs.audio_rx_fifo_write_idx = (m_mcp_regs.audio_tx_fifo_write_idx + 1) % std::size(m_mcp_regs.audio_tx_fifo);
	m_mcp_regs.audio_rx_fifo_count++;

	if (m_mcp_regs.audio_tx_fifo_count == std::size(m_mcp_regs.audio_tx_fifo))
		m_mcp_regs.mcsr &= ~(1 << MCSR_ANF_BIT);

	if (m_mcp_regs.audio_tx_fifo_count >= 4)
	{
		m_mcp_regs.mcsr &= ~(1 << MCSR_ATS_BIT);
		if (BIT(m_mcp_regs.mccr0, MCCR0_ATE_BIT))
			m_mcp_irqs->in_w<MCP_AUDIO_TX>(0);
	}
	else
	{
		m_mcp_regs.mcsr |= (1 << MCSR_ATS_BIT);
		if (BIT(m_mcp_regs.mccr0, MCCR0_ATE_BIT))
			m_mcp_irqs->in_w<MCP_AUDIO_TX>(1);
	}
}

void sa1110_periphs_device::mcp_telecom_tx_fifo_push(const uint16_t value)
{
	if (m_mcp_regs.telecom_rx_fifo_count == std::size(m_mcp_regs.telecom_tx_fifo))
		return;

	m_mcp_regs.telecom_tx_fifo[m_mcp_regs.telecom_tx_fifo_write_idx] = value;
	m_mcp_regs.telecom_rx_fifo_write_idx = (m_mcp_regs.telecom_tx_fifo_write_idx + 1) % std::size(m_mcp_regs.telecom_tx_fifo);
	m_mcp_regs.telecom_rx_fifo_count++;

	if (m_mcp_regs.telecom_tx_fifo_count == std::size(m_mcp_regs.telecom_tx_fifo))
		m_mcp_regs.mcsr &= ~(1 << MCSR_TNF_BIT);

	if (m_mcp_regs.audio_tx_fifo_count >= 4)
	{
		m_mcp_regs.mcsr &= ~(1 << MCSR_TTS_BIT);
		if (BIT(m_mcp_regs.mccr0, MCCR0_TTE_BIT))
			m_mcp_irqs->in_w<MCP_TELECOM_TX>(0);
	}
	else
	{
		m_mcp_regs.mcsr |= (1 << MCSR_TTS_BIT);
		if (BIT(m_mcp_regs.mccr0, MCCR0_TTE_BIT))
			m_mcp_irqs->in_w<MCP_TELECOM_TX>(1);
	}
}

void sa1110_periphs_device::mcp_codec_read(offs_t offset)
{
	if (!m_codec)
		return;

	const uint16_t data = m_codec->read(offset);
	m_mcp_regs.mcdr2 &= 0xffff0000;
	m_mcp_regs.mcdr2 |= data;

	m_mcp_regs.mcsr |= (1 << MCSR_CRC_BIT);
	m_mcp_regs.mcsr &= ~(1 << MCSR_CWC_BIT);
}

void sa1110_periphs_device::mcp_codec_write(offs_t offset, uint16_t data)
{
	if (!m_codec)
		return;

	m_codec->write(offset, data);
	m_mcp_regs.mcsr |= (1 << MCSR_CWC_BIT);
	m_mcp_regs.mcsr &= ~(1 << MCSR_CRC_BIT);
}

uint32_t sa1110_periphs_device::mcp_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_MCCR0:
		LOGMASKED(LOG_MCP, "%s: mcp_r: MCP Control Register 0: %08x & %08x\n", machine().describe_context(), m_mcp_regs.mccr0, mem_mask);
		return m_mcp_regs.mccr0;
	case REG_MCDR0:
	{
		const uint16_t data = mcp_read_audio_fifo() << 4;
		LOGMASKED(LOG_MCP, "%s: mcp_r: MCP Data Register 0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case REG_MCDR1:
	{
		const uint16_t data = mcp_read_telecom_fifo() << 4;
		LOGMASKED(LOG_MCP, "%s: mcp_r: MCP Data Register 1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case REG_MCDR2:
		LOGMASKED(LOG_MCP, "%s: mcp_r: MCP Data Register 2: %08x & %08x\n", machine().describe_context(), m_mcp_regs.mcdr2, mem_mask);
		LOGMASKED(LOG_MCP, "%s:        Value: %04x\n", machine().describe_context(), (uint16_t)m_mcp_regs.mcdr2);
		LOGMASKED(LOG_MCP, "%s:        Read/Write: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcdr2, 16));
		LOGMASKED(LOG_MCP, "%s:        Address: %01x\n", machine().describe_context(), (m_mcp_regs.mcdr2 >> 17) & 0xf);
		return m_mcp_regs.mcdr2;
	case REG_MCSR:
		LOGMASKED(LOG_MCP, "%s: mcp_r: MCP Status Register: %08x & %08x\n", machine().describe_context(), m_mcp_regs.mcsr, mem_mask);
		LOGMASKED(LOG_MCP, "%s:        Audio Xmit FIFO Service Request: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 0));
		LOGMASKED(LOG_MCP, "%s:        Audio Recv FIFO Service Request: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 1));
		LOGMASKED(LOG_MCP, "%s:        Telecom Xmit FIFO Service Request: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 2));
		LOGMASKED(LOG_MCP, "%s:        Telecom Recv FIFO Service Request: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 3));
		LOGMASKED(LOG_MCP, "%s:        Audio Xmit FIFO Underrun: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 4));
		LOGMASKED(LOG_MCP, "%s:        Audio Recv FIFO Overrun: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 5));
		LOGMASKED(LOG_MCP, "%s:        Telcom Xmit FIFO Underrun: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 6));
		LOGMASKED(LOG_MCP, "%s:        Telcom Recv FIFO Overrun: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 7));
		LOGMASKED(LOG_MCP, "%s:        Audio Xmit FIFO Not Full: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 8));
		LOGMASKED(LOG_MCP, "%s:        Audio Recv FIFO Not Empty: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 9));
		LOGMASKED(LOG_MCP, "%s:        Telcom Xmit FIFO Not Full: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 10));
		LOGMASKED(LOG_MCP, "%s:        Telcom Recv FIFO Not Empty: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 11));
		LOGMASKED(LOG_MCP, "%s:        Codec Write Complete: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 12));
		LOGMASKED(LOG_MCP, "%s:        Codec Read Complete: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 13));
		LOGMASKED(LOG_MCP, "%s:        Audio Codec Enabled: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 14));
		LOGMASKED(LOG_MCP, "%s:        Telecom Codec Enabled: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 15));
		return m_mcp_regs.mcsr;
	default:
		LOGMASKED(LOG_MCP | LOG_UNKNOWN, "%s: ostimer_r: Unknown address: %08x & %08x\n", machine().describe_context(), MCP_BASE_ADDR | (offset << 2), mem_mask);
		return 0;
	}
}

void sa1110_periphs_device::mcp_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_MCCR0:
	{
		LOGMASKED(LOG_MCP, "%s: mcp_w: MCP Control Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_MCP, "%s:        Audio Sample Rate Divisor: %02x\n", machine().describe_context(), data & MCCR0_ASD_MASK);
		LOGMASKED(LOG_MCP, "%s:        Telecom Sample Rate Divisor: %02x\n", machine().describe_context(), (data & MCCR0_TSD_MASK) >> MCCR0_TSD_BIT);
		LOGMASKED(LOG_MCP, "%s:        MCP Enable: %d\n", machine().describe_context(), BIT(data, MCCR0_MCE_BIT));
		LOGMASKED(LOG_MCP, "%s:        Clock Select: %s\n", machine().describe_context(), BIT(data, MCCR0_ECS_BIT) ? "External" : "Internal");
		LOGMASKED(LOG_MCP, "%s:        A/D Data Sampling Mode: %s Valid\n", machine().describe_context(), BIT(data, MCCR0_ADM_BIT) ? "First" : "Each");
		LOGMASKED(LOG_MCP, "%s:        Telecom Tx FIFO Interrupt Enable: %d\n", machine().describe_context(), BIT(data, MCCR0_TTE_BIT));
		LOGMASKED(LOG_MCP, "%s:        Telecom Rx FIFO Interrupt Enable: %d\n", machine().describe_context(), BIT(data, MCCR0_TRE_BIT));
		LOGMASKED(LOG_MCP, "%s:        Audio Tx FIFO Interrupt Enable: %d\n", machine().describe_context(), BIT(data, MCCR0_ATE_BIT));
		LOGMASKED(LOG_MCP, "%s:        Audio Rx FIFO Interrupt Enable: %d\n", machine().describe_context(), BIT(data, MCCR0_ARE_BIT));
		LOGMASKED(LOG_MCP, "%s:        Loopback Enable: %d\n", machine().describe_context(), BIT(data, MCCR0_LBM_BIT));
		LOGMASKED(LOG_MCP, "%s:        External Clock Prescaler: %d\n", machine().describe_context(), ((data & MCCR0_ECP_MASK) >> MCCR0_ECP_BIT) + 1);
		const uint32_t old = m_mcp_regs.mccr0;
		COMBINE_DATA(&m_mcp_regs.mccr0);
		const uint32_t changed = old ^ m_mcp_regs.mccr0;
		if (BIT(m_mcp_regs.mcsr, MCSR_ATS_BIT) && BIT(changed, MCCR0_ATE_BIT))
			m_mcp_irqs->in_w<MCP_AUDIO_TX>(BIT(m_mcp_regs.mcsr, MCSR_ATS_BIT));
		if (BIT(m_mcp_regs.mcsr, MCSR_ARS_BIT) && BIT(changed, MCCR0_ARE_BIT))
			m_mcp_irqs->in_w<MCP_AUDIO_RX>(BIT(m_mcp_regs.mcsr, MCSR_ARS_BIT));
		if (BIT(m_mcp_regs.mcsr, MCSR_TTS_BIT) && BIT(changed, MCCR0_TTE_BIT))
			m_mcp_irqs->in_w<MCP_TELECOM_TX>(BIT(m_mcp_regs.mcsr, MCSR_TTS_BIT));
		if (BIT(m_mcp_regs.mcsr, MCSR_TRS_BIT) && BIT(changed, MCCR0_TRE_BIT))
			m_mcp_irqs->in_w<MCP_TELECOM_RX>(BIT(m_mcp_regs.mcsr, MCSR_TRS_BIT));
		if (BIT(old, MCCR0_MCE_BIT) != BIT(m_mcp_regs.mccr0, MCCR0_MCE_BIT))
			mcp_set_enabled(BIT(m_mcp_regs.mccr0, MCCR0_MCE_BIT));
		break;
	}
	case REG_MCDR0:
		LOGMASKED(LOG_MCP, "%s: mcp_w: MCP Data Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		mcp_audio_tx_fifo_push((uint16_t)data);
		break;
	case REG_MCDR1:
		LOGMASKED(LOG_MCP, "%s: mcp_w: MCP Data Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		mcp_telecom_tx_fifo_push((uint16_t)data);
		break;
	case REG_MCDR2:
	{
		const offs_t addr = (data & MCDR2_ADDR_MASK) >> MCDR2_ADDR_BIT;
		LOGMASKED(LOG_MCP, "%s: mcp_w: MCP Data Register 2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_mcp_regs.mcdr2);
		m_mcp_regs.mcdr2 &= ~(1 << MCDR2_RW_BIT);

		if (BIT(data, MCDR2_RW_BIT))
			mcp_codec_write(addr, (uint16_t)data);
		else
			mcp_codec_read(addr);
		break;
	}
	case REG_MCSR:
	{
		LOGMASKED(LOG_MCP, "%s: mcp_w: MCP Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		const uint32_t old = m_mcp_regs.mcsr;
		const uint32_t sticky_mask = (1 << MCSR_ATU_BIT) | (1 << MCSR_ARO_BIT) | (1 << MCSR_TTU_BIT) | (1 << MCSR_TRO_BIT);
		m_mcp_regs.mcsr &= ~(data & mem_mask & sticky_mask);
		if (BIT(old, MCSR_ATU_BIT) && !BIT(m_mcp_regs.mcsr, MCSR_ATU_BIT))
			m_mcp_irqs->in_w<MCP_AUDIO_UNDERRUN>(0);
		if (BIT(old, MCSR_ARO_BIT) && !BIT(m_mcp_regs.mcsr, MCSR_ARO_BIT))
			m_mcp_irqs->in_w<MCP_AUDIO_OVERRUN>(0);
		if (BIT(old, MCSR_TTU_BIT) && !BIT(m_mcp_regs.mcsr, MCSR_TTU_BIT))
			m_mcp_irqs->in_w<MCP_TELECOM_UNDERRUN>(0);
		if (BIT(old, MCSR_TRO_BIT) && !BIT(m_mcp_regs.mcsr, MCSR_TRO_BIT))
			m_mcp_irqs->in_w<MCP_TELECOM_OVERRUN>(0);
		break;
	}
	default:
		LOGMASKED(LOG_MCP | LOG_UNKNOWN, "%s: mcp_w: Unknown address: %08x = %08x & %08x\n", machine().describe_context(), MCP_BASE_ADDR | (offset << 2),
			data, mem_mask);
		break;
	}
}

/*

  Intel SA-1110 SSP - Synchronous Serial Port

  pg. 331 to 347 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::ssp_rx_callback)
{
	// TODO: Implement receiving data serially rather than in bulk.
}

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::ssp_tx_callback)
{
	// TODO: Implement transmitting data serially rather than in bulk.
	if (m_ssp_regs.tx_fifo_count)
	{
		const uint16_t data = m_ssp_regs.tx_fifo[m_ssp_regs.tx_fifo_read_idx];
		m_ssp_out(data);

		m_ssp_regs.tx_fifo_read_idx = (m_ssp_regs.tx_fifo_read_idx + 1) % std::size(m_ssp_regs.tx_fifo);
		m_ssp_regs.tx_fifo_count--;

		m_ssp_regs.sssr |= (1 << SSSR_TNF_BIT);

		ssp_update_tx_level();
	}
}

void sa1110_periphs_device::ssp_update_enable_state()
{
	if (BIT(m_ssp_regs.sscr0, SSCR0_SSE_BIT))
	{
		if (m_ssp_regs.tx_fifo_count != std::size(m_ssp_regs.tx_fifo))
			m_ssp_regs.sssr |= (1 << SSSR_TNF_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_TNF_BIT);

		if (m_ssp_regs.rx_fifo_count != 0)
			m_ssp_regs.sssr |= (1 << SSSR_RNE_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_RNE_BIT);

		if (m_ssp_regs.tx_fifo_count != 0)
			m_ssp_regs.sssr |= (1 << SSSR_BSY_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_BSY_BIT);

		if (m_ssp_regs.tx_fifo_count <= 4)
			m_ssp_regs.sssr |= (1 << SSSR_TFS_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_TFS_BIT);

		if (m_ssp_regs.rx_fifo_count >= 4)
			m_ssp_regs.sssr |= (1 << SSSR_RFS_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_RFS_BIT);

		uint64_t bit_count = (m_ssp_regs.sscr0 & SSCR0_DSS_MASK) >> SSCR0_DSS_BIT;
		uint32_t clock_rate = 2 * (((m_ssp_regs.sscr0 & SSCR0_SCR_MASK) >> SSCR0_SCR_BIT) + 1);
		attotime packet_rate = attotime::from_ticks(bit_count * clock_rate, 3686400);
		m_ssp_regs.rx_timer->adjust(packet_rate, 0, packet_rate);
		m_ssp_regs.tx_timer->adjust(packet_rate, 0, packet_rate);
	}
	else
	{
		m_ssp_regs.sssr &= ~(1 << SSSR_TFS_BIT);
		m_ssp_regs.sssr &= ~(1 << SSSR_RFS_BIT);

		m_ssp_regs.rx_fifo_read_idx = 0;
		m_ssp_regs.rx_fifo_write_idx = 0;
		m_ssp_regs.rx_fifo_count = 0;
		m_ssp_regs.tx_fifo_read_idx = 0;
		m_ssp_regs.tx_fifo_write_idx = 0;
		m_ssp_regs.tx_fifo_count = 0;

		m_ssp_regs.rx_timer->adjust(attotime::never);
		m_ssp_regs.tx_timer->adjust(attotime::never);
	}
}

void sa1110_periphs_device::ssp_update_rx_level()
{
	if (m_ssp_regs.rx_fifo_count >= 4)
		m_ssp_regs.sssr |= (1 << SSSR_RFS_BIT);
	else
		m_ssp_regs.sssr &= ~(1 << SSSR_RFS_BIT);
}

void sa1110_periphs_device::ssp_rx_fifo_push(const uint16_t data)
{
	if (m_ssp_regs.rx_fifo_count < std::size(m_ssp_regs.rx_fifo))
	{
		m_ssp_regs.rx_fifo[m_ssp_regs.rx_fifo_write_idx] = data;
		m_ssp_regs.rx_fifo_write_idx = (m_ssp_regs.rx_fifo_write_idx + 1) % std::size(m_ssp_regs.rx_fifo);
		m_ssp_regs.rx_fifo_count++;

		m_ssp_regs.sssr |= (1 << SSSR_RNE_BIT);

		ssp_update_rx_level();
	}
}

void sa1110_periphs_device::ssp_update_tx_level()
{
	if (m_ssp_regs.tx_fifo_count <= 4)
		m_ssp_regs.sssr |= (1 << SSSR_TFS_BIT);
	else
		m_ssp_regs.sssr &= ~(1 << SSSR_TFS_BIT);
}

void sa1110_periphs_device::ssp_tx_fifo_push(const uint16_t data)
{
	if (m_ssp_regs.tx_fifo_count < std::size(m_ssp_regs.tx_fifo))
	{
		m_ssp_regs.tx_fifo[m_ssp_regs.tx_fifo_write_idx] = data;
		m_ssp_regs.tx_fifo_write_idx = (m_ssp_regs.tx_fifo_write_idx + 1) % std::size(m_ssp_regs.tx_fifo);
		m_ssp_regs.tx_fifo_count++;

		if (m_ssp_regs.tx_fifo_count != std::size(m_ssp_regs.tx_fifo))
			m_ssp_regs.sssr |= (1 << SSSR_TNF_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_TNF_BIT);

		ssp_update_tx_level();
	}

	if (m_ssp_regs.tx_fifo_count || m_ssp_regs.rx_fifo_count)
		m_ssp_regs.sssr |= (1 << SSSR_BSY_BIT);
	else
		m_ssp_regs.sssr &= ~(1 << SSSR_BSY_BIT);
}

uint16_t sa1110_periphs_device::ssp_rx_fifo_pop()
{
	uint16_t data = m_ssp_regs.rx_fifo[m_ssp_regs.rx_fifo_read_idx];
	if (m_ssp_regs.rx_fifo_count)
	{
		m_ssp_regs.rx_fifo_read_idx = (m_ssp_regs.rx_fifo_read_idx + 1) % std::size(m_ssp_regs.rx_fifo);
		m_ssp_regs.rx_fifo_count--;

		if (m_ssp_regs.rx_fifo_count == 0)
			m_ssp_regs.sssr &= ~(1 << SSSR_RNE_BIT);

		ssp_update_rx_level();
	}
	return data;
}

uint32_t sa1110_periphs_device::ssp_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_SSCR0:
		LOGMASKED(LOG_SSP, "%s: ssp_r: SSP Control Register 0: %08x & %08x\n", machine().describe_context(), m_ssp_regs.sscr0, mem_mask);
		return m_ssp_regs.sscr0;
	case REG_SSCR1:
		LOGMASKED(LOG_SSP, "%s: ssp_r: SSP Control Register 1: %08x & %08x\n", machine().describe_context(), m_ssp_regs.sscr1, mem_mask);
		return m_ssp_regs.sscr1;
	case REG_SSDR:
	{
		const uint32_t data = ssp_rx_fifo_pop();
		LOGMASKED(LOG_SSP, "%s: ssp_r: SSP Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		return data;
	}
	case REG_SSSR:
		LOGMASKED(LOG_SSP, "%s: ssp_r: SSP Status Register: %08x & %08x\n", machine().describe_context(), m_ssp_regs.sssr, mem_mask);
		LOGMASKED(LOG_SSP, "%s:        Transmit FIFO Not Full: %d\n", machine().describe_context(), BIT(m_ssp_regs.sssr, SSSR_TNF_BIT));
		LOGMASKED(LOG_SSP, "%s:        Receive FIFO Not Empty: %d\n", machine().describe_context(), BIT(m_ssp_regs.sssr, SSSR_RNE_BIT));
		LOGMASKED(LOG_SSP, "%s:        SSP Busy: %d\n", machine().describe_context(), BIT(m_ssp_regs.sssr, SSSR_BSY_BIT));
		LOGMASKED(LOG_SSP, "%s:        Transmit FIFO Service Request: %d\n", machine().describe_context(), BIT(m_ssp_regs.sssr, SSSR_TFS_BIT));
		LOGMASKED(LOG_SSP, "%s:        Receive FIFO Service Request: %d\n", machine().describe_context(), BIT(m_ssp_regs.sssr, SSSR_RFS_BIT));
		LOGMASKED(LOG_SSP, "%s:        Receive Overrun: %d\n", machine().describe_context(), BIT(m_ssp_regs.sssr, SSSR_ROR_BIT));
		return m_ssp_regs.sssr;
	default:
		LOGMASKED(LOG_SSP | LOG_UNKNOWN, "%s: ssp_r: Unknown address: %08x & %08x\n", machine().describe_context(), SSP_BASE_ADDR | (offset << 2), mem_mask);
		return 0;
	}
}

void sa1110_periphs_device::ssp_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_SSCR0:
	{
		static const char *const s_dss_sizes[16] =
		{
			"Invalid [1]", "Invalid [2]", "Invalid [3]", "4-bit",
			"5-bit", "6-bit", "7-bit", "8-bit",
			"9-bit", "10-bit", "11-bit", "12-bit",
			"13-bit", "14-bit", "15-bit", "16-bit"
		};
		static const char *const s_frf_formats[4] = { "Motorola SPI", "TI Synchronous Serial", "National Microwire", "Reserved" };
		LOGMASKED(LOG_SSP, "%s: ssp_w: SSP Control Register 0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_SSP, "%s:        Data Size Select: %s\n", machine().describe_context(), s_dss_sizes[(data & SSCR0_DSS_MASK) >> SSCR0_DSS_BIT]);
		LOGMASKED(LOG_SSP, "%s:        Frame Format: %s\n", machine().describe_context(), s_frf_formats[(data & SSCR0_FRF_MASK) >> SSCR0_FRF_BIT]);
		LOGMASKED(LOG_SSP, "%s:        SSP Enable: %d\n", machine().describe_context(), BIT(data, SSCR0_SSE_BIT));
		LOGMASKED(LOG_SSP, "%s:        Serial Clock Rate Divisor: %d\n", machine().describe_context(), 2 * (data & SSCR0_DSS_MASK) >> SSCR0_DSS_BIT);
		const uint32_t old = m_ssp_regs.sscr0;
		COMBINE_DATA(&m_ssp_regs.sscr0);
		if (BIT(old ^ m_ssp_regs.sscr0, SSCR0_SSE_BIT))
			ssp_update_enable_state();
		break;
	}
	case REG_SSCR1:
	{
		LOGMASKED(LOG_SSP, "%s: ssp_w: SSP Control Register 1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_SSP, "%s:        Receive FIFO Interrupt Enable: %d\n", machine().describe_context(), BIT(data, SSCR1_RIE_BIT));
		LOGMASKED(LOG_SSP, "%s:        Transmit FIFO Interrupt Enable: %d\n", machine().describe_context(), BIT(data, SSCR1_TIE_BIT));
		LOGMASKED(LOG_SSP, "%s:        Loopback Mode Enable: %d\n", machine().describe_context(), BIT(data, SSCR1_LBM_BIT));
		LOGMASKED(LOG_SSP, "%s:        Serial Clock Polarity: %d\n", machine().describe_context(), BIT(data, SSCR1_SPO_BIT));
		LOGMASKED(LOG_SSP, "%s:        Serial Clock Phase: %d\n", machine().describe_context(), BIT(data, SSCR1_SPH_BIT));
		LOGMASKED(LOG_SSP, "%s:        External Clock Select: %d\n", machine().describe_context(), BIT(data, SSCR1_ECS_BIT));
		COMBINE_DATA(&m_ssp_regs.sscr1);
		break;
	}
	case REG_SSDR:
		LOGMASKED(LOG_SSP, "%s: ssp_w: SSP Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		ssp_tx_fifo_push((uint16_t)data);
		break;
	case REG_SSSR:
		LOGMASKED(LOG_SSP, "%s: ssp_w: SSP Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_SSP, "%s:        Clear Receive Overrun: %d\n", machine().describe_context(), BIT(data, SSSR_ROR_BIT));
		break;
	default:
		LOGMASKED(LOG_SSP | LOG_UNKNOWN, "%s: ssp_w: Unknown address: %08x = %08x & %08x\n", machine().describe_context(), SSP_BASE_ADDR | (offset << 2), data, mem_mask);
		break;
	}
}

/*

  Intel SA-1110 Operating System Timer

  pg. 92 to 96 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::ostimer_tick_cb)
{
	const int channel = param;
	if (BIT(m_ostmr_regs.oier, channel))
	{
		m_ostmr_regs.ossr |= (1 << channel);
		set_irq_line(INT_OSTIMER0 + channel, 1);
		// TODO: Handle Channel 3, watchdog timer mode
	}
}

void sa1110_periphs_device::ostimer_update_count()
{
	const attotime time_delta = machine().time() - m_ostmr_regs.last_count_sync;
	const uint64_t ticks_elapsed = time_delta.as_ticks(INTERNAL_OSC);
	if (ticks_elapsed == 0ULL) // Accrue time until we can tick at least once
		return;

	const uint32_t wrapped_ticks = (uint32_t)ticks_elapsed;
	m_ostmr_regs.oscr += wrapped_ticks;
	m_ostmr_regs.last_count_sync = machine().time();
}

void sa1110_periphs_device::ostimer_update_match_timer(int channel)
{
	uint64_t ticks_remaining = m_ostmr_regs.osmr[channel] - m_ostmr_regs.oscr;
	if (m_ostmr_regs.oscr >= m_ostmr_regs.osmr[channel])
		ticks_remaining += 0x100000000ULL;
	m_ostmr_regs.timer[channel]->adjust(attotime::from_ticks(ticks_remaining, INTERNAL_OSC), channel);
}

uint32_t sa1110_periphs_device::ostimer_r(offs_t offset, uint32_t mem_mask)
{
	ostimer_update_count();

	switch (offset)
	{
	case REG_OSMR0:
		LOGMASKED(LOG_OSTIMER_HF, "%s: ostimer_r: OS Timer Match Register 0: %08x & %08x\n", machine().describe_context(), m_ostmr_regs.osmr[0], mem_mask);
		return m_ostmr_regs.osmr[0];
	case REG_OSMR1:
		LOGMASKED(LOG_OSTIMER, "%s: ostimer_r: OS Timer Match Register 1: %08x & %08x\n", machine().describe_context(), m_ostmr_regs.osmr[1], mem_mask);
		return m_ostmr_regs.osmr[1];
	case REG_OSMR2:
		LOGMASKED(LOG_OSTIMER, "%s: ostimer_r: OS Timer Match Register 2: %08x & %08x\n", machine().describe_context(), m_ostmr_regs.osmr[2], mem_mask);
		return m_ostmr_regs.osmr[2];
	case REG_OSMR3:
		LOGMASKED(LOG_OSTIMER, "%s: ostimer_r: OS Timer Match Register 3: %08x & %08x\n", machine().describe_context(), m_ostmr_regs.osmr[3], mem_mask);
		return m_ostmr_regs.osmr[3];
	case REG_OSCR:
		LOGMASKED(LOG_OSTIMER_HF, "%s: ostimer_r: OS Timer Counter Register: %08x & %08x\n", machine().describe_context(), m_ostmr_regs.oscr, mem_mask);
		return m_ostmr_regs.oscr;
	case REG_OSSR:
		LOGMASKED(LOG_OSTIMER, "%s: ostimer_r: OS Timer Status Register: %08x & %08x\n", machine().describe_context(), m_ostmr_regs.ossr, mem_mask);
		return m_ostmr_regs.ossr;
	case REG_OWER:
		LOGMASKED(LOG_OSTIMER, "%s: ostimer_r: OS Timer Watchdog Enable Register: %08x & %08x\n", machine().describe_context(), m_ostmr_regs.ower, mem_mask);
		return m_ostmr_regs.ower;
	case REG_OIER:
		LOGMASKED(LOG_OSTIMER, "%s: ostimer_r: OS Timer Interrupt Enable Register: %08x & %08x\n", machine().describe_context(), m_ostmr_regs.oier, mem_mask);
		return m_ostmr_regs.oier;
	default:
		LOGMASKED(LOG_OSTIMER | LOG_UNKNOWN, "%s: ostimer_r: Unknown address: %08x & %08x\n", machine().describe_context(), OSTMR_BASE_ADDR | (offset << 2), mem_mask);
		return 0;
	}
}

void sa1110_periphs_device::ostimer_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	ostimer_update_count();

	switch (offset)
	{
	case REG_OSMR0:
		LOGMASKED(LOG_OSTIMER_HF, "%s: ostimer_w: OS Timer Match Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ostmr_regs.osmr[0]);
		ostimer_update_match_timer(0);
		break;
	case REG_OSMR1:
		LOGMASKED(LOG_OSTIMER, "%s: ostimer_w: OS Timer Match Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ostmr_regs.osmr[1]);
		ostimer_update_match_timer(1);
		break;
	case REG_OSMR2:
		LOGMASKED(LOG_OSTIMER, "%s: ostimer_w: OS Timer Match Register 2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ostmr_regs.osmr[2]);
		ostimer_update_match_timer(2);
		break;
	case REG_OSMR3:
		LOGMASKED(LOG_OSTIMER, "%s: ostimer_w: OS Timer Match Register 3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ostmr_regs.osmr[3]);
		ostimer_update_match_timer(3);
		break;
	case REG_OSCR:
		LOGMASKED(LOG_OSTIMER, "%s: ostimer_w: OS Timer Counter Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ostmr_regs.oscr);
		m_ostmr_regs.last_count_sync = machine().time();
		for (int channel = 0; channel < 4; channel++)
		{
			if (m_ostmr_regs.oscr == m_ostmr_regs.osmr[channel] && BIT(m_ostmr_regs.oier, channel))
			{
				if (!BIT(m_ostmr_regs.ossr, channel))
				{
					m_ostmr_regs.ossr |= (1 << channel);
					set_irq_line(INT_OSTIMER0 + channel, 1);
				}
			}
			else
			{
				ostimer_update_match_timer(channel);
			}
		}
		break;
	case REG_OSSR:
	{
		LOGMASKED(LOG_OSTIMER_HF, "%s: ostimer_w: OS Timer Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		const uint32_t old = m_ostmr_regs.ossr;
		m_ostmr_regs.ossr &= ~(data & mem_mask);
		if (old != m_ostmr_regs.ossr)
		{
			for (int channel = 0; channel < 4; channel++)
			{
				if (BIT(old, channel))
					set_irq_line(INT_OSTIMER0 + channel, 0);
			}
		}
		break;
	}
	case REG_OWER:
		LOGMASKED(LOG_OSTIMER, "%s: ostimer_w: OS Timer Watchdog Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (!m_ostmr_regs.ower)
		{
			m_ostmr_regs.ower = data & mem_mask & 1;
		}
		break;
	case REG_OIER:
		LOGMASKED(LOG_OSTIMER, "%s: ostimer_w: OS Timer Interrupt Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ostmr_regs.oier);
		break;
	default:
		LOGMASKED(LOG_OSTIMER | LOG_UNKNOWN, "%s: ostimer_w: Unknown address: %08x = %08x & %08x\n", machine().describe_context(), OSTMR_BASE_ADDR | (offset << 2),
			data, mem_mask);
		break;
	}
}

/*

  Intel SA-1110 Real-Time Clock

  pg. 88 to 92 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::rtc_tick_cb)
{
	m_rtc_regs.rcnr++;
	m_rtc_regs.rtsr |= (1 << RTSR_HZ_BIT);

	if (m_rtc_regs.rcnr == m_rtc_regs.rtar)
	{
		m_rtc_regs.rtsr |= (1 << RTSR_AL_BIT);
		if (BIT(m_rtc_regs.rtsr, RTSR_ALE_BIT))
			set_irq_line(INT_RTC_ALARM, 1);
	}

	if (BIT(m_rtc_regs.rtsr, RTSR_HZE_BIT))
		set_irq_line(INT_RTC_TICK, 1);
}

uint32_t sa1110_periphs_device::rtc_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_RTAR:
		LOGMASKED(LOG_RTC, "%s: rtc_r: RTC Alarm Register: %08x & %08x\n", machine().describe_context(), m_rtc_regs.rtar, mem_mask);
		return m_rtc_regs.rtar;
	case REG_RCNR:
		LOGMASKED(LOG_RTC_HF, "%s: rtc_r: RTC Count Register: %08x & %08x\n", machine().describe_context(), m_rtc_regs.rcnr, mem_mask);
		return m_rtc_regs.rcnr;
	case REG_RTTR:
		LOGMASKED(LOG_RTC, "%s: rtc_r: RTC Timer Trim Register: %08x & %08x\n", machine().describe_context(), m_rtc_regs.rttr, mem_mask);
		return m_rtc_regs.rttr;
	case REG_RTSR:
		LOGMASKED(LOG_RTC, "%s: rtc_r: RTC Status Register: %08x & %08x\n", machine().describe_context(), m_rtc_regs.rtsr, mem_mask);
		return m_rtc_regs.rtsr;
	default:
		LOGMASKED(LOG_RTC | LOG_UNKNOWN, "%s: reset_r: Unknown address: %08x & %08x\n", machine().describe_context(), RTC_BASE_ADDR | (offset << 2), mem_mask);
		return 0;
	}
}

void sa1110_periphs_device::rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_RTAR:
		LOGMASKED(LOG_RTC, "%s: rtc_w: RTC Alarm Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_rtc_regs.rtar);
		break;
	case REG_RCNR:
		LOGMASKED(LOG_RTC, "%s: rtc_w: RTC Count Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_rtc_regs.rcnr);
		break;
	case REG_RTTR:
		LOGMASKED(LOG_RTC, "%s: rtc_w: RTC Timer Trim Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_rtc_regs.rttr);
		// TODO: Implement timer trimming
		break;
	case REG_RTSR:
	{
		LOGMASKED(LOG_RTC, "%s: rtc_w: RTC Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);

		const uint32_t old = m_rtc_regs.rtsr;
		const bool old_alarm_int = BIT(old, RTSR_AL_MASK) && BIT(m_rtc_regs.rtsr, RTSR_ALE_MASK);
		const bool old_tick_int = BIT(old, RTSR_HZ_MASK) && BIT(m_rtc_regs.rtsr, RTSR_HZE_MASK);

		m_rtc_regs.rtsr &= ~(data & (RTSR_AL_MASK | RTSR_HZ_MASK) & mem_mask);
		m_rtc_regs.rtsr &= ~(RTSR_ALE_MASK | RTSR_HZE_MASK);
		m_rtc_regs.rtsr |= (data & (RTSR_ALE_MASK | RTSR_HZE_MASK) & mem_mask);

		const bool new_alarm_int = BIT(m_rtc_regs.rtsr, RTSR_AL_MASK) && BIT(m_rtc_regs.rtsr, RTSR_ALE_MASK);
		const bool new_tick_int = BIT(m_rtc_regs.rtsr, RTSR_HZ_MASK) && BIT(m_rtc_regs.rtsr, RTSR_HZE_MASK);

		if (old_alarm_int != new_alarm_int)
			set_irq_line(INT_RTC_ALARM, (int)new_alarm_int);
		if (old_tick_int != new_tick_int)
			set_irq_line(INT_RTC_TICK, (int)new_tick_int);
		break;
	}
	default:
		LOGMASKED(LOG_RTC | LOG_UNKNOWN, "%s: reset_w: Unknown address: %08x = %08x & %08x\n", machine().describe_context(), RTC_BASE_ADDR | (offset << 2),
			data, mem_mask);
		break;
	}
}

/*

  Intel SA-1110 Power Controller

  pg. 104 to 111 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

uint32_t sa1110_periphs_device::power_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_PMCR:
		LOGMASKED(LOG_POWER, "%s: power_r: Power Manager Control Register: %08x & %08x\n", machine().describe_context(), m_power_regs.pmcr, mem_mask);
		return m_power_regs.pmcr;
	case REG_PSSR:
		LOGMASKED(LOG_POWER, "%s: power_r: Power Manager Sleep Status Register: %08x & %08x\n", machine().describe_context(), m_power_regs.pssr, mem_mask);
		return m_power_regs.pssr;
	case REG_PSPR:
		LOGMASKED(LOG_POWER_HF, "%s: power_r: Power Manager Scratch Pad Register: %08x & %08x\n", machine().describe_context(), m_power_regs.pspr, mem_mask);
		return m_power_regs.pspr;
	case REG_PWER:
		LOGMASKED(LOG_POWER, "%s: power_r: Power Manager Wake-up Enable Register: %08x & %08x\n", machine().describe_context(), m_power_regs.pwer, mem_mask);
		return m_power_regs.pwer;
	case REG_PCFR:
		LOGMASKED(LOG_POWER, "%s: power_r: Power Manager General Configuration Register: %08x & %08x\n", machine().describe_context(), m_power_regs.pcfr, mem_mask);
		return m_power_regs.pcfr;
	case REG_PPCR:
		LOGMASKED(LOG_POWER, "%s: power_r: Power Manager PLL Configuration Register: %08x & %08x\n", machine().describe_context(), m_power_regs.ppcr, mem_mask);
		return m_power_regs.ppcr;
	case REG_PGSR:
		LOGMASKED(LOG_POWER, "%s: power_r: Power Manager GPIO Sleep State Register: %08x & %08x\n", machine().describe_context(), m_power_regs.pgsr, mem_mask);
		return m_power_regs.pgsr;
	case REG_POSR:
		LOGMASKED(LOG_POWER, "%s: power_r: Power Manager Oscillator Status Register: %08x & %08x\n", machine().describe_context(), m_power_regs.posr, mem_mask);
		return m_power_regs.posr;
	default:
		LOGMASKED(LOG_POWER | LOG_UNKNOWN, "%s: power_r: Unknown address: %08x & %08x\n", machine().describe_context(), POWER_BASE_ADDR | (offset << 2), mem_mask);
		return 0;
	}
}

void sa1110_periphs_device::power_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_PMCR:
		LOGMASKED(LOG_POWER, "%s: power_w: Power Manager Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_power_regs.pmcr);
		break;
	case REG_PSSR:
		LOGMASKED(LOG_POWER, "%s: power_w: Power Manager Sleep Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_power_regs.pssr &= ~(data & 0x0000001f);
		break;
	case REG_PSPR:
		LOGMASKED(LOG_POWER_HF, "%s: power_w: Power Manager Scratch Pad Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_power_regs.pspr);
		break;
	case REG_PWER:
		LOGMASKED(LOG_POWER, "%s: power_w: Power Manager Wake-Up Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_power_regs.pwer);
		break;
	case REG_PCFR:
		LOGMASKED(LOG_POWER, "%s: power_w: Power Manager General Configuration Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_power_regs.pcfr);
		break;
	case REG_PPCR:
		LOGMASKED(LOG_POWER, "%s: power_w: Power Manager PLL Configuration Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_power_regs.ppcr);
		break;
	case REG_PGSR:
		LOGMASKED(LOG_POWER, "%s: power_w: Power Manager GPIO Sleep State Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_power_regs.pgsr);
		break;
	case REG_POSR:
		LOGMASKED(LOG_POWER, "%s: power_w: Power Manager Oscillator Status Register (ignored) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	default:
		LOGMASKED(LOG_POWER | LOG_UNKNOWN, "%s: power_w: Unknown address: %08x = %08x & %08x\n", machine().describe_context(), POWER_BASE_ADDR | (offset << 2),
			data, mem_mask);
		break;
	}
}

/*

  Intel SA-1110 Reset Controller

  pg. 112 to 114 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

uint32_t sa1110_periphs_device::reset_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_RSRR:
		LOGMASKED(LOG_RESET, "%s: reset_r: Reset Controller Software Reset Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case REG_RCSR:
		LOGMASKED(LOG_RESET, "%s: reset_r: Reset Controller Status Register: %08x & %08x\n", machine().describe_context(), m_rcsr, mem_mask);
		return m_rcsr;
	default:
		LOGMASKED(LOG_RESET | LOG_UNKNOWN, "%s: reset_r: Unknown address: %08x & %08x\n", machine().describe_context(), RESET_BASE_ADDR | (offset << 2), mem_mask);
		return 0;
	}
}

void sa1110_periphs_device::reset_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_RSRR:
		LOGMASKED(LOG_RESET, "%s: reset_w: Reset Controller Software Reset Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_RCSR:
		LOGMASKED(LOG_RESET, "%s: reset_w: Reset Controller Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_rcsr &= ~(data & mem_mask);
		break;
	default:
		LOGMASKED(LOG_RESET | LOG_UNKNOWN, "%s: reset_w: Unknown address: %08x = %08x & %08x\n", machine().describe_context(), RESET_BASE_ADDR | (offset << 2),
			data, mem_mask);
		break;
	}
}

/*

  Intel SA-1110 GPIO Controller

  pg. 71 to 80 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

void sa1110_periphs_device::gpio_in(const uint32_t line, const int state)
{
	const uint32_t mask = (1 << line);
	const uint32_t old_latch = m_gpio_regs.input_latch;
	m_gpio_regs.input_latch &= ~mask;
	m_gpio_regs.input_latch |= (state << line);

	LOGMASKED(LOG_GPIO, "gpio_in: Line %d, state %d\n", line, state);
	if (old_latch != m_gpio_regs.input_latch && !BIT(m_gpio_regs.gafr, line))
	{
		// TODO: The manual is unclear if edge detection functions on both inputs and outputs.
		//       If it can also function on outputs, remove the GPDR check below.
		if (!BIT(m_gpio_regs.gpdr, line) && BIT(m_gpio_regs.any_edge_mask, line))
		{
			const uint32_t old_edge = m_gpio_regs.gedr;
			if (state && BIT(m_gpio_regs.grer, line))
				m_gpio_regs.gedr |= mask;
			if (!state && BIT(m_gpio_regs.gfer, line))
				m_gpio_regs.gedr |= mask;
			if (old_edge != m_gpio_regs.gedr)
				gpio_update_interrupts(mask);
		}

		m_gpio_regs.gplr = (m_gpio_regs.input_latch & ~m_gpio_regs.gafr) | (m_gpio_regs.alt_input_latch & m_gpio_regs.gafr);
		LOGMASKED(LOG_GPIO, "gpio_in: New GPLR: %08x\n", m_gpio_regs.gplr);
	}
}

void sa1110_periphs_device::gpio_update_interrupts(const uint32_t changed_mask)
{
	uint32_t remaining_mask = changed_mask;
	for (uint32_t line = 0; line < 11; line++)
	{
		if (!BIT(remaining_mask, line))
			continue;

		set_irq_line(INT_GPIO0 + line, BIT(m_gpio_regs.gedr, line));
		remaining_mask &= ~(1 << line);
	}

	if (!remaining_mask)
		return;

	set_irq_line(INT_GPIOHI, (m_gpio_regs.gedr & 0x0ffff800) ? 1 : 0);
}

void sa1110_periphs_device::gpio_update_direction(const uint32_t old_gpdr)
{
	const uint32_t new_outputs = ~old_gpdr & m_gpio_regs.gpdr & ~m_gpio_regs.gafr;
	if (new_outputs)
	{
		for (uint32_t line = 0; line < 28; line++)
		{
			if (BIT(new_outputs, line))
			{
				m_gpio_out[line](BIT(m_gpio_regs.gplr, line));
			}
		}
	}

	// TODO: Do we need to check rising/falling edges based on the transition from output to input?
}

void sa1110_periphs_device::gpio_update_outputs(const uint32_t old_latch, const uint32_t changed)
{
	uint32_t remaining_changed = changed;

	for (uint32_t line = 0; line < 28 && remaining_changed != 0; line++)
	{
		if (BIT(remaining_changed, line))
		{
			m_gpio_out[line](BIT(m_gpio_regs.output_latch, line));
			remaining_changed &= ~(1 << line);
		}
	}
}

void sa1110_periphs_device::gpio_update_alternate_pins(const uint32_t changed_mask)
{
	// TODO
}

uint32_t sa1110_periphs_device::gpio_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_GPLR:
		LOGMASKED(LOG_GPIO, "%s: gpio_r: GPIO Pin-Level Register: %08x & %08x\n", machine().describe_context(), m_gpio_regs.gplr, mem_mask);
		return m_gpio_regs.gplr;
	case REG_GPDR:
		LOGMASKED(LOG_GPIO, "%s: gpio_r: GPIO Pin Direction Register: %08x & %08x\n", machine().describe_context(), m_gpio_regs.gpdr, mem_mask);
		return m_gpio_regs.gpdr;
	case REG_GPSR:
		LOGMASKED(LOG_GPIO, "%s: gpio_r: GPIO Pin Output Set Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case REG_GPCR:
		LOGMASKED(LOG_GPIO, "%s: gpio_r: GPIO Pin Output Clear Register (ignored): %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case REG_GRER:
		LOGMASKED(LOG_GPIO, "%s: gpio_r: GPIO Rising-Edge Detect Register: %08x & %08x\n", machine().describe_context(), m_gpio_regs.grer, mem_mask);
		return m_gpio_regs.grer;
	case REG_GFER:
		LOGMASKED(LOG_GPIO, "%s: gpio_r: GPIO Falling-Edge Detect Register: %08x & %08x\n", machine().describe_context(), m_gpio_regs.gfer, mem_mask);
		return m_gpio_regs.gfer;
	case REG_GEDR:
		LOGMASKED(LOG_GPIO, "%s: gpio_r: GPIO Edge Detect Status Register: %08x & %08x\n", machine().describe_context(), m_gpio_regs.gedr, mem_mask);
		return m_gpio_regs.gedr;
	case REG_GAFR:
		LOGMASKED(LOG_GPIO, "%s: gpio_r: GPIO Alternate Function Register: %08x & %08x\n", machine().describe_context(), m_gpio_regs.gafr, mem_mask);
		return m_gpio_regs.gafr;
	default:
		LOGMASKED(LOG_GPIO | LOG_UNKNOWN, "%s: gpio_r: Unknown address: %08x & %08x\n", machine().describe_context(), GPIO_BASE_ADDR | (offset << 2), mem_mask);
		return 0;
	}
}

void sa1110_periphs_device::gpio_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_GPLR:
		LOGMASKED(LOG_GPIO, "%s: gpio_w: GPIO Pin-Level Register (ignored): %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_GPDR:
	{
		LOGMASKED(LOG_GPIO, "%s: gpio_w: GPIO Pin Direction Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		const uint32_t old = m_gpio_regs.gpdr;
		COMBINE_DATA(&m_gpio_regs.gpdr);
		if (old != m_gpio_regs.gpdr)
			gpio_update_direction(old);
		break;
	}
	case REG_GPSR:
	{
		LOGMASKED(LOG_GPIO, "%s: gpio_w: GPIO Pin Output Set Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		const uint32_t old = m_gpio_regs.output_latch;
		m_gpio_regs.output_latch |= (data & mem_mask);
		const uint32_t changed = ((old ^ m_gpio_regs.output_latch) & m_gpio_regs.gpdr) & ~m_gpio_regs.gafr;
		if (changed)
			gpio_update_outputs(old, changed);
		break;
	}
	case REG_GPCR:
	{
		LOGMASKED(LOG_GPIO, "%s: gpio_w: GPIO Pin Output Clear Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		const uint32_t old = m_gpio_regs.output_latch;
		m_gpio_regs.output_latch &= ~(data & mem_mask);
		const uint32_t changed = ((old ^ m_gpio_regs.output_latch) & m_gpio_regs.gpdr) & ~m_gpio_regs.gafr;
		if (changed)
			gpio_update_outputs(old, changed);
		break;
	}
	case REG_GRER:
		LOGMASKED(LOG_GPIO, "%s: gpio_w: GPIO Rising-Edge Detect Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_gpio_regs.grer);
		m_gpio_regs.any_edge_mask = m_gpio_regs.grer | m_gpio_regs.gfer;
		break;
	case REG_GFER:
		LOGMASKED(LOG_GPIO, "%s: gpio_w: GPIO Falling-Edge Detect Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_gpio_regs.gfer);
		m_gpio_regs.any_edge_mask = m_gpio_regs.grer | m_gpio_regs.gfer;
		break;
	case REG_GEDR:
	{
		LOGMASKED(LOG_GPIO, "%s: gpio_w: GPIO Edge Detect Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		const uint32_t old = m_gpio_regs.gedr;
		m_gpio_regs.gedr &= ~(data & mem_mask);
		if (old != m_gpio_regs.gedr)
			gpio_update_interrupts(old ^ m_gpio_regs.gedr);
		break;
	}
	case REG_GAFR:
	{
		LOGMASKED(LOG_GPIO, "%s: gpio_w: GPIO Alternate Function Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		const uint32_t old = m_gpio_regs.gafr;
		COMBINE_DATA(&m_gpio_regs.gafr);
		if (old != m_gpio_regs.gafr)
			gpio_update_alternate_pins(old ^ m_gpio_regs.gafr);
		break;
	}
	default:
		LOGMASKED(LOG_GPIO | LOG_UNKNOWN, "%s: gpio_w: Unknown address: %08x = %08x & %08x\n", machine().describe_context(), GPIO_BASE_ADDR | (offset << 2), data, mem_mask);
		break;
	}
}

/*

  Intel SA-1110 Interrupt Controller

  pg. 81 to 88 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

void sa1110_periphs_device::set_irq_line(uint32_t line, int irq_state)
{
	const uint32_t line_mask = (1 << line);
	const uint32_t old_status = m_intc_regs.icpr;
	m_intc_regs.icpr &= ~line_mask;
	m_intc_regs.icpr |= irq_state ? line_mask : 0;

	if (m_intc_regs.icpr == old_status)
		return;

	update_interrupts();
}

void sa1110_periphs_device::update_interrupts()
{
	const uint32_t old_fiq = m_intc_regs.icfp;
	m_intc_regs.icfp = (m_intc_regs.icpr & m_intc_regs.icmr) & m_intc_regs.iclr;
	if (old_fiq != m_intc_regs.icfp)
	{
		m_maincpu->set_input_line(ARM7_FIRQ_LINE, m_intc_regs.icfp ? ASSERT_LINE : CLEAR_LINE);
	}

	const uint32_t old_irq = m_intc_regs.icip;
	m_intc_regs.icip = (m_intc_regs.icpr & m_intc_regs.icmr) & (~m_intc_regs.iclr);
	if (old_irq != m_intc_regs.icip)
	{
		m_maincpu->set_input_line(ARM7_IRQ_LINE, m_intc_regs.icip ? ASSERT_LINE : CLEAR_LINE);
	}
}

uint32_t sa1110_periphs_device::intc_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_ICIP:
		if (m_intc_regs.icip != 0x04000000)
		{
			LOGMASKED(LOG_INTC, "%s: intc_r: Interrupt Controller IRQ Pending Register: %08x & %08x\n", machine().describe_context(), m_intc_regs.icip, mem_mask);
		}
		return m_intc_regs.icip;
	case REG_ICMR:
		LOGMASKED(LOG_INTC, "%s: intc_r: Interrupt Controller Mask Register: %08x & %08x\n", machine().describe_context(), m_intc_regs.icmr, mem_mask);
		return m_intc_regs.icmr;
	case REG_ICLR:
		LOGMASKED(LOG_INTC, "%s: intc_r: Interrupt Controller Level Register: %08x & %08x\n", machine().describe_context(), m_intc_regs.iclr, mem_mask);
		return m_intc_regs.iclr;
	case REG_ICFP:
		LOGMASKED(LOG_INTC, "%s: intc_r: Interrupt Controller FIQ Pending Register: %08x & %08x\n", machine().describe_context(), m_intc_regs.icfp, mem_mask);
		return m_intc_regs.icfp;
	case REG_ICPR:
		LOGMASKED(LOG_INTC, "%s: intc_r: Interrupt Controller Pending Register: %08x & %08x\n", machine().describe_context(), m_intc_regs.icpr, mem_mask);
		return m_intc_regs.icpr;
	case REG_ICCR:
		LOGMASKED(LOG_INTC, "%s: intc_r: Interrupt Controller Control Register: %08x & %08x\n", machine().describe_context(), m_intc_regs.iccr, mem_mask);
		return m_intc_regs.iccr;
	default:
		LOGMASKED(LOG_INTC | LOG_UNKNOWN, "%s: intc_r: Unknown address: %08x & %08x\n", machine().describe_context(), INTC_BASE_ADDR | (offset << 2), mem_mask);
		return 0;
	}
}

void sa1110_periphs_device::intc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_ICIP:
		LOGMASKED(LOG_INTC, "%s: intc_w: (Invalid Write) Interrupt Controller IRQ Pending Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_ICMR:
	{
		LOGMASKED(LOG_INTC, "%s: intc_w: Interrupt Controller Mask Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		const uint32_t old = m_intc_regs.icmr;
		COMBINE_DATA(&m_intc_regs.icmr);
		if (old != m_intc_regs.icmr)
			update_interrupts();
		break;
	}
	case REG_ICLR:
	{
		LOGMASKED(LOG_INTC, "%s: intc_w: Interrupt Controller Level Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		const uint32_t old = m_intc_regs.iclr;
		COMBINE_DATA(&m_intc_regs.iclr);
		if (old != m_intc_regs.iclr)
			update_interrupts();
		break;
	}
	case REG_ICFP:
		LOGMASKED(LOG_INTC, "%s: intc_w: (Invalid Write) Interrupt Controller FIQ Pending Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_ICPR:
		LOGMASKED(LOG_INTC, "%s: intc_w: (Invalid Write) Interrupt Controller Pending Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case REG_ICCR:
		LOGMASKED(LOG_INTC, "%s: intc_w: Interrupt Controller Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_intc_regs.iccr = BIT(data, 0);
		break;
	default:
		LOGMASKED(LOG_INTC | LOG_UNKNOWN, "%s: intc_w: Unknown address: %08x = %08x & %08x\n", machine().describe_context(), INTC_BASE_ADDR | (offset << 2), data, mem_mask);
		break;
	}
}

/*

  Intel SA-1110 Peripheral Pin Controller

  pg. 347 to 357 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

uint32_t sa1110_periphs_device::ppc_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_PPDR:
		LOGMASKED(LOG_PPC, "%s: ppc_r: PPC Pin Direction Register: %08x & %08x\n", machine().describe_context(), m_ppc_regs.ppdr, mem_mask);
		return m_ppc_regs.ppdr;
	case REG_PPSR:
		LOGMASKED(LOG_PPC, "%s: ppc_r: PPC Pin State Register: %08x & %08x\n", machine().describe_context(), m_ppc_regs.ppsr, mem_mask);
		return m_ppc_regs.ppsr;
	case REG_PPAR:
		LOGMASKED(LOG_PPC, "%s: ppc_r: PPC Pin Assignment Register: %08x & %08x\n", machine().describe_context(), m_ppc_regs.ppar, mem_mask);
		return m_ppc_regs.ppar;
	case REG_PSDR:
		LOGMASKED(LOG_PPC, "%s: ppc_r: PPC Sleep Mode Direction Register: %08x & %08x\n", machine().describe_context(), m_ppc_regs.psdr, mem_mask);
		return m_ppc_regs.psdr;
	case REG_PPFR:
		LOGMASKED(LOG_PPC, "%s: ppc_r: PPC Pin Flag Register: %08x & %08x\n", machine().describe_context(), m_ppc_regs.ppfr, mem_mask);
		return m_ppc_regs.ppfr;
	default:
		LOGMASKED(LOG_PPC | LOG_UNKNOWN, "%s: ppc_r: Unknown address: %08x & %08x\n", machine().describe_context(), PPC_BASE_ADDR | (offset << 2), mem_mask);
		return 0;
	}
}

void sa1110_periphs_device::ppc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case REG_PPDR:
	{
		LOGMASKED(LOG_PPC, "%s: ppc_w: PPC Pin Direction Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ppc_regs.ppdr);
		//const uint32_t old_ppsr = m_ppc_regs.ppsr;
		m_ppc_regs.ppsr = (m_ppc_regs.ppsr_out & m_ppc_regs.ppdr) | (m_ppc_regs.ppsr_in & ~m_ppc_regs.ppdr);
		//const uint32_t changed_states = old_ppsr ^ m_ppc_regs.ppsr;
		//if (changed_states)
		//{
		//}
		break;
	}
	case REG_PPSR:
	{
		LOGMASKED(LOG_PPC, "%s: ppc_w: PPC Pin State Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		//const uint32_t old_latch = m_ppc_regs.ppsr_out;
		COMBINE_DATA(&m_ppc_regs.ppsr_out);
		m_ppc_regs.ppsr = (m_ppc_regs.ppsr_out & m_ppc_regs.ppdr) | (m_ppc_regs.ppsr_in & ~m_ppc_regs.ppdr);
		//const uint32_t changed_outputs = (old ^ m_ppc_regs.ppsr_out) & m_ppc_regs.ppdr;
		//if (changed_outputs)
		//{
			// Do stuff
		//}
		break;
	}
	case REG_PPAR:
		LOGMASKED(LOG_PPC, "%s: ppc_w: PPC Pin Assignment Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ppc_regs.ppar);
		break;
	case REG_PSDR:
		LOGMASKED(LOG_PPC, "%s: ppc_w: PPC Sleep Mode Direction Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ppc_regs.psdr);
		break;
	case REG_PPFR:
		LOGMASKED(LOG_PPC, "%s: ppc_w: PPC Pin Flag Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ppc_regs.ppfr);
		break;
	default:
		LOGMASKED(LOG_PPC | LOG_UNKNOWN, "%s: ppc_w: Unknown address: %08x = %08x & %08x\n", machine().describe_context(), PPC_BASE_ADDR | (offset << 2), data, mem_mask);
		break;
	}
}

/*

  Intel SA-1110 Peripheral Pin Controller

  pg. 186 to 194 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

void sa1110_periphs_device::dma_set_control_bits(int channel, uint32_t bits)
{
	dma_regs &regs = m_dma_regs[channel];
	const uint32_t old = regs.dsr;
	regs.dsr |= bits;
	const uint32_t newly_set = ~old & bits;
	if (newly_set == 0)
		return;

	const uint32_t irq_mask = (1 << DSR_ERROR_BIT) | (1 << DSR_DONEA_BIT) | (1 << DSR_DONEB_BIT);

	if (BIT(newly_set, DSR_RUN_BIT))
		regs.dsr &= ~(1 << DSR_ERROR_BIT);
	if (BIT(newly_set, DSR_DONEA_BIT) || BIT(newly_set, DSR_STRTA_BIT))
		regs.dsr &= ~(1 << DSR_DONEA_BIT);
	if (BIT(newly_set, DSR_DONEB_BIT) || BIT(newly_set, DSR_STRTB_BIT))
		regs.dsr &= ~(1 << DSR_DONEB_BIT);

	if (regs.ddar == 0x81400580 && BIT(regs.dsr, DSR_RUN_BIT))
	{
		const uint32_t buf = BIT(regs.dsr, DSR_BIU_BIT);
		const uint32_t count = regs.dbt[buf];
		if (count)
		{
			const uint32_t start_mask = (buf ? (1 << DSR_STRTB_BIT) : (1 << DSR_STRTA_BIT));
			const uint32_t done_mask = (buf ? (1 << DSR_DONEB_BIT) : (1 << DSR_DONEA_BIT));
			const uint32_t addr = regs.dbs[buf];
			address_space &space = m_maincpu->space(AS_PROGRAM);
			if (regs.dsr & start_mask)
			{
				for (uint32_t i = 0; i < count; i++)
				{
					const uint8_t value = space.read_byte(addr + i);
					if (value == 0x0d || value == 0x0a || (value >= 0x20 && value < 0x7f))
					{
						printf("%c", (char)value);
					}
				}
				printf("\n");
				regs.dsr &= ~start_mask;
				regs.dsr |= done_mask;
				regs.dsr ^= (1 << DSR_BIU_BIT);
			}
		}
	}

	set_irq_line(INT_DMA0 + channel, (BIT(regs.dsr, DSR_IE_BIT) && (regs.dsr & irq_mask)) ? 1 : 0);
}

void sa1110_periphs_device::dma_clear_control_bits(int channel, uint32_t bits)
{
	dma_regs &regs = m_dma_regs[channel];

	const uint32_t irq_mask = (1 << DSR_ERROR_BIT) | (1 << DSR_DONEA_BIT) | (1 << DSR_DONEB_BIT);

	regs.dsr &= ~bits;
	set_irq_line(INT_DMA0 + channel, (BIT(regs.dsr, DSR_IE_BIT) && (regs.dsr & irq_mask)) ? 1 : 0);
}

uint32_t sa1110_periphs_device::dma_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t channel = (offset >> 3) & 7;
	if (channel < 6)
	{
		switch (offset & 7)
		{
		case REG_DDAR:
			LOGMASKED(LOG_DMA, "%s: dma_r: DMA%d Device Address Register: %08x & %08x\n", machine().describe_context(), channel, m_dma_regs[channel].ddar, mem_mask);
			return m_dma_regs[channel].ddar;
		case REG_DSSR:
		case REG_DCSR:
		case REG_DSR:
			LOGMASKED(LOG_DMA, "%s: dma_r: DMA%d Control/Status Register: %08x & %08x\n", machine().describe_context(), channel, m_dma_regs[channel].dsr, mem_mask);
			return m_dma_regs[channel].dsr;
		case REG_DBSA:
			LOGMASKED(LOG_DMA, "%s: dma_r: DMA%d Buffer A Start Address: %08x & %08x\n", machine().describe_context(), channel, m_dma_regs[channel].dbs[0], mem_mask);
			return m_dma_regs[channel].dbs[0];
		case REG_DBTA:
			LOGMASKED(LOG_DMA, "%s: dma_r: DMA%d Buffer A Transfer Count: %08x & %08x\n", machine().describe_context(), channel, m_dma_regs[channel].dbt[0], mem_mask);
			return m_dma_regs[channel].dbt[0];
		case REG_DBSB:
			LOGMASKED(LOG_DMA, "%s: dma_r: DMA%d Buffer B Start Address: %08x & %08x\n", machine().describe_context(), channel, m_dma_regs[channel].dbs[1], mem_mask);
			return m_dma_regs[channel].dbs[1];
		case REG_DBTB:
			LOGMASKED(LOG_DMA, "%s: dma_r: DMA%d Buffer B Transfer Count: %08x & %08x\n", machine().describe_context(), channel, m_dma_regs[channel].dbt[1], mem_mask);
			return m_dma_regs[channel].dbt[1];
		default:
			LOGMASKED(LOG_DMA | LOG_UNKNOWN, "%s: dma_r: Unknown address: %08x & %08x\n", machine().describe_context(), DMA_BASE_ADDR | (offset << 2), mem_mask);
			return 0;
		}
	}
	return 0;
}

void sa1110_periphs_device::dma_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t channel = (offset >> 3) & 7;
	if (channel < 6)
	{
		switch (offset & 7)
		{
		case REG_DDAR:
			LOGMASKED(LOG_DMA, "%s: dma_w: DMA%d Device Address Register = %08x & %08x\n", machine().describe_context(), channel, data, mem_mask);
			COMBINE_DATA(&m_dma_regs[channel].ddar);
			break;
		case REG_DSSR:
			LOGMASKED(LOG_DMA, "%s: dma_w: DMA%d Control/Status Register (1S) = %08x & %08x\n", machine().describe_context(), channel, data, mem_mask);
			LOGMASKED(LOG_DMA, "%s:        Run Set: %d\n", machine().describe_context(), BIT(data, DSR_RUN_BIT));
			LOGMASKED(LOG_DMA, "%s:        Interrupt Enable Set: %d\n", machine().describe_context(), BIT(data, DSR_IE_BIT));
			LOGMASKED(LOG_DMA, "%s:        Error Set: %d\n", machine().describe_context(), BIT(data, DSR_ERROR_BIT));
			LOGMASKED(LOG_DMA, "%s:        Done A Set: %d\n", machine().describe_context(), BIT(data, DSR_DONEA_BIT));
			LOGMASKED(LOG_DMA, "%s:        Start A Set: %d\n", machine().describe_context(), BIT(data, DSR_STRTA_BIT));
			LOGMASKED(LOG_DMA, "%s:        Done B Set: %d\n", machine().describe_context(), BIT(data, DSR_DONEB_BIT));
			LOGMASKED(LOG_DMA, "%s:        Start B Set: %d\n", machine().describe_context(), BIT(data, DSR_STRTB_BIT));
			LOGMASKED(LOG_DMA, "%s:        Buffer In Use Set: %d\n", machine().describe_context(), BIT(data, DSR_BIU_BIT));
			dma_set_control_bits(channel, data & mem_mask);
			break;
		case REG_DCSR:
			LOGMASKED(LOG_DMA, "%s: dma_w: DMA%d Control/Status Register (1C) = %08x & %08x\n", machine().describe_context(), channel, data, mem_mask);
			LOGMASKED(LOG_DMA, "%s:        Run Clear: %d\n", machine().describe_context(), BIT(data, DSR_RUN_BIT));
			LOGMASKED(LOG_DMA, "%s:        Interrupt Enable Clear: %d\n", machine().describe_context(), BIT(data, DSR_IE_BIT));
			LOGMASKED(LOG_DMA, "%s:        Error Clear: %d\n", machine().describe_context(), BIT(data, DSR_ERROR_BIT));
			LOGMASKED(LOG_DMA, "%s:        Done A Clear: %d\n", machine().describe_context(), BIT(data, DSR_DONEA_BIT));
			LOGMASKED(LOG_DMA, "%s:        Start A Clear: %d\n", machine().describe_context(), BIT(data, DSR_STRTA_BIT));
			LOGMASKED(LOG_DMA, "%s:        Done B Clear: %d\n", machine().describe_context(), BIT(data, DSR_DONEB_BIT));
			LOGMASKED(LOG_DMA, "%s:        Start B Clear: %d\n", machine().describe_context(), BIT(data, DSR_STRTB_BIT));
			LOGMASKED(LOG_DMA, "%s:        Buffer In Use Clear: %d\n", machine().describe_context(), BIT(data, DSR_BIU_BIT));
			dma_clear_control_bits(channel, data & mem_mask);
			break;
		case REG_DSR:
			LOGMASKED(LOG_DMA, "%s: dma_w: DMA%d Control/Status Register (RO) = %08x & %08x\n", machine().describe_context(), channel, data, mem_mask);
			break;
		case REG_DBSA:
			LOGMASKED(LOG_DMA, "%s: dma_w: DMA%d Buffer A Start Address = %08x & %08x\n", machine().describe_context(), channel, data, mem_mask);
			if (!BIT(m_dma_regs[channel].dsr, DSR_STRTA_BIT))
				COMBINE_DATA(&m_dma_regs[channel].dbs[0]);
			break;
		case REG_DBTA:
			LOGMASKED(LOG_DMA, "%s: dma_w: DMA%d Buffer A Transfer Count = %08x & %08x\n", machine().describe_context(), channel, data, mem_mask);
			if (!BIT(m_dma_regs[channel].dsr, DSR_STRTA_BIT))
			{
				COMBINE_DATA(&m_dma_regs[channel].dbt[0]);
				m_dma_regs[channel].dbt[0] &= DBT_MASK;
			}
			break;
		case REG_DBSB:
			LOGMASKED(LOG_DMA, "%s: dma_w: DMA%d Buffer B Start Address = %08x & %08x\n", machine().describe_context(), channel, data, mem_mask);
			if (!BIT(m_dma_regs[channel].dsr, DSR_STRTB_BIT))
				COMBINE_DATA(&m_dma_regs[channel].dbs[1]);
			break;
		case REG_DBTB:
			LOGMASKED(LOG_DMA, "%s: dma_w: DMA%d Buffer B Transfer Count = %08x & %08x\n", machine().describe_context(), channel, data, mem_mask);
			if (!BIT(m_dma_regs[channel].dsr, DSR_STRTB_BIT))
			{
				COMBINE_DATA(&m_dma_regs[channel].dbt[1]);
				m_dma_regs[channel].dbt[1] &= DBT_MASK;
			}
			break;
		default:
			LOGMASKED(LOG_DMA | LOG_UNKNOWN, "%s: dma_w: Unknown address %08x = %08x & %08x\n", machine().describe_context(), DMA_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
		}
	}
}

void sa1110_periphs_device::device_start()
{
	save_item(NAME(m_udc_regs.udccr));
	save_item(NAME(m_udc_regs.udcar));
	save_item(NAME(m_udc_regs.udcomp));
	save_item(NAME(m_udc_regs.udcimp));
	save_item(NAME(m_udc_regs.udccs0));
	save_item(NAME(m_udc_regs.udccs1));
	save_item(NAME(m_udc_regs.udccs2));
	save_item(NAME(m_udc_regs.udcwc));
	save_item(NAME(m_udc_regs.udcsr));

	save_item(NAME(m_icp_regs.uart.utcr));
	save_item(NAME(m_icp_regs.uart.utsr0));
	save_item(NAME(m_icp_regs.uart.utsr1));
	save_item(NAME(m_icp_regs.uart.rx_fifo));
	save_item(NAME(m_icp_regs.uart.rx_fifo_read_idx));
	save_item(NAME(m_icp_regs.uart.rx_fifo_write_idx));
	save_item(NAME(m_icp_regs.uart.rx_fifo_count));
	m_icp_regs.uart_rx_timer = timer_alloc(FUNC(sa1110_periphs_device::icp_rx_callback), this);
	save_item(NAME(m_icp_regs.uart.tx_fifo));
	save_item(NAME(m_icp_regs.uart.tx_fifo_read_idx));
	save_item(NAME(m_icp_regs.uart.tx_fifo_write_idx));
	save_item(NAME(m_icp_regs.uart.tx_fifo_count));
	m_icp_regs.uart_tx_timer = timer_alloc(FUNC(sa1110_periphs_device::icp_tx_callback), this);
	save_item(NAME(m_icp_regs.uart.rx_break_interlock));

	save_item(NAME(m_icp_regs.utcr4));
	save_item(NAME(m_icp_regs.hssp.hscr0));
	save_item(NAME(m_icp_regs.hssp.hscr1));
	save_item(NAME(m_icp_regs.hssp.hssr0));
	save_item(NAME(m_icp_regs.hssp.hssr1));
	save_item(NAME(m_icp_regs.hssp.rx_fifo));
	save_item(NAME(m_icp_regs.hssp.rx_fifo_read_idx));
	save_item(NAME(m_icp_regs.hssp.rx_fifo_write_idx));
	save_item(NAME(m_icp_regs.hssp.rx_fifo_count));
	m_icp_regs.hssp.rx_timer = timer_alloc(FUNC(sa1110_periphs_device::hssp_rx_callback), this);
	save_item(NAME(m_icp_regs.hssp.tx_fifo));
	save_item(NAME(m_icp_regs.hssp.tx_fifo_read_idx));
	save_item(NAME(m_icp_regs.hssp.tx_fifo_write_idx));
	save_item(NAME(m_icp_regs.hssp.tx_fifo_count));
	m_icp_regs.hssp.tx_timer = timer_alloc(FUNC(sa1110_periphs_device::hssp_tx_callback), this);

	save_item(NAME(m_uart_regs.utcr));
	save_item(NAME(m_uart_regs.utsr0));
	save_item(NAME(m_uart_regs.utsr1));
	save_item(NAME(m_uart_regs.rx_fifo));
	save_item(NAME(m_uart_regs.rx_fifo_read_idx));
	save_item(NAME(m_uart_regs.rx_fifo_write_idx));
	save_item(NAME(m_uart_regs.rx_fifo_count));
	save_item(NAME(m_uart_regs.tx_fifo));
	save_item(NAME(m_uart_regs.tx_fifo_read_idx));
	save_item(NAME(m_uart_regs.tx_fifo_write_idx));
	save_item(NAME(m_uart_regs.tx_fifo_count));
	save_item(NAME(m_uart_regs.rx_break_interlock));

	save_item(NAME(m_mcp_regs.mccr0));
	save_item(NAME(m_mcp_regs.mccr1));
	save_item(NAME(m_mcp_regs.mcdr2));
	save_item(NAME(m_mcp_regs.mcsr));
	save_item(NAME(m_mcp_regs.audio_rx_fifo));
	save_item(NAME(m_mcp_regs.audio_rx_fifo_read_idx));
	save_item(NAME(m_mcp_regs.audio_rx_fifo_write_idx));
	save_item(NAME(m_mcp_regs.audio_rx_fifo_count));
	save_item(NAME(m_mcp_regs.audio_tx_fifo));
	save_item(NAME(m_mcp_regs.audio_tx_fifo_read_idx));
	save_item(NAME(m_mcp_regs.audio_tx_fifo_write_idx));
	save_item(NAME(m_mcp_regs.audio_tx_fifo_count));
	m_mcp_regs.audio_tx_timer = timer_alloc(FUNC(sa1110_periphs_device::mcp_audio_tx_callback), this);
	save_item(NAME(m_mcp_regs.telecom_rx_fifo));
	save_item(NAME(m_mcp_regs.telecom_rx_fifo_read_idx));
	save_item(NAME(m_mcp_regs.telecom_rx_fifo_write_idx));
	save_item(NAME(m_mcp_regs.telecom_rx_fifo_count));
	save_item(NAME(m_mcp_regs.telecom_tx_fifo));
	save_item(NAME(m_mcp_regs.telecom_tx_fifo_read_idx));
	save_item(NAME(m_mcp_regs.telecom_tx_fifo_write_idx));
	save_item(NAME(m_mcp_regs.telecom_tx_fifo_count));
	m_mcp_regs.telecom_tx_timer = timer_alloc(FUNC(sa1110_periphs_device::mcp_telecom_tx_callback), this);

	save_item(NAME(m_ssp_regs.sscr0));
	save_item(NAME(m_ssp_regs.sscr1));
	save_item(NAME(m_ssp_regs.sssr));
	save_item(NAME(m_ssp_regs.rx_fifo));
	save_item(NAME(m_ssp_regs.rx_fifo_read_idx));
	save_item(NAME(m_ssp_regs.rx_fifo_write_idx));
	save_item(NAME(m_ssp_regs.rx_fifo_count));
	m_ssp_regs.rx_timer = timer_alloc(FUNC(sa1110_periphs_device::ssp_rx_callback), this);
	save_item(NAME(m_ssp_regs.tx_fifo));
	save_item(NAME(m_ssp_regs.tx_fifo_read_idx));
	save_item(NAME(m_ssp_regs.tx_fifo_write_idx));
	save_item(NAME(m_ssp_regs.tx_fifo_count));
	m_ssp_regs.tx_timer = timer_alloc(FUNC(sa1110_periphs_device::ssp_tx_callback), this);

	save_item(NAME(m_ostmr_regs.osmr));
	save_item(NAME(m_ostmr_regs.oscr));
	save_item(NAME(m_ostmr_regs.ossr));
	save_item(NAME(m_ostmr_regs.ower));
	save_item(NAME(m_ostmr_regs.oier));
	for (int i = 0; i < 4; i++)
	{
		m_ostmr_regs.timer[i] = timer_alloc(FUNC(sa1110_periphs_device::ostimer_tick_cb), this);
	}

	save_item(NAME(m_rtc_regs.rtar));
	save_item(NAME(m_rtc_regs.rcnr));
	save_item(NAME(m_rtc_regs.rttr));
	save_item(NAME(m_rtc_regs.rtsr));
	m_rtc_regs.tick_timer = timer_alloc(FUNC(sa1110_periphs_device::rtc_tick_cb), this);

	save_item(NAME(m_power_regs.pmcr));
	save_item(NAME(m_power_regs.pssr));
	save_item(NAME(m_power_regs.pspr));
	save_item(NAME(m_power_regs.pwer));
	save_item(NAME(m_power_regs.pcfr));
	save_item(NAME(m_power_regs.ppcr));
	save_item(NAME(m_power_regs.pgsr));
	save_item(NAME(m_power_regs.posr));

	save_item(NAME(m_rcsr));

	save_item(NAME(m_gpio_regs.gplr));
	save_item(NAME(m_gpio_regs.gpdr));
	save_item(NAME(m_gpio_regs.grer));
	save_item(NAME(m_gpio_regs.gfer));
	save_item(NAME(m_gpio_regs.gedr));
	save_item(NAME(m_gpio_regs.gafr));
	save_item(NAME(m_gpio_regs.any_edge_mask));
	save_item(NAME(m_gpio_regs.output_latch));
	save_item(NAME(m_gpio_regs.input_latch));
	save_item(NAME(m_gpio_regs.alt_output_latch));
	save_item(NAME(m_gpio_regs.alt_input_latch));

	save_item(NAME(m_intc_regs.icip));
	save_item(NAME(m_intc_regs.icmr));
	save_item(NAME(m_intc_regs.iclr));
	save_item(NAME(m_intc_regs.iccr));
	save_item(NAME(m_intc_regs.icfp));
	save_item(NAME(m_intc_regs.icpr));

	save_item(NAME(m_ppc_regs.ppdr));
	save_item(NAME(m_ppc_regs.ppsr));
	save_item(NAME(m_ppc_regs.ppar));
	save_item(NAME(m_ppc_regs.psdr));
	save_item(NAME(m_ppc_regs.ppfr));

	save_item(STRUCT_MEMBER(m_dma_regs, ddar));
	save_item(STRUCT_MEMBER(m_dma_regs, dsr));
	save_item(STRUCT_MEMBER(m_dma_regs, dbs));
	save_item(STRUCT_MEMBER(m_dma_regs, dbt));
	save_item(NAME(m_dma_active_mask));

	m_gpio_out.resolve_all_safe();
	m_ssp_out.resolve_safe();
	m_uart3_tx_out.resolve_safe();
}

void sa1110_periphs_device::device_reset()
{
	m_udc_regs.udccr = (1 << UDCCR_SUSM_BIT) | (1 << UDCCR_UDD_BIT);
	m_udc_regs.udcar = 0;
	m_udc_regs.udcomp = 8;
	m_udc_regs.udcimp = 8;
	m_udc_regs.udccs0 = 0;
	m_udc_regs.udccs1 = 0;
	m_udc_regs.udccs2 = 0;
	m_udc_regs.udcwc = 0;
	m_udc_regs.udcsr = 0;

	// init ICP
	std::fill_n(&m_icp_regs.uart.utcr[0], 4, 0);
	m_icp_regs.uart.utsr0 = 0;
	m_icp_regs.uart.utsr1 = 0;
	std::fill_n(&m_icp_regs.uart.rx_fifo[0], 12, 0);
	m_icp_regs.uart.rx_fifo_read_idx = 0;
	m_icp_regs.uart.rx_fifo_write_idx = 0;
	m_icp_regs.uart.rx_fifo_count = 0;
	m_icp_regs.uart_rx_timer->adjust(attotime::never);
	std::fill_n(&m_icp_regs.uart.tx_fifo[0], 8, 0);
	m_icp_regs.uart.tx_fifo_read_idx = 0;
	m_icp_regs.uart.tx_fifo_write_idx = 0;
	m_icp_regs.uart.tx_fifo_count = 0;
	m_icp_regs.uart_tx_timer->adjust(attotime::never);
	m_icp_regs.uart.rx_break_interlock = false;

	m_icp_regs.utcr4 = 0;
	m_icp_regs.hssp.hscr0 = 0;
	m_icp_regs.hssp.hscr1 = 0;
	m_icp_regs.hssp.hssr0 = 0;
	m_icp_regs.hssp.hssr1 = 0;
	std::fill_n(&m_icp_regs.hssp.rx_fifo[0], 4, 0);
	m_icp_regs.hssp.rx_fifo_read_idx = 0;
	m_icp_regs.hssp.rx_fifo_write_idx = 0;
	m_icp_regs.hssp.rx_fifo_count = 0;
	m_icp_regs.hssp.rx_timer->adjust(attotime::never);
	std::fill_n(&m_icp_regs.hssp.tx_fifo[0], 12, 0);
	m_icp_regs.hssp.tx_fifo_read_idx = 0;
	m_icp_regs.hssp.tx_fifo_write_idx = 0;
	m_icp_regs.hssp.tx_fifo_count = 0;
	m_icp_regs.hssp.tx_timer->adjust(attotime::never);

	// init UART3
	std::fill_n(&m_uart_regs.utcr[0], 4, 0);
	m_uart_regs.utsr0 = 0;
	m_uart_regs.utsr1 = 0;
	std::fill_n(&m_uart_regs.rx_fifo[0], 12, 0);
	m_uart_regs.rx_fifo_read_idx = 0;
	m_uart_regs.rx_fifo_write_idx = 0;
	m_uart_regs.rx_fifo_count = 0;
	std::fill_n(&m_uart_regs.tx_fifo[0], 8, 0);
	m_uart_regs.tx_fifo_read_idx = 0;
	m_uart_regs.tx_fifo_write_idx = 0;
	m_uart_regs.tx_fifo_count = 0;
	m_uart_regs.rx_break_interlock = false;

	transmit_register_reset();
	receive_register_reset();

	// init MCP regs
	m_mcp_regs.mccr0 = 0;
	m_mcp_regs.mccr1 = 0;
	m_mcp_regs.mcdr2 = 0;
	m_mcp_regs.mcsr = (1 << MCSR_ANF_BIT) | (1 << MCSR_TNF_BIT);
	std::fill(std::begin(m_mcp_regs.audio_rx_fifo), std::end(m_mcp_regs.audio_rx_fifo), 0);
	m_mcp_regs.audio_rx_fifo_read_idx = 0;
	m_mcp_regs.audio_rx_fifo_write_idx = 0;
	m_mcp_regs.audio_rx_fifo_count = 0;
	std::fill(std::begin(m_mcp_regs.audio_tx_fifo), std::end(m_mcp_regs.audio_tx_fifo), 0);
	m_mcp_regs.audio_tx_fifo_read_idx = 0;
	m_mcp_regs.audio_tx_fifo_write_idx = 0;
	m_mcp_regs.audio_tx_fifo_count = 0;
	m_mcp_regs.audio_tx_timer->adjust(attotime::never);
	std::fill(std::begin(m_mcp_regs.telecom_rx_fifo), std::end(m_mcp_regs.telecom_rx_fifo), 0);
	m_mcp_regs.telecom_rx_fifo_read_idx = 0;
	m_mcp_regs.telecom_rx_fifo_write_idx = 0;
	m_mcp_regs.telecom_rx_fifo_count = 0;
	std::fill(std::begin(m_mcp_regs.telecom_tx_fifo), std::end(m_mcp_regs.telecom_tx_fifo), 0);
	m_mcp_regs.telecom_tx_fifo_read_idx = 0;
	m_mcp_regs.telecom_tx_fifo_write_idx = 0;
	m_mcp_regs.telecom_tx_fifo_count = 0;
	m_mcp_regs.telecom_tx_timer->adjust(attotime::never);

	// init SSP regs
	m_ssp_regs.sscr0 = 0;
	m_ssp_regs.sscr1 = 0;
	m_ssp_regs.sssr = (1 << SSSR_TNF_BIT);
	std::fill(std::begin(m_ssp_regs.rx_fifo), std::end(m_ssp_regs.rx_fifo), 0);
	m_ssp_regs.rx_fifo_read_idx = 0;
	m_ssp_regs.rx_fifo_write_idx = 0;
	m_ssp_regs.rx_fifo_count = 0;
	m_ssp_regs.rx_timer->adjust(attotime::never);
	std::fill(std::begin(m_ssp_regs.tx_fifo), std::end(m_ssp_regs.tx_fifo), 0);
	m_ssp_regs.tx_fifo_read_idx = 0;
	m_ssp_regs.tx_fifo_write_idx = 0;
	m_ssp_regs.tx_fifo_count = 0;
	m_ssp_regs.tx_timer->adjust(attotime::never);

	// init OS timers
	std::fill_n(&m_ostmr_regs.osmr[0], 4, 0);
	m_ostmr_regs.ower = 0;
	m_ostmr_regs.ossr = 0;
	m_ostmr_regs.oier = 0;
	for (int i = 0; i < 4; i++)
	{
		m_ostmr_regs.timer[i]->adjust(attotime::never);
	}
	m_ostmr_regs.last_count_sync = attotime::zero;

	// init RTC
	m_rtc_regs.rtar = 0;
	m_rtc_regs.rcnr = 0;
	m_rtc_regs.rttr = 0;
	m_rtc_regs.rtsr = 0;
	m_rtc_regs.tick_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	// init power regs
	m_power_regs.pmcr = 0;
	m_power_regs.pssr = 0;
	m_power_regs.pspr = 0;
	m_power_regs.pwer = 0;
	m_power_regs.pcfr = 0;
	m_power_regs.ppcr = 0;
	m_power_regs.pgsr = 0;
	m_power_regs.posr = 1; // flag oscillator OK

	// init PPC regs
	m_ppc_regs.ppdr = 0;
	m_ppc_regs.ppsr = 0;
	m_ppc_regs.ppar = 0;
	m_ppc_regs.psdr = 0x003fffff;
	m_ppc_regs.ppfr = 0x0007f001;

	// init DMA regs
	for (dma_regs &regs : m_dma_regs)
	{
		regs.ddar = 0;
		regs.dsr = 0;
		std::fill_n(&regs.dbs[0], 2, 0);
		std::fill_n(&regs.dbt[0], 2, 0);
	}

	m_rcsr = 0x00000001; // indicate hardware reset

	m_gpio_regs.gplr = 0;
	m_gpio_regs.gpdr = 0;
	m_gpio_regs.grer = 0;
	m_gpio_regs.gfer = 0;
	m_gpio_regs.gedr = 0;
	m_gpio_regs.gafr = 0;
	m_gpio_regs.any_edge_mask = 0;
	m_gpio_regs.output_latch = 0;
	m_gpio_regs.input_latch = 0;
	m_gpio_regs.alt_output_latch = 0;
	m_gpio_regs.alt_input_latch = 0;

	m_intc_regs.icip = 0;
	m_intc_regs.icmr = 0;
	m_intc_regs.iclr = 0;
	m_intc_regs.iccr = 0;
	m_intc_regs.icfp = 0;
	m_intc_regs.icpr = 0;

	uart_check_rx_fifo_service();
	uart_check_tx_fifo_service();
}

void sa1110_periphs_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_uart3_irqs).output_handler().set(FUNC(sa1110_periphs_device::uart3_irq_callback));
	INPUT_MERGER_ANY_HIGH(config, m_mcp_irqs).output_handler().set(FUNC(sa1110_periphs_device::mcp_irq_callback));
}
