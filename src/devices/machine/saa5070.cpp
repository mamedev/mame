// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    SAA5070 Viewdata Input/Output Peripheral (VIOP)

    https://bitsavers.org/components/philips/_dataBooks/1985_IC02Na_Philips_Bipolar_MOS_Video_and_Associated_Systems.pdf

    TODO:
    - tape UART
    - IBUS receiver/transmitter

*********************************************************************/

#include "emu.h"
#include "saa5070.h"


DEFINE_DEVICE_TYPE(SAA5070, saa5070_device, "saa5070", "SAA5070 Viewdata Input/Output Peripheral")
DEFINE_DEVICE_TYPE(SAA5070_UART, saa5070_uart_device, "saa5070_uart", "SAA5070 UART")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

saa5070_uart_device::saa5070_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SAA5070_UART, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_txd_handler(*this)
	, m_rx_enabled(0)
	, m_tx_enabled(0)
	, m_rxd(1)
{
}


void saa5070_uart_device::device_start()
{
	receive_register_reset();
	transmit_register_reset();

	save_item(NAME(m_status));
}

void saa5070_uart_device::device_reset()
{
	m_status = 0x80; // TXRDY
}


void saa5070_uart_device::set_baud_rate(int rxbaud, int txbaud, int parity)
{
	if (parity)
		set_data_frame(1, 7, (parity == 1) ? PARITY_ODD : PARITY_EVEN, STOP_BITS_1);
	else
		set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);

	set_rcv_rate(rxbaud);
	set_tra_rate(txbaud);

	receive_register_reset();
	transmit_register_reset();
}

void saa5070_uart_device::rcv_callback()
{
	if (m_rx_enabled) // UART enabled
		receive_register_update_bit(m_rxd);
}

void saa5070_uart_device::rcv_complete()
{
	receive_register_extract();

	m_status |= 0x40; // RXRDY

	if (is_receive_framing_error())
		m_status |= 0x10; // FERR
	else
		m_status &= ~0x10;

	if (is_receive_parity_error())
		m_status |= 0x08; // PERR
	else
		m_status &= ~0x08;
}

void saa5070_uart_device::tra_callback()
{
	if (m_tx_enabled) // UART enabled
		m_txd_handler(transmit_register_get_data_bit());
}

void saa5070_uart_device::tra_complete()
{
	m_status |= 0x80; // TXRDY
}


void saa5070_uart_device::write_rxd(int state)
{
	m_rxd = state;
	device_serial_interface::rx_w(state);
}

void saa5070_uart_device::write_dcd(int state)
{
	if (state)
		m_status |= 0x24; // DCD/IDCD
	else
		m_status &= ~0x24;
}

void saa5070_uart_device::tx_byte(uint8_t data)
{
	m_status &= ~0x80; // TXRDY

	transmit_register_setup(data);
}

uint8_t saa5070_uart_device::rx_byte()
{
	m_status &= ~0x40; // RXRDY

	return get_received_char();
}


saa5070_device::saa5070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SAA5070, tag, owner, clock)
	, m_line_uart(*this, "line_uart")
	, m_tape_uart(*this, "tape_uart")
	, m_in_a_handler(*this, 0x00)
	, m_in_b_handler(*this, 0x00)
	, m_out_a_handler(*this)
	, m_out_b_handler(*this)
	, m_imp_handler(*this)
	, m_txdata_handler(*this)
	, m_timeout_timer(nullptr)
	, m_dial_timer(nullptr)
{
}


void saa5070_device::device_start()
{
	memset(m_sr, 0, sizeof(m_sr));

	m_timeout_timer = timer_alloc(FUNC(saa5070_device::timeout), this);
	m_dial_timer = timer_alloc(FUNC(saa5070_device::dial), this);

	save_item(NAME(m_index));
	save_item(NAME(m_sr));
}

void saa5070_device::device_reset()
{
	write_direct(0, 0x81); // Status 0: LTXRDY, IBTXRDY
	write_direct(1, 0x80); // Status 1: TTXRDY
	write_direct(2, 0x00); // Mode
	write_direct(3, 0x00); // Command
	write_direct(6, 0x00); // Port A
	write_direct(7, 0x00); // Port B
	write_direct(8, 0x00); // Dial Control and Timing
}


void saa5070_device::device_add_mconfig(machine_config &config)
{
	SAA5070_UART(config, m_line_uart, DERIVED_CLOCK(1, 1));
	m_line_uart->txd_handler().set(FUNC(saa5070_device::write_txdata));
	SAA5070_UART(config, m_tape_uart, DERIVED_CLOCK(1, 1));
}


TIMER_CALLBACK_MEMBER(saa5070_device::timeout)
{
	m_sr[8] &= ~0x10; // 1.5s TIMER
	m_sr[8] &= ~0x40; // 60s TIMER
}

TIMER_CALLBACK_MEMBER(saa5070_device::dial)
{
	m_sr[8] &= ~0x20; // DIAL GO
}


void saa5070_device::address_w(uint8_t data)
{
	m_index = data & 0x0f;
}

