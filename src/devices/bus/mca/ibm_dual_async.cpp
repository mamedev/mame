// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

	MCA 16-bit Dual Asynchronous Communication Card

	Configuration info from the ADF:
	
	Connector 1:
		choice "SERIAL_1"  pos[0]=XXXX000Xb  io 03f8h-03ffh  int 4
		choice "SERIAL_2"  pos[0]=XXXX001Xb  io 02f8h-02ffh  int 3
		choice "SERIAL_3"  pos[0]=XXXX010Xb  io 3220h-3227h  int 3
		choice "SERIAL_4"  pos[0]=XXXX011Xb  io 3228h-322fh  int 3
		choice "SERIAL_5"  pos[0]=XXXX100Xb  io 4220h-4227h  int 3
		choice "SERIAL_6"  pos[0]=XXXX101Xb  io 4228h-422fh  int 3
		choice "SERIAL_7"  pos[0]=XXXX110Xb  io 5220h-5227h  int 3
		choice "SERIAL_8"  pos[0]=XXXX111Xb  io 5228h-522fh  int 3

	Connector 2:
		choice "SERIAL_1"  pos[0]=1000XXXXb  io 03f8h-03ffh  int 4
		choice "SERIAL_2"  pos[0]=1001XXXXb  io 02f8h-02ffh  int 3
		choice "SERIAL_3"  pos[0]=1010XXXXb  io 3220h-3227h  int 3
		choice "SERIAL_4"  pos[0]=1011XXXXb  io 3228h-322fh  int 3
		choice "SERIAL_5"  pos[0]=1100XXXXb  io 4220h-4227h  int 3
		choice "SERIAL_6"  pos[0]=1101XXXXb  io 4228h-422fh  int 3
		choice "SERIAL_7"  pos[0]=1110XXXXb  io 5220h-5227h  int 3
		choice "SERIAL_8"  pos[0]=1111XXXXb  io 5228h-522fh  int 3

***************************************************************************/

#include "emu.h"
#include "ibm_dual_async.h"

#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "machine/ins8250.h"

#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

enum io_assignments
{
	SERIAL_1 = 0x3F8,
	SERIAL_2 = 0x2F8,
	SERIAL_3 = 0x3220,
	SERIAL_4 = 0x3228,
	SERIAL_5 = 0x4220,
	SERIAL_6 = 0x4228,
	SERIAL_7 = 0x5220,
	SERIAL_8 = 0x5228
};

