// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio HPC-102 serial interface emulation

**********************************************************************/

#include "emu.h"
#include "hpc102.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define M82C50A_TAG     "u1"
#define RS232_TAG      "rs232"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(POFO_HPC102, pofo_hpc102_device, "pofo_hpc102", "Atari Portfolio HPC-102")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void pofo_hpc102_device::device_add_mconfig(machine_config &config)
{
	INS8250(config, m_uart, XTAL(1'843'200)); // should be INS8250A
	m_uart->out_tx_callback().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	m_uart->out_dtr_callback().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	m_uart->out_rts_callback().set(RS232_TAG, FUNC(rs232_port_device::write_rts));
	m_uart->out_int_callback().set(FUNC(device_portfolio_expansion_slot_interface::eint_w));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(ins8250_uart_device::rx_w));
	rs232.dcd_handler().set(m_uart, FUNC(ins8250_uart_device::dcd_w));
	rs232.dsr_handler().set(m_uart, FUNC(ins8250_uart_device::dsr_w));
	rs232.ri_handler().set(m_uart, FUNC(ins8250_uart_device::ri_w));
	rs232.cts_handler().set(m_uart, FUNC(ins8250_uart_device::cts_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pofo_hpc102_device - constructor
//-------------------------------------------------

pofo_hpc102_device::pofo_hpc102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, POFO_HPC102, tag, owner, clock),
	device_portfolio_expansion_slot_interface(mconfig, *this),
	m_uart(*this, M82C50A_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pofo_hpc102_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pofo_hpc102_device::device_reset()
{
	m_uart->reset();
}


//-------------------------------------------------
//  eack_r - external interrupt acknowledge
//-------------------------------------------------

uint8_t pofo_hpc102_device::eack_r()
{
	return m_vector;
}


//-------------------------------------------------
//  nrdi_r - read
//-------------------------------------------------

uint8_t pofo_hpc102_device::nrdi_r(offs_t offset, uint8_t data, bool iom, bool bcom, bool ncc1)
{
	if (!bcom)
	{
		if ((offset & 0x0f) == 0x0f)
		{
			data = 0x01;
		}

		if (!(offset & 0x08))
		{
			data = m_uart->ins8250_r(offset & 0x07);
		}
	}

	return data;
}


//-------------------------------------------------
//  nwri_w - write
//-------------------------------------------------

void pofo_hpc102_device::nwri_w(offs_t offset, uint8_t data, bool iom, bool bcom, bool ncc1)
{
	if (!bcom)
	{
		if ((offset & 0x0f) == 0x0f)
		{
			m_vector = data;
		}

		if (!(offset & 0x08))
		{
			m_uart->ins8250_w(offset & 0x07, data);
		}
	}
}