uint8_t saa5070_device::data_r()
{
	switch (m_index)
	{
	case 0x0: // Status Register 0
		// b0 IBTXRDY - IBUS B transmitter ready - previous transmission complete
		// b1 IBRXRDY - IBUS B receiver ready - data available
		// b2 LIDCD   - line instantaneous data carrier detect
		// b3 LPERR   - line receiver parity error flag
		// b4 LFERR   - line receiver framing error - received stop bit not HIGH
		// b5 LDCD    - line data carrier detected
		// b6 LRXRDY  - line receiver ready - data available
		// b7 LTXRDY  - line transmitter ready - transmit holding register empty
		m_sr[0] = (m_sr[0] & 0x27) | m_line_uart->status();
		break;
	case 0x1: // Status Register 1
		// b0
		// b1 IARXRDY - IBUS A receiver ready - data available
		// b2
		// b3 TPERR   - tape receiver parity error flag
		// b4 TFERR   - tape receiver framing error - received stop nit not HIGH
		// b5 TDCD    - tape data carrier detected
		// b6 TRXRDY  - tape receiver ready - data available
		// b7 TTXRDY  - tape transmitter ready - transmit holding register empty
		m_sr[1] = (m_sr[1] & 0x27) | m_tape_uart->status();
		break;
	case 0x2: // Mode Register
		break;
	case 0x3: // Command Register
		break;
	case 0x4: // Line Receive Holding Register
		m_sr[4] = m_line_uart->rx_byte();
		break;
	case 0x5: // Tape Receive Holding Register
		m_sr[5] = m_tape_uart->rx_byte();
		break;
	case 0x6: // Port A
		m_sr[6] = m_in_a_handler() & 0x1f;
		break;
	case 0x7: // Port B
		m_sr[7] = m_in_b_handler() & 0x0f;
		break;
	case 0x8: // Dial Control and Timing Register
		break;
	case 0xa: // IBUS A Registers
		logerror("IBUS Receiver A not implemented %02x\n", m_sr[m_index]);
		break;
	case 0xb: // IBUS B Registers
		logerror("IBUS Receiver B not implemented %02x\n", m_sr[m_index]);
		break;
	}

	return m_sr[m_index];
}

void saa5070_device::data_w(uint8_t data)
{
	m_sr[m_index] = data;

	switch (m_index)
	{
	case 0x2: // Mode Register
	{
		// b0
		// b1
		// b2 nTPEN    - tape parity enable command
		// b3 TPO/nE   - tape parity odd/even command
		// b4
		// b5 n75/1200 - baud rate selection command for line modulator and line transmit shift register
		// b6 nLPEN    - line parity enable command
		// b7 LPO/nE   - line parity odd/even command
		int tape_parity = device_serial_interface::PARITY_NONE;
		if (BIT(data, 2))
			tape_parity = BIT(data, 3) ? device_serial_interface::PARITY_ODD : device_serial_interface::PARITY_EVEN;
		m_tape_uart->set_baud_rate(1300, 1300, tape_parity);
		int line_parity = device_serial_interface::PARITY_NONE;
		if (BIT(data, 6))
			line_parity = BIT(data, 7) ? device_serial_interface::PARITY_ODD : device_serial_interface::PARITY_EVEN;
		m_line_uart->set_baud_rate(1200, BIT(data, 5) ? 1200 : 75, line_parity); // V23 Modem (1200/75 baud)
		break;
	}
	case 0x3: // Command Register
		// b0 nDLEN A/nDLIM A - three line/two line control for IBUS A receiver
		// b1 CLCK IN/nOUT    - input/output control for 62.5 kHz clock pin
		// b2 TTXEN           - tape transmitter enable
		// b3 TRXEN           - tape receiver enable
		// b4 LDBEN           - line demodulator output buffer enable
		// b5 RESET
		// b6 LTXEN           - line transmitter and modulator enable
		// b7 LRXEN           - line receiver enable
		m_tape_uart->tx_enable(BIT(data, 2));
		m_tape_uart->rx_enable(BIT(data, 3));

		if (BIT(data, 5))
		{
			reset();
		}

		m_line_uart->tx_enable(BIT(data, 6));
		m_line_uart->rx_enable(BIT(data, 7));
		break;
	case 0x4: // Line Transmit Holding Register
		m_line_uart->tx_byte(data);
		break;
	case 0x5: // Tape Transmit Holding Register
		m_tape_uart->tx_byte(data);
		break;
	case 0x6: // Port A
		m_out_a_handler(data & 0x1f);
		break;
	case 0x7: // Port B
		m_out_b_handler(data & 0x0f);
		break;
	case 0x8: // Dial Control and Timing Register
		// b0 D10
		// b1 D11
		// b2 D12
		// b3 D13
		// b4 1.5s TIMER
		// b5 DIAL GO
		// b6 60s TIMER
		// b7 UK/nEUR    - impulsing ratio control for UK and European standards
		if (BIT(data, 4))
		{
			m_timeout_timer->adjust(attotime::from_msec(1'500));
		}
		if (BIT(data, 5))
		{
			m_sr[0] &= ~0x24; // LIDCD/LDCD
			int digit_counter = (data & 0x0f) == 0 ? 10 : (data & 0x0f);
			// each digit impulse = 100ms, inter digit pause = 800ms
			m_dial_timer->adjust(attotime::from_msec((100 * digit_counter) + 800)); // TODO: digit impulse generator output on IMP line
		}
		if (BIT(data, 6))
		{
			m_timeout_timer->adjust(attotime::from_msec(60'000));
		}
		break;
	case 0xb: // IBUS B Registers
		logerror("IBUS Transmitter B not implemented %02x\n", m_sr[m_index]);
		break;
	}
}

uint8_t saa5070_device::read_direct(offs_t offset)
{
	m_index = offset & 0x0f;

	return data_r();
}

void saa5070_device::write_direct(offs_t offset, uint8_t data)
{
	m_index = offset & 0x0f;

	data_w(data);
}
