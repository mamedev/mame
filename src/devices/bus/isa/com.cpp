// license:BSD-3-Clause
// copyright-holders:Carl,Miodrag Milanovic
/***************************************************************************

  ISA 8 bit Generic Communication Card

***************************************************************************/

#include "emu.h"
#include "com.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/ser_mouse.h"
#include "bus/rs232/terminal.h"
#include "bus/rs232/null_modem.h"
#include "machine/ins8250.h"

static void isa_com(device_slot_interface &device)
{
	device.option_add("microsoft_mouse", MSFT_SERIAL_MOUSE);
	device.option_add("msystems_mouse", MSYSTEM_SERIAL_MOUSE);
	device.option_add("terminal", SERIAL_TERMINAL);
	device.option_add("null_modem", NULL_MODEM);
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_COM, isa8_com_device, "isa_com", "Communications Adapter PC/XT")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(isa8_com_device::device_add_mconfig)
	MCFG_DEVICE_ADD( "uart_0", INS8250, XTAL(1'843'200) )
	MCFG_INS8250_OUT_TX_CB(WRITELINE("serport0", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(WRITELINE("serport0", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(WRITELINE("serport0", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(WRITELINE(*this, isa8_com_device, pc_com_interrupt_1))
	MCFG_DEVICE_ADD( "uart_1", INS8250, XTAL(1'843'200) )
	MCFG_INS8250_OUT_TX_CB(WRITELINE("serport1", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(WRITELINE("serport1", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(WRITELINE("serport1", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(WRITELINE(*this, isa8_com_device, pc_com_interrupt_2))
	/*MCFG_DEVICE_ADD( "uart_2", INS8250, XTAL(1'843'200) )
	MCFG_INS8250_OUT_TX_CB(WRITELINE("serport2", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(WRITELINE("serport2", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(WRITELINE("serport2", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(WRITELINE(*this, isa8_com_device, pc_com_interrupt_1))
	MCFG_DEVICE_ADD( "uart_3", INS8250, XTAL(1'843'200) )
	MCFG_INS8250_OUT_TX_CB(WRITELINE("serport3", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(WRITELINE("serport3", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(WRITELINE("serport3", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(WRITELINE(*this, isa8_com_device, pc_com_interrupt_2))*/

	MCFG_DEVICE_ADD( "serport0", RS232_PORT, isa_com, "microsoft_mouse" )
	MCFG_RS232_RXD_HANDLER(WRITELINE("uart_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("uart_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(WRITELINE("uart_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(WRITELINE("uart_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("uart_0", ins8250_uart_device, cts_w))

	MCFG_DEVICE_ADD( "serport1", RS232_PORT, isa_com, nullptr )
	MCFG_RS232_RXD_HANDLER(WRITELINE("uart_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("uart_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(WRITELINE("uart_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(WRITELINE("uart_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("uart_1", ins8250_uart_device, cts_w))

	//MCFG_DEVICE_ADD( "serport2", RS232_PORT, isa_com, nullptr )
	//MCFG_RS232_RXD_HANDLER(WRITELINE("uart_1", ins8250_uart_device, rx_w))
	//MCFG_RS232_DCD_HANDLER(WRITELINE("uart_1", ins8250_uart_device, dcd_w))
	//MCFG_RS232_DSR_HANDLER(WRITELINE("uart_1", ins8250_uart_device, dsr_w))
	//MCFG_RS232_RI_HANDLER(WRITELINE("uart_1", ins8250_uart_device, ri_w))
	//MCFG_RS232_CTS_HANDLER(WRITELINE("uart_1", ins8250_uart_device, cts_w))

	//MCFG_DEVICE_ADD( "serport3", RS232_PORT, isa_com, nullptr )
	//MCFG_RS232_RXD_HANDLER(WRITELINE("uart_2", ins8250_uart_device, rx_w))
	//MCFG_RS232_DCD_HANDLER(WRITELINE("uart_2", ins8250_uart_device, dcd_w))
	//MCFG_RS232_DSR_HANDLER(WRITELINE("uart_2", ins8250_uart_device, dsr_w))
	//MCFG_RS232_RI_HANDLER(WRITELINE("uart_2", ins8250_uart_device, ri_w))
	//MCFG_RS232_CTS_HANDLER(WRITELINE("uart_2", ins8250_uart_device, cts_w))
MACHINE_CONFIG_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_com_device - constructor
//-------------------------------------------------

isa8_com_device::isa8_com_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_com_device(mconfig, ISA8_COM, tag, owner, clock)
{
}

isa8_com_device::isa8_com_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_com_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x03f8, 0x03ff, read8_delegate(FUNC(ins8250_device::ins8250_r), subdevice<ins8250_uart_device>("uart_0")), write8_delegate(FUNC(ins8250_device::ins8250_w), subdevice<ins8250_uart_device>("uart_0")) );
	m_isa->install_device(0x02f8, 0x02ff, read8_delegate(FUNC(ins8250_device::ins8250_r), subdevice<ins8250_uart_device>("uart_1")), write8_delegate(FUNC(ins8250_device::ins8250_w), subdevice<ins8250_uart_device>("uart_1")) );
//  m_isa->install_device(0x03e8, 0x03ef, read8_delegate(FUNC(ins8250_device::ins8250_r), subdevice<ins8250_uart_device>("uart_2")), write8_delegate(FUNC(ins8250_device::ins8250_w), subdevice<ins8250_uart_device>("uart_2")) );
//  m_isa->install_device(0x02e8, 0x02ef, read8_delegate(FUNC(ins8250_device::ins8250_r), subdevice<ins8250_uart_device>("uart_3")), write8_delegate(FUNC(ins8250_device::ins8250_w), subdevice<ins8250_uart_device>("uart_3")) );
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_com_device::device_reset()
{
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_COM_AT, isa8_com_at_device, "isa_com_at", "Communications Adapter")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(isa8_com_at_device::device_add_mconfig)
	MCFG_DEVICE_ADD( "uart_0", NS16450, XTAL(1'843'200) ) /* Verified: IBM P/N 6320947 Serial/Parallel card uses an NS16450N */
	MCFG_INS8250_OUT_TX_CB(WRITELINE("serport0", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(WRITELINE("serport0", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(WRITELINE("serport0", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(WRITELINE(*this, isa8_com_device, pc_com_interrupt_1))
	MCFG_DEVICE_ADD( "uart_1", NS16450, XTAL(1'843'200) )
	MCFG_INS8250_OUT_TX_CB(WRITELINE("serport1", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(WRITELINE("serport1", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(WRITELINE("serport1", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(WRITELINE(*this, isa8_com_device, pc_com_interrupt_2))
	/*MCFG_DEVICE_ADD( "uart_2", NS16450, XTAL(1'843'200) )
	MCFG_INS8250_OUT_TX_CB(WRITELINE("serport2", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(WRITELINE("serport2", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(WRITELINE("serport2", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(WRITELINE(*this, isa8_com_device, pc_com_interrupt_1))
	MCFG_DEVICE_ADD( "uart_3", NS16450, XTAL(1'843'200) )
	MCFG_INS8250_OUT_TX_CB(WRITELINE("serport3", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(WRITELINE("serport3", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(WRITELINE("serport3", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(WRITELINE(*this, isa8_com_device, pc_com_interrupt_2))*/
	MCFG_DEVICE_ADD( "serport0", RS232_PORT, isa_com, "microsoft_mouse" )
	MCFG_RS232_RXD_HANDLER(WRITELINE("uart_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("uart_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(WRITELINE("uart_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(WRITELINE("uart_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("uart_0", ins8250_uart_device, cts_w))

	MCFG_DEVICE_ADD( "serport1", RS232_PORT, isa_com, nullptr )
	MCFG_RS232_RXD_HANDLER(WRITELINE("uart_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("uart_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(WRITELINE("uart_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(WRITELINE("uart_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("uart_1", ins8250_uart_device, cts_w))

//  MCFG_DEVICE_ADD( "serport2", RS232_PORT, isa_com, nullptr )
//  MCFG_DEVICE_ADD( "serport3", RS232_PORT, isa_com, nullptr )
MACHINE_CONFIG_END

//-------------------------------------------------
//  isa8_com_device - constructor
//-------------------------------------------------

isa8_com_at_device::isa8_com_at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_com_device(mconfig, ISA8_COM_AT, tag, owner, clock)
{
}
