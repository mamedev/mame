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
                      x   x   0   x   x   1   x   X  ROM
                      x   x   0   x   1   x   x   X  Magnetic Card Reader
                      x   x   0   0   x   x   x   x  Barcode reader
                      x   x   0   1   x   x   x   x  USA modem
                      x   1   0   x   x   x   x   x  Modem
                      1   x   0   x   x   x   x   x  RS232 TTL

    TODO:
    - implement UART

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
	, m_in_a_handler(*this)
	, m_in_b_handler(*this)
	, m_out_a_handler(*this)
	, m_out_b_handler(*this)
	, m_out_c_handler(*this)
	, m_out_d_handler(*this)
	, m_out_e_handler(*this)
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
	m_in_a_handler.resolve();
	m_in_b_handler.resolve();
	m_out_a_handler.resolve_safe();
	m_out_b_handler.resolve_safe();
	m_out_c_handler.resolve_safe();
	m_out_d_handler.resolve_safe();
	m_out_e_handler.resolve_safe();

	save_item(NAME(m_port_b_counter));
	save_item(NAME(m_port_b_latch));
	save_item(NAME(m_port_b_mode));
	save_item(NAME(m_port_dc_writes));
	save_item(NAME(m_int_mask));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psion_asic5_device::device_reset()
{
	m_port_b_counter = 0x00;
	m_port_b_latch   = 0x00;
	m_port_b_mode    = 0x00;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void psion_asic5_device::data_w(uint16_t data)
{
	switch (data & 0x300)
	{
	case NULL_FRAME:
		break;

	case CONTROL_FRAME:
		m_sibo_control = data & 0xff;

		switch (m_sibo_control & 0xc0)
		{
		case 0x80: // SerialWrite
			if (m_sibo_control == 0x93) // Multi Port D and C writes
				m_port_dc_writes = 0;
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
			if (BIT(m_port_b_mode, 1, 2) == 1) // Latch mode
			{
				m_out_b_handler(m_port_b_latch = data);
			}
			break;

		case 0x02: // Port B control
			LOG("%s data_w: Port B mode %02x\n", machine().describe_context(), data);
			m_port_b_mode = data;
			break;

		case 0x03: // Port D and C write data
			if (m_port_dc_writes)
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
			m_port_dc_writes++;
			break;

		case 0x06: // Interrupt mask write
			LOG("%s data_w: Interrupt mask write %02x\n", machine().describe_context(), data);
			m_int_mask = data;
			break;

		case 0x07: // Control register
			LOG("%s data_w: Control register %02x\n", machine().describe_context(), data);
			m_out_e_handler(data & 7);
			break;

		case 0x08: // UART Control register
			LOG("%s data_w: UART Control register %02x\n", machine().describe_context(), data);
			set_data_frame(1, 5 + BIT(data, 1, 2), BIT(data, 3) ? (BIT(data, 4) ? PARITY_ODD : PARITY_EVEN) : PARITY_NONE, BIT(data, 5) ? STOP_BITS_2 : STOP_BITS_1);
			break;

		case 0x09: // UART Transmit holding register
			LOG("%s data_w: UART Transmit holding register %02x\n", machine().describe_context(), data);
			transmit_register_setup(data);
			break;

		case 0x0a: // UART Baud rate LSB
			LOG("%s data_w: UART Baud rate LSB %02x\n", machine().describe_context(), data);
			break;

		case 0x0b: // UART Baud rate MSB
			LOG("%s data_w: UART Baud rate MSB %02x\n", machine().describe_context(), data);
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

		case 0x08: // UART Status register
			data |= is_transmit_register_empty() ? 1 << 3 : 0;
			data |= is_receive_framing_error()   ? 1 << 6 : 0;
			data |= is_receive_parity_error()    ? 1 << 7 : 0;
			LOG("%s data_r: UART Status register %02x\n", machine().describe_context(), data);
			break;

		case 0x09: // UART Receive register
			data = get_received_char();
			LOG("%s data_r: UART Receive register %02x\n", machine().describe_context(), data);
			break;

		case 0x0c: // Synchronous Port 1 read
			LOG("%s data_r: Synchronous Port 1 read %02x\n", machine().describe_context(), data);
			break;

		case 0x0d: // Barcode read data
			//  0  PC4     4  PB4
			//  1  PC7     5  PB5
			//  2  PB2     6  PB6
			//  3  PB3     7  PB7
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