static void isa_com(device_slot_interface &device)
{
	device.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
	device.option_add("logitech_mouse", LOGITECH_HLE_SERIAL_MOUSE);
	device.option_add("wheel_mouse", WHEEL_HLE_SERIAL_MOUSE);
	device.option_add("msystems_mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
	device.option_add("rotatable_mouse", ROTATABLE_HLE_SERIAL_MOUSE);
	device.option_add("terminal", SERIAL_TERMINAL);
	device.option_add("null_modem", NULL_MODEM);
	device.option_add("sun_kbd", SUN_KBD_ADAPTOR);
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA16_IBM_DUAL_ASYNC, mca16_ibm_dual_async_device, "mca_ibm_dual_async", "IBM Dual Async Adapter (@EEFF)")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_ibm_dual_async_device::device_add_mconfig(machine_config &config)
{
	NS16550(config, m_uart1, XTAL(1'843'200));
	m_uart1->out_tx_callback().set("serport0", FUNC(rs232_port_device::write_txd));
	m_uart1->out_dtr_callback().set("serport0", FUNC(rs232_port_device::write_dtr));
	m_uart1->out_rts_callback().set("serport0", FUNC(rs232_port_device::write_rts));
	m_uart1->out_int_callback().set(FUNC(mca16_ibm_dual_async_device::pc_com_interrupt_1));
	NS16550(config, m_uart2, XTAL(1'843'200));
	m_uart2->out_tx_callback().set("serport1", FUNC(rs232_port_device::write_txd));
	m_uart2->out_dtr_callback().set("serport1", FUNC(rs232_port_device::write_dtr));
	m_uart2->out_rts_callback().set("serport1", FUNC(rs232_port_device::write_rts));
	m_uart2->out_int_callback().set(FUNC(mca16_ibm_dual_async_device::pc_com_interrupt_2));

	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, nullptr));
	serport0.rxd_handler().set(m_uart1, FUNC(ins8250_uart_device::rx_w));
	serport0.dcd_handler().set(m_uart1, FUNC(ins8250_uart_device::dcd_w));
	serport0.dsr_handler().set(m_uart1, FUNC(ins8250_uart_device::dsr_w));
	serport0.ri_handler().set(m_uart1, FUNC(ins8250_uart_device::ri_w));
	serport0.cts_handler().set(m_uart1, FUNC(ins8250_uart_device::cts_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set(m_uart2, FUNC(ins8250_uart_device::rx_w));
	serport1.dcd_handler().set(m_uart2, FUNC(ins8250_uart_device::dcd_w));
	serport1.dsr_handler().set(m_uart2, FUNC(ins8250_uart_device::dsr_w));
	serport1.ri_handler().set(m_uart2, FUNC(ins8250_uart_device::ri_w));
	serport1.cts_handler().set(m_uart2, FUNC(ins8250_uart_device::cts_w));
}

//-------------------------------------------------
//  isa8_com_device - constructor
//-------------------------------------------------

mca16_ibm_dual_async_device::mca16_ibm_dual_async_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_ibm_dual_async_device(mconfig, MCA16_IBM_DUAL_ASYNC, tag, owner, clock)
{
}

mca16_ibm_dual_async_device::mca16_ibm_dual_async_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, 0xeeff),
	m_uart1(*this, "uart1"),
	m_uart2(*this, "uart2")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_ibm_dual_async_device::device_start()
{
	set_mca_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_ibm_dual_async_device::device_reset()
{
}

uint8_t mca16_ibm_dual_async_device::pos_r(offs_t offset)
{
	LOG("%s\n", FUNCNAME);

	switch(offset)
	{
		case 0:
			// Adapter Identification b0-b7
			return get_card_id() & 0xFF;
		case 1:
			// Adapter Identification b8-b15
			return (get_card_id() & 0xFF00) >> 8;
		case 2:
			// Option Select Data 1
			break;
		case 3:
			// Option Select Data 2
			break;
		case 4:
			// Option Select Data 3
			break;
		case 5:
			// Option Select Data 4
			break;
		case 6:
			// Subaddress Extension Low
			break;
		case 7:
			// Subaddress Extension High
			break;
		default:
			break;
	}

	return 0xFF;
}

void mca16_ibm_dual_async_device::pos_w(offs_t offset, uint8_t data)
{
	LOG("%s\n", FUNCNAME);

	switch(offset)
	{
		case 0:
			// Adapter Identification b0-b7
		case 1:
			// Adapter Identification b8-b15
			break;
		case 2:
			// Option Select Data 1
			m_option_select[0] = data;
			remap();
			break;
		case 3:
			// Option Select Data 2
			break;
		case 4:
			// Option Select Data 3
			break;
		case 5:
			// Option Select Data 4
			break;
		case 6:
			// Subaddress Extension Low
			break;
		case 7:
			// Subaddress Extension High
			break;
		default:
			break;
	}
}

uint8_t mca16_ibm_dual_async_device::uart1_r(offs_t offset)
{
    assert_card_feedback();
    return m_uart1->ins8250_r(offset);
}

void mca16_ibm_dual_async_device::uart1_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    m_uart1->ins8250_w(offset, data);
}

uint8_t mca16_ibm_dual_async_device::uart2_r(offs_t offset)
{
    assert_card_feedback();
    return m_uart2->ins8250_r(offset);
}

void mca16_ibm_dual_async_device::uart2_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    m_uart2->ins8250_w(offset, data);
}

void mca16_ibm_dual_async_device::unmap()
{
	
}

void mca16_ibm_dual_async_device::remap()
{
	update_serial_assignment(m_option_select[0]);
}

