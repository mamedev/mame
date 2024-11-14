// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC5

    ASIC5 is a general purpose I/O chip with a built-in UART that can be set to
    run in a number of different modes thereby simplifying the task of peripheral
    design. For example, it is possible to set up ASIC5 to run as a Centronics
    parallel port interface, an 8-bit parallel I/O port, a serial bar code
    controller or a serial RS232 converter.

         PC6         PA7 PA6 PA5                           PA4 PA3             PA2 PA1 PA0
      Pack Mode      Memory Type                           No. devices         Memory Size
          1           0   0   0  RAM                        0   0  1 device     0   0   0  No memory
                      0   0   1  Type 1 Flash               0   1  2 devices    0   0   1  32K
                      0   1   0  Type 2 Flash               1   0  3 devices    0   1   0  64K
                      1   1   0  ROM                        1   1  4 devices    0   1   1  128K
                      1   1   1  Write protect                                  1   0   0  256K
                                                                                1   0   1  512K
                                                                                1   1   0  1M
    Peripheral Mode                                  Type
          0           x   x   0   x   x   x   x   1  RS232 port
                      x   x   0   x   x   x   1   x  Centronics (Parallel) port
                      x   x   0   x   x   1   x   x  ROM
                      x   x   0   x   1   x   x   x  Magnetic Card Reader
                      x   x   0   0   x   x   x   x  Barcode reader
                      x   x   0   1   x   x   x   x  USA modem
                      x   1   0   x   x   x   x   x  Modem
                      1   x   0   x   x   x   x   x  RS232 TTL

******************************************************************************/

#include "emu.h"
#include "psion_asic5.h"

#define VERBOSE 0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(PSION_ASIC5, psion_asic5_device, "psion_asic5", "Psion ASIC5")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

psion_asic5_device::psion_asic5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_ASIC5, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_mode(~uint8_t(0))
	, m_in_a_handler(*this, 0)
	, m_in_b_handler(*this, 0)
	, m_in_c_handler(*this, 0)
	, m_out_a_handler(*this)
	, m_out_b_handler(*this)
	, m_out_c_handler(*this)
	, m_out_d_handler(*this)
	, m_out_cs_handler(*this)
	, m_int_handler(*this)
	, m_txd_handler(*this)
	, m_rts_handler(*this)
	, m_dtr_handler(*this)
	, m_rxd(0)
	, m_cts(0)
	, m_dsr(0)
	, m_dcd(0)
{
}


//-------------------------------------------------
//  device_validity_check - device-specific checks
//-------------------------------------------------

