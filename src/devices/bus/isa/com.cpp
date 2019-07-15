// license:BSD-3-Clause
// copyright-holders:Carl,Miodrag Milanovic
/***************************************************************************

  ISA 8 bit Generic Communication Card

***************************************************************************/

#include "emu.h"
#include "com.h"

#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "machine/ins8250.h"

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

DEFINE_DEVICE_TYPE(ISA8_COM, isa8_com_device, "isa_com", "Communications Adapter PC/XT")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_com_device::device_add_mconfig(machine_config &config)
{
	ins8250_device &uart0(INS8250(config, "uart_0", XTAL(1'843'200)));
	uart0.out_tx_callback().set("serport0", FUNC(rs232_port_device::write_txd));
	uart0.out_dtr_callback().set("serport0", FUNC(rs232_port_device::write_dtr));
	uart0.out_rts_callback().set("serport0", FUNC(rs232_port_device::write_rts));
	uart0.out_int_callback().set(FUNC(isa8_com_device::pc_com_interrupt_1));
	ins8250_device &uart1(INS8250(config, "uart_1", XTAL(1'843'200)));
	uart1.out_tx_callback().set("serport1", FUNC(rs232_port_device::write_txd));
	uart1.out_dtr_callback().set("serport1", FUNC(rs232_port_device::write_dtr));
	uart1.out_rts_callback().set("serport1", FUNC(rs232_port_device::write_rts));
	uart1.out_int_callback().set(FUNC(isa8_com_device::pc_com_interrupt_2));
	/*ins8250_device &uart2(INS8250(config, "uart_2", XTAL(1'843'200)));
	uart2.out_tx_callback().set("serport2", FUNC(rs232_port_device::write_txd));
	uart2.out_dtr_callback().set("serport2", FUNC(rs232_port_device::write_dtr));
	uart2.out_rts_callback().set("serport2", FUNC(rs232_port_device::write_rts));
	uart2.out_int_callback().set(FUNC(isa8_com_device::pc_com_interrupt_1));
	ins8250_device &uart3(INS8250(config, "uart_3", XTAL(1'843'200)));
	uart3.out_tx_callback().set("serport3", FUNC(rs232_port_device::write_txd));
	uart3.out_dtr_callback().set("serport3", FUNC(rs232_port_device::write_dtr));
	uart3.out_rts_callback().set("serport3", FUNC(rs232_port_device::write_rts));
	uart3.out_int_callback().set(FUNC(isa8_com_device::pc_com_interrupt_2));*/

	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, "logitech_mouse"));
	serport0.rxd_handler().set(uart0, FUNC(ins8250_uart_device::rx_w));
	serport0.dcd_handler().set(uart0, FUNC(ins8250_uart_device::dcd_w));
	serport0.dsr_handler().set(uart0, FUNC(ins8250_uart_device::dsr_w));
	serport0.ri_handler().set(uart0, FUNC(ins8250_uart_device::ri_w));
	serport0.cts_handler().set(uart0, FUNC(ins8250_uart_device::cts_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set(uart1, FUNC(ins8250_uart_device::rx_w));
	serport1.dcd_handler().set(uart1, FUNC(ins8250_uart_device::dcd_w));
	serport1.dsr_handler().set(uart1, FUNC(ins8250_uart_device::dsr_w));
	serport1.ri_handler().set(uart1, FUNC(ins8250_uart_device::ri_w));
	serport1.cts_handler().set(uart1, FUNC(ins8250_uart_device::cts_w));

	//rs232_port_device &serport2(RS232_PORT(config, "serport2", isa_com, nullptr));
	//serport2.rxd_handler().set(uart2, FUNC(ins8250_uart_device::rx_w));
	//serport2.dcd_handler().set(uart2, FUNC(ins8250_uart_device::dcd_w));
	//serport2.dsr_handler().set(uart2, FUNC(ins8250_uart_device::dsr_w));
	//serport2.ri_handler().set(uart2, FUNC(ins8250_uart_device::ri_w));
	//serport2.cts_handler().set(uart2, FUNC(ins8250_uart_device::cts_w));

	//rs232_port_device &serport3(RS232_PORT(config, "serport3", isa_com, nullptr));
	//serport3.rxd_handler().set(uart3, FUNC(ins8250_uart_device::rx_w));
	//serport3.dcd_handler().set(uart3, FUNC(ins8250_uart_device::dcd_w));
	//serport3.dsr_handler().set(uart3, FUNC(ins8250_uart_device::dsr_w));
	//serport3.ri_handler().set(uart3, FUNC(ins8250_uart_device::ri_w));
	//serport3.cts_handler().set(uart3, FUNC(ins8250_uart_device::cts_w));
}

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
	m_isa->install_device(0x03f8, 0x03ff, read8sm_delegate(FUNC(ins8250_device::ins8250_r), subdevice<ins8250_uart_device>("uart_0")), write8sm_delegate(FUNC(ins8250_device::ins8250_w), subdevice<ins8250_uart_device>("uart_0")) );
	m_isa->install_device(0x02f8, 0x02ff, read8sm_delegate(FUNC(ins8250_device::ins8250_r), subdevice<ins8250_uart_device>("uart_1")), write8sm_delegate(FUNC(ins8250_device::ins8250_w), subdevice<ins8250_uart_device>("uart_1")) );
//  m_isa->install_device(0x03e8, 0x03ef, read8sm_delegate(FUNC(ins8250_device::ins8250_r), subdevice<ins8250_uart_device>("uart_2")), write8sm_delegate(FUNC(ins8250_device::ins8250_w), subdevice<ins8250_uart_device>("uart_2")) );
//  m_isa->install_device(0x02e8, 0x02ef, read8sm_delegate(FUNC(ins8250_device::ins8250_r), subdevice<ins8250_uart_device>("uart_3")), write8sm_delegate(FUNC(ins8250_device::ins8250_w), subdevice<ins8250_uart_device>("uart_3")) );
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

void isa8_com_at_device::device_add_mconfig(machine_config &config)
{
	ns16450_device &uart0(NS16450(config, "uart_0", XTAL(1'843'200))); /* Verified: IBM P/N 6320947 Serial/Parallel card uses an NS16450N */
	uart0.out_tx_callback().set("serport0", FUNC(rs232_port_device::write_txd));
	uart0.out_dtr_callback().set("serport0", FUNC(rs232_port_device::write_dtr));
	uart0.out_rts_callback().set("serport0", FUNC(rs232_port_device::write_rts));
	uart0.out_int_callback().set(FUNC(isa8_com_device::pc_com_interrupt_1));
	ns16450_device &uart1(NS16450(config, "uart_1", XTAL(1'843'200)));
	uart1.out_tx_callback().set("serport1", FUNC(rs232_port_device::write_txd));
	uart1.out_dtr_callback().set("serport1", FUNC(rs232_port_device::write_dtr));
	uart1.out_rts_callback().set("serport1", FUNC(rs232_port_device::write_rts));
	uart1.out_int_callback().set(FUNC(isa8_com_device::pc_com_interrupt_2));
	/*ns16450_device &uart2(NS16450(config, "uart_2", XTAL(1'843'200)));
	uart2.out_tx_callback().set("serport2", FUNC(rs232_port_device::write_txd));
	uart2.out_dtr_callback().set("serport2", FUNC(rs232_port_device::write_dtr));
	uart2.out_rts_callback().set("serport2", FUNC(rs232_port_device::write_rts));
	uart2.out_int_callback().set(FUNC(isa8_com_device::pc_com_interrupt_1));
	ns16450_device &uart3(NS16450(config, "uart_3", XTAL(1'843'200)));
	uart3.out_tx_callback().set("serport3", FUNC(rs232_port_device::write_txd));
	uart3.out_dtr_callback().set("serport3", FUNC(rs232_port_device::write_dtr));
	uart3.out_rts_callback().set("serport3", FUNC(rs232_port_device::write_rts));
	uart3.out_int_callback().set(FUNC(isa8_com_device::pc_com_interrupt_2));*/

	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, "logitech_mouse"));
	serport0.rxd_handler().set(uart0, FUNC(ins8250_uart_device::rx_w));
	serport0.dcd_handler().set(uart0, FUNC(ins8250_uart_device::dcd_w));
	serport0.dsr_handler().set(uart0, FUNC(ins8250_uart_device::dsr_w));
	serport0.ri_handler().set(uart0, FUNC(ins8250_uart_device::ri_w));
	serport0.cts_handler().set(uart0, FUNC(ins8250_uart_device::cts_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set(uart1, FUNC(ins8250_uart_device::rx_w));
	serport1.dcd_handler().set(uart1, FUNC(ins8250_uart_device::dcd_w));
	serport1.dsr_handler().set(uart1, FUNC(ins8250_uart_device::dsr_w));
	serport1.ri_handler().set(uart1, FUNC(ins8250_uart_device::ri_w));
	serport1.cts_handler().set(uart1, FUNC(ins8250_uart_device::cts_w));

	//RS232_PORT(config, "serport2", isa_com, nullptr);
	//RS232_PORT(config, "serport3", isa_com, nullptr);
}

//-------------------------------------------------
//  isa8_com_device - constructor
//-------------------------------------------------

isa8_com_at_device::isa8_com_at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_com_device(mconfig, ISA8_COM_AT, tag, owner, clock)
{
}