void mca16_ibm_dual_async_device::update_serial_assignment(uint8_t pos)
{
	uint8_t uart1_assignment = pos & 7;
	uint8_t uart2_assignment = (pos >> 4) & 7;

	uint8_t old_uart1_assignment = m_serial_assignment & 7;
	uint8_t old_uart2_assignment = (m_serial_assignment >> 4) & 7;

	if(m_is_mapped)
	{
		switch(old_uart1_assignment)
		{
			case 0: m_mca->unmap_device(SERIAL_1, SERIAL_1+7); break;
			case 1: m_mca->unmap_device(SERIAL_2, SERIAL_2+7); break;
			case 2: m_mca->unmap_device(SERIAL_3, SERIAL_3+7); break;
			case 3: m_mca->unmap_device(SERIAL_4, SERIAL_4+7); break;
			case 4: m_mca->unmap_device(SERIAL_5, SERIAL_5+7); break;
			case 5: m_mca->unmap_device(SERIAL_6, SERIAL_6+7); break;
			case 6: m_mca->unmap_device(SERIAL_7, SERIAL_7+7); break;
			case 7: m_mca->unmap_device(SERIAL_8, SERIAL_8+7); break;
			default: break;
		}

		switch(old_uart2_assignment)
		{
			case 0: m_mca->unmap_device(SERIAL_1, SERIAL_1+7); break;
			case 1: m_mca->unmap_device(SERIAL_2, SERIAL_2+7); break;
			case 2: m_mca->unmap_device(SERIAL_3, SERIAL_3+7); break;
			case 3: m_mca->unmap_device(SERIAL_4, SERIAL_4+7); break;
			case 4: m_mca->unmap_device(SERIAL_5, SERIAL_5+7); break;
			case 5: m_mca->unmap_device(SERIAL_6, SERIAL_6+7); break;
			case 6: m_mca->unmap_device(SERIAL_7, SERIAL_7+7); break;
			case 7: m_mca->unmap_device(SERIAL_8, SERIAL_8+7); break;
			default: break;
		}
	}

	switch(uart1_assignment)
	{
		case 0: 
		{ 
			m_mca->install_device(SERIAL_1, SERIAL_1+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_w)));
			m_cur_irq_uart1 = 4;
			break;
		}
		case 1:
		{
			m_mca->install_device(SERIAL_2, SERIAL_2+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_w)));
			m_cur_irq_uart1 = 3;
			break;
		}
		case 2: 
		{ 
			m_mca->install_device(SERIAL_3, SERIAL_3+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_w)));
			m_cur_irq_uart1 = 3;	
			break;	
		}
		case 3: 
		{
			m_mca->install_device(SERIAL_4, SERIAL_4+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_w)));
			m_cur_irq_uart1 = 3;	
			break;	
		}
		case 4: 
		{
			m_mca->install_device(SERIAL_5, SERIAL_5+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_w)));
			m_cur_irq_uart1 = 3;
			break;		
		} 
		case 5: 
		{ 
			m_mca->install_device(SERIAL_6, SERIAL_6+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_w)));
			m_cur_irq_uart1 = 3;
			break;		
		}
		case 6: 
		{
			m_mca->install_device(SERIAL_7, SERIAL_7+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_w)));
			m_cur_irq_uart1 = 3;
			break;			
		}
		case 7:
		{
			m_mca->install_device(SERIAL_8, SERIAL_8+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart1_w)));
			m_cur_irq_uart1 = 3;
			break;
		}
		default: break;
	}

	switch(uart2_assignment)
	{
		case 0: 
		{ 
			m_mca->install_device(SERIAL_1, SERIAL_1+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_w)));
			m_cur_irq_uart2 = 4;
			break;
		}
		case 1:
		{
			m_mca->install_device(SERIAL_2, SERIAL_2+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_w)));
			m_cur_irq_uart2 = 3;
			break;
		}
		case 2: 
		{ 
			m_mca->install_device(SERIAL_3, SERIAL_3+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_w)));
			m_cur_irq_uart2 = 3;	
			break;	
		}
		case 3: 
		{
			m_mca->install_device(SERIAL_4, SERIAL_4+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_w)));
			m_cur_irq_uart2 = 3;	
			break;	
		}
		case 4: 
		{
			m_mca->install_device(SERIAL_5, SERIAL_5+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_w)));
			m_cur_irq_uart2 = 3;	
			break;	
		} 
		case 5: 
		{ 
			m_mca->install_device(SERIAL_6, SERIAL_6+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_w)));
			m_cur_irq_uart2 = 3;		
			break;
		}
		case 6: 
		{
			m_mca->install_device(SERIAL_7, SERIAL_7+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_w)));
			m_cur_irq_uart2 = 3;
			break;		
		}
		case 7:
		{
			m_mca->install_device(SERIAL_8, SERIAL_8+7, 
				read8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_r)),
				write8sm_delegate(*this, FUNC(mca16_ibm_dual_async_device::uart2_w)));
			m_cur_irq_uart2 = 3;
			break;
		}
		default: break;
	}

	old_uart1_assignment = uart1_assignment;
	old_uart2_assignment = uart2_assignment;
}