void psion_asic5_device::device_validity_check(validity_checker &valid) const
{
	if ((PACK_MODE != m_mode) && (PERIPHERAL_MODE != m_mode))
		osd_printf_error("Power on reset pin PC6 not configured\n");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_asic5_device::device_start()
{
	save_item(NAME(m_port_b_counter));
	save_item(NAME(m_port_b_latch));
	save_item(NAME(m_port_b_mode));
	save_item(NAME(m_port_dc_select));
	save_item(NAME(m_int_mask));
	save_item(NAME(m_int_status));
	save_item(NAME(m_control));
	save_item(NAME(m_bdr));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psion_asic5_device::device_reset()
{
	m_port_b_counter = 0x00;
	m_port_b_latch   = 0x00;
	m_port_b_mode    = 0x00;
	m_port_dc_select = false;
	m_int_mask       = 0x00;
	m_int_status     = 0x02; // UART transmitter empty
	m_control        = 0x00;
	m_bdr            = 0xffff;

	receive_register_reset();
	transmit_register_reset();
}


void psion_asic5_device::update_interrupts()
{
	int irq = m_int_status & m_int_mask;

	m_int_handler(irq ? ASSERT_LINE : CLEAR_LINE);
}


void psion_asic5_device::rcv_callback()
{
	if (BIT(m_port_b_mode, 0)) // UART enabled
		receive_register_update_bit(m_rxd);
}

void psion_asic5_device::rcv_complete()
{
	receive_register_extract();

	if (is_receive_framing_error() || is_receive_parity_error())
		m_int_status |= 0x04;

	m_int_status |= 0x01;
	update_interrupts();
}

void psion_asic5_device::tra_callback()
{
	if (BIT(m_port_b_mode, 0)) // UART enabled
		m_txd_handler(transmit_register_get_data_bit());
}

void psion_asic5_device::tra_complete()
{
	m_int_status |= 0x02;
	update_interrupts();
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void psion_asic5_device::data_w(uint16_t data)
{
	switch (data & 0x300)
	{
	case NULL_FRAME:
		device_reset();
		break;

	case CONTROL_FRAME:
		m_sibo_control = data & 0xff;

		switch (m_sibo_control & 0xc0)
		{
		case 0x80: // SerialWrite
			if (m_sibo_control == 0x93) // Multi Port D and C writes
				m_port_dc_select = false;
			break;
		}
		break;

	case DATA_FRAME:
		data &= 0xff;
		switch (m_sibo_control & 0x0f)
		{
		case 0x00: // Port A write data
			LOG("%s data_w: Port A write data %02x\n", machine().describe_context(), data);
			m_out_a_handler(data);
			if (BIT(m_port_b_mode, 1, 2) == 0) // Counter mode
			{
				m_out_b_handler(++m_port_b_counter);
			}
			break;

		case 0x01: // Port B write data
			LOG("%s data_w: Port B write data %02x\n", machine().describe_context(), data);
			if (BIT(m_port_b_mode, 1, 2) == 1 && !BIT(m_control, 3)) // Latch mode
			{
				m_out_b_handler(m_port_b_latch = data);
			}
			break;

		case 0x02: // Port B control
			// b0 0 Memory mode, 1 Peripheral mode - enables UART
			// b1 Port B mode
			// b2 Port B mode
			// b3 0 Normal mode, 1 Test mode
			// b4 Not used
			// b5 Not used
			// b6 Not used
			// b7 Not used
			//
			// Port B mode
			// b2  b1  Mode
			// 0   0   Counter mode
			// 0   1   Latch mode
			// 1   0   Baud rate out on port B
			// 1   1   Test bus output on port B
			LOG("%s data_w: Port B mode %02x\n", machine().describe_context(), data);
			m_port_b_mode = data;
			break;

		case 0x03: // Port D and C write data
			switch (BIT(m_port_b_mode, 0))
			{
			case 0:
				if (m_port_dc_select)
				{
					LOG("%s data_w: Port C write data %02x\n", machine().describe_context(), data);
					m_out_c_handler(data);
				}
				else
				{
					LOG("%s data_w: Port D write data %02x\n", machine().describe_context(), data);
					m_out_d_handler(data);
					if (BIT(m_port_b_mode, 1, 2) == 0) // Counter mode
					{
						m_out_b_handler(m_port_b_counter = 0);
					}
				}
				m_port_dc_select = true;
				break;
			case 1: // UART enabled
				m_rts_handler(BIT(data, 1));
				m_dtr_handler(BIT(data, 2));
				break;
			}
			break;

		case 0x06: // Interrupt mask write
			// b0 UART - UART character received
			// b1 UART - UART transmitter empty
			// b2 UART - UART error or modem line status change
			// b3 PC7  - Barcode switch/general interrupt
			// b4 SR   - Synchronous port1 character received
			// b5 SR   - Synchronous port2 character received
			// b6 PC4  - Barcode data/general interrupt
			// b7 PA4  - Centronics busy low/general interrupt
			LOG("%s data_w: Interrupt mask write %02x\n", machine().describe_context(), data);
			m_int_mask = data;
			update_interrupts();
			break;

		case 0x07: // Control register
			LOG("%s data_w: Control register %02x\n", machine().describe_context(), data);
			m_control = data;
			m_out_cs_handler(data & 7);
			break;

		case 0x08: // UART Control register
			// b0 Generate break character
			// b1 Character length 1
			// b2 Character length 2
			// b3 Parity enabled if set
			// b4 Odd parity if, even if clear
			// b5 Set for two stop bits, clear for one
			// b6 Not used
			// b7 Not used
			LOG("%s data_w: UART Control register %02x\n", machine().describe_context(), data);
			set_data_frame(1, 5 + BIT(data, 1, 2), BIT(data, 3) ? (BIT(data, 4) ? PARITY_ODD : PARITY_EVEN) : PARITY_NONE, BIT(data, 5) ? STOP_BITS_2 : STOP_BITS_1);
			receive_register_reset();
			transmit_register_reset();
			break;

		case 0x09: // UART Transmit holding register
			LOG("%s data_w: UART Transmit holding register %02x\n", machine().describe_context(), data);
			transmit_register_setup(data);
			m_int_status &= ~0x02;
			update_interrupts();
			break;

		case 0x0a:
		case 0x0b: // UART Baud rate
			switch (m_sibo_control & 1)
			{
			case 0: m_bdr = (m_bdr & 0xff00) | (data << 0); break; // LSB
			case 1: m_bdr = (m_bdr & 0x00ff) | (data << 8); break; // MSB
			}
			LOG("%s data_w: UART Baud rate divisor %04x, Baud rate %d\n", machine().describe_context(), m_bdr, clock() / 16 / (1 - m_bdr));
			set_rate(clock() / 16 / (1 - m_bdr));
			break;

		case 0x0c: // Port 1 and 2 reset
			LOG("%s data_w: Port 1 and 2 reset %02x\n", machine().describe_context(), data);
			break;

		default:
			LOG("%s data_w: unknown control %02x data %02x\n", machine().describe_context(), m_sibo_control, data);
			break;
		}
		break;
	}
}


uint8_t psion_asic5_device::data_r()
{
	uint8_t data = 0x00;

	switch (m_sibo_control & 0xc0)
	{
	case 0x40: // SerialSelect
		switch (m_sibo_control & 0x0f)
		{
		case 0x02: // Asic5PackId
		case 0x03: // Asic5NormalId
			if (BIT(m_sibo_control, 0) == m_mode)
				data = m_info_byte; // A5InfoByte
			break;
		}
		break;

	case 0xc0: // SerialRead
		switch (m_sibo_control & 0x0f)
		{
		case 0x00: // Port A read data
			data = m_in_a_handler();
			LOG("%s data_r: Port A read data %02x\n", machine().describe_context(), data);
			if (BIT(m_port_b_mode, 1, 2) == 0) // Counter mode
			{
				m_out_b_handler(++m_port_b_counter);
			}
			break;

		case 0x01: // Port B read data
			if (BIT(m_port_b_mode, 1, 2) == 1) // Latch mode
			{
				data = m_port_b_latch;
			}
			LOG("%s data_r: Port B read data %02x\n", machine().describe_context(), data);
			break;

		case 0x02: // Port B counter
			if (BIT(m_port_b_mode, 1, 2) == 0) // Counter mode
			{
				m_out_b_handler(++m_port_b_counter);
			}
			LOG("%s data_r: Port B counter %02x\n", machine().describe_context(), m_port_b_counter);
			break;

		case 0x06: // Interrupt mask read
			data = m_int_mask;
			LOG("%s data_r: Interrupt mask read %02x\n", machine().describe_context(), data);
			break;

		case 0x07: // Interrupt status read
			data = m_int_status & m_int_mask;
			LOG("%s data_r: Interrupt status read %02x\n", machine().describe_context(), data);
			break;

		case 0x08: // UART Status register
			// b0 State of the CTS line (PA1)
			// b1 State of the DSR line (PA2)
			// b2 State of the DCD line (PA3)
			// b3 Transmitter buffer empty
			// b4 Transmitter busy
			// b5 Receive data waiting
			// b6 Overrun or framing error
			// b7 Parity error
			data |= m_cts << 0;
			data |= m_dsr << 1;
			data |= m_dcd << 2;
			data |= is_transmit_register_empty()  ? 1 << 3 : 0;
			data |= !is_transmit_register_empty() ? 1 << 4 : 0;
			data |= is_receive_register_full()    ? 1 << 5 : 0;
			data |= is_receive_framing_error()    ? 1 << 6 : 0;
			data |= is_receive_parity_error()     ? 1 << 7 : 0;
			LOG("%s data_r: UART Status register %02x\n", machine().describe_context(), data);
			m_int_status &= ~0x04;
			update_interrupts();
			break;

		case 0x09: // UART Receive register
			data = get_received_char();
			LOG("%s data_r: UART Receive register %02x\n", machine().describe_context(), data);
			m_int_status &= ~0x01;
			update_interrupts();
			break;

		case 0x0c: // Synchronous Port 1 read
			LOG("%s data_r: Synchronous Port 1 read %02x\n", machine().describe_context(), data);
			break;

		case 0x0d: // Barcode read data
			//  b0  PC4
			//  b1  PC7
			//  b2  PB2
			//  b3  PB3
			//  b4  PB4
			//  b5  PB5
			//  b6  PB6
			//  b7  PB7
			LOG("%s data_r: Barcode read data %02x\n", machine().describe_context(), data);
			break;

		case 0x0e: // Synchronous Port 2 read
			LOG("%s data_r: Synchronous Port 2 read %02x\n", machine().describe_context(), data);
			break;

		default:
			LOG("%s data_r: unknown control %02x data %02x\n", machine().describe_context(), m_sibo_control, data);
			break;
		}
		break;
	}

	return data;
}


void psion_asic5_device::write_rxd(int state)
{
	m_rxd = state;
	device_serial_interface::rx_w(state);
}

void psion_asic5_device::write_cts(int state)
{
	if (m_cts != state)
		m_int_status |= 0x04;
	m_cts = state;
	update_interrupts();

}
void psion_asic5_device::write_dsr(int state)
{
	if (m_dsr != state)
		m_int_status |= 0x04;
	m_dsr = state;
	update_interrupts();
}

void psion_asic5_device::write_dcd(int state)
{
	if (m_dcd != state)
		m_int_status |= 0x04;
	m_dcd = state;
	update_interrupts();
}
