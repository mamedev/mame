// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-805 RS-232 Interface for SVI 318/328

***************************************************************************/

#include "emu.h"
#include "sv805.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SV805, sv805_device, "sv805", "SV-805 RS-232 Interface")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sv805_device::device_add_mconfig(machine_config &config)
{
	INS8250(config, m_uart, XTAL(3'072'000));
	m_uart->out_int_callback().set(FUNC(sv805_device::uart_intr_w));
	m_uart->out_tx_callback().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_uart->out_dtr_callback().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_uart->out_rts_callback().set(m_rs232, FUNC(rs232_port_device::write_rts));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_uart, FUNC(ins8250_uart_device::rx_w));
	m_rs232->dcd_handler().set(m_uart, FUNC(ins8250_uart_device::dcd_w));
	m_rs232->dsr_handler().set(m_uart, FUNC(ins8250_uart_device::dsr_w));
	m_rs232->cts_handler().set(m_uart, FUNC(ins8250_uart_device::cts_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv805_device - constructor
//-------------------------------------------------

sv805_device::sv805_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SV805, tag, owner, clock),
	device_svi_slot_interface(mconfig, *this),
	m_uart(*this, "uart"),
	m_rs232(*this, "rs232")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sv805_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER( sv805_device::iorq_r )
{
	switch (offset)
	{
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
	case 0x2f:
		return m_uart->ins8250_r(space, offset & 0x07);
	}

	return 0xff;
}

WRITE8_MEMBER( sv805_device::iorq_w )
{
	switch (offset)
	{
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
	case 0x2f:
		m_uart->ins8250_w(space, offset & 0x07, data);
	}
}

WRITE_LINE_MEMBER( sv805_device::uart_intr_w )
{
	m_bus->int_w(state);
}
