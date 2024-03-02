// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion Condor

    The Condor chip was introduced with the Series 3c and Siena for serial
    communications. The new chip enabled serial port speeds upto 57.6K baud on
    the Series 3c (19.2K baud on the Siena)

    This is a 64 pin package and pinout is unknown, so everything needs verifying
    if a datasheet/specification is ever found.

    TODO:
    - everything, require datasheet/specification and 3c/Siena schematic for info.

******************************************************************************/

#include "emu.h"
#include "psion_condor.h"

#define VERBOSE 0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(PSION_CONDOR, psion_condor_device, "psion_condor", "Psion Condor")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

psion_condor_device::psion_condor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_CONDOR, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_txd_handler(*this)
	, m_rxc_handler(*this)
	, m_rts_handler(*this)
	, m_dtr_handler(*this)
	, m_int_handler(*this)
	, m_cts(1)
	, m_dsr(1)
	, m_ri(1)
	, m_dcd(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_condor_device::device_start()
{
	save_item(NAME(m_cts));
	save_item(NAME(m_dsr));
	save_item(NAME(m_ri));
	save_item(NAME(m_dcd));
	save_item(NAME(m_uart_divisor));
	save_item(NAME(m_uart_line_control));
	save_item(NAME(m_int_mask));
	save_item(NAME(m_control1));
	save_item(NAME(m_parallel_ddr));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psion_condor_device::device_reset()
{
	m_int_mask = 0x00;
	m_control1 = 0x00;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint8_t psion_condor_device::read(offs_t offset)
{
	uint8_t data = 0x00;

	switch (offset & 0x0f)
	{
	case 0x00: // UARTReceiveBufferRegister
		//receive_register_extract();
		//data = get_received_char();
		LOG("%s read: UARTReceiveBufferRegister => %02x\n", machine().describe_context(), data);
		break;

	case 0x01: // UARTInterruptEnableRegister
		LOG("%s read: UARTInterruptEnableRegister => %02x\n", machine().describe_context(), data);
		break;

	case 0x02: // UARTInterruptIdentificRegister
		// b0 ERBF
		// b1 ETBEI - Enable Transmit Interrupts
		// b2 ELSI
		// b3 EDSSI - Enable Modem Status Interrupts
		LOG("%s read: UARTInterruptIdentificRegister => %02x\n", machine().describe_context(), data);
		break;

	case 0x05: // UARTLineStatusRegister
		// b5 THRE
		// b6 TEMT - Transmitter Empty
		data |= is_transmit_register_empty() ? 0x40 : 0x00;
		LOG("%s read: UARTLineStatusRegister => %02x\n", machine().describe_context(), data);
		break;

	case 0x06: // UARTModemStatusRegister
		// b4 CTSLine
		// b5 DSRLine
		// b6 RILine
		// b7 DCDLine
		data |= m_cts << 4;
		data |= m_dsr << 5;
		data |= m_ri  << 6;
		data |= m_dcd << 7;
		LOG("%s read: UARTModemStatusRegister => %02x\n", machine().describe_context(), data);
		break;

	case 0x08: // FifoDataRegister
		data = m_rx_fifo.dequeue();
		LOG("%s read: FifoDataRegister => %02x\n", machine().describe_context(), data);
		break;

	case 0x0a: // ParallelDataRegister
		// b7 HighPowerEnable
		LOG("%s read: ParallelDataRegister => %02x\n", machine().describe_context(), data);
		break;

	case 0x0c: // InterruptStatusRegister
		// b0 UINT
		// b1 RHFINT - Receive Half Full
		// b2 RTOINT
		// b3 THEINT - Transmit Half Empty
		// b4 MP1INT
		// b5 MP2INT
		LOG("%s read: InterruptStatusRegister => %02x\n", machine().describe_context(), data);
		break;

	case 0x0e: // StatusRegister
		// b0 RXFE  - Receive FIFO Empty
		// b1 TXFE  - Transmit FIFO Empty
		// b2 RFERR
		// b3 RFE   - Receive Framing Error
		// b4 RPE   - Receive Parity Error
		// b5 FOVR
		// b6 PLOCK - Phase Locked Loop Locked
		data |= m_rx_fifo.empty()          ? 0x01 : 0x00;
		data |= m_tx_fifo.empty()          ? 0x02 : 0x00;
		data |= is_receive_framing_error() ? 0x08 : 0x00;
		data |= is_receive_parity_error()  ? 0x10 : 0x00;
		//LOG("%s read: StatusRegister => %02x\n", machine().describe_context(), data);
		break;

	default:
		LOG("%s read: unknown register %02x => %02x\n", machine().describe_context(), offset, data);
		break;
	}

	return data;
}


void psion_condor_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x0f)
	{
	case 0x00: // UARTTransmitterHoldingRegister / UARTDivisorLatchLSB
		if (BIT(m_uart_line_control, 7))
		{
			LOG("%s write: UARTDivisorLatchLSB <= %02x\n", machine().describe_context(), data);
			m_uart_divisor = (m_uart_divisor & 0xff00) | data;
		}
		else
		{
			LOG("%s write: UARTTransmitterHoldingRegister <= %02x\n", machine().describe_context(), data);
			//transmit_byte(data);
		}
		break;

	case 0x01: // UARTInterruptEnableRegister / UARTDivisorLatchMSB
		if (BIT(m_uart_line_control, 7))
		{
			LOG("%s write: UARTDivisorLatchMSB <= %02x\n", machine().describe_context(), data);
			m_uart_divisor = (m_uart_divisor & 0x00ff) | (data << 8);
		}
		else
		{
			// b0 ERBF
			// b1 ETBEI - Enable Transmit Interrupts
			// b2 ELSI
			// b3 EDSSI - Enable Modem Status Interrupts
			LOG("%s write: UARTInterruptEnableRegister <= %02x\n", machine().describe_context(), data);
		}
		break;

	case 0x03: // UARTLineControlRegister
		// b2       - Select Two Stop Bits
		// b7 DLAB  - Divisor Latch
		LOG("%s write: UARTLineControlRegister <= %02x\n", machine().describe_context(), data);
		m_uart_line_control = data;
		set_data_frame(1, 8, PARITY_NONE, BIT(data, 2) ? STOP_BITS_2 : STOP_BITS_1);
		break;

	case 0x04: // UARTModemControlRegister
		// b0 DTRLine1
		// b1 RTSLine1
		// b2 DTRLine2
		// b3 RTSLine2
		// b4 LOOPBACK
		m_dtr_handler(BIT(data, 0));
		m_rts_handler(BIT(data, 1));
		LOG("%s write: UARTModemControlRegister <= %02x\n", machine().describe_context(), data);
		break;

	case 0x07: // UARTScratchRegister
		LOG("%s write: UARTScratchRegister <= %02x\n", machine().describe_context(), data);
		break;

	case 0x08: // FifoDataRegister
		LOG("%s write: FifoDataRegister <= %02x\n", machine().describe_context(), data);
		m_tx_fifo.enqueue(data);
		break;

	case 0x09: // ControlRegister1
		// b0 TFCLR - Transmit FIFO Clear
		// b1 RFCLR - Receive FIFO Clear
		// b2
		// b3 IRSD
		// b4 ESEL1
		// b5 ESEL2
		// b6 CLKEN
		// b7 PSRST
		LOG("%s write: ControlRegister1 <= %02x\n", machine().describe_context(), data);
		m_control1 = data;
		if (BIT(data, 0)) m_tx_fifo.clear();
		if (BIT(data, 1)) m_rx_fifo.clear();
		break;

	case 0x0a: // ParallelDataRegister
		// b7 HighPowerEnable
		LOG("%s write: ParallelDataRegister <= %02x\n", machine().describe_context(), data);
		break;

	case 0x0b: // ParallelDataDirectionRegister
		LOG("%s write: ParallelDataDirectionRegister <= %02x\n", machine().describe_context(), data);
		m_parallel_ddr = data;
		break;

	case 0x0d: // InterruptMaskAndMiscRegister
		// b0 EUINT
		// b1 ERHFINT
		// b2 ERTOINT
		// b3 ETHEINT
		// b4 PLLBP
		// b5 EMPINT
		// b6 RS232PORT1
		// b7 RS232PORT2
		LOG("%s write: InterruptMaskAndMiscRegister <= %02x\n", machine().describe_context(), data);
		m_int_mask = data;
		break;

	case 0x0f: // ControlRegister2
		// b0 THECLR - Clear Transmit Interrupt
		// b1 RHFCLR - Clear Receive Interrupt
		LOG("%s write: ControlRegister2 <= %02x\n", machine().describe_context(), data);
		break;

	default:
		LOG("%s write: unknown register %02x <= %02x\n", machine().describe_context(), offset, data);
		break;
	}
}
