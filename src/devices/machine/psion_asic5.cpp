// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC5

    ASIC5 is a general purpose I/O chip with a built-in UART that can be set to
    run in a number of different modes thereby simplifying the task of peripheral
    design. For example, it is possible to set up ASIC5 to run as a Centronics
    parallel port interface, an 8-bit parallel I/O port, a serial bar code
    controller or a serial RS232 converter.

    TODO:
    - implement peripheral mode

******************************************************************************/

#include "emu.h"
#include "psion_asic5.h"

#define VERBOSE 0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(PSION_ASIC5_PACK,       psion_asic5_pack_device,       "psion_asic5_pack",       "Psion ASIC5 (Pack mode)")
DEFINE_DEVICE_TYPE(PSION_ASIC5_PERIPHERAL, psion_asic5_peripheral_device, "psion_asic5_peripheral", "Psion ASIC5 (Peripheral mode)")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

psion_asic5_device::psion_asic5_device(const machine_config &mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_in_a_handler(*this)
	, m_in_b_handler(*this)
	, m_out_a_handler(*this)
	, m_out_b_handler(*this)
	, m_out_c_handler(*this)
	, m_out_d_handler(*this)
{
}

psion_asic5_pack_device::psion_asic5_pack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: psion_asic5_device(mconfig, PSION_ASIC5_PACK, tag, owner, clock)
{
}

psion_asic5_peripheral_device::psion_asic5_peripheral_device(const machine_config &mconfig, const char * tag, device_t *owner, uint32_t clock)
	: psion_asic5_device(mconfig, PSION_ASIC5_PERIPHERAL, tag, owner, clock)
	, m_serial(*this, "serial")
	//, m_irq4_callback(*this)
	//, m_txd1_callback(*this)
	//, m_dtr1_callback(*this)
	//, m_rts1_callback(*this)
{
}


void psion_asic5_peripheral_device::device_add_mconfig(machine_config &config)
{
	NS16550(config, m_serial, clock() / 13);
	//m_serial->out_int_callback().set([this](int state) { m_irq4_callback(state); });
	//m_serial->out_tx_callback().set([this](int state) { m_txd1_callback(state); });
	//m_serial->out_dtr_callback().set([this](int state) { m_dtr1_callback(state); });
	//m_serial->out_rts_callback().set([this](int state) { m_rts1_callback(state); });
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
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psion_asic5_device::device_reset()
{
	m_portb_ctrl = 0x00;
	m_portb_counter = 0x00;
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
			//if (m_sibo_control == 0x82) // write A3Setup
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
			break;

		case 0x01: // Port B write data
			LOG("%s data_w: Port B write data %02x\n", machine().describe_context(), data);
			m_out_b_handler(data);
			break;

		case 0x02: // Port B control
			LOG("%s data_w: Port B control %02x\n", machine().describe_context(), data);
			m_portb_ctrl = data;
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
				m_portb_counter = 0x00;
				m_out_b_handler(m_portb_counter);
			}
			m_port_dc_writes++;
			break;

		case 0x06: // Interrupt mask write
			LOG("%s data_w: out int mask %02x\n", machine().describe_context(), data);
			m_interrupt_mask = data;
			break;

		//case 0x07: // Control register
		//  m = data;
		//  break;

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
		if (m_sibo_control == 0x42) // Asic5PackId
			data = m_info_byte; // A3InfoByte
		break;

	case 0xc0: // SerialRead
		switch (m_sibo_control & 0x0f)
		{
		case 0x00: // Port A read data
			data = m_in_a_handler();
			LOG("%s data_r: Port A read data %02x\n", machine().describe_context(), data);
			if (m_sibo_control & 0x90) // Multi, increment Port B
			{
				m_portb_counter++;
				m_out_b_handler(m_portb_counter);
			}
			break;

		case 0x01: // Port B read data
			data = m_in_b_handler();
			LOG("%s data_r: Port B read data %02x\n", machine().describe_context(), data);
			break;

		case 0x02: // Port B counter
			m_portb_counter++;
			m_out_b_handler(m_portb_counter);
			LOG("%s data_r: Port B counter %02x\n", machine().describe_context(), m_portb_counter);
			break;

		case 0x06: // Interrupt mask write
			data = m_interrupt_mask;
			LOG("%s data_r: in int mask %02x\n", machine().describe_context(), data);
			break;

			//case 0x07: // Control register
			//  m = data;
			//  break;

		default:
			LOG("%s data_r: unknown control %02x data %02x\n", machine().describe_context(), m_sibo_control, data);
			break;
		}
		break;
	}

	return data;
}
